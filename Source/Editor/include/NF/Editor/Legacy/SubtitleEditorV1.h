#pragma once
// NF::Editor — Subtitle editor v1: subtitle track and cue management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Subv1TrackState : uint8_t { Empty, Draft, Review, Final, Exported };
enum class Subv1CueStyle   : uint8_t { Normal, Emphasis, Whisper, Shout, Caption };

inline const char* subv1TrackStateName(Subv1TrackState s) {
    switch (s) {
        case Subv1TrackState::Empty:    return "Empty";
        case Subv1TrackState::Draft:    return "Draft";
        case Subv1TrackState::Review:   return "Review";
        case Subv1TrackState::Final:    return "Final";
        case Subv1TrackState::Exported: return "Exported";
    }
    return "Unknown";
}

inline const char* subv1CueStyleName(Subv1CueStyle s) {
    switch (s) {
        case Subv1CueStyle::Normal:   return "Normal";
        case Subv1CueStyle::Emphasis: return "Emphasis";
        case Subv1CueStyle::Whisper:  return "Whisper";
        case Subv1CueStyle::Shout:    return "Shout";
        case Subv1CueStyle::Caption:  return "Caption";
    }
    return "Unknown";
}

struct Subv1Cue {
    uint64_t      id      = 0;
    uint64_t      trackId = 0;
    std::string   text;
    float         startMs = 0.f;
    float         endMs   = 0.f;
    Subv1CueStyle style   = Subv1CueStyle::Normal;

    [[nodiscard]] bool isValid()   const { return id != 0 && trackId != 0 && !text.empty() && endMs > startMs; }
    [[nodiscard]] bool isCaption() const { return style == Subv1CueStyle::Caption; }
};

struct Subv1Track {
    uint64_t              id     = 0;
    std::string           name;
    std::string           locale;
    Subv1TrackState       state  = Subv1TrackState::Empty;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isFinal()    const { return state == Subv1TrackState::Final; }
    [[nodiscard]] bool isExported() const { return state == Subv1TrackState::Exported; }
};

using Subv1ChangeCallback = std::function<void(uint64_t)>;

class SubtitleEditorV1 {
public:
    static constexpr size_t MAX_TRACKS = 128;
    static constexpr size_t MAX_CUES   = 65536;

    bool addTrack(const Subv1Track& track) {
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

    [[nodiscard]] Subv1Track* findTrack(uint64_t id) {
        for (auto& t : m_tracks) if (t.id == id) return &t;
        return nullptr;
    }

    bool addCue(const Subv1Cue& cue) {
        if (!cue.isValid()) return false;
        for (const auto& c : m_cues) if (c.id == cue.id) return false;
        if (m_cues.size() >= MAX_CUES) return false;
        m_cues.push_back(cue);
        if (m_onChange) m_onChange(cue.trackId);
        return true;
    }

    bool removeCue(uint64_t id) {
        for (auto it = m_cues.begin(); it != m_cues.end(); ++it) {
            if (it->id == id) { m_cues.erase(it); return true; }
        }
        return false;
    }

    bool setState(uint64_t trackId, Subv1TrackState state) {
        auto* t = findTrack(trackId);
        if (!t) return false;
        t->state = state;
        if (m_onChange) m_onChange(trackId);
        return true;
    }

    [[nodiscard]] size_t trackCount() const { return m_tracks.size(); }
    [[nodiscard]] size_t cueCount()   const { return m_cues.size(); }

    [[nodiscard]] size_t finalCount() const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.isFinal()) ++c; return c;
    }
    [[nodiscard]] size_t exportedCount() const {
        size_t c = 0; for (const auto& t : m_tracks) if (t.isExported()) ++c; return c;
    }
    [[nodiscard]] size_t countByStyle(Subv1CueStyle style) const {
        size_t c = 0; for (const auto& cu : m_cues) if (cu.style == style) ++c; return c;
    }

    void setOnChange(Subv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Subv1Track> m_tracks;
    std::vector<Subv1Cue>   m_cues;
    Subv1ChangeCallback     m_onChange;
};

} // namespace NF
