#include "TreeFrame.hpp"
#include "Downloader.hpp"
#include "SettingsDialog.hpp"
#include "TreeStrings.hpp"
#include "Version.hpp"
#include "icon_data.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/iconbndl.h>
#include <wx/mstream.h>
#include <wx/busyinfo.h>
#include <wx/display.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(TreeFrame, wxFrame)
    EVT_CLOSE(TreeFrame::OnClose)
    EVT_MENU(ID_DOWNLOAD, TreeFrame::OnDownload)
    EVT_MENU(ID_LOAD_ARCHIVE, TreeFrame::OnLoadArchive)
    EVT_MENU(ID_EXPORT_SVG, TreeFrame::OnExportSVG)
    EVT_MENU(ID_EXPORT_PNG, TreeFrame::OnExportPNG)
    EVT_MENU(ID_SETTINGS, TreeFrame::OnSettings)
    EVT_MENU(wxID_EXIT, TreeFrame::OnExit)
    EVT_MENU(wxID_ABOUT, TreeFrame::OnAbout)
    EVT_MENU(ID_LAYOUT_VERTICAL, TreeFrame::OnLayoutVertical)
    EVT_MENU(ID_LAYOUT_HORIZONTAL, TreeFrame::OnLayoutHorizontal)
    EVT_MENU(ID_LAYOUT_FAN, TreeFrame::OnLayoutFan)
    EVT_MENU(ID_TOGGLE_BOXES, TreeFrame::OnToggleBoxes)
    EVT_MENU(ID_TOGGLE_FULL_TREE, TreeFrame::OnToggleFullTree)
    EVT_MENU(ID_ZOOM_IN, TreeFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT, TreeFrame::OnZoomOut)
    EVT_MENU(ID_ZOOM_RESET, TreeFrame::OnZoomReset)
    EVT_TEXT(ID_SEARCH_BOX, TreeFrame::OnSearchTextChanged)
    EVT_LISTBOX_DCLICK(ID_SEARCH_RESULTS, TreeFrame::OnSearchResultDoubleClick)
    EVT_LISTBOX_DCLICK(ID_COMPARE_LIST, TreeFrame::OnCompareItemDoubleClick)
    EVT_BUTTON(ID_CLEAR_COMPARE_LIST, TreeFrame::OnClearCompareList)
    EVT_COMBOBOX(ID_PRIMARY_LANGUAGE, TreeFrame::OnPrimaryLanguageChanged)
    EVT_COMBOBOX(ID_SECONDARY_LANGUAGE, TreeFrame::OnSecondaryLanguageChanged)
    EVT_TEXT_ENTER(ID_PRIMARY_LANGUAGE, TreeFrame::OnPrimaryLanguageChanged)
    EVT_TEXT_ENTER(ID_SECONDARY_LANGUAGE, TreeFrame::OnSecondaryLanguageChanged)
    EVT_TIMER(ID_SEARCH_TIMER, TreeFrame::OnSearchTimer)
wxEND_EVENT_TABLE()

TreeFrame::TreeFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, TR_TREE(TreeStringId::AppTitle), wxDefaultPosition, wxSize(1200, 800))
    , m_currentLayout(TreeLayout::Fan)
    , m_showBoxes(true)
    , m_showFullTree(false)
    , m_searchTimer(nullptr)
{
    m_taxonomyData = std::make_unique<TaxonomyData>();
    m_searchIndex = std::make_unique<SearchIndex>();
    m_treeRenderer = std::make_unique<TreeRenderer>();

    m_searchTimer = new wxTimer(this, ID_SEARCH_TIMER);

    CreateMenuBar();
    CreateControls();
    SetupStatusBar();
    SetApplicationIcon();

    // Load saved window position and size
    wxConfig config("TaxonomyTree");
    int x = config.Read("/Window/X", -1);
    int y = config.Read("/Window/Y", -1);
    int width = config.Read("/Window/Width", 1200);
    int height = config.Read("/Window/Height", 800);
    bool maximized = config.Read("/Window/Maximized", 0L) != 0;

    // Set size
    SetSize(width, height);

    // Set position if saved (otherwise center)
    if (x >= 0 && y >= 0)
    {
        // Check if position is visible on any monitor
        wxRect windowRect(x, y, width, height);
        bool isVisible = false;

        // Check all displays
        for (unsigned int i = 0; i < wxDisplay::GetCount(); i++)
        {
            wxDisplay display(i);
            wxRect displayRect = display.GetClientArea();

            // Check if at least part of the window would be visible on this display
            // (at least 100x100 pixels should be visible to be usable)
            if (displayRect.Intersects(windowRect))
            {
                wxRect intersection = displayRect.Intersect(windowRect);
                if (intersection.GetWidth() >= 100 && intersection.GetHeight() >= 100)
                {
                    isVisible = true;
                    break;
                }
            }
        }

        if (isVisible)
            SetPosition(wxPoint(x, y));
        else
            Centre();
    }
    else
    {
        Centre();
    }

    // Restore maximized state
    if (maximized)
        Maximize();

    // Auto-load taxonomy file if enabled in settings
    wxConfig autoLoadConfig("TaxonomyTree");
    bool loadOnStartup = autoLoadConfig.Read("/Taxonomy/LoadOnStartup", 0L) != 0;
    wxString taxonomyFilePath = autoLoadConfig.Read("/Taxonomy/FilePath", wxEmptyString);

    if (loadOnStartup && !taxonomyFilePath.IsEmpty() && wxFileExists(taxonomyFilePath))
    {
        // Use CallAfter to load after the window is fully displayed
        CallAfter(&TreeFrame::LoadTaxonomyFile, taxonomyFilePath);
    }
}

