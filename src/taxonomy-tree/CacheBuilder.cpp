#include "CacheBuilder.hpp"
#include "ZipReader.hpp"
#include "CSVParser.hpp"
#include "TreeStrings.hpp"
#include <wx/datetime.h>
#include <wx/filename.h>

CacheBuilder::CacheBuilder()
    : m_db(nullptr)
{
}

CacheBuilder::~CacheBuilder()
{
    CloseDatabase();
}

bool CacheBuilder::OpenDatabase(const wxString& path)
{
    CloseDatabase();

    try
    {
        m_db = std::make_unique<SqliteDatabase>(path);
        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to open database: %s", e.what());
        return false;
    }
}

void CacheBuilder::CloseDatabase()
{
    m_db.reset();
}

bool CacheBuilder::CreateSchema()
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        // Metadata table
        m_db->Execute(
            "CREATE TABLE IF NOT EXISTS cache_metadata ("
            "    key TEXT PRIMARY KEY,"
            "    value TEXT NOT NULL"
            ")"
        );

        // Main taxonomy table
        m_db->Execute(
            "CREATE TABLE IF NOT EXISTS taxonomy ("
            "    id INTEGER PRIMARY KEY,"
            "    parent_id INTEGER DEFAULT -1,"
            "    scientific_name TEXT NOT NULL,"
            "    taxon_rank TEXT,"
            "    kingdom TEXT,"
            "    phylum TEXT,"
            "    taxon_class TEXT,"
            "    taxon_order TEXT,"
            "    family TEXT,"
            "    genus TEXT,"
            "    specific_epithet TEXT,"
            "    infraspecific_epithet TEXT"
            ")"
        );

        m_db->Execute("CREATE INDEX IF NOT EXISTS idx_taxonomy_parent_id ON taxonomy(parent_id)");
        m_db->Execute("CREATE INDEX IF NOT EXISTS idx_taxonomy_rank ON taxonomy(taxon_rank)");

        // Vernacular names table
        m_db->Execute(
            "CREATE TABLE IF NOT EXISTS vernacular_names ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    taxon_id INTEGER NOT NULL,"
            "    language TEXT NOT NULL,"
            "    vernacular_name TEXT NOT NULL,"
            "    country_code TEXT"
            ")"
        );

        m_db->Execute("CREATE INDEX IF NOT EXISTS idx_vernacular_taxon_id ON vernacular_names(taxon_id)");
        m_db->Execute("CREATE INDEX IF NOT EXISTS idx_vernacular_language ON vernacular_names(language)");

        // Hierarchy table
        m_db->Execute(
            "CREATE TABLE IF NOT EXISTS hierarchy ("
            "    parent_id INTEGER NOT NULL,"
            "    child_id INTEGER NOT NULL,"
            "    PRIMARY KEY (parent_id, child_id)"
            ")"
        );

        m_db->Execute("CREATE INDEX IF NOT EXISTS idx_hierarchy_child ON hierarchy(child_id)");

        // FTS5 search index
        m_db->Execute(
            "CREATE VIRTUAL TABLE IF NOT EXISTS search_index USING fts5("
            "    taxon_id UNINDEXED,"
            "    name_type UNINDEXED,"
            "    language UNINDEXED,"
            "    name,"
            "    taxon_rank UNINDEXED,"
            "    tokenize='porter unicode61 remove_diacritics 2'"
            ")"
        );

        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to create schema: %s", e.what());
        return false;
    }
}

bool CacheBuilder::InsertTaxonomyData(const std::map<long, TaxonomyEntry>& data,
                                      std::function<void(const wxString&, int)> progressCallback)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "INSERT INTO taxonomy (id, parent_id, scientific_name, taxon_rank, "
            "kingdom, phylum, taxon_class, taxon_order, family, genus, "
            "specific_epithet, infraspecific_epithet) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

        size_t count = 0;
        size_t total = data.size();

        for (const auto& [id, entry] : data)
        {
            stmt.BindInt64(1, entry.id);
            stmt.BindInt64(2, entry.parentId);
            stmt.BindText(3, entry.scientificName);
            stmt.BindText(4, entry.taxonRank);
            stmt.BindText(5, entry.kingdom);
            stmt.BindText(6, entry.phylum);
            stmt.BindText(7, entry.taxonClass);
            stmt.BindText(8, entry.order);
            stmt.BindText(9, entry.family);
            stmt.BindText(10, entry.genus);
            stmt.BindText(11, entry.specificEpithet);
            stmt.BindText(12, entry.infraspecificEpithet);

            stmt.Execute();
            stmt.Reset();

            count++;
            if (count % 1000 == 0 && progressCallback)
            {
                int progress = 30 + (count * 30 / total);
                progressCallback(wxString::Format("Inserted %zu/%zu taxonomy entries", count, total), progress);
            }
        }

        m_lastStats.taxonomyEntries = count;
        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to insert taxonomy data: %s", e.what());
        return false;
    }
}

