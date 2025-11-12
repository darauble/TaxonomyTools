#include "MainFrame.hpp"
#include "TaxonomyProcessor.hpp"
#include "Downloader.hpp"
#include "DictStrings.hpp"
#include "SettingsDialog.hpp"
#include "icon_data.h"
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/icon.h>
#include <wx/iconbndl.h>
#include <wx/mstream.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_PREFERENCES, MainFrame::OnSettings)
    EVT_BUTTON(ID_Download, MainFrame::OnDownload)
    EVT_BUTTON(ID_LoadZip, MainFrame::OnLoadZip)
    EVT_LISTBOX(ID_SelectLanguage, MainFrame::OnLanguageSelect)
    EVT_BUTTON(ID_GenerateDict, MainFrame::OnGenerateDict)
    EVT_TIMER(ID_DownloadTimer, MainFrame::OnDownloadTimer)
    EVT_TIMER(ID_SearchTimer, MainFrame::OnSearchTimer)
    EVT_TIMER(ID_LoadTimer, MainFrame::OnLoadTimer)
    EVT_TIMER(ID_GenerateTimer, MainFrame::OnGenerateTimer)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, TR_DICT(DictStringId::AppTitle), wxDefaultPosition, wxSize(800, 600))
    , m_processor(std::make_unique<TaxonomyProcessor>())
    , m_downloadTimer(new wxTimer(this, ID_DownloadTimer))
    , m_searchTimer(new wxTimer(this, ID_SearchTimer))
    , m_loadTimer(new wxTimer(this, ID_LoadTimer))
    , m_generateTimer(new wxTimer(this, ID_GenerateTimer))
{
    CreateMenuBar();
    CreateControls();
    CreateLayout();
    SetApplicationIcon();
    
    // Bind thread events explicitly - TEMPORARILY DISABLED to test
    // Bind(wxEVT_DOWNLOAD_PROGRESS, &MainFrame::OnDownloadProgress, this);
    // Bind(wxEVT_DOWNLOAD_COMPLETE, &MainFrame::OnDownloadComplete, this);
    
    // Bind key events for incremental search
    m_sourceLanguage->Bind(wxEVT_CHAR, &MainFrame::OnSourceLanguageChar, this);
    m_targetLanguages->Bind(wxEVT_CHAR, &MainFrame::OnTargetLanguageChar, this);
    
    Centre();
}

MainFrame::~MainFrame()
{
    // Stop timers first
    if (m_downloadTimer && m_downloadTimer->IsRunning())
    {
        m_downloadTimer->Stop();
    }
    delete m_downloadTimer;
    
    if (m_searchTimer && m_searchTimer->IsRunning())
    {
        m_searchTimer->Stop();
    }
    delete m_searchTimer;
    
    if (m_loadTimer && m_loadTimer->IsRunning())
    {
        m_loadTimer->Stop();
    }
    delete m_loadTimer;
    
    if (m_generateTimer && m_generateTimer->IsRunning())
    {
        m_generateTimer->Stop();
    }
    delete m_generateTimer;
    
    // Cancel any ongoing loading
    if (m_loader)
    {
        if (m_loader->IsAlive()) {
            m_loader->Wait();
        }
        m_loader.reset();
    }
    
    // Cancel any ongoing dictionary generation
    if (m_dictThread)
    {
        if (m_dictThread->IsAlive()) {
            m_dictThread->Wait();
        }
        m_dictThread.reset();
    }
    
    // Cancel any ongoing download
    if (m_downloader)
    {
        // Cancel the download to prevent it from sending events to destroyed object
        m_downloader->Cancel();
        // For joinable threads, we need to wait before destroying
        if (m_downloader->IsAlive()) {
            m_downloader->Wait();
        }
        m_downloader.reset();
    }
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_PREFERENCES, "Settings", "Open settings dialog");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, TR_DICT(DictStringId::MenuFileExit), TR_DICT(DictStringId::MenuFileExitHelp));

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, TR_DICT(DictStringId::MenuHelpAbout), TR_DICT(DictStringId::MenuHelpAboutHelp));

    menuBar->Append(fileMenu, TR_DICT(DictStringId::MenuFile));
    menuBar->Append(helpMenu, TR_DICT(DictStringId::MenuHelp));

    SetMenuBar(menuBar);
}

