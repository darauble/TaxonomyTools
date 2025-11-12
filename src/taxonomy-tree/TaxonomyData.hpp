#pragma once

#include <wx/wx.h>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include "CSVParser.hpp"

class ZipReader;

// Extended taxonomy entry with hierarchy chain
struct TaxonNode
{
    TaxonomyEntry entry;
    std::vector<long> children;  // IDs of child taxa

    // Get hierarchy chain from this node up to kingdom
    std::vector<long> GetHierarchyChain(const std::map<long, TaxonNode>& taxonomyMap) const;

    // Get display name based on rank
    wxString GetDisplayName() const;
};

class TaxonomyData
{
public:
    TaxonomyData();
    ~TaxonomyData();

    // Load taxonomy data from zip archive
    bool LoadArchive(const wxString& archivePath, std::function<void(const wxString&, int)> progressCallback = nullptr);

    // Get available languages from the archive
    std::vector<wxString> GetAvailableLanguages() const;

    // Load vernacular names for a specific language
    bool LoadVernacularData(const wxString& language, std::vector<VernacularEntry>& entries);

    // Get taxon node by ID
    const TaxonNode* GetTaxonNode(long taxonId) const;

    // Find common ancestor of multiple taxa
    long FindCommonAncestor(const std::vector<long>& taxonIds) const;

    // Get hierarchy chain from taxon to root (kingdom level)
    std::vector<long> GetHierarchyChain(long taxonId) const;

    // Get all taxonomy entries (for indexing)
    const std::map<long, TaxonNode>& GetAllTaxa() const { return m_taxonomyMap; }

private:
    bool LoadTaxonomyData(std::function<void(const wxString&, int)> progressCallback);
    void BuildHierarchy();

    std::unique_ptr<ZipReader> m_zipReader;
    std::map<long, TaxonNode> m_taxonomyMap;
    std::vector<wxString> m_availableLanguages;

    // Cache for vernacular entries by language
    std::map<wxString, std::vector<VernacularEntry>> m_vernacularCache;
};
