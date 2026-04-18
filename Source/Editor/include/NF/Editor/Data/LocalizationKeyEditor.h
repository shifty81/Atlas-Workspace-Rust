#pragma once
// NF::Editor — localization key/namespace management editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class L10nKeyCategory : uint8_t { UI, Gameplay, Narrative, System, Debug };
inline const char* l10nKeyCategoryName(L10nKeyCategory v) {
    switch (v) {
        case L10nKeyCategory::UI:        return "UI";
        case L10nKeyCategory::Gameplay:  return "Gameplay";
        case L10nKeyCategory::Narrative: return "Narrative";
        case L10nKeyCategory::System:    return "System";
        case L10nKeyCategory::Debug:     return "Debug";
    }
    return "Unknown";
}

enum class L10nKeyStatus : uint8_t { Draft, Review, Approved, Deprecated, Missing };
inline const char* l10nKeyStatusName(L10nKeyStatus v) {
    switch (v) {
        case L10nKeyStatus::Draft:      return "Draft";
        case L10nKeyStatus::Review:     return "Review";
        case L10nKeyStatus::Approved:   return "Approved";
        case L10nKeyStatus::Deprecated: return "Deprecated";
        case L10nKeyStatus::Missing:    return "Missing";
    }
    return "Unknown";
}

class L10nKey {
public:
    explicit L10nKey(uint32_t id, const std::string& name,
                     L10nKeyCategory category, L10nKeyStatus status)
        : m_id(id), m_name(name), m_category(category), m_status(status) {}

    void setDefaultText(const std::string& v) { m_defaultText = v; }
    void setMaxLength(uint32_t v)              { m_maxLength   = v; }
    void setIsEnabled(bool v)                  { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] L10nKeyCategory    category()    const { return m_category;    }
    [[nodiscard]] L10nKeyStatus      status()      const { return m_status;      }
    [[nodiscard]] const std::string& defaultText() const { return m_defaultText; }
    [[nodiscard]] uint32_t           maxLength()   const { return m_maxLength;   }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t        m_id;
    std::string     m_name;
    L10nKeyCategory m_category;
    L10nKeyStatus   m_status;
    std::string     m_defaultText = "";
    uint32_t        m_maxLength   = 256u;
    bool            m_isEnabled   = true;
};

class LocalizationKeyEditor {
public:
    void setIsShowDeprecated(bool v)        { m_isShowDeprecated       = v; }
    void setIsGroupByCategory(bool v)       { m_isGroupByCategory      = v; }
    void setWarnMissingTranslation(bool v)  { m_warnMissingTranslation = v; }

    bool addKey(const L10nKey& k) {
        for (auto& x : m_keys) if (x.id() == k.id()) return false;
        m_keys.push_back(k); return true;
    }
    bool removeKey(uint32_t id) {
        auto it = std::find_if(m_keys.begin(), m_keys.end(),
            [&](const L10nKey& k){ return k.id() == id; });
        if (it == m_keys.end()) return false;
        m_keys.erase(it); return true;
    }
    [[nodiscard]] L10nKey* findKey(uint32_t id) {
        for (auto& k : m_keys) if (k.id() == id) return &k;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDeprecated()       const { return m_isShowDeprecated;       }
    [[nodiscard]] bool   isGroupByCategory()      const { return m_isGroupByCategory;      }
    [[nodiscard]] bool   warnMissingTranslation() const { return m_warnMissingTranslation; }
    [[nodiscard]] size_t keyCount()               const { return m_keys.size();            }

    [[nodiscard]] size_t countByCategory(L10nKeyCategory c) const {
        size_t n = 0; for (auto& k : m_keys) if (k.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(L10nKeyStatus s) const {
        size_t n = 0; for (auto& k : m_keys) if (k.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& k : m_keys) if (k.isEnabled()) ++n; return n;
    }

private:
    std::vector<L10nKey> m_keys;
    bool m_isShowDeprecated       = false;
    bool m_isGroupByCategory      = true;
    bool m_warnMissingTranslation = true;
};

} // namespace NF
