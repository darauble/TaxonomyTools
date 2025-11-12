#include "TreeRenderer.hpp"
#include <wx/graphics.h>
#include <wx/dcmemory.h>
#include <wx/image.h>
#include <cmath>
#include <algorithm>
#include <functional>

TreeRenderer::TreeRenderer()
    : m_root(nullptr)
{
}

void TreeRenderer::SetCompareTaxa(const std::vector<long>& taxonIds,
                                  const TaxonomyData& taxonomyData,
                                  const SearchIndex& searchIndex,
                                  bool showFullTree)
{
    // Clean up existing tree
    if (m_root)
    {
        DeleteTree(m_root);
        m_root = nullptr;
    }
    m_allNodes.clear();

    if (taxonIds.empty())
        return;

    BuildTree(taxonIds, taxonomyData, searchIndex, showFullTree);
}

void TreeRenderer::BuildTree(const std::vector<long>& taxonIds,
                             const TaxonomyData& taxonomyData,
                             const SearchIndex& searchIndex,
                             bool showFullTree)
{
    // Get hierarchy chains for all selected taxa
    std::vector<std::vector<long>> chains;
    for (long taxonId : taxonIds)
    {
        chains.push_back(taxonomyData.GetHierarchyChain(taxonId));
    }

    if (chains.empty())
        return;

    // Determine root taxon ID
    long rootTaxonId;

    if (showFullTree)
    {
        // Show full tree to kingdom: use the root (kingdom) from the first chain
        if (!chains.empty() && !chains[0].empty())
            rootTaxonId = chains[0][0];  // First element is the kingdom
        else
            return;
    }
    else
    {
        // Find common ancestor (lowest divergence point)
        rootTaxonId = taxonomyData.FindCommonAncestor(taxonIds);
        if (rootTaxonId == -1)
        {
            // No common ancestor within kingdom - use first taxon's root as the common root
            if (!chains.empty() && !chains[0].empty())
                rootTaxonId = chains[0][0];
            else
                return;
        }
    }

    // Build tree structure starting from determined root
    std::map<long, TreeNode*> nodeMap;

    // Create root node
    m_root = new TreeNode();
    m_root->taxonId = rootTaxonId;
    m_root->displayName = searchIndex.GetDisplayName(rootTaxonId, true);
    m_root->vernacularName = searchIndex.GetVernacularName(rootTaxonId);
    m_root->scientificName = searchIndex.GetScientificName(rootTaxonId);
    m_root->depth = 0;
    m_allNodes.push_back(m_root);
    nodeMap[rootTaxonId] = m_root;

    // Build paths from root to each selected taxon
    for (const auto& chain : chains)
    {
        // Find where this chain starts relative to root taxon
        size_t startIdx = 0;
        for (size_t i = 0; i < chain.size(); ++i)
        {
            if (chain[i] == rootTaxonId)
            {
                startIdx = i + 1;
                break;
            }
        }

        // Build nodes along the chain
        TreeNode* currentParent = m_root;
        for (size_t i = startIdx; i < chain.size(); ++i)
        {
            long taxonId = chain[i];

            // Check if node already exists
            auto it = nodeMap.find(taxonId);
            if (it != nodeMap.end())
            {
                currentParent = it->second;
                continue;
            }

            // Create new node
            TreeNode* newNode = new TreeNode();
            newNode->taxonId = taxonId;
            newNode->displayName = searchIndex.GetDisplayName(taxonId, true);
            newNode->vernacularName = searchIndex.GetVernacularName(taxonId);
            newNode->scientificName = searchIndex.GetScientificName(taxonId);
            newNode->parent = currentParent;
            newNode->depth = currentParent->depth + 1;

            currentParent->children.push_back(newNode);
            m_allNodes.push_back(newNode);
            nodeMap[taxonId] = newNode;

            currentParent = newNode;
        }
    }
}

void TreeRenderer::DeleteTree(TreeNode* node)
{
    if (!node)
        return;

    for (TreeNode* child : node->children)
        DeleteTree(child);

    delete node;
}

