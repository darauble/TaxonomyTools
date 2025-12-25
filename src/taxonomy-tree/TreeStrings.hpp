#pragma once

#include <wx/string.h>
#include "../common/Translator.hpp"
#include "../common/TranslatorValidator.hpp"

// String IDs for TaxonomyTree application
enum class TreeStringId {
    // Window title
    AppTitle,

    // Menu bar labels
    MenuFile,
    MenuView,
    MenuHelp,

    // File Menu items
    MenuFileDownload,
    MenuFileDownloadHelp,
    MenuFileLoadArchive,
    MenuFileLoadArchiveHelp,
    MenuFileExportSVG,
    MenuFileExportSVGHelp,
    MenuFileExportPNG,
    MenuFileExportPNGHelp,
    MenuFileSettings,
    MenuFileSettingsHelp,
    MenuFileExit,
    MenuFileExitHelp,

    // View Menu items
    MenuViewFanLayout,
    MenuViewFanLayoutHelp,
    MenuViewVerticalLayout,
    MenuViewVerticalLayoutHelp,
    MenuViewHorizontalLayout,
    MenuViewHorizontalLayoutHelp,
    MenuViewShowBoxes,
    MenuViewShowBoxesHelp,
    MenuViewShowFullTree,
    MenuViewShowFullTreeHelp,
    MenuViewZoomDefault,
    MenuViewZoomDefaultHelp,
    MenuViewZoomIn,
    MenuViewZoomInHelp,
    MenuViewZoomOut,
    MenuViewZoomOutHelp,

    // Help Menu
    MenuHelpAbout,
    MenuHelpAboutHelp,

    // Dialog titles
    DlgError,
    DlgSuccess,
    DlgCancelled,
    DlgSaveTaxonomyArchive,
    DlgOpenTaxonomyArchive,
    DlgExportSVG,
    DlgExportPNG,
    DlgAboutTitle,
    DlgDownloading,
    DlgLoading,
    DlgIndexing,
    DlgClosing,
    DlgSettings,
    DlgPNGExport,

    // File dialog filters
    FilterZipFiles,
    FilterAllFiles,
    FilterSVGFiles,
    FilterPNGFiles,

    // Default file names
    DefaultDownloadFilename,
    DefaultSVGFilename,
    DefaultPNGFilename,

    // Button labels
    BtnClearList,
    BtnBrowse,
    BtnOK,
    BtnCancel,

    // Control labels
    LabelPrimaryLanguage,
    LabelSecondaryLanguage,
    LabelSearchTaxa,
    LabelSearchResults,
    LabelCompareList,

    // Settings dialog labels
    SettingsBoxTaxonomyFile,
    LabelFilePath,
    CheckLoadOnStartup,

    // Status messages
    StatusReady,
    StatusLoadedTaxa,                // "Loaded %1$zu taxa"
    StatusFoundResults,              // "Found %1$zu results"

    // Special values
    ValueNoneLanguage,

    // Download messages
    MsgDownloadingData,
    MsgDownloadStartFailed,
    MsgDownloadCancelled,
    MsgDownloadSuccess,
    MsgDownloadFailed,               // "Download failed: %1$s"

    // Loading messages
    MsgLoadingData,
    MsgFileNotExist,                 // "File does not exist:\n%1$s"
    MsgLoadFailed,

    // Indexing messages
    MsgBuildingIndex,                // "Building search index for %1$s..."
    MsgBuildingIndexAnd,             // " and %1$s"

    // Export messages
    MsgNoTaxaSelected,
    MsgSVGExportSuccess,
    MsgSVGExportFailed,
    MsgPNGExportSuccess,             // "Tree exported to PNG at %1$ld DPI successfully!"
    MsgPNGExportFailed,
    MsgInvalidDPI,

    // DPI input
    PromptEnterDPI,
    ValueDefaultDPI,

    // Closing message
    MsgClosingCleanup,

    // About dialog
    AboutVersion,
    AboutDescription,
    AboutPurpose,

    // Settins dialog
    LabelLanguage,
    LabelUseSystemLanguage,
    LabelDetectedLanguage,
    LabelApplicationLanguage,

    // Progress messages for archive loading
    ProgressOpeningArchive,              // "Opening archive: %1$s"
    ProgressFailedOpenArchive,
    ProgressArchiveContainsFiles,        // "Archive contains %1$zu files"
    ProgressLoadingTaxonomyData,
    ProgressFailedLoadTaxonomyData,
    ProgressBuildingHierarchy,
    ProgressScanningLanguageFiles,
    ProgressFoundLanguageFiles,          // "Found %1$zu language files"
    ProgressArchiveLoaded,               // "Archive loaded! %1$zu taxa, %2$zu languages"
    ProgressReadingCSV,
    ProgressFailedReadCSV,
    ProgressFileRead,                    // "File read, size: %1$zu bytes"
    ProgressFailedParseCSV,
    ProgressParsedEntries,               // "Parsed %1$zu taxonomy entries"

    // Cache-related progress messages
    ProgressCreatingSchema,
    ProgressFoundAvailableLanguages,     // "Found %1$zu available languages"
    ProgressInsertingTaxonomyEntries,    // "Inserting %1$zu taxonomy entries..."
    ProgressLoadingLanguage,             // "Loading language: %1$s"
    ProgressBuildingSearchIndexVernacular,
    ProgressBuildingSearchIndexScientific,
    ProgressSearchIndexComplete,
    ProgressCacheBuildComplete,
    ProgressUpdatingSearchIndex,

    // Cache validation messages
    MsgCacheOutOfDate,
    MsgRebuildCache,
    MsgFileNotFoundAtPath,               // "Saved taxonomy file not found:\n%1$s\n\nPlease select a new file via File > Load Archive."

    COUNT
};

// Forward declare translation arrays (defined in TreeStrings_XX.cpp)
extern const char* const TREE_STRINGS_EN[static_cast<size_t>(TreeStringId::COUNT)];
extern const char* const TREE_STRINGS_LT[static_cast<size_t>(TreeStringId::COUNT)];
extern const char* const TREE_STRINGS_ES[static_cast<size_t>(TreeStringId::COUNT)];

// Translation getter function - returns wxString with proper UTF-8 conversion
inline wxString TR_TREE(TreeStringId id) {
    Language lang = TR_LANG();
    size_t index = static_cast<size_t>(id);

    const char* str = nullptr;
    switch (lang) {
        case Language::Lithuanian:
            str = TREE_STRINGS_LT[index];
            break;
        case Language::Spanish:
            str = TREE_STRINGS_ES[index];
            break;
        case Language::English:
        default:
            str = TREE_STRINGS_EN[index];
            break;
    }

    return wxString(str, wxConvUTF8);
}

// Helper for formatted strings - returns wxString with proper UTF-8 conversion
template<typename... Args>
inline wxString TR_TREE_FMT(TreeStringId id, Args... args) {
    Language lang = TR_LANG();
    size_t index = static_cast<size_t>(id);

    const char* fmt = nullptr;
    switch (lang) {
        case Language::Lithuanian:
            fmt = TREE_STRINGS_LT[index];
            break;
        case Language::Spanish:
            fmt = TREE_STRINGS_ES[index];
            break;
        case Language::English:
        default:
            fmt = TREE_STRINGS_EN[index];
            break;
    }

    std::string formatted = Translator::Format(fmt, args...);
    return wxString(formatted.c_str(), wxConvUTF8);
}