bool CacheBuilder::InsertVernacularData(const std::vector<VernacularEntry>& data,
                                        const wxString& language,
                                        std::function<void(const wxString&, int)> progressCallback)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "INSERT INTO vernacular_names (taxon_id, language, vernacular_name, country_code) "
            "VALUES (?, ?, ?, ?)"
        );

        size_t count = 0;
        for (const auto& entry : data)
        {
            stmt.BindInt64(1, entry.taxonId);
            stmt.BindText(2, entry.language);
            stmt.BindText(3, entry.vernacularName);

            if (!entry.countryCode.IsEmpty())
                stmt.BindText(4, entry.countryCode);
            else
                stmt.BindNull(4);

            stmt.Execute();
            stmt.Reset();
            count++;
        }

        m_lastStats.vernacularEntries += count;

        if (progressCallback)
        {
            progressCallback(wxString::Format("Inserted %zu %s names", count, language), 0);
        }

        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to insert vernacular data: %s", e.what());
        return false;
    }
}

bool CacheBuilder::BuildSearchIndex(std::function<void(const wxString&, int)> progressCallback)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        if (progressCallback)
            progressCallback(TR_TREE(TreeStringId::ProgressBuildingSearchIndexVernacular), 70);

        // Insert vernacular names into FTS5
        m_db->Execute(
            "INSERT INTO search_index (taxon_id, name_type, language, name, taxon_rank) "
            "SELECT v.taxon_id, 'vernacular', v.language, v.vernacular_name, t.taxon_rank "
            "FROM vernacular_names v "
            "JOIN taxonomy t ON v.taxon_id = t.id"
        );

        if (progressCallback)
            progressCallback(TR_TREE(TreeStringId::ProgressBuildingSearchIndexScientific), 80);

        // Insert scientific names into FTS5
        m_db->Execute(
            "INSERT INTO search_index (taxon_id, name_type, language, name, taxon_rank) "
            "SELECT id, 'scientific', NULL, scientific_name, taxon_rank "
            "FROM taxonomy WHERE scientific_name != ''"
        );

        if (progressCallback)
            progressCallback(TR_TREE(TreeStringId::ProgressSearchIndexComplete), 90);

        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to build search index: %s", e.what());
        return false;
    }
}

bool CacheBuilder::UpdateMetadata(const wxString& key, const wxString& value)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "INSERT OR REPLACE INTO cache_metadata (key, value) VALUES (?, ?)"
        );

        stmt.BindText(1, key);
        stmt.BindText(2, value);
        stmt.Execute();

        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to update metadata: %s", e.what());
        return false;
    }
}

wxString CacheBuilder::CalculateChecksum(const wxString& path)
{
    wxStructStat st;
    if (wxStat(path, &st) != 0)
        return wxEmptyString;

    // Simple checksum: file size + modification time
    return wxString::Format("%lld-%lld",
        (long long)st.st_size,
        (long long)st.st_mtime);
}

