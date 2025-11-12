#include "SettingsDialog.hpp"
#include "TreeStrings.hpp"
#include <wx/config.h>
#include <wx/statbox.h>
#include <wx/filedlg.h>

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(ID_BROWSE_BUTTON, SettingsDialog::OnBrowse)
    EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, SettingsDialog::OnCancel)
    EVT_CHECKBOX(ID_USE_SYSTEM_LANGUAGE, SettingsDialog::OnUseSystemLanguage)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, TR_TREE(TreeStringId::DlgSettings), wxDefaultPosition, wxSize(600, 400))
    , m_loadOnStartup(false)
    , m_useSystemLanguage(true)
    , m_selectedLanguage(Language::English)
    , m_languageChanged(false)
{
    LoadSettings();

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Taxonomy File Section
    wxStaticBoxSizer* taxonomyBox = new wxStaticBoxSizer(wxVERTICAL, this, TR_TREE(TreeStringId::SettingsBoxTaxonomyFile));

    // File path row
    wxBoxSizer* filePathSizer = new wxBoxSizer(wxHORIZONTAL);
    filePathSizer->Add(new wxStaticText(this, wxID_ANY, TR_TREE(TreeStringId::LabelFilePath)), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_filePathTextCtrl = new wxTextCtrl(this, wxID_ANY, m_taxonomyFilePath, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    filePathSizer->Add(m_filePathTextCtrl, 1, wxEXPAND | wxRIGHT, 10);

    wxButton* browseButton = new wxButton(this, ID_BROWSE_BUTTON, TR_TREE(TreeStringId::BtnBrowse));
    filePathSizer->Add(browseButton, 0, wxALIGN_CENTER_VERTICAL);

    taxonomyBox->Add(filePathSizer, 0, wxEXPAND | wxALL, 10);

    // Load on startup checkbox
    m_loadOnStartupCheckbox = new wxCheckBox(this, wxID_ANY, TR_TREE(TreeStringId::CheckLoadOnStartup));
    m_loadOnStartupCheckbox->SetValue(m_loadOnStartup);
    taxonomyBox->Add(m_loadOnStartupCheckbox, 0, wxALL, 10);

    mainSizer->Add(taxonomyBox, 0, wxEXPAND | wxALL, 10);

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
    langChoiceSizer->Add(new wxStaticText(this, wxID_ANY, TR_TREE(TreeStringId::LabelLanguage)), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    wxArrayString languages;
    languages.Add(wxString(Translator::GetLanguageNativeName(Language::English), wxConvUTF8));
    languages.Add(wxString(Translator::GetLanguageNativeName(Language::Lithuanian), wxConvUTF8));
    languages.Add(wxString(Translator::GetLanguageNativeName(Language::Spanish), wxConvUTF8));

    m_languageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, languages);
    m_languageChoice->SetSelection(static_cast<int>(m_selectedLanguage));
    langChoiceSizer->Add(m_languageChoice, 1, wxEXPAND);

    languageBox->Add(langChoiceSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    mainSizer->Add(languageBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Update UI state
    m_languageChoice->Enable(!m_useSystemLanguage);
    UpdateLanguageDetectionLabel();

    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(new wxButton(this, wxID_OK, TR_TREE(TreeStringId::BtnOK)), 0, wxRIGHT, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, TR_TREE(TreeStringId::BtnCancel)), 0);

    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);

    SetSizer(mainSizer);
    Centre();
}

void SettingsDialog::LoadSettings()
{
    wxConfig config("TaxonomyTree");
    m_taxonomyFilePath = config.Read("/Taxonomy/FilePath", wxEmptyString);
    m_loadOnStartup = config.Read("/Taxonomy/LoadOnStartup", 0L) != 0;
    m_useSystemLanguage = config.Read("/Language/UseSystem", 1L) != 0;

    long langValue = config.Read("/Language/Selected", static_cast<long>(Language::English));
    m_selectedLanguage = static_cast<Language>(langValue);
}

void SettingsDialog::SaveSettings()
{
    wxConfig config("TaxonomyTree");
    config.Write("/Taxonomy/FilePath", m_taxonomyFilePath);
    config.Write("/Taxonomy/LoadOnStartup", m_loadOnStartup);
    config.Write("/Language/UseSystem", m_useSystemLanguage);
    config.Write("/Language/Selected", static_cast<long>(m_selectedLanguage));
    config.Flush();
}

void SettingsDialog::UpdateLanguageDetectionLabel()
{
    Language detectedLang = Translator::DetectSystemLanguage();
    const char* langName = Translator::GetLanguageNativeName(detectedLang);
    wxString langNameWx = wxString(langName, wxConvUTF8);
    wxString labelText = TR_TREE_FMT(TreeStringId::LabelDetectedLanguage, langNameWx);
    m_languageDetectionLabel->SetLabel(labelText);
}

void SettingsDialog::OnBrowse(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog openDialog(this, TR_TREE(TreeStringId::DlgOpenTaxonomyArchive), "", "",
                           TR_TREE(TreeStringId::FilterZipFiles) + "|" + TR_TREE(TreeStringId::FilterAllFiles),
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    // Set initial directory to the current file path if it exists
    if (!m_taxonomyFilePath.IsEmpty() && wxFileExists(m_taxonomyFilePath))
    {
        wxFileName fn(m_taxonomyFilePath);
        openDialog.SetDirectory(fn.GetPath());
        openDialog.SetFilename(fn.GetFullName());
    }

    if (openDialog.ShowModal() == wxID_OK)
    {
        m_taxonomyFilePath = openDialog.GetPath();
        m_filePathTextCtrl->SetValue(m_taxonomyFilePath);
    }
}

void SettingsDialog::OnUseSystemLanguage(wxCommandEvent& event)
{
    bool useSystem = event.IsChecked();
    m_languageChoice->Enable(!useSystem);
}

void SettingsDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    m_taxonomyFilePath = m_filePathTextCtrl->GetValue();
    m_loadOnStartup = m_loadOnStartupCheckbox->GetValue();

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
