#pragma once
// NF::Editor — trophy editor
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

enum class TrophyTier : uint8_t {
    Bronze, Silver, Gold, Platinum, Diamond
};

inline const char* trophyTierName(TrophyTier t) {
    switch (t) {
        case TrophyTier::Bronze:   return "Bronze";
        case TrophyTier::Silver:   return "Silver";
        case TrophyTier::Gold:     return "Gold";
        case TrophyTier::Platinum: return "Platinum";
        case TrophyTier::Diamond:  return "Diamond";
    }
    return "Unknown";
}

enum class TrophyType : uint8_t {
    Campaign, Multiplayer, Speedrun, Collection, Mastery, Community, Seasonal, Special
};

inline const char* trophyTypeName(TrophyType t) {
    switch (t) {
        case TrophyType::Campaign:    return "Campaign";
        case TrophyType::Multiplayer: return "Multiplayer";
        case TrophyType::Speedrun:    return "Speedrun";
        case TrophyType::Collection:  return "Collection";
        case TrophyType::Mastery:     return "Mastery";
        case TrophyType::Community:   return "Community";
        case TrophyType::Seasonal:    return "Seasonal";
        case TrophyType::Special:     return "Special";
    }
    return "Unknown";
}

enum class TrophyDisplayMode : uint8_t {
    Cabinet, Shelf, Wall, Showcase
};

inline const char* trophyDisplayModeName(TrophyDisplayMode d) {
    switch (d) {
        case TrophyDisplayMode::Cabinet:  return "Cabinet";
        case TrophyDisplayMode::Shelf:    return "Shelf";
        case TrophyDisplayMode::Wall:     return "Wall";
        case TrophyDisplayMode::Showcase: return "Showcase";
    }
    return "Unknown";
}

class TrophyDef {
public:
    explicit TrophyDef(uint32_t id, const std::string& name, TrophyTier tier)
        : m_id(id), m_name(name), m_tier(tier) {}

    void setType(TrophyType v)          { m_type        = v; }
    void setPointValue(uint32_t v)      { m_pointValue  = v; }
    void setIsRare(bool v)              { m_isRare      = v; }
    void setIsStackable(bool v)         { m_isStackable = v; }
    void setModelPath(const std::string& v) { m_modelPath = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] TrophyTier         tier()        const { return m_tier;        }
    [[nodiscard]] TrophyType         type()        const { return m_type;        }
    [[nodiscard]] uint32_t           pointValue()  const { return m_pointValue;  }
    [[nodiscard]] bool               isRare()      const { return m_isRare;      }
    [[nodiscard]] bool               isStackable() const { return m_isStackable; }
    [[nodiscard]] const std::string& modelPath()   const { return m_modelPath;   }

private:
    uint32_t     m_id;
    std::string  m_name;
    TrophyTier   m_tier;
    TrophyType   m_type        = TrophyType::Campaign;
    uint32_t     m_pointValue  = 50u;
    bool         m_isRare      = false;
    bool         m_isStackable = false;
    std::string  m_modelPath;
};

class TrophyEditor {
public:
    void setDisplayMode(TrophyDisplayMode v) { m_displayMode  = v; }
    void setShowLabels(bool v)               { m_showLabels   = v; }
    void setShowAnimations(bool v)           { m_showAnimations = v; }
    void setPreviewScale(float v)            { m_previewScale = v; }

    bool addTrophy(const TrophyDef& t) {
        for (auto& e : m_trophies) if (e.id() == t.id()) return false;
        m_trophies.push_back(t); return true;
    }
    bool removeTrophy(uint32_t id) {
        auto it = std::find_if(m_trophies.begin(), m_trophies.end(),
            [&](const TrophyDef& e){ return e.id() == id; });
        if (it == m_trophies.end()) return false;
        m_trophies.erase(it); return true;
    }
    [[nodiscard]] TrophyDef* findTrophy(uint32_t id) {
        for (auto& e : m_trophies) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] TrophyDisplayMode displayMode()     const { return m_displayMode;    }
    [[nodiscard]] bool              isShowLabels()    const { return m_showLabels;     }
    [[nodiscard]] bool              isShowAnimations()const { return m_showAnimations; }
    [[nodiscard]] float             previewScale()    const { return m_previewScale;   }
    [[nodiscard]] size_t            trophyCount()     const { return m_trophies.size();}

    [[nodiscard]] size_t countByTier(TrophyTier t) const {
        size_t n = 0; for (auto& e : m_trophies) if (e.tier() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByType(TrophyType t) const {
        size_t n = 0; for (auto& e : m_trophies) if (e.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countRare() const {
        size_t n = 0; for (auto& e : m_trophies) if (e.isRare()) ++n; return n;
    }

private:
    std::vector<TrophyDef> m_trophies;
    TrophyDisplayMode m_displayMode   = TrophyDisplayMode::Cabinet;
    bool              m_showLabels    = true;
    bool              m_showAnimations = true;
    float             m_previewScale  = 1.0f;
};

} // namespace NF
