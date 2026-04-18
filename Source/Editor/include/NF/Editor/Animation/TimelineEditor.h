#pragma once
// NF::Editor — Timeline track + editor panel
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

enum class TimelineEventType : uint8_t {
    Keyframe, Marker, Clip, Trigger, Label, Camera, Audio, Custom
};

inline const char* timelineEventTypeName(TimelineEventType t) {
    switch (t) {
        case TimelineEventType::Keyframe: return "Keyframe";
        case TimelineEventType::Marker:   return "Marker";
        case TimelineEventType::Clip:     return "Clip";
        case TimelineEventType::Trigger:  return "Trigger";
        case TimelineEventType::Label:    return "Label";
        case TimelineEventType::Camera:   return "Camera";
        case TimelineEventType::Audio:    return "Audio";
        case TimelineEventType::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class TimelineTrackKind : uint8_t {
    Animation, Audio, Event, Camera
};

inline const char* timelineTrackKindName(TimelineTrackKind k) {
    switch (k) {
        case TimelineTrackKind::Animation: return "Animation";
        case TimelineTrackKind::Audio:     return "Audio";
        case TimelineTrackKind::Event:     return "Event";
        case TimelineTrackKind::Camera:    return "Camera";
    }
    return "Unknown";
}

struct TimelineEvent {
    std::string       id;
    TimelineEventType type     = TimelineEventType::Keyframe;
    float             time     = 0.0f;
    float             duration = 0.0f;
    bool              selected = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setTime(float t)     { time = t; }
    void setDuration(float d) { duration = d; }
};

class TimelineTrack {
public:
    static constexpr size_t MAX_EVENTS = 128;

    explicit TimelineTrack(const std::string& name, TimelineTrackKind kind = TimelineTrackKind::Animation)
        : m_name(name), m_kind(kind) {}

    [[nodiscard]] bool addEvent(const TimelineEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        if (m_events.size() >= MAX_EVENTS) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TimelineEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void selectAll()   { for (auto& e : m_events) e.select();   }
    void deselectAll() { for (auto& e : m_events) e.deselect(); }

    [[nodiscard]] size_t eventCount()    const { return m_events.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.selected) ++c; return c;
    }
    [[nodiscard]] float duration() const {
        float d = 0.0f;
        for (auto& e : m_events) { float end = e.time + e.duration; if (end > d) d = end; }
        return d;
    }
    [[nodiscard]] bool                muted()  const { return m_muted; }
    void                              setMuted(bool v) { m_muted = v; }
    [[nodiscard]] const std::string&  name()   const { return m_name; }
    [[nodiscard]] TimelineTrackKind   kind()   const { return m_kind; }

private:
    std::string              m_name;
    TimelineTrackKind        m_kind;
    std::vector<TimelineEvent> m_events;
    bool                     m_muted = false;
};

class TimelineEditorPanel {
public:
    static constexpr size_t MAX_TRACKS = 64;

    [[nodiscard]] bool addTrack(const TimelineTrack& track) {
        for (auto& t : m_tracks) if (t.name() == track.name()) return false;
        if (m_tracks.size() >= MAX_TRACKS) return false;
        m_tracks.push_back(track);
        return true;
    }

    [[nodiscard]] bool removeTrack(const std::string& name) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->name() == name) {
                if (m_activeTrack == name) m_activeTrack.clear();
                m_tracks.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TimelineTrack* findTrack(const std::string& name) {
        for (auto& t : m_tracks) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveTrack(const std::string& name) {
        for (auto& t : m_tracks) {
            if (t.name() == name) { m_activeTrack = name; return true; }
        }
        return false;
    }

    void setPlayhead(float t)  { m_playhead = t; }
    void play()  { m_playing = true;  }
    void pause() { m_playing = false; }
    void stop()  { m_playing = false; m_playhead = 0.0f; }

    [[nodiscard]] float              playhead()     const { return m_playhead; }
    [[nodiscard]] bool               isPlaying()    const { return m_playing; }
    [[nodiscard]] const std::string& activeTrack()  const { return m_activeTrack; }
    [[nodiscard]] size_t             trackCount()   const { return m_tracks.size(); }

    void selectAllEvents()   { for (auto& t : m_tracks) t.selectAll();   }
    void deselectAllEvents() { for (auto& t : m_tracks) t.deselectAll(); }

private:
    std::vector<TimelineTrack> m_tracks;
    std::string                m_activeTrack;
    float                      m_playhead = 0.0f;
    bool                       m_playing  = false;
};

// ── S32 — Particle Effect Editor ─────────────────────────────────


} // namespace NF
