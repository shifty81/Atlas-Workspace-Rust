#pragma once
// NF::Editor — Replay system editor v1: replay session recording and playback management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Rsyv1SessionState  : uint8_t { Idle, Recording, Playing, Paused, Stopped, Error };
enum class Rsyv1ReplayQuality : uint8_t { Low, Medium, High, Ultra, Lossless };

inline const char* rsyv1SessionStateName(Rsyv1SessionState s) {
    switch (s) {
        case Rsyv1SessionState::Idle:      return "Idle";
        case Rsyv1SessionState::Recording: return "Recording";
        case Rsyv1SessionState::Playing:   return "Playing";
        case Rsyv1SessionState::Paused:    return "Paused";
        case Rsyv1SessionState::Stopped:   return "Stopped";
        case Rsyv1SessionState::Error:     return "Error";
    }
    return "Unknown";
}

inline const char* rsyv1ReplayQualityName(Rsyv1ReplayQuality q) {
    switch (q) {
        case Rsyv1ReplayQuality::Low:      return "Low";
        case Rsyv1ReplayQuality::Medium:   return "Medium";
        case Rsyv1ReplayQuality::High:     return "High";
        case Rsyv1ReplayQuality::Ultra:    return "Ultra";
        case Rsyv1ReplayQuality::Lossless: return "Lossless";
    }
    return "Unknown";
}

struct Rsyv1Frame {
    uint64_t    id        = 0;
    uint64_t    sessionId = 0;
    float       timestampMs = 0.f;

    [[nodiscard]] bool isValid() const { return id != 0 && sessionId != 0; }
};

struct Rsyv1Session {
    uint64_t            id      = 0;
    std::string         name;
    Rsyv1SessionState   state   = Rsyv1SessionState::Idle;
    Rsyv1ReplayQuality  quality = Rsyv1ReplayQuality::Medium;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRecording() const { return state == Rsyv1SessionState::Recording; }
    [[nodiscard]] bool isPlaying()   const { return state == Rsyv1SessionState::Playing; }
    [[nodiscard]] bool hasError()    const { return state == Rsyv1SessionState::Error; }
};

using Rsyv1StateChangeCallback = std::function<void(uint64_t)>;

class ReplaySystemEditorV1 {
public:
    static constexpr size_t MAX_SESSIONS = 256;

    bool addSession(const Rsyv1Session& session) {
        if (!session.isValid()) return false;
        for (const auto& s : m_sessions) if (s.id == session.id) return false;
        if (m_sessions.size() >= MAX_SESSIONS) return false;
        m_sessions.push_back(session);
        return true;
    }

    bool removeSession(uint64_t id) {
        for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
            if (it->id == id) { m_sessions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Rsyv1Session* findSession(uint64_t id) {
        for (auto& s : m_sessions) if (s.id == id) return &s;
        return nullptr;
    }

    bool setState(uint64_t id, Rsyv1SessionState state) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state = state;
        if (m_onStateChange) m_onStateChange(id);
        return true;
    }

    [[nodiscard]] size_t sessionCount() const { return m_sessions.size(); }

    [[nodiscard]] size_t recordingCount() const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isRecording()) ++c; return c;
    }
    [[nodiscard]] size_t playingCount() const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isPlaying()) ++c; return c;
    }
    [[nodiscard]] size_t countByQuality(Rsyv1ReplayQuality quality) const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.quality == quality) ++c; return c;
    }

    void setOnStateChange(Rsyv1StateChangeCallback cb) { m_onStateChange = std::move(cb); }

private:
    std::vector<Rsyv1Session>   m_sessions;
    Rsyv1StateChangeCallback    m_onStateChange;
};

} // namespace NF
