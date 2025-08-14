#include "TaxonomyProcessor.hpp"
#include "ZipReader.hpp"
#include "DictionaryGenerator.hpp"
#include <wx/filename.h>
#include <iostream>

TaxonomyProcessor::TaxonomyProcessor()
    : m_zipReader(std::make_unique<ZipReader>())
    , m_dictGenerator(std::make_unique<DictionaryGenerator>())
{
}

TaxonomyProcessor::~TaxonomyProcessor() = default;

bool TaxonomyProcessor::LoadArchive(const wxString& archivePath)
{
    printf("LoadArchive: Opening archive: %s\n", archivePath.ToStdString().c_str());
    
    if (!m_zipReader->OpenArchive(archivePath))
    {
        printf("LoadArchive: Failed to open archive\n");
        return false;
    }
    
    // Debug: List all files in archive
    auto fileList = m_zipReader->GetFileList();
    printf("LoadArchive: Archive contains %zu files:\n", fileList.size());
    for (const auto& file : fileList)
    {
        printf("  - %s (%s)\n", file.filename.ToStdString().c_str(), file.isDirectory ? "dir" : "file");
    }
    
    // Load taxonomy data
    if (!LoadTaxonomyData())
    {
        printf("LoadArchive: Failed to load taxonomy data\n");
        return false;
    }
    
    // Get available languages
    std::vector<wxString> languageFiles = m_zipReader->GetLanguageFiles();
    printf("LoadArchive: Found %zu language files:\n", languageFiles.size());
    for (const auto& file : languageFiles)
    {
        printf("  - %s\n", file.ToStdString().c_str());
    }
    
    m_availableLanguages.clear();
    
    for (const auto& file : languageFiles)
    {
        wxString language = CSVParser::ExtractLanguageFromFilename(file);
        if (!language.IsEmpty())
        {
            m_availableLanguages.push_back(language);
            printf("  Language extracted: %s\n", language.ToStdString().c_str());
        }
    }
    
    printf("LoadArchive: Final available languages: %zu\n", m_availableLanguages.size());
    return !m_availableLanguages.empty();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
bool TaxonomyProcessor::LoadArchive(const wxString& archivePath, std::function<void(const wxString&, int)> progressCallback)
{
    auto updateProgress = [&](int percent, const char* format, auto... args) {
        char buffer[512];
        int result = snprintf(buffer, sizeof(buffer), format, args...);
        (void)result;  // Suppress unused variable warning
        if (progressCallback) {
            progressCallback(wxString(buffer), percent);
        }
    };
    
    updateProgress(5, "Opening archive: %s", archivePath.ToStdString().c_str());
    
    if (!m_zipReader->OpenArchive(archivePath))
    {
        updateProgress(0, "Failed to open archive");
        return false;
    }

    auto fileList = m_zipReader->GetFileList();
    updateProgress(10, "Archive contains %d files", (int)fileList.size());

    // Show some files in archive (not all to avoid too much spam)
    for (size_t i = 0; i < fileList.size(); ++i)
    {
        int percent = 10 + (i * 20) / fileList.size(); // 10-30%
        // Show every 5th file or important files
        if (i % 5 == 0 || fileList[i].filename.Contains("taxa") || fileList[i].filename.Contains("vernacular"))
        {
            updateProgress(percent, "Found file: %s", fileList[i].filename.ToStdString().c_str());
        }
    }

    updateProgress(30, "Starting taxonomy data loading...");
    if (!LoadTaxonomyData(progressCallback))
    {
        updateProgress(30, "Failed to load taxonomy data");
        return false;
    }

    updateProgress(70, "Scanning for language files...");
    std::vector<wxString> languageFiles = m_zipReader->GetLanguageFiles();
    updateProgress(75, "Found %d language files", (int)languageFiles.size());

    m_availableLanguages.clear();
    for (size_t i = 0; i < languageFiles.size(); ++i)
    {
        int percent = 75 + (i * 20) / languageFiles.size(); // 75-95%
        const auto& file = languageFiles[i];
        updateProgress(percent, "Processing language file: %s", file.ToStdString().c_str());
        
        wxString language = CSVParser::ExtractLanguageFromFilename(file);
        if (!language.IsEmpty())
        {
            m_availableLanguages.push_back(language);
            updateProgress(percent, "Language extracted: %s", language.ToStdString().c_str());
        }
    }

    updateProgress(100, "Archive loaded successfully! Found %d languages, %d taxonomy entries", 
                   (int)m_availableLanguages.size(), (int)m_taxonomyMap.size());

    return !m_availableLanguages.empty();
}

std::vector<wxString> TaxonomyProcessor::GetAvailableLanguages() const
{
    return m_availableLanguages;
}

bool TaxonomyProcessor::LoadTaxonomyData()
{
    printf("LoadTaxonomyData: Trying to load taxonomy data...\n");
    
    wxString taxonomyContent;
    if (!m_zipReader->ReadFileToString("taxa.csv", taxonomyContent))
    {
        printf("LoadTaxonomyData: Failed to read 'taxa.csv', trying 'taxonomy.csv'\n");
        // Try alternative filename
        if (!m_zipReader->ReadFileToString("taxonomy.csv", taxonomyContent))
        {
            printf("LoadTaxonomyData: Failed to read 'taxonomy.csv', trying 'taxa.txt'\n");
            if (!m_zipReader->ReadFileToString("taxa.txt", taxonomyContent))
            {
                printf("LoadTaxonomyData: Failed to read 'taxa.txt'\n");
                return false;
            }
        }
    }
    
    printf("LoadTaxonomyData: Successfully read taxonomy file, content length: %zu\n", taxonomyContent.length());
    
    CSVParser parser;
    bool result = parser.ParseTaxonomyCSV(taxonomyContent, m_taxonomyMap);
    printf("LoadTaxonomyData: Parse result: %s, taxonomy entries: %zu\n", 
                result ? "success" : "failed", m_taxonomyMap.size());
    return result;
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
bool TaxonomyProcessor::LoadTaxonomyData(std::function<void(const wxString&, int)> progressCallback)
{
    auto updateProgress = [&](int percent, const char* format, auto... args) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), format, args...);
        if (progressCallback) {
            progressCallback(wxString(buffer), percent);
        }
    };
    
    updateProgress(35, "Trying to load taxonomy data...");
    
    wxString taxonomyContent;
    if (!m_zipReader->ReadFileToString("taxa.csv", taxonomyContent))
    {
        updateProgress(40, "Failed to read 'taxa.csv', trying 'taxonomy.csv'");
        // Try alternative filename
        if (!m_zipReader->ReadFileToString("taxonomy.csv", taxonomyContent))
        {
            updateProgress(45, "Failed to read 'taxonomy.csv', trying 'taxa.txt'");
            if (!m_zipReader->ReadFileToString("taxa.txt", taxonomyContent))
            {
                updateProgress(45, "Failed to read 'taxa.txt'");
                return false;
            }
        }
    }
    
    updateProgress(50, "Successfully read taxonomy file, content length: %d bytes", (int)taxonomyContent.length());
    
    updateProgress(55, "Parsing taxonomy CSV data...");
    CSVParser parser;
    bool result = parser.ParseTaxonomyCSV(taxonomyContent, m_taxonomyMap);
    updateProgress(65, "Parse result: %s, taxonomy entries: %d", 
                result ? "success" : "failed", (int)m_taxonomyMap.size());
    
    return result;
}
#pragma GCC diagnostic pop