void MainFrame::CreateControls()
{
    // Download section
    m_urlText = new wxTextCtrl(this, wxID_ANY,
        TR_DICT(DictStringId::ValueDefaultURL),
        wxDefaultPosition, wxDefaultSize);
    m_downloadBtn = new wxButton(this, ID_Download, TR_DICT(DictStringId::BtnDownloadArchive));
    m_loadZipBtn = new wxButton(this, ID_LoadZip, TR_DICT(DictStringId::BtnLoadArchive));

    m_statusText = new wxStaticText(this, wxID_ANY, TR_DICT(DictStringId::StatusReady));
    m_progressGauge = new wxGauge(this, wxID_ANY, 100);

    // Language selection section
    m_sourceLanguage = new wxListBox(this, ID_SelectLanguage, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
    m_sourceSearchDisplay = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    m_sourceSearchDisplay->SetBackgroundColour(wxColour(240, 240, 240));
    m_sourceSearchDisplay->Hide();

    m_targetLanguages = new wxCheckListBox(this, wxID_ANY);
    m_targetSearchDisplay = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    m_targetSearchDisplay->SetBackgroundColour(wxColour(240, 240, 240));
    m_targetSearchDisplay->Hide();
    m_includeScientific = new wxCheckBox(this, wxID_ANY, TR_DICT(DictStringId::CheckIncludeScientific));
    m_includeScientific->SetValue(true);
    m_filterOnlyTargets = new wxCheckBox(this, wxID_ANY, TR_DICT(DictStringId::CheckFilterTargets));
    m_filterOnlyTargets->SetValue(false);

    // Output section
    m_outputPath = new wxTextCtrl(this, wxID_ANY, TR_DICT(DictStringId::ValueDefaultOutput));
    m_browseOutputBtn = new wxButton(this, wxID_ANY, TR_DICT(DictStringId::BtnBrowse));
    m_generateBtn = new wxButton(this, ID_GenerateDict, TR_DICT(DictStringId::BtnGenerateDict));
    m_generateBtn->Enable(false);

    // Initially disable language selection
    m_sourceLanguage->Enable(false);
    m_targetLanguages->Enable(false);
    m_includeScientific->Enable(false);
}

void MainFrame::CreateLayout()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Download section
    wxStaticBoxSizer* downloadBox = new wxStaticBoxSizer(wxVERTICAL, this, TR_DICT(DictStringId::SectionDownload));
    downloadBox->Add(new wxStaticText(this, wxID_ANY, TR_DICT(DictStringId::LabelTaxonomyURL)), 0, wxALL, 5);
    downloadBox->Add(m_urlText, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* downloadBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    downloadBtnSizer->Add(m_downloadBtn, 0, wxALL, 5);
    downloadBtnSizer->Add(m_loadZipBtn, 0, wxALL, 5);
    downloadBox->Add(downloadBtnSizer, 0, wxALL, 5);

    downloadBox->Add(m_statusText, 0, wxALL, 5);
    downloadBox->Add(m_progressGauge, 0, wxEXPAND | wxALL, 5);

    mainSizer->Add(downloadBox, 0, wxEXPAND | wxALL, 10);

    // Language selection section
    wxStaticBoxSizer* langBox = new wxStaticBoxSizer(wxVERTICAL, this, TR_DICT(DictStringId::SectionLanguageSelection));

    wxBoxSizer* langSizer = new wxBoxSizer(wxHORIZONTAL);

    // Source language
    wxBoxSizer* sourceSizer = new wxBoxSizer(wxVERTICAL);
    sourceSizer->Add(new wxStaticText(this, wxID_ANY, TR_DICT(DictStringId::LabelSourceLanguage)), 0, wxALL, 5);
    sourceSizer->Add(m_sourceLanguage, 1, wxEXPAND | wxALL, 5);
    sourceSizer->Add(m_sourceSearchDisplay, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    langSizer->Add(sourceSizer, 1, wxEXPAND | wxALL, 5);

    // Target languages
    wxBoxSizer* targetSizer = new wxBoxSizer(wxVERTICAL);
    targetSizer->Add(new wxStaticText(this, wxID_ANY, TR_DICT(DictStringId::LabelTargetLanguages)), 0, wxALL, 5);
    targetSizer->Add(m_targetLanguages, 1, wxEXPAND | wxALL, 5);
    targetSizer->Add(m_targetSearchDisplay, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    targetSizer->Add(m_includeScientific, 0, wxALL, 5);
    targetSizer->Add(m_filterOnlyTargets, 0, wxALL, 5);
    langSizer->Add(targetSizer, 1, wxEXPAND | wxALL, 5);

    langBox->Add(langSizer, 1, wxEXPAND);
    mainSizer->Add(langBox, 1, wxEXPAND | wxALL, 10);

    // Output section
    wxStaticBoxSizer* outputBox = new wxStaticBoxSizer(wxVERTICAL, this, TR_DICT(DictStringId::SectionOutput));

    wxBoxSizer* pathSizer = new wxBoxSizer(wxHORIZONTAL);
    pathSizer->Add(new wxStaticText(this, wxID_ANY, TR_DICT(DictStringId::LabelOutputPath)), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    pathSizer->Add(m_outputPath, 1, wxEXPAND | wxALL, 5);
    pathSizer->Add(m_browseOutputBtn, 0, wxALL, 5);
    outputBox->Add(pathSizer, 0, wxEXPAND);

    outputBox->Add(m_generateBtn, 0, wxALIGN_CENTER | wxALL, 10);

    mainSizer->Add(outputBox, 0, wxEXPAND | wxALL, 10);

    SetSizer(mainSizer);
}

void MainFrame::SetApplicationIcon()
{
#ifdef __WXMSW__
    // On Windows, the icon is automatically loaded from the resource file
    SetIcon(wxIcon("IDI_ICON1"));
#else
    // On Linux, load the PNG icon from embedded data
    wxMemoryInputStream stream(taxonomy_dict_icon_png, taxonomy_dict_icon_png_len);
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

void MainFrame::OnExit(wxCommandEvent& event)
{
    (void)event;
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    (void)event;
    wxMessageBox(TR_DICT(DictStringId::AboutMessage),
                 TR_DICT(DictStringId::DlgAbout), wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnSettings(wxCommandEvent& event)
{
    (void)event;
    SettingsDialog dialog(this);
    if (dialog.ShowModal() == wxID_OK)
    {
        if (dialog.LanguageChanged())
        {
            wxMessageBox("Language changed. Please restart the application for changes to take effect.",
                        "Settings", wxOK | wxICON_INFORMATION, this);
        }
    }
}

void MainFrame::OnDownload(wxCommandEvent& event)
{
    (void)event;
    wxString url = m_urlText->GetValue();
    if (url.IsEmpty())
    {
        wxMessageBox(TR_DICT(DictStringId::ErrEnterURL), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        return;
    }

    // Validate URL format
    if (!url.StartsWith("http://") && !url.StartsWith("https://"))
    {
        wxMessageBox(TR_DICT(DictStringId::ErrInvalidURLScheme), TR_DICT(DictStringId::DlgInvalidURL), wxOK | wxICON_ERROR, this);
        return;
    }


    wxFileDialog saveDialog(this, TR_DICT(DictStringId::DlgSaveArchive), "", "inaturalist-taxonomy.zip",
                           TR_DICT(DictStringId::FilterZipFiles), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL)
        return;

    m_zipFilePath = saveDialog.GetPath();

    // Disable all buttons during download
    EnableAllButtons(false);
    EnableLanguageControls(false);
    m_statusText->SetLabel(TR_DICT(DictStringId::StatusDownloading));
    m_progressGauge->SetValue(0);


    m_downloader = std::make_unique<Downloader>(url, m_zipFilePath);
    wxThreadError threadResult = m_downloader->Run();
    if (threadResult != wxTHREAD_NO_ERROR)
    {
        wxString errorMsg = TR_DICT_FMT(DictStringId::ErrThreadStartFailed, threadResult);
        m_statusText->SetLabel(errorMsg);
        // Re-enable all buttons on error
        EnableAllButtons(true);
        EnableLanguageControls(true);
        wxMessageBox(errorMsg, TR_DICT(DictStringId::DlgThreadError), wxOK | wxICON_ERROR, this);
    }
    else
    {
        m_statusText->SetLabel(TR_DICT(DictStringId::StatusDownloadStarted));
        m_downloadTimer->Start(500); // Poll every 500ms
    }
}

void MainFrame::OnLoadZip(wxCommandEvent& event)
{
    (void)event;
    wxFileDialog openDialog(this, TR_DICT(DictStringId::DlgChooseArchive), "", "",
                           TR_DICT(DictStringId::FilterZipFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() == wxID_CANCEL)
        return;

    m_zipFilePath = openDialog.GetPath();

    // Disable all buttons during loading
    EnableAllButtons(false);
    EnableLanguageControls(false);
    m_statusText->SetLabel(TR_DICT(DictStringId::StatusLoadingArchive));

    m_loader = std::make_unique<ArchiveLoader>(m_processor.get(), m_zipFilePath, this);
    if (m_loader->Run() == wxTHREAD_NO_ERROR)
    {
        m_loadTimer->Start(250); // Poll every 250ms
    }
    else
    {
        m_statusText->SetLabel(TR_DICT(DictStringId::StatusLoadStartFailed));
        // Re-enable all buttons on error
        EnableAllButtons(true);
        EnableLanguageControls(true);
        wxMessageBox(TR_DICT(DictStringId::ErrLoadThreadFailed), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        m_loader.reset();
    }
}

void MainFrame::OnLanguageSelect(wxCommandEvent& event)
{
    (void)event;
    int selection = m_sourceLanguage->GetSelection();
    if (selection != wxNOT_FOUND)
    {
        // Enable generate button if we have both source and at least one target
        bool hasTarget = false;
        for (unsigned int i = 0; i < m_targetLanguages->GetCount(); ++i)
        {
            if (m_targetLanguages->IsChecked(i))
            {
                hasTarget = true;
                break;
            }
        }
        
        m_generateBtn->Enable(hasTarget || m_includeScientific->GetValue());
    }
}

void MainFrame::OnGenerateDict(wxCommandEvent& event)
{
    (void)event;
    int sourceSelection = m_sourceLanguage->GetSelection();
    if (sourceSelection == wxNOT_FOUND)
    {
        wxMessageBox(TR_DICT(DictStringId::ErrNoSourceLanguage), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        return;
    }

    wxString sourceLanguage = m_sourceLanguage->GetStringSelection();
    std::vector<wxString> targetLanguages;

    for (unsigned int i = 0; i < m_targetLanguages->GetCount(); ++i)
    {
        if (m_targetLanguages->IsChecked(i))
        {
            targetLanguages.push_back(m_targetLanguages->GetString(i));
        }
    }

    bool includeScientific = m_includeScientific->GetValue();
    bool filterOnlyTargets = m_filterOnlyTargets->GetValue();
    wxString outputPath = m_outputPath->GetValue();

    if (targetLanguages.empty() && !includeScientific)
    {
        wxMessageBox(TR_DICT(DictStringId::ErrNoTargetLanguage), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        return;
    }

    // Disable all buttons during dictionary generation
    EnableAllButtons(false);
    EnableLanguageControls(false);
    m_statusText->SetLabel(TR_DICT(DictStringId::StatusGenerating));
    m_progressGauge->SetValue(0);

    m_dictThread = std::make_unique<DictionaryGeneratorThread>(m_processor.get(), sourceLanguage, targetLanguages,
                                                              includeScientific, filterOnlyTargets, outputPath, this);
    if (m_dictThread->Run() == wxTHREAD_NO_ERROR)
    {
        m_generateTimer->Start(250); // Poll every 250ms
    }
    else
    {
        m_statusText->SetLabel(TR_DICT(DictStringId::StatusGenerateFailed));
        // Re-enable all buttons on error
        EnableAllButtons(true);
        EnableLanguageControls(true);
        wxMessageBox(TR_DICT(DictStringId::ErrGenerateThreadFailed), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        m_dictThread.reset();
    }
}

void MainFrame::OnDownloadProgress(wxThreadEvent& event)
{
    int progress = event.GetInt();
    m_progressGauge->SetValue(progress);
    wxString statusMsg = wxString::Format("Downloading... %d%%", progress);
    m_statusText->SetLabel(statusMsg);
}

void MainFrame::OnDownloadComplete(wxThreadEvent& event)
{
    (void)event;
    // Minimal handler - don't access any event data at all
    
    // Only re-enable the button - avoid any other operations
    if (m_downloadBtn)
        m_downloadBtn->Enable(true);
    
    // Don't even set status text to avoid any string operations
}

void MainFrame::SafeUpdateProgress(int progress)
{
    if (m_progressGauge)
        m_progressGauge->SetValue(progress);
}

void MainFrame::SafeDownloadComplete(bool success, const wxString& message)
{
    if (m_downloadBtn)
        m_downloadBtn->Enable(true);

    if (m_progressGauge)
        m_progressGauge->SetValue(success ? 100 : 0);

    if (m_statusText)
    {
        if (success)
            m_statusText->SetLabel(TR_DICT(DictStringId::StatusDownloadComplete));
        else
            m_statusText->SetLabel(TR_DICT_FMT(DictStringId::StatusDownloadFailed, message.ToStdString().c_str()));
    }

    // Clean up downloader
    m_downloader.reset();
}

void MainFrame::SafeUpdateStatus(const wxString& status)
{
    if (m_statusText)
    {
        m_statusText->SetLabel(status);
    }
}

void MainFrame::OnDownloadTimer(wxTimerEvent& event)
{
    (void)event;
    if (!m_downloader)
    {
        m_downloadTimer->Stop();
        return;
    }
    
    // Poll download progress
    int progress = m_downloader->GetProgress();
    bool isComplete = m_downloader->IsComplete();
    bool isSuccess = m_downloader->IsSuccess();
    
    // Check if download is complete FIRST
    if (isComplete)
    {
        // IMMEDIATELY copy all state while thread is still alive
        wxString message = m_downloader->GetStatusMessage();
        int finalProgress = isSuccess ? 100 : 0;
        
        // Stop timer and clean up downloader properly for joinable thread
        m_downloadTimer->Stop();
        
        // Wait for joinable thread to complete, then clean up
        if (m_downloader->IsAlive()) {
            m_downloader->Wait();
        }
        m_downloader.reset();
        
        // Use CallAfter with ONLY copied values - no more thread access
        CallAfter([this, isSuccess, message, finalProgress]() {
            // Re-enable all buttons
            EnableAllButtons(true);
            EnableLanguageControls(true);

            // Set final progress bar value
            m_progressGauge->SetValue(finalProgress);

            if (isSuccess)
            {
                m_statusText->SetLabel(TR_DICT(DictStringId::StatusDownloadComplete));
            }
            else
            {
                m_statusText->SetLabel(TR_DICT_FMT(DictStringId::StatusDownloadFailed, message.ToStdString().c_str()));
            }
        });
    }
    else
    {
        // Use CallAfter for progress updates (only when not complete)
        CallAfter([this, progress]() {
            // Always update progress bar, even with progress 0
            m_progressGauge->SetValue(progress);

            // Update status text to show progress
            wxString statusMsg = TR_DICT_FMT(DictStringId::StatusDownloadingPercent, progress);
            m_statusText->SetLabel(statusMsg);
        });
    }
}

void MainFrame::OnSourceLanguageChar(wxKeyEvent& event)
{
    int keyCode = event.GetKeyCode();
    
    if (keyCode == WXK_ESCAPE)
    {
        // Clear search immediately
        m_sourceSearchBuffer.Clear();
        m_sourceSearchDisplay->Hide();
        GetSizer()->Layout();
        m_searchTimer->Stop();
    }
    else if (keyCode == WXK_BACK && !m_sourceSearchBuffer.IsEmpty())
    {
        // Remove last character
        m_sourceSearchBuffer = m_sourceSearchBuffer.Left(m_sourceSearchBuffer.Length() - 1);
        if (m_sourceSearchBuffer.IsEmpty())
        {
            m_sourceSearchDisplay->Hide();
            GetSizer()->Layout();
            m_searchTimer->Stop();
        }
        else
        {
            m_sourceSearchDisplay->SetValue(TR_DICT_FMT(DictStringId::SearchPrompt, m_sourceSearchBuffer.ToStdString().c_str()));
            PerformSearch(m_sourceLanguage, m_sourceSearchBuffer);
            m_searchTimer->Start(2000, true); // 2 second timeout
        }
    }
    else if (keyCode >= 32 && keyCode < 127) // Printable ASCII characters
    {
        // Add character to search buffer
        m_sourceSearchBuffer += (wxChar)keyCode;
        m_sourceSearchDisplay->SetValue(TR_DICT_FMT(DictStringId::SearchPrompt, m_sourceSearchBuffer.ToStdString().c_str()));
        m_sourceSearchDisplay->Show();
        GetSizer()->Layout();
        PerformSearch(m_sourceLanguage, m_sourceSearchBuffer);
        m_searchTimer->Start(2000, true); // 2 second timeout
    }
    else
    {
        // Pass through other keys (arrows, etc.)
        event.Skip();
    }
}

void MainFrame::OnTargetLanguageChar(wxKeyEvent& event)
{
    int keyCode = event.GetKeyCode();

    if (keyCode == WXK_ESCAPE)
    {
        // Clear search immediately
        m_targetSearchBuffer.Clear();
        m_targetSearchDisplay->Hide();
        GetSizer()->Layout();
        m_searchTimer->Stop();
    }
    else if (keyCode == WXK_BACK && !m_targetSearchBuffer.IsEmpty())
    {
        // Remove last character
        m_targetSearchBuffer = m_targetSearchBuffer.Left(m_targetSearchBuffer.Length() - 1);
        if (m_targetSearchBuffer.IsEmpty())
        {
            m_targetSearchDisplay->Hide();
            GetSizer()->Layout();
            m_searchTimer->Stop();
        }
        else
        {
            m_targetSearchDisplay->SetValue(TR_DICT_FMT(DictStringId::SearchPrompt, m_targetSearchBuffer.ToStdString().c_str()));
            PerformSearch(m_targetLanguages, m_targetSearchBuffer);
            m_searchTimer->Start(2000, true); // 2 second timeout
        }
    }
    else if (keyCode >= 32 && keyCode < 127) // Printable ASCII characters
    {
        // Add character to search buffer
        m_targetSearchBuffer += (wxChar)keyCode;
        m_targetSearchDisplay->SetValue(TR_DICT_FMT(DictStringId::SearchPrompt, m_targetSearchBuffer.ToStdString().c_str()));
        m_targetSearchDisplay->Show();
        GetSizer()->Layout();
        PerformSearch(m_targetLanguages, m_targetSearchBuffer);
        m_searchTimer->Start(2000, true); // 2 second timeout
    }
    else
    {
        // Pass through other keys (arrows, etc.)
        event.Skip();
    }
}

void MainFrame::OnSearchTimer(wxTimerEvent& event)
{
    (void)event;
    // Clear search buffers and hide search displays
    m_sourceSearchBuffer.Clear();
    m_targetSearchBuffer.Clear();
    m_sourceSearchDisplay->Hide();
    m_targetSearchDisplay->Hide();
    GetSizer()->Layout();
}

void MainFrame::PerformSearch(wxControlWithItems* listCtrl, const wxString& searchText)
{
    if (searchText.IsEmpty()) return;
    
    wxString searchLower = searchText.Lower();
    unsigned int count = listCtrl->GetCount();
    
    for (unsigned int i = 0; i < count; ++i)
    {
        wxString itemText = listCtrl->GetString(i).Lower();
        if (itemText.StartsWith(searchLower))
        {
            listCtrl->SetSelection(i);
            // For list boxes, ensure the selected item is visible
            if (wxListBox* listBox = wxDynamicCast(listCtrl, wxListBox))
            {
                listBox->EnsureVisible(i);
            }
            else if (wxCheckListBox* checkListBox = wxDynamicCast(listCtrl, wxCheckListBox))
            {
                checkListBox->EnsureVisible(i);
            }
            break;
        }
    }
}

void MainFrame::EnableAllButtons(bool enable)
{
    // Main action buttons
    m_downloadBtn->Enable(enable);
    m_loadZipBtn->Enable(enable);
    m_generateBtn->Enable(enable);
    m_browseOutputBtn->Enable(enable);
    
    // Text controls
    m_urlText->Enable(enable);
    m_outputPath->Enable(enable);
}

void MainFrame::EnableLanguageControls(bool enable)
{
    m_sourceLanguage->Enable(enable);
    m_targetLanguages->Enable(enable);
    m_includeScientific->Enable(enable);
    m_filterOnlyTargets->Enable(enable);
}

void MainFrame::OnLoadTimer(wxTimerEvent& event)
{
    (void)event;
    if (!m_loader)
    {
        m_loadTimer->Stop();
        return;
    }
    
    if (m_loader->IsComplete())
    {
        m_loadTimer->Stop();
        
        if (m_loader->IsSuccess())
        {
            // Loading succeeded, populate UI
            std::vector<wxString> languages = m_processor->GetAvailableLanguages();
            
            // Sort languages alphabetically
            std::sort(languages.begin(), languages.end());
            
            m_sourceLanguage->Clear();
            m_targetLanguages->Clear();
            
            // Add Scientific Name as the first source language option
            m_sourceLanguage->Append(TR_DICT(DictStringId::ValueScientificName));

            // Add regular languages
            for (const auto& lang : languages)
            {
                m_sourceLanguage->Append(lang);
                m_targetLanguages->Append(lang);
            }

            // Re-enable all buttons and controls
            EnableAllButtons(true);
            EnableLanguageControls(true);

            // The final message from the callback should already be displayed via SafeUpdateStatus
            // No need to override it here as it contains the taxonomy entry count
        }
        else
        {
            m_statusText->SetLabel(TR_DICT(DictStringId::StatusLoadFailed));
            // Re-enable all buttons on failure
            EnableAllButtons(true);
            EnableLanguageControls(true);
            wxMessageBox(TR_DICT(DictStringId::ErrArchiveLoadFailed), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        }
        
        // Clean up loader
        if (m_loader->IsAlive()) {
            m_loader->Wait();
        }
        m_loader.reset();
    }
}

// ArchiveLoader implementation
ArchiveLoader::ArchiveLoader(TaxonomyProcessor* processor, const wxString& path, wxWindow* parent)
    : wxThread(wxTHREAD_JOINABLE)
    , m_processor(processor)
    , m_archivePath(path)
    , m_parent(parent)
    , m_success(false)
    , m_complete(false)
{
}

wxThread::ExitCode ArchiveLoader::Entry()
{
    // Create a callback that posts status updates to the main thread
    auto progressCallback = [this](const wxString& status, int percent) {
        if (m_parent && !TestDestroy()) {
            m_parent->CallAfter([this, status, percent]() {
                if (MainFrame* frame = wxDynamicCast(m_parent, MainFrame)) {
                    frame->SafeUpdateStatus(status);
                    frame->SafeUpdateProgress(percent);
                }
            });
        }
    };
    
    // Call the callback version of LoadArchive
    m_success = m_processor->LoadArchive(m_archivePath, progressCallback);
    m_complete = true;
    
    return (ExitCode)0;
}

void MainFrame::OnGenerateTimer(wxTimerEvent& event)
{
    (void)event;
    if (!m_dictThread)
    {
        m_generateTimer->Stop();
        return;
    }
    
    if (m_dictThread->IsComplete())
    {
        m_generateTimer->Stop();
        
        if (m_dictThread->IsSuccess())
        {
            // Generation succeeded
            // The final status message should already be displayed via SafeUpdateStatus
        }
        else
        {
            m_statusText->SetLabel(TR_DICT(DictStringId::StatusGenerateFailed));
            wxMessageBox(TR_DICT(DictStringId::ErrGenerationFailed), TR_DICT(DictStringId::DlgError), wxOK | wxICON_ERROR, this);
        }
        
        // Re-enable all buttons
        EnableAllButtons(true);
        EnableLanguageControls(true);
        
        // Clean up thread
        if (m_dictThread->IsAlive()) {
            m_dictThread->Wait();
        }
        m_dictThread.reset();
    }
}

// DictionaryGeneratorThread implementation
DictionaryGeneratorThread::DictionaryGeneratorThread(TaxonomyProcessor* processor, 
                                                   const wxString& sourceLanguage,
                                                   const std::vector<wxString>& targetLanguages,
                                                   bool includeScientific,
                                                   bool filterOnlyTargets,
                                                   const wxString& outputPath,
                                                   wxWindow* parent)
    : wxThread(wxTHREAD_JOINABLE)
    , m_processor(processor)
    , m_sourceLanguage(sourceLanguage)
    , m_targetLanguages(targetLanguages)
    , m_includeScientific(includeScientific)
    , m_filterOnlyTargets(filterOnlyTargets)
    , m_outputPath(outputPath)
    , m_parent(parent)
    , m_success(false)
    , m_complete(false)
{
}

wxThread::ExitCode DictionaryGeneratorThread::Entry()
{
    // Create a callback that posts status updates to the main thread
    auto progressCallback = [this](const wxString& status, int percent) {
        if (m_parent && !TestDestroy()) {
            m_parent->CallAfter([this, status, percent]() {
                if (MainFrame* frame = wxDynamicCast(m_parent, MainFrame)) {
                    frame->SafeUpdateStatus(status);
                    frame->SafeUpdateProgress(percent);
                }
            });
        }
    };
    
    // Call the callback version of GenerateDictionary
    m_success = m_processor->GenerateDictionary(m_sourceLanguage, m_targetLanguages, 
                                               m_includeScientific, m_filterOnlyTargets, 
                                               m_outputPath, progressCallback);
    m_complete = true;
    
    return (ExitCode)0;
}