#pragma once
// NF::Editor — Virtual camera editor v1: lens profile and tracking rig authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Vcv1LensProfile  : uint8_t { Wide, Normal, Telephoto, Macro, Fisheye, Anamorphic };
enum class Vcv1TrackingMode : uint8_t { Free, Locked, Orbit, Rail, Spline, Handheld };
enum class Vcv1CameraState  : uint8_t { Idle, Active, Streaming, Playback, Calibrating };

inline const char* vcv1LensProfileName(Vcv1LensProfile p) {
    switch (p) {
        case Vcv1LensProfile::Wide:       return "Wide";
        case Vcv1LensProfile::Normal:     return "Normal";
        case Vcv1LensProfile::Telephoto:  return "Telephoto";
        case Vcv1LensProfile::Macro:      return "Macro";
        case Vcv1LensProfile::Fisheye:    return "Fisheye";
        case Vcv1LensProfile::Anamorphic: return "Anamorphic";
    }
    return "Unknown";
}

inline const char* vcv1TrackingModeName(Vcv1TrackingMode m) {
    switch (m) {
        case Vcv1TrackingMode::Free:     return "Free";
        case Vcv1TrackingMode::Locked:   return "Locked";
        case Vcv1TrackingMode::Orbit:    return "Orbit";
        case Vcv1TrackingMode::Rail:     return "Rail";
        case Vcv1TrackingMode::Spline:   return "Spline";
        case Vcv1TrackingMode::Handheld: return "Handheld";
    }
    return "Unknown";
}

inline const char* vcv1CameraStateName(Vcv1CameraState s) {
    switch (s) {
        case Vcv1CameraState::Idle:        return "Idle";
        case Vcv1CameraState::Active:      return "Active";
        case Vcv1CameraState::Streaming:   return "Streaming";
        case Vcv1CameraState::Playback:    return "Playback";
        case Vcv1CameraState::Calibrating: return "Calibrating";
    }
    return "Unknown";
}

struct Vcv1CameraRig {
    uint64_t          id            = 0;
    std::string       name;
    std::string       targetEntity;
    Vcv1LensProfile   lensProfile   = Vcv1LensProfile::Normal;
    Vcv1TrackingMode  trackingMode  = Vcv1TrackingMode::Free;
    Vcv1CameraState   state         = Vcv1CameraState::Idle;
    float             focalLength   = 50.f;
    float             aperture      = 2.8f;
    bool              isLive        = false;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty() && focalLength > 0.f; }
    [[nodiscard]] bool isActive()    const { return state == Vcv1CameraState::Active; }
    [[nodiscard]] bool isStreaming() const { return state == Vcv1CameraState::Streaming; }
};

using Vcv1ChangeCallback = std::function<void(uint64_t)>;

class VirtualCameraEditorV1 {
public:
    static constexpr size_t MAX_RIGS = 32;

    bool addRig(const Vcv1CameraRig& rig) {
        if (!rig.isValid()) return false;
        for (const auto& r : m_rigs) if (r.id == rig.id) return false;
        if (m_rigs.size() >= MAX_RIGS) return false;
        m_rigs.push_back(rig);
        return true;
    }

    bool removeRig(uint64_t id) {
        for (auto it = m_rigs.begin(); it != m_rigs.end(); ++it) {
            if (it->id == id) {
                if (m_activeId == id) m_activeId = 0;
                m_rigs.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Vcv1CameraRig* findRig(uint64_t id) {
        for (auto& r : m_rigs) if (r.id == id) return &r;
        return nullptr;
    }

    bool setActive(uint64_t id) {
        for (const auto& r : m_rigs) if (r.id == id) { m_activeId = id; return true; }
        return false;
    }

    bool setState(uint64_t id, Vcv1CameraState state) {
        auto* r = findRig(id);
        if (!r) return false;
        r->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setLensProfile(uint64_t id, Vcv1LensProfile profile) {
        auto* r = findRig(id);
        if (!r) return false;
        r->lensProfile = profile;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setLive(uint64_t id, bool live) {
        auto* r = findRig(id);
        if (!r) return false;
        r->isLive = live;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setFocalLength(uint64_t id, float focal) {
        if (focal <= 0.f) return false;
        auto* r = findRig(id);
        if (!r) return false;
        r->focalLength = focal;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activeId()       const { return m_activeId; }
    [[nodiscard]] size_t   rigCount()       const { return m_rigs.size(); }
    [[nodiscard]] size_t   liveCount()      const {
        size_t c = 0; for (const auto& r : m_rigs) if (r.isLive)       ++c; return c;
    }
    [[nodiscard]] size_t   streamingCount() const {
        size_t c = 0; for (const auto& r : m_rigs) if (r.isStreaming()) ++c; return c;
    }
    [[nodiscard]] size_t   countByLens(Vcv1LensProfile profile) const {
        size_t c = 0; for (const auto& r : m_rigs) if (r.lensProfile == profile) ++c; return c;
    }
    [[nodiscard]] size_t   countByMode(Vcv1TrackingMode mode) const {
        size_t c = 0; for (const auto& r : m_rigs) if (r.trackingMode == mode) ++c; return c;
    }

    void setOnChange(Vcv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Vcv1CameraRig> m_rigs;
    uint64_t                   m_activeId = 0;
    Vcv1ChangeCallback         m_onChange;
};

} // namespace NF
