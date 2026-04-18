#pragma once
// NF::Editor — Touch input mapper v1: gesture definition and action mapping management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Timv1GestureType  : uint8_t { Tap, DoubleTap, LongPress, Swipe, Pinch, Rotate, Pan };
enum class Timv1BindingState : uint8_t { Unbound, Bound, Disabled, Conflicting };

inline const char* timv1GestureTypeName(Timv1GestureType t) {
    switch (t) {
        case Timv1GestureType::Tap:       return "Tap";
        case Timv1GestureType::DoubleTap: return "DoubleTap";
        case Timv1GestureType::LongPress: return "LongPress";
        case Timv1GestureType::Swipe:     return "Swipe";
        case Timv1GestureType::Pinch:     return "Pinch";
        case Timv1GestureType::Rotate:    return "Rotate";
        case Timv1GestureType::Pan:       return "Pan";
    }
    return "Unknown";
}

inline const char* timv1BindingStateName(Timv1BindingState s) {
    switch (s) {
        case Timv1BindingState::Unbound:    return "Unbound";
        case Timv1BindingState::Bound:      return "Bound";
        case Timv1BindingState::Disabled:   return "Disabled";
        case Timv1BindingState::Conflicting:return "Conflicting";
    }
    return "Unknown";
}

struct Timv1Gesture {
    uint64_t           id          = 0;
    std::string        name;
    Timv1GestureType   gestureType = Timv1GestureType::Tap;
    Timv1BindingState  state       = Timv1BindingState::Unbound;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isBound()      const { return state == Timv1BindingState::Bound; }
    [[nodiscard]] bool isDisabled()   const { return state == Timv1BindingState::Disabled; }
    [[nodiscard]] bool isConflicting()const { return state == Timv1BindingState::Conflicting; }
};

struct Timv1Mapping {
    uint64_t    id        = 0;
    uint64_t    gestureId = 0;
    std::string action;

    [[nodiscard]] bool isValid() const { return id != 0 && gestureId != 0 && !action.empty(); }
};

using Timv1ChangeCallback = std::function<void(uint64_t)>;

class TouchInputMapperV1 {
public:
    static constexpr size_t MAX_GESTURES = 256;
    static constexpr size_t MAX_MAPPINGS = 1024;

    bool addGesture(const Timv1Gesture& gesture) {
        if (!gesture.isValid()) return false;
        for (const auto& g : m_gestures) if (g.id == gesture.id) return false;
        if (m_gestures.size() >= MAX_GESTURES) return false;
        m_gestures.push_back(gesture);
        if (m_onChange) m_onChange(gesture.id);
        return true;
    }

    bool removeGesture(uint64_t id) {
        for (auto it = m_gestures.begin(); it != m_gestures.end(); ++it) {
            if (it->id == id) { m_gestures.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Timv1Gesture* findGesture(uint64_t id) {
        for (auto& g : m_gestures) if (g.id == id) return &g;
        return nullptr;
    }

    bool addMapping(const Timv1Mapping& mapping) {
        if (!mapping.isValid()) return false;
        for (const auto& m : m_mappings) if (m.id == mapping.id) return false;
        if (m_mappings.size() >= MAX_MAPPINGS) return false;
        m_mappings.push_back(mapping);
        if (m_onChange) m_onChange(mapping.gestureId);
        return true;
    }

    bool removeMapping(uint64_t id) {
        for (auto it = m_mappings.begin(); it != m_mappings.end(); ++it) {
            if (it->id == id) { m_mappings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t gestureCount() const { return m_gestures.size(); }
    [[nodiscard]] size_t mappingCount() const { return m_mappings.size(); }

    [[nodiscard]] size_t boundCount() const {
        size_t c = 0; for (const auto& g : m_gestures) if (g.isBound()) ++c; return c;
    }
    [[nodiscard]] size_t conflictingCount() const {
        size_t c = 0; for (const auto& g : m_gestures) if (g.isConflicting()) ++c; return c;
    }
    [[nodiscard]] size_t countByGestureType(Timv1GestureType type) const {
        size_t c = 0; for (const auto& g : m_gestures) if (g.gestureType == type) ++c; return c;
    }

    void setOnChange(Timv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Timv1Gesture> m_gestures;
    std::vector<Timv1Mapping> m_mappings;
    Timv1ChangeCallback       m_onChange;
};

} // namespace NF