void TreeRenderer::Render(wxDC& dc, const TreeRenderOptions& options)
{
    if (!m_root)
        return;

    // Calculate layout
    CalculateLayout(options);

    // Set font
    wxFont font(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);

    // Use different rendering for fan layout
    if (options.layout == TreeLayout::Fan)
    {
        // Render connections first (so they appear behind nodes)
        for (TreeNode* node : m_allNodes)
        {
            for (TreeNode* child : node->children)
            {
                RenderFanConnection(dc, node, child, options);
            }
        }

        // Render nodes
        for (TreeNode* node : m_allNodes)
        {
            RenderFanNode(dc, node, options);
        }
    }
    else
    {
        // Render connections first (so they appear behind nodes)
        for (TreeNode* node : m_allNodes)
        {
            for (TreeNode* child : node->children)
            {
                RenderConnection(dc, node, child, options);
            }
        }

        // Render nodes
        for (TreeNode* node : m_allNodes)
        {
            RenderNode(dc, node, options);
        }
    }
}

void TreeRenderer::CalculateLayout(const TreeRenderOptions& options)
{
    if (!m_root)
        return;

    switch (options.layout)
    {
        case TreeLayout::Vertical:
            CalculateVerticalLayout(options);
            break;
        case TreeLayout::Horizontal:
            CalculateHorizontalLayout(options);
            break;
        case TreeLayout::Fan:
            CalculateFanLayout(options);
            break;
    }
}

void TreeRenderer::CalculateVerticalLayout(const TreeRenderOptions& options)
{
    if (!m_root)
        return;

    // Create a temporary DC to measure text
    wxBitmap tempBitmap(1, 1);
    wxMemoryDC tempDC(tempBitmap);
    wxFont regularFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont italicFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);

    // Calculate actual box widths for each node (two lines: vernacular + scientific)
    std::map<TreeNode*, int> nodeWidths;
    for (TreeNode* node : m_allNodes)
    {
        wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
        wxString line2 = node->scientificName;

        tempDC.SetFont(regularFont);
        wxSize textSize1 = tempDC.GetTextExtent(line1);
        tempDC.SetFont(italicFont);
        wxSize textSize2 = tempDC.GetTextExtent(line2);

        int maxWidth = std::max(textSize1.GetWidth(), textSize2.GetWidth());
        nodeWidths[node] = maxWidth + 20;  // 20 = padding (10 on each side)
    }

    // Use proper tree layout algorithm with actual widths
    // Post-order traversal to assign x-coordinates based on subtree width
    double nextX = 50.0;
    std::function<double(TreeNode*)> calculateSubtreeLayout = [&](TreeNode* node) -> double {
        if (!node)
            return 0.0;

        if (node->children.empty())
        {
            // Leaf node: assign next available x position (center of box)
            node->position.x = nextX + nodeWidths[node] / 2.0;
            nextX += nodeWidths[node] + options.nodeSpacing;
            return nodeWidths[node] + options.nodeSpacing;
        }

        // Internal node: position based on children
        double subtreeWidth = 0.0;

        for (TreeNode* child : node->children)
        {
            subtreeWidth += calculateSubtreeLayout(child);
        }

        // Position this node at the center of its children
        if (!node->children.empty())
        {
            double childrenLeft = node->children.front()->position.x;
            double childrenRight = node->children.back()->position.x;
            node->position.x = (childrenLeft + childrenRight) / 2.0;
        }

        return subtreeWidth;
    };

    calculateSubtreeLayout(m_root);

    // Second pass: assign y-coordinates based on depth
    int maxDepth = 0;
    for (TreeNode* node : m_allNodes)
    {
        node->position.y = 50 + node->depth * options.levelSpacing;
        if (node->depth > maxDepth)
            maxDepth = node->depth;
    }

    // Calculate required size
    int maxX = 0;
    for (TreeNode* node : m_allNodes)
    {
        int rightEdge = node->position.x + nodeWidths[node] / 2 + 50;
        if (rightEdge > maxX)
            maxX = rightEdge;
    }

    m_requiredSize = wxSize(maxX + 100, (maxDepth + 1) * options.levelSpacing + 100);
}

