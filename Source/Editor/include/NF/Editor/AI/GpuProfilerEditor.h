#pragma once
// NF::Editor — GPU pass/marker profiler configuration editor
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

enum class GpuProfPass : uint8_t { Shadow, Depth, GBuffer, Lighting, PostProcess, UI };
inline const char* gpuProfPassName(GpuProfPass v) {
    switch (v) {
        case GpuProfPass::Shadow:      return "Shadow";
        case GpuProfPass::Depth:       return "Depth";
        case GpuProfPass::GBuffer:     return "GBuffer";
        case GpuProfPass::Lighting:    return "Lighting";
        case GpuProfPass::PostProcess: return "PostProcess";
        case GpuProfPass::UI:          return "UI";
    }
    return "Unknown";
}

enum class GpuProfCounter : uint8_t { DrawCalls, Triangles, Pixels, ComputeDispatches, MemBandwidth };
inline const char* gpuProfCounterName(GpuProfCounter v) {
    switch (v) {
        case GpuProfCounter::DrawCalls:          return "DrawCalls";
        case GpuProfCounter::Triangles:          return "Triangles";
        case GpuProfCounter::Pixels:             return "Pixels";
        case GpuProfCounter::ComputeDispatches:  return "ComputeDispatches";
        case GpuProfCounter::MemBandwidth:       return "MemBandwidth";
    }
    return "Unknown";
}

class GpuProfilerConfig {
public:
    explicit GpuProfilerConfig(uint32_t id, const std::string& name,
                                GpuProfPass pass, GpuProfCounter counter)
        : m_id(id), m_name(name), m_pass(pass), m_counter(counter) {}

    void setMaxMarkerDepth(uint32_t v) { m_maxMarkerDepth  = v; }
    void setIncludeCompute(bool v)     { m_includeCompute  = v; }
    void setIsEnabled(bool v)          { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] GpuProfPass        pass()           const { return m_pass;           }
    [[nodiscard]] GpuProfCounter     counter()        const { return m_counter;        }
    [[nodiscard]] uint32_t           maxMarkerDepth() const { return m_maxMarkerDepth; }
    [[nodiscard]] bool               includeCompute() const { return m_includeCompute; }
    [[nodiscard]] bool               isEnabled()      const { return m_isEnabled;      }

private:
    uint32_t      m_id;
    std::string   m_name;
    GpuProfPass   m_pass;
    GpuProfCounter m_counter;
    uint32_t      m_maxMarkerDepth = 32u;
    bool          m_includeCompute = true;
    bool          m_isEnabled      = true;
};

class GpuProfilerEditor {
public:
    void setIsShowDisabled(bool v)          { m_isShowDisabled       = v; }
    void setIsGroupByPass(bool v)           { m_isGroupByPass        = v; }
    void setDefaultMaxMarkerDepth(uint32_t v){ m_defaultMaxMarkerDepth = v; }

    bool addConfig(const GpuProfilerConfig& c) {
        for (auto& x : m_configs) if (x.id() == c.id()) return false;
        m_configs.push_back(c); return true;
    }
    bool removeConfig(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const GpuProfilerConfig& c){ return c.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] GpuProfilerConfig* findConfig(uint32_t id) {
        for (auto& c : m_configs) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()        const { return m_isShowDisabled;        }
    [[nodiscard]] bool     isGroupByPass()          const { return m_isGroupByPass;         }
    [[nodiscard]] uint32_t defaultMaxMarkerDepth()  const { return m_defaultMaxMarkerDepth; }
    [[nodiscard]] size_t   configCount()            const { return m_configs.size();        }

    [[nodiscard]] size_t countByPass(GpuProfPass p) const {
        size_t n = 0; for (auto& c : m_configs) if (c.pass() == p) ++n; return n;
    }
    [[nodiscard]] size_t countByCounter(GpuProfCounter co) const {
        size_t n = 0; for (auto& c : m_configs) if (c.counter() == co) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& c : m_configs) if (c.isEnabled()) ++n; return n;
    }

private:
    std::vector<GpuProfilerConfig> m_configs;
    bool     m_isShowDisabled        = false;
    bool     m_isGroupByPass         = true;
    uint32_t m_defaultMaxMarkerDepth = 16u;
};

} // namespace NF
