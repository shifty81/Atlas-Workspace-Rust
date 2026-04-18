#pragma once
// NF::Editor — CPU thread/scope profiler configuration editor
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

enum class CpuProfScope : uint8_t { Function, Block, Thread, Frame, Custom };
inline const char* cpuProfScopeName(CpuProfScope v) {
    switch (v) {
        case CpuProfScope::Function: return "Function";
        case CpuProfScope::Block:    return "Block";
        case CpuProfScope::Thread:   return "Thread";
        case CpuProfScope::Frame:    return "Frame";
        case CpuProfScope::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class CpuProfSampler : uint8_t { Instrumented, Statistical, Hardware, Hybrid };
inline const char* cpuProfSamplerName(CpuProfSampler v) {
    switch (v) {
        case CpuProfSampler::Instrumented: return "Instrumented";
        case CpuProfSampler::Statistical:  return "Statistical";
        case CpuProfSampler::Hardware:     return "Hardware";
        case CpuProfSampler::Hybrid:       return "Hybrid";
    }
    return "Unknown";
}

class CpuProfilerConfig {
public:
    explicit CpuProfilerConfig(uint32_t id, const std::string& name,
                                CpuProfScope scope, CpuProfSampler sampler)
        : m_id(id), m_name(name), m_scope(scope), m_sampler(sampler) {}

    void setMaxCallDepth(uint32_t v) { m_maxCallDepth  = v; }
    void setIncludeKernel(bool v)    { m_includeKernel = v; }
    void setIsEnabled(bool v)        { m_isEnabled     = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;            }
    [[nodiscard]] const std::string& name()          const { return m_name;          }
    [[nodiscard]] CpuProfScope       scope()         const { return m_scope;         }
    [[nodiscard]] CpuProfSampler     sampler()       const { return m_sampler;       }
    [[nodiscard]] uint32_t           maxCallDepth()  const { return m_maxCallDepth;  }
    [[nodiscard]] bool               includeKernel() const { return m_includeKernel; }
    [[nodiscard]] bool               isEnabled()     const { return m_isEnabled;     }

private:
    uint32_t      m_id;
    std::string   m_name;
    CpuProfScope  m_scope;
    CpuProfSampler m_sampler;
    uint32_t      m_maxCallDepth  = 64u;
    bool          m_includeKernel = false;
    bool          m_isEnabled     = true;
};

class CpuProfilerEditor {
public:
    void setIsShowDisabled(bool v)        { m_isShowDisabled      = v; }
    void setIsGroupByScope(bool v)        { m_isGroupByScope      = v; }
    void setDefaultMaxCallDepth(uint32_t v){ m_defaultMaxCallDepth = v; }

    bool addConfig(const CpuProfilerConfig& c) {
        for (auto& x : m_configs) if (x.id() == c.id()) return false;
        m_configs.push_back(c); return true;
    }
    bool removeConfig(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const CpuProfilerConfig& c){ return c.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] CpuProfilerConfig* findConfig(uint32_t id) {
        for (auto& c : m_configs) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()       const { return m_isShowDisabled;       }
    [[nodiscard]] bool     isGroupByScope()        const { return m_isGroupByScope;       }
    [[nodiscard]] uint32_t defaultMaxCallDepth()   const { return m_defaultMaxCallDepth;  }
    [[nodiscard]] size_t   configCount()           const { return m_configs.size();       }

    [[nodiscard]] size_t countByScope(CpuProfScope s) const {
        size_t n = 0; for (auto& c : m_configs) if (c.scope() == s) ++n; return n;
    }
    [[nodiscard]] size_t countBySampler(CpuProfSampler s) const {
        size_t n = 0; for (auto& c : m_configs) if (c.sampler() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& c : m_configs) if (c.isEnabled()) ++n; return n;
    }

private:
    std::vector<CpuProfilerConfig> m_configs;
    bool     m_isShowDisabled      = false;
    bool     m_isGroupByScope      = true;
    uint32_t m_defaultMaxCallDepth = 32u;
};

} // namespace NF