void TreeRenderer::CalculateHorizontalLayout(const TreeRenderOptions& options)
{
    if (!m_root)
        return;

    // Create a temporary DC to measure text
    wxBitmap tempBitmap(1, 1);
    wxMemoryDC tempDC(tempBitmap);
    wxFont regularFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont italicFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);

    // Calculate actual box heights for each node (two lines: vernacular + scientific)
    std::map<TreeNode*, int> nodeHeights;
    for (TreeNode* node : m_allNodes)
    {
        wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
        wxString line2 = node->scientificName;

        tempDC.SetFont(regularFont);
        wxSize textSize1 = tempDC.GetTextExtent(line1);
        tempDC.SetFont(italicFont);
        wxSize textSize2 = tempDC.GetTextExtent(line2);

        int lineSpacing = 2;
        int totalHeight = textSize1.GetHeight() + lineSpacing + textSize2.GetHeight();
        nodeHeights[node] = totalHeight + 10;  // 10 = padding (5 on each side)
    }

    // Use proper tree layout algorithm with actual heights
    // Post-order traversal to assign y-coordinates based on subtree height
    double nextY = 50.0;
    std::function<double(TreeNode*)> calculateSubtreeLayout = [&](TreeNode* node) -> double {
        if (!node)
            return 0.0;

        if (node->children.empty())
        {
            // Leaf node: assign next available y position (center of box)
            node->position.y = nextY + nodeHeights[node] / 2.0;
            nextY += nodeHeights[node] + options.nodeSpacing;
            return nodeHeights[node] + options.nodeSpacing;
        }

        // Internal node: position based on children
        double subtreeHeight = 0.0;

        for (TreeNode* child : node->children)
        {
            subtreeHeight += calculateSubtreeLayout(child);
        }

        // Position this node at the center of its children
        if (!node->children.empty())
        {
            double childrenTop = node->children.front()->position.y;
            double childrenBottom = node->children.back()->position.y;
            node->position.y = (childrenTop + childrenBottom) / 2.0;
        }

        return subtreeHeight;
    };

    calculateSubtreeLayout(m_root);

    // Second pass: assign x-coordinates based on depth
    // Calculate maximum box width at each level to prevent overlaps
    std::map<int, int> maxWidthAtLevel;
    for (TreeNode* node : m_allNodes)
    {
        wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
        wxString line2 = node->scientificName;

        tempDC.SetFont(regularFont);
        wxSize textSize1 = tempDC.GetTextExtent(line1);
        tempDC.SetFont(italicFont);
        wxSize textSize2 = tempDC.GetTextExtent(line2);

        int maxWidth = std::max(textSize1.GetWidth(), textSize2.GetWidth());
        int boxWidth = maxWidth + 40;
        if (boxWidth > maxWidthAtLevel[node->depth])
            maxWidthAtLevel[node->depth] = boxWidth;
    }

    // Position nodes with cumulative spacing to account for box widths
    int maxDepth = 0;
    int xPos = 50;
    for (int depth = 0; depth <= (int)m_allNodes.size(); depth++)
    {
        if (maxWidthAtLevel.find(depth) == maxWidthAtLevel.end())
            break;

        for (TreeNode* node : m_allNodes)
        {
            if (node->depth == depth)
            {
                node->position.x = xPos + maxWidthAtLevel[depth] / 2;
                if (node->depth > maxDepth)
                    maxDepth = node->depth;
            }
        }
        xPos += maxWidthAtLevel[depth] + options.levelSpacing;
    }

    // Calculate required size by finding actual extent of all nodes
    int maxX = 0;
    int maxY = 0;
    for (TreeNode* node : m_allNodes)
    {
        // Calculate the width of this node's box
        wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
        wxString line2 = node->scientificName;

        tempDC.SetFont(regularFont);
        wxSize textSize1 = tempDC.GetTextExtent(line1);
        tempDC.SetFont(italicFont);
        wxSize textSize2 = tempDC.GetTextExtent(line2);

        int maxWidth = std::max(textSize1.GetWidth(), textSize2.GetWidth());
        int boxWidth = maxWidth + 40;

        int rightEdge = node->position.x + boxWidth / 2 + 50;
        if (rightEdge > maxX)
            maxX = rightEdge;

        int bottomEdge = node->position.y + nodeHeights[node] / 2 + 50;
        if (bottomEdge > maxY)
            maxY = bottomEdge;
    }

    m_requiredSize = wxSize(maxX + 100, maxY + 100);
}

