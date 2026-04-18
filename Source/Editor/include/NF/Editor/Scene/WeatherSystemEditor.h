#pragma once
// NF::Editor — weather system editor
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

enum class WeatherCondition : uint8_t {
    Clear, Cloudy, Overcast, Rain, Thunderstorm, Snow, Blizzard, Fog, Sandstorm, Hail
};

inline const char* weatherConditionName(WeatherCondition c) {
    switch (c) {
        case WeatherCondition::Clear:       return "Clear";
        case WeatherCondition::Cloudy:      return "Cloudy";
        case WeatherCondition::Overcast:    return "Overcast";
        case WeatherCondition::Rain:        return "Rain";
        case WeatherCondition::Thunderstorm:return "Thunderstorm";
        case WeatherCondition::Snow:        return "Snow";
        case WeatherCondition::Blizzard:    return "Blizzard";
        case WeatherCondition::Fog:         return "Fog";
        case WeatherCondition::Sandstorm:   return "Sandstorm";
        case WeatherCondition::Hail:        return "Hail";
    }
    return "Unknown";
}

enum class WindDirection : uint8_t {
    N, NE, E, SE, S, SW, W, NW
};

inline const char* windDirectionName(WindDirection d) {
    switch (d) {
        case WindDirection::N:  return "N";
        case WindDirection::NE: return "NE";
        case WindDirection::E:  return "E";
        case WindDirection::SE: return "SE";
        case WindDirection::S:  return "S";
        case WindDirection::SW: return "SW";
        case WindDirection::W:  return "W";
        case WindDirection::NW: return "NW";
    }
    return "Unknown";
}

enum class WeatherTransitionMode : uint8_t {
    Instant, Smooth, Random, Scripted
};

inline const char* weatherTransitionModeName(WeatherTransitionMode m) {
    switch (m) {
        case WeatherTransitionMode::Instant:  return "Instant";
        case WeatherTransitionMode::Smooth:   return "Smooth";
        case WeatherTransitionMode::Random:   return "Random";
        case WeatherTransitionMode::Scripted: return "Scripted";
    }
    return "Unknown";
}

class WeatherState {
public:
    explicit WeatherState(uint32_t id, const std::string& name, WeatherCondition condition)
        : m_id(id), m_name(name), m_condition(condition) {}

    void setWindDirection(WindDirection v)   { m_windDirection = v; }
    void setWindSpeed(float v)               { m_windSpeed = v; }
    void setTemperature(float v)             { m_temperature = v; }
    void setPrecipitation(float v)           { m_precipitation = v; }
    void setLightningEnabled(bool v)         { m_isLightningEnabled = v; }

    [[nodiscard]] uint32_t           id()                const { return m_id; }
    [[nodiscard]] const std::string& name()              const { return m_name; }
    [[nodiscard]] WeatherCondition   condition()         const { return m_condition; }
    [[nodiscard]] WindDirection      windDirection()     const { return m_windDirection; }
    [[nodiscard]] float              windSpeed()         const { return m_windSpeed; }
    [[nodiscard]] float              temperature()       const { return m_temperature; }
    [[nodiscard]] float              precipitation()     const { return m_precipitation; }
    [[nodiscard]] bool               isLightningEnabled() const { return m_isLightningEnabled; }

private:
    uint32_t         m_id;
    std::string      m_name;
    WeatherCondition m_condition         = WeatherCondition::Clear;
    WindDirection    m_windDirection     = WindDirection::N;
    float            m_windSpeed         = 0.0f;
    float            m_temperature       = 20.0f;
    float            m_precipitation     = 0.0f;
    bool             m_isLightningEnabled = false;
};

class WeatherSystemEditor {
public:
    bool addState(const WeatherState& state) {
        for (const auto& s : m_states)
            if (s.id() == state.id()) return false;
        m_states.push_back(state);
        return true;
    }

    bool removeState(uint32_t id) {
        for (auto it = m_states.begin(); it != m_states.end(); ++it) {
            if (it->id() == id) { m_states.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] WeatherState* findState(uint32_t id) {
        for (auto& s : m_states)
            if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] size_t stateCount() const { return m_states.size(); }

    [[nodiscard]] size_t countByCondition(WeatherCondition c) const {
        size_t n = 0;
        for (const auto& s : m_states) if (s.condition() == c) ++n;
        return n;
    }

    [[nodiscard]] size_t countWithLightning() const {
        size_t n = 0;
        for (const auto& s : m_states) if (s.isLightningEnabled()) ++n;
        return n;
    }

    void setTransitionMode(WeatherTransitionMode v) { m_transitionMode = v; }
    void setTransitionDuration(float v)             { m_transitionDuration = v; }
    void setActiveStateId(uint32_t v)               { m_activeStateId = v; }
    void setRealTimePreview(bool v)                 { m_isRealTimePreview = v; }

    [[nodiscard]] WeatherTransitionMode transitionMode()     const { return m_transitionMode; }
    [[nodiscard]] float                 transitionDuration() const { return m_transitionDuration; }
    [[nodiscard]] uint32_t              activeStateId()      const { return m_activeStateId; }
    [[nodiscard]] bool                  isRealTimePreview()  const { return m_isRealTimePreview; }

private:
    std::vector<WeatherState>  m_states;
    WeatherTransitionMode m_transitionMode     = WeatherTransitionMode::Smooth;
    float                 m_transitionDuration = 30.0f;
    uint32_t              m_activeStateId      = 0;
    bool                  m_isRealTimePreview  = false;
};

} // namespace NF
