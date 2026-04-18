#pragma once
// NF::Editor — Animation track + keyframe editor
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

enum class KeyframeInterpolation : uint8_t { Linear, Step, Bezier, CubicSpline, EaseIn, EaseOut, EaseInOut, Custom };

inline const char* keyframeInterpolationName(KeyframeInterpolation i) {
    switch (i) {
        case KeyframeInterpolation::Linear:      return "Linear";
        case KeyframeInterpolation::Step:        return "Step";
        case KeyframeInterpolation::Bezier:      return "Bezier";
        case KeyframeInterpolation::CubicSpline: return "CubicSpline";
        case KeyframeInterpolation::EaseIn:      return "EaseIn";
        case KeyframeInterpolation::EaseOut:     return "EaseOut";
        case KeyframeInterpolation::EaseInOut:   return "EaseInOut";
        case KeyframeInterpolation::Custom:      return "Custom";
        default:                                 return "Unknown";
    }
}

enum class AnimationTrackType : uint8_t { Position, Rotation, Scale, Opacity, Color, Float, Bool, Event };

inline const char* animationTrackTypeName(AnimationTrackType t) {
    switch (t) {
        case AnimationTrackType::Position: return "Position";
        case AnimationTrackType::Rotation: return "Rotation";
        case AnimationTrackType::Scale:    return "Scale";
        case AnimationTrackType::Opacity:  return "Opacity";
        case AnimationTrackType::Color:    return "Color";
        case AnimationTrackType::Float:    return "Float";
        case AnimationTrackType::Bool:     return "Bool";
        case AnimationTrackType::Event:    return "Event";
        default:                           return "Unknown";
    }
}

struct Keyframe {
    float                  time         = 0.f;
    float                  value        = 0.f;
    KeyframeInterpolation  interpolation = KeyframeInterpolation::Linear;
    bool                   selected     = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setTime(float t)  { time  = t; }
    void setValue(float v) { value = v; }
};

class AnimationTrack {
public:
    explicit AnimationTrack(const std::string& name, AnimationTrackType type = AnimationTrackType::Float)
        : m_name(name), m_type(type) {}

    bool addKeyframe(Keyframe kf) {
        for (auto& existing : m_keyframes) if (existing.time == kf.time) return false;
        m_keyframes.push_back(kf);
        return true;
    }

    bool removeKeyframe(float time) {
        for (auto it = m_keyframes.begin(); it != m_keyframes.end(); ++it) {
            if (it->time == time) { m_keyframes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Keyframe* findKeyframe(float time) {
        for (auto& kf : m_keyframes) if (kf.time == time) return &kf;
        return nullptr;
    }

    [[nodiscard]] size_t keyframeCount() const { return m_keyframes.size(); }

    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0;
        for (auto& kf : m_keyframes) if (kf.selected) c++;
        return c;
    }

    void selectAll()   { for (auto& kf : m_keyframes) kf.select();   }
    void deselectAll() { for (auto& kf : m_keyframes) kf.deselect(); }

    [[nodiscard]] const std::string&    name() const { return m_name; }
    [[nodiscard]] AnimationTrackType    type() const { return m_type; }

    [[nodiscard]] float duration() const {
        float d = 0.f;
        for (auto& kf : m_keyframes) if (kf.time > d) d = kf.time;
        return d;
    }

private:
    std::string            m_name;
    AnimationTrackType     m_type;
    std::vector<Keyframe>  m_keyframes;
};

class KeyframeAnimationEditor {
public:
    static constexpr size_t MAX_TRACKS = 64;

    bool addTrack(AnimationTrack track) {
        if (m_tracks.size() >= MAX_TRACKS) return false;
        for (auto& existing : m_tracks) if (existing.name() == track.name()) return false;
        m_tracks.push_back(std::move(track));
        return true;
    }

    bool removeTrack(const std::string& name) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->name() == name) { m_tracks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] AnimationTrack* findTrack(const std::string& name) {
        for (auto& t : m_tracks) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t trackCount() const { return m_tracks.size(); }

    [[nodiscard]] float totalDuration() const {
        float d = 0.f;
        for (auto& t : m_tracks) if (t.duration() > d) d = t.duration();
        return d;
    }

    void setPlayhead(float time) { m_playhead = time; }
    [[nodiscard]] float playhead() const { return m_playhead; }

    void play()  { m_playing = true;  }
    void pause() { m_playing = false; }
    void stop()  { m_playing = false; m_playhead = 0.f; }

    [[nodiscard]] bool isPlaying() const { return m_playing; }

    void selectAllKeyframes()   { for (auto& t : m_tracks) t.selectAll();   }
    void deselectAllKeyframes() { for (auto& t : m_tracks) t.deselectAll(); }

private:
    std::vector<AnimationTrack> m_tracks;
    float                       m_playhead = 0.f;
    bool                        m_playing  = false;
};

// ============================================================
// S29 — Curve Editor
// ============================================================


} // namespace NF
