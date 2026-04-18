#pragma once
// NF::Editor — Material node editor v1: node graph for material authoring
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class MnvNodeType : uint8_t { Texture, Color, Math, Blend, Normal, Output };
enum class MnvDataType : uint8_t { Float, Vec2, Vec3, Vec4, Sampler2D };

struct MnvPort {
    uint32_t    id       = 0;
    std::string name;
    MnvDataType type     = MnvDataType::Float;
    bool        isOutput = false;
};

struct MnvNode {
    uint32_t            id   = 0;
    MnvNodeType         type = MnvNodeType::Color;
    std::string         name;
    float               x    = 0.f;
    float               y    = 0.f;
    std::vector<MnvPort> ports;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
    MnvPort* findPort(const std::string& pname) {
        for (auto& p : ports) if (p.name == pname) return &p;
        return nullptr;
    }
};

struct MnvEdge {
    uint32_t id       = 0;
    uint32_t fromNode = 0;
    uint32_t fromPort = 0;
    uint32_t toNode   = 0;
    uint32_t toPort   = 0;
    [[nodiscard]] bool isValid() const { return id != 0 && fromNode != 0 && toNode != 0; }
};

class MaterialNodeEditorV1 {
public:
    bool addNode(const MnvNode& node) {
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

    bool addEdge(const MnvEdge& edge) {
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

    [[nodiscard]] bool validate() const {
        for (const auto& n : m_nodes)
            if (n.type == MnvNodeType::Output) return true;
        return false;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount() const { return m_edges.size(); }

private:
    std::vector<MnvNode> m_nodes;
    std::vector<MnvEdge> m_edges;
};

} // namespace NF
