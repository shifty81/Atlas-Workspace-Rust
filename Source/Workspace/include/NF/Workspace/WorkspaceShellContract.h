#pragma once
// NF::Editor — Workspace shell contract: formal surface API for shell operations
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include "NF/Workspace/NotificationSystem.h"
#include "NF/Workspace/LayoutManagerV1.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Shell Event Type ──────────────────────────────────────────────

enum class ShellEventType : uint8_t {
    AppLaunched,
    AppClosed,
    ProjectOpened,
    ProjectClosed,
    LayoutChanged,
    NotificationPosted,
    ThemeChanged,
    SettingsChanged,
    FocusPanelChanged,
};

inline const char* shellEventTypeName(ShellEventType t) {
    switch (t) {
        case ShellEventType::AppLaunched:       return "AppLaunched";
        case ShellEventType::AppClosed:         return "AppClosed";
        case ShellEventType::ProjectOpened:     return "ProjectOpened";
        case ShellEventType::ProjectClosed:     return "ProjectClosed";
        case ShellEventType::LayoutChanged:     return "LayoutChanged";
        case ShellEventType::NotificationPosted:return "NotificationPosted";
        case ShellEventType::ThemeChanged:      return "ThemeChanged";
        case ShellEventType::SettingsChanged:   return "SettingsChanged";
        case ShellEventType::FocusPanelChanged: return "FocusPanelChanged";
    }
    return "Unknown";
}

// ── Shell Event ───────────────────────────────────────────────────

struct ShellEvent {
    ShellEventType type    = ShellEventType::SettingsChanged;
    std::string    payload;  // context-dependent string payload
    uint64_t       seq     = 0;

    [[nodiscard]] bool isValid() const { return seq != 0; }
};

// ── Shell Status ──────────────────────────────────────────────────

enum class ShellStatus : uint8_t {
    Idle,
    Loading,
    Ready,
    Error,
    ShuttingDown,
};

inline const char* shellStatusName(ShellStatus s) {
    switch (s) {
        case ShellStatus::Idle:         return "Idle";
        case ShellStatus::Loading:      return "Loading";
        case ShellStatus::Ready:        return "Ready";
        case ShellStatus::Error:        return "Error";
        case ShellStatus::ShuttingDown: return "ShuttingDown";
    }
    return "Unknown";
}

// ── Workspace Shell Contract ──────────────────────────────────────
// The shell contract is the single authoritative surface for:
//  - launching/stopping registered apps
//  - posting notifications
//  - changing layouts
//  - routing events to subscribers

using ShellEventCallback = std::function<void(const ShellEvent&)>;

class WorkspaceShellContract {
public:
    static constexpr size_t MAX_SUBSCRIBERS = 64;

    void initialize() {
        m_status = ShellStatus::Ready;
        m_initialized = true;
    }

    void shutdown() {
        m_status = ShellStatus::ShuttingDown;
    }

    // ── App Launch/Stop ───────────────────────────────────────────

    bool launchApp(WorkspaceAppId appId, const std::string& args = "") {
        if (!m_initialized) return false;
        if (m_appRegistry) {
            const auto* desc = m_appRegistry->find(appId);
            if (!desc || !desc->isValid()) return false;
            if (!desc->allowDirectLaunch && appId != WorkspaceAppId::TileEditor) return false;
        }
        m_runningApps.push_back(appId);
        std::string payload = std::string(workspaceAppName(appId)) + " " + args;
        postEvent(ShellEventType::AppLaunched, payload);
        ++m_launchCount;
        return true;
    }

    bool stopApp(WorkspaceAppId appId) {
        auto it = std::find(m_runningApps.begin(), m_runningApps.end(), appId);
        if (it == m_runningApps.end()) return false;
        m_runningApps.erase(it);
        postEvent(ShellEventType::AppClosed, workspaceAppName(appId));
        return true;
    }

    [[nodiscard]] bool isAppRunning(WorkspaceAppId appId) const {
        return std::find(m_runningApps.begin(), m_runningApps.end(), appId) != m_runningApps.end();
    }

