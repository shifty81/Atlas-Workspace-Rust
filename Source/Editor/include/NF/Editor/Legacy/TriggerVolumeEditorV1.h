#pragma once
// NF::Editor — Trigger volume editor v1: box, sphere, cylinder and capsule triggers
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Tvv1TriggerType : uint8_t { Box, Sphere, Cylinder, Capsule };
enum class Tvv1TriggerState: uint8_t { Active, Disabled, Fired };

inline const char* tvv1TriggerTypeName(Tvv1TriggerType t) {
    switch (t) {
        case Tvv1TriggerType::Box:      return "Box";
        case Tvv1TriggerType::Sphere:   return "Sphere";
        case Tvv1TriggerType::Cylinder: return "Cylinder";
        case Tvv1TriggerType::Capsule:  return "Capsule";
    }
    return "Unknown";
}

inline const char* tvv1TriggerStateName(Tvv1TriggerState s) {
    switch (s) {
        case Tvv1TriggerState::Active:   return "Active";
        case Tvv1TriggerState::Disabled: return "Disabled";
        case Tvv1TriggerState::Fired:    return "Fired";
    }
    return "Unknown";
}

struct Tvv1Trigger {
    uint64_t         id        = 0;
    std::string      name;
    Tvv1TriggerType  type      = Tvv1TriggerType::Box;
    Tvv1TriggerState state     = Tvv1TriggerState::Active;
    float            sizeX     = 1.f;
    float            sizeY     = 1.f;
    float            sizeZ     = 1.f;
    uint32_t         layerMask = 0xFFFFFFFFu;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Tvv1TriggerState::Active; }
};

using Tvv1ChangeCallback = std::function<void(uint64_t)>;

class TriggerVolumeEditorV1 {
public:
    static constexpr size_t MAX_TRIGGERS = 256;

    bool addTrigger(const Tvv1Trigger& trigger) {
        if (!trigger.isValid()) return false;
        for (const auto& t : m_triggers) if (t.id == trigger.id) return false;
        if (m_triggers.size() >= MAX_TRIGGERS) return false;
        m_triggers.push_back(trigger);
        if (m_onChange) m_onChange(trigger.id);
        return true;
    }

    bool removeTrigger(uint64_t id) {
        for (auto it = m_triggers.begin(); it != m_triggers.end(); ++it) {
            if (it->id == id) { m_triggers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Tvv1Trigger* findTrigger(uint64_t id) {
        for (auto& t : m_triggers) if (t.id == id) return &t;
        return nullptr;
    }

    bool setState(uint64_t id, Tvv1TriggerState state) {
        auto* t = findTrigger(id);
        if (!t) return false;
        t->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setSize(uint64_t id, float sx, float sy, float sz) {
        auto* t = findTrigger(id);
        if (!t) return false;
        t->sizeX = std::max(0.01f, sx);
        t->sizeY = std::max(0.01f, sy);
        t->sizeZ = std::max(0.01f, sz);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setLayerMask(uint64_t id, uint32_t mask) {
        auto* t = findTrigger(id);
        if (!t) return false;
        t->layerMask = mask;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t triggerCount() const { return m_triggers.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& t : m_triggers) if (t.isActive()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Tvv1TriggerType type) const {
        size_t c = 0;
        for (const auto& t : m_triggers) if (t.type == type) ++c;
        return c;
    }

    void setOnChange(Tvv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Tvv1Trigger> m_triggers;
    Tvv1ChangeCallback       m_onChange;
};

} // namespace NF
