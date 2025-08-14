#include "TaxonomyApp.hpp"
#include "MainFrame.hpp"
#include <curl/curl.h>

wxIMPLEMENT_APP(TaxonomyApp);

bool TaxonomyApp::OnInit()
{
    // Initialize libcurl globally
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        wxMessageBox("Failed to initialize CURL", "Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    MainFrame* frame = new MainFrame();
    frame->Show(true);
    return true;
}

int TaxonomyApp::OnExit()
{
    // Cleanup libcurl
    curl_global_cleanup();
    return wxApp::OnExit();
}