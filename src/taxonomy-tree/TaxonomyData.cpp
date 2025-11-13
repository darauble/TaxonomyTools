#include "TaxonomyData.hpp"
#include "ZipReader.hpp"
#include "TreeStrings.hpp"
#include <algorithm>
#include <set>

std::vector<long> TaxonNode::GetHierarchyChain(const std::map<long, TaxonNode>& taxonomyMap) const
{
    std::vector<long> chain;
    long currentId = entry.id;

    // Build chain from current node up to root
    while (currentId != -1)
    {
        chain.push_back(currentId);
        auto it = taxonomyMap.find(currentId);
        if (it != taxonomyMap.end())
            currentId = it->second.entry.parentId;
        else
            break;
    }

    // Reverse so root is first
    std::reverse(chain.begin(), chain.end());
    return chain;
}

wxString TaxonNode::GetDisplayName() const
{
    // Use scientific name as primary display name
    if (!entry.scientificName.IsEmpty())
        return entry.scientificName;

    // Fallback to rank-specific field
    if (!entry.genus.IsEmpty())
        return entry.genus;
    if (!entry.family.IsEmpty())
        return entry.family;
    if (!entry.order.IsEmpty())
        return entry.order;
    if (!entry.taxonClass.IsEmpty())
        return entry.taxonClass;
    if (!entry.phylum.IsEmpty())
        return entry.phylum;
    if (!entry.kingdom.IsEmpty())
        return entry.kingdom;

    return wxString::Format("Taxon %ld", entry.id);
}

TaxonomyData::TaxonomyData()
    : m_zipReader(std::make_unique<ZipReader>())
{
}

TaxonomyData::~TaxonomyData() = default;

bool TaxonomyData::LoadArchive(const wxString& archivePath, std::function<void(const wxString&, int)> progressCallback)
{
    auto updateProgress = [&](int percent, const wxString& message) {
        if (progressCallback)
            progressCallback(message, percent);
    };

    updateProgress(5, TR_TREE_FMT(TreeStringId::ProgressOpeningArchive, archivePath.ToStdString().c_str()));

    if (!m_zipReader->OpenArchive(archivePath))
    {
        updateProgress(0, TR_TREE(TreeStringId::ProgressFailedOpenArchive));
        return false;
    }

    auto fileList = m_zipReader->GetFileList();
    updateProgress(10, TR_TREE_FMT(TreeStringId::ProgressArchiveContainsFiles, static_cast<unsigned long long>(fileList.size())));

    updateProgress(30, TR_TREE(TreeStringId::ProgressLoadingTaxonomyData));
    if (!LoadTaxonomyData(progressCallback))
    {
        updateProgress(0, TR_TREE(TreeStringId::ProgressFailedLoadTaxonomyData));
        return false;
    }

    updateProgress(70, TR_TREE(TreeStringId::ProgressBuildingHierarchy));
    BuildHierarchy();

    updateProgress(80, TR_TREE(TreeStringId::ProgressScanningLanguageFiles));
    std::vector<wxString> languageFiles = m_zipReader->GetLanguageFiles();
    updateProgress(85, TR_TREE_FMT(TreeStringId::ProgressFoundLanguageFiles, static_cast<unsigned long long>(languageFiles.size())));

    m_availableLanguages.clear();
    for (const auto& file : languageFiles)
    {
        wxString language = CSVParser::ExtractLanguageFromFilename(file);
        if (!language.IsEmpty())
            m_availableLanguages.push_back(language);
    }

    updateProgress(100, TR_TREE_FMT(TreeStringId::ProgressArchiveLoaded,
                                    m_taxonomyMap.size(), m_availableLanguages.size()));

    return !m_availableLanguages.empty();
}

std::vector<wxString> TaxonomyData::GetAvailableLanguages() const
{
    return m_availableLanguages;
}

bool TaxonomyData::LoadVernacularData(const wxString& language, std::vector<VernacularEntry>& entries)
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

bool TaxonomyData::LoadTaxonomyData(std::function<void(const wxString&, int)> progressCallback)
{
    auto updateProgress = [&](int percent, const wxString& message) {
        if (progressCallback)
            progressCallback(message, percent);
    };

    updateProgress(35, TR_TREE(TreeStringId::ProgressReadingCSV));

    wxString taxonomyContent;
    if (!m_zipReader->ReadFileToString("taxa.csv", taxonomyContent))
    {
        updateProgress(40, TR_TREE(TreeStringId::ProgressFailedReadCSV));
        if (!m_zipReader->ReadFileToString("taxonomy.csv", taxonomyContent))
        {
            if (!m_zipReader->ReadFileToString("taxa.txt", taxonomyContent))
            {
                updateProgress(0, TR_TREE(TreeStringId::ProgressFailedReadCSV));
                return false;
            }
        }
    }

    updateProgress(50, TR_TREE_FMT(TreeStringId::ProgressFileRead, taxonomyContent.length()));

    // Parse into temporary map
    std::map<long, TaxonomyEntry> tempMap;
    CSVParser parser;
    if (!parser.ParseTaxonomyCSV(taxonomyContent, tempMap))
    {
        updateProgress(0, TR_TREE(TreeStringId::ProgressFailedParseCSV));
        return false;
    }

    updateProgress(65, TR_TREE_FMT(TreeStringId::ProgressParsedEntries, static_cast<unsigned long long>(tempMap.size())));

    // Convert to TaxonNode map
    m_taxonomyMap.clear();
    for (const auto& pair : tempMap)
    {
        TaxonNode node;
        node.entry = pair.second;
        m_taxonomyMap[pair.first] = node;
    }

    return true;
}

void TaxonomyData::BuildHierarchy()
{
    // Build parent-child relationships
    for (auto& pair : m_taxonomyMap)
    {
        TaxonNode& node = pair.second;
        long parentId = node.entry.parentId;

        if (parentId != -1)
        {
            auto parentIt = m_taxonomyMap.find(parentId);
            if (parentIt != m_taxonomyMap.end())
            {
                parentIt->second.children.push_back(node.entry.id);
            }
        }
    }
}

const TaxonNode* TaxonomyData::GetTaxonNode(long taxonId) const
{
    auto it = m_taxonomyMap.find(taxonId);
    if (it != m_taxonomyMap.end())
        return &it->second;
    return nullptr;
}

long TaxonomyData::FindCommonAncestor(const std::vector<long>& taxonIds) const
{
    if (taxonIds.empty())
        return -1;
    if (taxonIds.size() == 1)
        return taxonIds[0];

    // Get hierarchy chains for all taxa
    std::vector<std::vector<long>> chains;
    for (long taxonId : taxonIds)
    {
        chains.push_back(GetHierarchyChain(taxonId));
    }

    // Find common ancestor by comparing chains
    long commonAncestor = -1;
    size_t minChainLength = chains[0].size();
    for (const auto& chain : chains)
    {
        if (chain.size() < minChainLength)
            minChainLength = chain.size();
    }

    for (size_t i = 0; i < minChainLength; ++i)
    {
        long currentId = chains[0][i];
        bool allMatch = true;

        for (size_t j = 1; j < chains.size(); ++j)
        {
            if (chains[j][i] != currentId)
            {
                allMatch = false;
                break;
            }
        }

        if (allMatch)
            commonAncestor = currentId;
        else
            break;
    }

    return commonAncestor;
}

std::vector<long> TaxonomyData::GetHierarchyChain(long taxonId) const
{
    const TaxonNode* node = GetTaxonNode(taxonId);
    if (!node)
        return std::vector<long>();

    return node->GetHierarchyChain(m_taxonomyMap);
}
