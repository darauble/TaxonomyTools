#include "TaxonomyApp.hpp"
#include "MainFrame.hpp"
#include "Translator.hpp"
#include <wx/image.h>
#include <wx/config.h>
#include <curl/curl.h>

wxIMPLEMENT_APP(TaxonomyApp);

bool TaxonomyApp::OnInit()
{
    // Initialize image handlers (required for loading PNG from memory)
    wxInitAllImageHandlers();

    // Load language settings from config
    wxConfig config("TaxonomyDict");
    bool useSystemLanguage = config.Read("/Language/UseSystem", 1L) != 0;
    long langValue = config.Read("/Language/Selected", static_cast<long>(Language::English));
    Language selectedLanguage = static_cast<Language>(langValue);

    // Initialize translation system based on settings
    if (useSystemLanguage)
    {
        TR_SET_LANG(Translator::DetectSystemLanguage());
    }
    else
    {
        TR_SET_LANG(selectedLanguage);
    }

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