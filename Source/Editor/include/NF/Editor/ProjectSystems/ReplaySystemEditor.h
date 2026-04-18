#pragma once
// NF::Editor — replay system editor
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

enum class ReplayRecordingMode : uint8_t {
    Continuous, OnEvent, Periodic, Manual, Compressed
};

inline const char* replayRecordingModeName(ReplayRecordingMode m) {
    switch (m) {
        case ReplayRecordingMode::Continuous:  return "Continuous";
        case ReplayRecordingMode::OnEvent:     return "OnEvent";
        case ReplayRecordingMode::Periodic:    return "Periodic";
        case ReplayRecordingMode::Manual:      return "Manual";
        case ReplayRecordingMode::Compressed:  return "Compressed";
    }
    return "Unknown";
}

enum class ReplayPlaybackSpeed : uint8_t {
    Eighth, Quarter, Half, Normal, Double, Quadruple
};

inline const char* replayPlaybackSpeedName(ReplayPlaybackSpeed s) {
    switch (s) {
        case ReplayPlaybackSpeed::Eighth:    return "Eighth";
        case ReplayPlaybackSpeed::Quarter:   return "Quarter";
        case ReplayPlaybackSpeed::Half:      return "Half";
        case ReplayPlaybackSpeed::Normal:    return "Normal";
        case ReplayPlaybackSpeed::Double:    return "Double";
        case ReplayPlaybackSpeed::Quadruple: return "Quadruple";
    }
    return "Unknown";
}

enum class ReplayDataChannel : uint8_t {
    Transform, Physics, Animation, Audio, Input, Network, Custom, All
};

inline const char* replayDataChannelName(ReplayDataChannel c) {
    switch (c) {
        case ReplayDataChannel::Transform:  return "Transform";
        case ReplayDataChannel::Physics:    return "Physics";
        case ReplayDataChannel::Animation:  return "Animation";
        case ReplayDataChannel::Audio:      return "Audio";
        case ReplayDataChannel::Input:      return "Input";
        case ReplayDataChannel::Network:    return "Network";
        case ReplayDataChannel::Custom:     return "Custom";
        case ReplayDataChannel::All:        return "All";
    }
    return "Unknown";
}

class ReplayClip {
public:
    explicit ReplayClip(uint32_t id, const std::string& name, ReplayRecordingMode recordingMode)
        : m_id(id), m_name(name), m_recordingMode(recordingMode) {}

    void setPlaybackSpeed(ReplayPlaybackSpeed v) { m_playbackSpeed    = v; }
    void setDurationSeconds(float v)             { m_durationSeconds  = v; }
    void setFileSizeKB(uint32_t v)               { m_fileSizeKB       = v; }
    void setLooping(bool v)                      { m_isLooping        = v; }
    void setAudioEnabled(bool v)                 { m_hasAudio         = v; }

    [[nodiscard]] uint32_t             id()              const { return m_id;             }
    [[nodiscard]] const std::string&   name()            const { return m_name;           }
    [[nodiscard]] ReplayRecordingMode  recordingMode()   const { return m_recordingMode;  }
    [[nodiscard]] ReplayPlaybackSpeed  playbackSpeed()   const { return m_playbackSpeed;  }
    [[nodiscard]] float                durationSeconds() const { return m_durationSeconds;}
    [[nodiscard]] uint32_t             fileSizeKB()      const { return m_fileSizeKB;     }
    [[nodiscard]] bool                 isLooping()       const { return m_isLooping;      }
    [[nodiscard]] bool                 hasAudio()        const { return m_hasAudio;       }

private:
    uint32_t            m_id;
    std::string         m_name;
    ReplayRecordingMode m_recordingMode;
    ReplayPlaybackSpeed m_playbackSpeed   = ReplayPlaybackSpeed::Normal;
    float               m_durationSeconds = 0.0f;
    uint32_t            m_fileSizeKB      = 0u;
    bool                m_isLooping       = false;
    bool                m_hasAudio        = true;
};

class ReplaySystemEditor {
public:
    void setActiveChannel(ReplayDataChannel v) { m_activeChannel = v; }
    void setShowTimeline(bool v)               { m_showTimeline  = v; }
    void setShowMarkers(bool v)                { m_showMarkers   = v; }
    void setPlaybackRate(float v)              { m_playbackRate  = v; }

    bool addClip(const ReplayClip& c) {
        for (auto& e : m_clips) if (e.id() == c.id()) return false;
        m_clips.push_back(c); return true;
    }
    bool removeClip(uint32_t id) {
        auto it = std::find_if(m_clips.begin(), m_clips.end(),
            [&](const ReplayClip& e){ return e.id() == id; });
        if (it == m_clips.end()) return false;
        m_clips.erase(it); return true;
    }
    [[nodiscard]] ReplayClip* findClip(uint32_t id) {
        for (auto& e : m_clips) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] ReplayDataChannel activeChannel()  const { return m_activeChannel; }
    [[nodiscard]] bool              isShowTimeline() const { return m_showTimeline;  }
    [[nodiscard]] bool              isShowMarkers()  const { return m_showMarkers;   }
    [[nodiscard]] float             playbackRate()   const { return m_playbackRate;  }
    [[nodiscard]] size_t            clipCount()      const { return m_clips.size();  }

    [[nodiscard]] size_t countByMode(ReplayRecordingMode m) const {
        size_t c = 0; for (auto& e : m_clips) if (e.recordingMode() == m) ++c; return c;
    }
    [[nodiscard]] size_t countBySpeed(ReplayPlaybackSpeed s) const {
        size_t c = 0; for (auto& e : m_clips) if (e.playbackSpeed() == s) ++c; return c;
    }
    [[nodiscard]] size_t countLooping() const {
        size_t c = 0; for (auto& e : m_clips) if (e.isLooping()) ++c; return c;
    }

private:
    std::vector<ReplayClip> m_clips;
    ReplayDataChannel m_activeChannel = ReplayDataChannel::All;
    bool              m_showTimeline  = true;
    bool              m_showMarkers   = true;
    float             m_playbackRate  = 1.0f;
};

} // namespace NF
