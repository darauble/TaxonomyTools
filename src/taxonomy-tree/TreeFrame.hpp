#pragma once

#include <wx/wx.h>
#include <wx/listbox.h>
#include <wx/splitter.h>
#include <wx/scrolwin.h>
#include <memory>
#include "TaxonomyData.hpp"
#include "SearchIndex.hpp"
#include "TreeRenderer.hpp"

class TreeCanvas;

class TreeFrame : public wxFrame
{
public:
    TreeFrame(const wxString& title);
    ~TreeFrame();

private:
    void CreateMenuBar();
    void CreateControls();
    void SetupStatusBar();
    void SetApplicationIcon();

    // Event handlers
    void OnDownload(wxCommandEvent& event);
    void OnLoadArchive(wxCommandEvent& event);
    void OnExportSVG(wxCommandEvent& event);
    void OnExportPNG(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnSearchTextChanged(wxCommandEvent& event);
    void OnSearchResultDoubleClick(wxCommandEvent& event);
    void OnCompareItemDoubleClick(wxCommandEvent& event);

    void OnLayoutVertical(wxCommandEvent& event);
    void OnLayoutHorizontal(wxCommandEvent& event);
    void OnLayoutFan(wxCommandEvent& event);
    void OnToggleBoxes(wxCommandEvent& event);
    void OnToggleFullTree(wxCommandEvent& event);

    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnZoomReset(wxCommandEvent& event);

    void OnPrimaryLanguageChanged(wxCommandEvent& event);
    void OnSecondaryLanguageChanged(wxCommandEvent& event);
    void OnSearchTimer(wxTimerEvent& event);
    void OnClearCompareList(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    // Helper functions
    void UpdateSearchResults();
    void UpdateTree();
    void UpdateLanguageChoices();
    void LoadTaxonomyFile(const wxString& filePath);

    // UI Componentsz
    wxTextCtrl* m_searchBox;
    wxListBox* m_searchResults;
    wxListBox* m_compareList;
    TreeCanvas* m_treeCanvas;

    wxComboBox* m_primaryLanguageChoice;
    wxComboBox* m_secondaryLanguageChoice;

    wxMenuItem* m_layoutFanMenuItem;
    wxMenuItem* m_layoutVerticalMenuItem;
    wxMenuItem* m_layoutHorizontalMenuItem;
    wxMenuItem* m_showBoxesMenuItem;
    wxMenuItem* m_showFullTreeMenuItem;

    // Data
    std::unique_ptr<TaxonomyData> m_taxonomyData;
    std::unique_ptr<SearchIndex> m_searchIndex;
    std::unique_ptr<TreeRenderer> m_treeRenderer;

    TreeLayout m_currentLayout;
    bool m_showBoxes;
    bool m_showFullTree;

    std::vector<long> m_selectedTaxonIds;
    std::map<int, long> m_searchResultIndexToTaxonId;  // Map list index to taxon ID

    // Track last indexed languages to avoid redundant reindexing
    wxString m_lastPrimaryLanguage;
    wxString m_lastSecondaryLanguage;

    // Search delay timer for fast typing
    wxTimer* m_searchTimer;

    wxDECLARE_EVENT_TABLE();
};

// Custom canvas for tree rendering
class TreeCanvas : public wxScrolledWindow
{
public:
    TreeCanvas(wxWindow* parent);

    void SetRenderer(TreeRenderer* renderer);
    void SetRenderOptions(const TreeRenderOptions& options);
    void UpdateView();

    void ZoomIn();
    void ZoomOut();
    void ZoomReset();

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);

    TreeRenderer* m_renderer;
    TreeRenderOptions m_options;

    // Zoom and pan state
    double m_zoomFactor;
    bool m_dragging;
    wxPoint m_dragStart;
    int m_scrollStartX;
    int m_scrollStartY;

    wxDECLARE_EVENT_TABLE();
};

// Menu/Control IDs
enum
{
    ID_DOWNLOAD = wxID_HIGHEST + 1,
    ID_LOAD_ARCHIVE,
    ID_EXPORT_SVG,
    ID_EXPORT_PNG,
    ID_SETTINGS,
    ID_LAYOUT_VERTICAL,
    ID_LAYOUT_HORIZONTAL,
    ID_LAYOUT_FAN,
    ID_TOGGLE_BOXES,
    ID_TOGGLE_FULL_TREE,
    ID_ZOOM_IN,
    ID_ZOOM_OUT,
    ID_ZOOM_RESET,
    ID_SEARCH_BOX,
    ID_SEARCH_RESULTS,
    ID_COMPARE_LIST,
    ID_CLEAR_COMPARE_LIST,
    ID_PRIMARY_LANGUAGE,
    ID_SECONDARY_LANGUAGE,
    ID_SEARCH_TIMER
};