void TreeRenderer::CalculateFanLayout(const TreeRenderOptions& options)
{
    // Fan layout: quarter circle with center at top-left
    // Angles from -90° (up) to 0° (right)
    // Boxes are circular segments bounded by two radial lines and two arcs

    if (!m_root)
        return;

    // Create a temporary DC to measure text
    wxBitmap tempBitmap(1, 1);
    wxMemoryDC tempDC(tempBitmap);
    wxFont font(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    tempDC.SetFont(font);

    // Calculate total leaf count to determine angle allocation
    std::function<int(TreeNode*)> countLeaves = [&](TreeNode* node) -> int {
        if (!node)
            return 0;
        if (node->children.empty())
            return 1;
        int count = 0;
        for (TreeNode* child : node->children)
            count += countLeaves(child);
        return count;
    };

    int totalLeaves = countLeaves(m_root);

    // Fan parameters: 0° (right/east) to 90° (down/south) - quarter circle
    // In standard math coords: 0° = right, 90° = down, 180° = left, 270° = up
    const double startAngleDeg = 0.0;    // Right (0 degrees)
    const double endAngleDeg = 90.0;     // Down (90 degrees)
    const double startAngle = startAngleDeg * M_PI / 180.0;
    const double endAngle = endAngleDeg * M_PI / 180.0;
    const double fullAngleRange = endAngle - startAngle;  // 90 degrees

    // Spacing between sibling boxes: 5% of full 90 degree arc
    const double spacingAngle = fullAngleRange * 0.05;

    // Determine actual angle range based on leaf count
    double actualAngleRange;
    if (totalLeaves < 6)
    {
        // Use proportional spacing for fewer leaves
        actualAngleRange = fullAngleRange * totalLeaves / 6.0;
    }
    else
    {
        // Use full 90 degrees for 6 or more leaves
        actualAngleRange = fullAngleRange;
    }

    const double actualEndAngle = startAngle + actualAngleRange;

    // Spacer width in points (30pt between levels)
    const double spacerWidth = 30.0;

    // Initial spacer before root node - large enough for text at inner radius
    const double initialSpacerRadius = 200.0;

    // Center Y position (fixed)
    const int centerY = 100;

    // Center X will be calculated dynamically after we know the total tree width
    // (calculated later after angle allocation)

    // Calculate maximum depth
    int maxDepth = 0;
    for (TreeNode* node : m_allNodes)
    {
        if (node->depth > maxDepth)
            maxDepth = node->depth;
    }

    // Step 1: Find maximum text width (for readable horizontal text) and count nodes at each depth
    std::map<int, double> maxTextWidthAtDepth;
    std::map<int, int> nodeCountAtDepth;

    for (TreeNode* node : m_allNodes)
    {
        // Measure both lines of text
        wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
        wxString line2 = node->scientificName;

        wxSize textSize1 = tempDC.GetTextExtent(line1);
        wxSize textSize2 = tempDC.GetTextExtent(line2);

        // Find the wider of the two lines (text is displayed horizontally/readably)
        double nodeMaxWidth = std::max(textSize1.GetWidth(), textSize2.GetWidth());

        // Track maximum text width at this depth
        if (nodeMaxWidth > maxTextWidthAtDepth[node->depth])
            maxTextWidthAtDepth[node->depth] = nodeMaxWidth;

        // Count nodes at this depth
        nodeCountAtDepth[node->depth]++;
    }

    // Step 2: Calculate radius for each depth level based on text width
    // When text is rotated along radial lines, the TEXT WIDTH becomes the radial extent
    std::map<int, std::pair<double, double>> depthRadii;  // depth -> (innerRadius, outerRadius)

    double currentRadius = initialSpacerRadius;
    for (int depth = 0; depth <= maxDepth; depth++)
    {
        double innerRadius = currentRadius;

        // Radial thickness = maximum text width at this depth + margins
        // Text is rotated, so width extends radially
        double textWidth = maxTextWidthAtDepth[depth];
        double radialThickness = textWidth + 20.0;  // Add 20pt margin (10pt on each side)

        double outerRadius = currentRadius + radialThickness;
        depthRadii[depth] = {innerRadius, outerRadius};
        currentRadius = outerRadius + spacerWidth;  // Add spacer for next level
    }

    // Step 3: Calculate angular size bottom-up
    // Leaf nodes get equal size, parent nodes get sum of children's sizes
    std::map<TreeNode*, double> nodeAngles;  // Assigned angle for each node

    // For leaf nodes (outermost circle), divide available angle equally
    // Use totalLeaves (all nodes with no children) to handle mixed-depth comparisons
    // This treats branches ending early as "virtual leaves" at maxDepth
    int numLeaves = totalLeaves;
    double totalSpacingAtDeepest = spacingAngle * (numLeaves - 1);
    double availableAngleAtDeepest = actualEndAngle - startAngle - totalSpacingAtDeepest;
    double anglePerLeaf = availableAngleAtDeepest / numLeaves;

    // Helper: recursively calculate angles from leaves up to root
    std::function<double(TreeNode*)> calculateNodeAngle;
    calculateNodeAngle = [&](TreeNode* node) -> double {
        if (!node)
            return 0.0;

        // If it's a leaf node, use the equal distribution
        if (node->children.empty())
        {
            double angle = anglePerLeaf;
            nodeAngles[node] = angle;
            return angle;
        }

        // For parent nodes, calculate based on children
        double totalChildAngle = 0.0;
        for (TreeNode* child : node->children)
        {
            totalChildAngle += calculateNodeAngle(child);
        }

        // Add spacing between children
        double totalSpacing = spacingAngle * (node->children.size() - 1);
        double nodeAngle = totalChildAngle + totalSpacing;

        nodeAngles[node] = nodeAngle;
        return nodeAngle;
    };

    // Calculate angles for the entire tree
    calculateNodeAngle(m_root);

    // Step 6: Recursively assign angle ranges to nodes
    // All boxes at same depth have equal size, parent boxes are centered over their children
    std::function<void(TreeNode*, double, double)> assignAngles;
    assignAngles = [&](TreeNode* node, double angleStart, double angleEnd) {
        if (!node)
            return;

        // Set this node's angle range (passed from parent or root)
        node->angleStart = angleStart;
        node->angleEnd = angleEnd;

        // Set radii based on depth
        auto radii = depthRadii[node->depth];
        node->radiusInner = radii.first;
        node->radiusOuter = radii.second;

        // Position will be calculated later after centerX is determined

        if (node->children.empty())
            return;

        // Calculate total span needed for all children
        double totalChildAngle = 0.0;
        for (TreeNode* child : node->children)
            totalChildAngle += nodeAngles[child];

        double totalSpacing = spacingAngle * (node->children.size() - 1);
        double childrenTotalSpan = totalChildAngle + totalSpacing;

        // Center children within parent's angle range
        double parentAngle = angleEnd - angleStart;
        double childrenStartOffset = (parentAngle - childrenTotalSpan) / 2.0;

        // Start assigning children from the centered position
        // IMPORTANT: Clamp to never go below the global startAngle (0 degrees)
        double currentAngle = angleStart + childrenStartOffset;
        if (currentAngle < startAngle)
            currentAngle = startAngle;

        for (TreeNode* child : node->children)
        {
            double childAngle = nodeAngles[child];
            assignAngles(child, currentAngle, currentAngle + childAngle);
            currentAngle += childAngle + spacingAngle;
        }
    };

    // Assign root with its calculated angle
    double rootAngle = nodeAngles[m_root];
    assignAngles(m_root, startAngle, startAngle + rootAngle);

    // Step 7: Set fan center at top-left corner with margin
    // Simple fixed position - no dynamic calculation
    m_fanCenterX = 100;  // Left margin
    m_fanCenterY = centerY;  // Top margin

    double maxRadius = depthRadii[maxDepth].second;

    // Now update all node positions with the calculated centerX
    for (TreeNode* node : m_allNodes)
    {
        double centerAngle = (node->angleStart + node->angleEnd) / 2.0;
        double centerRadius = (node->radiusInner + node->radiusOuter) / 2.0;
        node->position = wxPoint(
            m_fanCenterX + centerRadius * std::cos(centerAngle),
            m_fanCenterY + centerRadius * std::sin(centerAngle)
        );
    }

    // Calculate required size based on maximum radius and actual rendering extent
    // Find rightmost point (at 0 degrees) and bottommost point (at 90 degrees)
    int rightmostX = m_fanCenterX + static_cast<int>(maxRadius);
    int bottommostY = m_fanCenterY + static_cast<int>(maxRadius);

    m_requiredSize = wxSize(rightmostX + 100, bottommostY + 100);
}

void TreeRenderer::RenderNode(wxDC& dc, const TreeNode* node, const TreeRenderOptions& options)
{
    if (!node)
        return;

    // Prepare two lines of text
    wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
    wxString line2 = node->scientificName;

    // Regular font for vernacular name
    wxFont regularFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    // Italic font for scientific name
    wxFont italicFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);

    // Measure both lines
    dc.SetFont(regularFont);
    wxSize textSize1 = dc.GetTextExtent(line1);
    dc.SetFont(italicFont);
    wxSize textSize2 = dc.GetTextExtent(line2);

    // Calculate box size
    int boxWidth = std::max(textSize1.GetWidth(), textSize2.GetWidth()) + 20;
    int lineSpacing = 2;
    int boxHeight = textSize1.GetHeight() + lineSpacing + textSize2.GetHeight() + 10;

    int x = node->position.x - boxWidth / 2;
    int y = node->position.y - boxHeight / 2;

    if (options.showBoxes)
    {
        dc.SetBrush(wxBrush(options.boxColor));
        dc.SetPen(wxPen(options.lineColor, 1));
        dc.DrawRoundedRectangle(x, y, boxWidth, boxHeight, 5);
    }
    else
    {
        // Draw white background behind text to prevent line intersection
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(x, y, boxWidth, boxHeight);
    }

    dc.SetTextForeground(options.textColor);

    // Draw line 1 (vernacular name or scientific name if no vernacular) - regular font
    dc.SetFont(regularFont);
    dc.DrawText(line1, x + 10, y + 5);

    // Draw line 2 (scientific name) - italic font
    dc.SetFont(italicFont);
    dc.DrawText(line2, x + 10, y + 5 + textSize1.GetHeight() + lineSpacing);
}

