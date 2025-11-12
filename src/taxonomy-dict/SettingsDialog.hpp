#pragma once

#include <wx/wx.h>
#include "../common/Translator.hpp"

class SettingsDialog : public wxDialog
{
public:
    SettingsDialog(wxWindow* parent);

    bool LanguageChanged() const { return m_languageChanged; }

private:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnUseSystemLanguage(wxCommandEvent& event);

    void LoadSettings();
    void SaveSettings();
    void UpdateLanguageDetectionLabel();

    wxCheckBox* m_useSystemLanguageCheckbox;
    wxChoice* m_languageChoice;
    wxStaticText* m_languageDetectionLabel;

    bool m_useSystemLanguage;
    Language m_selectedLanguage;
    bool m_languageChanged;

    enum
    {
        ID_USE_SYSTEM_LANGUAGE = wxID_HIGHEST + 2000
    };

    wxDECLARE_EVENT_TABLE();
};
