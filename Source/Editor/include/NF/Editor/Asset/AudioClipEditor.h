#pragma once
// NF::Editor — Audio clip asset + editor
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

enum class AudioClipFormat : uint8_t {
    WAV, OGG, MP3, FLAC, AIFF
};
inline const char* audioClipFormatName(AudioClipFormat f) {
    switch (f) {
        case AudioClipFormat::WAV:  return "WAV";
        case AudioClipFormat::OGG:  return "OGG";
        case AudioClipFormat::MP3:  return "MP3";
        case AudioClipFormat::FLAC: return "FLAC";
        case AudioClipFormat::AIFF: return "AIFF";
    }
    return "Unknown";
}

enum class AudioClipState : uint8_t {
    Idle, Playing, Paused, Stopped, Finished
};
inline const char* audioClipStateName(AudioClipState s) {
    switch (s) {
        case AudioClipState::Idle:     return "Idle";
        case AudioClipState::Playing:  return "Playing";
        case AudioClipState::Paused:   return "Paused";
        case AudioClipState::Stopped:  return "Stopped";
        case AudioClipState::Finished: return "Finished";
    }
    return "Unknown";
}

enum class AudioLoopMode : uint8_t {
    None, Loop, PingPong, LoopPoint, Shuffle
};
inline const char* audioLoopModeName(AudioLoopMode m) {
    switch (m) {
        case AudioLoopMode::None:      return "None";
        case AudioLoopMode::Loop:      return "Loop";
        case AudioLoopMode::PingPong:  return "PingPong";
        case AudioLoopMode::LoopPoint: return "LoopPoint";
        case AudioLoopMode::Shuffle:   return "Shuffle";
    }
    return "Unknown";
}

class AudioClipAsset {
public:
    explicit AudioClipAsset(const std::string& name,
                             float durationSec = 1.0f,
                             uint32_t sampleRate = 44100)
        : m_name(name), m_durationSec(durationSec), m_sampleRate(sampleRate) {}

    void setFormat(AudioClipFormat f)   { m_format   = f; }
    void setState(AudioClipState s)     { m_state    = s; }
    void setLoopMode(AudioLoopMode m)   { m_loopMode = m; }
    void setVolume(float v)             { m_volume   = v; }
    void setPitch(float p)              { m_pitch    = p; }
    void setStreaming(bool v)           { m_streaming = v; }
    void setDirty(bool v)               { m_dirty     = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] AudioClipFormat    format()     const { return m_format;     }
    [[nodiscard]] AudioClipState     state()      const { return m_state;      }
    [[nodiscard]] AudioLoopMode      loopMode()   const { return m_loopMode;   }
    [[nodiscard]] float              durationSec()const { return m_durationSec;}
    [[nodiscard]] uint32_t           sampleRate() const { return m_sampleRate; }
    [[nodiscard]] float              volume()     const { return m_volume;     }
    [[nodiscard]] float              pitch()      const { return m_pitch;      }
    [[nodiscard]] bool               isStreaming()const { return m_streaming;  }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isPlaying()  const { return m_state == AudioClipState::Playing;  }
    [[nodiscard]] bool isPaused()   const { return m_state == AudioClipState::Paused;   }
    [[nodiscard]] bool isFinished() const { return m_state == AudioClipState::Finished; }
    [[nodiscard]] bool isLooping()  const { return m_loopMode != AudioLoopMode::None;   }

private:
    std::string      m_name;
    AudioClipFormat  m_format     = AudioClipFormat::WAV;
    AudioClipState   m_state      = AudioClipState::Idle;
    AudioLoopMode    m_loopMode   = AudioLoopMode::None;
    float            m_durationSec;
    uint32_t         m_sampleRate;
    float            m_volume     = 1.0f;
    float            m_pitch      = 1.0f;
    bool             m_streaming  = false;
    bool             m_dirty      = false;
};

class AudioClipEditor {
public:
    static constexpr size_t MAX_CLIPS = 512;

    [[nodiscard]] bool addClip(const AudioClipAsset& clip) {
        if (m_clips.size() >= MAX_CLIPS) return false;
        for (auto& c : m_clips) if (c.name() == clip.name()) return false;
        m_clips.push_back(clip);
        return true;
    }

    [[nodiscard]] bool removeClip(const std::string& name) {
        for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
            if (it->name() == name) {
                if (m_activeClip == name) m_activeClip.clear();
                m_clips.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AudioClipAsset* findClip(const std::string& name) {
        for (auto& c : m_clips) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] bool setActiveClip(const std::string& name) {
        for (auto& c : m_clips)
            if (c.name() == name) { m_activeClip = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeClip() const { return m_activeClip; }
    [[nodiscard]] size_t clipCount()     const { return m_clips.size(); }
    [[nodiscard]] size_t dirtyCount()    const {
        size_t n = 0; for (auto& c : m_clips) if (c.isDirty())     ++n; return n;
    }
    [[nodiscard]] size_t playingCount()  const {
        size_t n = 0; for (auto& c : m_clips) if (c.isPlaying())   ++n; return n;
    }
    [[nodiscard]] size_t streamingCount()const {
        size_t n = 0; for (auto& c : m_clips) if (c.isStreaming())  ++n; return n;
    }
    [[nodiscard]] size_t loopingCount()  const {
        size_t n = 0; for (auto& c : m_clips) if (c.isLooping())   ++n; return n;
    }
    [[nodiscard]] size_t countByFormat(AudioClipFormat f) const {
        size_t n = 0; for (auto& c : m_clips) if (c.format() == f) ++n; return n;
    }
    [[nodiscard]] size_t countByState(AudioClipState s) const {
        size_t n = 0; for (auto& c : m_clips) if (c.state()  == s) ++n; return n;
    }

private:
    std::vector<AudioClipAsset> m_clips;
    std::string                 m_activeClip;
};

// ── S41 — Video Clip Editor ──────────────────────────────────────


} // namespace NF
