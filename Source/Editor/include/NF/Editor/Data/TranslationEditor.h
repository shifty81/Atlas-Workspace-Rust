#pragma once
// NF::Editor — per-locale translation entry management editor
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

enum class TranslationQuality : uint8_t { MachineTranslated, Draft, Reviewed, Approved, Certified };
inline const char* translationQualityName(TranslationQuality v) {
    switch (v) {
        case TranslationQuality::MachineTranslated: return "MachineTranslated";
        case TranslationQuality::Draft:             return "Draft";
        case TranslationQuality::Reviewed:          return "Reviewed";
        case TranslationQuality::Approved:          return "Approved";
        case TranslationQuality::Certified:         return "Certified";
    }
    return "Unknown";
}

enum class TranslationDirection : uint8_t { LTR, RTL, TTB };
inline const char* translationDirectionName(TranslationDirection v) {
    switch (v) {
        case TranslationDirection::LTR: return "LTR";
        case TranslationDirection::RTL: return "RTL";
        case TranslationDirection::TTB: return "TTB";
    }
    return "Unknown";
}

class TranslationRecord {
public:
    explicit TranslationRecord(uint32_t id, uint32_t keyId,
                                const std::string& locale, TranslationQuality quality)
        : m_id(id), m_keyId(keyId), m_locale(locale), m_quality(quality) {}

    void setDirection(TranslationDirection v) { m_direction = v; }
    void setIsPlural(bool v)                  { m_isPlural  = v; }
    void setIsEnabled(bool v)                 { m_isEnabled = v; }

    [[nodiscard]] uint32_t              id()        const { return m_id;        }
    [[nodiscard]] uint32_t              keyId()     const { return m_keyId;     }
    [[nodiscard]] const std::string&    locale()    const { return m_locale;    }
    [[nodiscard]] TranslationQuality    quality()   const { return m_quality;   }
    [[nodiscard]] TranslationDirection  direction() const { return m_direction; }
    [[nodiscard]] bool                  isPlural()  const { return m_isPlural;  }
    [[nodiscard]] bool                  isEnabled() const { return m_isEnabled; }

private:
    uint32_t             m_id;
    uint32_t             m_keyId;
    std::string          m_locale;
    TranslationQuality   m_quality;
    TranslationDirection m_direction = TranslationDirection::LTR;
    bool                 m_isPlural  = false;
    bool                 m_isEnabled = true;
};

class TranslationEditor {
public:
    void setIsShowDisabled(bool v)             { m_isShowDisabled  = v; }
    void setIsGroupByLocale(bool v)            { m_isGroupByLocale = v; }
    void setDefaultQuality(TranslationQuality v){ m_defaultQuality = v; }

    bool addEntry(const TranslationRecord& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const TranslationRecord& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] TranslationRecord* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool               isShowDisabled()  const { return m_isShowDisabled;  }
    [[nodiscard]] bool               isGroupByLocale() const { return m_isGroupByLocale; }
    [[nodiscard]] TranslationQuality defaultQuality()  const { return m_defaultQuality;  }
    [[nodiscard]] size_t             entryCount()      const { return m_entries.size();  }

    [[nodiscard]] size_t countByQuality(TranslationQuality q) const {
        size_t n = 0; for (auto& e : m_entries) if (e.quality() == q) ++n; return n;
    }
    [[nodiscard]] size_t countByDirection(TranslationDirection d) const {
        size_t n = 0; for (auto& e : m_entries) if (e.direction() == d) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_entries) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<TranslationRecord> m_entries;
    bool               m_isShowDisabled  = false;
    bool               m_isGroupByLocale = false;
    TranslationQuality m_defaultQuality  = TranslationQuality::Draft;
};

} // namespace NF
