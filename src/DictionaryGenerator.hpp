#pragma once

#include <wx/wx.h>
#include <map>
#include <vector>

class DictionaryGenerator
{
public:
    DictionaryGenerator();
    
    bool GenerateTSV(const std::map<wxString, std::vector<wxString>>& dictionary, 
                     const wxString& outputPath);
    
    bool GenerateStarDict(const std::map<wxString, std::vector<wxString>>& dictionary,
                         const wxString& outputDir,
                         const wxString& baseName);
                         
    bool GenerateStarDict(const std::map<wxString, std::vector<wxString>>& dictionary,
                         const wxString& outputDir,
                         const wxString& baseName,
                         const wxString& sourceLanguage,
                         const std::vector<wxString>& targetLanguages);

private:
    bool GenerateStarDictIfo(const wxString& outputDir, const wxString& baseName, size_t wordCount, size_t idxFileSize);
    bool GenerateStarDictIfo(const wxString& outputDir, const wxString& baseName, size_t wordCount, size_t idxFileSize,
                            const wxString& sourceLanguage, const std::vector<wxString>& targetLanguages);
    bool GenerateStarDictIdx(const std::map<wxString, std::vector<wxString>>& dictionary,
                            const wxString& outputDir, const wxString& baseName,
                            std::vector<std::pair<wxString, size_t>>& indexData);
    bool GenerateStarDictDict(const std::map<wxString, std::vector<wxString>>& dictionary,
                             const wxString& outputDir, const wxString& baseName,
                             const std::vector<std::pair<wxString, size_t>>& indexData);
    
    wxString EscapeTabSeparated(const wxString& text);
    wxString FormatDefinition(const std::vector<wxString>& translations);
    wxString FormatTranslationWithLanguage(const wxString& translation);
};