#include "EditorEventBus.h"
#include <algorithm>

namespace atlas::tools {

EditorEventBus::SubscriptionId
EditorEventBus::Subscribe(EditorEventType type, EditorEventHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    SubscriptionId id = m_nextId++;
    m_subs.push_back({id, type, std::move(handler)});
    return id;
}

void EditorEventBus::Unsubscribe(SubscriptionId id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subs.erase(
        std::remove_if(m_subs.begin(), m_subs.end(),
                        [id](const Sub& s) { return s.id == id; }),
        m_subs.end());
}

void EditorEventBus::Publish(const EditorEvent& event) {
    // Snapshot matching handlers under lock, then invoke outside the lock
    // to avoid deadlocks if a handler modifies subscriptions.
    // Note: copying std::function objects is intentional here — the editor
    // event bus is not a hot-path, and correctness (no deadlock) is more
    // important than avoiding the copy overhead.
    std::vector<EditorEventHandler> matching;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& s : m_subs) {
            if (s.type == event.type) {
                matching.push_back(s.handler);
            }
        }
    }
    for (const auto& h : matching) {
        h(event);
    }
}

size_t EditorEventBus::SubscriberCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_subs.size();
}

void EditorEventBus::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subs.clear();
    m_nextId = 1;
}

} // namespace atlas::tools
