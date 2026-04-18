#pragma once
// NF::Workspace — Editor event bus
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class EditorEventPriority : uint8_t {
    Lowest  = 0,
    Low     = 1,
    Normal  = 2,
    High    = 3,
    Highest = 4,
    System  = 5,
    Critical = 6,
    Realtime = 7,
};

inline const char* editorEventPriorityName(EditorEventPriority p) {
    switch (p) {
        case EditorEventPriority::Lowest:   return "Lowest";
        case EditorEventPriority::Low:      return "Low";
        case EditorEventPriority::Normal:   return "Normal";
        case EditorEventPriority::High:     return "High";
        case EditorEventPriority::Highest:  return "Highest";
        case EditorEventPriority::System:   return "System";
        case EditorEventPriority::Critical: return "Critical";
        case EditorEventPriority::Realtime: return "Realtime";
        default:                            return "Unknown";
    }
}

enum class EditorBusState : uint8_t {
    Idle      = 0,
    Posting   = 1,
    Flushing  = 2,
    Suspended = 3,
};

struct EditorBusEvent {
    std::string          topic;
    std::string          payload;
    EditorEventPriority  priority  = EditorEventPriority::Normal;
    uint64_t             timestamp = 0;
    bool                 consumed  = false;

    void consume() { consumed = true; }
    [[nodiscard]] bool isConsumed() const { return consumed; }
    [[nodiscard]] bool isHighPrio() const { return priority >= EditorEventPriority::High; }
    [[nodiscard]] bool isCritical() const { return priority >= EditorEventPriority::Critical; }
};

class EditorEventSubscription {
public:
    using Handler = std::function<void(const EditorBusEvent&)>;

    EditorEventSubscription(std::string topic, EditorEventPriority minPrio, Handler handler)
        : m_topic(std::move(topic)), m_minPriority(minPrio), m_handler(std::move(handler)) {}

    [[nodiscard]] const std::string& topic()      const { return m_topic;       }
    [[nodiscard]] EditorEventPriority minPriority() const { return m_minPriority; }
    [[nodiscard]] size_t             callCount()  const { return m_callCount;   }
    [[nodiscard]] bool               isActive()   const { return m_active;      }

    void deliver(const EditorBusEvent& ev) {
        if (!m_active) return;
        if (ev.priority < m_minPriority) return;
        if (m_handler) { m_handler(ev); ++m_callCount; }
    }

    void cancel() { m_active = false; }

private:
    std::string          m_topic;
    EditorEventPriority  m_minPriority;
    Handler              m_handler;
    size_t               m_callCount = 0;
    bool                 m_active    = true;
};

class EditorEventBus {
public:
    static constexpr size_t MAX_SUBSCRIPTIONS = 256;
    static constexpr size_t MAX_QUEUE         = 512;

    EditorEventSubscription* subscribe(const std::string& topic, EditorEventPriority minPrio,
                                        EditorEventSubscription::Handler handler) {
        if (m_subscriptions.size() >= MAX_SUBSCRIPTIONS) return nullptr;
        m_subscriptions.emplace_back(topic, minPrio, std::move(handler));
        return &m_subscriptions.back();
    }

    bool post(const EditorBusEvent& ev) {
        if (m_state == EditorBusState::Suspended) return false;
        if (m_queue.size() >= MAX_QUEUE) return false;
        m_queue.push_back(ev);
        return true;
    }

    size_t flush() {
        if (m_state == EditorBusState::Suspended) return 0;
        m_state = EditorBusState::Flushing;
        size_t dispatched = 0;
        for (auto& ev : m_queue) {
            for (auto& sub : m_subscriptions) {
                if (sub.topic() == ev.topic || sub.topic() == "*") {
                    sub.deliver(ev);
                    ++dispatched;
                }
            }
        }
        m_queue.clear();
        m_state = EditorBusState::Idle;
        return dispatched;
    }

    void suspend()    { m_state = EditorBusState::Suspended; }
    void resume()     { if (m_state == EditorBusState::Suspended) m_state = EditorBusState::Idle; }
    void clearQueue() { m_queue.clear(); }

    [[nodiscard]] EditorBusState state()             const { return m_state;                 }
    [[nodiscard]] size_t         queueSize()          const { return m_queue.size();          }
    [[nodiscard]] size_t         subscriptionCount()  const { return m_subscriptions.size(); }
    [[nodiscard]] bool           isSuspended()        const { return m_state == EditorBusState::Suspended; }

private:
    std::vector<EditorEventSubscription> m_subscriptions;
    std::vector<EditorBusEvent>          m_queue;
    EditorBusState                       m_state = EditorBusState::Idle;
};

// ============================================================
// S22 — Workspace Layout Manager
// ============================================================


} // namespace NF
