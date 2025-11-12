#pragma once

#include <wx/wx.h>
#include <map>
#include <vector>
#include <set>
#include "CSVParser.hpp"
#include "TaxonomyData.hpp"

// Search result entry
struct SearchResult
{
    long taxonId;
    wxString displayName;
    wxString scientificName;
    wxString taxonRank;
    wxString matchedTerm;  // The term that was matched

    bool operator<(const SearchResult& other) const
    {
        // Sort by rank priority (subspecies > species > genus > family > order > class > phylum > kingdom)
        static std::map<wxString, int> rankPriority = {
            {"subspecies", 1}, {"variety", 1}, {"form", 1},
            {"species", 2},
            {"genus", 3},
            {"family", 4},
            {"order", 5},
            {"class", 6},
            {"phylum", 7},
            {"kingdom", 8}
        };

        int thisPriority = 99;
        int otherPriority = 99;

        auto thisIt = rankPriority.find(taxonRank.Lower());
        if (thisIt != rankPriority.end())
            thisPriority = thisIt->second;

        auto otherIt = rankPriority.find(other.taxonRank.Lower());
        if (otherIt != rankPriority.end())
            otherPriority = otherIt->second;

        if (thisPriority != otherPriority)
            return thisPriority < otherPriority;

        // If same priority, sort alphabetically
        return displayName.CmpNoCase(other.displayName) < 0;
    }
};

class SearchIndex
{
public:
    SearchIndex();

    // Build index from taxonomy data and vernacular names
    void BuildIndex(const TaxonomyData& taxonomyData,
                   const wxString& primaryLanguage,
                   const wxString& secondaryLanguage);

    // Search for taxa by term (case-insensitive, word-based)
    std::vector<SearchResult> Search(const wxString& query) const;

    // Get display name for a taxon (with primary/secondary language fallback)
    wxString GetDisplayName(long taxonId, bool includeScientific = true) const;

    // Get vernacular name for a taxon (returns first primary or secondary language name)
    wxString GetVernacularName(long taxonId) const;

    // Get scientific name for a taxon
    wxString GetScientificName(long taxonId) const;

private:
    // Word -> set of taxon IDs
    std::map<wxString, std::set<long>> m_wordIndex;

    // Taxon ID -> vernacular names in primary language
    std::map<long, std::vector<wxString>> m_primaryNames;

    // Taxon ID -> vernacular names in secondary language
    std::map<long, std::vector<wxString>> m_secondaryNames;

    // Taxon ID -> scientific name and rank
    std::map<long, std::pair<wxString, wxString>> m_scientificNames;

    wxString m_primaryLanguage;
    wxString m_secondaryLanguage;

    // Helper: add words from a name to the index
    void IndexWords(const wxString& text, long taxonId);

    // Helper: normalize word for indexing (lowercase, trim)
    wxString NormalizeWord(const wxString& word) const;
};
