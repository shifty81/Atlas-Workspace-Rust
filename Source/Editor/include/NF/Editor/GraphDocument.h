#pragma once
// NF::Editor — GraphDocument: visual logic graph document model.
//
// Phase G.5 — Visual Logic Editor full tool wiring.
//
// A GraphDocument stores the authoring state for one visual script graph:
//   - Node table (typed nodes with display metadata and position)
//   - Per-node pin table (input/output execution + data pins)
//   - Connection table (pin-to-pin wiring)
//   - Per-node property bag (string-encoded)
//   - Compile result (error/warning list)
//   - Dirty tracking + save/load contract
//
// VisualLogicEditorTool owns one GraphDocument at a time.

#include "NF/Core/Core.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace NF {

// ── Identifiers ────────────────────────────────────────────────────────────────

using GraphNodeId = uint32_t;
using GraphPinId  = uint32_t;
using GraphConnId = uint32_t;

static constexpr GraphNodeId kInvalidGraphNodeId = 0u;
static constexpr GraphPinId  kInvalidGraphPinId  = 0u;
static constexpr GraphConnId kInvalidGraphConnId  = 0u;

// ── Pin category ───────────────────────────────────────────────────────────────

enum class GraphPinDir : uint8_t { Input, Output };

enum class GraphPinCategory : uint8_t {
    Execution, ///< control-flow pin (exec wire)
    Data,      ///< typed data pin
};

// ── GraphPin ───────────────────────────────────────────────────────────────────

struct GraphPin {
    GraphPinId       id        = kInvalidGraphPinId;
    GraphNodeId      nodeId    = kInvalidGraphNodeId;
    std::string      name;
    GraphPinDir      direction = GraphPinDir::Input;
    GraphPinCategory category  = GraphPinCategory::Data;
    std::string      dataType; ///< "float", "bool", "string", …; ignored for Execution pins
};

// ── GraphConnection ────────────────────────────────────────────────────────────

struct GraphConnection {
    GraphConnId conn     = kInvalidGraphConnId;
    GraphNodeId fromNode = kInvalidGraphNodeId;
    GraphPinId  fromPin  = kInvalidGraphPinId;
    GraphNodeId toNode   = kInvalidGraphNodeId;
    GraphPinId  toPin    = kInvalidGraphPinId;
};

// ── GraphNode ──────────────────────────────────────────────────────────────────

struct GraphNode {
    GraphNodeId id       = kInvalidGraphNodeId;
    std::string typeName; ///< e.g. "OnGameStart", "BranchNode", "PrintString"
    std::string title;    ///< display title (may differ from typeName)
    float       posX     = 0.f;
    float       posY     = 0.f;
    bool        hasError = false;
    std::map<std::string, std::string> properties;
    std::vector<GraphPin> pins;
};

// ── Compile result ─────────────────────────────────────────────────────────────

enum class GraphCompileSeverity : uint8_t { Error, Warning };

struct GraphCompileMessage {
    GraphCompileSeverity severity    = GraphCompileSeverity::Error;
    GraphNodeId          nodeId      = kInvalidGraphNodeId;
    std::string          message;
};

struct GraphCompileResult {
    bool                              success = true;
    std::vector<GraphCompileMessage>  messages;

    [[nodiscard]] uint32_t errorCount() const {
        uint32_t n = 0;
        for (const auto& m : messages) {
            if (m.severity == GraphCompileSeverity::Error) ++n;
        }
        return n;
    }

    [[nodiscard]] uint32_t warningCount() const {
        return static_cast<uint32_t>(messages.size()) - errorCount();
    }
};

// ── GraphDocument ──────────────────────────────────────────────────────────────

class GraphDocument {
public:
    GraphDocument() = default;
    explicit GraphDocument(const std::string& graphName) : m_graphName(graphName) {}

    // ── Identity ───────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& graphName()  const { return m_graphName; }
    [[nodiscard]] const std::string& assetPath()  const { return m_assetPath; }
    void setGraphName(const std::string& n)  { m_graphName = n; markDirty(); }
    void setAssetPath(const std::string& p)  { m_assetPath = p; }

    // ── Dirty tracking ─────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── Node management ────────────────────────────────────────────────────────

    GraphNodeId addNode(const std::string& typeName, const std::string& title,
                         float posX = 0.f, float posY = 0.f) {
        GraphNodeId id = ++m_nextNodeId;
        GraphNode node;
        node.id       = id;
        node.typeName = typeName;
        node.title    = title;
        node.posX     = posX;
        node.posY     = posY;
        m_nodes.push_back(std::move(node));
        markDirty();
        return id;
    }