TreeFrame::~TreeFrame()
{
    // Save window position and size
    wxConfig config("TaxonomyTree");

    bool isMaximized = IsMaximized();
    config.Write("/Window/Maximized", isMaximized);

    if (!isMaximized)
    {
        wxPoint pos = GetPosition();
        wxSize size = GetSize();

        config.Write("/Window/X", pos.x);
        config.Write("/Window/Y", pos.y);
        config.Write("/Window/Width", size.GetWidth());
        config.Write("/Window/Height", size.GetHeight());
    }

    // Flush to ensure data is written to disk
    config.Flush();
}

void TreeFrame::SetApplicationIcon()
{
#ifdef __WXMSW__
    // On Windows, the icon is automatically loaded from the resource file
    SetIcon(wxIcon("IDI_ICON1"));
#else
    // On Linux, load the PNG icon from embedded data
    wxMemoryInputStream stream(taxonomy_tree_icon_png, taxonomy_tree_icon_png_len);
    wxImage image(stream, wxBITMAP_TYPE_PNG);

    if (image.IsOk())
    {
        // Scale to appropriate icon size
        image = image.Scale(48, 48, wxIMAGE_QUALITY_HIGH);
        wxIcon icon;
        icon.CopyFromBitmap(wxBitmap(image));
        SetIcon(icon);
    }
#endif
}

void TreeFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    // File menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_DOWNLOAD, TR_TREE(TreeStringId::MenuFileDownload), TR_TREE(TreeStringId::MenuFileDownloadHelp));
    fileMenu->Append(ID_LOAD_ARCHIVE, TR_TREE(TreeStringId::MenuFileLoadArchive), TR_TREE(TreeStringId::MenuFileLoadArchiveHelp));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_EXPORT_SVG, TR_TREE(TreeStringId::MenuFileExportSVG), TR_TREE(TreeStringId::MenuFileExportSVGHelp));
    fileMenu->Append(ID_EXPORT_PNG, TR_TREE(TreeStringId::MenuFileExportPNG), TR_TREE(TreeStringId::MenuFileExportPNGHelp));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_SETTINGS, TR_TREE(TreeStringId::MenuFileSettings), TR_TREE(TreeStringId::MenuFileSettingsHelp));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, TR_TREE(TreeStringId::MenuFileExit), TR_TREE(TreeStringId::MenuFileExitHelp));
    menuBar->Append(fileMenu, TR_TREE(TreeStringId::MenuFile));

    // View menu - use check items instead of radio items for manual control
    wxMenu* viewMenu = new wxMenu();
    m_layoutFanMenuItem = viewMenu->AppendCheckItem(ID_LAYOUT_FAN, TR_TREE(TreeStringId::MenuViewFanLayout), TR_TREE(TreeStringId::MenuViewFanLayoutHelp));
    m_layoutVerticalMenuItem = viewMenu->AppendCheckItem(ID_LAYOUT_VERTICAL, TR_TREE(TreeStringId::MenuViewVerticalLayout), TR_TREE(TreeStringId::MenuViewVerticalLayoutHelp));
    m_layoutHorizontalMenuItem = viewMenu->AppendCheckItem(ID_LAYOUT_HORIZONTAL, TR_TREE(TreeStringId::MenuViewHorizontalLayout), TR_TREE(TreeStringId::MenuViewHorizontalLayoutHelp));
    viewMenu->AppendSeparator();
    m_showBoxesMenuItem = viewMenu->AppendCheckItem(ID_TOGGLE_BOXES, TR_TREE(TreeStringId::MenuViewShowBoxes), TR_TREE(TreeStringId::MenuViewShowBoxesHelp));
    m_showBoxesMenuItem->Check(m_showBoxes);
    m_showFullTreeMenuItem = viewMenu->AppendCheckItem(ID_TOGGLE_FULL_TREE, TR_TREE(TreeStringId::MenuViewShowFullTree), TR_TREE(TreeStringId::MenuViewShowFullTreeHelp));
    m_showFullTreeMenuItem->Check(m_showFullTree);
    viewMenu->AppendSeparator();
    viewMenu->Append(ID_ZOOM_RESET, TR_TREE(TreeStringId::MenuViewZoomDefault), TR_TREE(TreeStringId::MenuViewZoomDefaultHelp));
    viewMenu->Append(ID_ZOOM_IN, TR_TREE(TreeStringId::MenuViewZoomIn), TR_TREE(TreeStringId::MenuViewZoomInHelp));
    viewMenu->Append(ID_ZOOM_OUT, TR_TREE(TreeStringId::MenuViewZoomOut), TR_TREE(TreeStringId::MenuViewZoomOutHelp));

    menuBar->Append(viewMenu, TR_TREE(TreeStringId::MenuView));

    // Help menu
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, TR_TREE(TreeStringId::MenuHelpAbout), TR_TREE(TreeStringId::MenuHelpAboutHelp));
    menuBar->Append(helpMenu, TR_TREE(TreeStringId::MenuHelp));

    SetMenuBar(menuBar);

    // Set initial radio button selection - do this AFTER SetMenuBar
    m_layoutFanMenuItem->Check(true);

    // Add accelerators for numpad keys
    wxAcceleratorEntry entries[6];
    entries[0].Set(wxACCEL_CTRL, WXK_NUMPAD1, ID_LAYOUT_FAN);        // Ctrl+Numpad1
    entries[1].Set(wxACCEL_CTRL, WXK_NUMPAD2, ID_LAYOUT_VERTICAL);   // Ctrl+Numpad2
    entries[2].Set(wxACCEL_CTRL, WXK_NUMPAD3, ID_LAYOUT_HORIZONTAL); // Ctrl+Numpad3
    entries[3].Set(wxACCEL_CTRL, WXK_NUMPAD_ADD, ID_ZOOM_IN);        // Ctrl+Numpad+
    entries[4].Set(wxACCEL_CTRL, WXK_NUMPAD_SUBTRACT, ID_ZOOM_OUT);  // Ctrl+Numpad-
    entries[5].Set(wxACCEL_CTRL, WXK_NUMPAD0, ID_ZOOM_RESET);        // Ctrl+Numpad0
    wxAcceleratorTable accel(6, entries);
    SetAcceleratorTable(accel);
}

