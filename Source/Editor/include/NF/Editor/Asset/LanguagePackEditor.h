#pragma once
// NF::Editor — distributable language pack management editor
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

enum class LangPackFormat : uint8_t { JSON, CSV, PO, XLIFF, Binary };
inline const char* langPackFormatName(LangPackFormat v) {
    switch (v) {
        case LangPackFormat::JSON:   return "JSON";
        case LangPackFormat::CSV:    return "CSV";
        case LangPackFormat::PO:     return "PO";
        case LangPackFormat::XLIFF:  return "XLIFF";
        case LangPackFormat::Binary: return "Binary";
    }
    return "Unknown";
}

enum class LangPackStatus : uint8_t { Incomplete, Complete, Verified, Published, Archived };
inline const char* langPackStatusName(LangPackStatus v) {
    switch (v) {
        case LangPackStatus::Incomplete: return "Incomplete";
        case LangPackStatus::Complete:   return "Complete";
        case LangPackStatus::Verified:   return "Verified";
        case LangPackStatus::Published:  return "Published";
        case LangPackStatus::Archived:   return "Archived";
    }
    return "Unknown";
}

class LanguagePack {
public:
    explicit LanguagePack(uint32_t id, const std::string& locale,
                           LangPackFormat format, LangPackStatus status)
        : m_id(id), m_locale(locale), m_format(format), m_status(status) {}

    void setCompletionPct(float v) { m_completionPct = v; }
    void setIsRTL(bool v)          { m_isRTL         = v; }
    void setIsEnabled(bool v)      { m_isEnabled     = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;            }
    [[nodiscard]] const std::string& locale()        const { return m_locale;        }
    [[nodiscard]] LangPackFormat     format()        const { return m_format;        }
    [[nodiscard]] LangPackStatus     status()        const { return m_status;        }
    [[nodiscard]] float              completionPct() const { return m_completionPct; }
    [[nodiscard]] bool               isRTL()         const { return m_isRTL;         }
    [[nodiscard]] bool               isEnabled()     const { return m_isEnabled;     }

private:
    uint32_t       m_id;
    std::string    m_locale;
    LangPackFormat m_format;
    LangPackStatus m_status;
    float          m_completionPct = 0.0f;
    bool           m_isRTL         = false;
    bool           m_isEnabled     = true;
};

class LanguagePackEditor {
public:
    void setIsShowArchived(bool v)      { m_isShowArchived    = v; }
    void setIsGroupByFormat(bool v)     { m_isGroupByFormat   = v; }
    void setMinCompletionPct(float v)   { m_minCompletionPct  = v; }

    bool addPack(const LanguagePack& p) {
        for (auto& x : m_packs) if (x.id() == p.id()) return false;
        m_packs.push_back(p); return true;
    }
    bool removePack(uint32_t id) {
        auto it = std::find_if(m_packs.begin(), m_packs.end(),
            [&](const LanguagePack& p){ return p.id() == id; });
        if (it == m_packs.end()) return false;
        m_packs.erase(it); return true;
    }
    [[nodiscard]] LanguagePack* findPack(uint32_t id) {
        for (auto& p : m_packs) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool   isShowArchived()   const { return m_isShowArchived;   }
    [[nodiscard]] bool   isGroupByFormat()  const { return m_isGroupByFormat;  }
    [[nodiscard]] float  minCompletionPct() const { return m_minCompletionPct; }
    [[nodiscard]] size_t packCount()        const { return m_packs.size();     }

    [[nodiscard]] size_t countByFormat(LangPackFormat f) const {
        size_t n = 0; for (auto& p : m_packs) if (p.format() == f) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(LangPackStatus s) const {
        size_t n = 0; for (auto& p : m_packs) if (p.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countComplete() const {
        size_t n = 0;
        for (auto& p : m_packs) if (p.completionPct() >= m_minCompletionPct) ++n;
        return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& p : m_packs) if (p.isEnabled()) ++n; return n;
    }

private:
    std::vector<LanguagePack> m_packs;
    bool  m_isShowArchived   = false;
    bool  m_isGroupByFormat  = false;
    float m_minCompletionPct = 80.0f;
};

} // namespace NF
