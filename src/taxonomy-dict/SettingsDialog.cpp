#include "SettingsDialog.hpp"
#include "DictStrings.hpp"
#include "TreeStrings.hpp"
#include <wx/config.h>
#include <wx/statbox.h>

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, SettingsDialog::OnCancel)
    EVT_CHECKBOX(ID_USE_SYSTEM_LANGUAGE, SettingsDialog::OnUseSystemLanguage)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(550, 300))
    , m_useSystemLanguage(true)
    , m_selectedLanguage(Language::English)
    , m_languageChanged(false)
{
    LoadSettings();

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Language Section
    wxStaticBoxSizer* languageBox = new wxStaticBoxSizer(wxVERTICAL, this, TR_TREE(TreeStringId::LabelLanguage));

    // Use system language checkbox
    m_useSystemLanguageCheckbox = new wxCheckBox(this, ID_USE_SYSTEM_LANGUAGE, TR_TREE(TreeStringId::LabelUseSystemLanguage));
    m_useSystemLanguageCheckbox->SetValue(m_useSystemLanguage);
    languageBox->Add(m_useSystemLanguageCheckbox, 0, wxALL, 10);

    // Language detection label
    m_languageDetectionLabel = new wxStaticText(this, wxID_ANY, "");
    wxFont detectionFont = m_languageDetectionLabel->GetFont();
    detectionFont.SetPointSize(detectionFont.GetPointSize() - 1);
    detectionFont.MakeItalic();
    m_languageDetectionLabel->SetFont(detectionFont);
    m_languageDetectionLabel->SetForegroundColour(wxColour(100, 100, 100));
    languageBox->Add(m_languageDetectionLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Language choice
    wxBoxSizer* langChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    langChoiceSizer->Add(new wxStaticText(this, wxID_ANY, TR_TREE(TreeStringId::LabelApplicationLanguage)), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    wxArrayString languages;
    languages.Add(wxString(Translator::GetLanguageNativeName(Language::English), wxConvUTF8));
    languages.Add(wxString(Translator::GetLanguageNativeName(Language::Lithuanian), wxConvUTF8));
    languages.Add(wxString(Translator::GetLanguageNativeName(Language::Spanish), wxConvUTF8));

    m_languageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, languages);
    m_languageChoice->SetSelection(static_cast<int>(m_selectedLanguage));
    langChoiceSizer->Add(m_languageChoice, 1, wxEXPAND);

    languageBox->Add(langChoiceSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    mainSizer->Add(languageBox, 0, wxEXPAND | wxALL, 10);

    // Update UI state
    m_languageChoice->Enable(!m_useSystemLanguage);
    UpdateLanguageDetectionLabel();

    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(new wxButton(this, wxID_OK, "OK"), 0, wxRIGHT, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0);

    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);

    SetSizer(mainSizer);
    Centre();
}

void SettingsDialog::LoadSettings()
{
    wxConfig config("TaxonomyDict");
    m_useSystemLanguage = config.Read("/Language/UseSystem", 1L) != 0;

    long langValue = config.Read("/Language/Selected", static_cast<long>(Language::English));
    m_selectedLanguage = static_cast<Language>(langValue);
}

void SettingsDialog::SaveSettings()
{
    wxConfig config("TaxonomyDict");
    config.Write("/Language/UseSystem", m_useSystemLanguage);
    config.Write("/Language/Selected", static_cast<long>(m_selectedLanguage));
    config.Flush();
}

void SettingsDialog::UpdateLanguageDetectionLabel()
{
    Language detectedLang = Translator::DetectSystemLanguage();
    const char* langName = Translator::GetLanguageNativeName(detectedLang);
    wxString langNameWx = wxString(langName, wxConvUTF8);
    wxString labelText = TR_TREE_FMT(TreeStringId::LabelDetectedLanguage, langNameWx); // wxString::Format("System language detected: %s", langNameWx);
    m_languageDetectionLabel->SetLabel(labelText);
}

void SettingsDialog::OnUseSystemLanguage(wxCommandEvent& event)
{
    bool useSystem = event.IsChecked();
    m_languageChoice->Enable(!useSystem);
}

void SettingsDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    // Check if language changed
    Language oldLanguage = TR_LANG();
    bool oldUseSystem = m_useSystemLanguage;

    m_useSystemLanguage = m_useSystemLanguageCheckbox->GetValue();
    int selectedIndex = m_languageChoice->GetSelection();
    if (selectedIndex != wxNOT_FOUND)
    {
        m_selectedLanguage = static_cast<Language>(selectedIndex);
    }

    // Determine new language
    Language newLanguage = m_useSystemLanguage ? Translator::DetectSystemLanguage() : m_selectedLanguage;

    // Check if language actually changed
    m_languageChanged = (newLanguage != oldLanguage) || (m_useSystemLanguage != oldUseSystem);

    if (m_languageChanged)
    {
        TR_SET_LANG(newLanguage);
    }

    SaveSettings();
    EndModal(wxID_OK);
}

void SettingsDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}
