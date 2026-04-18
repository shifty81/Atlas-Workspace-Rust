#pragma once
// NF::Editor — Weather system editor v1: weather zone and condition transition management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wthv1Condition : uint8_t { Clear, Cloudy, Rain, Thunderstorm, Snow, Fog };
enum class Wthv1ZoneState : uint8_t { Inactive, Active, Transitioning, Locked, Disabled };

inline const char* wthv1ConditionName(Wthv1Condition c) {
    switch (c) {
        case Wthv1Condition::Clear:       return "Clear";
        case Wthv1Condition::Cloudy:      return "Cloudy";
        case Wthv1Condition::Rain:        return "Rain";
        case Wthv1Condition::Thunderstorm:return "Thunderstorm";
        case Wthv1Condition::Snow:        return "Snow";
        case Wthv1Condition::Fog:         return "Fog";
    }
    return "Unknown";
}

inline const char* wthv1ZoneStateName(Wthv1ZoneState s) {
    switch (s) {
        case Wthv1ZoneState::Inactive:     return "Inactive";
        case Wthv1ZoneState::Active:       return "Active";
        case Wthv1ZoneState::Transitioning:return "Transitioning";
        case Wthv1ZoneState::Locked:       return "Locked";
        case Wthv1ZoneState::Disabled:     return "Disabled";
    }
    return "Unknown";
}

struct Wthv1Zone {
    uint64_t        id        = 0;
    std::string     name;
    Wthv1Condition  condition = Wthv1Condition::Clear;
    Wthv1ZoneState  state     = Wthv1ZoneState::Inactive;

    [[nodiscard]] bool isValid()         const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()        const { return state == Wthv1ZoneState::Active; }
    [[nodiscard]] bool isTransitioning() const { return state == Wthv1ZoneState::Transitioning; }
    [[nodiscard]] bool isLocked()        const { return state == Wthv1ZoneState::Locked; }
};

struct Wthv1Transition {
    uint64_t       id            = 0;
    uint64_t       zoneId        = 0;
    Wthv1Condition fromCondition = Wthv1Condition::Clear;
    Wthv1Condition toCondition   = Wthv1Condition::Cloudy;

    [[nodiscard]] bool isValid() const { return id != 0 && zoneId != 0; }
};

using Wthv1ChangeCallback = std::function<void(uint64_t)>;

class WeatherSystemEditorV1 {
public:
    static constexpr size_t MAX_ZONES       = 256;
    static constexpr size_t MAX_TRANSITIONS = 1024;

    bool addZone(const Wthv1Zone& zone) {
        if (!zone.isValid()) return false;
        for (const auto& z : m_zones) if (z.id == zone.id) return false;
        if (m_zones.size() >= MAX_ZONES) return false;
        m_zones.push_back(zone);
        if (m_onChange) m_onChange(zone.id);
        return true;
    }

    bool removeZone(uint64_t id) {
        for (auto it = m_zones.begin(); it != m_zones.end(); ++it) {
            if (it->id == id) { m_zones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Wthv1Zone* findZone(uint64_t id) {
        for (auto& z : m_zones) if (z.id == id) return &z;
        return nullptr;
    }

    bool addTransition(const Wthv1Transition& tr) {
        if (!tr.isValid()) return false;
        for (const auto& t : m_transitions) if (t.id == tr.id) return false;
        if (m_transitions.size() >= MAX_TRANSITIONS) return false;
        m_transitions.push_back(tr);
        if (m_onChange) m_onChange(tr.zoneId);
        return true;
    }

    bool removeTransition(uint64_t id) {
        for (auto it = m_transitions.begin(); it != m_transitions.end(); ++it) {
            if (it->id == id) { m_transitions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t zoneCount()       const { return m_zones.size(); }
    [[nodiscard]] size_t transitionCount() const { return m_transitions.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByCondition(Wthv1Condition cond) const {
        size_t c = 0; for (const auto& z : m_zones) if (z.condition == cond) ++c; return c;
    }

    void setOnChange(Wthv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wthv1Zone>       m_zones;
    std::vector<Wthv1Transition> m_transitions;
    Wthv1ChangeCallback          m_onChange;
};

} // namespace NF
