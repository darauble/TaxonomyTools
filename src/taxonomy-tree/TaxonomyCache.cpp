#include "TaxonomyCache.hpp"
#include "CacheBuilder.hpp"
#include "TaxonomyData.hpp"
#include "ZipReader.hpp"
#include <wx/filename.h>

TaxonomyCache::TaxonomyCache()
    : m_db(nullptr)
{
}

TaxonomyCache::~TaxonomyCache()
{
    CloseDatabase();
}

wxString TaxonomyCache::GetCachePath(const wxString& zipPath)
{
    wxString cachePath = zipPath;
    cachePath.Replace(".zip", ".sqlite");
    return cachePath;
}

bool TaxonomyCache::OpenDatabase(const wxString& path)
{
    CloseDatabase();

    try
    {
        m_db = std::make_unique<SqliteDatabase>(path, SQLITE_OPEN_READWRITE);
        m_cachePath = path;
        return true;
    }
    catch (const SqliteException& e)
    {
        printf("Failed to open cache database: %s\n", e.what());
        return false;
    }
}

void TaxonomyCache::CloseDatabase()
{
    m_db.reset();
    m_cachePath.Clear();
}

bool TaxonomyCache::IsOpen() const
{
    return m_db && m_db->IsOpen();
}

wxString TaxonomyCache::CalculateChecksum(const wxString& path)
{
    wxStructStat st;
    if (wxStat(path, &st) != 0)
        return wxEmptyString;

    // Simple checksum: file size + modification time
    return wxString::Format("%lld-%lld",
        (long long)st.st_size,
        (long long)st.st_mtime);
}

wxString TaxonomyCache::GetMetadata(const wxString& key)
{
    if (!m_db || !m_db->IsOpen())
        return wxEmptyString;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "SELECT value FROM cache_metadata WHERE key = ?"
        );

        stmt.BindText(1, key);

        if (stmt.Step())
        {
            return stmt.GetColumnText(0);
        }

        return wxEmptyString;
    }
    catch (const SqliteException& e)
    {
        printf("Failed to get metadata: %s\n", e.what());
        return wxEmptyString;
    }
}

bool TaxonomyCache::UpdateMetadata(const wxString& key, const wxString& value)
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
        printf("Failed to update metadata: %s\n", e.what());
        return false;
    }
}

bool TaxonomyCache::IsCacheValid(const wxString& zipPath)
{
    wxString cachePath = GetCachePath(zipPath);

    if (!wxFileExists(cachePath))
        return false;

    // Open cache database
    if (!OpenDatabase(cachePath))
        return false;

    // Read stored checksum
    wxString cachedChecksum = GetMetadata("source_checksum");
    if (cachedChecksum.IsEmpty())
        return false;

    // Calculate current checksum
    wxString currentChecksum = CalculateChecksum(zipPath);

    // Compare
    return (cachedChecksum == currentChecksum);
}

bool TaxonomyCache::LoadOrBuild(
    const wxString& zipPath,
    const std::vector<wxString>& requiredLanguages,
    std::function<void(const wxString&, int)> progressCallback)
{
    wxString cachePath = GetCachePath(zipPath);

    // Check if cache exists and is valid
    if (!wxFileExists(cachePath) || !IsCacheValid(zipPath))
    {
        // Build new cache
        CacheBuilder builder;
        if (!builder.BuildCache(zipPath, cachePath, requiredLanguages, progressCallback))
        {
            printf("Failed to build cache\n");
            return false;
        }
    }

    // Open the cache
    if (!OpenDatabase(cachePath))
        return false;

    // Check if required languages are cached
    wxString cachedLangs = GetMetadata("cached_languages");
    wxArrayString cachedLangArray = wxSplit(cachedLangs, ',');

    for (const auto& lang : requiredLanguages)
    {
        bool found = false;
        for (const auto& cached : cachedLangArray)
        {
            if (cached == lang)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Add missing language
            if (!AddLanguage(zipPath, lang, progressCallback))
            {
                printf("Failed to add language: %s\n", lang.utf8_str().data());
            }
        }
    }

    return true;
}

