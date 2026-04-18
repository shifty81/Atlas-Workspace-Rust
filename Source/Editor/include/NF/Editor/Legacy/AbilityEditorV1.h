#pragma once
// NF::Editor — Ability editor v1: ability definition and cost/effect management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Aeev1AbilityType  : uint8_t { Active, Passive, Toggle, Channelled, Reactive };
enum class Aeev1AbilityState : uint8_t { Draft, Enabled, Disabled, Locked };

inline const char* aeev1AbilityTypeName(Aeev1AbilityType t) {
    switch (t) {
        case Aeev1AbilityType::Active:     return "Active";
        case Aeev1AbilityType::Passive:    return "Passive";
        case Aeev1AbilityType::Toggle:     return "Toggle";
        case Aeev1AbilityType::Channelled: return "Channelled";
        case Aeev1AbilityType::Reactive:   return "Reactive";
    }
    return "Unknown";
}

inline const char* aeev1AbilityStateName(Aeev1AbilityState s) {
    switch (s) {
        case Aeev1AbilityState::Draft:    return "Draft";
        case Aeev1AbilityState::Enabled:  return "Enabled";
        case Aeev1AbilityState::Disabled: return "Disabled";
        case Aeev1AbilityState::Locked:   return "Locked";
    }
    return "Unknown";
}

struct Aeev1Ability {
    uint64_t          id          = 0;
    std::string       name;
    Aeev1AbilityType  abilityType = Aeev1AbilityType::Active;
    Aeev1AbilityState state       = Aeev1AbilityState::Draft;
    float             cooldownMs  = 0.0f;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEnabled()  const { return state == Aeev1AbilityState::Enabled; }
    [[nodiscard]] bool isDisabled() const { return state == Aeev1AbilityState::Disabled; }
    [[nodiscard]] bool isLocked()   const { return state == Aeev1AbilityState::Locked; }
};

struct Aeev1AbilityEffect {
    uint64_t    id        = 0;
    uint64_t    abilityId = 0;
    std::string name;
    float       magnitude = 0.0f;

    [[nodiscard]] bool isValid() const { return id != 0 && abilityId != 0 && !name.empty(); }
};

using Aeev1ChangeCallback = std::function<void(uint64_t)>;

class AbilityEditorV1 {
public:
    static constexpr size_t MAX_ABILITIES = 512;
    static constexpr size_t MAX_EFFECTS   = 2048;

    bool addAbility(const Aeev1Ability& ability) {
        if (!ability.isValid()) return false;
        for (const auto& a : m_abilities) if (a.id == ability.id) return false;
        if (m_abilities.size() >= MAX_ABILITIES) return false;
        m_abilities.push_back(ability);
        if (m_onChange) m_onChange(ability.id);
        return true;
    }

    bool removeAbility(uint64_t id) {
        for (auto it = m_abilities.begin(); it != m_abilities.end(); ++it) {
            if (it->id == id) { m_abilities.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Aeev1Ability* findAbility(uint64_t id) {
        for (auto& a : m_abilities) if (a.id == id) return &a;
        return nullptr;
    }

    bool addEffect(const Aeev1AbilityEffect& effect) {
        if (!effect.isValid()) return false;
        for (const auto& e : m_effects) if (e.id == effect.id) return false;
        if (m_effects.size() >= MAX_EFFECTS) return false;
        m_effects.push_back(effect);
        if (m_onChange) m_onChange(effect.id);
        return true;
    }

    bool removeEffect(uint64_t id) {
        for (auto it = m_effects.begin(); it != m_effects.end(); ++it) {
            if (it->id == id) { m_effects.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t abilityCount() const { return m_abilities.size(); }
    [[nodiscard]] size_t effectCount()  const { return m_effects.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (const auto& a : m_abilities) if (a.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& a : m_abilities) if (a.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Aeev1AbilityType type) const {
        size_t c = 0; for (const auto& a : m_abilities) if (a.abilityType == type) ++c; return c;
    }
    [[nodiscard]] size_t effectsForAbility(uint64_t abilityId) const {
        size_t c = 0; for (const auto& e : m_effects) if (e.abilityId == abilityId) ++c; return c;
    }

    void setOnChange(Aeev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Aeev1Ability>       m_abilities;
    std::vector<Aeev1AbilityEffect> m_effects;
    Aeev1ChangeCallback             m_onChange;
};

} // namespace NF
