#pragma once
// NF::Editor — Phase H.5: Notification Center Controller
//
// Provides a UI-level notification center: persistent history, transient toast
// popups, severity filtering, click-to-navigate for actionable notifications.
//
//   ToastNotification    — transient popup with ttl and severity
//   NotificationRecord   — persistent history entry
//   NotificationNavigateAction — navigate callback attached to a record
//   NotificationCenterController —
//       post(record)           — add to history; optionally push as toast
//       postToast(msg, sev)    — push ephemeral toast only
//       tick(dt)               — expire toasts
//       dismiss(id)            — dismiss a history entry
//       dismissAll()           — dismiss all entries
//       setFilter(severity)    — filter visible entries by severity
//       filtered()             — entries passing current filter (newest-first)
//       allHistory()           — full unfiltered history
//       toasts()               — currently active toasts
//       navigateTo(id)         — invoke navigate action for a record
//       historyCount() / toastCount()

#include "NF/Workspace/WorkspacePreferences.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// NotificationSeverity (local, standalone — not dependent on other
// notification enums to avoid transitive header coupling)
// ═════════════════════════════════════════════════════════════════

enum class NotifSeverity : uint8_t {
    Info     = 0,
    Success  = 1,
    Warning  = 2,
    Error    = 3,
    Critical = 4,
};

