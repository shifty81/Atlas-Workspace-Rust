#pragma once
// NF::Editor — Timeline sequencer for in-game events
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class SeqTrackKind : uint8_t {
    Event, Animation, Audio, Property, Camera, Script, Subtitle
};

inline const char* seqTrackKindName(SeqTrackKind k) {
    switch (k) {
        case SeqTrackKind::Event:     return "Event";
        case SeqTrackKind::Animation: return "Animation";
        case SeqTrackKind::Audio:     return "Audio";
        case SeqTrackKind::Property:  return "Property";
        case SeqTrackKind::Camera:    return "Camera";
        case SeqTrackKind::Script:    return "Script";
        case SeqTrackKind::Subtitle:  return "Subtitle";
    }
    return "Unknown";
}

enum class SeqPlaybackState : uint8_t {
    Idle, Playing, Paused, Stopped, Scrubbing
};

inline const char* seqPlaybackStateName(SeqPlaybackState s) {
    switch (s) {
        case SeqPlaybackState::Idle:     return "Idle";
        case SeqPlaybackState::Playing:  return "Playing";
        case SeqPlaybackState::Paused:   return "Paused";
        case SeqPlaybackState::Stopped:  return "Stopped";
        case SeqPlaybackState::Scrubbing:return "Scrubbing";
    }
    return "Unknown";
}

enum class SeqLoopMode : uint8_t {
    None, Loop, PingPong, Hold
};

inline const char* seqLoopModeName(SeqLoopMode m) {
    switch (m) {
        case SeqLoopMode::None:     return "None";
        case SeqLoopMode::Loop:     return "Loop";
        case SeqLoopMode::PingPong: return "PingPong";
        case SeqLoopMode::Hold:     return "Hold";
    }
    return "Unknown";
}

class SeqTrack {
public:
    explicit SeqTrack(const std::string& name, SeqTrackKind kind)
        : m_name(name), m_kind(kind) {}

    void setEnabled(bool v)   { m_enabled = v; }
    void setMuted(bool v)     { m_muted   = v; }
    void setLocked(bool v)    { m_locked  = v; }
    void setKeyCount(uint32_t n) { m_keyCount = n; }

    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] SeqTrackKind       kind()     const { return m_kind;     }
    [[nodiscard]] bool               isEnabled()const { return m_enabled;  }
    [[nodiscard]] bool               isMuted()  const { return m_muted;    }
    [[nodiscard]] bool               isLocked() const { return m_locked;   }
    [[nodiscard]] uint32_t           keyCount() const { return m_keyCount; }

private:
    std::string  m_name;
    SeqTrackKind m_kind;
    uint32_t     m_keyCount = 0;
    bool         m_enabled  = true;
    bool         m_muted    = false;
    bool         m_locked   = false;
};

class TimelineSequencer {
public:
    static constexpr size_t MAX_TRACKS = 128;

    [[nodiscard]] bool addTrack(const SeqTrack& track) {
        for (auto& t : m_tracks) if (t.name() == track.name()) return false;
        if (m_tracks.size() >= MAX_TRACKS) return false;
        m_tracks.push_back(track);
        return true;
    }

    [[nodiscard]] bool removeTrack(const std::string& name) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->name() == name) { m_tracks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SeqTrack* findTrack(const std::string& name) {
        for (auto& t : m_tracks) if (t.name() == name) return &t;
        return nullptr;
    }

    void setState(SeqPlaybackState s)  { m_state     = s; }
    void setLoopMode(SeqLoopMode m)    { m_loopMode  = m; }
    void setCurrentTime(float t)       { m_currentTime = t; }
    void setDuration(float d)          { m_duration  = d; }

    [[nodiscard]] SeqPlaybackState state()       const { return m_state;       }
    [[nodiscard]] SeqLoopMode      loopMode()    const { return m_loopMode;    }
    [[nodiscard]] float            currentTime() const { return m_currentTime; }
    [[nodiscard]] float            duration()    const { return m_duration;    }
    [[nodiscard]] size_t           trackCount()  const { return m_tracks.size(); }

    [[nodiscard]] bool isPlaying() const { return m_state == SeqPlaybackState::Playing; }
    [[nodiscard]] size_t mutedTrackCount() const {
        size_t c = 0; for (auto& t : m_tracks) if (t.isMuted()) ++c; return c;
    }
    [[nodiscard]] size_t countByKind(SeqTrackKind k) const {
        size_t c = 0; for (auto& t : m_tracks) if (t.kind() == k) ++c; return c;
    }

private:
    std::vector<SeqTrack> m_tracks;
    SeqPlaybackState      m_state      = SeqPlaybackState::Idle;
    SeqLoopMode           m_loopMode   = SeqLoopMode::None;
    float                 m_currentTime = 0.0f;
    float                 m_duration   = 0.0f;
};

} // namespace NF