bool CacheBuilder::BuildCache(
    const wxString& zipPath,
    const wxString& cachePath,
    const std::vector<wxString>& languages,
    std::function<void(const wxString&, int)> progressCallback)
{
    wxDateTime startTime = wxDateTime::Now();
    m_lastStats = BuildStats();

    // Delete old cache if exists
    if (wxFileExists(cachePath))
    {
        wxRemoveFile(cachePath);
    }

    // Open database
    if (!OpenDatabase(cachePath))
        return false;

    try
    {
        // Enable performance optimizations
        m_db->Execute("PRAGMA journal_mode=WAL");
        m_db->Execute("PRAGMA synchronous=NORMAL");
        m_db->Execute("PRAGMA cache_size=-64000");
        m_db->Execute("PRAGMA temp_store=MEMORY");

        if (progressCallback)
            progressCallback(TR_TREE(TreeStringId::ProgressCreatingSchema), 5);

        // Create schema
        if (!CreateSchema())
            return false;

        if (progressCallback)
            progressCallback("Opening taxonomy archive...", 10);

        // Open ZIP archive
        auto zipReader = std::make_unique<ZipReader>();
        if (!zipReader->OpenArchive(zipPath))
        {
            wxLogError("Failed to open ZIP archive: %s", zipPath);
            return false;
        }

        // Scan for all available languages in the ZIP
        std::vector<wxString> languageFiles = zipReader->GetLanguageFiles();
        std::vector<wxString> availableLanguages;
        for (const auto& file : languageFiles)
        {
            wxString language = CSVParser::ExtractLanguageFromFilename(file);
            if (!language.IsEmpty())
                availableLanguages.push_back(language);
        }

        if (progressCallback)
            progressCallback(TR_TREE_FMT(TreeStringId::ProgressFoundAvailableLanguages, availableLanguages.size()), 12);

        // Begin transaction
        SqliteTransaction transaction(*m_db);

        if (progressCallback)
            progressCallback("Loading taxonomy data...", 15);

        // Read and parse taxonomy CSV
        wxString taxonomyContent;
        if (!zipReader->ReadFileToString("taxa.csv", taxonomyContent))
        {
            wxLogError("Failed to read taxa.csv from archive");
            return false;
        }

        std::map<long, TaxonomyEntry> taxonomyMap;
        CSVParser parser;
        if (!parser.ParseTaxonomyCSV(taxonomyContent, taxonomyMap))
        {
            wxLogError("Failed to parse taxonomy CSV");
            return false;
        }

        if (progressCallback)
            progressCallback(TR_TREE_FMT(TreeStringId::ProgressInsertingTaxonomyEntries, taxonomyMap.size()), 20);

        // Insert taxonomy data
        if (!InsertTaxonomyData(taxonomyMap, progressCallback))
            return false;

        if (progressCallback)
            progressCallback("Building hierarchy...", 60);

        // Build hierarchy table
        m_db->Execute(
            "INSERT INTO hierarchy (parent_id, child_id) "
            "SELECT parent_id, id FROM taxonomy WHERE parent_id != -1"
        );

        // Load vernacular data for each language
        // Build language name -> code mapping (e.g., "lithuanian" -> "lt")
        std::map<wxString, wxString> languageMap;
        int langProgress = 0;
        for (const auto& lang : languages)
        {
            wxString filename = wxString::Format("VernacularNames-%s.csv", lang);
            wxString content;

            if (progressCallback)
            {
                int progress = 60 + (langProgress * 10 / languages.size());
                progressCallback(TR_TREE_FMT(TreeStringId::ProgressLoadingLanguage, lang.c_str()), progress);
            }

            if (zipReader->ReadFileToString(filename, content))
            {
                std::vector<VernacularEntry> entries;
                if (parser.ParseVernacularCSV(content, entries))
                {
                    InsertVernacularData(entries, lang, progressCallback);

                    // Extract actual language code from CSV data (e.g., "lt" from lithuanian.csv)
                    if (!entries.empty() && !entries[0].language.IsEmpty())
                    {
                        languageMap[lang] = entries[0].language;
                    }
                }
            }
            langProgress++;
        }

        // Commit transaction
        transaction.Commit();

        // Build search index (outside main transaction)
        if (!BuildSearchIndex(progressCallback))
            return false;

        // Store metadata
        UpdateMetadata("version", "1.0");
        UpdateMetadata("source_path", zipPath);
        UpdateMetadata("source_checksum", CalculateChecksum(zipPath));

        // Store ALL available languages (from ZIP file)
        wxArrayString availLangArray;
        for (const auto& lang : availableLanguages)
            availLangArray.Add(lang);
        UpdateMetadata("available_languages", wxJoin(availLangArray, ','));

        // Store cached languages (actually loaded)
        wxArrayString cachedLangArray;
        for (const auto& lang : languages)
            cachedLangArray.Add(lang);
        UpdateMetadata("cached_languages", wxJoin(cachedLangArray, ','));

        // Store language name -> code mapping (e.g., "lithuanian:lt,english:en")
        wxArrayString langMapArray;
        for (const auto& pair : languageMap)
        {
            langMapArray.Add(pair.first + ":" + pair.second);
        }
        UpdateMetadata("language_map", wxJoin(langMapArray, ','));

        UpdateMetadata("created_at", wxDateTime::Now().FormatISOCombined());

        if (progressCallback)
            progressCallback(TR_TREE(TreeStringId::ProgressCacheBuildComplete), 100);

        // Calculate build time
        wxTimeSpan buildTime = wxDateTime::Now() - startTime;
        m_lastStats.buildTimeSeconds = buildTime.GetSeconds().ToDouble();

        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to build cache: %s", e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        wxLogError("Failed to build cache: %s", e.what());
        return false;
    }
}

