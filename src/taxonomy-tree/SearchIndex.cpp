#include "SearchIndex.hpp"
#include "TaxonomyCache.hpp"
#include <wx/tokenzr.h>
#include <algorithm>

SearchIndex::SearchIndex()
    : m_cache(nullptr)
{
}

void SearchIndex::BuildIndex(const TaxonomyData& taxonomyData,
                             const wxString& primaryLanguage,
                             const wxString& secondaryLanguage)
{
    m_primaryLanguage = primaryLanguage;
    m_secondaryLanguage = secondaryLanguage;

    // Clear existing index
    m_wordIndex.clear();
    m_primaryNames.clear();
    m_secondaryNames.clear();
    m_scientificNames.clear();

    // Load vernacular names for both languages
    std::vector<VernacularEntry> primaryEntries, secondaryEntries;
    const_cast<TaxonomyData&>(taxonomyData).LoadVernacularData(primaryLanguage, primaryEntries);
    if (!secondaryLanguage.IsEmpty())
        const_cast<TaxonomyData&>(taxonomyData).LoadVernacularData(secondaryLanguage, secondaryEntries);

    // Index primary language names
    for (const auto& entry : primaryEntries)
    {
        m_primaryNames[entry.taxonId].push_back(entry.vernacularName);
        IndexWords(entry.vernacularName, entry.taxonId);
    }

    // Index secondary language names
    for (const auto& entry : secondaryEntries)
    {
        m_secondaryNames[entry.taxonId].push_back(entry.vernacularName);
        IndexWords(entry.vernacularName, entry.taxonId);
    }

    // Index scientific names
    const auto& allTaxa = taxonomyData.GetAllTaxa();
    for (const auto& pair : allTaxa)
    {
        const TaxonNode& node = pair.second;
        if (!node.entry.scientificName.IsEmpty())
        {
            m_scientificNames[node.entry.id] = std::make_pair(node.entry.scientificName, node.entry.taxonRank);
            IndexWords(node.entry.scientificName, node.entry.id);
        }
    }
}

void SearchIndex::IndexWords(const wxString& text, long taxonId)
{
    wxStringTokenizer tokenizer(text, " \t\r\n-()[]", wxTOKEN_STRTOK);
    while (tokenizer.HasMoreTokens())
    {
        wxString word = NormalizeWord(tokenizer.GetNextToken());
        if (!word.IsEmpty() && word.length() >= 2)  // Ignore single-letter words
        {
            m_wordIndex[word].insert(taxonId);
        }
    }
}

wxString SearchIndex::NormalizeWord(const wxString& word) const
{
    return word.Lower().Trim(true).Trim(false);
}

std::vector<SearchResult> SearchIndex::Search(const wxString& query) const
{
    if (query.IsEmpty())
        return std::vector<SearchResult>();

    wxString normalizedQuery = query.Lower().Trim(true).Trim(false);
    if (normalizedQuery.length() < 3)
        return std::vector<SearchResult>();

    std::vector<SearchResult> results;
    std::set<long> addedTaxonIds;  // To avoid duplicates

    // Helper function to check if a name matches the query (phrase matching)
    auto matchesQuery = [&normalizedQuery](const wxString& name) -> bool {
        wxString normalizedName = name.Lower();
        // Check if the normalized name starts with the query (left-to-right matching)
        return normalizedName.StartsWith(normalizedQuery) ||
               normalizedName.Find(" " + normalizedQuery) != wxNOT_FOUND;
    };

    // Search in primary language names first
    for (const auto& pair : m_primaryNames)
    {
        long taxonId = pair.first;
        const std::vector<wxString>& names = pair.second;

        for (const wxString& name : names)
        {
            if (matchesQuery(name))
            {
                if (addedTaxonIds.find(taxonId) == addedTaxonIds.end())
                {
                    SearchResult result;
                    result.taxonId = taxonId;
                    result.displayName = name;
                    result.matchedTerm = name;

                    auto sciIt = m_scientificNames.find(taxonId);
                    if (sciIt != m_scientificNames.end())
                    {
                        result.scientificName = sciIt->second.first;
                        result.taxonRank = sciIt->second.second;
                    }

                    results.push_back(result);
                    addedTaxonIds.insert(taxonId);
                }
                break;  // Only add once per taxon
            }
        }
    }

    // Search in secondary language names
    for (const auto& pair : m_secondaryNames)
    {
        long taxonId = pair.first;
        const std::vector<wxString>& names = pair.second;

        for (const wxString& name : names)
        {
            if (matchesQuery(name))
            {
                if (addedTaxonIds.find(taxonId) == addedTaxonIds.end())
                {
                    SearchResult result;
                    result.taxonId = taxonId;
                    result.displayName = name;
                    result.matchedTerm = name;

                    auto sciIt = m_scientificNames.find(taxonId);
                    if (sciIt != m_scientificNames.end())
                    {
                        result.scientificName = sciIt->second.first;
                        result.taxonRank = sciIt->second.second;
                    }

                    results.push_back(result);
                    addedTaxonIds.insert(taxonId);
                }
                break;  // Only add once per taxon
            }
        }
    }

    // Search in scientific names
    for (const auto& pair : m_scientificNames)
    {
        long taxonId = pair.first;
        const wxString& scientificName = pair.second.first;
        const wxString& taxonRank = pair.second.second;

        if (matchesQuery(scientificName))
        {
            if (addedTaxonIds.find(taxonId) == addedTaxonIds.end())
            {
                SearchResult result;
                result.taxonId = taxonId;
                result.displayName = GetDisplayName(taxonId, false);
                result.scientificName = scientificName;
                result.taxonRank = taxonRank;
                result.matchedTerm = scientificName;

                results.push_back(result);
                addedTaxonIds.insert(taxonId);
            }
        }
    }

    // Sort results by rank priority
    std::sort(results.begin(), results.end());

    return results;
}

