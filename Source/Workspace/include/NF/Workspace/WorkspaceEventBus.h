#pragma once
// NF::Workspace — Phase 12: Event Bus and Workspace Notifications
//
// Workspace-level publish/subscribe event infrastructure:
//   WorkspaceEventType    — event category enum
//   WorkspaceEvent        — typed event descriptor
//   WorkspaceEventBus     — synchronous pub/sub with per-type lists and wildcards
//   WorkspaceEventQueue   — deferred event accumulation with tick-based drain
//   WorkspaceNotificationBus — higher-level notification model layered on EventBus

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// WorkspaceEventType — event category classification
// ═════════════════════════════════════════════════════════════════

enum class WorkspaceEventType : uint8_t {
    Tool         = 0,   // Tool lifecycle (activate, suspend, shutdown)
    Panel        = 1,   // Panel visibility, focus, context change
    Project      = 2,   // Project load, unload, save, switch
    Asset        = 3,   // Asset import, dirty, delete, rename
    Command      = 4,   // Command execute, undo, redo
    Selection    = 5,   // Selection changed, cleared
    Layout       = 6,   // Layout save, load, reset, dock change
    Notification = 7,   // Notification posted, dismissed, expired
    AI           = 8,   // AtlasAI broker events, suggestions
    System       = 9,   // Startup, shutdown, settings change
    Custom       = 10,  // User-defined / plugin events
};

