#include "Translator.hpp"
#include <wx/wx.h>
#include <wx/intl.h>

Language Translator::DetectSystemLanguage() {
    int sysLangInt = wxLocale::GetSystemLanguage();
    wxLanguage sysLang = static_cast<wxLanguage>(sysLangInt);

    switch (sysLang) {
        case wxLANGUAGE_LITHUANIAN:
            return Language::Lithuanian;

        // Spanish variants
        case wxLANGUAGE_SPANISH:
        case wxLANGUAGE_SPANISH_ARGENTINA:
        case wxLANGUAGE_SPANISH_BOLIVIA:
        case wxLANGUAGE_SPANISH_CHILE:
        case wxLANGUAGE_SPANISH_COLOMBIA:
        case wxLANGUAGE_SPANISH_COSTA_RICA:
        case wxLANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
        case wxLANGUAGE_SPANISH_ECUADOR:
        case wxLANGUAGE_SPANISH_EL_SALVADOR:
        case wxLANGUAGE_SPANISH_GUATEMALA:
        case wxLANGUAGE_SPANISH_HONDURAS:
        case wxLANGUAGE_SPANISH_MEXICAN:
        case wxLANGUAGE_SPANISH_NICARAGUA:
        case wxLANGUAGE_SPANISH_PANAMA:
        case wxLANGUAGE_SPANISH_PARAGUAY:
        case wxLANGUAGE_SPANISH_PERU:
        case wxLANGUAGE_SPANISH_PUERTO_RICO:
        case wxLANGUAGE_SPANISH_URUGUAY:
        case wxLANGUAGE_SPANISH_US:
        case wxLANGUAGE_SPANISH_VENEZUELA:
            return Language::Spanish;

        default:
            // Default to English for all other languages
            return Language::English;
    }
}
