#pragma once
// NF::Editor — Render stats panel
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

enum class RenderStatCategory : uint8_t {
    General, Geometry, Shadow, PostProcess, Particle, Lighting, Transparency, Compute
};

inline const char* renderStatCategoryName(RenderStatCategory c) {
    switch (c) {
        case RenderStatCategory::General:      return "General";
        case RenderStatCategory::Geometry:     return "Geometry";
        case RenderStatCategory::Shadow:       return "Shadow";
        case RenderStatCategory::PostProcess:  return "PostProcess";
        case RenderStatCategory::Particle:     return "Particle";
        case RenderStatCategory::Lighting:     return "Lighting";
        case RenderStatCategory::Transparency: return "Transparency";
        case RenderStatCategory::Compute:      return "Compute";
    }
    return "Unknown";
}

enum class RenderStatUnit : uint8_t {
    Count, Milliseconds, Megabytes, Percentage, MegaPixels
};

inline const char* renderStatUnitName(RenderStatUnit u) {
    switch (u) {
        case RenderStatUnit::Count:       return "Count";
        case RenderStatUnit::Milliseconds:return "Milliseconds";
        case RenderStatUnit::Megabytes:   return "Megabytes";
        case RenderStatUnit::Percentage:  return "Percentage";
        case RenderStatUnit::MegaPixels:  return "MegaPixels";
    }
    return "Unknown";
}

enum class RenderStatsPanelMode : uint8_t {
    Compact, Detailed, Graph, Overlay
};

inline const char* renderStatsPanelModeName(RenderStatsPanelMode m) {
    switch (m) {
        case RenderStatsPanelMode::Compact:  return "Compact";
        case RenderStatsPanelMode::Detailed: return "Detailed";
        case RenderStatsPanelMode::Graph:    return "Graph";
        case RenderStatsPanelMode::Overlay:  return "Overlay";
    }
    return "Unknown";
}

class RenderStat {
public:
    explicit RenderStat(const std::string& name, RenderStatCategory cat, RenderStatUnit unit)
        : m_name(name), m_category(cat), m_unit(unit) {}

    void setValue(double v)    { m_value   = v; }
    void setEnabled(bool v)    { m_enabled = v; }
    void setPinned(bool v)     { m_pinned  = v; }

    [[nodiscard]] const std::string&  name()     const { return m_name;     }
    [[nodiscard]] RenderStatCategory  category() const { return m_category; }
    [[nodiscard]] RenderStatUnit      unit()     const { return m_unit;     }
    [[nodiscard]] double              value()    const { return m_value;    }
    [[nodiscard]] bool                isEnabled()const { return m_enabled;  }
    [[nodiscard]] bool                isPinned() const { return m_pinned;   }

private:
    std::string         m_name;
    RenderStatCategory  m_category;
    RenderStatUnit      m_unit;
    double              m_value   = 0.0;
    bool                m_enabled = true;
    bool                m_pinned  = false;
};

class RenderStatsPanel {
public:
    void setMode(RenderStatsPanelMode m) { m_mode = m; }
    void setVisible(bool v)              { m_visible = v; }
    void setFreezeStats(bool v)          { m_frozen  = v; }

    void addStat(const RenderStat& stat) { m_stats.push_back(stat); }

    [[nodiscard]] RenderStat* findStat(const std::string& name) {
        for (auto& s : m_stats) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] RenderStatsPanelMode mode()       const { return m_mode;    }
    [[nodiscard]] bool                 isVisible()  const { return m_visible; }
    [[nodiscard]] bool                 isFrozen()   const { return m_frozen;  }
    [[nodiscard]] size_t               statCount()  const { return m_stats.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& s : m_stats) if (s.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t pinnedCount()  const {
        size_t c = 0; for (auto& s : m_stats) if (s.isPinned()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(RenderStatCategory cat) const {
        size_t c = 0; for (auto& s : m_stats) if (s.category() == cat) ++c; return c;
    }
    [[nodiscard]] double totalValueForCategory(RenderStatCategory cat) const {
        double t = 0.0; for (auto& s : m_stats) if (s.category() == cat) t += s.value(); return t;
    }

private:
    std::vector<RenderStat>  m_stats;
    RenderStatsPanelMode     m_mode    = RenderStatsPanelMode::Compact;
    bool                     m_visible = true;
    bool                     m_frozen  = false;
};

} // namespace NF
