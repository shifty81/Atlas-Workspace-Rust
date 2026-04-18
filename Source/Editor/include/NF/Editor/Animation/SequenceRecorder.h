#pragma once
// NF::Editor — Sequence recorder for in-editor capture
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

enum class RecordTarget : uint8_t {
    Transform, Animation, Physics, Audio, Custom, All
};

inline const char* recordTargetName(RecordTarget t) {
    switch (t) {
        case RecordTarget::Transform:  return "Transform";
        case RecordTarget::Animation:  return "Animation";
        case RecordTarget::Physics:    return "Physics";
        case RecordTarget::Audio:      return "Audio";
        case RecordTarget::Custom:     return "Custom";
        case RecordTarget::All:        return "All";
    }
    return "Unknown";
}

enum class RecordQuality : uint8_t {
    Draft, Preview, High, Master
};

inline const char* recordQualityName(RecordQuality q) {
    switch (q) {
        case RecordQuality::Draft:   return "Draft";
        case RecordQuality::Preview: return "Preview";
        case RecordQuality::High:    return "High";
        case RecordQuality::Master:  return "Master";
    }
    return "Unknown";
}

enum class RecorderState : uint8_t {
    Idle, Counting, Recording, Paused, Stopped, Exporting
};

inline const char* recorderStateName(RecorderState s) {
    switch (s) {
        case RecorderState::Idle:      return "Idle";
        case RecorderState::Counting:  return "Counting";
        case RecorderState::Recording: return "Recording";
        case RecorderState::Paused:    return "Paused";
        case RecorderState::Stopped:   return "Stopped";
        case RecorderState::Exporting: return "Exporting";
    }
    return "Unknown";
}

class RecordTrackConfig {
public:
    explicit RecordTrackConfig(const std::string& actorName, RecordTarget target)
        : m_actorName(actorName), m_target(target) {}

    void setEnabled(bool v)    { m_enabled = v; }
    void setSampleRate(float r){ m_sampleRate = r; }

    [[nodiscard]] const std::string& actorName()  const { return m_actorName;  }
    [[nodiscard]] RecordTarget       target()     const { return m_target;     }
    [[nodiscard]] bool               isEnabled()  const { return m_enabled;    }
    [[nodiscard]] float              sampleRate() const { return m_sampleRate; }

private:
    std::string  m_actorName;
    RecordTarget m_target;
    float        m_sampleRate = 30.0f;
    bool         m_enabled    = true;
};

class SequenceRecorderPanel {
public:
    static constexpr size_t MAX_TRACKS = 64;

    [[nodiscard]] bool addTrack(const RecordTrackConfig& track) {
        if (m_tracks.size() >= MAX_TRACKS) return false;
        for (auto& t : m_tracks)
            if (t.actorName() == track.actorName() && t.target() == track.target()) return false;
        m_tracks.push_back(track);
        return true;
    }

    [[nodiscard]] bool removeTrack(const std::string& actorName) {
        for (auto it = m_tracks.begin(); it != m_tracks.end(); ++it) {
            if (it->actorName() == actorName) { m_tracks.erase(it); return true; }
        }
        return false;
    }

    void startRecord()  { m_state = RecorderState::Recording; }
    void pauseRecord()  { if (m_state == RecorderState::Recording) m_state = RecorderState::Paused; }
    void stopRecord()   { m_state = RecorderState::Stopped; }

    void setQuality(RecordQuality q)       { m_quality  = q; }
    void setOutputName(const std::string& n) { m_outputName = n; }

    [[nodiscard]] RecorderState      state()         const { return m_state;       }
    [[nodiscard]] RecordQuality      quality()       const { return m_quality;     }
    [[nodiscard]] const std::string& outputName()    const { return m_outputName;  }
    [[nodiscard]] size_t             trackCount()    const { return m_tracks.size(); }
    [[nodiscard]] bool               isRecording()   const { return m_state == RecorderState::Recording; }
    [[nodiscard]] bool               isPaused()      const { return m_state == RecorderState::Paused;    }

    [[nodiscard]] size_t enabledTrackCount() const {
        size_t c = 0; for (auto& t : m_tracks) if (t.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByTarget(RecordTarget tgt) const {
        size_t c = 0; for (auto& t : m_tracks) if (t.target() == tgt) ++c; return c;
    }

private:
    std::vector<RecordTrackConfig> m_tracks;
    RecorderState                  m_state      = RecorderState::Idle;
    RecordQuality                  m_quality    = RecordQuality::Preview;
    std::string                    m_outputName;
};

} // namespace NF
