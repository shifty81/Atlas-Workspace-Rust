#pragma once
// NF::Editor — Playtest recorder v1: playtest session capture and annotation
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ptrv1SessionState    : uint8_t { NotStarted, Running, Paused, Ended, Analyzed };
enum class Ptrv1AnnotationType  : uint8_t { Bug, Feedback, Crash, Performance, UX };

inline const char* ptrv1SessionStateName(Ptrv1SessionState s) {
    switch (s) {
        case Ptrv1SessionState::NotStarted: return "NotStarted";
        case Ptrv1SessionState::Running:    return "Running";
        case Ptrv1SessionState::Paused:     return "Paused";
        case Ptrv1SessionState::Ended:      return "Ended";
        case Ptrv1SessionState::Analyzed:   return "Analyzed";
    }
    return "Unknown";
}

inline const char* ptrv1AnnotationTypeName(Ptrv1AnnotationType t) {
    switch (t) {
        case Ptrv1AnnotationType::Bug:         return "Bug";
        case Ptrv1AnnotationType::Feedback:    return "Feedback";
        case Ptrv1AnnotationType::Crash:       return "Crash";
        case Ptrv1AnnotationType::Performance: return "Performance";
        case Ptrv1AnnotationType::UX:          return "UX";
    }
    return "Unknown";
}

struct Ptrv1Annotation {
    uint64_t              id        = 0;
    uint64_t              sessionId = 0;
    Ptrv1AnnotationType   type      = Ptrv1AnnotationType::Feedback;
    std::string           note;
    float                 timestampMs = 0.f;

    [[nodiscard]] bool isValid()  const { return id != 0 && sessionId != 0; }
    [[nodiscard]] bool isBug()    const { return type == Ptrv1AnnotationType::Bug; }
    [[nodiscard]] bool isCrash()  const { return type == Ptrv1AnnotationType::Crash; }
};

struct Ptrv1Session {
    uint64_t            id    = 0;
    std::string         name;
    Ptrv1SessionState   state = Ptrv1SessionState::NotStarted;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRunning()  const { return state == Ptrv1SessionState::Running; }
    [[nodiscard]] bool isEnded()    const { return state == Ptrv1SessionState::Ended; }
    [[nodiscard]] bool isAnalyzed() const { return state == Ptrv1SessionState::Analyzed; }
};

using Ptrv1ChangeCallback = std::function<void(uint64_t)>;

class PlaytestRecorderV1 {
public:
    static constexpr size_t MAX_SESSIONS     = 512;
    static constexpr size_t MAX_ANNOTATIONS  = 65536;

    bool addSession(const Ptrv1Session& session) {
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

    [[nodiscard]] Ptrv1Session* findSession(uint64_t id) {
        for (auto& s : m_sessions) if (s.id == id) return &s;
        return nullptr;
    }

    bool addAnnotation(const Ptrv1Annotation& annotation) {
        if (!annotation.isValid()) return false;
        for (const auto& a : m_annotations) if (a.id == annotation.id) return false;
        if (m_annotations.size() >= MAX_ANNOTATIONS) return false;
        m_annotations.push_back(annotation);
        if (m_onChange) m_onChange(annotation.sessionId);
        return true;
    }

    bool removeAnnotation(uint64_t id) {
        for (auto it = m_annotations.begin(); it != m_annotations.end(); ++it) {
            if (it->id == id) { m_annotations.erase(it); return true; }
        }
        return false;
    }

    bool setState(uint64_t id, Ptrv1SessionState state) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t sessionCount()    const { return m_sessions.size(); }
    [[nodiscard]] size_t annotationCount() const { return m_annotations.size(); }

    [[nodiscard]] size_t endedCount() const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isEnded()) ++c; return c;
    }
    [[nodiscard]] size_t analyzedCount() const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isAnalyzed()) ++c; return c;
    }
    [[nodiscard]] size_t countByAnnotationType(Ptrv1AnnotationType type) const {
        size_t c = 0; for (const auto& a : m_annotations) if (a.type == type) ++c; return c;
    }

    void setOnChange(Ptrv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ptrv1Session>    m_sessions;
    std::vector<Ptrv1Annotation> m_annotations;
    Ptrv1ChangeCallback          m_onChange;
};

} // namespace NF
