#pragma once
// NF::Editor — Decal editor v1: decal definition and placement management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Dev1DecalType  : uint8_t { Damage, Blood, Dirt, Graffiti, Burn, Wet, Custom };
enum class Dev1DecalState : uint8_t { Inactive, Active, Fading, Removed };

inline const char* dev1DecalTypeName(Dev1DecalType t) {
    switch (t) {
        case Dev1DecalType::Damage:   return "Damage";
        case Dev1DecalType::Blood:    return "Blood";
        case Dev1DecalType::Dirt:     return "Dirt";
        case Dev1DecalType::Graffiti: return "Graffiti";
        case Dev1DecalType::Burn:     return "Burn";
        case Dev1DecalType::Wet:      return "Wet";
        case Dev1DecalType::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* dev1DecalStateName(Dev1DecalState s) {
    switch (s) {
        case Dev1DecalState::Inactive: return "Inactive";
        case Dev1DecalState::Active:   return "Active";
        case Dev1DecalState::Fading:   return "Fading";
        case Dev1DecalState::Removed:  return "Removed";
    }
    return "Unknown";
}

struct Dev1Decal {
    uint64_t       id        = 0;
    std::string    name;
    Dev1DecalType  decalType = Dev1DecalType::Custom;
    Dev1DecalState state     = Dev1DecalState::Inactive;
    float          opacity   = 1.0f;
    float          fadeTime  = 0.0f;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()  const { return state == Dev1DecalState::Active; }
    [[nodiscard]] bool isFading()  const { return state == Dev1DecalState::Fading; }
    [[nodiscard]] bool isRemoved() const { return state == Dev1DecalState::Removed; }
};

struct Dev1PlacementZone {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Dev1ChangeCallback = std::function<void(uint64_t)>;

class DecalEditorV1 {
public:
    static constexpr size_t MAX_DECALS = 1024;
    static constexpr size_t MAX_ZONES  = 256;

    bool addDecal(const Dev1Decal& decal) {
        if (!decal.isValid()) return false;
        for (const auto& d : m_decals) if (d.id == decal.id) return false;
        if (m_decals.size() >= MAX_DECALS) return false;
        m_decals.push_back(decal);
        if (m_onChange) m_onChange(decal.id);
        return true;
    }

    bool removeDecal(uint64_t id) {
        for (auto it = m_decals.begin(); it != m_decals.end(); ++it) {
            if (it->id == id) { m_decals.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Dev1Decal* findDecal(uint64_t id) {
        for (auto& d : m_decals) if (d.id == id) return &d;
        return nullptr;
    }

    bool addZone(const Dev1PlacementZone& zone) {
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

    [[nodiscard]] size_t decalCount() const { return m_decals.size(); }
    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& d : m_decals) if (d.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Dev1DecalType type) const {
        size_t c = 0; for (const auto& d : m_decals) if (d.decalType == type) ++c; return c;
    }

    void setOnChange(Dev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Dev1Decal>         m_decals;
    std::vector<Dev1PlacementZone> m_zones;
    Dev1ChangeCallback             m_onChange;
};

} // namespace NF
