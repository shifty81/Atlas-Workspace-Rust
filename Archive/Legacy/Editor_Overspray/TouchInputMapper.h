#pragma once
// NF::Editor — Touch input mapper and gesture editor
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

enum class TouchGestureType : uint8_t {
    Tap, DoubleTap, LongPress, Swipe, Pinch, Rotate, Pan, EdgeSwipe
};

inline const char* touchGestureTypeName(TouchGestureType g) {
    switch (g) {
        case TouchGestureType::Tap:       return "Tap";
        case TouchGestureType::DoubleTap: return "DoubleTap";
        case TouchGestureType::LongPress: return "LongPress";
        case TouchGestureType::Swipe:     return "Swipe";
        case TouchGestureType::Pinch:     return "Pinch";
        case TouchGestureType::Rotate:    return "Rotate";
        case TouchGestureType::Pan:       return "Pan";
        case TouchGestureType::EdgeSwipe: return "EdgeSwipe";
    }
    return "Unknown";
}

enum class TouchZoneShape : uint8_t {
    FullScreen, Rect, Circle, Strip, Corner
};

inline const char* touchZoneShapeName(TouchZoneShape s) {
    switch (s) {
        case TouchZoneShape::FullScreen: return "FullScreen";
        case TouchZoneShape::Rect:       return "Rect";
        case TouchZoneShape::Circle:     return "Circle";
        case TouchZoneShape::Strip:      return "Strip";
        case TouchZoneShape::Corner:     return "Corner";
    }
    return "Unknown";
}

enum class TouchFingerCount : uint8_t {
    One = 1, Two = 2, Three = 3, Four = 4, Five = 5
};

inline const char* touchFingerCountName(TouchFingerCount f) {
    switch (f) {
        case TouchFingerCount::One:   return "One";
        case TouchFingerCount::Two:   return "Two";
        case TouchFingerCount::Three: return "Three";
        case TouchFingerCount::Four:  return "Four";
        case TouchFingerCount::Five:  return "Five";
    }
    return "Unknown";
}

class TouchBinding {
public:
    explicit TouchBinding(const std::string& actionName,
                           TouchGestureType gesture,
                           TouchZoneShape zone)
        : m_actionName(actionName), m_gesture(gesture), m_zone(zone) {}

    void setFingerCount(TouchFingerCount f) { m_fingers = f; }
    void setEnabled(bool v)                 { m_enabled = v; }
    void setThreshold(float t)              { m_threshold = t; }

    [[nodiscard]] const std::string& actionName()  const { return m_actionName; }
    [[nodiscard]] TouchGestureType   gesture()     const { return m_gesture;    }
    [[nodiscard]] TouchZoneShape     zone()        const { return m_zone;       }
    [[nodiscard]] TouchFingerCount   fingerCount() const { return m_fingers;    }
    [[nodiscard]] bool               isEnabled()   const { return m_enabled;    }
    [[nodiscard]] float              threshold()   const { return m_threshold;  }

private:
    std::string      m_actionName;
    TouchGestureType m_gesture;
    TouchZoneShape   m_zone;
    TouchFingerCount m_fingers   = TouchFingerCount::One;
    float            m_threshold = 0.1f;
    bool             m_enabled   = true;
};

class TouchInputMapperPanel {
public:
    static constexpr size_t MAX_BINDINGS = 128;

    [[nodiscard]] bool addBinding(const TouchBinding& b) {
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        for (auto& e : m_bindings)
            if (e.actionName() == b.actionName() && e.gesture() == b.gesture()) return false;
        m_bindings.push_back(b);
        return true;
    }

    [[nodiscard]] bool removeBinding(const std::string& actionName) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->actionName() == actionName) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TouchBinding* findBinding(const std::string& actionName) {
        for (auto& b : m_bindings) if (b.actionName() == actionName) return &b;
        return nullptr;
    }

    [[nodiscard]] size_t bindingCount() const { return m_bindings.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& b : m_bindings) if (b.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByGesture(TouchGestureType g) const {
        size_t c = 0; for (auto& b : m_bindings) if (b.gesture() == g) ++c; return c;
    }
    [[nodiscard]] size_t countByZone(TouchZoneShape z) const {
        size_t c = 0; for (auto& b : m_bindings) if (b.zone() == z) ++c; return c;
    }

private:
    std::vector<TouchBinding> m_bindings;
};

} // namespace NF
