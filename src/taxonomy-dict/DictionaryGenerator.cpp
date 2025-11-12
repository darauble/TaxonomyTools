#include "DictionaryGenerator.hpp"
#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <wx/filename.h>
#include <algorithm>
#include <cstdint>

DictionaryGenerator::DictionaryGenerator()
{
}

bool DictionaryGenerator::GenerateTSV(const std::map<wxString, std::vector<wxString>>& dictionary, 
                                     const wxString& outputPath)
{
    wxTextFile file;
    
    // If file exists, open it and clear it; otherwise create it
    if (wxFileExists(outputPath))
    {
        if (!file.Open(outputPath))
            return false;
        file.Clear();
    }
    else
    {
        if (!file.Create(outputPath))
            return false;
    }
    
    for (const auto& entry : dictionary)
    {
        if (entry.second.empty())
            continue;
            
        wxString word = EscapeTabSeparated(entry.first);
        wxString definition = EscapeTabSeparated(FormatDefinition(entry.second));
        
        wxString line = word + wxT("\t") + definition;
        file.AddLine(line);
    }
    
    return file.Write();
}

bool DictionaryGenerator::GenerateStarDict(const std::map<wxString, std::vector<wxString>>& dictionary,
                                          const wxString& outputDir,
                                          const wxString& baseName)
{
    if (dictionary.empty())
        return false;
    
    // Generate .idx and collect index data first
    std::vector<std::pair<wxString, size_t>> indexData;
    if (!GenerateStarDictIdx(dictionary, outputDir, baseName, indexData))
        return false;
        
    // Get actual IDX file size
    wxString idxPath = outputDir + "/" + baseName + ".idx";
    wxFileName idxFile(idxPath);
    size_t idxFileSize = idxFile.GetSize().ToULong();
    
    // Generate .ifo file with correct IDX size
    if (!GenerateStarDictIfo(outputDir, baseName, dictionary.size(), idxFileSize))
        return false;
    
    // Generate .dict file
    if (!GenerateStarDictDict(dictionary, outputDir, baseName, indexData))
        return false;
    
    return true;
}

bool DictionaryGenerator::GenerateStarDict(const std::map<wxString, std::vector<wxString>>& dictionary,
                                          const wxString& outputDir,
                                          const wxString& baseName,
                                          const wxString& sourceLanguage,
                                          const std::vector<wxString>& targetLanguages)
{
    if (dictionary.empty())
        return false;
    
    // Generate .idx and collect index data first
    std::vector<std::pair<wxString, size_t>> indexData;
    if (!GenerateStarDictIdx(dictionary, outputDir, baseName, indexData))
        return false;
        
    // Get actual IDX file size
    wxString idxPath = outputDir + "/" + baseName + ".idx";
    wxFileName idxFile(idxPath);
    size_t idxFileSize = idxFile.GetSize().ToULong();
    
    // Generate .ifo file with language information and correct IDX size
    if (!GenerateStarDictIfo(outputDir, baseName, dictionary.size(), idxFileSize, sourceLanguage, targetLanguages))
        return false;
    
    // Generate .dict file
    if (!GenerateStarDictDict(dictionary, outputDir, baseName, indexData))
        return false;
    
    return true;
}

bool DictionaryGenerator::GenerateStarDictIfo(const wxString& outputDir, const wxString& baseName, size_t wordCount, size_t idxFileSize)
{
    wxString ifoPath = outputDir + "/" + baseName + ".ifo";
    wxTextFile file;
    
    // If file exists, open it and clear it; otherwise create it
    if (wxFileExists(ifoPath))
    {
        if (!file.Open(ifoPath))
            return false;
        file.Clear();
    }
    else
    {
        if (!file.Create(ifoPath))
            return false;
    }
    
    file.AddLine("StarDict's dict ifo file");
    file.AddLine("version=2.4.2");
    file.AddLine("wordcount=" + wxString::Format("%zu", wordCount));
    file.AddLine("idxfilesize=" + wxString::Format("%zu", idxFileSize));
    file.AddLine("bookname=Taxonomy Dictionary");
    file.AddLine("author=TaxonomyDict");
    file.AddLine("email=");
    file.AddLine("website=");
    file.AddLine("description=Generated from iNaturalist taxonomy data");
    file.AddLine("date=" + wxDateTime::Now().Format("%Y.%m.%d"));
    file.AddLine("sametypesequence=h");
    
    return file.Write();
}

bool DictionaryGenerator::GenerateStarDictIfo(const wxString& outputDir, const wxString& baseName, size_t wordCount, size_t idxFileSize,
                                              const wxString& sourceLanguage, const std::vector<wxString>& targetLanguages)
{
    wxString ifoPath = outputDir + "/" + baseName + ".ifo";
    wxTextFile file;
    
    // If file exists, open it and clear it; otherwise create it
    if (wxFileExists(ifoPath))
    {
        if (!file.Open(ifoPath))
            return false;
        file.Clear();
    }
    else
    {
        if (!file.Create(ifoPath))
            return false;
    }
    
    // Create book name with source and target languages
    wxString bookName = "Taxonomy Dictionary (" + sourceLanguage + " -> ";
    if (targetLanguages.empty())
    {
        bookName += "Scientific Names";
    }
    else if (targetLanguages.size() == 1)
    {
        bookName += targetLanguages[0];
    }
    else
    {
        for (size_t i = 0; i < targetLanguages.size(); ++i)
        {
            if (i > 0)
                bookName += ", ";
            bookName += targetLanguages[i];
        }
    }
    bookName += ")";
    
    file.AddLine("StarDict's dict ifo file");
    file.AddLine("version=2.4.2");
    file.AddLine("wordcount=" + wxString::Format("%zu", wordCount));
    file.AddLine("idxfilesize=" + wxString::Format("%zu", idxFileSize));
    file.AddLine("bookname=" + bookName);
    file.AddLine("author=TaxonomyDict");
    file.AddLine("email=darau.ble@gmail.com");
    file.AddLine("website=https://darauble.wordpress.com");
    file.AddLine("description=Generated from iNaturalist taxonomy data");
    file.AddLine("date=" + wxDateTime::Now().Format("%Y.%m.%d"));
    file.AddLine("sametypesequence=h");
    
    return file.Write();
}

