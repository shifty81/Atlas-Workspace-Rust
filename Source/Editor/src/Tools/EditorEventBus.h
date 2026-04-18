#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace atlas::tools {

/// Identifies a category of editor event.
enum class EditorEventType : uint8_t {
    ToolActivated,
    ToolDeactivated,
    SelectionChanged,
    EntityCreated,
    EntityDeleted,
    PropertyChanged,
    BookmarkSaved,
    BookmarkRestored,
    LayerVisibilityChanged,
    UndoPerformed,
    RedoPerformed,
    Custom
};

/// Payload for an editor event.
struct EditorEvent {
    EditorEventType type = EditorEventType::Custom;
    uint32_t entityId = 0;
    std::string key;
    std::string value;
};

using EditorEventHandler = std::function<void(const EditorEvent&)>;

/// Publish / subscribe event bus for editor-wide notifications.
///
/// Thread-safe.  Subscribers register via Subscribe() and receive events
/// synchronously when Publish() is called (on the caller's thread).
class EditorEventBus {
public:
    using SubscriptionId = uint32_t;

    /// Subscribe to events of a given type.  Returns an id that can be
    /// used to Unsubscribe later.
    SubscriptionId Subscribe(EditorEventType type, EditorEventHandler handler);

    /// Remove a subscription.
    void Unsubscribe(SubscriptionId id);

    /// Publish an event — all matching subscribers are called synchronously.
    void Publish(const EditorEvent& event);

    /// Number of active subscriptions.
    size_t SubscriberCount() const;

    /// Remove all subscriptions.
    void Clear();

private:
    struct Sub {
        SubscriptionId id;
        EditorEventType type;
        EditorEventHandler handler;
    };

    mutable std::mutex m_mutex;
    std::vector<Sub> m_subs;
    SubscriptionId m_nextId = 1;
};

} // namespace atlas::tools
