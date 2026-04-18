#pragma once
// NF::Editor — async message queue management editor
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

enum class MsgQueuePolicy : uint8_t { FIFO, LIFO, Priority, RoundRobin, Custom };
inline const char* msgQueuePolicyName(MsgQueuePolicy v) {
    switch (v) {
        case MsgQueuePolicy::FIFO:       return "FIFO";
        case MsgQueuePolicy::LIFO:       return "LIFO";
        case MsgQueuePolicy::Priority:   return "Priority";
        case MsgQueuePolicy::RoundRobin: return "RoundRobin";
        case MsgQueuePolicy::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class MsgQueueState : uint8_t { Idle, Active, Paused, Draining, Error };
inline const char* msgQueueStateName(MsgQueueState v) {
    switch (v) {
        case MsgQueueState::Idle:     return "Idle";
        case MsgQueueState::Active:   return "Active";
        case MsgQueueState::Paused:   return "Paused";
        case MsgQueueState::Draining: return "Draining";
        case MsgQueueState::Error:    return "Error";
    }
    return "Unknown";
}

class MessageQueue {
public:
    explicit MessageQueue(uint32_t id, const std::string& name, MsgQueuePolicy policy)
        : m_id(id), m_name(name), m_policy(policy) {}

    void setState(MsgQueueState v)  { m_state     = v; }
    void setMaxDepth(uint32_t v)    { m_maxDepth   = v; }
    void setIsDeduped(bool v)       { m_isDeduped  = v; }
    void setIsEnabled(bool v)       { m_isEnabled  = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] MsgQueuePolicy     policy()    const { return m_policy;    }
    [[nodiscard]] MsgQueueState      state()     const { return m_state;     }
    [[nodiscard]] uint32_t           maxDepth()  const { return m_maxDepth;  }
    [[nodiscard]] bool               isDeduped() const { return m_isDeduped; }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t      m_id;
    std::string   m_name;
    MsgQueuePolicy m_policy;
    MsgQueueState m_state     = MsgQueueState::Idle;
    uint32_t      m_maxDepth  = 256u;
    bool          m_isDeduped = false;
    bool          m_isEnabled = true;
};

class MessageQueueEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled   = v; }
    void setIsGroupByPolicy(bool v)     { m_isGroupByPolicy  = v; }
    void setDefaultMaxDepth(uint32_t v) { m_defaultMaxDepth  = v; }

    bool addQueue(const MessageQueue& q) {
        for (auto& x : m_queues) if (x.id() == q.id()) return false;
        m_queues.push_back(q); return true;
    }
    bool removeQueue(uint32_t id) {
        auto it = std::find_if(m_queues.begin(), m_queues.end(),
            [&](const MessageQueue& q){ return q.id() == id; });
        if (it == m_queues.end()) return false;
        m_queues.erase(it); return true;
    }
    [[nodiscard]] MessageQueue* findQueue(uint32_t id) {
        for (auto& q : m_queues) if (q.id() == id) return &q;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()   const { return m_isShowDisabled;  }
    [[nodiscard]] bool     isGroupByPolicy()  const { return m_isGroupByPolicy; }
    [[nodiscard]] uint32_t defaultMaxDepth()  const { return m_defaultMaxDepth; }
    [[nodiscard]] size_t   queueCount()       const { return m_queues.size();   }

    [[nodiscard]] size_t countByPolicy(MsgQueuePolicy p) const {
        size_t n = 0; for (auto& q : m_queues) if (q.policy() == p) ++n; return n;
    }
    [[nodiscard]] size_t countByState(MsgQueueState s) const {
        size_t n = 0; for (auto& q : m_queues) if (q.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& q : m_queues) if (q.isEnabled()) ++n; return n;
    }

private:
    std::vector<MessageQueue> m_queues;
    bool     m_isShowDisabled  = false;
    bool     m_isGroupByPolicy = false;
    uint32_t m_defaultMaxDepth = 128u;
};

} // namespace NF
