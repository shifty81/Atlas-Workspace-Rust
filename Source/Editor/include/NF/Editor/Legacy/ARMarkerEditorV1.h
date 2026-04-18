#pragma once
// NF::Editor — AR marker editor v1: image target and spatial anchor authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Armv1MarkerType     : uint8_t { Image, QR, Spatial, Plane, Face, Object };
enum class Armv1TrackingState  : uint8_t { Lost, Initializing, Tracking, Limited, Paused };
enum class Armv1AnchorMode     : uint8_t { Fixed, Dynamic, Persistent, Session };

inline const char* armv1MarkerTypeName(Armv1MarkerType t) {
    switch (t) {
        case Armv1MarkerType::Image:   return "Image";
        case Armv1MarkerType::QR:      return "QR";
        case Armv1MarkerType::Spatial: return "Spatial";
        case Armv1MarkerType::Plane:   return "Plane";
        case Armv1MarkerType::Face:    return "Face";
        case Armv1MarkerType::Object:  return "Object";
    }
    return "Unknown";
}

inline const char* armv1TrackingStateName(Armv1TrackingState s) {
    switch (s) {
        case Armv1TrackingState::Lost:         return "Lost";
        case Armv1TrackingState::Initializing: return "Initializing";
        case Armv1TrackingState::Tracking:     return "Tracking";
        case Armv1TrackingState::Limited:      return "Limited";
        case Armv1TrackingState::Paused:       return "Paused";
    }
    return "Unknown";
}

inline const char* armv1AnchorModeName(Armv1AnchorMode m) {
    switch (m) {
        case Armv1AnchorMode::Fixed:      return "Fixed";
        case Armv1AnchorMode::Dynamic:    return "Dynamic";
        case Armv1AnchorMode::Persistent: return "Persistent";
        case Armv1AnchorMode::Session:    return "Session";
    }
    return "Unknown";
}

struct Armv1Marker {
    uint64_t            id             = 0;
    std::string         name;
    std::string         assetPath;
    Armv1MarkerType     markerType     = Armv1MarkerType::Image;
    Armv1TrackingState  trackingState  = Armv1TrackingState::Lost;
    Armv1AnchorMode     anchorMode     = Armv1AnchorMode::Session;
    float               physicalWidth  = 0.1f;
    float               confidence     = 0.f;
    bool                isEnabled      = true;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isTracking() const { return trackingState == Armv1TrackingState::Tracking; }
    [[nodiscard]] bool isLost()     const { return trackingState == Armv1TrackingState::Lost; }
};

using Armv1ChangeCallback = std::function<void(uint64_t)>;

class ARMarkerEditorV1 {
public:
    static constexpr size_t MAX_MARKERS = 128;

    bool addMarker(const Armv1Marker& marker) {
        if (!marker.isValid()) return false;
        for (const auto& m : m_markers) if (m.id == marker.id) return false;
        if (m_markers.size() >= MAX_MARKERS) return false;
        m_markers.push_back(marker);
        return true;
    }

    bool removeMarker(uint64_t id) {
        for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
            if (it->id == id) { m_markers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Armv1Marker* findMarker(uint64_t id) {
        for (auto& m : m_markers) if (m.id == id) return &m;
        return nullptr;
    }

    bool setTrackingState(uint64_t id, Armv1TrackingState state) {
        auto* m = findMarker(id);
        if (!m) return false;
        m->trackingState = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setEnabled(uint64_t id, bool enabled) {
        auto* m = findMarker(id);
        if (!m) return false;
        m->isEnabled = enabled;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setConfidence(uint64_t id, float confidence) {
        auto* m = findMarker(id);
        if (!m) return false;
        m->confidence = confidence;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t markerCount()   const { return m_markers.size(); }
    [[nodiscard]] size_t trackingCount() const {
        size_t c = 0; for (const auto& m : m_markers) if (m.isTracking()) ++c; return c;
    }
    [[nodiscard]] size_t lostCount()     const {
        size_t c = 0; for (const auto& m : m_markers) if (m.isLost())     ++c; return c;
    }
    [[nodiscard]] size_t enabledCount()  const {
        size_t c = 0; for (const auto& m : m_markers) if (m.isEnabled)    ++c; return c;
    }
    [[nodiscard]] size_t countByType(Armv1MarkerType type) const {
        size_t c = 0; for (const auto& m : m_markers) if (m.markerType == type) ++c; return c;
    }

    void setOnChange(Armv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Armv1Marker> m_markers;
    Armv1ChangeCallback      m_onChange;
};

} // namespace NF
