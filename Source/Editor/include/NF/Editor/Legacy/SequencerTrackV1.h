#pragma once
// NF::Editor — Sequencer track v1: multi-track timeline lane authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Stv1TrackType    : uint8_t { Animation, Audio, Event, Property, Camera, FX };
enum class Stv1TrackState   : uint8_t { Active, Muted, Solo, Locked };
enum class Stv1BlendMode    : uint8_t { Override, Additive, Multiply, Average };

inline const char* stv1TrackTypeName(Stv1TrackType t) {
    switch (t) {
        case Stv1TrackType::Animation: return "Animation";
        case Stv1TrackType::Audio:     return "Audio";
        case Stv1TrackType::Event:     return "Event";
        case Stv1TrackType::Property:  return "Property";
        case Stv1TrackType::Camera:    return "Camera";
        case Stv1TrackType::FX:        return "FX";
    }
    return "Unknown";
}

inline const char* stv1TrackStateName(Stv1TrackState s) {
    switch (s) {
        case Stv1TrackState::Active: return "Active";
        case Stv1TrackState::Muted:  return "Muted";
        case Stv1TrackState::Solo:   return "Solo";
        case Stv1TrackState::Locked: return "Locked";
    }
    return "Unknown";
}

inline const char* stv1BlendModeName(Stv1BlendMode m) {
    switch (m) {
        case Stv1BlendMode::Override: return "Override";
        case Stv1BlendMode::Additive: return "Additive";
        case Stv1BlendMode::Multiply: return "Multiply";
        case Stv1BlendMode::Average:  return "Average";
    }
    return "Unknown";
}

struct Stv1Clip {
    uint64_t    id        = 0;
    std::string name;
    float       startTime = 0.f;
    float       duration  = 1.f;
    float       weight    = 1.f;
    bool        looping   = false;

    [[nodiscard]] bool  isValid()   const { return id != 0 && !name.empty() && duration > 0.f; }
    [[nodiscard]] float endTime()   const { return startTime + duration; }
};

struct Stv1Track {
    uint64_t        id         = 0;
    std::string     name;
    Stv1TrackType   type       = Stv1TrackType::Animation;
    Stv1TrackState  state      = Stv1TrackState::Active;
    Stv1BlendMode   blendMode  = Stv1BlendMode::Override;
    float           weight     = 1.0f;
    std::vector<Stv1Clip> clips;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Stv1TrackState::Active; }
    [[nodiscard]] bool isMuted()  const { return state == Stv1TrackState::Muted; }

    bool addClip(const Stv1Clip& clip) {
        if (!clip.isValid()) return false;
        for (const auto& c : clips) if (c.id == clip.id) return false;
        clips.push_back(clip);
        return true;
    }

    bool removeClip(uint64_t clipId) {
        for (auto it = clips.begin(); it != clips.end(); ++it) {
            if (it->id == clipId) { clips.erase(it); return true; }
        }
        return false;
    }
};

using Stv1ChangeCallback = std::function<void(uint64_t)>;

class SequencerTrackV1 {
public:
    static constexpr size_t MAX_TRACKS = 256;

    bool addTrack(const Stv1Track& track) {
        if (!track.isValid()) return false;
        for (const auto& t : m_tracks) if (t.id == track.id) return false;
        if (m_tracks.size() >= MAX_TRACKS) return false;
        m_tracks.push_back(track);
        return true;
    }

    bool removeTrack(uint64_t id) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->id == id) { m_tracks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Stv1Track* findTrack(uint64_t id) {
        for (auto& t : m_tracks) if (t.id == id) return &t;
        return nullptr;
    }

    bool setState(uint64_t id, Stv1TrackState state) {
        auto* t = findTrack(id);
        if (!t) return false;
        t->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setBlendMode(uint64_t id, Stv1BlendMode mode) {
        auto* t = findTrack(id);
        if (!t) return false;
        t->blendMode = mode;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool addClipToTrack(uint64_t trackId, const Stv1Clip& clip) {
        auto* t = findTrack(trackId);
        if (!t) return false;
        bool ok = t->addClip(clip);
        if (ok && m_onChange) m_onChange(trackId);
        return ok;
    }

    [[nodiscard]] size_t trackCount()  const { return m_tracks.size(); }
    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t mutedCount()  const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.isMuted())  ++c; return c;
    }
    [[nodiscard]] size_t countByType(Stv1TrackType type) const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.type == type) ++c; return c;
    }

    void setOnChange(Stv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Stv1Track> m_tracks;
    Stv1ChangeCallback     m_onChange;
};

} // namespace NF
