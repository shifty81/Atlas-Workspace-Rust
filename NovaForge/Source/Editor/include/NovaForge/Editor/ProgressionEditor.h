#pragma once
// NF::Editor — progression editor
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

enum class ProgressionCurveType : uint8_t {
    Linear, Exponential, Logarithmic, Flat, Custom
};

inline const char* progressionCurveTypeName(ProgressionCurveType t) {
    switch (t) {
        case ProgressionCurveType::Linear:      return "Linear";
        case ProgressionCurveType::Exponential: return "Exponential";
        case ProgressionCurveType::Logarithmic: return "Logarithmic";
        case ProgressionCurveType::Flat:        return "Flat";
        case ProgressionCurveType::Custom:      return "Custom";
    }
    return "Unknown";
}

enum class AttributeType : uint8_t {
    Health, Mana, Strength, Agility, Intelligence, Defense, Speed, Luck
};

inline const char* attributeTypeName(AttributeType t) {
    switch (t) {
        case AttributeType::Health:       return "Health";
        case AttributeType::Mana:         return "Mana";
        case AttributeType::Strength:     return "Strength";
        case AttributeType::Agility:      return "Agility";
        case AttributeType::Intelligence: return "Intelligence";
        case AttributeType::Defense:      return "Defense";
        case AttributeType::Speed:        return "Speed";
        case AttributeType::Luck:         return "Luck";
    }
    return "Unknown";
}

enum class LevelRewardType : uint8_t {
    AttributePoint, SkillPoint, Ability, Item, Currency, Feature
};

inline const char* levelRewardTypeName(LevelRewardType t) {
    switch (t) {
        case LevelRewardType::AttributePoint: return "AttributePoint";
        case LevelRewardType::SkillPoint:     return "SkillPoint";
        case LevelRewardType::Ability:        return "Ability";
        case LevelRewardType::Item:           return "Item";
        case LevelRewardType::Currency:       return "Currency";
        case LevelRewardType::Feature:        return "Feature";
    }
    return "Unknown";
}

class ProgressionLevel {
public:
    explicit ProgressionLevel(uint32_t level, uint64_t requiredXP)
        : m_level(level), m_requiredXP(requiredXP) {}

    void setRewardType(LevelRewardType v)  { m_rewardType = v; }
    void setRewardValue(uint32_t v)        { m_rewardValue = v; }
    void setAttributeType(AttributeType v) { m_attributeType = v; }

    [[nodiscard]] uint32_t        level()         const { return m_level; }
    [[nodiscard]] uint64_t        requiredXP()    const { return m_requiredXP; }
    [[nodiscard]] LevelRewardType rewardType()    const { return m_rewardType; }
    [[nodiscard]] uint32_t        rewardValue()   const { return m_rewardValue; }
    [[nodiscard]] AttributeType   attributeType() const { return m_attributeType; }

private:
    uint32_t        m_level;
    uint64_t        m_requiredXP;
    LevelRewardType m_rewardType   = LevelRewardType::AttributePoint;
    uint32_t        m_rewardValue  = 1u;
    AttributeType   m_attributeType = AttributeType::Health;
};

class ProgressionEditor {
public:
    bool addLevel(const ProgressionLevel& lvl) {
        for (const auto& l : m_levels)
            if (l.level() == lvl.level()) return false;
        m_levels.push_back(lvl);
        return true;
    }

    bool removeLevel(uint32_t level) {
        for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
            if (it->level() == level) { m_levels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ProgressionLevel* findLevel(uint32_t level) {
        for (auto& l : m_levels)
            if (l.level() == level) return &l;
        return nullptr;
    }

    [[nodiscard]] size_t levelCount() const { return m_levels.size(); }

    [[nodiscard]] size_t countByRewardType(LevelRewardType t) const {
        size_t n = 0;
        for (const auto& l : m_levels) if (l.rewardType() == t) ++n;
        return n;
    }

    [[nodiscard]] uint32_t maxLevel() const {
        uint32_t max = 0;
        for (const auto& l : m_levels) if (l.level() > max) max = l.level();
        return max;
    }

    void setCurveType(ProgressionCurveType v) { m_curveType = v; }
    void setBaseXP(uint64_t v)                { m_baseXP = v; }
    void setXpScaleFactor(float v)            { m_xpScaleFactor = v; }

    [[nodiscard]] ProgressionCurveType curveType()      const { return m_curveType; }
    [[nodiscard]] uint64_t             baseXP()         const { return m_baseXP; }
    [[nodiscard]] float                xpScaleFactor()  const { return m_xpScaleFactor; }

private:
    std::vector<ProgressionLevel> m_levels;
    ProgressionCurveType m_curveType     = ProgressionCurveType::Exponential;
    uint64_t             m_baseXP        = 100;
    float                m_xpScaleFactor = 1.5f;
};

} // namespace NF
