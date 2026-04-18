#pragma once
// NF::Editor — publish-subscribe topic management editor
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

enum class PubSubDelivery : uint8_t { AtMostOnce, AtLeastOnce, ExactlyOnce, BestEffort };
inline const char* pubSubDeliveryName(PubSubDelivery v) {
    switch (v) {
        case PubSubDelivery::AtMostOnce:  return "AtMostOnce";
        case PubSubDelivery::AtLeastOnce: return "AtLeastOnce";
        case PubSubDelivery::ExactlyOnce: return "ExactlyOnce";
        case PubSubDelivery::BestEffort:  return "BestEffort";
    }
    return "Unknown";
}

enum class PubSubRetention : uint8_t { None, Session, Persistent, Infinite };
inline const char* pubSubRetentionName(PubSubRetention v) {
    switch (v) {
        case PubSubRetention::None:       return "None";
        case PubSubRetention::Session:    return "Session";
        case PubSubRetention::Persistent: return "Persistent";
        case PubSubRetention::Infinite:   return "Infinite";
    }
    return "Unknown";
}

class PubSubTopic {
public:
    explicit PubSubTopic(uint32_t id, const std::string& name,
                          PubSubDelivery delivery, PubSubRetention retention)
        : m_id(id), m_name(name), m_delivery(delivery), m_retention(retention) {}

    void setMaxSubscribers(uint32_t v) { m_maxSubscribers = v; }
    void setIsCached(bool v)           { m_isCached        = v; }
    void setIsEnabled(bool v)          { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] PubSubDelivery     delivery()       const { return m_delivery;       }
    [[nodiscard]] PubSubRetention    retention()      const { return m_retention;      }
    [[nodiscard]] uint32_t           maxSubscribers() const { return m_maxSubscribers; }
    [[nodiscard]] bool               isCached()       const { return m_isCached;       }
    [[nodiscard]] bool               isEnabled()      const { return m_isEnabled;      }

private:
    uint32_t       m_id;
    std::string    m_name;
    PubSubDelivery  m_delivery;
    PubSubRetention m_retention;
    uint32_t       m_maxSubscribers = 128u;
    bool           m_isCached       = false;
    bool           m_isEnabled      = true;
};

class PubSubEditor {
public:
    void setIsShowDisabled(bool v)          { m_isShowDisabled       = v; }
    void setIsGroupByDelivery(bool v)       { m_isGroupByDelivery    = v; }
    void setDefaultMaxSubscribers(uint32_t v){ m_defaultMaxSubscribers = v; }

    bool addTopic(const PubSubTopic& t) {
        for (auto& x : m_topics) if (x.id() == t.id()) return false;
        m_topics.push_back(t); return true;
    }
    bool removeTopic(uint32_t id) {
        auto it = std::find_if(m_topics.begin(), m_topics.end(),
            [&](const PubSubTopic& t){ return t.id() == id; });
        if (it == m_topics.end()) return false;
        m_topics.erase(it); return true;
    }
    [[nodiscard]] PubSubTopic* findTopic(uint32_t id) {
        for (auto& t : m_topics) if (t.id() == id) return &t;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()        const { return m_isShowDisabled;        }
    [[nodiscard]] bool     isGroupByDelivery()     const { return m_isGroupByDelivery;     }
    [[nodiscard]] uint32_t defaultMaxSubscribers() const { return m_defaultMaxSubscribers; }
    [[nodiscard]] size_t   topicCount()            const { return m_topics.size();         }

    [[nodiscard]] size_t countByDelivery(PubSubDelivery d) const {
        size_t n = 0; for (auto& t : m_topics) if (t.delivery() == d) ++n; return n;
    }
    [[nodiscard]] size_t countByRetention(PubSubRetention r) const {
        size_t n = 0; for (auto& t : m_topics) if (t.retention() == r) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& t : m_topics) if (t.isEnabled()) ++n; return n;
    }

private:
    std::vector<PubSubTopic> m_topics;
    bool     m_isShowDisabled        = false;
    bool     m_isGroupByDelivery     = false;
    uint32_t m_defaultMaxSubscribers = 64u;
};

} // namespace NF