void TreeFrame::CreateControls()
{
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Language selection panel
    wxPanel* langPanel = new wxPanel(mainPanel);
    wxBoxSizer* langSizer = new wxBoxSizer(wxHORIZONTAL);

    langSizer->Add(new wxStaticText(langPanel, wxID_ANY, TR_TREE(TreeStringId::LabelPrimaryLanguage)), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_primaryLanguageChoice = new wxComboBox(langPanel, ID_PRIMARY_LANGUAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN | wxTE_PROCESS_ENTER);
    langSizer->Add(m_primaryLanguageChoice, 1, wxEXPAND | wxRIGHT, 20);

    langSizer->Add(new wxStaticText(langPanel, wxID_ANY, TR_TREE(TreeStringId::LabelSecondaryLanguage)), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_secondaryLanguageChoice = new wxComboBox(langPanel, ID_SECONDARY_LANGUAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN | wxTE_PROCESS_ENTER);
    langSizer->Add(m_secondaryLanguageChoice, 1, wxEXPAND);

    // Bind focus events to trigger validation when user leaves the combobox
    m_primaryLanguageChoice->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& event) {
        event.Skip();  // Allow default processing
        wxCommandEvent dummyEvent(wxEVT_COMBOBOX, ID_PRIMARY_LANGUAGE);
        OnPrimaryLanguageChanged(dummyEvent);
    });

    m_secondaryLanguageChoice->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& event) {
        event.Skip();  // Allow default processing
        wxCommandEvent dummyEvent(wxEVT_COMBOBOX, ID_SECONDARY_LANGUAGE);
        OnSecondaryLanguageChanged(dummyEvent);
    });

    langPanel->SetSizer(langSizer);
    mainSizer->Add(langPanel, 0, wxEXPAND | wxALL, 5);

    // Main horizontal splitter
    wxSplitterWindow* splitter = new wxSplitterWindow(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);

    // Left panel: Search and comparison
    wxPanel* leftPanel = new wxPanel(splitter);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // Search box
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, TR_TREE(TreeStringId::LabelSearchTaxa)), 0, wxEXPAND | wxALL, 5);
    m_searchBox = new wxTextCtrl(leftPanel, ID_SEARCH_BOX, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    leftSizer->Add(m_searchBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Search results
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, TR_TREE(TreeStringId::LabelSearchResults)), 0, wxEXPAND | wxALL, 5);
    m_searchResults = new wxListBox(leftPanel, ID_SEARCH_RESULTS, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
    leftSizer->Add(m_searchResults, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Comparison list
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, TR_TREE(TreeStringId::LabelCompareList)), 0, wxEXPAND | wxALL, 5);
    m_compareList = new wxListBox(leftPanel, ID_COMPARE_LIST, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
    leftSizer->Add(m_compareList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Clear button
    wxButton* clearButton = new wxButton(leftPanel, ID_CLEAR_COMPARE_LIST, TR_TREE(TreeStringId::BtnClearList));
    leftSizer->Add(clearButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    leftPanel->SetSizer(leftSizer);

    // Right panel: Tree view
    m_treeCanvas = new TreeCanvas(splitter);
    m_treeCanvas->SetRenderer(m_treeRenderer.get());

    // Set initial render options to match default layout
    TreeRenderOptions initialOptions;
    initialOptions.layout = m_currentLayout;  // Fan
    initialOptions.showBoxes = m_showBoxes;
    m_treeCanvas->SetRenderOptions(initialOptions);

    // Setup splitter
    splitter->SplitVertically(leftPanel, m_treeCanvas, 300);
    splitter->SetMinimumPaneSize(200);

    mainSizer->Add(splitter, 1, wxEXPAND);

    mainPanel->SetSizer(mainSizer);
}

void TreeFrame::SetupStatusBar()
{
    wxStatusBar* statusBar = CreateStatusBar(2);
    int widths[] = {-1, 200};
    statusBar->SetStatusWidths(2, widths);
    SetStatusText(TR_TREE(TreeStringId::StatusReady), 0);
}

void TreeFrame::OnDownload(wxCommandEvent& WXUNUSED(event))
{
    wxString url = "https://www.inaturalist.org/taxa/inaturalist-taxonomy.dwca.zip";

    wxFileDialog saveDialog(this, TR_TREE(TreeStringId::DlgSaveTaxonomyArchive), "",
                           TR_TREE(TreeStringId::DefaultDownloadFilename),
                           TR_TREE(TreeStringId::FilterZipFiles), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString outputPath = saveDialog.GetPath();

    // Create progress dialog
    wxProgressDialog* progressDlg = new wxProgressDialog(
        TR_TREE(TreeStringId::DlgDownloading),
        TR_TREE(TreeStringId::MsgDownloadingData),
        100, this,
        wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT);

    Downloader* downloader = new Downloader(url, outputPath);
    if (downloader->Create() != wxTHREAD_NO_ERROR || downloader->Run() != wxTHREAD_NO_ERROR)
    {
        delete downloader;
        progressDlg->Destroy();
        wxMessageBox(TR_TREE(TreeStringId::MsgDownloadStartFailed), TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
        return;
    }

    // Poll download progress
    while (!downloader->IsComplete())
    {
        wxMilliSleep(100);
        int progress = downloader->GetProgress();
        wxString status = downloader->GetStatusMessage();

        if (!progressDlg->Update(progress, status))
        {
            downloader->Cancel();
            downloader->Wait();
            delete downloader;
            progressDlg->Destroy();
            wxMessageBox(TR_TREE(TreeStringId::MsgDownloadCancelled), TR_TREE(TreeStringId::DlgCancelled), wxOK | wxICON_INFORMATION);
            return;
        }
    }

    bool success = downloader->IsSuccess();
    wxString statusMsg = downloader->GetStatusMessage();
    downloader->Wait();
    delete downloader;
    progressDlg->Destroy();

    if (success)
    {
        wxMessageBox(TR_TREE(TreeStringId::MsgDownloadSuccess),
                    TR_TREE(TreeStringId::DlgSuccess), wxOK | wxICON_INFORMATION);
    }
    else
    {
        wxString errorMsg = TR_TREE_FMT(TreeStringId::MsgDownloadFailed, statusMsg.ToStdString().c_str());
        wxMessageBox(errorMsg, TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
    }
}

void TreeFrame::OnLoadArchive(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog openDialog(this, TR_TREE(TreeStringId::DlgOpenTaxonomyArchive), "", "",
                           TR_TREE(TreeStringId::FilterZipFiles) + "|" + TR_TREE(TreeStringId::FilterAllFiles),
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString archivePath = openDialog.GetPath();

    LoadTaxonomyFile(archivePath);

    // Save the file path to settings when successfully loaded via Load Archive
    if (!m_taxonomyData->GetAllTaxa().empty())
    {
        wxConfig config("TaxonomyTree");
        config.Write("/Taxonomy/FilePath", archivePath);
        config.Flush();
    }
}

void TreeFrame::LoadTaxonomyFile(const wxString& filePath)
{
    if (!wxFileExists(filePath))
    {
        wxString errorMsg = TR_TREE_FMT(TreeStringId::MsgFileNotExist, filePath.ToStdString().c_str());
        wxMessageBox(errorMsg, TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
        return;
    }

    wxProgressDialog progressDlg(TR_TREE(TreeStringId::DlgLoading), TR_TREE(TreeStringId::MsgLoadingData), 100, this,
                                 wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    bool success = m_taxonomyData->LoadArchive(filePath,
        [&](const wxString& message, int progress) {
            progressDlg.Update(progress, message);
        });

    if (success)
    {
        UpdateLanguageChoices();
        wxString statusMsg = TR_TREE_FMT(TreeStringId::StatusLoadedTaxa, m_taxonomyData->GetAllTaxa().size());
        SetStatusText(statusMsg, 0);
        // No need for success message box - loading progress is already shown
    }
    else
    {
        wxMessageBox(TR_TREE(TreeStringId::MsgLoadFailed), TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
    }
}

void TreeFrame::UpdateLanguageChoices()
{
    auto languages = m_taxonomyData->GetAvailableLanguages();

    // Filter out unusual/system language codes
    std::vector<wxString> filteredLanguages;
    for (const auto& lang : languages)
    {
        // Skip codes that don't look like normal language names
        if (lang.Find("-letter-") != wxNOT_FOUND || lang.Find("-code") != wxNOT_FOUND)
            continue;

        filteredLanguages.push_back(lang);
    }

    // Sort languages alphabetically
    std::sort(filteredLanguages.begin(), filteredLanguages.end(),
        [](const wxString& a, const wxString& b) {
            return a.CmpNoCase(b) < 0;
        });

    m_primaryLanguageChoice->Clear();
    m_secondaryLanguageChoice->Clear();

    m_secondaryLanguageChoice->Append(TR_TREE(TreeStringId::ValueNoneLanguage));

    // Convert to wxArrayString for AutoComplete
    wxArrayString langArray;
    for (const auto& lang : filteredLanguages)
    {
        langArray.Add(lang);
        m_primaryLanguageChoice->Append(lang);
        m_secondaryLanguageChoice->Append(lang);
    }

    // Enable autocomplete
    m_primaryLanguageChoice->AutoComplete(langArray);

    wxArrayString secondaryLangs;
    secondaryLangs.Add(TR_TREE(TreeStringId::ValueNoneLanguage));
    for (const auto& lang : filteredLanguages)
        secondaryLangs.Add(lang);
    m_secondaryLanguageChoice->AutoComplete(secondaryLangs);

    m_secondaryLanguageChoice->SetSelection(0);  // (None)

    // Load saved language preferences (if any)
    wxConfig config("TaxonomyTree");
    wxString savedPrimary = config.Read("/Languages/Primary", wxEmptyString);
    wxString savedSecondary = config.Read("/Languages/Secondary", wxEmptyString);

    // Restore primary language selection
    if (!savedPrimary.IsEmpty())
    {
        for (unsigned int i = 0; i < m_primaryLanguageChoice->GetCount(); i++)
        {
            if (m_primaryLanguageChoice->GetString(i).CmpNoCase(savedPrimary) == 0)
            {
                m_primaryLanguageChoice->SetSelection(i);
                break;
            }
        }
    }

    // Restore secondary language selection
    if (!savedSecondary.IsEmpty())
    {
        for (unsigned int i = 0; i < m_secondaryLanguageChoice->GetCount(); i++)
        {
            if (m_secondaryLanguageChoice->GetString(i).CmpNoCase(savedSecondary) == 0)
            {
                m_secondaryLanguageChoice->SetSelection(i);
                break;
            }
        }
    }

    // Update search index with languages (if a primary language was selected)
    if (m_primaryLanguageChoice->GetSelection() != wxNOT_FOUND)
    {
        wxCommandEvent dummyEvent;
        OnPrimaryLanguageChanged(dummyEvent);
    }
}

void TreeFrame::OnPrimaryLanguageChanged(wxCommandEvent& WXUNUSED(event))
{
    wxString primaryLang = m_primaryLanguageChoice->GetValue().Trim();
    wxString secondaryLang = m_secondaryLanguageChoice->GetValue().Trim();

    if (secondaryLang == TR_TREE(TreeStringId::ValueNoneLanguage))
        secondaryLang = "";

    // Validate that the language exists in our list
    bool primaryValid = false;
    for (unsigned int i = 0; i < m_primaryLanguageChoice->GetCount(); i++)
    {
        if (m_primaryLanguageChoice->GetString(i).CmpNoCase(primaryLang) == 0)
        {
            primaryValid = true;
            primaryLang = m_primaryLanguageChoice->GetString(i);  // Use exact case from list
            break;
        }
    }

    if (!primaryValid)
        return;

    // Only reindex if language actually changed
    if (primaryLang == m_lastPrimaryLanguage && secondaryLang == m_lastSecondaryLanguage)
        return;

    if (!primaryLang.IsEmpty())
    {
        // Show progress dialog for indexing
        wxString indexMsg = TR_TREE_FMT(TreeStringId::MsgBuildingIndex, primaryLang.ToStdString().c_str());
        if (!secondaryLang.IsEmpty())
            indexMsg += TR_TREE_FMT(TreeStringId::MsgBuildingIndexAnd, secondaryLang.ToStdString().c_str());

        wxProgressDialog indexDlg(TR_TREE(TreeStringId::DlgIndexing), indexMsg, 100, this,
                                   wxPD_APP_MODAL | wxPD_AUTO_HIDE);
        indexDlg.Pulse();  // Show indeterminate progress
        wxYield();

        m_searchIndex->BuildIndex(*m_taxonomyData, primaryLang, secondaryLang);

        // Update last indexed languages
        m_lastPrimaryLanguage = primaryLang;
        m_lastSecondaryLanguage = secondaryLang;

        // Save language preferences
        wxConfig config("TaxonomyTree");
        config.Write("/Languages/Primary", primaryLang);
        config.Write("/Languages/Secondary", secondaryLang.IsEmpty() ? TR_TREE(TreeStringId::ValueNoneLanguage) : secondaryLang);
        config.Flush();  // Ensure data is written to disk

        SetStatusText(TR_TREE(TreeStringId::StatusReady), 1);

        // Update tree display if we have selected taxa
        if (!m_selectedTaxonIds.empty())
            UpdateTree();
    }
}

void TreeFrame::OnSecondaryLanguageChanged(wxCommandEvent& event)
{
    OnPrimaryLanguageChanged(event);
}

void TreeFrame::OnSearchTextChanged(wxCommandEvent& WXUNUSED(event))
{
    // Restart timer on each keystroke (300ms delay for fast typers)
    if (m_searchTimer->IsRunning())
        m_searchTimer->Stop();

    m_searchTimer->StartOnce(300);
}

void TreeFrame::OnSearchTimer(wxTimerEvent& WXUNUSED(event))
{
    // Timer fired - perform the actual search
    UpdateSearchResults();
}

void TreeFrame::UpdateSearchResults()
{
    wxString query = m_searchBox->GetValue();
    m_searchResults->Clear();
    m_searchResultIndexToTaxonId.clear();

    // Minimum 3 characters for search
    if (query.IsEmpty() || query.length() < 3)
        return;

    auto results = m_searchIndex->Search(query);

    int index = 0;
    for (const auto& result : results)
    {
        wxString displayText = result.displayName;
        if (!result.scientificName.IsEmpty())
            displayText += wxString::Format(" (%s)", result.scientificName);

        m_searchResults->Append(displayText);
        m_searchResultIndexToTaxonId[index] = result.taxonId;
        index++;
    }

    wxString statusMsg = TR_TREE_FMT(TreeStringId::StatusFoundResults, results.size());
    SetStatusText(statusMsg, 1);
}

void TreeFrame::OnSearchResultDoubleClick(wxCommandEvent& WXUNUSED(event))
{
    int selection = m_searchResults->GetSelection();
    if (selection == wxNOT_FOUND)
        return;

    auto it = m_searchResultIndexToTaxonId.find(selection);
    if (it == m_searchResultIndexToTaxonId.end())
        return;

    long taxonId = it->second;

    // Check if already in compare list
    if (std::find(m_selectedTaxonIds.begin(), m_selectedTaxonIds.end(), taxonId) != m_selectedTaxonIds.end())
        return;

    // Add to compare list
    m_selectedTaxonIds.push_back(taxonId);
    wxString displayName = m_searchIndex->GetDisplayName(taxonId, true);
    m_compareList->Append(displayName);

    // Update tree
    UpdateTree();
}

void TreeFrame::OnCompareItemDoubleClick(wxCommandEvent& WXUNUSED(event))
{
    int selection = m_compareList->GetSelection();
    if (selection == wxNOT_FOUND || selection >= (int)m_selectedTaxonIds.size())
        return;

    // Remove from compare list
    m_selectedTaxonIds.erase(m_selectedTaxonIds.begin() + selection);
    m_compareList->Delete(selection);

    // Update tree
    UpdateTree();
}

void TreeFrame::OnClearCompareList(wxCommandEvent& WXUNUSED(event))
{
    // Clear the compare list
    m_selectedTaxonIds.clear();
    m_compareList->Clear();

    // Update tree
    UpdateTree();
}

void TreeFrame::UpdateTree()
{
    if (m_selectedTaxonIds.empty())
    {
        // Clear the tree renderer data by passing empty list
        m_treeRenderer->SetCompareTaxa({}, *m_taxonomyData, *m_searchIndex, m_showFullTree);
        m_treeCanvas->UpdateView();
        return;
    }

    m_treeRenderer->SetCompareTaxa(m_selectedTaxonIds, *m_taxonomyData, *m_searchIndex, m_showFullTree);
    m_treeCanvas->UpdateView();
}

void TreeFrame::OnLayoutVertical(wxCommandEvent& WXUNUSED(event))
{
    m_currentLayout = TreeLayout::Vertical;
    TreeRenderOptions options;
    options.layout = m_currentLayout;
    options.showBoxes = m_showBoxes;
    m_treeCanvas->SetRenderOptions(options);
    m_treeCanvas->UpdateView();

    // Manually implement radio button behavior with check items
    m_layoutVerticalMenuItem->Check(true);
    m_layoutHorizontalMenuItem->Check(false);
    m_layoutFanMenuItem->Check(false);
}

void TreeFrame::OnLayoutHorizontal(wxCommandEvent& WXUNUSED(event))
{
    m_currentLayout = TreeLayout::Horizontal;
    TreeRenderOptions options;
    options.layout = m_currentLayout;
    options.showBoxes = m_showBoxes;
    m_treeCanvas->SetRenderOptions(options);
    m_treeCanvas->UpdateView();

    // Manually implement radio button behavior with check items
    m_layoutHorizontalMenuItem->Check(true);
    m_layoutVerticalMenuItem->Check(false);
    m_layoutFanMenuItem->Check(false);
}

void TreeFrame::OnLayoutFan(wxCommandEvent& WXUNUSED(event))
{
    m_currentLayout = TreeLayout::Fan;
    TreeRenderOptions options;
    options.layout = m_currentLayout;
    options.showBoxes = m_showBoxes;
    m_treeCanvas->SetRenderOptions(options);
    m_treeCanvas->UpdateView();

    // Manually implement radio button behavior with check items
    m_layoutFanMenuItem->Check(true);
    m_layoutVerticalMenuItem->Check(false);
    m_layoutHorizontalMenuItem->Check(false);
}

void TreeFrame::OnToggleBoxes(wxCommandEvent& WXUNUSED(event))
{
    m_showBoxes = !m_showBoxes;
    TreeRenderOptions options;
    options.layout = m_currentLayout;
    options.showBoxes = m_showBoxes;
    m_treeCanvas->SetRenderOptions(options);
    m_treeCanvas->UpdateView();
}

void TreeFrame::OnToggleFullTree(wxCommandEvent& WXUNUSED(event))
{
    m_showFullTree = !m_showFullTree;
    UpdateTree();  // Rebuild tree with new setting
}

void TreeFrame::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    m_treeCanvas->ZoomIn();
}

void TreeFrame::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    m_treeCanvas->ZoomOut();
}

void TreeFrame::OnZoomReset(wxCommandEvent& WXUNUSED(event))
{
    m_treeCanvas->ZoomReset();
}

void TreeFrame::OnExportSVG(wxCommandEvent& WXUNUSED(event))
{
    if (m_selectedTaxonIds.empty())
    {
        wxMessageBox(TR_TREE(TreeStringId::MsgNoTaxaSelected), TR_TREE(TreeStringId::DlgExportSVG), wxOK | wxICON_INFORMATION);
        return;
    }

    wxFileDialog saveDialog(this, TR_TREE(TreeStringId::DlgExportSVG), "",
                           TR_TREE(TreeStringId::DefaultSVGFilename),
                           TR_TREE(TreeStringId::FilterSVGFiles), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL)
        return;

    TreeRenderOptions options;
    options.layout = m_currentLayout;
    options.showBoxes = m_showBoxes;

    bool success = m_treeRenderer->ExportToSVG(saveDialog.GetPath(), options);

    if (success)
        wxMessageBox(TR_TREE(TreeStringId::MsgSVGExportSuccess), TR_TREE(TreeStringId::DlgSuccess), wxOK | wxICON_INFORMATION);
    else
        wxMessageBox(TR_TREE(TreeStringId::MsgSVGExportFailed), TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
}

void TreeFrame::OnExportPNG(wxCommandEvent& WXUNUSED(event))
{
    if (m_selectedTaxonIds.empty())
    {
        wxMessageBox(TR_TREE(TreeStringId::MsgNoTaxaSelected), TR_TREE(TreeStringId::DlgExportPNG), wxOK | wxICON_INFORMATION);
        return;
    }

    wxFileDialog saveDialog(this, TR_TREE(TreeStringId::DlgExportPNG), "",
                           TR_TREE(TreeStringId::DefaultPNGFilename),
                           TR_TREE(TreeStringId::FilterPNGFiles), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL)
        return;

    // Ask for DPI
    wxString dpiStr = wxGetTextFromUser(TR_TREE(TreeStringId::PromptEnterDPI), TR_TREE(TreeStringId::DlgExportPNG),
                                        TR_TREE(TreeStringId::ValueDefaultDPI), this);
    if (dpiStr.IsEmpty())
        return;

    long dpi;
    if (!dpiStr.ToLong(&dpi) || dpi < 70 || dpi > 600)
    {
        wxMessageBox(TR_TREE(TreeStringId::MsgInvalidDPI), TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
        return;
    }

    TreeRenderOptions options;
    options.layout = m_currentLayout;
    options.showBoxes = m_showBoxes;

    bool success = m_treeRenderer->ExportToPNG(saveDialog.GetPath(), options, dpi);

    if (success)
    {
        wxString successMsg = TR_TREE_FMT(TreeStringId::MsgPNGExportSuccess, dpi);
        wxMessageBox(successMsg, TR_TREE(TreeStringId::DlgSuccess), wxOK | wxICON_INFORMATION);
    }
    else
        wxMessageBox(TR_TREE(TreeStringId::MsgPNGExportFailed), TR_TREE(TreeStringId::DlgError), wxOK | wxICON_ERROR);
}

void TreeFrame::OnSettings(wxCommandEvent& WXUNUSED(event))
{
    SettingsDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        // Check if language changed
        if (dlg.LanguageChanged())
        {
            wxMessageBox("Language changed. Please restart the application for changes to take effect.",
                        "Language Changed", wxOK | wxICON_INFORMATION);
        }
    }
}

void TreeFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void TreeFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString aboutMsg = wxString("TaxonomyTree v") +
                       TAXONOMYTOOLS_VERSION_STRING +
                       TR_TREE(TreeStringId::AboutVersion) +
                       TR_TREE(TreeStringId::AboutDescription) +
                       TR_TREE(TreeStringId::AboutPurpose);
    wxMessageBox(aboutMsg, TR_TREE(TreeStringId::DlgAboutTitle), wxOK | wxICON_INFORMATION);
}

void TreeFrame::OnClose(wxCloseEvent& event)
{
    // Disable the window to prevent further interaction
    Disable();

    // Create a simple dialog to show during cleanup
    wxDialog* cleanupDlg = new wxDialog(this, wxID_ANY, TR_TREE(TreeStringId::DlgClosing),
                                        wxDefaultPosition, wxSize(250, 100),
                                        wxCAPTION | wxSTAY_ON_TOP);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* text = new wxStaticText(cleanupDlg, wxID_ANY,
                                          TR_TREE(TreeStringId::MsgClosingCleanup));
    text->SetFont(text->GetFont().MakeBold());
    sizer->Add(text, 1, wxALL | wxALIGN_CENTER, 20);
    cleanupDlg->SetSizer(sizer);
    cleanupDlg->Centre();

    // Show the dialog non-modally so it displays but doesn't block
    cleanupDlg->Show();
    wxYield();

    // Perform cleanup operations that might take time
    // Reset large data structures in correct order (dependencies first)

    // Clear tree renderer first (it may reference the other data)
    if (m_treeRenderer)
    {
        m_treeRenderer.reset();
        wxYield();
    }

    // Then clear search index
    if (m_searchIndex)
    {
        m_searchIndex.reset();
        wxYield();
    }

    // Finally clear taxonomy data (largest structure)
    if (m_taxonomyData)
    {
        m_taxonomyData.reset();
        wxYield();
    }

    // Close the cleanup dialog
    cleanupDlg->Destroy();

    // Allow the close event to proceed (this will call the destructor and close everything)
    event.Skip();
}

// TreeCanvas implementation
wxBEGIN_EVENT_TABLE(TreeCanvas, wxScrolledWindow)
    EVT_PAINT(TreeCanvas::OnPaint)
    EVT_SIZE(TreeCanvas::OnSize)
    EVT_MOUSEWHEEL(TreeCanvas::OnMouseWheel)
    EVT_LEFT_DOWN(TreeCanvas::OnMouseDown)
    EVT_MOTION(TreeCanvas::OnMouseMove)
    EVT_LEFT_UP(TreeCanvas::OnMouseUp)
wxEND_EVENT_TABLE()

TreeCanvas::TreeCanvas(wxWindow* parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxFULL_REPAINT_ON_RESIZE)
    , m_renderer(nullptr)
    , m_zoomFactor(1.0)
    , m_dragging(false)
    , m_scrollStartX(0)
    , m_scrollStartY(0)
{
    SetBackgroundColour(*wxWHITE);
    SetScrollRate(10, 10);
}

void TreeCanvas::SetRenderer(TreeRenderer* renderer)
{
    m_renderer = renderer;
}

void TreeCanvas::SetRenderOptions(const TreeRenderOptions& options)
{
    m_options = options;
}

void TreeCanvas::UpdateView()
{
    if (m_renderer)
    {
        // Create a properly sized memory DC to calculate the layout
        wxClientDC clientDC(this);
        wxMemoryDC memDC;
        // Create a bitmap large enough to not affect text measurements
        wxBitmap dummyBitmap(2000, 2000);
        memDC.SelectObject(dummyBitmap);

        // Copy font from the client DC to ensure proper text measurements
        memDC.SetFont(clientDC.GetFont());

        // Render to the memory DC to trigger layout calculation
        // This updates m_requiredSize in the renderer
        m_renderer->Render(memDC, m_options);

        // Now get the correct size and apply zoom
        wxSize size = m_renderer->GetRequiredSize(m_options);
        SetVirtualSize(wxSize(size.x * m_zoomFactor, size.y * m_zoomFactor));
    }
    Refresh();
}

void TreeCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxBufferedPaintDC baseDC(this);
    wxGCDC dc(baseDC);  // Wrap with wxGCDC to ensure graphics context on all platforms
    DoPrepareDC(dc);

    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    if (m_renderer)
    {
        // Apply zoom scaling
        dc.SetUserScale(m_zoomFactor, m_zoomFactor);
        m_renderer->Render(dc, m_options);
    }
}

void TreeCanvas::OnSize(wxSizeEvent& WXUNUSED(event))
{
    Refresh();
}

void TreeCanvas::OnMouseWheel(wxMouseEvent& event)
{
    // Ctrl+Scroll for zooming
    if (event.ControlDown())
    {
        double oldZoom = m_zoomFactor;

        if (event.GetWheelRotation() > 0)
        {
            // Zoom in
            m_zoomFactor *= 1.1;
            if (m_zoomFactor > 5.0)
                m_zoomFactor = 5.0;
        }
        else
        {
            // Zoom out
            m_zoomFactor /= 1.1;
            if (m_zoomFactor < 0.1)
                m_zoomFactor = 0.1;
        }

        // Adjust scroll position to maintain zoom center
        int scrollX, scrollY;
        GetViewStart(&scrollX, &scrollY);

        int newScrollX = scrollX * (m_zoomFactor / oldZoom);
        int newScrollY = scrollY * (m_zoomFactor / oldZoom);

        UpdateView();
        Scroll(newScrollX, newScrollY);

        event.Skip(false);  // Don't propagate the event
    }
    else
    {
        // Normal scrolling
        event.Skip();
    }
}

void TreeCanvas::OnMouseDown(wxMouseEvent& event)
{
    m_dragging = true;
    m_dragStart = event.GetPosition();
    GetViewStart(&m_scrollStartX, &m_scrollStartY);
    SetCursor(wxCursor(wxCURSOR_HAND));
    CaptureMouse();
}

void TreeCanvas::OnMouseMove(wxMouseEvent& event)
{
    if (m_dragging)
    {
        wxPoint currentPos = event.GetPosition();
        wxPoint delta = m_dragStart - currentPos;

        // Convert pixel delta to scroll units
        int scrollX = m_scrollStartX + delta.x / 10;
        int scrollY = m_scrollStartY + delta.y / 10;

        Scroll(scrollX, scrollY);
    }
}

void TreeCanvas::OnMouseUp(wxMouseEvent& WXUNUSED(event))
{
    if (m_dragging)
    {
        m_dragging = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
        if (HasCapture())
            ReleaseMouse();
    }
}

void TreeCanvas::ZoomIn()
{
    double oldZoom = m_zoomFactor;
    m_zoomFactor *= 1.1;
    if (m_zoomFactor > 5.0)
        m_zoomFactor = 5.0;

    // Adjust scroll position to maintain zoom center
    int scrollX, scrollY;
    GetViewStart(&scrollX, &scrollY);
    int newScrollX = scrollX * (m_zoomFactor / oldZoom);
    int newScrollY = scrollY * (m_zoomFactor / oldZoom);

    UpdateView();
    Scroll(newScrollX, newScrollY);
}

void TreeCanvas::ZoomOut()
{
    double oldZoom = m_zoomFactor;
    m_zoomFactor /= 1.1;
    if (m_zoomFactor < 0.1)
        m_zoomFactor = 0.1;

    // Adjust scroll position to maintain zoom center
    int scrollX, scrollY;
    GetViewStart(&scrollX, &scrollY);
    int newScrollX = scrollX * (m_zoomFactor / oldZoom);
    int newScrollY = scrollY * (m_zoomFactor / oldZoom);

    UpdateView();
    Scroll(newScrollX, newScrollY);
}

void TreeCanvas::ZoomReset()
{
    double oldZoom = m_zoomFactor;
    m_zoomFactor = 1.0;

    // Adjust scroll position to maintain zoom center
    int scrollX, scrollY;
    GetViewStart(&scrollX, &scrollY);
    int newScrollX = scrollX * (m_zoomFactor / oldZoom);
    int newScrollY = scrollY * (m_zoomFactor / oldZoom);

    UpdateView();
    Scroll(newScrollX, newScrollY);
}
