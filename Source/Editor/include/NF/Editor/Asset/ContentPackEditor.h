#pragma once
// NF::Editor — content pack editor
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

enum class ContentPackCategory : uint8_t {
    DLC, Expansion, CosmeticPack, MapPack, SoundPack, StoryPack, SeasonPass, Free
};

inline const char* contentPackCategoryName(ContentPackCategory c) {
    switch (c) {
        case ContentPackCategory::DLC:          return "DLC";
        case ContentPackCategory::Expansion:    return "Expansion";
        case ContentPackCategory::CosmeticPack: return "CosmeticPack";
        case ContentPackCategory::MapPack:      return "MapPack";
        case ContentPackCategory::SoundPack:    return "SoundPack";
        case ContentPackCategory::StoryPack:    return "StoryPack";
        case ContentPackCategory::SeasonPass:   return "SeasonPass";
        case ContentPackCategory::Free:         return "Free";
    }
    return "Unknown";
}

enum class ContentPackStatus : uint8_t {
    Draft, InReview, Approved, Published, Deprecated, Recalled
};

inline const char* contentPackStatusName(ContentPackStatus s) {
    switch (s) {
        case ContentPackStatus::Draft:       return "Draft";
        case ContentPackStatus::InReview:    return "InReview";
        case ContentPackStatus::Approved:    return "Approved";
        case ContentPackStatus::Published:   return "Published";
        case ContentPackStatus::Deprecated:  return "Deprecated";
        case ContentPackStatus::Recalled:    return "Recalled";
    }
    return "Unknown";
}

enum class ContentPackPlatform : uint8_t {
    PC, Console, Mobile, All
};

inline const char* contentPackPlatformName(ContentPackPlatform p) {
    switch (p) {
        case ContentPackPlatform::PC:      return "PC";
        case ContentPackPlatform::Console: return "Console";
        case ContentPackPlatform::Mobile:  return "Mobile";
        case ContentPackPlatform::All:     return "All";
    }
    return "Unknown";
}

class ContentPackDef {
public:
    explicit ContentPackDef(uint32_t id, const std::string& name, ContentPackCategory category)
        : m_id(id), m_name(name), m_category(category) {}

    void setStatus(ContentPackStatus v)     { m_status    = v; }
    void setPlatform(ContentPackPlatform v) { m_platform  = v; }
    void setSizeMB(uint32_t v)             { m_sizeMB    = v; }
    void setIsFree(bool v)                 { m_isFree    = v; }
    void setVersion(const std::string& v)  { m_version   = v; }

    [[nodiscard]] uint32_t              id()        const { return m_id;       }
    [[nodiscard]] const std::string&    name()      const { return m_name;     }
    [[nodiscard]] ContentPackCategory   category()  const { return m_category; }
    [[nodiscard]] ContentPackStatus     status()    const { return m_status;   }
    [[nodiscard]] ContentPackPlatform   platform()  const { return m_platform; }
    [[nodiscard]] uint32_t              sizeMB()    const { return m_sizeMB;   }
    [[nodiscard]] bool                  isFree()    const { return m_isFree;   }
    [[nodiscard]] const std::string&    version()   const { return m_version;  }

private:
    uint32_t             m_id;
    std::string          m_name;
    ContentPackCategory  m_category;
    ContentPackStatus    m_status   = ContentPackStatus::Draft;
    ContentPackPlatform  m_platform = ContentPackPlatform::All;
    uint32_t             m_sizeMB   = 0u;
    bool                 m_isFree   = false;
    std::string          m_version  = "1.0.0";
};

class ContentPackEditor {
public:
    void setShowDeprecated(bool v)  { m_showDeprecated = v; }
    void setShowPreview(bool v)     { m_showPreview    = v; }
    void setFilterPlatform(ContentPackPlatform v) { m_filterPlatform = v; }

    bool addPack(const ContentPackDef& p) {
        for (auto& e : m_packs) if (e.id() == p.id()) return false;
        m_packs.push_back(p); return true;
    }
    bool removePack(uint32_t id) {
        auto it = std::find_if(m_packs.begin(), m_packs.end(),
            [&](const ContentPackDef& e){ return e.id() == id; });
        if (it == m_packs.end()) return false;
        m_packs.erase(it); return true;
    }
    [[nodiscard]] ContentPackDef* findPack(uint32_t id) {
        for (auto& e : m_packs) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool                isShowDeprecated() const { return m_showDeprecated; }
    [[nodiscard]] bool                isShowPreview()    const { return m_showPreview;    }
    [[nodiscard]] ContentPackPlatform filterPlatform()   const { return m_filterPlatform; }
    [[nodiscard]] size_t              packCount()        const { return m_packs.size();   }

    [[nodiscard]] size_t countByCategory(ContentPackCategory c) const {
        size_t n = 0; for (auto& e : m_packs) if (e.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(ContentPackStatus s) const {
        size_t n = 0; for (auto& e : m_packs) if (e.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countFree() const {
        size_t n = 0; for (auto& e : m_packs) if (e.isFree()) ++n; return n;
    }

private:
    std::vector<ContentPackDef> m_packs;
    bool                m_showDeprecated = false;
    bool                m_showPreview    = true;
    ContentPackPlatform m_filterPlatform = ContentPackPlatform::All;
};

} // namespace NF