bool CacheBuilder::AddLanguageToCache(
    const wxString& zipPath,
    const wxString& cachePath,
    const wxString& language,
    std::function<void(const wxString&, int)> progressCallback)
{
    if (!OpenDatabase(cachePath))
        return false;

    try
    {
        // Check if language already cached
        SqliteStatement checkStmt(m_db->Get(),
            "SELECT value FROM cache_metadata WHERE key = 'cached_languages'"
        );

        if (checkStmt.Step())
        {
            wxString cachedLangs = checkStmt.GetColumnText(0);
            wxArrayString cachedLangArray = wxSplit(cachedLangs, ',');
            for (const auto& cached : cachedLangArray)
            {
                if (cached == language)
                {
                    // Already cached
                    return true;
                }
            }
        }

        if (progressCallback)
            progressCallback(TR_TREE_FMT(TreeStringId::ProgressLoadingLanguage, language.c_str()), 30);

        // Open ZIP and parse vernacular file
        auto zipReader = std::make_unique<ZipReader>();
        if (!zipReader->OpenArchive(zipPath))
            return false;

        wxString filename = wxString::Format("VernacularNames-%s.csv", language);
        wxString content;
        if (!zipReader->ReadFileToString(filename, content))
        {
            wxLogError("Failed to read %s from archive", filename);
            return false;
        }

        std::vector<VernacularEntry> entries;
        CSVParser parser;
        if (!parser.ParseVernacularCSV(content, entries))
        {
            wxLogError("Failed to parse vernacular CSV");
            return false;
        }

        // Begin transaction
        SqliteTransaction transaction(*m_db);

        // Insert vernacular data
        if (!InsertVernacularData(entries, language, progressCallback))
            return false;

        if (progressCallback)
            progressCallback(TR_TREE(TreeStringId::ProgressUpdatingSearchIndex), 70);

        // Extract actual language code from CSV data
        wxString languageCode;
        if (!entries.empty() && !entries[0].language.IsEmpty())
        {
            languageCode = entries[0].language;
        }

        // Update FTS5 index (using actual language code from CSV)
        if (!languageCode.IsEmpty())
        {
            m_db->Execute(wxString::Format(
                "INSERT INTO search_index (taxon_id, name_type, language, name, taxon_rank) "
                "SELECT v.taxon_id, 'vernacular', v.language, v.vernacular_name, t.taxon_rank "
                "FROM vernacular_names v "
                "JOIN taxonomy t ON v.taxon_id = t.id "
                "WHERE v.language = '%s'",
                languageCode
            ));
        }

        // Update cached_languages metadata
        SqliteStatement metaStmt(m_db->Get(),
            "SELECT value FROM cache_metadata WHERE key = 'cached_languages'"
        );

        wxString cachedLangs;
        if (metaStmt.Step())
        {
            cachedLangs = metaStmt.GetColumnText(0);
        }

        if (!cachedLangs.IsEmpty())
            cachedLangs += ",";
        cachedLangs += language;

        UpdateMetadata("cached_languages", cachedLangs);

        // Update language_map metadata
        if (!languageCode.IsEmpty())
        {
            SqliteStatement mapStmt(m_db->Get(),
                "SELECT value FROM cache_metadata WHERE key = 'language_map'"
            );

            wxString langMap;
            if (mapStmt.Step())
            {
                langMap = mapStmt.GetColumnText(0);
            }

            if (!langMap.IsEmpty())
                langMap += ",";
            langMap += language + ":" + languageCode;

            UpdateMetadata("language_map", langMap);
        }

        transaction.Commit();

        if (progressCallback)
            progressCallback(wxString::Format("Language %s added successfully", language), 100);

        return true;
    }
    catch (const SqliteException& e)
    {
        wxLogError("Failed to add language: %s", e.what());
        return false;
    }
}