    // ── Notifications ─────────────────────────────────────────────

    bool postNotification(const Notification& n) {
        if (!m_initialized) return false;
        m_recentNotifications.push_back(n);
        if (m_recentNotifications.size() > 100) m_recentNotifications.erase(m_recentNotifications.begin());
        postEvent(ShellEventType::NotificationPosted, n.title);
        ++m_notificationCount;
        return true;
    }

    // ── Layout ────────────────────────────────────────────────────

    bool applyLayout(uint32_t layoutSlotId) {
        if (!m_layoutManager) return false;
        if (!m_layoutManager->findSlot(layoutSlotId)) return false;
        m_activeLayoutSlotId = layoutSlotId;
        m_layoutManager->setActiveSlot(layoutSlotId);
        postEvent(ShellEventType::LayoutChanged, std::to_string(layoutSlotId));
        return true;
    }

    // ── Focus Panel ───────────────────────────────────────────────

    void setFocusPanel(const std::string& panelId) {
        m_focusPanelId = panelId;
        postEvent(ShellEventType::FocusPanelChanged, panelId);
    }

    // ── Event subscription ────────────────────────────────────────

    bool subscribe(const std::string& subscriberId, ShellEventCallback cb) {
        if (m_subscribers.size() >= MAX_SUBSCRIBERS) return false;
        for (const auto& [id, _] : m_subscribers) if (id == subscriberId) return false;
        m_subscribers.push_back({ subscriberId, std::move(cb) });
        return true;
    }

    bool unsubscribe(const std::string& subscriberId) {
        for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it) {
            if (it->first == subscriberId) { m_subscribers.erase(it); return true; }
        }
        return false;
    }

    // ── Bindings ─────────────────────────────────────────────────

    void bindAppRegistry(WorkspaceAppRegistry* reg) { m_appRegistry    = reg; }
    void bindLayoutManager(LayoutManagerV1* mgr)    { m_layoutManager  = mgr; }

    // ── Accessors ─────────────────────────────────────────────────

    [[nodiscard]] bool         isInitialized()       const { return m_initialized;     }
    [[nodiscard]] ShellStatus  status()              const { return m_status;          }
    [[nodiscard]] size_t       launchCount()         const { return m_launchCount;     }
    [[nodiscard]] size_t       notificationCount()   const { return m_notificationCount; }
    [[nodiscard]] size_t       subscriberCount()     const { return m_subscribers.size(); }
    [[nodiscard]] size_t       eventCount()          const { return m_eventSeq;         }
    [[nodiscard]] uint32_t     activeLayoutSlotId()  const { return m_activeLayoutSlotId; }
    [[nodiscard]] const std::string& focusPanelId()  const { return m_focusPanelId;    }
    [[nodiscard]] const std::vector<WorkspaceAppId>& runningApps() const { return m_runningApps; }
    [[nodiscard]] const std::vector<Notification>& recentNotifications() const { return m_recentNotifications; }

private:
    void postEvent(ShellEventType type, const std::string& payload = "") {
        ShellEvent e;
        e.type    = type;
        e.payload = payload;
        e.seq     = ++m_eventSeq;
        for (auto& [id, cb] : m_subscribers) cb(e);
    }

    WorkspaceAppRegistry* m_appRegistry   = nullptr;
    LayoutManagerV1*      m_layoutManager = nullptr;

    std::vector<WorkspaceAppId> m_runningApps;
    std::vector<Notification>   m_recentNotifications;
    std::vector<std::pair<std::string, ShellEventCallback>> m_subscribers;

    std::string  m_focusPanelId;
    ShellStatus  m_status            = ShellStatus::Idle;
    uint32_t     m_activeLayoutSlotId = 0;
    uint64_t     m_eventSeq           = 0;
    size_t       m_launchCount        = 0;
    size_t       m_notificationCount  = 0;
    bool         m_initialized        = false;
};

} // namespace NF
