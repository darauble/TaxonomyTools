#include "Downloader.hpp"
#include "MainFrame.hpp"
#include <wx/filename.h>
#include <string>

wxDEFINE_EVENT(wxEVT_DOWNLOAD_PROGRESS, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_DOWNLOAD_COMPLETE, wxThreadEvent);

Downloader::Downloader(const wxString& url, const wxString& outputPath)
    : wxThread(wxTHREAD_JOINABLE)
    , m_url(url)
    , m_outputPath(outputPath)
    , m_file(nullptr)
    , m_cancelled(false)
    , m_currentProgress(0)
    , m_isComplete(false)
    , m_isSuccess(false)
{
    // Debug constructor parameters
    if (m_url.IsEmpty())
    {
    }
    if (m_outputPath.IsEmpty())
    {
    }
}

Downloader::~Downloader()
{
    if (m_file)
    {
        fclose(m_file);
    }
}

wxThread::ExitCode Downloader::Entry()
{
    
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        m_isComplete = true;
        m_isSuccess = false;
        m_statusMessage = "Failed to initialize CURL";
        return (ExitCode)1;
    }
    

    // Create output directory if it doesn't exist
    wxFileName outputFile(m_outputPath);
    if (!outputFile.GetPath().IsEmpty())
    {
        outputFile.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    m_file = fopen(m_outputPath.mb_str(), "wb");
    if (!m_file)
    {
        curl_easy_cleanup(curl);
        m_isComplete = true;
        m_isSuccess = false;
        m_statusMessage = "Failed to create output file";
        return (ExitCode)1;
    }

    // Convert URL to std::string for safety
    std::string urlStr = m_url.ToStdString();
    if (urlStr.empty())
    {
        curl_easy_cleanup(curl);
        m_isComplete = true;
        m_isSuccess = false;
        m_statusMessage = "URL conversion failed - empty URL";
        return (ExitCode)1;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, urlStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "TaxonomyDict/1.0");

    CURLcode res = curl_easy_perform(curl);
    
    fclose(m_file);
    m_file = nullptr;

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);

    // Update completion status for polling
    if (!m_cancelled)
    {
        m_isSuccess = (res == CURLE_OK && response_code == 200);
        
        if (m_isSuccess) {
            m_statusMessage = "Download completed successfully";
        } else if (res != CURLE_OK) {
            m_statusMessage = wxString::Format("Network error: %s", curl_easy_strerror(res));
        } else if (response_code == 404) {
            m_statusMessage = "File not found (404) - check URL";
        } else if (response_code >= 400) {
            m_statusMessage = wxString::Format("Server error: HTTP %ld", response_code);
        } else {
            m_statusMessage = wxString::Format("Download failed: %s (HTTP %ld)", curl_easy_strerror(res), response_code);
        }
        
        m_currentProgress = m_isSuccess ? 100 : 0;
        m_isComplete = true;
    }
    return (ExitCode)0;
}

size_t Downloader::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    Downloader* downloader = static_cast<Downloader*>(userp);
    if (downloader->m_cancelled || !downloader->m_file)
    {
        return 0;
    }
    
    size_t written = fwrite(contents, size, nmemb, downloader->m_file);
    
    return written;
}

int Downloader::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    (void)ultotal;  // Suppress unused parameter warning
    (void)ulnow;    // Suppress unused parameter warning
    Downloader* downloader = static_cast<Downloader*>(clientp);
    
    if (downloader->m_cancelled)
    {
        return 1; // Cancel download
    }


    // Update progress for polling (no GUI communication)
    if (dltotal > 0 && !downloader->m_cancelled)
    {
        int progress = static_cast<int>((dlnow * 100) / dltotal);
        downloader->m_currentProgress = progress;
    }
    else if (dlnow > 0)
    {
        // If we don't know total but are downloading, show some activity
        downloader->m_currentProgress = 50; // Show 50% as indeterminate progress
    }

    return 0;
}