void TreeRenderer::RenderConnection(wxDC& dc, const TreeNode* parent, const TreeNode* child, const TreeRenderOptions& options)
{
    if (!parent || !child)
        return;

    dc.SetPen(wxPen(options.lineColor, 2));
    dc.DrawLine(parent->position, child->position);
}

void TreeRenderer::RenderFanNode(wxDC& dc, const TreeNode* node, const TreeRenderOptions& options)
{
    if (!node)
        return;

    // Use center point calculated in CalculateFanLayout
    const int centerX = m_fanCenterX;
    const int centerY = m_fanCenterY;

    wxFont regularFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont italicFont(options.fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);

    // Draw the circular segment (box)
    if (options.showBoxes)
    {
        dc.SetBrush(wxBrush(options.boxColor));
        dc.SetPen(wxPen(options.lineColor, 1));
    }
    else
    {
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.SetPen(*wxTRANSPARENT_PEN);
    }

    // Draw the circular segment using arcs and lines
    wxGraphicsContext* gc = dc.GetGraphicsContext();
    if (gc)
    {
        // Create path for the circular segment
        wxGraphicsPath path = gc->CreatePath();

        // Start at inner arc, angleStart
        double x1 = centerX + node->radiusInner * std::cos(node->angleStart);
        double y1 = centerY + node->radiusInner * std::sin(node->angleStart);
        path.MoveToPoint(x1, y1);

        // Arc along inner radius from angleStart to angleEnd
        path.AddArc(centerX, centerY, node->radiusInner,
                    node->angleStart, node->angleEnd, true);

        // Radial line from inner to outer at angleEnd
        double x2 = centerX + node->radiusOuter * std::cos(node->angleEnd);
        double y2 = centerY + node->radiusOuter * std::sin(node->angleEnd);
        path.AddLineToPoint(x2, y2);

        // Arc along outer radius from angleEnd to angleStart (backwards)
        path.AddArc(centerX, centerY, node->radiusOuter,
                    node->angleEnd, node->angleStart, false);

        // Close path (radial line back to start)
        path.CloseSubpath();

        // Fill and stroke
        if (options.showBoxes)
        {
            gc->SetBrush(wxBrush(options.boxColor));
            gc->SetPen(wxPen(options.lineColor, 1));
        }
        else
        {
            gc->SetBrush(*wxWHITE_BRUSH);
            gc->SetPen(*wxTRANSPARENT_PEN);
        }
        gc->FillPath(path);
        if (options.showBoxes)
            gc->StrokePath(path);
    }

    // Draw text along the radial direction
    dc.SetTextForeground(options.textColor);

    // Calculate center angle and radius for text
    double centerAngle = (node->angleStart + node->angleEnd) / 2.0;
    double textRadius = (node->radiusInner + node->radiusOuter) / 2.0;

    // Calculate rotation angle for text (convert to degrees)
    double textAngleDeg = centerAngle * 180.0 / M_PI;

    // Adjust text angle so it's always readable (not upside down)
    if (textAngleDeg > 90 && textAngleDeg < 270)
        textAngleDeg += 180;

    // Position for text (slightly inward from center)
    double textX = centerX + textRadius * std::cos(centerAngle);
    double textY = centerY + textRadius * std::sin(centerAngle);

    if (gc)
    {
        // Prepare two lines of text
        wxString line1 = node->vernacularName.IsEmpty() ? node->scientificName : node->vernacularName;
        wxString line2 = node->scientificName;

        // Measure both lines with appropriate fonts
        double width1, height1, width2, height2;
        gc->SetFont(regularFont, options.textColor);
        gc->GetTextExtent(line1, &width1, &height1);
        gc->SetFont(italicFont, options.textColor);
        gc->GetTextExtent(line2, &width2, &height2);

        double lineSpacing = 2.0;  // Small spacing between lines
        double totalHeight = height1 + lineSpacing + height2;

        // Save current transform
        gc->PushState();

        // Translate to text position and rotate
        gc->Translate(textX, textY);
        gc->Rotate(textAngleDeg * M_PI / 180.0);

        // Draw line 1 (vernacular name) above center - regular font
        gc->SetFont(regularFont, options.textColor);
        gc->DrawText(line1, -width1 / 2.0, -totalHeight / 2.0);

        // Draw line 2 (scientific name) below center - italic font
        gc->SetFont(italicFont, options.textColor);
        gc->DrawText(line2, -width2 / 2.0, -totalHeight / 2.0 + height1 + lineSpacing);

        gc->PopState();
    }
    else
    {
        // Fallback: draw single line horizontally at position
        dc.SetFont(regularFont);
        wxSize textSize = dc.GetTextExtent(node->displayName);
        dc.DrawText(node->displayName, textX - textSize.GetWidth() / 2, textY - textSize.GetHeight() / 2);
    }
}

