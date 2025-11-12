#pragma once

#include "Translator.hpp"
#include <cstdlib>

// Runtime validator for translation tables
// Ensures that all translations are present and no missing entries exist

template<typename StringIdEnum>
class TranslationValidator {
public:
    // Validate that translation table has correct size
    template<size_t N>
    static bool ValidateTableSize(const char* const (&)[N]) {
        return N == static_cast<size_t>(StringIdEnum::COUNT);
    }

    // Validate that no translation is nullptr or empty
    template<size_t N>
    static bool ValidateNoNullEntries(const char* const (&table)[N]) {
        for (size_t i = 0; i < N; ++i) {
            if (table[i] == nullptr || table[i][0] == '\0') {
                return false;
            }
        }
        return true;
    }

    // Validate entire translation table
    template<size_t N>
    static bool ValidateTable(const char* const (&table)[N]) {
        return ValidateTableSize(table) && ValidateNoNullEntries(table);
    }
};

// Macro to validate translation tables at runtime (during static initialization)
// If validation fails, the program will abort with an error message
#define VALIDATE_TRANSLATION_TABLE(StringIdEnum, table) \
    namespace { \
        struct TableValidator_##table { \
            TableValidator_##table() { \
                if (!TranslationValidator<StringIdEnum>::ValidateTableSize(table)) { \
                    std::abort(); \
                } \
                if (!TranslationValidator<StringIdEnum>::ValidateNoNullEntries(table)) { \
                    std::abort(); \
                } \
            } \
        }; \
        static TableValidator_##table validator_##table; \
    }