bool TaxonomyProcessor::LoadVernacularData(const wxString& language, std::vector<VernacularEntry>& entries)
{
    // Check cache first
    auto it = m_vernacularCache.find(language);
    if (it != m_vernacularCache.end())
    {
        entries = it->second;
        return true;
    }
    
    wxString filename = wxString::Format("VernacularNames-%s.csv", language);
    wxString content;
    
    if (!m_zipReader->ReadFileToString(filename, content))
        return false;
    
    CSVParser parser;
    if (!parser.ParseVernacularCSV(content, entries))
        return false;
    
    // Cache the results
    m_vernacularCache[language] = entries;
    return true;
}

bool TaxonomyProcessor::LoadScientificNameEntries(std::vector<VernacularEntry>& entries)
{
    entries.clear();
    
    // Convert taxonomy entries to vernacular-like entries for processing
    for (const auto& taxonomyPair : m_taxonomyMap)
    {
        const auto& taxonomyEntry = taxonomyPair.second;
        if (!taxonomyEntry.scientificName.IsEmpty())
        {
            VernacularEntry scientificEntry;
            scientificEntry.taxonId = taxonomyEntry.id; // Both are now long
            scientificEntry.vernacularName = taxonomyEntry.scientificName;
            scientificEntry.language = "Scientific Name";
            scientificEntry.countryCode = "";
            entries.push_back(scientificEntry);
        }
    }
    
    return !entries.empty();
}

