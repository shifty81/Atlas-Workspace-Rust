#pragma once
// NF::Editor — Editor-side diagnostic profiler panel
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

enum class DiagSamplerType : uint8_t {
    CPU, GPU, Memory, IO, Network, Script, Physics, Render
};

inline const char* diagSamplerTypeName(DiagSamplerType t) {
    switch (t) {
        case DiagSamplerType::CPU:     return "CPU";
        case DiagSamplerType::GPU:     return "GPU";
        case DiagSamplerType::Memory:  return "Memory";
        case DiagSamplerType::IO:      return "IO";
        case DiagSamplerType::Network: return "Network";
        case DiagSamplerType::Script:  return "Script";
        case DiagSamplerType::Physics: return "Physics";
        case DiagSamplerType::Render:  return "Render";
    }
    return "Unknown";
}

enum class DiagProfilerState : uint8_t {
    Idle, Sampling, Paused, Stopped, Exporting
};

inline const char* diagProfilerStateName(DiagProfilerState s) {
    switch (s) {
        case DiagProfilerState::Idle:      return "Idle";
        case DiagProfilerState::Sampling:  return "Sampling";
        case DiagProfilerState::Paused:    return "Paused";
        case DiagProfilerState::Stopped:   return "Stopped";
        case DiagProfilerState::Exporting: return "Exporting";
    }
    return "Unknown";
}

enum class DiagAlertLevel : uint8_t {
    None, Info, Warning, Critical
};

inline const char* diagAlertLevelName(DiagAlertLevel l) {
    switch (l) {
        case DiagAlertLevel::None:     return "None";
        case DiagAlertLevel::Info:     return "Info";
        case DiagAlertLevel::Warning:  return "Warning";
        case DiagAlertLevel::Critical: return "Critical";
    }
    return "Unknown";
}

class DiagSampler {
public:
    explicit DiagSampler(const std::string& name, DiagSamplerType type)
        : m_name(name), m_type(type) {}

    void setEnabled(bool v)               { m_enabled   = v; }
    void setSampleRateHz(float r)         { m_sampleRate = r; }
    void setAlertLevel(DiagAlertLevel l)  { m_alertLevel = l; }
    void setThreshold(float t)            { m_threshold  = t; }

    [[nodiscard]] const std::string& name()         const { return m_name;       }
    [[nodiscard]] DiagSamplerType    type()         const { return m_type;       }
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;    }
    [[nodiscard]] float              sampleRateHz() const { return m_sampleRate; }
    [[nodiscard]] DiagAlertLevel     alertLevel()   const { return m_alertLevel; }
    [[nodiscard]] float              threshold()    const { return m_threshold;  }

private:
    std::string    m_name;
    DiagSamplerType m_type;
    DiagAlertLevel m_alertLevel = DiagAlertLevel::None;
    float          m_sampleRate = 60.0f;
    float          m_threshold  = 0.0f;
    bool           m_enabled    = true;
};

class DiagnosticProfiler {
public:
    static constexpr size_t MAX_SAMPLERS = 64;

    [[nodiscard]] bool addSampler(const DiagSampler& sampler) {
        for (auto& s : m_samplers) if (s.name() == sampler.name()) return false;
        if (m_samplers.size() >= MAX_SAMPLERS) return false;
        m_samplers.push_back(sampler);
        return true;
    }

    [[nodiscard]] bool removeSampler(const std::string& name) {
        for (auto it = m_samplers.begin(); it != m_samplers.end(); ++it) {
            if (it->name() == name) { m_samplers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DiagSampler* findSampler(const std::string& name) {
        for (auto& s : m_samplers) if (s.name() == name) return &s;
        return nullptr;
    }

    void setState(DiagProfilerState s)          { m_state     = s; }
    void setMaxSampleMs(float ms)               { m_maxSampleMs = ms; }

    [[nodiscard]] DiagProfilerState state()        const { return m_state;       }
    [[nodiscard]] float             maxSampleMs()  const { return m_maxSampleMs; }
    [[nodiscard]] size_t            samplerCount() const { return m_samplers.size(); }

    [[nodiscard]] bool isSampling() const { return m_state == DiagProfilerState::Sampling; }

    [[nodiscard]] size_t enabledSamplerCount() const {
        size_t c = 0; for (auto& s : m_samplers) if (s.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(DiagSamplerType t) const {
        size_t c = 0; for (auto& s : m_samplers) if (s.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByAlertLevel(DiagAlertLevel l) const {
        size_t c = 0; for (auto& s : m_samplers) if (s.alertLevel() == l) ++c; return c;
    }

private:
    std::vector<DiagSampler> m_samplers;
    DiagProfilerState        m_state      = DiagProfilerState::Idle;
    float                    m_maxSampleMs = 16.67f;
};

} // namespace NF
