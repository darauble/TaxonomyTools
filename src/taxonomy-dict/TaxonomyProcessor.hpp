#pragma once

#include <wx/wx.h>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include "CSVParser.hpp"

class ZipReader;
class DictionaryGenerator;

struct DictionaryEntry
{
    wxString sourceWord;
    std::vector<wxString> translations;
};

class TaxonomyProcessor
{
public:
    TaxonomyProcessor();
    ~TaxonomyProcessor();

    bool LoadArchive(const wxString& archivePath);
    bool LoadArchive(const wxString& archivePath, std::function<void(const wxString&, int)> progressCallback);
    std::vector<wxString> GetAvailableLanguages() const;
    
    bool GenerateDictionary(const wxString& sourceLanguage, 
                          const std::vector<wxString>& targetLanguages,
                          bool includeScientific,
                          const wxString& outputPath);
                          
    bool GenerateDictionary(const wxString& sourceLanguage, 
                          const std::vector<wxString>& targetLanguages,
                          bool includeScientific,
                          bool filterOnlyTargets,
                          const wxString& outputPath,
                          std::function<void(const wxString&, int)> progressCallback);

private:
    bool LoadTaxonomyData();
    bool LoadTaxonomyData(std::function<void(const wxString&, int)> progressCallback);
    bool LoadVernacularData(const wxString& language, std::vector<VernacularEntry>& entries);
    bool LoadScientificNameEntries(std::vector<VernacularEntry>& entries);
    
    std::map<wxString, std::vector<wxString>> BuildDictionary(
        const wxString& sourceLanguage,
        const std::vector<wxString>& targetLanguages,
        bool includeScientific);
        
    std::map<wxString, std::vector<wxString>> BuildDictionary(
        const wxString& sourceLanguage,
        const std::vector<wxString>& targetLanguages,
        bool includeScientific,
        bool filterOnlyTargets,
        std::function<void(const wxString&, int)> progressCallback);

    std::unique_ptr<ZipReader> m_zipReader;
    std::unique_ptr<DictionaryGenerator> m_dictGenerator;
    
    std::map<long, TaxonomyEntry> m_taxonomyMap;
    std::vector<wxString> m_availableLanguages;
    
    // Cache for vernacular entries by language
    std::map<wxString, std::vector<VernacularEntry>> m_vernacularCache;
};