void TreeRenderer::RenderFanConnection(wxDC& dc, const TreeNode* parent, const TreeNode* child, const TreeRenderOptions& options)
{
    if (!parent || !child)
        return;

    // Use center point calculated in CalculateFanLayout
    const int centerX = m_fanCenterX;
    const int centerY = m_fanCenterY;

    // Parent outer radius (end of parent box)
    double parentRadius = parent->radiusOuter;

    // Child inner radius (start of child box)
    double childRadius = child->radiusInner;

    // Middle of spacer circle
    double spacerRadius = (parentRadius + childRadius) / 2.0;

    // Parent and child center angles
    double parentAngle = (parent->angleStart + parent->angleEnd) / 2.0;
    double childAngle = (child->angleStart + child->angleEnd) / 2.0;

    dc.SetPen(wxPen(options.lineColor, 2));

    wxGraphicsContext* gc = dc.GetGraphicsContext();
    if (gc)
    {
        gc->SetPen(wxPen(options.lineColor, 2));

        // Draw radial line from parent to spacer
        double x1 = centerX + parentRadius * std::cos(parentAngle);
        double y1 = centerY + parentRadius * std::sin(parentAngle);
        double x2 = centerX + spacerRadius * std::cos(parentAngle);
        double y2 = centerY + spacerRadius * std::sin(parentAngle);

        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(x1, y1);
        path.AddLineToPoint(x2, y2);

        // Draw arc in spacer circle from parent angle to child angle
        path.AddArc(centerX, centerY, spacerRadius,
                    parentAngle, childAngle, childAngle > parentAngle);

        // Draw radial line from spacer to child
        double x4 = centerX + childRadius * std::cos(childAngle);
        double y4 = centerY + childRadius * std::sin(childAngle);
        path.AddLineToPoint(x4, y4);

        gc->StrokePath(path);
    }
    else
    {
        // Fallback: simple line from parent to child position
        dc.DrawLine(parent->position, child->position);
    }
}