bool TaxonomyProcessor::GenerateDictionary(const wxString& sourceLanguage, 
                                         const std::vector<wxString>& targetLanguages,
                                         bool includeScientific,
                                         const wxString& outputPath)
{
    auto dictionary = BuildDictionary(sourceLanguage, targetLanguages, includeScientific);
    
    if (dictionary.empty())
        return false;
    
    // Create output directory if it doesn't exist
    wxFileName outputDir(outputPath, "");
    if (!outputDir.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
    {
        if (!outputDir.DirExists())
            return false;
    }
    
    // Generate output files
    wxString baseName = "taxonomy_" + sourceLanguage + "_dictionary";
    
    // Generate TAB-separated file
    if (!m_dictGenerator->GenerateTSV(dictionary, outputPath + "/" + baseName + ".tsv"))
        return false;
    
    // Generate StarDict files
    if (!m_dictGenerator->GenerateStarDict(dictionary, outputPath, baseName, sourceLanguage, targetLanguages))
        return false;
    
    return true;
}

std::map<wxString, std::vector<wxString>> TaxonomyProcessor::BuildDictionary(
    const wxString& sourceLanguage,
    const std::vector<wxString>& targetLanguages,
    bool includeScientific)
{
    std::map<wxString, std::vector<wxString>> dictionary;
    
    // Load source language data
    std::vector<VernacularEntry> sourceEntries;
    if (!LoadVernacularData(sourceLanguage, sourceEntries))
        return dictionary;
    
    // Load target language data
    std::map<wxString, std::vector<VernacularEntry>> targetData;
    for (const auto& targetLang : targetLanguages)
    {
        std::vector<VernacularEntry> targetEntries;
        if (LoadVernacularData(targetLang, targetEntries))
        {
            targetData[targetLang] = targetEntries;
        }
    }
    
    // Build dictionary mapping
    for (const auto& sourceEntry : sourceEntries)
    {
        std::vector<wxString> translations;
        
        // Add scientific name if requested
        if (includeScientific)
        {
            auto taxonomyIt = m_taxonomyMap.find(sourceEntry.taxonId);
            if (taxonomyIt != m_taxonomyMap.end() && !taxonomyIt->second.scientificName.IsEmpty())
            {
                // Add the scientific name with language information
                translations.push_back(taxonomyIt->second.scientificName + wxT("|Scientific Name"));
                
                // Add iNaturalist URL for this taxon
                wxString inaturalistUrl = wxString::Format("https://www.inaturalist.org/taxa/%ld", sourceEntry.taxonId);
                translations.push_back(inaturalistUrl);
            }
        }
        
        // Add translations from target languages
        for (const auto& targetPair : targetData)
        {
            const wxString& targetLanguage = targetPair.first;
            const auto& targetEntries = targetPair.second;
            
            for (const auto& targetEntry : targetEntries)
            {
                if (targetEntry.taxonId == sourceEntry.taxonId)
                {
                    // Add translation with language information
                    wxString translationWithLang = targetEntry.vernacularName + wxT("|") + targetLanguage;
                    // Avoid duplicates
                    if (std::find(translations.begin(), translations.end(), translationWithLang) == translations.end())
                    {
                        translations.push_back(translationWithLang);
                    }
                }
            }
        }
        
        if (!translations.empty())
        {
            // If we already have this source word, merge translations
            auto existingIt = dictionary.find(sourceEntry.vernacularName);
            if (existingIt != dictionary.end())
            {
                for (const auto& translation : translations)
                {
                    if (std::find(existingIt->second.begin(), existingIt->second.end(), translation) == existingIt->second.end())
                    {
                        existingIt->second.push_back(translation);
                    }
                }
            }
            else
            {
                dictionary[sourceEntry.vernacularName] = translations;
            }
        }
    }
    
    return dictionary;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
bool TaxonomyProcessor::GenerateDictionary(const wxString& sourceLanguage, 
                                         const std::vector<wxString>& targetLanguages,
                                         bool includeScientific,
                                         bool filterOnlyTargets,
                                         const wxString& outputPath,
                                         std::function<void(const wxString&, int)> progressCallback)
{
    auto updateProgress = [&](int percent, const char* format, auto... args) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), format, args...);
        if (progressCallback) {
            progressCallback(wxString(buffer), percent);
        }
    };
    
    updateProgress(5, "Starting dictionary generation...");
    
    auto dictionary = BuildDictionary(sourceLanguage, targetLanguages, includeScientific, filterOnlyTargets, progressCallback);
    
    if (dictionary.empty())
    {
        updateProgress(0, "Dictionary generation failed - no entries found");
        return false;
    }
    
    updateProgress(70, "Dictionary built with %d entries, creating output directory...", (int)dictionary.size());
    
    // Create output directory if it doesn't exist
    wxFileName outputDir(outputPath, "");
    if (!outputDir.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
    {
        if (!outputDir.DirExists())
        {
            updateProgress(70, "Failed to create output directory");
            return false;
        }
    }
    
    updateProgress(75, "Output directory ready, generating files...");
    
    // Generate output files
    wxString baseName = "taxonomy_" + sourceLanguage + "_dictionary";
    
    updateProgress(80, "Generating TSV file...");
    // Generate TAB-separated file
    if (!m_dictGenerator->GenerateTSV(dictionary, outputPath + "/" + baseName + ".tsv"))
    {
        updateProgress(80, "Failed to generate TSV file");
        return false;
    }
    
    updateProgress(90, "Generating StarDict files...");
    // Generate StarDict files
    if (!m_dictGenerator->GenerateStarDict(dictionary, outputPath, baseName, sourceLanguage, targetLanguages))
    {
        updateProgress(90, "Failed to generate StarDict files");
        return false;
    }
    
    updateProgress(100, "Dictionary generation completed successfully! Generated %d entries", (int)dictionary.size());
    return true;
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
std::map<wxString, std::vector<wxString>> TaxonomyProcessor::BuildDictionary(
    const wxString& sourceLanguage,
    const std::vector<wxString>& targetLanguages,
    bool includeScientific,
    bool filterOnlyTargets,
    std::function<void(const wxString&, int)> progressCallback)
{
    auto updateProgress = [&](int percent, const char* format, auto... args) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), format, args...);
        if (progressCallback) {
            progressCallback(wxString(buffer), percent);
        }
    };
    
    std::map<wxString, std::vector<wxString>> dictionary;
    
    updateProgress(10, "Loading source language data: %s", sourceLanguage.ToStdString().c_str());
    
    // Load source language data
    std::vector<VernacularEntry> sourceEntries;
    if (sourceLanguage == "Scientific Name")
    {
        if (!LoadScientificNameEntries(sourceEntries))
        {
            updateProgress(10, "Failed to load scientific name data");
            return dictionary;
        }
    }
    else
    {
        if (!LoadVernacularData(sourceLanguage, sourceEntries))
        {
            updateProgress(10, "Failed to load source language data");
            return dictionary;
        }
    }
    
    updateProgress(20, "Loaded %d source entries, loading target languages...", (int)sourceEntries.size());
    
    // Load target language data
    std::map<wxString, std::vector<VernacularEntry>> targetData;
    for (size_t i = 0; i < targetLanguages.size(); ++i)
    {
        const auto& targetLang = targetLanguages[i];
        int percent = 20 + (i * 30) / targetLanguages.size(); // 20-50%
        
        // Skip "Scientific Name" as target language - it's handled through includeScientific flag
        if (targetLang == "Scientific Name")
        {
            updateProgress(percent, "Skipping Scientific Name as target language (use 'Include Scientific Names' instead)");
            continue;
        }
        
        updateProgress(percent, "Loading target language: %s", targetLang.ToStdString().c_str());
        
        std::vector<VernacularEntry> targetEntries;
        if (LoadVernacularData(targetLang, targetEntries))
        {
            targetData[targetLang] = targetEntries;
            updateProgress(percent, "Loaded %d entries for %s", (int)targetEntries.size(), targetLang.ToStdString().c_str());
        }
    }
    
    updateProgress(50, "Building dictionary mappings...");
    
    // Basic debugging info
    std::cout << "Dictionary generation: " << sourceLanguage.ToStdString() 
              << " -> " << targetLanguages.size() << " target language(s), "
              << "filter=" << (filterOnlyTargets ? "enabled" : "disabled") << std::endl;
    
    // Build dictionary mapping
    for (size_t i = 0; i < sourceEntries.size(); ++i)
    {
        if (i % 100 == 0) // Update progress every 100 entries
        {
            int percent = 50 + (i * 20) / sourceEntries.size(); // 50-70%
            updateProgress(percent, "Processing entry %d of %d...", (int)(i + 1), (int)sourceEntries.size());
        }
        
        const auto& sourceEntry = sourceEntries[i];
        std::vector<wxString> translations;
        
        // Add scientific name if requested
        if (includeScientific)
        {
            auto taxonomyIt = m_taxonomyMap.find(sourceEntry.taxonId);
            if (taxonomyIt != m_taxonomyMap.end() && !taxonomyIt->second.scientificName.IsEmpty())
            {
                // Add the scientific name with language information
                translations.push_back(taxonomyIt->second.scientificName + wxT("|Scientific Name"));
                
                // Add iNaturalist URL for this taxon
                wxString inaturalistUrl = wxString::Format("https://www.inaturalist.org/taxa/%ld", sourceEntry.taxonId);
                translations.push_back(inaturalistUrl);
            }
        }
        
        // Add translations from target languages
        bool hasTargetTranslation = false;
        for (const auto& targetPair : targetData)
        {
            const wxString& targetLanguage = targetPair.first;
            const auto& targetEntries = targetPair.second;
            
            for (const auto& targetEntry : targetEntries)
            {
                if (targetEntry.taxonId == sourceEntry.taxonId)
                {
                    hasTargetTranslation = true;
                    // Add translation with language information
                    wxString translationWithLang = targetEntry.vernacularName + wxT("|") + targetLanguage;
                    // Avoid duplicates
                    if (std::find(translations.begin(), translations.end(), translationWithLang) == translations.end())
                    {
                        translations.push_back(translationWithLang);
                        
                    }
                }
            }
        }
        
        // Apply filtering logic
        bool shouldInclude = !translations.empty();
        if (filterOnlyTargets && !targetLanguages.empty())
        {
            // Only include if there are actual target language translations (not just scientific name)
            shouldInclude = hasTargetTranslation;
            
        }
        
        if (shouldInclude)
        {
            // If we already have this source word, merge translations
            auto existingIt = dictionary.find(sourceEntry.vernacularName);
            if (existingIt != dictionary.end())
            {
                for (const auto& translation : translations)
                {
                    if (std::find(existingIt->second.begin(), existingIt->second.end(), translation) == existingIt->second.end())
                    {
                        existingIt->second.push_back(translation);
                    }
                }
            }
            else
            {
                dictionary[sourceEntry.vernacularName] = translations;
            }
        }
    }
    
    updateProgress(70, "Dictionary building completed, found %d entries", (int)dictionary.size());
    
    // Final result summary
    std::cout << "Dictionary completed: " << dictionary.size() << " entries generated" << std::endl;
    
    return dictionary;
}
#pragma GCC diagnostic pop