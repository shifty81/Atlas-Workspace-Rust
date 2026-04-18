#pragma once
// NF::Editor — Workspace notification center v1: notification lifecycle and routing authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wncv1Severity   : uint8_t { Info, Warning, Error, Critical };
enum class Wncv1NotifState : uint8_t { New, Read, Dismissed, Archived, Pinned };
enum class Wncv1NotifScope : uint8_t { Global, Tool, Project, Build, AtlasAI };

inline const char* wncv1SeverityName(Wncv1Severity s) {
    switch (s) {
        case Wncv1Severity::Info:     return "Info";
        case Wncv1Severity::Warning:  return "Warning";
        case Wncv1Severity::Error:    return "Error";
        case Wncv1Severity::Critical: return "Critical";
    }
    return "Unknown";
}

inline const char* wncv1NotifStateName(Wncv1NotifState s) {
    switch (s) {
        case Wncv1NotifState::New:       return "New";
        case Wncv1NotifState::Read:      return "Read";
        case Wncv1NotifState::Dismissed: return "Dismissed";
        case Wncv1NotifState::Archived:  return "Archived";
        case Wncv1NotifState::Pinned:    return "Pinned";
    }
    return "Unknown";
}

inline const char* wncv1NotifScopeName(Wncv1NotifScope s) {
    switch (s) {
        case Wncv1NotifScope::Global:  return "Global";
        case Wncv1NotifScope::Tool:    return "Tool";
        case Wncv1NotifScope::Project: return "Project";
        case Wncv1NotifScope::Build:   return "Build";
        case Wncv1NotifScope::AtlasAI: return "AtlasAI";
    }
    return "Unknown";
}

struct Wncv1Notification {
    uint64_t          id         = 0;
    std::string       title;
    std::string       message;
    Wncv1Severity     severity   = Wncv1Severity::Info;
    Wncv1NotifState   state      = Wncv1NotifState::New;
    Wncv1NotifScope   scope      = Wncv1NotifScope::Global;
    std::string       sourceId;
    bool              hasAction  = false;

    [[nodiscard]] bool isValid()     const { return id != 0 && !title.empty(); }
    [[nodiscard]] bool isNew()       const { return state == Wncv1NotifState::New; }
    [[nodiscard]] bool isPinned()    const { return state == Wncv1NotifState::Pinned; }
    [[nodiscard]] bool isArchived()  const { return state == Wncv1NotifState::Archived; }
    [[nodiscard]] bool isCritical()  const { return severity == Wncv1Severity::Critical; }
};

using Wncv1NotifCallback = std::function<void(uint64_t)>;

class WorkspaceNotificationCenterV1 {
public:
    static constexpr size_t MAX_NOTIFICATIONS = 1024;

    bool post(const Wncv1Notification& notif) {
        if (!notif.isValid()) return false;
        for (const auto& n : m_notifications) if (n.id == notif.id) return false;
        if (m_notifications.size() >= MAX_NOTIFICATIONS) return false;
        m_notifications.push_back(notif);
        if (m_onPost) m_onPost(notif.id);
        return true;
    }

    bool dismiss(uint64_t id) {
        auto* n = findNotif(id);
        if (!n) return false;
        n->state = Wncv1NotifState::Dismissed;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool archive(uint64_t id) {
        auto* n = findNotif(id);
        if (!n) return false;
        n->state = Wncv1NotifState::Archived;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool pin(uint64_t id) {
        auto* n = findNotif(id);
        if (!n) return false;
        n->state = Wncv1NotifState::Pinned;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool markRead(uint64_t id) {
        auto* n = findNotif(id);
        if (!n) return false;
        if (n->state == Wncv1NotifState::New) n->state = Wncv1NotifState::Read;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool remove(uint64_t id) {
        for (auto it = m_notifications.begin(); it != m_notifications.end(); ++it) {
            if (it->id == id) { m_notifications.erase(it); return true; }
        }
        return false;
    }

    void clearDismissed() {
        m_notifications.erase(
            std::remove_if(m_notifications.begin(), m_notifications.end(),
                [](const Wncv1Notification& n){ return n.state == Wncv1NotifState::Dismissed; }),
            m_notifications.end());
    }

    [[nodiscard]] Wncv1Notification* findNotif(uint64_t id) {
        for (auto& n : m_notifications) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] size_t notifCount()    const { return m_notifications.size(); }
    [[nodiscard]] size_t newCount()      const {
        size_t c = 0; for (const auto& n : m_notifications) if (n.isNew())      ++c; return c;
    }
    [[nodiscard]] size_t pinnedCount()   const {
        size_t c = 0; for (const auto& n : m_notifications) if (n.isPinned())   ++c; return c;
    }
    [[nodiscard]] size_t criticalCount() const {
        size_t c = 0; for (const auto& n : m_notifications) if (n.isCritical()) ++c; return c;
    }
    [[nodiscard]] size_t countBySeverity(Wncv1Severity sev) const {
        size_t c = 0; for (const auto& n : m_notifications) if (n.severity == sev) ++c; return c;
    }
    [[nodiscard]] size_t countByScope(Wncv1NotifScope scope) const {
        size_t c = 0; for (const auto& n : m_notifications) if (n.scope == scope) ++c; return c;
    }

    void setOnPost(Wncv1NotifCallback cb)   { m_onPost   = std::move(cb); }
    void setOnChange(Wncv1NotifCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wncv1Notification> m_notifications;
    Wncv1NotifCallback             m_onPost;
    Wncv1NotifCallback             m_onChange;
};

} // namespace NF