bool DictionaryGenerator::GenerateStarDictIdx(const std::map<wxString, std::vector<wxString>>& dictionary,
                                             const wxString& outputDir, const wxString& baseName,
                                             std::vector<std::pair<wxString, size_t>>& indexData)
{
    wxString idxPath = outputDir + "/" + baseName + ".idx";
    
    // Delete existing file to ensure clean overwrite
    if (wxFileExists(idxPath))
    {
        wxRemoveFile(idxPath);
    }
    
    wxFileOutputStream stream(idxPath);
    
    if (!stream.IsOk())
        return false;
    
    // Create sorted list of words for index
    std::vector<std::pair<wxString, std::vector<wxString>>> sortedWords;
    for (const auto& entry : dictionary)
    {
        if (!entry.second.empty())
        {
            sortedWords.push_back(entry);
        }
    }
    
    // Sort by word
    std::sort(sortedWords.begin(), sortedWords.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });
    
    size_t currentOffset = 0;
    
    for (const auto& entry : sortedWords)
    {
        // Write word (null-terminated)
        wxString word = entry.first;
        const char* wordBytes = word.mb_str(wxConvUTF8);
        stream.Write(wordBytes, strlen(wordBytes) + 1); // +1 for null terminator
        
        // Write offset (4 bytes, big-endian)
        uint32_t offset = wxUINT32_SWAP_ON_LE(static_cast<uint32_t>(currentOffset));
        stream.Write(&offset, 4);
        
        // Write size (4 bytes, big-endian)
        wxString definition = FormatDefinition(entry.second);
        const char* defBytes = definition.mb_str(wxConvUTF8);
        size_t byteSize = strlen(defBytes);
        uint32_t size = wxUINT32_SWAP_ON_LE(static_cast<uint32_t>(byteSize));
        stream.Write(&size, 4);
        
        indexData.push_back(std::make_pair(entry.first, byteSize));
        currentOffset += byteSize;
    }
    
    stream.Close();
    return stream.GetLastError() == wxSTREAM_NO_ERROR;
}

bool DictionaryGenerator::GenerateStarDictDict(const std::map<wxString, std::vector<wxString>>& dictionary,
                                              const wxString& outputDir, const wxString& baseName,
                                              const std::vector<std::pair<wxString, size_t>>& indexData)
{
    wxString dictPath = outputDir + "/" + baseName + ".dict";
    
    // Delete existing file to ensure clean overwrite
    if (wxFileExists(dictPath))
    {
        wxRemoveFile(dictPath);
    }
    
    wxFileOutputStream stream(dictPath);
    
    if (!stream.IsOk())
        return false;
    
    // Write definitions in the same order as the index
    for (const auto& indexEntry : indexData)
    {
        auto dictIt = dictionary.find(indexEntry.first);
        if (dictIt != dictionary.end())
        {
            wxString definition = FormatDefinition(dictIt->second);
            const char* defBytes = definition.mb_str(wxConvUTF8);
            stream.Write(defBytes, strlen(defBytes));
        }
    }
    
    stream.Close();
    return stream.GetLastError() == wxSTREAM_NO_ERROR;
}

wxString DictionaryGenerator::EscapeTabSeparated(const wxString& text)
{
    wxString escaped = text;
    escaped.Replace(wxT("\t"), wxT("\\t"));
    escaped.Replace(wxT("\n"), wxT("\\n"));
    escaped.Replace(wxT("\r"), wxT("\\r"));
    return escaped;
}

wxString DictionaryGenerator::FormatDefinition(const std::vector<wxString>& translations)
{
    if (translations.empty())
        return wxEmptyString;
    
    if (translations.size() == 1)
        return FormatTranslationWithLanguage(translations[0]);
    
    wxString result;
    std::vector<wxString> normalTranslations;
    std::vector<wxString> urlTranslations;
    
    // Separate URLs from normal translations
    for (const auto& translation : translations)
    {
        if (translation.StartsWith("http://") || translation.StartsWith("https://") || 
            translation.Contains("www.") || translation.Contains(".org") || translation.Contains(".com"))
        {
            urlTranslations.push_back(translation);
        }
        else
        {
            normalTranslations.push_back(translation);
        }
    }
    
    // Add normal translations first
    for (size_t i = 0; i < normalTranslations.size(); ++i)
    {
        if (i > 0)
            result += wxT("<br>");
        result += FormatTranslationWithLanguage(normalTranslations[i]);
    }
    
    // Add URLs at the end with additional spacing
    for (const auto& url : urlTranslations)
    {
        if (!result.IsEmpty())
            result += wxT("<br><br>");
        result += url;
    }
    
    return result;
}

wxString DictionaryGenerator::FormatTranslationWithLanguage(const wxString& translation)
{
    // Check if translation already contains language information in the format "translation|language"
    int pipePos = translation.Find('|');
    if (pipePos != wxNOT_FOUND)
    {
        wxString text = translation.Left(pipePos);
        wxString language = translation.Mid(pipePos + 1);
        
        if (!language.IsEmpty())
        {
            return text + wxT(" <i>(") + language + wxT(")</i>");
        }
    }
    
    // Return the translation as-is if no language information is found
    return translation;
}