inline const char* workspaceEventTypeName(WorkspaceEventType t) {
    switch (t) {
        case WorkspaceEventType::Tool:         return "Tool";
        case WorkspaceEventType::Panel:        return "Panel";
        case WorkspaceEventType::Project:      return "Project";
        case WorkspaceEventType::Asset:        return "Asset";
        case WorkspaceEventType::Command:      return "Command";
        case WorkspaceEventType::Selection:    return "Selection";
        case WorkspaceEventType::Layout:       return "Layout";
        case WorkspaceEventType::Notification: return "Notification";
        case WorkspaceEventType::AI:           return "AI";
        case WorkspaceEventType::System:       return "System";
        case WorkspaceEventType::Custom:       return "Custom";
        default:                               return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// WorkspaceEventPriority — dispatch ordering within a tick
// ═════════════════════════════════════════════════════════════════

enum class WorkspaceEventPriority : uint8_t {
    Low      = 0,
    Normal   = 1,
    High     = 2,
    Critical = 3,
};

inline const char* workspaceEventPriorityName(WorkspaceEventPriority p) {
    switch (p) {
        case WorkspaceEventPriority::Low:      return "Low";
        case WorkspaceEventPriority::Normal:   return "Normal";
        case WorkspaceEventPriority::High:     return "High";
        case WorkspaceEventPriority::Critical: return "Critical";
        default:                               return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// WorkspaceEvent — typed event descriptor
// ═════════════════════════════════════════════════════════════════

struct WorkspaceEvent {
    WorkspaceEventType     eventType  = WorkspaceEventType::Custom;
    std::string            source;           // originator identifier
    std::string            payload;          // free-form payload string
    uint64_t               timestampToken = 0;
    WorkspaceEventPriority priority   = WorkspaceEventPriority::Normal;

    [[nodiscard]] bool isValid() const {
        return !source.empty();
    }

    [[nodiscard]] bool isHighPriority() const {
        return priority >= WorkspaceEventPriority::High;
    }

    [[nodiscard]] bool isCritical() const {
        return priority == WorkspaceEventPriority::Critical;
    }

    static WorkspaceEvent make(WorkspaceEventType type, const std::string& src,
                               const std::string& pay = {},
                               WorkspaceEventPriority prio = WorkspaceEventPriority::Normal,
                               uint64_t ts = 0) {
        return {type, src, pay, ts, prio};
    }
};

// ═════════════════════════════════════════════════════════════════
// WorkspaceEventSubscription — subscriber entry
// ═════════════════════════════════════════════════════════════════

class WorkspaceEventSubscription {
public:
    using SubscriptionId = uint32_t;
    using Handler = std::function<void(const WorkspaceEvent&)>;

    WorkspaceEventSubscription(SubscriptionId id, WorkspaceEventType type,
                                const std::string& sourceFilter, Handler handler)
        : m_id(id), m_eventType(type), m_sourceFilter(sourceFilter),
          m_handler(std::move(handler)) {}

    [[nodiscard]] SubscriptionId id()          const { return m_id;           }
    [[nodiscard]] WorkspaceEventType type()    const { return m_eventType;    }
    [[nodiscard]] const std::string& source()  const { return m_sourceFilter; }
    [[nodiscard]] bool   isActive()            const { return m_active;       }
    [[nodiscard]] size_t deliveryCount()       const { return m_deliveryCount;}
    [[nodiscard]] bool   isWildcard()          const { return m_wildcard;     }

    void setWildcard(bool w) { m_wildcard = w; }
    void cancel() { m_active = false; }

    bool matches(const WorkspaceEvent& ev) const {
        if (!m_active) return false;
        if (!m_wildcard && ev.eventType != m_eventType) return false;
        if (!m_sourceFilter.empty() && ev.source != m_sourceFilter) return false;
        return true;
    }

    void deliver(const WorkspaceEvent& ev) {
        if (!matches(ev)) return;
        if (m_handler) {
            m_handler(ev);
            ++m_deliveryCount;
        }
    }

private:
    SubscriptionId       m_id;
    WorkspaceEventType   m_eventType;
    std::string          m_sourceFilter;  // empty = match all sources
    Handler              m_handler;
    size_t               m_deliveryCount = 0;
    bool                 m_active        = true;
    bool                 m_wildcard      = false; // true = match all event types
};

// ═════════════════════════════════════════════════════════════════
// WorkspaceEventBus — synchronous publish/subscribe
// ═════════════════════════════════════════════════════════════════

class WorkspaceEventBus {
public:
    using SubscriptionId = WorkspaceEventSubscription::SubscriptionId;
    static constexpr size_t MAX_SUBSCRIPTIONS = 256;

    // Subscribe to a specific event type. sourceFilter empty = all sources.
    SubscriptionId subscribe(WorkspaceEventType type,
                             WorkspaceEventSubscription::Handler handler,
                             const std::string& sourceFilter = {}) {
        if (m_subscriptions.size() >= MAX_SUBSCRIPTIONS) return 0;
        SubscriptionId id = ++m_nextId;
        m_subscriptions.emplace_back(id, type, sourceFilter, std::move(handler));
        return id;
    }

    // Subscribe to ALL event types (wildcard).
    SubscriptionId subscribeAll(WorkspaceEventSubscription::Handler handler,
                                const std::string& sourceFilter = {}) {
        if (m_subscriptions.size() >= MAX_SUBSCRIPTIONS) return 0;
        SubscriptionId id = ++m_nextId;
        m_subscriptions.emplace_back(id, WorkspaceEventType::Custom, sourceFilter,
                                      std::move(handler));
        m_subscriptions.back().setWildcard(true);
        return id;
    }

    // Unsubscribe by id. Returns true if found and removed.
    bool unsubscribe(SubscriptionId id) {
        for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
            if (it->id() == id) {
                m_subscriptions.erase(it);
                return true;
            }
        }
        return false;
    }

    // Publish synchronously — dispatches to all matching subscribers immediately.
    size_t publish(const WorkspaceEvent& ev) {
        if (!ev.isValid()) return 0;
        size_t dispatched = 0;
        for (auto& sub : m_subscriptions) {
            if (sub.matches(ev)) {
                sub.deliver(ev);
                ++dispatched;
            }
        }
        ++m_totalPublished;
        m_totalDispatches += dispatched;
        return dispatched;
    }

    // Find subscription by id.
    [[nodiscard]] const WorkspaceEventSubscription* find(SubscriptionId id) const {
        for (const auto& sub : m_subscriptions) {
            if (sub.id() == id) return &sub;
        }
        return nullptr;
    }

    // Count subscriptions for a given event type.
    [[nodiscard]] size_t countByType(WorkspaceEventType type) const {
        size_t c = 0;
        for (const auto& sub : m_subscriptions) {
            if (!sub.isWildcard() && sub.type() == type) ++c;
        }
        return c;
    }

    [[nodiscard]] size_t subscriptionCount() const { return m_subscriptions.size(); }
    [[nodiscard]] size_t totalPublished()    const { return m_totalPublished;       }
    [[nodiscard]] size_t totalDispatches()   const { return m_totalDispatches;      }
    [[nodiscard]] bool   empty()             const { return m_subscriptions.empty();}

    void clear() {
        m_subscriptions.clear();
        m_totalPublished  = 0;
        m_totalDispatches = 0;
    }

private:
    std::vector<WorkspaceEventSubscription> m_subscriptions;
    SubscriptionId m_nextId         = 0;
    size_t         m_totalPublished  = 0;
    size_t         m_totalDispatches = 0;
};

// ═════════════════════════════════════════════════════════════════
// WorkspaceEventQueue — deferred/async event accumulation
// ═════════════════════════════════════════════════════════════════

class WorkspaceEventQueue {
public:
    static constexpr size_t MAX_QUEUE_SIZE = 1024;

    explicit WorkspaceEventQueue(WorkspaceEventBus& bus) : m_bus(bus) {}

    // Enqueue an event for deferred dispatch.
    bool enqueue(const WorkspaceEvent& ev) {
        if (!ev.isValid()) return false;
        if (m_queue.size() >= MAX_QUEUE_SIZE) return false;
        m_queue.push_back(ev);
        return true;
    }

    // Drain the queue — dispatches accumulated events via the bus.
    // Events are sorted by priority (Critical first) before dispatch.
    // Returns total number of subscriber deliveries.
    size_t drain() {
        if (m_queue.empty()) return 0;

        // Sort by priority descending (Critical > High > Normal > Low)
        std::stable_sort(m_queue.begin(), m_queue.end(),
            [](const WorkspaceEvent& a, const WorkspaceEvent& b) {
                return static_cast<uint8_t>(a.priority) > static_cast<uint8_t>(b.priority);
            });

        size_t totalDeliveries = 0;
        for (const auto& ev : m_queue) {
            totalDeliveries += m_bus.publish(ev);
        }
        size_t drained = m_queue.size();
        m_totalDrained += drained;
        m_queue.clear();
        return totalDeliveries;
    }

    // Tick-based drain: accumulate events over time, drain when interval elapses.
    // Returns number of deliveries if drained this tick, 0 otherwise.
    size_t tick(float dt) {
        m_elapsed += dt;
        if (m_elapsed >= m_drainIntervalSeconds && !m_queue.empty()) {
            m_elapsed = 0.f;
            return drain();
        }
        return 0;
    }

    void setDrainInterval(float seconds) {
        m_drainIntervalSeconds = seconds > 0.f ? seconds : 0.016f;
    }

    [[nodiscard]] size_t queueSize()       const { return m_queue.size();           }
    [[nodiscard]] size_t totalDrained()    const { return m_totalDrained;           }
    [[nodiscard]] float  drainInterval()   const { return m_drainIntervalSeconds;   }
    [[nodiscard]] float  elapsed()         const { return m_elapsed;                }
    [[nodiscard]] bool   empty()           const { return m_queue.empty();          }

    void clearQueue() { m_queue.clear(); }

    // Peek at pending events (read-only).
    [[nodiscard]] const std::vector<WorkspaceEvent>& pending() const { return m_queue; }

private:
    WorkspaceEventBus&         m_bus;
    std::vector<WorkspaceEvent> m_queue;
    float  m_drainIntervalSeconds = 0.1f;  // default: drain every 100ms
    float  m_elapsed              = 0.f;
    size_t m_totalDrained         = 0;
};

// ═════════════════════════════════════════════════════════════════
// WorkspaceNotificationSeverity — notification severity levels
// ═════════════════════════════════════════════════════════════════

enum class WsNotificationSeverity : uint8_t {
    Info     = 0,
    Success  = 1,
    Warning  = 2,
    Error    = 3,
    Critical = 4,
};

inline const char* wsNotificationSeverityName(WsNotificationSeverity s) {
    switch (s) {
        case WsNotificationSeverity::Info:     return "Info";
        case WsNotificationSeverity::Success:  return "Success";
        case WsNotificationSeverity::Warning:  return "Warning";
        case WsNotificationSeverity::Error:    return "Error";
        case WsNotificationSeverity::Critical: return "Critical";
        default:                               return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// WorkspaceNotificationEntry — a single notification record
// ═════════════════════════════════════════════════════════════════

struct WorkspaceNotificationEntry {
    uint32_t                 id          = 0;
    std::string              title;
    std::string              message;
    std::string              source;
    WsNotificationSeverity   severity    = WsNotificationSeverity::Info;
    uint64_t                 timestampMs = 0;
    bool                     read        = false;

    void markRead() { read = true; }

    [[nodiscard]] bool isValid()    const { return !title.empty(); }
    [[nodiscard]] bool isError()    const { return severity >= WsNotificationSeverity::Error; }
    [[nodiscard]] bool isCritical() const { return severity == WsNotificationSeverity::Critical; }
    [[nodiscard]] bool isUnread()   const { return !read; }
};

// ═════════════════════════════════════════════════════════════════
// WorkspaceNotificationBus — higher-level notification model
//   layered on top of WorkspaceEventBus
// ═════════════════════════════════════════════════════════════════

class WorkspaceNotificationBus {
public:
    static constexpr size_t MAX_HISTORY = 256;

    explicit WorkspaceNotificationBus(WorkspaceEventBus& bus) : m_bus(bus) {}

    // Post a notification. Publishes a WorkspaceEvent of type Notification
    // and stores the entry in history.
    uint32_t notify(WsNotificationSeverity severity, const std::string& title,
                    const std::string& message, const std::string& source = {},
                    uint64_t timestampMs = 0) {
        uint32_t id = ++m_nextId;
        WorkspaceNotificationEntry entry;
        entry.id          = id;
        entry.title       = title;
        entry.message     = message;
        entry.source      = source.empty() ? "workspace" : source;
        entry.severity    = severity;
        entry.timestampMs = timestampMs;

        // Store in history (trim oldest if over capacity)
        m_history.push_back(entry);
        if (m_history.size() > MAX_HISTORY) {
            m_history.erase(m_history.begin());
        }

        // Publish event on the bus
        WorkspaceEventPriority prio = WorkspaceEventPriority::Normal;
        if (severity >= WsNotificationSeverity::Error)   prio = WorkspaceEventPriority::High;
        if (severity == WsNotificationSeverity::Critical) prio = WorkspaceEventPriority::Critical;

        WorkspaceEvent ev = WorkspaceEvent::make(
            WorkspaceEventType::Notification,
            entry.source,
            title + ": " + message,
            prio,
            timestampMs
        );
        m_bus.publish(ev);

        return id;
    }

    // Convenience helpers
    uint32_t info(const std::string& title, const std::string& msg, const std::string& src = {}) {
        return notify(WsNotificationSeverity::Info, title, msg, src);
    }
    uint32_t success(const std::string& title, const std::string& msg, const std::string& src = {}) {
        return notify(WsNotificationSeverity::Success, title, msg, src);
    }
    uint32_t warning(const std::string& title, const std::string& msg, const std::string& src = {}) {
        return notify(WsNotificationSeverity::Warning, title, msg, src);
    }
    uint32_t error(const std::string& title, const std::string& msg, const std::string& src = {}) {
        return notify(WsNotificationSeverity::Error, title, msg, src);
    }
    uint32_t critical(const std::string& title, const std::string& msg, const std::string& src = {}) {
        return notify(WsNotificationSeverity::Critical, title, msg, src);
    }

    // Mark a notification as read.
    bool markRead(uint32_t id) {
        for (auto& e : m_history) {
            if (e.id == id) { e.markRead(); return true; }
        }
        return false;
    }

    // Find notification by id.
    [[nodiscard]] const WorkspaceNotificationEntry* find(uint32_t id) const {
        for (const auto& e : m_history) {
            if (e.id == id) return &e;
        }
        return nullptr;
    }

    // Count unread notifications.
    [[nodiscard]] size_t unreadCount() const {
        size_t c = 0;
        for (const auto& e : m_history) if (e.isUnread()) ++c;
        return c;
    }

    // Count by severity.
    [[nodiscard]] size_t countBySeverity(WsNotificationSeverity sev) const {
        size_t c = 0;
        for (const auto& e : m_history) if (e.severity == sev) ++c;
        return c;
    }

    // Count error-or-higher notifications.
    [[nodiscard]] size_t errorCount() const {
        size_t c = 0;
        for (const auto& e : m_history) if (e.isError()) ++c;
        return c;
    }

    // Mark all as read.
    void markAllRead() {
        for (auto& e : m_history) e.markRead();
    }

    // Clear all history.
    void clearHistory() { m_history.clear(); }

    [[nodiscard]] size_t historySize()  const { return m_history.size(); }
    [[nodiscard]] bool   empty()        const { return m_history.empty(); }

    [[nodiscard]] const std::vector<WorkspaceNotificationEntry>& history() const {
        return m_history;
    }

private:
    WorkspaceEventBus& m_bus;
    std::vector<WorkspaceNotificationEntry> m_history;
    uint32_t m_nextId = 0;
};

} // namespace NF
