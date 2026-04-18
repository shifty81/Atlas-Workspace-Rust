#pragma once
// NF::Editor — Notification workflow rules, severity routing, priority queue
#include "NF/Workspace/NotificationSystem.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Workflow Rule ────────────────────────────────────────────────
// Defines automatic behavior for notifications based on severity.

enum class NotificationAction : uint8_t {
    Show,           // Display normally
    AutoDismiss,    // Auto-dismiss after duration
    Pin,            // Pin to notification center
    Escalate,       // Bump severity up one level
    Suppress,       // Don't show
    Log,            // Log only (to console/codex)
    Sound,          // Play a sound
};

inline const char* notificationActionName(NotificationAction a) {
    switch (a) {
        case NotificationAction::Show:        return "Show";
        case NotificationAction::AutoDismiss: return "AutoDismiss";
        case NotificationAction::Pin:         return "Pin";
        case NotificationAction::Escalate:    return "Escalate";
        case NotificationAction::Suppress:    return "Suppress";
        case NotificationAction::Log:         return "Log";
        case NotificationAction::Sound:       return "Sound";
        default:                              return "Unknown";
    }
}

struct WorkflowRule {
    std::string          name;
    NotificationSeverity matchSeverity = NotificationSeverity::Info;
    NotificationAction   action        = NotificationAction::Show;
    uint32_t             autoDismissMs = 3000;
    bool                 enabled       = true;
    int                  priority      = 0;   // higher = checked first

    [[nodiscard]] bool matches(NotificationSeverity sev) const {
        return enabled && sev == matchSeverity;
    }
};

// ── Rate Limiter ─────────────────────────────────────────────────
// Prevents notification flooding by tracking recent post counts.

class NotificationRateLimiter {
public:
    explicit NotificationRateLimiter(size_t maxPerWindow = 10, float windowSeconds = 5.f)
        : m_maxPerWindow(maxPerWindow), m_windowSeconds(windowSeconds) {}

    bool shouldAllow() {
        if (m_recentCount >= m_maxPerWindow) return false;
        ++m_recentCount;
        return true;
    }

    void tick(float dt) {
        m_elapsed += dt;
        if (m_elapsed >= m_windowSeconds) {
            m_recentCount = 0;
            m_elapsed = 0.f;
        }
    }

    void reset() {
        m_recentCount = 0;
        m_elapsed = 0.f;
    }

    [[nodiscard]] size_t recentCount()    const { return m_recentCount;    }
    [[nodiscard]] size_t maxPerWindow()   const { return m_maxPerWindow;   }
    [[nodiscard]] float  windowSeconds()  const { return m_windowSeconds;  }
    [[nodiscard]] bool   isThrottled()    const { return m_recentCount >= m_maxPerWindow; }

    void setMaxPerWindow(size_t max) { m_maxPerWindow = max; }
    void setWindowSeconds(float sec) { m_windowSeconds = sec > 0.f ? sec : 1.f; }

private:
    size_t m_maxPerWindow;
    float  m_windowSeconds;
    size_t m_recentCount = 0;
    float  m_elapsed     = 0.f;
};

// ── Priority Queue ───────────────────────────────────────────────
// Orders notifications by severity for display priority.

class NotificationPriorityQueue {
public:
    void enqueue(Notification n) {
        m_queue.push_back(std::move(n));
        sortQueue();
    }

    [[nodiscard]] bool empty() const { return m_queue.empty(); }
    [[nodiscard]] size_t size() const { return m_queue.size(); }

    [[nodiscard]] const Notification& top() const { return m_queue.front(); }

    Notification dequeue() {
        Notification n = std::move(m_queue.front());
        m_queue.erase(m_queue.begin());
        return n;
    }

    void clear() { m_queue.clear(); }

