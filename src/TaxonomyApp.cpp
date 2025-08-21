#include "TaxonomyApp.hpp"
#include "MainFrame.hpp"
#include <wx/image.h>
#include <curl/curl.h>

wxIMPLEMENT_APP(TaxonomyApp);

bool TaxonomyApp::OnInit()
{
    // Initialize image handlers (required for loading PNG from memory)
    wxInitAllImageHandlers();
    
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