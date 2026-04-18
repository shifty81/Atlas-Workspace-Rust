#pragma once
// NF::Editor — Shader graph editor
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

enum class ShaderNodeType : uint8_t {
    Input, Output, Math, Texture, Color, Vector, Blend, Custom
};

inline const char* shaderNodeTypeName(ShaderNodeType t) {
    switch (t) {
        case ShaderNodeType::Input:   return "Input";
        case ShaderNodeType::Output:  return "Output";
        case ShaderNodeType::Math:    return "Math";
        case ShaderNodeType::Texture: return "Texture";
        case ShaderNodeType::Color:   return "Color";
        case ShaderNodeType::Vector:  return "Vector";
        case ShaderNodeType::Blend:   return "Blend";
        case ShaderNodeType::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class ShaderPortKind : uint8_t {
    Float, Vector2, Vector3, Vector4
};

inline const char* shaderPortKindName(ShaderPortKind k) {
    switch (k) {
        case ShaderPortKind::Float:   return "Float";
        case ShaderPortKind::Vector2: return "Vector2";
        case ShaderPortKind::Vector3: return "Vector3";
        case ShaderPortKind::Vector4: return "Vector4";
    }
    return "Unknown";
}

struct ShaderNode {
    std::string    id;
    ShaderNodeType type     = ShaderNodeType::Math;
    float          posX     = 0.0f;
    float          posY     = 0.0f;
    bool           selected = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setPosition(float x, float y) { posX = x; posY = y; }
};

struct ShaderGraphEdge {
    std::string  id;
    std::string  fromNode;
    std::string  toNode;
    ShaderPortKind portKind = ShaderPortKind::Float;
};

class ShaderGraphEditor {
public:
    static constexpr size_t MAX_NODES = 256;
    static constexpr size_t MAX_EDGES = 512;

    [[nodiscard]] bool addNode(const ShaderNode& node) {
        for (auto& n : m_nodes) if (n.id == node.id) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        m_nodes.push_back(node);
        return true;
    }

    [[nodiscard]] bool removeNode(const std::string& id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) {
                // remove edges connected to this node
                m_edges.erase(
                    std::remove_if(m_edges.begin(), m_edges.end(),
                        [&id](const ShaderGraphEdge& e) {
                            return e.fromNode == id || e.toNode == id;
                        }),
                    m_edges.end());
                if (m_activeNode == id) m_activeNode.clear();
                m_nodes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ShaderNode* findNode(const std::string& id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] bool addEdge(const ShaderGraphEdge& edge) {
        for (auto& e : m_edges) if (e.id == edge.id) return false;
        if (m_edges.size() >= MAX_EDGES) return false;
        // both endpoints must exist
        if (!findNode(edge.fromNode) || !findNode(edge.toNode)) return false;
        m_edges.push_back(edge);
        return true;
    }

    [[nodiscard]] bool removeEdge(const std::string& id) {
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            if (it->id == id) { m_edges.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] bool setActiveNode(const std::string& id) {
        for (auto& n : m_nodes) {
            if (n.id == id) { m_activeNode = id; return true; }
        }
        return false;
    }

    void selectAll()   { for (auto& n : m_nodes) n.select();   }
    void deselectAll() { for (auto& n : m_nodes) n.deselect(); }

    [[nodiscard]] size_t nodeCount()     const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount()     const { return m_edges.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.selected) ++c; return c;
    }
    [[nodiscard]] const std::string& activeNode() const { return m_activeNode; }

private:
    std::vector<ShaderNode>      m_nodes;
    std::vector<ShaderGraphEdge> m_edges;
    std::string                  m_activeNode;
};

// ── S34 — Material Editor ─────────────────────────────────────────


} // namespace NF
