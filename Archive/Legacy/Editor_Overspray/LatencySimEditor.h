#pragma once
// NF::Editor — network latency simulation preset management
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
#include <string>
#include <vector>
#include <cstdint>

namespace NF {

enum class LatSimCondition : uint8_t { Perfect, Good, Average, Poor, Terrible };
inline const char* latSimConditionName(LatSimCondition v) {
    switch (v) {
        case LatSimCondition::Perfect:  return "Perfect";
        case LatSimCondition::Good:     return "Good";
        case LatSimCondition::Average:  return "Average";
        case LatSimCondition::Poor:     return "Poor";
        case LatSimCondition::Terrible: return "Terrible";
    }
    return "Unknown";
}

enum class LatSimJitter : uint8_t { None, Low, Medium, High, Spike };
inline const char* latSimJitterName(LatSimJitter v) {
    switch (v) {
        case LatSimJitter::None:   return "None";
        case LatSimJitter::Low:    return "Low";
        case LatSimJitter::Medium: return "Medium";
        case LatSimJitter::High:   return "High";
        case LatSimJitter::Spike:  return "Spike";
    }
    return "Unknown";
}

class LatencySimPreset {
public:
    explicit LatencySimPreset(uint32_t id, const std::string& name,
                               LatSimCondition condition, LatSimJitter jitter)
        : m_id(id), m_name(name), m_condition(condition), m_jitter(jitter) {}

    void setLatencyMs(uint32_t v)      { m_latencyMs      = v; }
    void setPacketLossPct(float v)     { m_packetLossPct  = v; }
    void setIsEnabled(bool v)          { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;           }
    [[nodiscard]] const std::string& name()          const { return m_name;         }
    [[nodiscard]] LatSimCondition    condition()     const { return m_condition;    }
    [[nodiscard]] LatSimJitter       jitter()        const { return m_jitter;       }
    [[nodiscard]] uint32_t           latencyMs()     const { return m_latencyMs;    }
    [[nodiscard]] float              packetLossPct() const { return m_packetLossPct; }
    [[nodiscard]] bool               isEnabled()     const { return m_isEnabled;    }

private:
    uint32_t        m_id;
    std::string     m_name;
    LatSimCondition m_condition;
    LatSimJitter    m_jitter;
    uint32_t        m_latencyMs     = 50u;
    float           m_packetLossPct = 0.0f;
    bool            m_isEnabled     = true;
};

class LatencySimEditor {
public:
    void setIsShowDisabled(bool v)       { m_isShowDisabled      = v; }
    void setIsGroupByCondition(bool v)   { m_isGroupByCondition  = v; }
    void setDefaultLatencyMs(uint32_t v) { m_defaultLatencyMs    = v; }

    bool addPreset(const LatencySimPreset& p) {
        for (auto& x : m_presets) if (x.id() == p.id()) return false;
        m_presets.push_back(p); return true;
    }
    bool removePreset(uint32_t id) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
            [&](const LatencySimPreset& p){ return p.id() == id; });
        if (it == m_presets.end()) return false;
        m_presets.erase(it); return true;
    }
    [[nodiscard]] LatencySimPreset* findPreset(uint32_t id) {
        for (auto& p : m_presets) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool     isGroupByCondition() const { return m_isGroupByCondition; }
    [[nodiscard]] uint32_t defaultLatencyMs()  const { return m_defaultLatencyMs;  }
    [[nodiscard]] size_t   presetCount()       const { return m_presets.size();    }

    [[nodiscard]] size_t countByCondition(LatSimCondition c) const {
        size_t n = 0; for (auto& x : m_presets) if (x.condition() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByJitter(LatSimJitter j) const {
        size_t n = 0; for (auto& x : m_presets) if (x.jitter() == j) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_presets) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<LatencySimPreset> m_presets;
    bool     m_isShowDisabled     = false;
    bool     m_isGroupByCondition = false;
    uint32_t m_defaultLatencyMs   = 100u;
};

} // namespace NF
