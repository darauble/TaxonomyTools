#pragma once

#include <wx/wx.h>
#include <wx/dcsvg.h>
#include <vector>
#include "TaxonomyData.hpp"
#include "SearchIndex.hpp"

enum class TreeLayout
{
    Vertical,    // Top to bottom
    Horizontal,  // Left to right
    Fan          // Radial, top to left (90 degrees)
};

struct TreeRenderOptions
{
    TreeLayout layout = TreeLayout::Fan;
    bool showBoxes = true;
    int fontSize = 12;
    int nodeSpacing = 40;
    int levelSpacing = 100;
    wxColour lineColor = *wxBLACK;
    wxColour boxColor = wxColour(240, 240, 240);
    wxColour textColor = *wxBLACK;
};

class TreeRenderer
{
public:
    TreeRenderer();

    // Set the taxa to compare (build the tree from these)
    void SetCompareTaxa(const std::vector<long>& taxonIds,
                       const TaxonomyData& taxonomyData,
                       const SearchIndex& searchIndex,
                       bool showFullTree = false);

    // Render tree to a DC (can be screen DC, memory DC, or SVG DC)
    void Render(wxDC& dc, const TreeRenderOptions& options);

    // Export to SVG file
    bool ExportToSVG(const wxString& filename, const TreeRenderOptions& options);

    // Export to PNG file at specific DPI
    bool ExportToPNG(const wxString& filename, const TreeRenderOptions& options, int dpi);

    // Get required size for rendering
    wxSize GetRequiredSize(const TreeRenderOptions& options) const;

private:
    struct TreeNode
    {
        long taxonId;
        wxString displayName;        // Combined display name
        wxString vernacularName;     // Vernacular name only
        wxString scientificName;     // Scientific name only
        std::vector<TreeNode*> children;
        int depth;  // Depth in tree (0 = root)
        wxPoint position;  // Position for rendering
        TreeNode* parent;

        // For fan layout
        double angleStart;  // Start angle in radians
        double angleEnd;    // End angle in radians
        double radiusInner; // Inner radius
        double radiusOuter; // Outer radius

        TreeNode() : taxonId(-1), depth(0), parent(nullptr),
                     angleStart(0), angleEnd(0), radiusInner(0), radiusOuter(0) {}
    };

    void BuildTree(const std::vector<long>& taxonIds,
                  const TaxonomyData& taxonomyData,
                  const SearchIndex& searchIndex,
                  bool showFullTree);

    void CalculateLayout(const TreeRenderOptions& options);
    void CalculateVerticalLayout(const TreeRenderOptions& options);
    void CalculateHorizontalLayout(const TreeRenderOptions& options);
    void CalculateFanLayout(const TreeRenderOptions& options);

    void RenderNode(wxDC& dc, const TreeNode* node, const TreeRenderOptions& options);
    void RenderConnection(wxDC& dc, const TreeNode* parent, const TreeNode* child, const TreeRenderOptions& options);

    // Fan layout specific rendering
    void RenderFanNode(wxDC& dc, const TreeNode* node, const TreeRenderOptions& options);
    void RenderFanConnection(wxDC& dc, const TreeNode* parent, const TreeNode* child, const TreeRenderOptions& options);

    void DeleteTree(TreeNode* node);

    TreeNode* m_root;
    std::vector<TreeNode*> m_allNodes;
    wxSize m_requiredSize;

    // Fan layout parameters (calculated in CalculateFanLayout, used in rendering)
    int m_fanCenterX;
    int m_fanCenterY;
};
