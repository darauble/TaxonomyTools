#include "TreeStrings.hpp"

// English translations for TaxonomyTree
const char* const TREE_STRINGS_EN[] = {
    // Window title
    "Taxonomy Tree - Taxonomic Family Tree Viewer",

    // Menu bar labels
    "&File",
    "&View",
    "&Help",

    // File Menu items
    "&Download Taxonomy Data\tCtrl+D",
    "Download taxonomy data from iNaturalist",
    "&Load Archive\tCtrl+O",
    "Load taxonomy data archive",
    "Export to &SVG\tCtrl+S",
    "Export tree to SVG format",
    "Export to &PNG\tCtrl+P",
    "Export tree to PNG format",
    "Se&ttings",
    "Open settings dialog",
    "E&xit\tCtrl+Q",
    "Exit application",

    // View Menu items
    "&Fan Layout\tCtrl+1",
    "Display tree in fan layout",
    "&Vertical Layout\tCtrl+2",
    "Display tree vertically",
    "&Horizontal Layout\tCtrl+3",
    "Display tree horizontally",
    "Show &Boxes\tCtrl+B",
    "Toggle node boxes",
    "Show &Full Tree\tCtrl+F",
    "Show full tree to kingdom",
    "Default &Zoom (100%)\tCtrl+0",
    "Reset zoom to 100%",
    "Zoom &In\tCtrl+=",
    "Zoom in",
    "Zoom &Out\tCtrl+-",
    "Zoom out",

    // Help Menu
    "&About\tF1",
    "About Taxonomy Tree",

    // Dialog titles
    "Error",
    "Success",
    "Cancelled",
    "Save taxonomy archive",
    "Open taxonomy archive",
    "Export to SVG",
    "Export to PNG",
    "About TaxonomyTree",
    "Downloading",
    "Loading",
    "Indexing",
    "Closing",
    "Settings",
    "PNG Export",

    // File dialog filters
    "ZIP files (*.zip)|*.zip",
    "All files (*.*)|*.*",
    "SVG files (*.svg)|*.svg",
    "PNG files (*.png)|*.png",

    // Default file names
    "inaturalist-taxonomy.zip",
    "taxonomy_tree.svg",
    "taxonomy_tree.png",

    // Button labels
    "Clear List",
    "Browse...",
    "OK",
    "Cancel",

    // Control labels
    "Primary Language:",
    "Secondary Language:",
    "Search Taxa:",
    "Search Results (double-click to add):",
    "Compare List (double-click to remove):",

    // Settings dialog labels
    "Taxonomy File",
    "File Path:",
    "Load taxonomy data on startup",

    // Status messages
    "Ready",
    "Loaded %1$zu taxa",
    "Found %1$zu results",

    // Special values
    "(None)",

    // Download messages
    "Downloading taxonomy data...",
    "Failed to start download",
    "Download cancelled",
    "Download completed successfully!\n\nYou can now load the archive using File -> Load Archive.",
    "Download failed: %1$s",

    // Loading messages
    "Loading taxonomy data...",
    "File does not exist:\n%1$s",
    "Failed to load taxonomy data",

    // Indexing messages
    "Building search index for %1$s...",
    " and %1$s",

    // Export messages
    "No taxa selected for comparison",
    "Tree exported to SVG successfully!",
    "Failed to export tree to SVG",
    "Tree exported to PNG at %1$ld DPI successfully!",
    "Failed to export tree to PNG",
    "Invalid DPI value. Please enter a number between 70 and 600.",

    // DPI input
    "Enter DPI (70-600):",
    "150",

    // Closing message
    "Cleaning up and closing...\nPlease wait.",

    // About dialog (note: version will be prepended at runtime to first string)
    "\n\n",
    "Taxonomic Family Tree Viewer\n\n",
    "Compare species and visualize their taxonomic relationships.",

    // Settins dialog
    "Language",
    "Use system language settings",
    "Detected Language:",
    "Application Language:",

    // Progress messages for archive loading
    "Opening archive: %1$s",
    "Failed to open archive",
    "Archive contains %1$zu files",
    "Loading taxonomy data...",
    "Failed to load taxonomy data",
    "Building taxonomy hierarchy...",
    "Scanning for language files...",
    "Found %1$zu language files",
    "Archive loaded! %1$zu taxa, %2$zu languages",
    "Reading taxonomy CSV file...",
    "Failed to read taxonomy file",
    "File read, size: %1$zu bytes",
    "Failed to parse taxonomy CSV",
    "Parsed %1$zu taxonomy entries",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(TreeStringId, TREE_STRINGS_EN);
