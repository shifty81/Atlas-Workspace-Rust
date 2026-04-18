#pragma once
// NF::Editor — Trail editor v1: trail definition and keyframe management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Trev1RendererType : uint8_t { Line, Ribbon, Tube, Billboard, Mesh };
enum class Trev1TrailState   : uint8_t { Inactive, Active, Paused, Finished };

inline const char* trev1RendererTypeName(Trev1RendererType t) {
    switch (t) {
        case Trev1RendererType::Line:      return "Line";
        case Trev1RendererType::Ribbon:    return "Ribbon";
        case Trev1RendererType::Tube:      return "Tube";
        case Trev1RendererType::Billboard: return "Billboard";
        case Trev1RendererType::Mesh:      return "Mesh";
    }
    return "Unknown";
}

inline const char* trev1TrailStateName(Trev1TrailState s) {
    switch (s) {
        case Trev1TrailState::Inactive: return "Inactive";
        case Trev1TrailState::Active:   return "Active";
        case Trev1TrailState::Paused:   return "Paused";
        case Trev1TrailState::Finished: return "Finished";
    }
    return "Unknown";
}

struct Trev1Trail {
    uint64_t           id           = 0;
    std::string        name;
    Trev1RendererType  rendererType = Trev1RendererType::Ribbon;
    Trev1TrailState    state        = Trev1TrailState::Inactive;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Trev1TrailState::Active; }
    [[nodiscard]] bool isPaused()   const { return state == Trev1TrailState::Paused; }
    [[nodiscard]] bool isFinished() const { return state == Trev1TrailState::Finished; }
};

struct Trev1Keyframe {
    uint64_t id      = 0;
    uint64_t trailId = 0;
    float    time    = 0.f;

    [[nodiscard]] bool isValid() const { return id != 0 && trailId != 0; }
};

using Trev1ChangeCallback = std::function<void(uint64_t)>;

class TrailEditorV1 {
public:
    static constexpr size_t MAX_TRAILS    = 512;
    static constexpr size_t MAX_KEYFRAMES = 8192;

    bool addTrail(const Trev1Trail& trail) {
        if (!trail.isValid()) return false;
        for (const auto& t : m_trails) if (t.id == trail.id) return false;
        if (m_trails.size() >= MAX_TRAILS) return false;
        m_trails.push_back(trail);
        if (m_onChange) m_onChange(trail.id);
        return true;
    }

    bool removeTrail(uint64_t id) {
        for (auto it = m_trails.begin(); it != m_trails.end(); ++it) {
            if (it->id == id) { m_trails.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Trev1Trail* findTrail(uint64_t id) {
        for (auto& t : m_trails) if (t.id == id) return &t;
        return nullptr;
    }

    bool addKeyframe(const Trev1Keyframe& kf) {
        if (!kf.isValid()) return false;
        for (const auto& k : m_keyframes) if (k.id == kf.id) return false;
        if (m_keyframes.size() >= MAX_KEYFRAMES) return false;
        m_keyframes.push_back(kf);
        if (m_onChange) m_onChange(kf.trailId);
        return true;
    }

    bool removeKeyframe(uint64_t id) {
        for (auto it = m_keyframes.begin(); it != m_keyframes.end(); ++it) {
            if (it->id == id) { m_keyframes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t trailCount()    const { return m_trails.size(); }
    [[nodiscard]] size_t keyframeCount() const { return m_keyframes.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& t : m_trails) if (t.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t finishedCount() const {
        size_t c = 0; for (const auto& t : m_trails) if (t.isFinished()) ++c; return c;
    }
    [[nodiscard]] size_t countByRendererType(Trev1RendererType type) const {
        size_t c = 0; for (const auto& t : m_trails) if (t.rendererType == type) ++c; return c;
    }

    void setOnChange(Trev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Trev1Trail>    m_trails;
    std::vector<Trev1Keyframe> m_keyframes;
    Trev1ChangeCallback        m_onChange;
};

} // namespace NF
