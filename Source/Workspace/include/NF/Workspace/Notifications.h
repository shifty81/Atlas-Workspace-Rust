#pragma once
// NF::Editor — Notification queue
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

enum class NotificationType : uint8_t { Info, Success, Warning, Error };

struct EditorNotification {
    NotificationType type = NotificationType::Info;
    std::string message;
    float ttl = 3.f;      // seconds before it expires
    float elapsed = 0.f;

    bool isExpired() const { return elapsed >= ttl; }
    float progress() const { return ttl > 0.f ? std::min(elapsed / ttl, 1.f) : 1.f; }
};

class NotificationQueue {
public:
    void push(NotificationType type, const std::string& message, float ttl = 3.f) {
        EditorNotification n;
        n.type = type;
        n.message = message;
        n.ttl = ttl;
        n.elapsed = 0.f;
        m_queue.push_back(std::move(n));
    }

    void tick(float dt) {
        for (auto& n : m_queue) n.elapsed += dt;
        m_queue.erase(
            std::remove_if(m_queue.begin(), m_queue.end(),
                           [](const EditorNotification& n){ return n.isExpired(); }),
            m_queue.end());
    }

    const EditorNotification* current() const {
        return m_queue.empty() ? nullptr : &m_queue.front();
    }

    bool hasActive() const { return !m_queue.empty(); }
    int count() const { return (int)m_queue.size(); }
    void clear() { m_queue.clear(); }

private:
    std::vector<EditorNotification> m_queue;
};

// ── Orbital Editor Camera ─────────────────────────────────────────


} // namespace NF
