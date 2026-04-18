#pragma once
// NF::Editor — Cutscene editor v1: cutscene track and keyframe management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ccev1TrackType  : uint8_t { Camera, Actor, Audio, Event, Subtitle, VFX };
enum class Ccev1TrackState : uint8_t { Hidden, Visible, Locked, Solo };

inline const char* ccev1TrackTypeName(Ccev1TrackType t) {
    switch (t) {
        case Ccev1TrackType::Camera:   return "Camera";
        case Ccev1TrackType::Actor:    return "Actor";
        case Ccev1TrackType::Audio:    return "Audio";
        case Ccev1TrackType::Event:    return "Event";
        case Ccev1TrackType::Subtitle: return "Subtitle";
        case Ccev1TrackType::VFX:      return "VFX";
    }
    return "Unknown";
}

inline const char* ccev1TrackStateName(Ccev1TrackState s) {
    switch (s) {
        case Ccev1TrackState::Hidden:  return "Hidden";
        case Ccev1TrackState::Visible: return "Visible";
        case Ccev1TrackState::Locked:  return "Locked";
        case Ccev1TrackState::Solo:    return "Solo";
    }
    return "Unknown";
}

struct Ccev1CutsceneTrack {
    uint64_t        id        = 0;
    std::string     name;
    Ccev1TrackType  trackType = Ccev1TrackType::Camera;
    Ccev1TrackState state     = Ccev1TrackState::Hidden;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isVisible() const { return state == Ccev1TrackState::Visible; }
    [[nodiscard]] bool isLocked()  const { return state == Ccev1TrackState::Locked; }
    [[nodiscard]] bool isSolo()    const { return state == Ccev1TrackState::Solo; }
};

struct Ccev1CutsceneKeyframe {
    uint64_t    id      = 0;
    uint64_t    trackId = 0;
    float       timeMs  = 0.0f;
    std::string data;

    [[nodiscard]] bool isValid() const { return id != 0 && trackId != 0; }
};

using Ccev1ChangeCallback = std::function<void(uint64_t)>;

class CutsceneEditorV1 {
public:
    static constexpr size_t MAX_TRACKS    = 256;
    static constexpr size_t MAX_KEYFRAMES = 16384;

    bool addTrack(const Ccev1CutsceneTrack& track) {
        if (!track.isValid()) return false;
        for (const auto& t : m_tracks) if (t.id == track.id) return false;
        if (m_tracks.size() >= MAX_TRACKS) return false;
        m_tracks.push_back(track);
        if (m_onChange) m_onChange(track.id);
        return true;
    }

    bool removeTrack(uint64_t id) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->id == id) { m_tracks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ccev1CutsceneTrack* findTrack(uint64_t id) {
        for (auto& t : m_tracks) if (t.id == id) return &t;
        return nullptr;
    }

    bool addKeyframe(const Ccev1CutsceneKeyframe& kf) {
        if (!kf.isValid()) return false;
        for (const auto& k : m_keyframes) if (k.id == kf.id) return false;
        if (m_keyframes.size() >= MAX_KEYFRAMES) return false;
        m_keyframes.push_back(kf);
        if (m_onChange) m_onChange(kf.id);
        return true;
    }

    bool removeKeyframe(uint64_t id) {
        for (auto it = m_keyframes.begin(); it != m_keyframes.end(); ++it) {
            if (it->id == id) { m_keyframes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t trackCount()    const { return m_tracks.size(); }
    [[nodiscard]] size_t keyframeCount() const { return m_keyframes.size(); }

    [[nodiscard]] size_t visibleCount() const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.isVisible()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Ccev1TrackType type) const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.trackType == type) ++c; return c;
    }
    [[nodiscard]] size_t keyframesForTrack(uint64_t trackId) const {
        size_t c = 0; for (const auto& k : m_keyframes) if (k.trackId == trackId) ++c; return c;
    }

    void setOnChange(Ccev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ccev1CutsceneTrack>    m_tracks;
    std::vector<Ccev1CutsceneKeyframe> m_keyframes;
    Ccev1ChangeCallback                m_onChange;
};

} // namespace NF
