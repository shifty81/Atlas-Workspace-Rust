#pragma once
// NF::Editor — benchmark suite editor
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

enum class BenchmarkCategory : uint8_t {
    CPU, GPU, Memory, IO, Network, Physics, Rendering, Audio, Scripting, Custom
};

inline const char* benchmarkCategoryName(BenchmarkCategory c) {
    switch (c) {
        case BenchmarkCategory::CPU:       return "CPU";
        case BenchmarkCategory::GPU:       return "GPU";
        case BenchmarkCategory::Memory:    return "Memory";
        case BenchmarkCategory::IO:        return "IO";
        case BenchmarkCategory::Network:   return "Network";
        case BenchmarkCategory::Physics:   return "Physics";
        case BenchmarkCategory::Rendering: return "Rendering";
        case BenchmarkCategory::Audio:     return "Audio";
        case BenchmarkCategory::Scripting: return "Scripting";
        case BenchmarkCategory::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class BenchmarkRunState : uint8_t {
    Pending, Running, Completed, Failed, Skipped, Baseline
};

inline const char* benchmarkRunStateName(BenchmarkRunState s) {
    switch (s) {
        case BenchmarkRunState::Pending:   return "Pending";
        case BenchmarkRunState::Running:   return "Running";
        case BenchmarkRunState::Completed: return "Completed";
        case BenchmarkRunState::Failed:    return "Failed";
        case BenchmarkRunState::Skipped:   return "Skipped";
        case BenchmarkRunState::Baseline:  return "Baseline";
    }
    return "Unknown";
}

enum class BenchmarkMetric : uint8_t {
    FPS, FrameTimeMs, MemoryMB, CPUPercent, GPUPercent, DrawCalls, Triangles, Latency
};

inline const char* benchmarkMetricName(BenchmarkMetric m) {
    switch (m) {
        case BenchmarkMetric::FPS:         return "FPS";
        case BenchmarkMetric::FrameTimeMs: return "FrameTimeMs";
        case BenchmarkMetric::MemoryMB:    return "MemoryMB";
        case BenchmarkMetric::CPUPercent:  return "CPUPercent";
        case BenchmarkMetric::GPUPercent:  return "GPUPercent";
        case BenchmarkMetric::DrawCalls:   return "DrawCalls";
        case BenchmarkMetric::Triangles:   return "Triangles";
        case BenchmarkMetric::Latency:     return "Latency";
    }
    return "Unknown";
}

class BenchmarkCase {
public:
    explicit BenchmarkCase(uint32_t id, const std::string& name, BenchmarkCategory category)
        : m_id(id), m_name(name), m_category(category) {}

    void setState(BenchmarkRunState v)    { m_state        = v; }
    void setPrimaryMetric(BenchmarkMetric v) { m_primaryMetric = v; }
    void setIsEnabled(bool v)             { m_isEnabled    = v; }
    void setDurationSec(float v)          { m_durationSec  = v; }
    void setWarmupSec(float v)            { m_warmupSec    = v; }

    [[nodiscard]] uint32_t            id()            const { return m_id;            }
    [[nodiscard]] const std::string&  name()          const { return m_name;          }
    [[nodiscard]] BenchmarkCategory   category()      const { return m_category;      }
    [[nodiscard]] BenchmarkRunState   state()         const { return m_state;         }
    [[nodiscard]] BenchmarkMetric     primaryMetric() const { return m_primaryMetric; }
    [[nodiscard]] bool                isEnabled()     const { return m_isEnabled;     }
    [[nodiscard]] float               durationSec()   const { return m_durationSec;   }
    [[nodiscard]] float               warmupSec()     const { return m_warmupSec;     }

private:
    uint32_t          m_id;
    std::string       m_name;
    BenchmarkCategory m_category;
    BenchmarkRunState m_state         = BenchmarkRunState::Pending;
    BenchmarkMetric   m_primaryMetric = BenchmarkMetric::FPS;
    bool              m_isEnabled     = true;
    float             m_durationSec   = 10.0f;
    float             m_warmupSec     = 2.0f;
};

class BenchmarkSuiteEditor {
public:
    void setShowCompleted(bool v)       { m_showCompleted  = v; }
    void setAutoBaseline(bool v)        { m_autoBaseline   = v; }
    void setRepeatCount(uint32_t v)     { m_repeatCount    = v; }

    bool addCase(const BenchmarkCase& c) {
        for (auto& x : m_cases) if (x.id() == c.id()) return false;
        m_cases.push_back(c); return true;
    }
    bool removeCase(uint32_t id) {
        auto it = std::find_if(m_cases.begin(), m_cases.end(),
            [&](const BenchmarkCase& c){ return c.id() == id; });
        if (it == m_cases.end()) return false;
        m_cases.erase(it); return true;
    }
    [[nodiscard]] BenchmarkCase* findCase(uint32_t id) {
        for (auto& c : m_cases) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool     isShowCompleted() const { return m_showCompleted; }
    [[nodiscard]] bool     isAutoBaseline()  const { return m_autoBaseline;  }
    [[nodiscard]] uint32_t repeatCount()     const { return m_repeatCount;   }
    [[nodiscard]] size_t   caseCount()       const { return m_cases.size();  }

    [[nodiscard]] size_t countByCategory(BenchmarkCategory c) const {
        size_t n = 0; for (auto& bc : m_cases) if (bc.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByState(BenchmarkRunState s) const {
        size_t n = 0; for (auto& bc : m_cases) if (bc.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& bc : m_cases) if (bc.isEnabled()) ++n; return n;
    }

private:
    std::vector<BenchmarkCase> m_cases;
    bool     m_showCompleted = true;
    bool     m_autoBaseline  = false;
    uint32_t m_repeatCount   = 3u;
};

} // namespace NF
