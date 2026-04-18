#pragma once
// NF::Editor — Telemetry editor v1: telemetry event tracking and session monitoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Telv1EventSeverity : uint8_t { Trace, Debug, Info, Warning, Critical };
enum class Telv1SessionState  : uint8_t { Unknown, Active, Paused, Ended, Crashed };

inline const char* telv1EventSeverityName(Telv1EventSeverity s) {
    switch (s) {
        case Telv1EventSeverity::Trace:    return "Trace";
        case Telv1EventSeverity::Debug:    return "Debug";
        case Telv1EventSeverity::Info:     return "Info";
        case Telv1EventSeverity::Warning:  return "Warning";
        case Telv1EventSeverity::Critical: return "Critical";
    }
    return "Unknown";
}

inline const char* telv1SessionStateName(Telv1SessionState s) {
    switch (s) {
        case Telv1SessionState::Unknown: return "Unknown";
        case Telv1SessionState::Active:  return "Active";
        case Telv1SessionState::Paused:  return "Paused";
        case Telv1SessionState::Ended:   return "Ended";
        case Telv1SessionState::Crashed: return "Crashed";
    }
    return "Unknown";
}

struct Telv1EventRecord {
    uint64_t           id       = 0;
    std::string        name;
    Telv1EventSeverity severity = Telv1EventSeverity::Info;
    std::string        payload;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isCritical() const { return severity == Telv1EventSeverity::Critical; }
    [[nodiscard]] bool isWarning()  const { return severity == Telv1EventSeverity::Warning; }
};

struct Telv1Session {
    uint64_t          id    = 0;
    std::string       name;
    Telv1SessionState state = Telv1SessionState::Unknown;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()  const { return state == Telv1SessionState::Active; }
    [[nodiscard]] bool hasCrashed() const { return state == Telv1SessionState::Crashed; }
    [[nodiscard]] bool isPaused()  const { return state == Telv1SessionState::Paused; }
};

using Telv1EventCallback = std::function<void(uint64_t)>;

class TelemetryEditorV1 {
public:
    static constexpr size_t MAX_EVENTS   = 4096;
    static constexpr size_t MAX_SESSIONS = 256;

    bool addEvent(const Telv1EventRecord& event) {
        if (!event.isValid()) return false;
        for (const auto& e : m_events) if (e.id == event.id) return false;
        if (m_events.size() >= MAX_EVENTS) return false;
        m_events.push_back(event);
        return true;
    }

    bool removeEvent(uint64_t id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Telv1EventRecord* findEvent(uint64_t id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    bool addSession(const Telv1Session& session) {
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

    [[nodiscard]] Telv1Session* findSession(uint64_t id) {
        for (auto& s : m_sessions) if (s.id == id) return &s;
        return nullptr;
    }

    bool setSessionState(uint64_t id, Telv1SessionState state) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state = state;
        if (m_onEvent) m_onEvent(id);
        return true;
    }

    [[nodiscard]] size_t eventCount()         const { return m_events.size(); }
    [[nodiscard]] size_t sessionCount()       const { return m_sessions.size(); }
    [[nodiscard]] size_t activeSessionCount() const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t countBySeverity(Telv1EventSeverity severity) const {
        size_t c = 0; for (const auto& e : m_events) if (e.severity == severity) ++c; return c;
    }

    void setOnEvent(Telv1EventCallback cb) { m_onEvent = std::move(cb); }

private:
    std::vector<Telv1EventRecord> m_events;
    std::vector<Telv1Session>     m_sessions;
    Telv1EventCallback            m_onEvent;
};

} // namespace NF
