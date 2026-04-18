#pragma once
// NF::Editor — Notification channel + system (advanced)
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

enum class NotificationSeverity : uint8_t { Info, Success, Warning, Error, Critical, Debug, Trace, System };

inline const char* notificationSeverityName(NotificationSeverity s) {
    switch (s) {
        case NotificationSeverity::Info:     return "Info";
        case NotificationSeverity::Success:  return "Success";
        case NotificationSeverity::Warning:  return "Warning";
        case NotificationSeverity::Error:    return "Error";
        case NotificationSeverity::Critical: return "Critical";
        case NotificationSeverity::Debug:    return "Debug";
        case NotificationSeverity::Trace:    return "Trace";
        case NotificationSeverity::System:   return "System";
        default:                             return "Unknown";
    }
}

enum class NotificationState : uint8_t { Pending, Shown, Dismissed, Expired };

inline const char* notificationStateName(NotificationState s) {
    switch (s) {
        case NotificationState::Pending:   return "Pending";
        case NotificationState::Shown:     return "Shown";
        case NotificationState::Dismissed: return "Dismissed";
        case NotificationState::Expired:   return "Expired";
        default:                           return "Unknown";
    }
}

struct Notification {
    std::string          id;
    std::string          title;
    std::string          message;
    NotificationSeverity severity   = NotificationSeverity::Info;
    NotificationState    state      = NotificationState::Pending;
    uint32_t             durationMs = 3000;
    bool                 persistent = false;

    void dismiss() { state = NotificationState::Dismissed; }
    void expire()  { state = NotificationState::Expired;   }
    void show()    { state = NotificationState::Shown;     }

    [[nodiscard]] bool isDismissed() const { return state == NotificationState::Dismissed; }
    [[nodiscard]] bool isExpired()   const { return state == NotificationState::Expired;   }
    [[nodiscard]] bool isVisible()   const { return state == NotificationState::Shown;     }
    [[nodiscard]] bool isError()     const { return severity >= NotificationSeverity::Error; }
    [[nodiscard]] bool isCritical()  const { return severity == NotificationSeverity::Critical; }
};

class NotificationChannel {
public:
    explicit NotificationChannel(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name()              const { return m_name; }
    [[nodiscard]] size_t             notificationCount() const { return m_notifications.size(); }

    bool post(Notification n) {
        for (auto& existing : m_notifications) if (existing.id == n.id) return false;
        n.show();
        m_notifications.push_back(std::move(n));
        return true;
    }

    bool dismiss(const std::string& id) {
        for (auto& n : m_notifications) {
            if (n.id == id) { n.dismiss(); return true; }
        }
        return false;
    }

    [[nodiscard]] Notification* find(const std::string& id) {
        for (auto& n : m_notifications) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& n : m_notifications) if (n.isVisible()) c++;
        return c;
    }

    [[nodiscard]] size_t errorCount() const {
        size_t c = 0;
        for (auto& n : m_notifications) if (n.isError()) c++;
        return c;
    }

    size_t clearDismissed() {
        size_t before = m_notifications.size();
        m_notifications.erase(
            std::remove_if(m_notifications.begin(), m_notifications.end(),
                [](const Notification& n){ return n.isDismissed() || n.isExpired(); }),
            m_notifications.end());
        return before - m_notifications.size();
    }

private:
    std::string               m_name;
    std::vector<Notification> m_notifications;
};

class NotificationSystem {
public:
    static constexpr size_t MAX_CHANNELS = 16;

    NotificationChannel* createChannel(const std::string& name) {
        if (m_channels.size() >= MAX_CHANNELS) return nullptr;
        for (auto& c : m_channels) if (c.name() == name) return nullptr;
        m_channels.emplace_back(name);
        return &m_channels.back();
    }

    bool removeChannel(const std::string& name) {
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            if (it->name() == name) { m_channels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] NotificationChannel* findChannel(const std::string& name) {
        for (auto& c : m_channels) if (c.name() == name) return &c;
        return nullptr;
    }

    bool post(const std::string& channelName, Notification n) {
        auto* ch = findChannel(channelName);
        if (!ch) return false;
        return ch->post(std::move(n));
    }

    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

    [[nodiscard]] size_t totalActive() const {
        size_t c = 0;
        for (auto& ch : m_channels) c += ch.activeCount();
        return c;
    }

private:
    std::vector<NotificationChannel> m_channels;
};

// ============================================================
// S25 — Undo/Redo System
// ============================================================


} // namespace NF
