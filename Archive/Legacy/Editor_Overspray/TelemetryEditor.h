#pragma once
// NF::Editor — telemetry editor
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

enum class TelemetryEventType : uint8_t {
    PlayerAction, SessionStart, SessionEnd, LevelLoad, LevelComplete,
    Purchase, Error, Performance, Custom
};

inline const char* telemetryEventTypeName(TelemetryEventType t) {
    switch (t) {
        case TelemetryEventType::PlayerAction:   return "PlayerAction";
        case TelemetryEventType::SessionStart:   return "SessionStart";
        case TelemetryEventType::SessionEnd:     return "SessionEnd";
        case TelemetryEventType::LevelLoad:      return "LevelLoad";
        case TelemetryEventType::LevelComplete:  return "LevelComplete";
        case TelemetryEventType::Purchase:       return "Purchase";
        case TelemetryEventType::Error:          return "Error";
        case TelemetryEventType::Performance:    return "Performance";
        case TelemetryEventType::Custom:         return "Custom";
    }
    return "Unknown";
}

enum class TelemetrySamplingRate : uint8_t {
    Every, EverySecond, EveryMinute, OnChange, Manual
};

inline const char* telemetrySamplingRateName(TelemetrySamplingRate r) {
    switch (r) {
        case TelemetrySamplingRate::Every:       return "Every";
        case TelemetrySamplingRate::EverySecond: return "EverySecond";
        case TelemetrySamplingRate::EveryMinute: return "EveryMinute";
        case TelemetrySamplingRate::OnChange:    return "OnChange";
        case TelemetrySamplingRate::Manual:      return "Manual";
    }
    return "Unknown";
}

enum class TelemetryPrivacyLevel : uint8_t {
    Anonymous, Pseudonymous, Identified, Restricted
};

inline const char* telemetryPrivacyLevelName(TelemetryPrivacyLevel p) {
    switch (p) {
        case TelemetryPrivacyLevel::Anonymous:    return "Anonymous";
        case TelemetryPrivacyLevel::Pseudonymous: return "Pseudonymous";
        case TelemetryPrivacyLevel::Identified:   return "Identified";
        case TelemetryPrivacyLevel::Restricted:   return "Restricted";
    }
    return "Unknown";
}

class TelemetryEvent {
public:
    explicit TelemetryEvent(uint32_t id, const std::string& name, TelemetryEventType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setSamplingRate(TelemetrySamplingRate v) { m_samplingRate  = v; }
    void setPrivacyLevel(TelemetryPrivacyLevel v) { m_privacyLevel  = v; }
    void setIsEnabled(bool v)                     { m_isEnabled     = v; }
    void setBufferSize(uint32_t v)                { m_bufferSize    = v; }

    [[nodiscard]] uint32_t              id()           const { return m_id;          }
    [[nodiscard]] const std::string&    name()         const { return m_name;        }
    [[nodiscard]] TelemetryEventType    type()         const { return m_type;        }
    [[nodiscard]] TelemetrySamplingRate samplingRate() const { return m_samplingRate;}
    [[nodiscard]] TelemetryPrivacyLevel privacyLevel() const { return m_privacyLevel;}
    [[nodiscard]] bool                  isEnabled()    const { return m_isEnabled;   }
    [[nodiscard]] uint32_t              bufferSize()   const { return m_bufferSize;  }

private:
    uint32_t              m_id;
    std::string           m_name;
    TelemetryEventType    m_type;
    TelemetrySamplingRate m_samplingRate = TelemetrySamplingRate::OnChange;
    TelemetryPrivacyLevel m_privacyLevel = TelemetryPrivacyLevel::Anonymous;
    bool                  m_isEnabled   = true;
    uint32_t              m_bufferSize  = 256u;
};

class TelemetryEditor {
public:
    void setShowDisabled(bool v)   { m_showDisabled  = v; }
    void setShowPrivate(bool v)    { m_showPrivate   = v; }
    void setFlushIntervalMs(uint32_t v) { m_flushIntervalMs = v; }

    bool addEvent(const TelemetryEvent& e) {
        for (auto& x : m_events) if (x.id() == e.id()) return false;
        m_events.push_back(e); return true;
    }
    bool removeEvent(uint32_t id) {
        auto it = std::find_if(m_events.begin(), m_events.end(),
            [&](const TelemetryEvent& e){ return e.id() == id; });
        if (it == m_events.end()) return false;
        m_events.erase(it); return true;
    }
    [[nodiscard]] TelemetryEvent* findEvent(uint32_t id) {
        for (auto& e : m_events) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_showDisabled;    }
    [[nodiscard]] bool     isShowPrivate()     const { return m_showPrivate;     }
    [[nodiscard]] uint32_t flushIntervalMs()   const { return m_flushIntervalMs; }
    [[nodiscard]] size_t   eventCount()        const { return m_events.size();   }

    [[nodiscard]] size_t countByType(TelemetryEventType t) const {
        size_t n = 0; for (auto& e : m_events) if (e.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByPrivacyLevel(TelemetryPrivacyLevel p) const {
        size_t n = 0; for (auto& e : m_events) if (e.privacyLevel() == p) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_events) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<TelemetryEvent> m_events;
    bool     m_showDisabled   = false;
    bool     m_showPrivate    = false;
    uint32_t m_flushIntervalMs = 5000u;
};

} // namespace NF