    bool removeNode(GraphNodeId id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) {
                // Remove all connections touching this node
                m_connections.erase(
                    std::remove_if(m_connections.begin(), m_connections.end(),
                        [id](const GraphConnection& c) {
                            return c.fromNode == id || c.toNode == id;
                        }),
                    m_connections.end());
                m_nodes.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t nodeCount() const {
        return static_cast<uint32_t>(m_nodes.size());
    }

    [[nodiscard]] const GraphNode* findNode(GraphNodeId id) const {
        for (const auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] GraphNode* findNode(GraphNodeId id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] const std::vector<GraphNode>& nodes() const { return m_nodes; }

    // ── Property management ────────────────────────────────────────────────────

    bool setNodeProperty(GraphNodeId id, const std::string& key, const std::string& value) {
        auto* n = findNode(id);
        if (!n) return false;
        n->properties[key] = value;
        markDirty();
        return true;
    }

    [[nodiscard]] std::string getNodeProperty(GraphNodeId id, const std::string& key,
                                               const std::string& def = "") const {
        if (const auto* n = findNode(id)) {
            auto it = n->properties.find(key);
            if (it != n->properties.end()) return it->second;
        }
        return def;
    }

    // ── Pin management ─────────────────────────────────────────────────────────

    GraphPinId addPin(GraphNodeId nodeId, const std::string& name,
                       GraphPinDir dir, GraphPinCategory cat = GraphPinCategory::Data,
                       const std::string& dataType = "") {
        auto* n = findNode(nodeId);
        if (!n) return kInvalidGraphPinId;

        GraphPinId pid = ++m_nextPinId;
        GraphPin pin;
        pin.id        = pid;
        pin.nodeId    = nodeId;
        pin.name      = name;
        pin.direction = dir;
        pin.category  = cat;
        pin.dataType  = dataType;
        n->pins.push_back(std::move(pin));
        markDirty();
        return pid;
    }

    bool removePin(GraphNodeId nodeId, GraphPinId pinId) {
        auto* n = findNode(nodeId);
        if (!n) return false;
        auto& pins = n->pins;
        for (auto it = pins.begin(); it != pins.end(); ++it) {
            if (it->id == pinId) {
                m_connections.erase(
                    std::remove_if(m_connections.begin(), m_connections.end(),
                        [pinId](const GraphConnection& c) {
                            return c.fromPin == pinId || c.toPin == pinId;
                        }),
                    m_connections.end());
                pins.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t pinCount(GraphNodeId nodeId) const {
        if (const auto* n = findNode(nodeId)) {
            return static_cast<uint32_t>(n->pins.size());
        }
        return 0u;
    }

    // ── Connection management ──────────────────────────────────────────────────

    GraphConnId connect(GraphNodeId fromNode, GraphPinId fromPin,
                         GraphNodeId toNode,   GraphPinId toPin) {
        if (!findNode(fromNode) || !findNode(toNode)) return kInvalidGraphConnId;

        GraphConnId cid = ++m_nextConnId;
        GraphConnection c;
        c.conn     = cid;
        c.fromNode = fromNode;
        c.fromPin  = fromPin;
        c.toNode   = toNode;
        c.toPin    = toPin;
        m_connections.push_back(c);
        markDirty();
        return cid;
    }

    bool disconnect(GraphConnId connId) {
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it->conn == connId) {
                m_connections.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool isConnected(GraphNodeId toNode, GraphPinId toPin) const {
        for (const auto& c : m_connections) {
            if (c.toNode == toNode && c.toPin == toPin) return true;
        }
        return false;
    }

    [[nodiscard]] uint32_t connectionCount() const {
        return static_cast<uint32_t>(m_connections.size());
    }

    [[nodiscard]] const std::vector<GraphConnection>& connections() const {
        return m_connections;
    }

    // ── Compile ────────────────────────────────────────────────────────────────

    /// Validates the graph: checks all input execution pins are connected.
    /// Returns a compile result with errors listed.
    [[nodiscard]] GraphCompileResult compile() const {
        GraphCompileResult result;
        for (const auto& node : m_nodes) {
            for (const auto& pin : node.pins) {
                if (pin.direction == GraphPinDir::Input &&
                    pin.category  == GraphPinCategory::Execution) {
                    if (!isConnected(node.id, pin.id)) {
                        GraphCompileMessage msg;
                        msg.severity = GraphCompileSeverity::Warning;
                        msg.nodeId   = node.id;
                        msg.message  = "Input exec pin '" + pin.name + "' not connected";
                        result.messages.push_back(msg);
                    }
                }
            }
        }
        result.success = (result.errorCount() == 0u);
        return result;
    }

    // ── Save / load ────────────────────────────────────────────────────────────

    bool save(const std::string& path = "") {
        if (!path.empty()) m_assetPath = path;
        if (m_assetPath.empty()) return false;
        clearDirty();
        return true;
    }

    bool load(const std::string& /*json*/) {
        clearDirty();
        return true;
    }

    [[nodiscard]] std::string serialize() const {
        std::string out = "{\"graph\":\"" + m_graphName + "\",";
        out += "\"nodes\":" + std::to_string(m_nodes.size()) + ",";
        out += "\"connections\":" + std::to_string(m_connections.size()) + "}";
        return out;
    }

private:
    std::string  m_graphName;
    std::string  m_assetPath;
    bool         m_dirty = false;

    GraphNodeId m_nextNodeId = 0u;
    GraphPinId  m_nextPinId  = 0u;
    GraphConnId m_nextConnId = 0u;

    std::vector<GraphNode>       m_nodes;
    std::vector<GraphConnection> m_connections;
};

} // namespace NF
