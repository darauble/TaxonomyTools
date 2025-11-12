#include "TreeApp.hpp"
#include "TreeFrame.hpp"
#include "Translator.hpp"
#include <wx/image.h>
#include <wx/config.h>

wxIMPLEMENT_APP(TreeApp);

bool TreeApp::OnInit()
{
    // Initialize image handlers (required for loading PNG from memory)
    wxInitAllImageHandlers();

    // Load language settings from config
    wxConfig config("TaxonomyTree");
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

    m_frame = new TreeFrame("TaxonomyTree - Taxonomic Family Tree Viewer");
    m_frame->Show(true);
    return true;
}
