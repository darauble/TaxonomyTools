#pragma once

#include <wx/wx.h>
#include <wx/filepicker.h>
#include "../common/Translator.hpp"

class SettingsDialog : public wxDialog
{
public:
    SettingsDialog(wxWindow* parent);

    wxString GetTaxonomyFilePath() const { return m_taxonomyFilePath; }
    bool GetLoadOnStartup() const { return m_loadOnStartup; }
    bool LanguageChanged() const { return m_languageChanged; }

private:
    void OnBrowse(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnUseSystemLanguage(wxCommandEvent& event);

    void LoadSettings();
    void SaveSettings();
    void UpdateLanguageDetectionLabel();

    wxTextCtrl* m_filePathTextCtrl;
    wxCheckBox* m_loadOnStartupCheckbox;
    wxCheckBox* m_useSystemLanguageCheckbox;
    wxChoice* m_languageChoice;
    wxStaticText* m_languageDetectionLabel;

    wxString m_taxonomyFilePath;
    bool m_loadOnStartup;
    bool m_useSystemLanguage;
    Language m_selectedLanguage;
    bool m_languageChanged;

    enum
    {
        ID_BROWSE_BUTTON = wxID_HIGHEST + 1000,
        ID_USE_SYSTEM_LANGUAGE
    };

    wxDECLARE_EVENT_TABLE();
};
