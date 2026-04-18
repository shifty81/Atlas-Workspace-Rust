#pragma once
// NF::Editor — VR interaction editor v1: interaction zone and hand input authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Vriv1ZoneType          : uint8_t { Grab, Touch, Proximity, Teleport, UI, Physics };
enum class Vriv1HandSide          : uint8_t { Left, Right, Both, None };
enum class Vriv1InteractionState  : uint8_t { Idle, Hovered, Active, Locked, Disabled };
enum class Vriv1HapticProfile     : uint8_t { None, Light, Medium, Heavy, Custom };

inline const char* vriv1ZoneTypeName(Vriv1ZoneType t) {
    switch (t) {
        case Vriv1ZoneType::Grab:      return "Grab";
        case Vriv1ZoneType::Touch:     return "Touch";
        case Vriv1ZoneType::Proximity: return "Proximity";
        case Vriv1ZoneType::Teleport:  return "Teleport";
        case Vriv1ZoneType::UI:        return "UI";
        case Vriv1ZoneType::Physics:   return "Physics";
    }
    return "Unknown";
}

inline const char* vriv1HandSideName(Vriv1HandSide s) {
    switch (s) {
        case Vriv1HandSide::Left:  return "Left";
        case Vriv1HandSide::Right: return "Right";
        case Vriv1HandSide::Both:  return "Both";
        case Vriv1HandSide::None:  return "None";
    }
    return "Unknown";
}

inline const char* vriv1InteractionStateName(Vriv1InteractionState s) {
    switch (s) {
        case Vriv1InteractionState::Idle:     return "Idle";
        case Vriv1InteractionState::Hovered:  return "Hovered";
        case Vriv1InteractionState::Active:   return "Active";
        case Vriv1InteractionState::Locked:   return "Locked";
        case Vriv1InteractionState::Disabled: return "Disabled";
    }
    return "Unknown";
}

struct Vriv1Zone {
    uint64_t               id            = 0;
    std::string            name;
    std::string            attachedEntity;
    Vriv1ZoneType          zoneType      = Vriv1ZoneType::Grab;
    Vriv1HandSide          handSide      = Vriv1HandSide::Both;
    Vriv1InteractionState  state         = Vriv1InteractionState::Idle;
    Vriv1HapticProfile     hapticProfile = Vriv1HapticProfile::Light;
    float                  radius        = 0.1f;
    bool                   isVisible     = true;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty() && radius > 0.f; }
    [[nodiscard]] bool isActive()   const { return state == Vriv1InteractionState::Active; }
    [[nodiscard]] bool isDisabled() const { return state == Vriv1InteractionState::Disabled; }
};

using Vriv1ChangeCallback = std::function<void(uint64_t)>;

class VRInteractionEditorV1 {
public:
    static constexpr size_t MAX_ZONES = 256;

    bool addZone(const Vriv1Zone& zone) {
        if (!zone.isValid()) return false;
        for (const auto& z : m_zones) if (z.id == zone.id) return false;
        if (m_zones.size() >= MAX_ZONES) return false;
        m_zones.push_back(zone);
        return true;
    }

    bool removeZone(uint64_t id) {
        for (auto it = m_zones.begin(); it != m_zones.end(); ++it) {
            if (it->id == id) { m_zones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Vriv1Zone* findZone(uint64_t id) {
        for (auto& z : m_zones) if (z.id == id) return &z;
        return nullptr;
    }

    bool setState(uint64_t id, Vriv1InteractionState state) {
        auto* z = findZone(id);
        if (!z) return false;
        z->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setHapticProfile(uint64_t id, Vriv1HapticProfile profile) {
        auto* z = findZone(id);
        if (!z) return false;
        z->hapticProfile = profile;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setVisible(uint64_t id, bool visible) {
        auto* z = findZone(id);
        if (!z) return false;
        z->isVisible = visible;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setRadius(uint64_t id, float radius) {
        if (radius <= 0.f) return false;
        auto* z = findZone(id);
        if (!z) return false;
        z->radius = radius;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t zoneCount()    const { return m_zones.size(); }
    [[nodiscard]] size_t activeCount()  const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isActive())   ++c; return c;
    }
    [[nodiscard]] size_t visibleCount() const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isVisible)    ++c; return c;
    }
    [[nodiscard]] size_t countByType(Vriv1ZoneType type) const {
        size_t c = 0; for (const auto& z : m_zones) if (z.zoneType == type) ++c; return c;
    }
    [[nodiscard]] size_t countByHand(Vriv1HandSide side) const {
        size_t c = 0; for (const auto& z : m_zones) if (z.handSide == side) ++c; return c;
    }

    void setOnChange(Vriv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Vriv1Zone> m_zones;
    Vriv1ChangeCallback    m_onChange;
};

} // namespace NF
