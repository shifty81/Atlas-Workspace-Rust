#pragma once
// NF::Editor — Event timeline v1: discrete event tracks with playback control
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class EtvEventType : uint8_t { Instant, Duration, Marker, Trigger };
enum class EtvPlayState : uint8_t { Stopped, Playing, Paused };

struct EtvEvent {
    uint64_t    id         = 0;
    std::string label;
    EtvEventType type      = EtvEventType::Instant;
    float        timeMs    = 0.f;
    float        durationMs= 0.f;
    bool         enabled   = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty(); }
};

using EtvEventCallback = std::function<void(const EtvEvent&)>;

class EventTimelineV1 {
public:
    bool addEvent(const EtvEvent& ev) {
        if (!ev.isValid()) return false;
        for (const auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    bool removeEvent(uint64_t id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    bool play() {
        if (m_state == EtvPlayState::Playing) return false;
        m_state = EtvPlayState::Playing;
        return true;
    }

    bool pause() {
        if (m_state != EtvPlayState::Playing) return false;
        m_state = EtvPlayState::Paused;
        return true;
    }

    bool stop() {
        if (m_state == EtvPlayState::Stopped) return false;
        m_state = EtvPlayState::Stopped;
        m_cursorMs = 0.f;
        return true;
    }

    bool seekTo(float ms) {
        if (ms < 0.f) return false;
        m_cursorMs = ms;
        return true;
    }

    void setDuration(float ms) { m_durationMs = ms; }

    [[nodiscard]] EtvPlayState playState()   const { return m_state;      }
    [[nodiscard]] float        cursor()      const { return m_cursorMs;   }
    [[nodiscard]] float        duration()    const { return m_durationMs; }
    [[nodiscard]] size_t       eventCount()  const { return m_events.size(); }

    void setOnFire(EtvEventCallback cb) { m_onFire = std::move(cb); }

    bool fireEvent(uint64_t id) {
        for (const auto& e : m_events) {
            if (e.id == id && e.enabled) {
                if (m_onFire) m_onFire(e);
                return true;
            }
        }
        return false;
    }

private:
    std::vector<EtvEvent> m_events;
    EtvPlayState          m_state     = EtvPlayState::Stopped;
    float                 m_cursorMs  = 0.f;
    float                 m_durationMs= 0.f;
    EtvEventCallback      m_onFire;
};

} // namespace NF