wxSize TreeRenderer::GetRequiredSize(const TreeRenderOptions& options) const
{
    return m_requiredSize;
}

bool TreeRenderer::ExportToSVG(const wxString& filename, const TreeRenderOptions& options)
{
    if (!m_root)
        return false;

    // Calculate layout first
    const_cast<TreeRenderer*>(this)->CalculateLayout(options);

    wxSize size = GetRequiredSize(options);

    // For Fan layout, wxSVGFileDC doesn't support wxGraphicsContext features properly
    // (arcs, curved paths, rotated text). Export as PNG instead and warn user.
    if (options.layout == TreeLayout::Fan)
    {
        // Change extension to .png and export as high-quality PNG
        wxString pngFilename = filename;
        if (pngFilename.EndsWith(".svg"))
            pngFilename = pngFilename.BeforeLast('.') + ".png";

        // Export at 300 DPI for publication quality
        bool success = ExportToPNG(pngFilename, options, 300);

        if (success)
        {
            // Inform user that PNG was created instead
            wxLogWarning("Fan layout cannot be exported to SVG format due to wxWidgets limitations.\n"
                        "A high-resolution PNG (300 DPI) has been saved instead: %s", pngFilename);
        }

        return success;
    }

    wxSVGFileDC svgDC(filename, size.GetWidth(), size.GetHeight());
    Render(svgDC, options);

    return true;
}

bool TreeRenderer::ExportToPNG(const wxString& filename, const TreeRenderOptions& options, int dpi)
{
    if (!m_root)
        return false;

    // Calculate layout first
    const_cast<TreeRenderer*>(this)->CalculateLayout(options);

    wxSize size = GetRequiredSize(options);

    // Scale size based on DPI (assuming 96 DPI as base)
    double scale = dpi / 96.0;
    int scaledWidth = size.GetWidth() * scale;
    int scaledHeight = size.GetHeight() * scale;

    // Create bitmap
    wxBitmap bitmap(scaledWidth, scaledHeight);
    wxMemoryDC memDC(bitmap);

    // Fill background with white
    memDC.SetBackground(*wxWHITE_BRUSH);
    memDC.Clear();

    // Scale the DC
    memDC.SetUserScale(scale, scale);

    // Render
    Render(memDC, options);

    memDC.SelectObject(wxNullBitmap);

    // Save as PNG
    return bitmap.SaveFile(filename, wxBITMAP_TYPE_PNG);
}
