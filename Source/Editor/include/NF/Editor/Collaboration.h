#pragma once
// NF::Editor — Collab session, conflict resolver, live collaboration
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

enum class CollabUserRole : uint8_t {
    Owner,
    Admin,
    Editor,
    Reviewer,
    Viewer,
    Builder,
    Tester,
    Guest
};

inline const char* collabUserRoleName(CollabUserRole r) noexcept {
    switch (r) {
        case CollabUserRole::Owner:    return "Owner";
        case CollabUserRole::Admin:    return "Admin";
        case CollabUserRole::Editor:   return "Editor";
        case CollabUserRole::Reviewer: return "Reviewer";
        case CollabUserRole::Viewer:   return "Viewer";
        case CollabUserRole::Builder:  return "Builder";
        case CollabUserRole::Tester:   return "Tester";
        case CollabUserRole::Guest:    return "Guest";
        default:                       return "Unknown";
    }
}

struct CollabUser {
    std::string userId;
    std::string displayName;
    CollabUserRole role = CollabUserRole::Guest;
    bool connected = false;
    double lastActivityTime = 0.0;

    [[nodiscard]] bool canEdit() const {
        return role == CollabUserRole::Owner ||
               role == CollabUserRole::Admin ||
               role == CollabUserRole::Editor;
    }

    [[nodiscard]] bool canReview() const {
        return canEdit() || role == CollabUserRole::Reviewer;
    }

    [[nodiscard]] bool isConnected() const { return connected; }
    void connect(double time) { connected = true; lastActivityTime = time; }
    void disconnect() { connected = false; }
    void touch(double time) { lastActivityTime = time; }
};

enum class CollabEditType : uint8_t {
    Insert,
    Delete,
    Modify,
    Move,
    Rename,
    Create,
    Lock,
    Unlock
};

inline const char* collabEditTypeName(CollabEditType t) noexcept {
    switch (t) {
        case CollabEditType::Insert:  return "Insert";
        case CollabEditType::Delete:  return "Delete";
        case CollabEditType::Modify:  return "Modify";
        case CollabEditType::Move:    return "Move";
        case CollabEditType::Rename:  return "Rename";
        case CollabEditType::Create:  return "Create";
        case CollabEditType::Lock:    return "Lock";
        case CollabEditType::Unlock:  return "Unlock";
        default:                      return "Unknown";
    }
}

struct CollabEditAction {
    std::string actionId;
    std::string userId;
    CollabEditType type = CollabEditType::Modify;
    std::string targetPath;
    std::string payload;
    double timestamp = 0.0;
    size_t sequenceNum = 0;
    bool applied = false;
    bool conflicted = false;

    [[nodiscard]] bool isValid() const { return !actionId.empty() && !userId.empty() && !targetPath.empty(); }
    void markApplied() { applied = true; }
    void markConflicted() { conflicted = true; }
};

class CollabSession {
public:
    explicit CollabSession(const std::string& sessionName)
        : m_name(sessionName) {}

    bool addUser(const CollabUser& user) {
        if (m_users.size() >= kMaxUsers) return false;
        for (const auto& u : m_users) {
            if (u.userId == user.userId) return false;
        }
        m_users.push_back(user);
        return true;
    }