    // Peek at notification by index
    [[nodiscard]] const Notification* at(size_t index) const {
        return index < m_queue.size() ? &m_queue[index] : nullptr;
    }

private:
    void sortQueue() {
        std::stable_sort(m_queue.begin(), m_queue.end(),
            [](const Notification& a, const Notification& b) {
                return static_cast<uint8_t>(a.severity) > static_cast<uint8_t>(b.severity);
            });
    }

    std::vector<Notification> m_queue;
};

// ── Notification Workflow Engine ─────────────────────────────────
// Applies rules to incoming notifications, manages rate limiting
// and priority ordering.

class NotificationWorkflowEngine {
public:
    static constexpr size_t MAX_RULES = 32;

    bool addRule(const WorkflowRule& rule) {
        if (m_rules.size() >= MAX_RULES) return false;
        for (const auto& r : m_rules) if (r.name == rule.name) return false;
        m_rules.push_back(rule);
        sortRules();
        return true;
    }

    bool removeRule(const std::string& name) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->name == name) { m_rules.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const WorkflowRule* findRule(const std::string& name) const {
        for (const auto& r : m_rules) if (r.name == name) return &r;
        return nullptr;
    }

    void enableRule(const std::string& name, bool enabled) {
        for (auto& r : m_rules) {
            if (r.name == name) { r.enabled = enabled; return; }
        }
    }

    // Process a notification through rules. Returns the action to take.
    NotificationAction processNotification(const Notification& n) {
        if (!m_rateLimiter.shouldAllow()) {
            ++m_suppressed;
            return NotificationAction::Suppress;
        }

        for (const auto& rule : m_rules) {
            if (rule.matches(n.severity)) {
                ++m_processed;
                return rule.action;
            }
        }
        ++m_processed;
        return NotificationAction::Show; // default
    }

    void tick(float dt) {
        m_rateLimiter.tick(dt);
    }

    // Accessors
    [[nodiscard]] size_t ruleCount()     const { return m_rules.size(); }
    [[nodiscard]] size_t processed()     const { return m_processed;    }
    [[nodiscard]] size_t suppressed()    const { return m_suppressed;   }
    [[nodiscard]] const std::vector<WorkflowRule>& rules() const { return m_rules; }

    [[nodiscard]] NotificationRateLimiter& rateLimiter() { return m_rateLimiter; }
    [[nodiscard]] const NotificationRateLimiter& rateLimiter() const { return m_rateLimiter; }

    [[nodiscard]] NotificationPriorityQueue& priorityQueue() { return m_priorityQueue; }
    [[nodiscard]] const NotificationPriorityQueue& priorityQueue() const { return m_priorityQueue; }

    // Add default rules for standard severities
    void loadDefaults() {
        addRule({"info_auto_dismiss", NotificationSeverity::Info,
                 NotificationAction::AutoDismiss, 3000, true, 0});
        addRule({"success_auto_dismiss", NotificationSeverity::Success,
                 NotificationAction::AutoDismiss, 2000, true, 0});
        addRule({"warning_show", NotificationSeverity::Warning,
                 NotificationAction::Show, 5000, true, 10});
        addRule({"error_pin", NotificationSeverity::Error,
                 NotificationAction::Pin, 0, true, 20});
        addRule({"critical_pin", NotificationSeverity::Critical,
                 NotificationAction::Pin, 0, true, 30});
        addRule({"debug_log", NotificationSeverity::Debug,
                 NotificationAction::Log, 0, true, -10});
        addRule({"trace_suppress", NotificationSeverity::Trace,
                 NotificationAction::Suppress, 0, true, -20});
    }

private:
    void sortRules() {
        std::stable_sort(m_rules.begin(), m_rules.end(),
            [](const WorkflowRule& a, const WorkflowRule& b) {
                return a.priority > b.priority;
            });
    }

    std::vector<WorkflowRule>   m_rules;
    NotificationRateLimiter     m_rateLimiter;
    NotificationPriorityQueue   m_priorityQueue;
    size_t m_processed  = 0;
    size_t m_suppressed = 0;
};


} // namespace NF
