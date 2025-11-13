#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <utility>
#include <type_traits>
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

    // Helper for formatted strings - handles wxString arguments properly
    // This overload converts all wxString arguments to std::string
    template<typename... Args>
    static std::string Format(const char* fmt, const Args&... args) {
        // Store converted strings to ensure they live long enough
        auto converted = std::make_tuple(ConvertForFormat(args)...);
        return FormatWithTuple(fmt, converted, std::index_sequence_for<Args...>{});
    }

private:
    // Convert wxString to std::string for formatting
    static std::string ConvertForFormat(const wxString& arg) {
        return arg.ToStdString();
    }

    // Keep const char* as-is
    static const char* ConvertForFormat(const char* arg) {
        return arg;
    }

    // Keep std::string as-is
    static std::string ConvertForFormat(const std::string& arg) {
        return arg;
    }

    // Keep numeric types as-is
    template<typename T>
    static T ConvertForFormat(T arg) {
        return arg;
    }

    // Extract c_str() from std::string in tuple, pass others as-is
    template<typename T>
    static auto ExtractArg(const T& arg) -> decltype(auto) {
        if constexpr (std::is_same_v<T, std::string>) {
            return arg.c_str();
        } else {
            return arg;
        }
    }

    // Format with tuple of converted arguments
    template<typename Tuple, std::size_t... Is>
    static std::string FormatWithTuple(const char* fmt, Tuple& tuple, std::index_sequence<Is...>) {
        // Determine buffer size
        int size = std::snprintf(nullptr, 0, fmt, ExtractArg(std::get<Is>(tuple))...);
        if (size <= 0) {
            return std::string(fmt);
        }

        // Format into buffer
        std::vector<char> buffer(size + 1);
        std::snprintf(buffer.data(), buffer.size(), fmt, ExtractArg(std::get<Is>(tuple))...);
        return std::string(buffer.data(), size);
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
