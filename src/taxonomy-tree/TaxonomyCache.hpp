#pragma once

#include <wx/wx.h>
#include <sqlite3.h>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include "CSVParser.hpp"
#include "SearchIndex.hpp"
#include "SqliteUtils.hpp"

// Forward declarations
class ZipReader;
struct TaxonNode;

class TaxonomyCache
{
public:
    TaxonomyCache();
    ~TaxonomyCache();

    // Validate and load cache, rebuild if necessary
    bool LoadOrBuild(
        const wxString& zipPath,
        const std::vector<wxString>& requiredLanguages,
        std::function<void(const wxString&, int)> progressCallback = nullptr
    );

    // Check if cache is valid (checksum matches)
    bool IsCacheValid(const wxString& zipPath);

    // Load taxonomy data into memory map
    bool LoadTaxonomyMap(std::map<long, TaxonNode>& taxonomyMap);

    // Load vernacular data for specific language
    bool LoadVernacularData(const wxString& language, std::vector<VernacularEntry>& entries);

    // Get taxon by ID (direct query, no full load needed)
    bool GetTaxonNode(long taxonId, TaxonNode& node);

    // Search using FTS5 (KEY PERFORMANCE WIN)
    std::vector<SearchResult> Search(
        const wxString& query,
        const std::vector<wxString>& languages,
        int maxResults = 100
    );

    // Get available languages in cache
    std::vector<wxString> GetCachedLanguages();

    // Get all available languages (from ZIP file)
    std::vector<wxString> GetAvailableLanguages();

    // Add language to existing cache
    bool AddLanguage(
        const wxString& zipPath,
        const wxString& language,
        std::function<void(const wxString&, int)> progressCallback = nullptr
    );

    // Get metadata value
    wxString GetMetadata(const wxString& key);

    // Check if database is open
    bool IsOpen() const;

private:
    // Open/close database
    bool OpenDatabase(const wxString& path);
    void CloseDatabase();

    // Update metadata
    bool UpdateMetadata(const wxString& key, const wxString& value);

    // Calculate checksum of file
    wxString CalculateChecksum(const wxString& path);

    // Get cache path from ZIP path
    wxString GetCachePath(const wxString& zipPath);

    std::unique_ptr<SqliteDatabase> m_db;
    wxString m_cachePath;
};
