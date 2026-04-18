#pragma once
// NF::Editor — Sound cue editor v1: sound cue and trigger management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Scev1CueType  : uint8_t { OneShot, Looping, Ambience, Music, SFX, Voice };
enum class Scev1CueState : uint8_t { Idle, Playing, Paused, Stopped, Error };

inline const char* scev1CueTypeName(Scev1CueType t) {
    switch (t) {
        case Scev1CueType::OneShot:  return "OneShot";
        case Scev1CueType::Looping:  return "Looping";
        case Scev1CueType::Ambience: return "Ambience";
        case Scev1CueType::Music:    return "Music";
        case Scev1CueType::SFX:      return "SFX";
        case Scev1CueType::Voice:    return "Voice";
    }
    return "Unknown";
}

inline const char* scev1CueStateName(Scev1CueState s) {
    switch (s) {
        case Scev1CueState::Idle:    return "Idle";
        case Scev1CueState::Playing: return "Playing";
        case Scev1CueState::Paused:  return "Paused";
        case Scev1CueState::Stopped: return "Stopped";
        case Scev1CueState::Error:   return "Error";
    }
    return "Unknown";
}

struct Scev1SoundCue {
    uint64_t       id        = 0;
    std::string    name;
    std::string    assetPath;
    Scev1CueType   cueType   = Scev1CueType::OneShot;
    Scev1CueState  state     = Scev1CueState::Idle;
    float          volume    = 1.0f;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isPlaying() const { return state == Scev1CueState::Playing; }
    [[nodiscard]] bool isPaused()  const { return state == Scev1CueState::Paused; }
    [[nodiscard]] bool isError()   const { return state == Scev1CueState::Error; }
};

struct Scev1SoundTrigger {
    uint64_t    id     = 0;
    uint64_t    cueId  = 0;
    std::string event;

    [[nodiscard]] bool isValid() const { return id != 0 && cueId != 0 && !event.empty(); }
};

using Scev1ChangeCallback = std::function<void(uint64_t)>;

class SoundCueEditorV1 {
public:
    static constexpr size_t MAX_CUES     = 512;
    static constexpr size_t MAX_TRIGGERS = 1024;

    bool addCue(const Scev1SoundCue& cue) {
        if (!cue.isValid()) return false;
        for (const auto& c : m_cues) if (c.id == cue.id) return false;
        if (m_cues.size() >= MAX_CUES) return false;
        m_cues.push_back(cue);
        if (m_onChange) m_onChange(cue.id);
        return true;
    }

    bool removeCue(uint64_t id) {
        for (auto it = m_cues.begin(); it != m_cues.end(); ++it) {
            if (it->id == id) { m_cues.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Scev1SoundCue* findCue(uint64_t id) {
        for (auto& c : m_cues) if (c.id == id) return &c;
        return nullptr;
    }

    bool addTrigger(const Scev1SoundTrigger& trigger) {
        if (!trigger.isValid()) return false;
        for (const auto& t : m_triggers) if (t.id == trigger.id) return false;
        if (m_triggers.size() >= MAX_TRIGGERS) return false;
        m_triggers.push_back(trigger);
        if (m_onChange) m_onChange(trigger.id);
        return true;
    }

    bool removeTrigger(uint64_t id) {
        for (auto it = m_triggers.begin(); it != m_triggers.end(); ++it) {
            if (it->id == id) { m_triggers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t cueCount()     const { return m_cues.size(); }
    [[nodiscard]] size_t triggerCount() const { return m_triggers.size(); }

    [[nodiscard]] size_t playingCount() const {
        size_t c = 0; for (const auto& cu : m_cues) if (cu.isPlaying()) ++c; return c;
    }
    [[nodiscard]] size_t errorCount() const {
        size_t c = 0; for (const auto& cu : m_cues) if (cu.isError()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Scev1CueType type) const {
        size_t c = 0; for (const auto& cu : m_cues) if (cu.cueType == type) ++c; return c;
    }
    [[nodiscard]] size_t triggersForCue(uint64_t cueId) const {
        size_t c = 0; for (const auto& t : m_triggers) if (t.cueId == cueId) ++c; return c;
    }

    void setOnChange(Scev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Scev1SoundCue>     m_cues;
    std::vector<Scev1SoundTrigger> m_triggers;
    Scev1ChangeCallback            m_onChange;
};

} // namespace NF
