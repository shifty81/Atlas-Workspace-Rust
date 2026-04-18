#pragma once
// NF::Editor — broadcast message management editor
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

enum class BroadcastType : uint8_t { System, Event, News, Maintenance, Custom };
inline const char* broadcastTypeName(BroadcastType v) {
    switch (v) {
        case BroadcastType::System:      return "System";
        case BroadcastType::Event:       return "Event";
        case BroadcastType::News:        return "News";
        case BroadcastType::Maintenance: return "Maintenance";
        case BroadcastType::Custom:      return "Custom";
    }
    return "Unknown";
}

enum class BroadcastStatus : uint8_t { Draft, Scheduled, Live, Completed, Cancelled };
inline const char* broadcastStatusName(BroadcastStatus v) {
    switch (v) {
        case BroadcastStatus::Draft:     return "Draft";
        case BroadcastStatus::Scheduled: return "Scheduled";
        case BroadcastStatus::Live:      return "Live";
        case BroadcastStatus::Completed: return "Completed";
        case BroadcastStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

class BroadcastMessage {
public:
    explicit BroadcastMessage(uint32_t id, const std::string& title, BroadcastType type)
        : m_id(id), m_title(title), m_type(type) {}

    void setStatus(BroadcastStatus v)  { m_status       = v; }
    void setAudienceSize(uint32_t v)   { m_audienceSize  = v; }
    void setIsUrgent(bool v)           { m_isUrgent     = v; }
    void setIsEnabled(bool v)          { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& title()        const { return m_title;        }
    [[nodiscard]] BroadcastType      type()         const { return m_type;         }
    [[nodiscard]] BroadcastStatus    status()       const { return m_status;       }
    [[nodiscard]] uint32_t           audienceSize() const { return m_audienceSize;  }
    [[nodiscard]] bool               isUrgent()     const { return m_isUrgent;     }
    [[nodiscard]] bool               isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t       m_id;
    std::string    m_title;
    BroadcastType  m_type;
    BroadcastStatus m_status       = BroadcastStatus::Draft;
    uint32_t        m_audienceSize  = 0u;
    bool            m_isUrgent     = false;
    bool            m_isEnabled    = true;
};

class BroadcastEditor {
public:
    void setIsShowCompleted(bool v)     { m_isShowCompleted  = v; }
    void setIsGroupByType(bool v)       { m_isGroupByType    = v; }
    void setMaxAudienceSize(uint32_t v) { m_maxAudienceSize  = v; }

    bool addMessage(const BroadcastMessage& m) {
        for (auto& x : m_messages) if (x.id() == m.id()) return false;
        m_messages.push_back(m); return true;
    }
    bool removeMessage(uint32_t id) {
        auto it = std::find_if(m_messages.begin(), m_messages.end(),
            [&](const BroadcastMessage& m){ return m.id() == id; });
        if (it == m_messages.end()) return false;
        m_messages.erase(it); return true;
    }
    [[nodiscard]] BroadcastMessage* findMessage(uint32_t id) {
        for (auto& m : m_messages) if (m.id() == id) return &m;
        return nullptr;
    }

    [[nodiscard]] bool     isShowCompleted()  const { return m_isShowCompleted;  }
    [[nodiscard]] bool     isGroupByType()    const { return m_isGroupByType;    }
    [[nodiscard]] uint32_t maxAudienceSize()  const { return m_maxAudienceSize;  }
    [[nodiscard]] size_t   messageCount()     const { return m_messages.size();  }

    [[nodiscard]] size_t countByType(BroadcastType t) const {
        size_t n = 0; for (auto& m : m_messages) if (m.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(BroadcastStatus s) const {
        size_t n = 0; for (auto& m : m_messages) if (m.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countUrgent() const {
        size_t n = 0; for (auto& m : m_messages) if (m.isUrgent()) ++n; return n;
    }

private:
    std::vector<BroadcastMessage> m_messages;
    bool     m_isShowCompleted  = false;
    bool     m_isGroupByType    = false;
    uint32_t m_maxAudienceSize  = 1000000u;
};

} // namespace NF
