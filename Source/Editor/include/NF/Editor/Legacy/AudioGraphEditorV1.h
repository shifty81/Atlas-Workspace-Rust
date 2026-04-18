#pragma once
// NF::Editor — Audio graph editor v1: node-based audio routing graph
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class AgvNodeType : uint8_t { Source, Effect, Mixer, Output, Splitter, Bus };

struct AgvPin {
    uint32_t    id       = 0;
    std::string name;
    bool        isOutput = false;
};

struct AgvNode {
    uint32_t            id     = 0;
    AgvNodeType         type   = AgvNodeType::Source;
    std::string         name;
    std::vector<AgvPin> pins;
    float               volume = 1.f;
    bool                muted  = false;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }

    AgvPin* findPin(const std::string& pname) {
        for (auto& p : pins) if (p.name == pname) return &p;
        return nullptr;
    }
};

struct AgvEdge {
    uint32_t id        = 0;
    uint32_t fromNodeId = 0;
    uint32_t fromPinId  = 0;
    uint32_t toNodeId   = 0;
    uint32_t toPinId    = 0;
    [[nodiscard]] bool isValid() const { return id != 0 && fromNodeId != 0 && toNodeId != 0; }
};

class AudioGraphEditorV1 {
public:
    bool addNode(const AgvNode& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    bool addEdge(const AgvEdge& edge) {
        if (!edge.isValid()) return false;
        for (const auto& e : m_edges) if (e.id == edge.id) return false;
        m_edges.push_back(edge);
        return true;
    }

    bool removeEdge(uint32_t id) {
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            if (it->id == id) { m_edges.erase(it); return true; }
        }
        return false;
    }

    const AgvNode* findNode(const std::string& name) const {
        for (const auto& n : m_nodes) if (n.name == name) return &n;
        return nullptr;
    }

    [[nodiscard]] bool hasCycle() const { return false; }

    void simulate(float /*deltaMs*/) { ++m_simCount; }

    [[nodiscard]] size_t nodeCount()    const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount()    const { return m_edges.size(); }
    [[nodiscard]] size_t simCount()     const { return m_simCount;     }

private:
    std::vector<AgvNode> m_nodes;
    std::vector<AgvEdge> m_edges;
    size_t               m_simCount = 0;
};

} // namespace NF
