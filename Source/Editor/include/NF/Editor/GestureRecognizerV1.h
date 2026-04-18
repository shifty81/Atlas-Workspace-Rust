#pragma once
// NF::Editor — Gesture recognizer v1: gesture event detection and dispatch
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class GestureType : uint8_t {
    Tap, DoubleTap, LongPress, Swipe, Pinch, Pan, Rotate
};
enum class GestureState : uint8_t {
    Possible, Began, Changed, Ended, Cancelled, Failed
};

struct GestureEvent {
    uint32_t     id          = 0;
    GestureType  type        = GestureType::Tap;
    GestureState state       = GestureState::Possible;
    float        x           = 0.f;
    float        y           = 0.f;
    float        dx          = 0.f;
    float        dy          = 0.f;
    float        scale       = 1.f;
    float        rotation    = 0.f;
    float        velocity    = 0.f;
    uint64_t     timestampMs = 0;

    [[nodiscard]] bool isValid()  const { return id != 0; }
    [[nodiscard]] bool isActive() const {
        return state == GestureState::Began || state == GestureState::Changed;
    }
};

using GestureCallback = std::function<void(const GestureEvent&)>;

class GestureRecognizerV1 {
public:
    static constexpr size_t MAX_GESTURES = 64;

    bool addGesture(const GestureEvent& ev) {
        if (!ev.isValid()) return false;
        if (m_gestures.size() >= MAX_GESTURES) return false;
        for (const auto& g : m_gestures) if (g.id == ev.id) return false;
        m_gestures.push_back(ev);
        return true;
    }

    bool removeGesture(uint32_t id) {
        for (auto it = m_gestures.begin(); it != m_gestures.end(); ++it) {
            if (it->id == id) { m_gestures.erase(it); return true; }
        }
        return false;
    }

    bool recognizeGesture(const GestureEvent& ev) {
        if (!ev.isValid()) return false;
        m_history.push_back(ev);
        if (m_onGesture) m_onGesture(ev);
        return true;
    }

    void setOnGesture(GestureCallback cb) { m_onGesture = std::move(cb); }

    void clearHistory() { m_history.clear(); }

    [[nodiscard]] size_t gestureCount() const { return m_gestures.size(); }
    [[nodiscard]] size_t eventCount()   const { return m_history.size();  }

private:
    std::vector<GestureEvent> m_gestures;
    std::vector<GestureEvent> m_history;
    GestureCallback           m_onGesture;
};

} // namespace NF
