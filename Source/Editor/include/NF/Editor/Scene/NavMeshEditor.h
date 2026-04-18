#pragma once
// NF::Editor — Navigation mesh editor and debugger
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

enum class NavMeshBuildStatus : uint8_t {
    Idle, Building, Done, Failed, Outdated
};

inline const char* navMeshBuildStatusName(NavMeshBuildStatus s) {
    switch (s) {
        case NavMeshBuildStatus::Idle:     return "Idle";
        case NavMeshBuildStatus::Building: return "Building";
        case NavMeshBuildStatus::Done:     return "Done";
        case NavMeshBuildStatus::Failed:   return "Failed";
        case NavMeshBuildStatus::Outdated: return "Outdated";
    }
    return "Unknown";
}

enum class NavAgentCategory : uint8_t {
    Humanoid, Vehicle, Flying, Crawling, Custom
};

inline const char* navAgentCategoryName(NavAgentCategory c) {
    switch (c) {
        case NavAgentCategory::Humanoid: return "Humanoid";
        case NavAgentCategory::Vehicle:  return "Vehicle";
        case NavAgentCategory::Flying:   return "Flying";
        case NavAgentCategory::Crawling: return "Crawling";
        case NavAgentCategory::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class NavMeshDebugView : uint8_t {
    None, Polygons, Connections, Areas, Boundaries, All
};

inline const char* navMeshDebugViewName(NavMeshDebugView v) {
    switch (v) {
        case NavMeshDebugView::None:        return "None";
        case NavMeshDebugView::Polygons:    return "Polygons";
        case NavMeshDebugView::Connections: return "Connections";
        case NavMeshDebugView::Areas:       return "Areas";
        case NavMeshDebugView::Boundaries:  return "Boundaries";
        case NavMeshDebugView::All:         return "All";
    }
    return "Unknown";
}

class NavAgentConfig {
public:
    explicit NavAgentConfig(const std::string& name, NavAgentCategory category)
        : m_name(name), m_category(category) {}

    void setRadius(float r)      { m_radius    = r; }
    void setHeight(float h)      { m_height    = h; }
    void setMaxSlope(float s)    { m_maxSlope  = s; }
    void setStepHeight(float sh) { m_stepHeight = sh; }
    void setEnabled(bool v)      { m_enabled   = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] NavAgentCategory   category()   const { return m_category;   }
    [[nodiscard]] float              radius()     const { return m_radius;     }
    [[nodiscard]] float              height()     const { return m_height;     }
    [[nodiscard]] float              maxSlope()   const { return m_maxSlope;   }
    [[nodiscard]] float              stepHeight() const { return m_stepHeight; }
    [[nodiscard]] bool               isEnabled()  const { return m_enabled;    }

private:
    std::string       m_name;
    NavAgentCategory  m_category;
    float             m_radius     = 0.5f;
    float             m_height     = 2.0f;
    float             m_maxSlope   = 45.0f;
    float             m_stepHeight = 0.3f;
    bool              m_enabled    = true;
};

class NavMeshEditor {
public:
    static constexpr size_t MAX_AGENTS = 16;

    [[nodiscard]] bool addAgent(const NavAgentConfig& agent) {
        for (auto& a : m_agents) if (a.name() == agent.name()) return false;
        if (m_agents.size() >= MAX_AGENTS) return false;
        m_agents.push_back(agent);
        return true;
    }

    [[nodiscard]] bool removeAgent(const std::string& name) {
        for (auto it = m_agents.begin(); it != m_agents.end(); ++it) {
            if (it->name() == name) { m_agents.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] NavAgentConfig* findAgent(const std::string& name) {
        for (auto& a : m_agents) if (a.name() == name) return &a;
        return nullptr;
    }

    void setBuildStatus(NavMeshBuildStatus s) { m_buildStatus = s; }
    void setDebugView(NavMeshDebugView v)     { m_debugView   = v; }
    void setAutoRebuild(bool v)               { m_autoRebuild = v; }
    void setCellSize(float s)                 { m_cellSize    = s; }

    [[nodiscard]] NavMeshBuildStatus buildStatus() const { return m_buildStatus; }
    [[nodiscard]] NavMeshDebugView   debugView()   const { return m_debugView;   }
    [[nodiscard]] bool               autoRebuild() const { return m_autoRebuild; }
    [[nodiscard]] float              cellSize()    const { return m_cellSize;    }
    [[nodiscard]] size_t             agentCount()  const { return m_agents.size(); }

    [[nodiscard]] bool isBuilt() const { return m_buildStatus == NavMeshBuildStatus::Done; }
    [[nodiscard]] size_t enabledAgentCount() const {
        size_t c = 0; for (auto& a : m_agents) if (a.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(NavAgentCategory cat) const {
        size_t c = 0; for (auto& a : m_agents) if (a.category() == cat) ++c; return c;
    }

private:
    std::vector<NavAgentConfig> m_agents;
    NavMeshBuildStatus          m_buildStatus = NavMeshBuildStatus::Idle;
    NavMeshDebugView            m_debugView   = NavMeshDebugView::None;
    float                       m_cellSize    = 0.3f;
    bool                        m_autoRebuild = false;
};

} // namespace NF
