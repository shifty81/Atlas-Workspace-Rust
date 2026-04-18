#pragma once
// NF::Editor — season pass tier management editor
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

enum class SeasonTier : uint8_t { Free, Bronze, Silver, Gold, Platinum, Custom };
inline const char* seasonTierName(SeasonTier v) {
    switch (v) {
        case SeasonTier::Free:     return "Free";
        case SeasonTier::Bronze:   return "Bronze";
        case SeasonTier::Silver:   return "Silver";
        case SeasonTier::Gold:     return "Gold";
        case SeasonTier::Platinum: return "Platinum";
        case SeasonTier::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class SeasonPassState : uint8_t { Upcoming, Active, Ending, Ended, Draft };
inline const char* seasonPassStateName(SeasonPassState v) {
    switch (v) {
        case SeasonPassState::Upcoming: return "Upcoming";
        case SeasonPassState::Active:   return "Active";
        case SeasonPassState::Ending:   return "Ending";
        case SeasonPassState::Ended:    return "Ended";
        case SeasonPassState::Draft:    return "Draft";
    }
    return "Unknown";
}

class SeasonPassEntry {
public:
    explicit SeasonPassEntry(uint32_t id, const std::string& name, SeasonTier tier)
        : m_id(id), m_name(name), m_tier(tier) {}

    void setState(SeasonPassState v)  { m_state       = v; }
    void setRewardCount(uint32_t v)   { m_rewardCount = v; }
    void setIsPremium(bool v)         { m_isPremium   = v; }
    void setIsEnabled(bool v)         { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] SeasonTier         tier()        const { return m_tier;        }
    [[nodiscard]] SeasonPassState    state()       const { return m_state;       }
    [[nodiscard]] uint32_t           rewardCount() const { return m_rewardCount; }
    [[nodiscard]] bool               isPremium()   const { return m_isPremium;   }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t      m_id;
    std::string   m_name;
    SeasonTier    m_tier;
    SeasonPassState m_state       = SeasonPassState::Draft;
    uint32_t        m_rewardCount = 0u;
    bool            m_isPremium   = false;
    bool            m_isEnabled   = true;
};

class SeasonPassEditor {
public:
    void setIsShowDraft(bool v)            { m_isShowDraft         = v; }
    void setIsGroupByTier(bool v)          { m_isGroupByTier       = v; }
    void setSeasonDurationDays(uint32_t v) { m_seasonDurationDays  = v; }

    bool addPassEntry(const SeasonPassEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removePassEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const SeasonPassEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] SeasonPassEntry* findPassEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDraft()          const { return m_isShowDraft;         }
    [[nodiscard]] bool     isGroupByTier()        const { return m_isGroupByTier;       }
    [[nodiscard]] uint32_t seasonDurationDays()   const { return m_seasonDurationDays;  }
    [[nodiscard]] size_t   entryCount()           const { return m_entries.size();      }

    [[nodiscard]] size_t countByTier(SeasonTier t) const {
        size_t n = 0; for (auto& e : m_entries) if (e.tier() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByState(SeasonPassState s) const {
        size_t n = 0; for (auto& e : m_entries) if (e.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countPremium() const {
        size_t n = 0; for (auto& e : m_entries) if (e.isPremium()) ++n; return n;
    }

private:
    std::vector<SeasonPassEntry> m_entries;
    bool     m_isShowDraft         = true;
    bool     m_isGroupByTier       = false;
    uint32_t m_seasonDurationDays  = 90u;
};

} // namespace NF
