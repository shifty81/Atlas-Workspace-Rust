#pragma once
// NF::Editor — Graph host contract v1: formal interface for embedded graph editors
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Graph Host Capability Flags V1 ────────────────────────────────
// (extends what GraphHostContract.h stubbed earlier)

enum class GraphCapV1 : uint32_t {
    None            = 0,
    Undo            = 1 << 0,
    Redo            = 1 << 1,
    Zoom            = 1 << 2,
    Pan             = 1 << 3,
    MultiSelect     = 1 << 4,
    Minimap         = 1 << 5,
    Search          = 1 << 6,
    Validation      = 1 << 7,
    CopyPaste       = 1 << 8,
    DragDrop        = 1 << 9,
    Grouping        = 1 << 10,
    Bookmarks       = 1 << 11,
};

inline GraphCapV1 operator|(GraphCapV1 a, GraphCapV1 b) {
    return static_cast<GraphCapV1>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool hasGraphCapV1(GraphCapV1 caps, GraphCapV1 flag) {
    return (static_cast<uint32_t>(caps) & static_cast<uint32_t>(flag)) != 0;
}

// ── Graph Node Pin Type ────────────────────────────────────────────

enum class GraphPinType : uint8_t { Exec, Bool, Int, Float, String, Vector, Object, Any };

inline const char* graphPinTypeName(GraphPinType t) {
    switch (t) {
        case GraphPinType::Exec:   return "Exec";
        case GraphPinType::Bool:   return "Bool";
        case GraphPinType::Int:    return "Int";
        case GraphPinType::Float:  return "Float";
        case GraphPinType::String: return "String";
        case GraphPinType::Vector: return "Vector";
        case GraphPinType::Object: return "Object";
        case GraphPinType::Any:    return "Any";
    }
    return "Unknown";
}

// ── Graph Pin ─────────────────────────────────────────────────────

struct GraphPinV1 {
    uint32_t      id       = 0;
    std::string   name;
    GraphPinType  type     = GraphPinType::Any;
    bool          isOutput = false;
    bool          connected = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

// ── Graph Node V1 ─────────────────────────────────────────────────

struct GraphNodeV1 {
    uint32_t                 id      = 0;
    std::string              title;
    std::string              category;
    float                    x       = 0.f;
    float                    y       = 0.f;
    float                    width   = 160.f;
    float                    height  = 60.f;
    bool                     selected = false;
    std::vector<GraphPinV1>  pins;

    [[nodiscard]] bool isValid() const { return id != 0 && !title.empty(); }
    [[nodiscard]] size_t pinCount() const { return pins.size(); }

    const GraphPinV1* findPin(uint32_t pinId) const {
        for (const auto& p : pins) if (p.id == pinId) return &p;
        return nullptr;
    }
    void addPin(const GraphPinV1& pin) { pins.push_back(pin); }
};

// ── Graph Edge V1 ─────────────────────────────────────────────────

struct GraphEdgeV1 {
    uint32_t id        = 0;
    uint32_t fromNode  = 0;
    uint32_t fromPin   = 0;
    uint32_t toNode    = 0;
    uint32_t toPin     = 0;

    [[nodiscard]] bool isValid() const {
        return id != 0 && fromNode != 0 && toNode != 0;
    }
};

// ── Graph Host Contract V1 ────────────────────────────────────────
// Manages a graph's node/edge model and provides host operations.

using GraphChangeCallback = std::function<void()>;

class GraphHostContractV1 {
public:
    explicit GraphHostContractV1(const std::string& graphId = "")
        : m_graphId(graphId) {}

    void initialize(GraphCapV1 caps = GraphCapV1::None) {
        m_capabilities = caps;
        m_initialized  = true;
    }

    // Node management
    bool addNode(const GraphNodeV1& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        m_nodes.push_back(node);
        ++m_changeSeq;
        notifyChange();
        return true;
    }

    bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) {
                // Remove connected edges
                m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(),
                    [id](const GraphEdgeV1& e) {
                        return e.fromNode == id || e.toNode == id;
                    }), m_edges.end());
                m_nodes.erase(it);
                ++m_changeSeq;
                notifyChange();
                return true;
            }
        }
        return false;
    }

    bool moveNode(uint32_t id, float x, float y) {
        for (auto& n : m_nodes) {
            if (n.id == id) { n.x = x; n.y = y; ++m_changeSeq; notifyChange(); return true; }
        }
        return false;
    }

    // Edge management
    bool addEdge(const GraphEdgeV1& edge) {
        if (!edge.isValid()) return false;
        for (const auto& e : m_edges) if (e.id == edge.id) return false;
        // Validate endpoints exist
        if (!findNode(edge.fromNode) || !findNode(edge.toNode)) return false;
        m_edges.push_back(edge);
        ++m_changeSeq;
        notifyChange();
        return true;
    }

    bool removeEdge(uint32_t id) {
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            if (it->id == id) { m_edges.erase(it); ++m_changeSeq; notifyChange(); return true; }
        }
        return false;
    }

    // Selection
    void selectNode(uint32_t id, bool exclusive = true) {
        if (exclusive) for (auto& n : m_nodes) n.selected = false;
        for (auto& n : m_nodes) if (n.id == id) { n.selected = true; break; }
    }

    void clearSelection() {
        for (auto& n : m_nodes) n.selected = false;
    }

    [[nodiscard]] std::vector<uint32_t> selectedNodeIds() const {
        std::vector<uint32_t> ids;
        for (const auto& n : m_nodes) if (n.selected) ids.push_back(n.id);
        return ids;
    }

    // View state
    void setViewZoom(float zoom) {
        m_zoom = zoom < 0.1f ? 0.1f : (zoom > 8.f ? 8.f : zoom);
    }
    void setViewScroll(float x, float y) { m_scrollX = x; m_scrollY = y; }

    // Accessors
    [[nodiscard]] const GraphNodeV1* findNode(uint32_t id) const {
        for (const auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }
    [[nodiscard]] const GraphEdgeV1* findEdge(uint32_t id) const {
        for (const auto& e : m_edges) if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool        isInitialized() const { return m_initialized; }
    [[nodiscard]] const std::string& graphId() const { return m_graphId;    }
    [[nodiscard]] GraphCapV1  capabilities()   const { return m_capabilities; }
    [[nodiscard]] size_t      nodeCount()      const { return m_nodes.size(); }
    [[nodiscard]] size_t      edgeCount()      const { return m_edges.size(); }
    [[nodiscard]] uint64_t    changeSeq()      const { return m_changeSeq;   }
    [[nodiscard]] float       zoom()           const { return m_zoom;         }
    [[nodiscard]] float       scrollX()        const { return m_scrollX;      }
    [[nodiscard]] float       scrollY()        const { return m_scrollY;      }

    [[nodiscard]] const std::vector<GraphNodeV1>& nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<GraphEdgeV1>& edges() const { return m_edges; }

    void setOnChange(GraphChangeCallback cb) { m_onChange = std::move(cb); }

    bool hasCapability(GraphCapV1 flag) const { return hasGraphCapV1(m_capabilities, flag); }

private:
    void notifyChange() { if (m_onChange) m_onChange(); }

    std::string             m_graphId;
    GraphCapV1              m_capabilities = GraphCapV1::None;
    std::vector<GraphNodeV1> m_nodes;
    std::vector<GraphEdgeV1> m_edges;
    GraphChangeCallback     m_onChange;
    float    m_zoom    = 1.f;
    float    m_scrollX = 0.f;
    float    m_scrollY = 0.f;
    uint64_t m_changeSeq   = 0;
    bool     m_initialized = false;
};

} // namespace NF
