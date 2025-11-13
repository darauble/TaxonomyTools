#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <functional>
#include <wx/string.h>

// Language enumeration
enum class Language {
    English = 0,
    Lithuanian = 1,
    Spanish = 2,
    COUNT
};

// Forward declarations for string ID enums (defined in each application)
enum class DictStringId;
enum class TreeStringId;

// Language change listener interface
class LanguageChangeListener {
public:
    virtual ~LanguageChangeListener() = default;
    virtual void OnLanguageChanged() = 0;
};

// Core translator class (singleton pattern)
class Translator {
public:
    // Get singleton instance
    static Translator& Instance() {
        static Translator instance;
        return instance;
    }

    // Set current language and notify listeners
    void SetLanguage(Language lang) {
        if (lang >= Language::COUNT) {
            lang = Language::English; // Fallback
        }
        m_currentLanguage = lang;
        NotifyListeners();
    }

    // Get current language
    Language GetLanguage() const {
        return m_currentLanguage;
    }

    // Detect system language using wxLocale (implemented in cpp)
    static Language DetectSystemLanguage();

    // Get language name in its native language
    static const char* GetLanguageNativeName(Language lang) {
        switch (lang) {
            case Language::English: return "English";
            case Language::Lithuanian: return "Lietuvių";
            case Language::Spanish: return "Español";
            default: return "English";
        }
    }

    // Register/unregister language change listeners
    void RegisterListener(LanguageChangeListener* listener) {
        m_listeners.push_back(listener);
    }

    void UnregisterListener(LanguageChangeListener* listener) {
        m_listeners.erase(
            std::remove(m_listeners.begin(), m_listeners.end(), listener),
            m_listeners.end()
        );
    }

    // Helper for formatted strings with positional parameters
    template<typename... Args>
    static std::string Format(const char* fmt, Args... args) {
        try {
            // Convert POSIX positional format to simple format
            wxString wxFmt = ConvertFormatString(fmt);

            // Use wxString::Format for cross-platform formatting
            wxString result = wxString::Format(wxFmt, args...);

            // Convert to std::string (UTF-8)
            return result.ToStdString();
        } catch (...) {
            // Fallback: return format string on any error
            return std::string(fmt);
        }
    }

private:
    // Convert POSIX positional format specifiers to simple format
    static wxString ConvertFormatString(const char* fmt) {
        wxString result(fmt, wxConvUTF8);

        // Remove positional parameter syntax (n$)
        // Works because all format strings use sequential parameters
        result.Replace(wxT("%1$"), wxT("%"));
        result.Replace(wxT("%2$"), wxT("%"));
        result.Replace(wxT("%3$"), wxT("%"));
        result.Replace(wxT("%4$"), wxT("%"));

        return result;
    }
    Translator() : m_currentLanguage(Language::English) {}

    void NotifyListeners() {
        for (auto* listener : m_listeners) {
            if (listener) {
                listener->OnLanguageChanged();
            }
        }
    }

    Language m_currentLanguage;
    std::vector<LanguageChangeListener*> m_listeners;

    // Prevent copying
    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;
};

// Convenience macros for translations
#define TR_LANG() Translator::Instance().GetLanguage()
#define TR_SET_LANG(lang) Translator::Instance().SetLanguage(lang)