inline const char* notifSeverityName(NotifSeverity s) {
    switch (s) {
        case NotifSeverity::Info:     return "Info";
        case NotifSeverity::Success:  return "Success";
        case NotifSeverity::Warning:  return "Warning";
        case NotifSeverity::Error:    return "Error";
        case NotifSeverity::Critical: return "Critical";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ToastNotification — transient popup
// ═════════════════════════════════════════════════════════════════

struct ToastNotification {
    std::string  id;
    std::string  message;
    NotifSeverity severity  = NotifSeverity::Info;
    float        ttlSec     = 4.f;
    float        elapsedSec = 0.f;

    [[nodiscard]] bool isExpired() const { return elapsedSec >= ttlSec; }
    [[nodiscard]] float progress() const {
        return ttlSec > 0.f ? std::min(elapsedSec / ttlSec, 1.f) : 1.f;
    }
};

// ═════════════════════════════════════════════════════════════════
// NotificationRecord — persistent history entry
// ═════════════════════════════════════════════════════════════════

using NotificationNavigateAction = std::function<void()>;

struct NotificationRecord {
    std::string                  id;
    std::string                  title;
    std::string                  message;
    NotifSeverity                severity   = NotifSeverity::Info;
    bool                         dismissed  = false;
    bool                         hasToast   = true;  // also push a toast?
    float                        toastTtl   = 4.f;
    NotificationNavigateAction   navigateTo; // callable if actionable; nullptr otherwise

    [[nodiscard]] bool isActionable() const { return navigateTo != nullptr; }
    [[nodiscard]] bool isError()      const { return severity >= NotifSeverity::Error; }
    [[nodiscard]] bool isCritical()   const { return severity == NotifSeverity::Critical; }
};

// ═════════════════════════════════════════════════════════════════
// NotificationCenterController
// ═════════════════════════════════════════════════════════════════

class NotificationCenterController {
public:
    static constexpr size_t MAX_HISTORY = 256;
    static constexpr size_t MAX_TOASTS  = 8;

    // ── Panel open / close ────────────────────────────────────────

    void open()  { m_isOpen = true; }
    void close() { m_isOpen = false; }

    [[nodiscard]] bool isOpen() const { return m_isOpen; }

    // ── Post to history (and optionally as toast) ─────────────────

    bool post(NotificationRecord record) {
        if (record.id.empty() || record.message.empty()) return false;
        // Deduplicate by id
        for (const auto& r : m_history) if (r.id == record.id) return false;
        // Cap history
        if (m_history.size() >= MAX_HISTORY) m_history.erase(m_history.begin());
        if (record.hasToast) pushToast(record.id, record.message, record.severity, record.toastTtl);
        m_history.push_back(std::move(record));
        if (m_onPost) m_onPost(m_history.back());
        return true;
    }

    // ── Post a transient toast without history entry ──────────────

    bool postToast(const std::string& id,
                   const std::string& message,
                   NotifSeverity      severity = NotifSeverity::Info,
                   float              ttlSec   = 4.f) {
        return pushToast(id, message, severity, ttlSec);
    }

    // ── Tick — expire toasts ──────────────────────────────────────

    void tick(float dtSec) {
        for (auto& t : m_toasts) t.elapsedSec += dtSec;
        m_toasts.erase(
            std::remove_if(m_toasts.begin(), m_toasts.end(),
                           [](const ToastNotification& t) { return t.isExpired(); }),
            m_toasts.end());
    }

    // ── Dismiss ───────────────────────────────────────────────────

    bool dismiss(const std::string& id) {
        for (auto& r : m_history) {
            if (r.id == id) { r.dismissed = true; return true; }
        }
        // Also dismiss matching toast
        m_toasts.erase(
            std::remove_if(m_toasts.begin(), m_toasts.end(),
                           [&id](const ToastNotification& t) { return t.id == id; }),
            m_toasts.end());
        return false;
    }

    void dismissAll() {
        for (auto& r : m_history) r.dismissed = true;
        m_toasts.clear();
    }

    // ── Filtering ─────────────────────────────────────────────────

    void setFilter(std::optional<NotifSeverity> severity) { m_filter = severity; }
    void clearFilter() { m_filter = std::nullopt; }

    [[nodiscard]] std::optional<NotifSeverity> filter() const { return m_filter; }

    [[nodiscard]] std::vector<const NotificationRecord*> filtered() const {
        std::vector<const NotificationRecord*> results;
        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
            if (it->dismissed) continue;
            if (m_filter && it->severity != *m_filter) continue;
            results.push_back(&(*it));
        }
        return results;
    }

    [[nodiscard]] std::vector<const NotificationRecord*> allHistory() const {
        std::vector<const NotificationRecord*> results;
        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it)
            results.push_back(&(*it));
        return results;
    }

    // ── Toasts ───────────────────────────────────────────────────

    [[nodiscard]] const std::vector<ToastNotification>& toasts() const { return m_toasts; }

    // ── Click-to-navigate ─────────────────────────────────────────

    bool navigateTo(const std::string& id) {
        for (const auto& r : m_history) {
            if (r.id == id && r.navigateTo) {
                r.navigateTo();
                return true;
            }
        }
        return false;
    }

    // ── Counts ────────────────────────────────────────────────────

    [[nodiscard]] size_t historyCount() const { return m_history.size(); }
    [[nodiscard]] size_t toastCount()   const { return m_toasts.size(); }
    [[nodiscard]] size_t undismissedCount() const {
        size_t c = 0;
        for (const auto& r : m_history) if (!r.dismissed) ++c;
        return c;
    }
    [[nodiscard]] size_t errorCount() const {
        size_t c = 0;
        for (const auto& r : m_history)
            if (!r.dismissed && r.severity >= NotifSeverity::Error) ++c;
        return c;
    }

    // ── Callbacks ─────────────────────────────────────────────────

    void setOnPost(std::function<void(const NotificationRecord&)> cb) {
        m_onPost = std::move(cb);
    }

private:
    std::vector<NotificationRecord>  m_history;
    std::vector<ToastNotification>   m_toasts;
    std::optional<NotifSeverity>     m_filter;
    bool                             m_isOpen = false;
    std::function<void(const NotificationRecord&)> m_onPost;

    bool pushToast(const std::string& id, const std::string& message,
                   NotifSeverity severity, float ttl) {
        if (m_toasts.size() >= MAX_TOASTS) m_toasts.erase(m_toasts.begin());
        // Replace existing toast with same id
        for (auto& t : m_toasts) {
            if (t.id == id) {
                t.message     = message;
                t.severity    = severity;
                t.ttlSec      = ttl;
                t.elapsedSec  = 0.f;
                return true;
            }
        }
        ToastNotification toast;
        toast.id         = id;
        toast.message    = message;
        toast.severity   = severity;
        toast.ttlSec     = ttl;
        toast.elapsedSec = 0.f;
        m_toasts.push_back(std::move(toast));
        return true;
    }
};

} // namespace NF
