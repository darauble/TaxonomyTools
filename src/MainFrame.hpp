#pragma once

#include <wx/wx.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/textctrl.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <memory>

class TaxonomyProcessor;
class Downloader;

// Simple loader thread for archive loading
class ArchiveLoader : public wxThread
{
public:
    ArchiveLoader(TaxonomyProcessor* processor, const wxString& path, wxWindow* parent);
    virtual ExitCode Entry() override;
    
    bool IsSuccess() const { return m_success; }
    bool IsComplete() const { return m_complete; }
    
private:
    TaxonomyProcessor* m_processor;
    wxString m_archivePath;
    wxWindow* m_parent;
    volatile bool m_success;
    volatile bool m_complete;
};

// Dictionary generator thread
class DictionaryGeneratorThread : public wxThread
{
public:
    DictionaryGeneratorThread(TaxonomyProcessor* processor, 
                             const wxString& sourceLanguage,
                             const std::vector<wxString>& targetLanguages,
                             bool includeScientific,
                             bool filterOnlyTargets,
                             const wxString& outputPath,
                             wxWindow* parent);
    virtual ExitCode Entry() override;
    
    bool IsSuccess() const { return m_success; }
    bool IsComplete() const { return m_complete; }
    
private:
    TaxonomyProcessor* m_processor;
    wxString m_sourceLanguage;
    std::vector<wxString> m_targetLanguages;
    bool m_includeScientific;
    bool m_filterOnlyTargets;
    wxString m_outputPath;
    wxWindow* m_parent;
    volatile bool m_success;
    volatile bool m_complete;
};

class MainFrame : public wxFrame
{
public:
    MainFrame();
    virtual ~MainFrame();
    
    // Thread-safe methods for progress updates (public for Downloader access)
    void SafeUpdateProgress(int progress);
    void SafeDownloadComplete(bool success, const wxString& message);
    void SafeUpdateStatus(const wxString& status);

private:
    enum
    {
        ID_Download = 1000,
        ID_LoadZip,
        ID_SelectLanguage,
        ID_GenerateDict,
        ID_DownloadTimer,
        ID_SearchTimer,
        ID_LoadTimer,
        ID_GenerateTimer
    };

    void CreateMenuBar();
    void CreateControls();
    void CreateLayout();

    // Event handlers
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnDownload(wxCommandEvent& event);
    void OnLoadZip(wxCommandEvent& event);
    void OnLanguageSelect(wxCommandEvent& event);
    void OnGenerateDict(wxCommandEvent& event);
    void OnDownloadProgress(wxThreadEvent& event);
    void OnDownloadComplete(wxThreadEvent& event);
    void OnDownloadTimer(wxTimerEvent& event);
    void OnSourceLanguageChar(wxKeyEvent& event);
    void OnTargetLanguageChar(wxKeyEvent& event);
    void OnSearchTimer(wxTimerEvent& event);
    void OnLoadTimer(wxTimerEvent& event);
    void OnGenerateTimer(wxTimerEvent& event);
    
    // Helper methods
    void PerformSearch(wxControlWithItems* listCtrl, const wxString& searchText);
    void EnableAllButtons(bool enable);
    void EnableLanguageControls(bool enable);

    // Controls
    wxTextCtrl* m_urlText;
    wxButton* m_downloadBtn;
    wxButton* m_loadZipBtn;
    wxStaticText* m_statusText;
    wxGauge* m_progressGauge;
    
    wxListBox* m_sourceLanguage;
    wxTextCtrl* m_sourceSearchDisplay;
    wxCheckListBox* m_targetLanguages;
    wxTextCtrl* m_targetSearchDisplay;
    wxCheckBox* m_includeScientific;
    wxCheckBox* m_filterOnlyTargets;
    
    wxButton* m_generateBtn;
    wxTextCtrl* m_outputPath;
    wxButton* m_browseOutputBtn;
    
    // Data members
    std::unique_ptr<TaxonomyProcessor> m_processor;
    std::unique_ptr<Downloader> m_downloader;
    wxString m_zipFilePath;
    wxTimer* m_downloadTimer;
    
    // Search functionality
    wxString m_sourceSearchBuffer;
    wxString m_targetSearchBuffer;
    wxTimer* m_searchTimer;
    
    // Archive loading
    std::unique_ptr<ArchiveLoader> m_loader;
    wxTimer* m_loadTimer;
    
    // Dictionary generation
    std::unique_ptr<DictionaryGeneratorThread> m_dictThread;
    wxTimer* m_generateTimer;

    wxDECLARE_EVENT_TABLE();
};