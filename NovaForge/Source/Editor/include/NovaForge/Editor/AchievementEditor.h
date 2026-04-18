#pragma once
// NF::Editor — achievement editor
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

enum class AchievementCategory : uint8_t {
    Combat, Exploration, Collection, Social, Progression, Secret, Seasonal, Challenge
};

inline const char* achievementCategoryName(AchievementCategory c) {
    switch (c) {
        case AchievementCategory::Combat:      return "Combat";
        case AchievementCategory::Exploration: return "Exploration";
        case AchievementCategory::Collection:  return "Collection";
        case AchievementCategory::Social:      return "Social";
        case AchievementCategory::Progression: return "Progression";
        case AchievementCategory::Secret:      return "Secret";
        case AchievementCategory::Seasonal:    return "Seasonal";
        case AchievementCategory::Challenge:   return "Challenge";
    }
    return "Unknown";
}

enum class AchievementRarity : uint8_t {
    Common, Uncommon, Rare, Epic, Legendary
};

inline const char* achievementRarityName(AchievementRarity r) {
    switch (r) {
        case AchievementRarity::Common:    return "Common";
        case AchievementRarity::Uncommon:  return "Uncommon";
        case AchievementRarity::Rare:      return "Rare";
        case AchievementRarity::Epic:      return "Epic";
        case AchievementRarity::Legendary: return "Legendary";
    }
    return "Unknown";
}

enum class AchievementTrigger : uint8_t {
    Immediate, Cumulative, Threshold, Sequence, Timed, Composite
};

inline const char* achievementTriggerName(AchievementTrigger t) {
    switch (t) {
        case AchievementTrigger::Immediate:  return "Immediate";
        case AchievementTrigger::Cumulative: return "Cumulative";
        case AchievementTrigger::Threshold:  return "Threshold";
        case AchievementTrigger::Sequence:   return "Sequence";
        case AchievementTrigger::Timed:      return "Timed";
        case AchievementTrigger::Composite:  return "Composite";
    }
    return "Unknown";
}

class AchievementDef {
public:
    explicit AchievementDef(uint32_t id, const std::string& name, AchievementCategory category)
        : m_id(id), m_name(name), m_category(category) {}

    void setRarity(AchievementRarity v)    { m_rarity      = v; }
    void setTrigger(AchievementTrigger v)  { m_trigger     = v; }
    void setPointValue(uint32_t v)         { m_pointValue  = v; }
    void setIsHidden(bool v)               { m_isHidden    = v; }
    void setIsRepeatable(bool v)           { m_isRepeatable = v; }

    [[nodiscard]] uint32_t             id()           const { return m_id;          }
    [[nodiscard]] const std::string&   name()         const { return m_name;        }
    [[nodiscard]] AchievementCategory  category()     const { return m_category;    }
    [[nodiscard]] AchievementRarity    rarity()       const { return m_rarity;      }
    [[nodiscard]] AchievementTrigger   trigger()      const { return m_trigger;     }
    [[nodiscard]] uint32_t             pointValue()   const { return m_pointValue;  }
    [[nodiscard]] bool                 isHidden()     const { return m_isHidden;    }
    [[nodiscard]] bool                 isRepeatable() const { return m_isRepeatable;}

private:
    uint32_t             m_id;
    std::string          m_name;
    AchievementCategory  m_category;
    AchievementRarity    m_rarity       = AchievementRarity::Common;
    AchievementTrigger   m_trigger      = AchievementTrigger::Immediate;
    uint32_t             m_pointValue   = 10u;
    bool                 m_isHidden     = false;
    bool                 m_isRepeatable = false;
};

class AchievementEditor {
public:
    void setShowHidden(bool v)    { m_showHidden    = v; }
    void setShowProgress(bool v)  { m_showProgress  = v; }
    void setGroupByCategory(bool v) { m_groupByCategory = v; }

    bool addAchievement(const AchievementDef& a) {
        for (auto& e : m_achievements) if (e.id() == a.id()) return false;
        m_achievements.push_back(a); return true;
    }
    bool removeAchievement(uint32_t id) {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
            [&](const AchievementDef& e){ return e.id() == id; });
        if (it == m_achievements.end()) return false;
        m_achievements.erase(it); return true;
    }
    [[nodiscard]] AchievementDef* findAchievement(uint32_t id) {
        for (auto& e : m_achievements) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isShowHidden()      const { return m_showHidden;         }
    [[nodiscard]] bool   isShowProgress()    const { return m_showProgress;       }
    [[nodiscard]] bool   isGroupByCategory() const { return m_groupByCategory;    }
    [[nodiscard]] size_t achievementCount()  const { return m_achievements.size();}

    [[nodiscard]] size_t countByCategory(AchievementCategory c) const {
        size_t n = 0; for (auto& e : m_achievements) if (e.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByRarity(AchievementRarity r) const {
        size_t n = 0; for (auto& e : m_achievements) if (e.rarity() == r) ++n; return n;
    }
    [[nodiscard]] size_t countHidden() const {
        size_t n = 0; for (auto& e : m_achievements) if (e.isHidden()) ++n; return n;
    }

private:
    std::vector<AchievementDef> m_achievements;
    bool m_showHidden      = false;
    bool m_showProgress    = true;
    bool m_groupByCategory = true;
};

} // namespace NF
