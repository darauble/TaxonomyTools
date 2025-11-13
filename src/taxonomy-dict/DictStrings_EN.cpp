#include "DictStrings.hpp"

// English translations for TaxonomyDict
const char* const DICT_STRINGS_EN[] = {
    // Window titles
    "Taxonomy Dictionary Generator",

    // Menu items
    "&File",
    "&Exit\tCtrl+Q",
    "Exit the application",
    "&Help",
    "&About\tF1",
    "About this application",

    // Button labels
    "Download Archive",
    "Load Existing Archive...",
    "Generate Dictionary",
    "Browse...",

    // Dialog titles
    "About",
    "Save archive as...",
    "Choose archive file",
    "Error",
    "Invalid URL",
    "Thread Error",

    // Section titles
    "Download Archive",
    "Language Selection",
    "Output",

    // Control labels
    "iNaturalist Taxonomy URL:",
    "Source Language:",
    "Target Languages:",
    "Output Path:",

    // Checkbox labels
    "Include Scientific Names",
    "Filter only target languages",

    // Static values
    "Scientific Name",
    "https://www.inaturalist.org/taxa/inaturalist-taxonomy.dwca.zip",
    "./output",

    // Status messages
    "Ready",
    "Downloading...",
    "Download thread started...",
    "Downloading... %d%%",
    "Download completed successfully! Use 'Load Existing Archive' to process it.",
    "Download failed: %s",
    "Loading archive...",
    "Failed to start loading thread",
    "Failed to load archive",
    "Starting dictionary generation...",
    "Dictionary generation failed",

    // Error messages
    "Please enter a URL to download",
    "URL must start with http:// or https://",
    "Failed to start download thread (error: %d)",
    "Failed to start loading thread",
    "Please select a source language",
    "Please select at least one target language or include scientific names",
    "Failed to start dictionary generation thread",
    "Failed to load archive",
    "Dictionary generation failed",
    "Failed to initialize CURL",

    // About dialog message (note: version will be prepended at runtime)
    "\n\nGenerates translation dictionaries from iNaturalist taxonomy data.",

    // File dialog filters
    "ZIP files (*.zip)|*.zip",

    // Progress messages - Archive operations
    "Opening archive: %s",
    "Archive contains %d files",
    "Found file: %s",
    "Starting taxonomy data loading...",
    "Scanning for language files...",
    "Found %d language files",
    "Processing language file: %s",
    "Language extracted: %s",
    "Archive loaded successfully! Found %d languages, %d taxonomy entries",
    "Failed to open archive",

    // Progress messages - Taxonomy loading
    "Trying to load taxonomy data...",
    "Failed to read 'taxa.csv', trying 'taxonomy.csv'",
    "Failed to read 'taxonomy.csv', trying 'taxa.txt'",
    "Failed to read 'taxa.txt'",
    "Successfully read taxonomy file, content length: %d bytes",
    "Parsing taxonomy CSV data...",
    "Parse result: %s, taxonomy entries: %d",

    // Progress messages - Dictionary generation
    "Starting dictionary generation...",
    "Dictionary generation failed - no entries found",
    "Dictionary built with %d entries, creating output directory...",
    "Output directory ready, generating files...",
    "Generating TSV file...",
    "Generating StarDict files...",
    "Dictionary generation completed successfully! Generated %d entries",
    "Loading source language data: %s",
    "Failed to load scientific name data",
    "Failed to load source language data",
    "Loaded %d source entries, loading target languages...",
    "Skipping Scientific Name as target language (use 'Include Scientific Names' instead)",
    "Loading target language: %s",
    "Loaded %d entries for %s",
    "Building dictionary mappings...",
    "Processing entry %d of %d...",
    "Dictionary building completed, found %d entries",

    // Search UI
    "Search: %s",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(DictStringId, DICT_STRINGS_EN);
