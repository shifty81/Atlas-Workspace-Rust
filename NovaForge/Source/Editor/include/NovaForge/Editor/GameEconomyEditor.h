#pragma once
// NF::Editor — game economy editor
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

enum class EconomyZone : uint8_t {
    EarlyGame, MidGame, LateGame, Endgame, Custom
};

inline const char* economyZoneName(EconomyZone z) {
    switch (z) {
        case EconomyZone::EarlyGame: return "EarlyGame";
        case EconomyZone::MidGame:   return "MidGame";
        case EconomyZone::LateGame:  return "LateGame";
        case EconomyZone::Endgame:   return "Endgame";
        case EconomyZone::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class BalanceParameter : uint8_t {
    ResourceGain, ResourceCost, DropRate, SpawnRate, DifficultyScale
};

inline const char* balanceParameterName(BalanceParameter p) {
    switch (p) {
        case BalanceParameter::ResourceGain:    return "ResourceGain";
        case BalanceParameter::ResourceCost:    return "ResourceCost";
        case BalanceParameter::DropRate:        return "DropRate";
        case BalanceParameter::SpawnRate:       return "SpawnRate";
        case BalanceParameter::DifficultyScale: return "DifficultyScale";
    }
    return "Unknown";
}

class EconomyRule {
public:
    explicit EconomyRule(uint32_t id, const std::string& name,
                         EconomyZone zone, BalanceParameter parameter)
        : m_id(id), m_name(name), m_zone(zone), m_parameter(parameter) {}

    void setMultiplier(float v) { m_multiplier = v; }
    void setIsActive(bool v)    { m_isActive   = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] EconomyZone        zone()       const { return m_zone;       }
    [[nodiscard]] BalanceParameter   parameter()  const { return m_parameter;  }
    [[nodiscard]] float              multiplier() const { return m_multiplier; }
    [[nodiscard]] bool               isActive()   const { return m_isActive;   }

private:
    uint32_t         m_id;
    std::string      m_name;
    EconomyZone      m_zone;
    BalanceParameter m_parameter;
    float            m_multiplier = 1.0f;
    bool             m_isActive   = true;
};

class GameEconomyEditor {
public:
    void setIsShowInactive(bool v)    { m_isShowInactive  = v; }
    void setIsGroupByZone(bool v)     { m_isGroupByZone   = v; }
    void setGlobalMultiplier(float v) { m_globalMultiplier = v; }

    bool addRule(const EconomyRule& r) {
        for (auto& x : m_rules) if (x.id() == r.id()) return false;
        m_rules.push_back(r); return true;
    }
    bool removeRule(uint32_t id) {
        auto it = std::find_if(m_rules.begin(), m_rules.end(),
            [&](const EconomyRule& r){ return r.id() == id; });
        if (it == m_rules.end()) return false;
        m_rules.erase(it); return true;
    }
    [[nodiscard]] EconomyRule* findRule(uint32_t id) {
        for (auto& r : m_rules) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool  isShowInactive()   const { return m_isShowInactive;  }
    [[nodiscard]] bool  isGroupByZone()    const { return m_isGroupByZone;   }
    [[nodiscard]] float globalMultiplier() const { return m_globalMultiplier; }
    [[nodiscard]] size_t ruleCount()       const { return m_rules.size();    }

    [[nodiscard]] size_t countByZone(EconomyZone z) const {
        size_t n = 0; for (auto& r : m_rules) if (r.zone() == z) ++n; return n;
    }
    [[nodiscard]] size_t countByParameter(BalanceParameter p) const {
        size_t n = 0; for (auto& r : m_rules) if (r.parameter() == p) ++n; return n;
    }
    [[nodiscard]] size_t countActive() const {
        size_t n = 0; for (auto& r : m_rules) if (r.isActive()) ++n; return n;
    }

private:
    std::vector<EconomyRule> m_rules;
    bool  m_isShowInactive  = false;
    bool  m_isGroupByZone   = false;
    float m_globalMultiplier = 1.0f;
};

} // namespace NF