    bool removeUser(const std::string& userId) {
        for (auto it = m_users.begin(); it != m_users.end(); ++it) {
            if (it->userId == userId) {
                m_users.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] CollabUser* findUser(const std::string& userId) {
        for (auto& u : m_users) {
            if (u.userId == userId) return &u;
        }
        return nullptr;
    }

    [[nodiscard]] const CollabUser* findUser(const std::string& userId) const {
        for (const auto& u : m_users) {
            if (u.userId == userId) return &u;
        }
        return nullptr;
    }

    bool submitAction(const CollabEditAction& action) {
        if (!action.isValid()) return false;
        auto* user = findUser(action.userId);
        if (!user || !user->canEdit()) return false;
        if (m_actions.size() >= kMaxActions) return false;

        CollabEditAction a = action;
        a.sequenceNum = m_nextSeqNum++;
        a.applied = true;

        // Check for conflicts on same path
        for (const auto& existing : m_actions) {
            if (existing.targetPath == a.targetPath && !existing.conflicted &&
                existing.applied && existing.sequenceNum > 0 &&
                existing.userId != a.userId) {
                // Potential conflict detected if same path edited by different user recently
                if (a.timestamp - existing.timestamp < m_conflictWindowSec) {
                    a.markConflicted();
                    m_conflictCount++;
                    break;
                }
            }
        }

        m_actions.push_back(a);
        user->touch(a.timestamp);
        return true;
    }

    [[nodiscard]] size_t userCount() const { return m_users.size(); }
    [[nodiscard]] size_t actionCount() const { return m_actions.size(); }
    [[nodiscard]] size_t conflictCount() const { return m_conflictCount; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<CollabUser>& users() const { return m_users; }
    [[nodiscard]] const std::vector<CollabEditAction>& actions() const { return m_actions; }

    [[nodiscard]] size_t connectedCount() const {
        size_t count = 0;
        for (const auto& u : m_users) {
            if (u.isConnected()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t editorCount() const {
        size_t count = 0;
        for (const auto& u : m_users) {
            if (u.canEdit()) ++count;
        }
        return count;
    }

    void setConflictWindow(double seconds) { m_conflictWindowSec = seconds; }

    static constexpr size_t kMaxUsers = 32;
    static constexpr size_t kMaxActions = 4096;

private:
    std::string m_name;
    std::vector<CollabUser> m_users;
    std::vector<CollabEditAction> m_actions;
    size_t m_nextSeqNum = 1;
    size_t m_conflictCount = 0;
    double m_conflictWindowSec = 2.0;
};

class CollabConflictResolver {
public:
    struct Resolution {
        std::string actionId;
        bool autoResolved = false;
        std::string strategy;
    };

    Resolution resolve(const CollabEditAction& a, const CollabEditAction& b) {
        Resolution res;
        res.actionId = a.actionId;
        m_totalResolutions++;

        if (a.targetPath != b.targetPath) {
            res.autoResolved = true;
            res.strategy = "no_conflict";
            m_autoResolved++;
            return res;
        }

        // Same path — last-writer-wins if same edit type
        if (a.type == b.type) {
            res.autoResolved = true;
            res.strategy = "last_writer_wins";
            m_autoResolved++;
            return res;
        }

        // Different types on same path — needs manual resolution
        res.autoResolved = false;
        res.strategy = "manual";
        m_manualRequired++;
        return res;
    }

    [[nodiscard]] size_t totalResolutions() const { return m_totalResolutions; }
    [[nodiscard]] size_t autoResolved() const { return m_autoResolved; }
    [[nodiscard]] size_t manualRequired() const { return m_manualRequired; }

    void reset() {
        m_totalResolutions = 0;
        m_autoResolved = 0;
        m_manualRequired = 0;
    }

private:
    size_t m_totalResolutions = 0;
    size_t m_autoResolved = 0;
    size_t m_manualRequired = 0;
};

struct LiveCollabConfig {
    std::string serverAddress = "localhost";
    uint16_t port = 9090;
    double heartbeatIntervalSec = 5.0;
    double inactivityTimeoutSec = 300.0;
    size_t maxSessions = 16;
    size_t maxUsersPerSession = 32;
};

class LiveCollaborationSystem {
public:
    void init(const LiveCollabConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_sessions.clear();
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    int createSession(const std::string& sessionName) {
        if (!m_initialized) return -1;
        if (m_sessions.size() >= m_config.maxSessions) return -1;
        for (const auto& s : m_sessions) {
            if (s.name() == sessionName) return -1;
        }
        m_sessions.emplace_back(sessionName);
        return static_cast<int>(m_sessions.size()) - 1;
    }

    [[nodiscard]] CollabSession* session(int index) {
        if (index < 0 || index >= static_cast<int>(m_sessions.size())) return nullptr;
        return &m_sessions[static_cast<size_t>(index)];
    }

    [[nodiscard]] CollabSession* sessionByName(const std::string& name) {
        for (auto& s : m_sessions) {
            if (s.name() == name) return &s;
        }
        return nullptr;
    }

    bool joinSession(const std::string& sessionName, const CollabUser& user) {
        auto* s = sessionByName(sessionName);
        if (!s) return false;
        return s->addUser(user);
    }

    bool leaveSession(const std::string& sessionName, const std::string& userId) {
        auto* s = sessionByName(sessionName);
        if (!s) return false;
        return s->removeUser(userId);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t sessionCount() const { return m_sessions.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] const LiveCollabConfig& config() const { return m_config; }
    [[nodiscard]] CollabConflictResolver& resolver() { return m_resolver; }
    [[nodiscard]] const CollabConflictResolver& resolver() const { return m_resolver; }

    [[nodiscard]] size_t totalConnectedUsers() const {
        size_t count = 0;
        for (const auto& s : m_sessions) count += s.connectedCount();
        return count;
    }

    [[nodiscard]] size_t totalActions() const {
        size_t count = 0;
        for (const auto& s : m_sessions) count += s.actionCount();
        return count;
    }

    [[nodiscard]] size_t totalConflicts() const {
        size_t count = 0;
        for (const auto& s : m_sessions) count += s.conflictCount();
        return count;
    }

private:
    LiveCollabConfig m_config;
    std::vector<CollabSession> m_sessions;
    CollabConflictResolver m_resolver;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ── S12 — Version Control Integration ───────────────────────────


} // namespace NF
