#pragma once
// NF::Editor — Camera rig editor v1: camera rail, crane, and follow-target authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Crv1RigType  : uint8_t { Rail, Crane, Orbit, Follow, Static, Shake };
enum class Crv1RigState : uint8_t { Active, Disabled, Preview, Recording };

inline const char* crv1RigTypeName(Crv1RigType t) {
    switch (t) {
        case Crv1RigType::Rail:   return "Rail";
        case Crv1RigType::Crane:  return "Crane";
        case Crv1RigType::Orbit:  return "Orbit";
        case Crv1RigType::Follow: return "Follow";
        case Crv1RigType::Static: return "Static";
        case Crv1RigType::Shake:  return "Shake";
    }
    return "Unknown";
}

inline const char* crv1RigStateName(Crv1RigState s) {
    switch (s) {
        case Crv1RigState::Active:    return "Active";
        case Crv1RigState::Disabled:  return "Disabled";
        case Crv1RigState::Preview:   return "Preview";
        case Crv1RigState::Recording: return "Recording";
    }
    return "Unknown";
}

struct Crv1Waypoint {
    uint64_t    id     = 0;
    float       posX   = 0.f;
    float       posY   = 0.f;
    float       posZ   = 0.f;
    float       time   = 0.f;  // time in seconds along the rig path

    [[nodiscard]] bool isValid() const { return id != 0; }
};

struct Crv1Rig {
    uint64_t       id          = 0;
    std::string    name;
    Crv1RigType    type        = Crv1RigType::Rail;
    Crv1RigState   state       = Crv1RigState::Active;
    float          fov         = 60.f;
    float          nearClip    = 0.1f;
    float          farClip     = 1000.f;
    float          speed       = 1.f;
    uint64_t       targetId    = 0; // follow-target entity
    std::vector<Crv1Waypoint> waypoints;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Crv1RigState::Active; }
};

using Crv1ChangeCallback = std::function<void(uint64_t)>;

class CameraRigEditorV1 {
public:
    static constexpr size_t MAX_RIGS = 64;

    bool addRig(const Crv1Rig& rig) {
        if (!rig.isValid()) return false;
        for (const auto& r : m_rigs) if (r.id == rig.id) return false;
        if (m_rigs.size() >= MAX_RIGS) return false;
        m_rigs.push_back(rig);
        if (m_onChange) m_onChange(rig.id);
        return true;
    }

    bool removeRig(uint64_t id) {
        for (auto it = m_rigs.begin(); it != m_rigs.end(); ++it) {
            if (it->id == id) { m_rigs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Crv1Rig* findRig(uint64_t id) {
        for (auto& r : m_rigs) if (r.id == id) return &r;
        return nullptr;
    }

    bool setState(uint64_t id, Crv1RigState state) {
        auto* r = findRig(id);
        if (!r) return false;
        r->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool addWaypoint(uint64_t rigId, const Crv1Waypoint& wp) {
        auto* r = findRig(rigId);
        if (!r || !wp.isValid()) return false;
        for (const auto& w : r->waypoints) if (w.id == wp.id) return false;
        r->waypoints.push_back(wp);
        if (m_onChange) m_onChange(rigId);
        return true;
    }

    bool setFov(uint64_t id, float fov) {
        auto* r = findRig(id);
        if (!r) return false;
        r->fov = std::clamp(fov, 10.f, 170.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t rigCount() const { return m_rigs.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& r : m_rigs) if (r.isActive()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Crv1RigType type) const {
        size_t c = 0;
        for (const auto& r : m_rigs) if (r.type == type) ++c;
        return c;
    }

    void setOnChange(Crv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Crv1Rig> m_rigs;
    Crv1ChangeCallback   m_onChange;
};

} // namespace NF