bool TaxonomyCache::LoadTaxonomyMap(std::map<long, TaxonNode>& taxonomyMap)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "SELECT id, parent_id, scientific_name, taxon_rank, "
            "kingdom, phylum, taxon_class, taxon_order, family, genus, "
            "specific_epithet, infraspecific_epithet "
            "FROM taxonomy"
        );

        while (stmt.Step())
        {
            TaxonNode node;
            node.entry.id = stmt.GetColumnInt64(0);
            node.entry.parentId = stmt.GetColumnInt64(1);
            node.entry.scientificName = stmt.GetColumnText(2);
            node.entry.taxonRank = stmt.GetColumnText(3);
            node.entry.kingdom = stmt.GetColumnText(4);
            node.entry.phylum = stmt.GetColumnText(5);
            node.entry.taxonClass = stmt.GetColumnText(6);
            node.entry.order = stmt.GetColumnText(7);
            node.entry.family = stmt.GetColumnText(8);
            node.entry.genus = stmt.GetColumnText(9);
            node.entry.specificEpithet = stmt.GetColumnText(10);
            node.entry.infraspecificEpithet = stmt.GetColumnText(11);

            taxonomyMap[node.entry.id] = node;
        }

        return true;
    }
    catch (const SqliteException& e)
    {
        printf("Failed to load taxonomy map: %s\n", e.what());
        return false;
    }
}

bool TaxonomyCache::LoadVernacularData(const wxString& language, std::vector<VernacularEntry>& entries)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "SELECT taxon_id, language, vernacular_name, country_code "
            "FROM vernacular_names WHERE language = ?"
        );

        stmt.BindText(1, language);

        while (stmt.Step())
        {
            VernacularEntry entry;
            entry.taxonId = stmt.GetColumnInt64(0);
            entry.language = stmt.GetColumnText(1);
            entry.vernacularName = stmt.GetColumnText(2);
            entry.countryCode = stmt.GetColumnText(3);

            entries.push_back(entry);
        }

        return true;
    }
    catch (const SqliteException& e)
    {
        printf("Failed to load vernacular data: %s\n", e.what());
        return false;
    }
}

bool TaxonomyCache::GetTaxonNode(long taxonId, TaxonNode& node)
{
    if (!m_db || !m_db->IsOpen())
        return false;

    try
    {
        SqliteStatement stmt(m_db->Get(),
            "SELECT id, parent_id, scientific_name, taxon_rank, "
            "kingdom, phylum, taxon_class, taxon_order, family, genus, "
            "specific_epithet, infraspecific_epithet "
            "FROM taxonomy WHERE id = ?"
        );

        stmt.BindInt64(1, taxonId);

        if (stmt.Step())
        {
            node.entry.id = stmt.GetColumnInt64(0);
            node.entry.parentId = stmt.GetColumnInt64(1);
            node.entry.scientificName = stmt.GetColumnText(2);
            node.entry.taxonRank = stmt.GetColumnText(3);
            node.entry.kingdom = stmt.GetColumnText(4);
            node.entry.phylum = stmt.GetColumnText(5);
            node.entry.taxonClass = stmt.GetColumnText(6);
            node.entry.order = stmt.GetColumnText(7);
            node.entry.family = stmt.GetColumnText(8);
            node.entry.genus = stmt.GetColumnText(9);
            node.entry.specificEpithet = stmt.GetColumnText(10);
            node.entry.infraspecificEpithet = stmt.GetColumnText(11);

            return true;
        }

        return false;
    }
    catch (const SqliteException& e)
    {
        printf("Failed to get taxon node: %s\n", e.what());
        return false;
    }
}

