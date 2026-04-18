#pragma once
// NF::Editor — in-editor notification management
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

enum class NotifChannel : uint8_t { InEditor, Email, Push, Webhook, Slack, Custom };
inline const char* notifChannelName(NotifChannel v) {
    switch (v) {
        case NotifChannel::InEditor: return "InEditor";
        case NotifChannel::Email:    return "Email";
        case NotifChannel::Push:     return "Push";
        case NotifChannel::Webhook:  return "Webhook";
        case NotifChannel::Slack:    return "Slack";
        case NotifChannel::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class NotifPriority : uint8_t { Low, Normal, High, Critical };
inline const char* notifPriorityName(NotifPriority v) {
    switch (v) {
        case NotifPriority::Low:      return "Low";
        case NotifPriority::Normal:   return "Normal";
        case NotifPriority::High:     return "High";
        case NotifPriority::Critical: return "Critical";
    }
    return "Unknown";
}

class NotifEntry {
public:
    explicit NotifEntry(uint32_t id, const std::string& title, NotifChannel channel, NotifPriority priority)
        : m_id(id), m_title(title), m_channel(channel), m_priority(priority) {}

    void setIsRead(bool v)    { m_isRead    = v; }
    void setIsMuted(bool v)   { m_isMuted   = v; }
    void setIsEnabled(bool v) { m_isEnabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;       }
    [[nodiscard]] const std::string& title()     const { return m_title;    }
    [[nodiscard]] NotifChannel       channel()   const { return m_channel;  }
    [[nodiscard]] NotifPriority      priority()  const { return m_priority; }
    [[nodiscard]] bool               isRead()    const { return m_isRead;   }
    [[nodiscard]] bool               isMuted()   const { return m_isMuted;  }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t      m_id;
    std::string   m_title;
    NotifChannel  m_channel;
    NotifPriority m_priority;
    bool m_isRead    = false;
    bool m_isMuted   = false;
    bool m_isEnabled = true;
};

class NotificationCenterEditor {
public:
    void setIsShowMuted(bool v)          { m_isShowMuted        = v; }
    void setIsGroupByChannel(bool v)     { m_isGroupByChannel   = v; }
    void setMaxNotifications(uint32_t v) { m_maxNotifications   = v; }

    bool addNotif(const NotifEntry& n) {
        for (auto& x : m_notifs) if (x.id() == n.id()) return false;
        m_notifs.push_back(n); return true;
    }
    bool removeNotif(uint32_t id) {
        auto it = std::find_if(m_notifs.begin(), m_notifs.end(),
            [&](const NotifEntry& n){ return n.id() == id; });
        if (it == m_notifs.end()) return false;
        m_notifs.erase(it); return true;
    }
    [[nodiscard]] NotifEntry* findNotif(uint32_t id) {
        for (auto& n : m_notifs) if (n.id() == id) return &n;
        return nullptr;
    }

    [[nodiscard]] bool     isShowMuted()        const { return m_isShowMuted;        }
    [[nodiscard]] bool     isGroupByChannel()   const { return m_isGroupByChannel;   }
    [[nodiscard]] uint32_t maxNotifications()   const { return m_maxNotifications;   }
    [[nodiscard]] size_t   notifCount()         const { return m_notifs.size();      }

    [[nodiscard]] size_t countByChannel(NotifChannel c) const {
        size_t n = 0; for (auto& x : m_notifs) if (x.channel() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByPriority(NotifPriority p) const {
        size_t n = 0; for (auto& x : m_notifs) if (x.priority() == p) ++n; return n;
    }
    [[nodiscard]] size_t countUnread() const {
        size_t n = 0; for (auto& x : m_notifs) if (!x.isRead()) ++n; return n;
    }

private:
    std::vector<NotifEntry> m_notifs;
    bool     m_isShowMuted       = false;
    bool     m_isGroupByChannel  = false;
    uint32_t m_maxNotifications  = 200u;
};

} // namespace NF
