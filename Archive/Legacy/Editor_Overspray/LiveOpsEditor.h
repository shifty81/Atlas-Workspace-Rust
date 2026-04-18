#pragma once
// NF::Editor — live ops editor
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

enum class LiveOpsEventType : uint8_t {
    LimitedTimeOffer, SeasonalEvent, DailyChallenge, WeeklyChallenge,
    BonusXP, DoubleRewards, FlashSale, CommunityGoal
};

inline const char* liveOpsEventTypeName(LiveOpsEventType t) {
    switch (t) {
        case LiveOpsEventType::LimitedTimeOffer: return "LimitedTimeOffer";
        case LiveOpsEventType::SeasonalEvent:    return "SeasonalEvent";
        case LiveOpsEventType::DailyChallenge:   return "DailyChallenge";
        case LiveOpsEventType::WeeklyChallenge:  return "WeeklyChallenge";
        case LiveOpsEventType::BonusXP:          return "BonusXP";
        case LiveOpsEventType::DoubleRewards:    return "DoubleRewards";
        case LiveOpsEventType::FlashSale:        return "FlashSale";
        case LiveOpsEventType::CommunityGoal:    return "CommunityGoal";
    }
    return "Unknown";
}

enum class LiveOpsEventState : uint8_t {
    Draft, Scheduled, Active, Paused, Completed, Cancelled
};

inline const char* liveOpsEventStateName(LiveOpsEventState s) {
    switch (s) {
        case LiveOpsEventState::Draft:     return "Draft";
        case LiveOpsEventState::Scheduled: return "Scheduled";
        case LiveOpsEventState::Active:    return "Active";
        case LiveOpsEventState::Paused:    return "Paused";
        case LiveOpsEventState::Completed: return "Completed";
        case LiveOpsEventState::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

enum class LiveOpsTargetAudience : uint8_t {
    All, NewPlayers, Veterans, Churned, Whales, Regional, Custom
};

inline const char* liveOpsTargetAudienceName(LiveOpsTargetAudience a) {
    switch (a) {
        case LiveOpsTargetAudience::All:        return "All";
        case LiveOpsTargetAudience::NewPlayers:  return "NewPlayers";
        case LiveOpsTargetAudience::Veterans:    return "Veterans";
        case LiveOpsTargetAudience::Churned:     return "Churned";
        case LiveOpsTargetAudience::Whales:      return "Whales";
        case LiveOpsTargetAudience::Regional:    return "Regional";
        case LiveOpsTargetAudience::Custom:      return "Custom";
    }
    return "Unknown";
}

class LiveOpsEvent {
public:
    explicit LiveOpsEvent(uint32_t id, const std::string& name, LiveOpsEventType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setState(LiveOpsEventState v)           { m_state          = v; }
    void setTargetAudience(LiveOpsTargetAudience v) { m_targetAudience = v; }
    void setDurationHours(float v)               { m_durationHours  = v; }
    void setIsFeatured(bool v)                   { m_isFeatured     = v; }

    [[nodiscard]] uint32_t               id()             const { return m_id;             }
    [[nodiscard]] const std::string&     name()           const { return m_name;           }
    [[nodiscard]] LiveOpsEventType       type()           const { return m_type;           }
    [[nodiscard]] LiveOpsEventState      state()          const { return m_state;          }
    [[nodiscard]] LiveOpsTargetAudience  targetAudience() const { return m_targetAudience; }
    [[nodiscard]] float                  durationHours()  const { return m_durationHours;  }
    [[nodiscard]] bool                   isFeatured()     const { return m_isFeatured;     }

private:
    uint32_t              m_id;
    std::string           m_name;
    LiveOpsEventType      m_type;
    LiveOpsEventState     m_state          = LiveOpsEventState::Draft;
    LiveOpsTargetAudience m_targetAudience = LiveOpsTargetAudience::All;
    float                 m_durationHours  = 24.0f;
    bool                  m_isFeatured     = false;
};

class LiveOpsEditor {
public:
    void setShowInactive(bool v)         { m_showInactive  = v; }
    void setPreviewMode(bool v)          { m_previewMode   = v; }
    void setDefaultDurationHours(float v){ m_defaultDurationHours = v; }

    bool addEvent(const LiveOpsEvent& e) {
        for (auto& x : m_events) if (x.id() == e.id()) return false;
        m_events.push_back(e); return true;
    }
    bool removeEvent(uint32_t id) {
        auto it = std::find_if(m_events.begin(), m_events.end(),
            [&](const LiveOpsEvent& e){ return e.id() == id; });
        if (it == m_events.end()) return false;
        m_events.erase(it); return true;
    }
    [[nodiscard]] LiveOpsEvent* findEvent(uint32_t id) {
        for (auto& e : m_events) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool  isShowInactive()       const { return m_showInactive;       }
    [[nodiscard]] bool  isPreviewMode()        const { return m_previewMode;        }
    [[nodiscard]] float defaultDurationHours() const { return m_defaultDurationHours; }
    [[nodiscard]] size_t eventCount()          const { return m_events.size();      }

    [[nodiscard]] size_t countByType(LiveOpsEventType t) const {
        size_t n = 0; for (auto& e : m_events) if (e.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByState(LiveOpsEventState s) const {
        size_t n = 0; for (auto& e : m_events) if (e.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countFeatured() const {
        size_t n = 0; for (auto& e : m_events) if (e.isFeatured()) ++n; return n;
    }

private:
    std::vector<LiveOpsEvent> m_events;
    bool  m_showInactive        = false;
    bool  m_previewMode         = false;
    float m_defaultDurationHours = 24.0f;
};

} // namespace NF
