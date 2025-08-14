#pragma once

#include <wx/wx.h>
#include <wx/thread.h>
#include <string>
#include <functional>
#include <curl/curl.h>

class MainFrame; // Forward declaration

class Downloader : public wxThread
{
public:
    Downloader(const wxString& url, const wxString& outputPath);
    virtual ~Downloader();

    virtual ExitCode Entry() override;
    void Cancel() { m_cancelled = true; }
    
    // Polling interface for GUI
    int GetProgress() const { return m_currentProgress; }
    bool IsComplete() const { return m_isComplete; }
    bool IsSuccess() const { return m_isSuccess; }
    wxString GetStatusMessage() const { return m_statusMessage; }

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    wxString m_url;
    wxString m_outputPath;
    FILE* m_file;
    bool m_cancelled;
    
    // Shared state for polling (no direct communication with GUI)
    volatile int m_currentProgress;
    volatile bool m_isComplete;
    volatile bool m_isSuccess;
    wxString m_statusMessage;
};

// Custom events
wxDECLARE_EVENT(wxEVT_DOWNLOAD_PROGRESS, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_DOWNLOAD_COMPLETE, wxThreadEvent);