wxString SearchIndex::GetDisplayName(long taxonId, bool includeScientific) const
{
    wxString displayName;

    // Try primary language first
    auto primaryIt = m_primaryNames.find(taxonId);
    if (primaryIt != m_primaryNames.end() && !primaryIt->second.empty())
    {
        displayName = primaryIt->second[0];  // Use first vernacular name
    }
    else
    {
        // Fallback to secondary language
        auto secondaryIt = m_secondaryNames.find(taxonId);
        if (secondaryIt != m_secondaryNames.end() && !secondaryIt->second.empty())
        {
            displayName = secondaryIt->second[0];
        }
    }

    // Always include scientific name if requested
    if (includeScientific)
    {
        auto sciIt = m_scientificNames.find(taxonId);
        if (sciIt != m_scientificNames.end())
        {
            if (!displayName.IsEmpty())
                displayName += wxString::Format(" (%s)", sciIt->second.first);
            else
                displayName = sciIt->second.first;
        }
    }

    // If still empty, use scientific name alone
    if (displayName.IsEmpty())
    {
        auto sciIt = m_scientificNames.find(taxonId);
        if (sciIt != m_scientificNames.end())
            displayName = sciIt->second.first;
        else
            displayName = wxString::Format("Taxon #%ld", taxonId);
    }

    return displayName;
}

wxString SearchIndex::GetVernacularName(long taxonId) const
{
    // Try primary language first
    auto primaryIt = m_primaryNames.find(taxonId);
    if (primaryIt != m_primaryNames.end() && !primaryIt->second.empty())
    {
        return primaryIt->second[0];  // Use first vernacular name
    }

    // Fallback to secondary language
    auto secondaryIt = m_secondaryNames.find(taxonId);
    if (secondaryIt != m_secondaryNames.end() && !secondaryIt->second.empty())
    {
        return secondaryIt->second[0];
    }

    return wxEmptyString;
}

wxString SearchIndex::GetScientificName(long taxonId) const
{
    auto sciIt = m_scientificNames.find(taxonId);
    if (sciIt != m_scientificNames.end())
    {
        return sciIt->second.first;
    }

    return wxEmptyString;
}

void SearchIndex::BuildFromCache(
    const TaxonomyData& taxonomyData,
    TaxonomyCache* cache,
    const wxString& primaryLanguage,
    const wxString& secondaryLanguage)
{
    m_cache = cache;
    m_primaryLanguage = primaryLanguage;
    m_secondaryLanguage = secondaryLanguage;

    // Clear existing index
    m_wordIndex.clear();
    m_primaryNames.clear();
    m_secondaryNames.clear();
    m_scientificNames.clear();

    if (!cache || !cache->IsOpen())
        return;

    // Load vernacular names into memory (for GetDisplayName, etc.)
    std::vector<VernacularEntry> primaryEntries, secondaryEntries;
    if (!primaryLanguage.IsEmpty())
    {
        cache->LoadVernacularData(primaryLanguage, primaryEntries);
        printf("Loaded %zu primary vernacular entries for '%s'\n", primaryEntries.size(), primaryLanguage.utf8_str().data());
    }

    if (!secondaryLanguage.IsEmpty())
    {
        cache->LoadVernacularData(secondaryLanguage, secondaryEntries);
        printf("Loaded %zu secondary vernacular entries for '%s'\n", secondaryEntries.size(), secondaryLanguage.utf8_str().data());
    }

    // Populate name maps (same as before)
    for (const auto& entry : primaryEntries)
    {
        m_primaryNames[entry.taxonId].push_back(entry.vernacularName);
    }

    for (const auto& entry : secondaryEntries)
    {
        m_secondaryNames[entry.taxonId].push_back(entry.vernacularName);
    }

    // Load scientific names from taxonomy data (for GetDisplayName, tree rendering, etc.)
    const auto& allTaxa = taxonomyData.GetAllTaxa();
    for (const auto& pair : allTaxa)
    {
        const TaxonNode& node = pair.second;
        if (!node.entry.scientificName.IsEmpty())
        {
            m_scientificNames[node.entry.id] = std::make_pair(node.entry.scientificName, node.entry.taxonRank);
        }
    }

    // NOTE: We don't build m_wordIndex in memory!
    // Search will use FTS5 directly via SearchWithCache()
}

std::vector<SearchResult> SearchIndex::SearchWithCache(
    const wxString& query,
    TaxonomyCache* cache) const
{
    if (!cache || !cache->IsOpen())
    {
        // Fallback to old search method
        return Search(query);
    }

    // Use FTS5-powered search (50-100x faster!)
    std::vector<wxString> languages;
    if (!m_primaryLanguage.IsEmpty())
        languages.push_back(m_primaryLanguage);
    if (!m_secondaryLanguage.IsEmpty())
        languages.push_back(m_secondaryLanguage);

    return cache->Search(query, languages);
}
