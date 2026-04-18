#pragma once
// NF::Editor — reward system configuration editor
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

enum class RewardType : uint8_t { Currency, Item, XP, Cosmetic, Booster, Custom };
inline const char* rewardTypeName(RewardType v) {
    switch (v) {
        case RewardType::Currency: return "Currency";
        case RewardType::Item:     return "Item";
        case RewardType::XP:       return "XP";
        case RewardType::Cosmetic: return "Cosmetic";
        case RewardType::Booster:  return "Booster";
        case RewardType::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class RewardTrigger : uint8_t { LevelUp, QuestComplete, Achievement, Login, Purchase, Streak };
inline const char* rewardTriggerName(RewardTrigger v) {
    switch (v) {
        case RewardTrigger::LevelUp:       return "LevelUp";
        case RewardTrigger::QuestComplete: return "QuestComplete";
        case RewardTrigger::Achievement:   return "Achievement";
        case RewardTrigger::Login:         return "Login";
        case RewardTrigger::Purchase:      return "Purchase";
        case RewardTrigger::Streak:        return "Streak";
    }
    return "Unknown";
}

class RewardEntry {
public:
    explicit RewardEntry(uint32_t id, const std::string& name, RewardType type, RewardTrigger trigger)
        : m_id(id), m_name(name), m_type(type), m_trigger(trigger) {}

    void setQuantity(uint32_t v)    { m_quantity    = v; }
    void setIsStackable(bool v)     { m_isStackable = v; }
    void setIsEnabled(bool v)       { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] RewardType         type()        const { return m_type;        }
    [[nodiscard]] RewardTrigger      trigger()     const { return m_trigger;     }
    [[nodiscard]] uint32_t           quantity()    const { return m_quantity;    }
    [[nodiscard]] bool               isStackable() const { return m_isStackable; }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t      m_id;
    std::string   m_name;
    RewardType    m_type;
    RewardTrigger m_trigger;
    uint32_t m_quantity    = 1u;
    bool     m_isStackable = true;
    bool     m_isEnabled   = true;
};

class RewardSystemEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled   = v; }
    void setIsGroupByType(bool v)     { m_isGroupByType    = v; }
    void setGlobalMultiplier(float v) { m_globalMultiplier = v; }

    bool addReward(const RewardEntry& r) {
        for (auto& x : m_rewards) if (x.id() == r.id()) return false;
        m_rewards.push_back(r); return true;
    }
    bool removeReward(uint32_t id) {
        auto it = std::find_if(m_rewards.begin(), m_rewards.end(),
            [&](const RewardEntry& r){ return r.id() == id; });
        if (it == m_rewards.end()) return false;
        m_rewards.erase(it); return true;
    }
    [[nodiscard]] RewardEntry* findReward(uint32_t id) {
        for (auto& r : m_rewards) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()    const { return m_isShowDisabled;   }
    [[nodiscard]] bool   isGroupByType()     const { return m_isGroupByType;    }
    [[nodiscard]] float  globalMultiplier()  const { return m_globalMultiplier; }
    [[nodiscard]] size_t rewardCount()       const { return m_rewards.size();   }

    [[nodiscard]] size_t countByType(RewardType t) const {
        size_t n = 0; for (auto& r : m_rewards) if (r.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByTrigger(RewardTrigger t) const {
        size_t n = 0; for (auto& r : m_rewards) if (r.trigger() == t) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& r : m_rewards) if (r.isEnabled()) ++n; return n;
    }

private:
    std::vector<RewardEntry> m_rewards;
    bool  m_isShowDisabled   = false;
    bool  m_isGroupByType    = false;
    float m_globalMultiplier = 1.0f;
};

} // namespace NF
