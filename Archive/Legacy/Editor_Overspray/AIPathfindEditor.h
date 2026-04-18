#pragma once
// NF::Editor — AI pathfinding configuration management
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

enum class AiPathAlgo : uint8_t { AStar, Dijkstra, NavMesh, FlowField, Custom };
inline const char* aiPathAlgoName(AiPathAlgo v) {
    switch (v) {
        case AiPathAlgo::AStar:     return "AStar";
        case AiPathAlgo::Dijkstra:  return "Dijkstra";
        case AiPathAlgo::NavMesh:   return "NavMesh";
        case AiPathAlgo::FlowField: return "FlowField";
        case AiPathAlgo::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class AiPathHeuristic : uint8_t { Manhattan, Euclidean, Chebyshev, Octile, Zero };
inline const char* aiPathHeuristicName(AiPathHeuristic v) {
    switch (v) {
        case AiPathHeuristic::Manhattan: return "Manhattan";
        case AiPathHeuristic::Euclidean: return "Euclidean";
        case AiPathHeuristic::Chebyshev: return "Chebyshev";
        case AiPathHeuristic::Octile:    return "Octile";
        case AiPathHeuristic::Zero:      return "Zero";
    }
    return "Unknown";
}

class AiPathfindConfig {
public:
    explicit AiPathfindConfig(uint32_t id, const std::string& name,
                               AiPathAlgo algo, AiPathHeuristic heuristic)
        : m_id(id), m_name(name), m_algo(algo), m_heuristic(heuristic) {}

    void setMaxSearchNodes(uint32_t v) { m_maxSearchNodes = v; }
    void setAgentRadius(float v)       { m_agentRadius    = v; }
    void setIsEnabled(bool v)          { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] AiPathAlgo         algo()           const { return m_algo;           }
    [[nodiscard]] AiPathHeuristic    heuristic()      const { return m_heuristic;      }
    [[nodiscard]] uint32_t           maxSearchNodes() const { return m_maxSearchNodes; }
    [[nodiscard]] float              agentRadius()    const { return m_agentRadius;    }
    [[nodiscard]] bool               isEnabled()      const { return m_isEnabled;      }

private:
    uint32_t        m_id;
    std::string     m_name;
    AiPathAlgo      m_algo;
    AiPathHeuristic m_heuristic;
    uint32_t        m_maxSearchNodes = 1024u;
    float           m_agentRadius    = 0.5f;
    bool            m_isEnabled      = true;
};

class AIPathfindEditor {
public:
    void setIsShowDisabled(bool v)          { m_isShowDisabled       = v; }
    void setIsGroupByAlgo(bool v)           { m_isGroupByAlgo        = v; }
    void setDefaultMaxSearchNodes(uint32_t v) { m_defaultMaxSearchNodes = v; }

    bool addConfig(const AiPathfindConfig& c) {
        for (auto& x : m_configs) if (x.id() == c.id()) return false;
        m_configs.push_back(c); return true;
    }
    bool removeConfig(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const AiPathfindConfig& c){ return c.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] AiPathfindConfig* findConfig(uint32_t id) {
        for (auto& c : m_configs) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()        const { return m_isShowDisabled;        }
    [[nodiscard]] bool     isGroupByAlgo()         const { return m_isGroupByAlgo;         }
    [[nodiscard]] uint32_t defaultMaxSearchNodes() const { return m_defaultMaxSearchNodes; }
    [[nodiscard]] size_t   configCount()           const { return m_configs.size();        }

    [[nodiscard]] size_t countByAlgo(AiPathAlgo a) const {
        size_t n = 0; for (auto& x : m_configs) if (x.algo() == a) ++n; return n;
    }
    [[nodiscard]] size_t countByHeuristic(AiPathHeuristic h) const {
        size_t n = 0; for (auto& x : m_configs) if (x.heuristic() == h) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_configs) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<AiPathfindConfig> m_configs;
    bool     m_isShowDisabled        = false;
    bool     m_isGroupByAlgo         = true;
    uint32_t m_defaultMaxSearchNodes = 512u;
};

} // namespace NF
