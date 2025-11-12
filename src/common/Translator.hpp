#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <functional>

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
        // Calculate required buffer size
        int size = std::snprintf(nullptr, 0, fmt, args...) + 1;
        if (size <= 0) {
            return std::string(fmt); // Return format string on error
        }

        // Allocate and format
        std::vector<char> buffer(size);
        std::snprintf(buffer.data(), size, fmt, args...);
        return std::string(buffer.data(), buffer.data() + size - 1);
    }

private:
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