std::vector<SearchResult> TaxonomyCache::Search(
    const wxString& query,
    const std::vector<wxString>& languages,
    int maxResults)
{
    std::vector<SearchResult> results;

    if (!m_db || !m_db->IsOpen())
        return results;

    if (query.Length() < 3)
        return results;

    try
    {
        // Build FTS5 prefix query
        wxString ftsQuery = query + "*";

        // Build language filter using filename-based language names
        wxString langFilter;
        if (!languages.empty())
        {
            wxArrayString quotedLangs;
            for (const auto& lang : languages)
            {
                quotedLangs.Add("'" + lang + "'");
            }
            langFilter = wxString::Format(
                " AND (s.language IN (%s) OR s.name_type = 'scientific')",
                wxJoin(quotedLangs, ',')
            );
        }

        // Query with BM25 ranking
        // Note: FTS5 MATCH and bm25() require the actual table name, not alias
        wxString sql = wxString::Format(
            "SELECT s.taxon_id, s.name, s.name_type, s.language, s.taxon_rank, "
            "       t.scientific_name, bm25(search_index) as rank "
            "FROM search_index s "
            "JOIN taxonomy t ON s.taxon_id = t.id "
            "WHERE search_index MATCH ? %s "
            "ORDER BY bm25(search_index) "
            "LIMIT ?",
            langFilter
        );

        SqliteStatement stmt(m_db->Get(), sql);
        stmt.BindText(1, ftsQuery);
        stmt.BindInt64(2, maxResults);

        std::set<long> seenTaxonIds;  // Deduplication

        while (stmt.Step())
        {
            long taxonId = stmt.GetColumnInt64(0);

            // Skip duplicates
            if (seenTaxonIds.count(taxonId))
                continue;
            seenTaxonIds.insert(taxonId);

            SearchResult result;
            result.taxonId = taxonId;
            result.matchedTerm = stmt.GetColumnText(1);
            wxString nameType = stmt.GetColumnText(2);
            result.taxonRank = stmt.GetColumnText(4);
            result.scientificName = stmt.GetColumnText(5);

            // Determine display name
            result.displayName = (nameType == "vernacular")
                ? result.matchedTerm
                : result.scientificName;

            results.push_back(result);
        }

        int vernacularCount = 0, scientificCount = 0;
        for (const auto& r : results)
        {
            if (r.displayName == r.scientificName)
                scientificCount++;
            else
                vernacularCount++;
        }

        // Apply custom rank priority sorting
        std::sort(results.begin(), results.end());

        return results;
    }
    catch (const SqliteException& e)
    {
        printf("Failed to search: %s\n", e.what());
        return results;
    }
}

std::vector<wxString> TaxonomyCache::GetCachedLanguages()
{
    std::vector<wxString> languages;

    wxString cachedLangs = GetMetadata("cached_languages");
    if (!cachedLangs.IsEmpty())
    {
        wxArrayString langArray = wxSplit(cachedLangs, ',');
        for (const auto& lang : langArray)
        {
            languages.push_back(lang);
        }
    }

    return languages;
}

std::vector<wxString> TaxonomyCache::GetAvailableLanguages()
{
    std::vector<wxString> languages;

    wxString availLangs = GetMetadata("available_languages");
    if (!availLangs.IsEmpty())
    {
        wxArrayString langArray = wxSplit(availLangs, ',');
        for (const auto& lang : langArray)
        {
            languages.push_back(lang);
        }
    }

    return languages;
}

bool TaxonomyCache::AddLanguage(
    const wxString& zipPath,
    const wxString& language,
    std::function<void(const wxString&, int)> progressCallback)
{
    // Save cache path before closing (CloseDatabase clears m_cachePath!)
    wxString cachePath = m_cachePath;
    CloseDatabase();

    CacheBuilder builder;
    bool result = builder.AddLanguageToCache(zipPath, cachePath, language, progressCallback);

    // Reopen the cache after adding language
    if (result)
        OpenDatabase(cachePath);

    return result;
}
