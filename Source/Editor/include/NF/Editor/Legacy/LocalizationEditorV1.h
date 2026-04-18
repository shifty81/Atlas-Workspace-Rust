#pragma once
// NF::Editor — Localization editor v1: multi-locale string management with status tracking
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class LevLocaleStatus : uint8_t { Missing, Draft, Reviewed, Approved };

struct LevString {
    uint32_t         id     = 0;
    std::string      key;
    std::string      locale;
    std::string      value;
    LevLocaleStatus  status = LevLocaleStatus::Draft;
    [[nodiscard]] bool isValid() const { return id != 0 && !key.empty() && !locale.empty(); }
};

struct LevLocale {
    std::string code;
    std::string name;
    bool        rtl  = false;
};

class LocalizationEditorV1 {
public:
    bool addLocale(const LevLocale& locale) {
        if (locale.code.empty()) return false;
        for (const auto& l : m_locales) if (l.code == locale.code) return false;
        m_locales.push_back(locale);
        return true;
    }

    bool removeLocale(const std::string& code) {
        for (auto it = m_locales.begin(); it != m_locales.end(); ++it) {
            if (it->code == code) { m_locales.erase(it); return true; }
        }
        return false;
    }

    bool addString(const LevString& str) {
        if (!str.isValid()) return false;
        for (const auto& s : m_strings)
            if (s.key == str.key && s.locale == str.locale) return false;
        m_strings.push_back(str);
        return true;
    }

    bool removeString(uint32_t id) {
        for (auto it = m_strings.begin(); it != m_strings.end(); ++it) {
            if (it->id == id) { m_strings.erase(it); return true; }
        }
        return false;
    }

    std::string getString(const std::string& key, const std::string& locale) const {
        for (const auto& s : m_strings)
            if (s.key == key && s.locale == locale) return s.value;
        return {};
    }

    bool setString(const std::string& key, const std::string& locale, const std::string& value) {
        for (auto& s : m_strings) {
            if (s.key == key && s.locale == locale) {
                s.value = value;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] size_t localeCount() const { return m_locales.size(); }

    [[nodiscard]] size_t stringCount(const std::string& locale) const {
        size_t n = 0;
        for (const auto& s : m_strings) if (s.locale == locale) ++n;
        return n;
    }

    [[nodiscard]] size_t missingCount(const std::string& locale) const {
        size_t n = 0;
        for (const auto& s : m_strings)
            if (s.locale == locale && s.status == LevLocaleStatus::Missing) ++n;
        return n;
    }

    void importFrom(const std::string& data) { m_importedData = data; }
    [[nodiscard]] std::string exportTo() const { return m_importedData; }

private:
    std::vector<LevLocale> m_locales;
    std::vector<LevString> m_strings;
    std::string            m_importedData;
};

} // namespace NF
