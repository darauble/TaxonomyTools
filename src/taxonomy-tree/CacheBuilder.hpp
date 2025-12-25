#pragma once

#include <wx/wx.h>
#include <sqlite3.h>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include "CSVParser.hpp"
#include "SqliteUtils.hpp"

// Forward declarations
class ZipReader;

class CacheBuilder
{
public:
    struct BuildStats
    {
        size_t taxonomyEntries = 0;
        size_t vernacularEntries = 0;
        size_t searchIndexEntries = 0;
        double buildTimeSeconds = 0.0;
    };

    CacheBuilder();
    ~CacheBuilder();

    // Build cache from ZIP archive
    bool BuildCache(
        const wxString& zipPath,
        const wxString& cachePath,
        const std::vector<wxString>& languages,
        std::function<void(const wxString&, int)> progressCallback = nullptr
    );

    // Add language to existing cache
    bool AddLanguageToCache(
        const wxString& zipPath,
        const wxString& cachePath,
        const wxString& language,
        std::function<void(const wxString&, int)> progressCallback = nullptr
    );

    BuildStats GetLastBuildStats() const { return m_lastStats; }

private:
    // Database operations
    bool OpenDatabase(const wxString& path);
    void CloseDatabase();

    // Schema creation
    bool CreateSchema();

    // Data insertion
    bool InsertTaxonomyData(const std::map<long, TaxonomyEntry>& data,
                           std::function<void(const wxString&, int)> progressCallback);
    bool InsertVernacularData(const std::vector<VernacularEntry>& data,
                             const wxString& language,
                             std::function<void(const wxString&, int)> progressCallback);

    // Index building
    bool BuildSearchIndex(std::function<void(const wxString&, int)> progressCallback);

    // Metadata
    bool UpdateMetadata(const wxString& key, const wxString& value);
    wxString CalculateChecksum(const wxString& path);

    std::unique_ptr<SqliteDatabase> m_db;
    BuildStats m_lastStats;
};
