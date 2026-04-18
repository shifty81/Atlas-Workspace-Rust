#pragma once
// NF::Editor — Performance budget editor
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

enum class BudgetCategory : uint8_t {
    CPU, GPU, Memory, DrawCalls, Triangles, TextureMemory, AudioMemory, NetworkBandwidth
};

inline const char* budgetCategoryName(BudgetCategory c) {
    switch (c) {
        case BudgetCategory::CPU:              return "CPU";
        case BudgetCategory::GPU:              return "GPU";
        case BudgetCategory::Memory:           return "Memory";
        case BudgetCategory::DrawCalls:        return "DrawCalls";
        case BudgetCategory::Triangles:        return "Triangles";
        case BudgetCategory::TextureMemory:    return "TextureMemory";
        case BudgetCategory::AudioMemory:      return "AudioMemory";
        case BudgetCategory::NetworkBandwidth: return "NetworkBandwidth";
    }
    return "Unknown";
}

enum class BudgetStatus : uint8_t {
    Ok, Warning, Critical, Exceeded
};

inline const char* budgetStatusName(BudgetStatus s) {
    switch (s) {
        case BudgetStatus::Ok:       return "Ok";
        case BudgetStatus::Warning:  return "Warning";
        case BudgetStatus::Critical: return "Critical";
        case BudgetStatus::Exceeded: return "Exceeded";
    }
    return "Unknown";
}

enum class BudgetTargetPlatform : uint8_t {
    PC, Console, Mobile, VR, WebGL
};

inline const char* budgetTargetPlatformName(BudgetTargetPlatform p) {
    switch (p) {
        case BudgetTargetPlatform::PC:      return "PC";
        case BudgetTargetPlatform::Console: return "Console";
        case BudgetTargetPlatform::Mobile:  return "Mobile";
        case BudgetTargetPlatform::VR:      return "VR";
        case BudgetTargetPlatform::WebGL:   return "WebGL";
    }
    return "Unknown";
}

class BudgetEntry {
public:
    explicit BudgetEntry(BudgetCategory cat, float budget, float warnThreshold = 0.8f)
        : m_category(cat), m_budget(budget), m_warnThreshold(warnThreshold) {}

    void setCurrent(float v)            { m_current = v; }
    void setEnabled(bool v)             { m_enabled = v; }
    void setWarnThreshold(float v)      { m_warnThreshold = v; }

    [[nodiscard]] BudgetCategory category()       const { return m_category;       }
    [[nodiscard]] float          budget()         const { return m_budget;         }
    [[nodiscard]] float          current()        const { return m_current;        }
    [[nodiscard]] float          warnThreshold()  const { return m_warnThreshold;  }
    [[nodiscard]] bool           isEnabled()      const { return m_enabled;        }
    [[nodiscard]] float          utilization()    const {
        return (m_budget > 0.0f) ? (m_current / m_budget) : 0.0f;
    }
    [[nodiscard]] BudgetStatus   status() const {
        float u = utilization();
        if (u > 1.0f)              return BudgetStatus::Exceeded;
        if (u > m_warnThreshold + 0.1f) return BudgetStatus::Critical;
        if (u > m_warnThreshold)   return BudgetStatus::Warning;
        return BudgetStatus::Ok;
    }

private:
    BudgetCategory m_category;
    float          m_budget;
    float          m_current        = 0.0f;
    float          m_warnThreshold  = 0.8f;
    bool           m_enabled        = true;
};

class PerformanceBudgetEditor {
public:
    void setTargetPlatform(BudgetTargetPlatform p) { m_platform = p; }
    void setTargetFPS(float fps)                   { m_targetFPS = fps; }

    [[nodiscard]] bool addEntry(const BudgetEntry& entry) {
        for (auto& e : m_entries) if (e.category() == entry.category()) return false;
        m_entries.push_back(entry);
        return true;
    }

    [[nodiscard]] bool removeEntry(BudgetCategory cat) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->category() == cat) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] BudgetEntry* findEntry(BudgetCategory cat) {
        for (auto& e : m_entries) if (e.category() == cat) return &e;
        return nullptr;
    }

    [[nodiscard]] BudgetTargetPlatform targetPlatform() const { return m_platform;    }
    [[nodiscard]] float                targetFPS()      const { return m_targetFPS;   }
    [[nodiscard]] size_t               entryCount()     const { return m_entries.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t exceededCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.status() == BudgetStatus::Exceeded) ++c; return c;
    }
    [[nodiscard]] size_t warningCount() const {
        size_t c = 0;
        for (auto& e : m_entries) {
            auto s = e.status();
            if (s == BudgetStatus::Warning || s == BudgetStatus::Critical) ++c;
        }
        return c;
    }
    [[nodiscard]] size_t okCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.status() == BudgetStatus::Ok) ++c; return c;
    }

private:
    std::vector<BudgetEntry>  m_entries;
    BudgetTargetPlatform      m_platform  = BudgetTargetPlatform::PC;
    float                     m_targetFPS = 60.0f;
};

} // namespace NF
