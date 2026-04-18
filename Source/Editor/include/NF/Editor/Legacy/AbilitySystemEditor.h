#pragma once
// NF::Editor — ability system editor
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

enum class AbilityTargetType : uint8_t {
    Self, Single, MultiTarget, AoE, Line, Cone, Chain, Global
};

inline const char* abilityTargetTypeName(AbilityTargetType t) {
    switch (t) {
        case AbilityTargetType::Self:        return "Self";
        case AbilityTargetType::Single:      return "Single";
        case AbilityTargetType::MultiTarget: return "MultiTarget";
        case AbilityTargetType::AoE:         return "AoE";
        case AbilityTargetType::Line:        return "Line";
        case AbilityTargetType::Cone:        return "Cone";
        case AbilityTargetType::Chain:       return "Chain";
        case AbilityTargetType::Global:      return "Global";
    }
    return "Unknown";
}

enum class AbilityResourceType : uint8_t {
    Mana, Stamina, Energy, Rage, Focus, Heat, None
};

inline const char* abilityResourceTypeName(AbilityResourceType t) {
    switch (t) {
        case AbilityResourceType::Mana:    return "Mana";
        case AbilityResourceType::Stamina: return "Stamina";
        case AbilityResourceType::Energy:  return "Energy";
        case AbilityResourceType::Rage:    return "Rage";
        case AbilityResourceType::Focus:   return "Focus";
        case AbilityResourceType::Heat:    return "Heat";
        case AbilityResourceType::None:    return "None";
    }
    return "Unknown";
}

enum class AbilityCategory : uint8_t {
    Active, Passive, Toggle, Channeled, Reaction
};

inline const char* abilityCategoryName(AbilityCategory c) {
    switch (c) {
        case AbilityCategory::Active:    return "Active";
        case AbilityCategory::Passive:   return "Passive";
        case AbilityCategory::Toggle:    return "Toggle";
        case AbilityCategory::Channeled: return "Channeled";
        case AbilityCategory::Reaction:  return "Reaction";
    }
    return "Unknown";
}

class AbilityEntry {
public:
    explicit AbilityEntry(uint32_t id, const std::string& name, AbilityCategory category)
        : m_id(id), m_name(name), m_category(category) {}

    void setCategory(AbilityCategory v)        { m_category = v; }
    void setTargetType(AbilityTargetType v)    { m_targetType = v; }
    void setResourceType(AbilityResourceType v){ m_resourceType = v; }
    void setCooldown(float v)                  { m_cooldown = v; }
    void setResourceCost(float v)              { m_resourceCost = v; }

    [[nodiscard]] uint32_t             id()           const { return m_id; }
    [[nodiscard]] const std::string&   name()         const { return m_name; }
    [[nodiscard]] AbilityCategory      category()     const { return m_category; }
    [[nodiscard]] AbilityTargetType    targetType()   const { return m_targetType; }
    [[nodiscard]] AbilityResourceType  resourceType() const { return m_resourceType; }
    [[nodiscard]] float                cooldown()     const { return m_cooldown; }
    [[nodiscard]] float                resourceCost() const { return m_resourceCost; }

private:
    uint32_t            m_id;
    std::string         m_name;
    AbilityCategory     m_category     = AbilityCategory::Active;
    AbilityTargetType   m_targetType   = AbilityTargetType::Single;
    AbilityResourceType m_resourceType = AbilityResourceType::Mana;
    float               m_cooldown     = 0.0f;
    float               m_resourceCost = 0.0f;
};

class AbilitySystemEditor {
public:
    bool addAbility(const AbilityEntry& ability) {
        for (const auto& a : m_abilities)
            if (a.id() == ability.id()) return false;
        m_abilities.push_back(ability);
        return true;
    }

    bool removeAbility(uint32_t id) {
        for (auto it = m_abilities.begin(); it != m_abilities.end(); ++it) {
            if (it->id() == id) { m_abilities.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] AbilityEntry* findAbility(uint32_t id) {
        for (auto& a : m_abilities)
            if (a.id() == id) return &a;
        return nullptr;
    }

    [[nodiscard]] size_t abilityCount() const { return m_abilities.size(); }

    [[nodiscard]] size_t countByCategory(AbilityCategory c) const {
        size_t n = 0;
        for (const auto& a : m_abilities) if (a.category() == c) ++n;
        return n;
    }

    [[nodiscard]] size_t countByTargetType(AbilityTargetType t) const {
        size_t n = 0;
        for (const auto& a : m_abilities) if (a.targetType() == t) ++n;
        return n;
    }

    void setMaxAbilities(uint32_t v)            { m_maxAbilities = v; }
    void setGlobalCooldownEnabled(bool v)       { m_globalCooldownEnabled = v; }
    void setComboSystemEnabled(bool v)          { m_comboSystemEnabled = v; }

    [[nodiscard]] uint32_t maxAbilities()           const { return m_maxAbilities; }
    [[nodiscard]] bool     isGlobalCooldownEnabled() const { return m_globalCooldownEnabled; }
    [[nodiscard]] bool     isComboSystemEnabled()    const { return m_comboSystemEnabled; }

private:
    std::vector<AbilityEntry> m_abilities;
    uint32_t m_maxAbilities          = 32;
    bool     m_globalCooldownEnabled = false;
    bool     m_comboSystemEnabled    = false;
};

} // namespace NF
