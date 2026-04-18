#pragma once
// NF::Editor — Cloud sync editor v1: sync sessions, conflict resolution, push/pull operations
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Csynv1SyncState          : uint8_t { Idle, Pushing, Pulling, Conflict, Complete, Error };
enum class Csynv1ConflictResolution : uint8_t { None, Local, Remote, Merge, Skip };

inline const char* csynv1SyncStateName(Csynv1SyncState s) {
    switch (s) {
        case Csynv1SyncState::Idle:     return "Idle";
        case Csynv1SyncState::Pushing:  return "Pushing";
        case Csynv1SyncState::Pulling:  return "Pulling";
        case Csynv1SyncState::Conflict: return "Conflict";
        case Csynv1SyncState::Complete: return "Complete";
        case Csynv1SyncState::Error:    return "Error";
    }
    return "Unknown";
}

inline const char* csynv1ConflictResolutionName(Csynv1ConflictResolution r) {
    switch (r) {
        case Csynv1ConflictResolution::None:   return "None";
        case Csynv1ConflictResolution::Local:  return "Local";
        case Csynv1ConflictResolution::Remote: return "Remote";
        case Csynv1ConflictResolution::Merge:  return "Merge";
        case Csynv1ConflictResolution::Skip:   return "Skip";
    }
    return "Unknown";
}

struct Csynv1ConflictRecord {
    std::string              filePath;
    Csynv1ConflictResolution resolution = Csynv1ConflictResolution::None;
    bool isValid()    const { return !filePath.empty(); }
    bool isResolved() const { return resolution != Csynv1ConflictResolution::None; }
};

struct Csynv1SyncSession {
    uint64_t               id    = 0;
    std::string            name;
    Csynv1SyncState        state = Csynv1SyncState::Idle;
    std::vector<Csynv1ConflictRecord> conflicts;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isPushing()  const { return state == Csynv1SyncState::Pushing; }
    [[nodiscard]] bool isPulling()  const { return state == Csynv1SyncState::Pulling; }
    [[nodiscard]] bool hasConflict() const { return state == Csynv1SyncState::Conflict; }
    [[nodiscard]] bool isComplete() const { return state == Csynv1SyncState::Complete; }
    [[nodiscard]] bool hasError()   const { return state == Csynv1SyncState::Error; }

    bool addConflict(const Csynv1ConflictRecord& record) {
        if (!record.isValid()) return false;
        for (const auto& c : conflicts) if (c.filePath == record.filePath) return false;
        conflicts.push_back(record);
        return true;
    }
};

using Csynv1StateChangeCallback = std::function<void(uint64_t)>;

class CloudSyncEditorV1 {
public:
    static constexpr size_t MAX_SESSIONS = 64;

    bool addSession(const Csynv1SyncSession& session) {
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

    [[nodiscard]] Csynv1SyncSession* findSession(uint64_t id) {
        for (auto& s : m_sessions) if (s.id == id) return &s;
        return nullptr;
    }

    bool setState(uint64_t id, Csynv1SyncState state) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state = state;
        if (m_onStateChange) m_onStateChange(id);
        return true;
    }

    bool resolveConflict(uint64_t sessionId, const std::string& filePath, Csynv1ConflictResolution resolution) {
        auto* s = findSession(sessionId);
        if (!s) return false;
        for (auto& c : s->conflicts) {
            if (c.filePath == filePath) { c.resolution = resolution; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t sessionCount()  const { return m_sessions.size(); }
    [[nodiscard]] size_t conflictCount() const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.hasConflict()) ++c; return c;
    }
    [[nodiscard]] size_t countByState(Csynv1SyncState state) const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.state == state) ++c; return c;
    }

    void setOnStateChange(Csynv1StateChangeCallback cb) { m_onStateChange = std::move(cb); }

private:
    std::vector<Csynv1SyncSession> m_sessions;
    Csynv1StateChangeCallback      m_onStateChange;
};

} // namespace NF
