#pragma once

#include <wx/string.h>
#include "../common/Translator.hpp"
#include "../common/TranslatorValidator.hpp"

// String IDs for TaxonomyDict application
enum class DictStringId {
    // Window titles
    AppTitle,

    // Menu items
    MenuFile,
    MenuFileExit,
    MenuFileExitHelp,
    MenuHelp,
    MenuHelpAbout,
    MenuHelpAboutHelp,

    // Button labels
    BtnDownloadArchive,
    BtnLoadArchive,
    BtnGenerateDict,
    BtnBrowse,

    // Dialog titles
    DlgAbout,
    DlgSaveArchive,
    DlgChooseArchive,
    DlgError,
    DlgInvalidURL,
    DlgThreadError,

    // Section titles
    SectionDownload,
    SectionLanguageSelection,
    SectionOutput,

    // Control labels
    LabelTaxonomyURL,
    LabelSourceLanguage,
    LabelTargetLanguages,
    LabelOutputPath,

    // Checkbox labels
    CheckIncludeScientific,
    CheckFilterTargets,

    // Static values
    ValueScientificName,
    ValueDefaultURL,
    ValueDefaultOutput,

    // Status messages
    StatusReady,
    StatusDownloading,
    StatusDownloadStarted,
    StatusDownloadingPercent,      // "Downloading... %1$d%%"
    StatusDownloadComplete,
    StatusDownloadFailed,           // "Download failed: %1$s"
    StatusLoadingArchive,
    StatusLoadStartFailed,
    StatusLoadFailed,
    StatusGenerating,
    StatusGenerateFailed,

    // Error messages
    ErrEnterURL,
    ErrInvalidURLScheme,
    ErrThreadStartFailed,           // "Failed to start download thread (error: %1$d)"
    ErrLoadThreadFailed,
    ErrNoSourceLanguage,
    ErrNoTargetLanguage,
    ErrGenerateThreadFailed,
    ErrArchiveLoadFailed,
    ErrGenerationFailed,
    ErrCURLInitFailed,

    // About dialog message
    AboutMessage,

    // File dialog filters
    FilterZipFiles,

    // Progress messages - Archive operations
    ProgOpeningArchive,             // "Opening archive: %1$s"
    ProgArchiveFileCount,           // "Archive contains %1$d files"
    ProgFoundFile,                  // "Found file: %1$s"
    ProgStartingTaxonomyLoad,
    ProgScanningLanguages,
    ProgFoundLanguageFiles,         // "Found %1$d language files"
    ProgProcessingLanguageFile,     // "Processing language file: %1$s"
    ProgLanguageExtracted,          // "Language extracted: %1$s"
    ProgArchiveLoadSuccess,         // "Archive loaded successfully! Found %1$d languages, %2$d taxonomy entries"
    ProgFailedOpenArchive,

    // Progress messages - Taxonomy loading
    ProgTryingTaxonomyLoad,
    ProgFailedTaxaCSV,
    ProgFailedTaxonomyCSV,
    ProgFailedTaxaTXT,
    ProgSuccessReadTaxonomy,        // "Successfully read taxonomy file, content length: %1$d bytes"
    ProgParsingTaxonomyCSV,
    ProgParseResult,                // "Parse result: %1$s, taxonomy entries: %2$d"

    // Progress messages - Dictionary generation
    ProgStartingGeneration,
    ProgGenerationFailedNoEntries,
    ProgDictionaryBuilt,            // "Dictionary built with %1$d entries, creating output directory..."
    ProgOutputReady,
    ProgGeneratingTSV,
    ProgGeneratingStarDict,
    ProgGenerationComplete,         // "Dictionary generation completed successfully! Generated %1$d entries"
    ProgLoadingSourceData,          // "Loading source language data: %1$s"
    ProgFailedScientificData,
    ProgFailedSourceData,
    ProgLoadedSourceEntries,        // "Loaded %1$d source entries, loading target languages..."
    ProgSkippingScientificTarget,
    ProgLoadingTargetLanguage,      // "Loading target language: %1$s"
    ProgLoadedTargetEntries,        // "Loaded %1$d entries for %2$s"
    ProgBuildingMappings,
    ProgProcessingEntry,            // "Processing entry %1$d of %2$d..."
    ProgBuildingComplete,           // "Dictionary building completed, found %1$d entries"

    // Search UI
    SearchPrompt,                   // "Search: %1$s"

    COUNT
};

// Forward declare translation arrays (defined in DictStrings_XX.cpp)
extern const char* const DICT_STRINGS_EN[static_cast<size_t>(DictStringId::COUNT)];
extern const char* const DICT_STRINGS_LT[static_cast<size_t>(DictStringId::COUNT)];
extern const char* const DICT_STRINGS_ES[static_cast<size_t>(DictStringId::COUNT)];

// Translation getter function - returns wxString with proper UTF-8 conversion
inline wxString TR_DICT(DictStringId id) {
    Language lang = TR_LANG();
    size_t index = static_cast<size_t>(id);

    const char* str = nullptr;
    switch (lang) {
        case Language::Lithuanian:
            str = DICT_STRINGS_LT[index];
            break;
        case Language::Spanish:
            str = DICT_STRINGS_ES[index];
            break;
        case Language::English:
        default:
            str = DICT_STRINGS_EN[index];
            break;
    }

    return wxString(str, wxConvUTF8);
}

// Helper for formatted strings - returns wxString with proper UTF-8 conversion
template<typename... Args>
inline wxString TR_DICT_FMT(DictStringId id, Args... args) {
    Language lang = TR_LANG();
    size_t index = static_cast<size_t>(id);

    const char* fmt = nullptr;
    switch (lang) {
        case Language::Lithuanian:
            fmt = DICT_STRINGS_LT[index];
            break;
        case Language::Spanish:
            fmt = DICT_STRINGS_ES[index];
            break;
        case Language::English:
        default:
            fmt = DICT_STRINGS_EN[index];
            break;
    }

    std::string formatted = Translator::Format(fmt, args...);
    return wxString(formatted.c_str(), wxConvUTF8);
}
