#pragma once
// NF::Editor — game event bus management editor
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

enum class EventBusPriority : uint8_t { Low, Normal, High, Critical, Realtime };
inline const char* eventBusPriorityName(EventBusPriority v) {
    switch (v) {
        case EventBusPriority::Low:      return "Low";
        case EventBusPriority::Normal:   return "Normal";
        case EventBusPriority::High:     return "High";
        case EventBusPriority::Critical: return "Critical";
        case EventBusPriority::Realtime: return "Realtime";
    }
    return "Unknown";
}

enum class EventBusScope : uint8_t { Local, Scene, Global, Network };
inline const char* eventBusScopeName(EventBusScope v) {
    switch (v) {
        case EventBusScope::Local:   return "Local";
        case EventBusScope::Scene:   return "Scene";
        case EventBusScope::Global:  return "Global";
        case EventBusScope::Network: return "Network";
    }
    return "Unknown";
}

class EventBusChannel {
public:
    explicit EventBusChannel(uint32_t id, const std::string& name,
                              EventBusPriority priority, EventBusScope scope)
        : m_id(id), m_name(name), m_priority(priority), m_scope(scope) {}

    void setMaxListeners(uint32_t v) { m_maxListeners = v; }
    void setIsBuffered(bool v)       { m_isBuffered   = v; }
    void setIsEnabled(bool v)        { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] EventBusPriority   priority()     const { return m_priority;     }
    [[nodiscard]] EventBusScope      scope()        const { return m_scope;        }
    [[nodiscard]] uint32_t           maxListeners() const { return m_maxListeners; }
    [[nodiscard]] bool               isBuffered()   const { return m_isBuffered;   }
    [[nodiscard]] bool               isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t        m_id;
    std::string     m_name;
    EventBusPriority m_priority;
    EventBusScope   m_scope;
    uint32_t        m_maxListeners = 64u;
    bool            m_isBuffered   = false;
    bool            m_isEnabled    = true;
};

class EventBusEditor {
public:
    void setIsShowDisabled(bool v)         { m_isShowDisabled      = v; }
    void setIsGroupByScope(bool v)         { m_isGroupByScope      = v; }
    void setDefaultMaxListeners(uint32_t v){ m_defaultMaxListeners = v; }

    bool addChannel(const EventBusChannel& c) {
        for (auto& x : m_channels) if (x.id() == c.id()) return false;
        m_channels.push_back(c); return true;
    }
    bool removeChannel(uint32_t id) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
            [&](const EventBusChannel& c){ return c.id() == id; });
        if (it == m_channels.end()) return false;
        m_channels.erase(it); return true;
    }
    [[nodiscard]] EventBusChannel* findChannel(uint32_t id) {
        for (auto& c : m_channels) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()       const { return m_isShowDisabled;      }
    [[nodiscard]] bool     isGroupByScope()        const { return m_isGroupByScope;      }
    [[nodiscard]] uint32_t defaultMaxListeners()  const { return m_defaultMaxListeners; }
    [[nodiscard]] size_t   channelCount()         const { return m_channels.size();     }

    [[nodiscard]] size_t countByPriority(EventBusPriority p) const {
        size_t n = 0; for (auto& c : m_channels) if (c.priority() == p) ++n; return n;
    }
    [[nodiscard]] size_t countByScope(EventBusScope s) const {
        size_t n = 0; for (auto& c : m_channels) if (c.scope() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& c : m_channels) if (c.isEnabled()) ++n; return n;
    }

private:
    std::vector<EventBusChannel> m_channels;
    bool     m_isShowDisabled      = false;
    bool     m_isGroupByScope      = false;
    uint32_t m_defaultMaxListeners = 32u;
};

} // namespace NF
