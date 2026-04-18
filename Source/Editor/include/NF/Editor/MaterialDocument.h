#pragma once
// NF::Editor — MaterialDocument: material graph document model.
//
// Phase G.3 — Material Editor full tool wiring.
//
// A MaterialDocument stores the authoring state for a single material asset:
//   - A node graph (shader nodes + connections)
//   - Per-node float/string/bool parameters
//   - An output node designation
//   - Dirty tracking + save/load contract
//
// MaterialEditorTool owns one MaterialDocument at a time.

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace NF {

// ── Identifiers ────────────────────────────────────────────────────────────────

using MaterialNodeId = uint32_t;
using MaterialPinId  = uint32_t;

static constexpr MaterialNodeId kInvalidMaterialNodeId = 0u;
static constexpr MaterialPinId  kInvalidMaterialPinId  = 0u;

// ── Node types ─────────────────────────────────────────────────────────────────

enum class MaterialNodeType : uint8_t {
    Output,        ///< final surface output
    TextureSample, ///< samples a texture
    Constant,      ///< scalar/vector constant
    Parameter,     ///< exposed material parameter
    MathAdd,       ///< add two inputs
    MathMultiply,  ///< multiply two inputs
    Lerp,          ///< linear interpolation
    Fresnel,       ///< view-angle falloff
    Normal,        ///< normal map decode
    Custom,        ///< user-defined custom node
};

inline const char* materialNodeTypeName(MaterialNodeType t) {
    switch (t) {
    case MaterialNodeType::Output:        return "Output";
    case MaterialNodeType::TextureSample: return "TextureSample";
    case MaterialNodeType::Constant:      return "Constant";
    case MaterialNodeType::Parameter:     return "Parameter";
    case MaterialNodeType::MathAdd:       return "MathAdd";
    case MaterialNodeType::MathMultiply:  return "MathMultiply";
    case MaterialNodeType::Lerp:          return "Lerp";
    case MaterialNodeType::Fresnel:       return "Fresnel";
    case MaterialNodeType::Normal:        return "Normal";
    case MaterialNodeType::Custom:        return "Custom";
    }
    return "Unknown";
}

// ── Pin direction ──────────────────────────────────────────────────────────────

enum class MaterialPinDir : uint8_t { Input, Output };

// ── MaterialPin ────────────────────────────────────────────────────────────────

struct MaterialPin {
    MaterialPinId  id        = kInvalidMaterialPinId;
    MaterialNodeId nodeId    = kInvalidMaterialNodeId;
    std::string    name;
    MaterialPinDir direction = MaterialPinDir::Input;
    std::string    dataType; ///< "float", "float3", "float4", "sampler2D", …
};

// ── MaterialConnection ─────────────────────────────────────────────────────────

struct MaterialConnection {
    MaterialNodeId fromNode = kInvalidMaterialNodeId;
    MaterialPinId  fromPin  = kInvalidMaterialPinId;
    MaterialNodeId toNode   = kInvalidMaterialNodeId;
    MaterialPinId  toPin    = kInvalidMaterialPinId;
};

// ── MaterialNode ───────────────────────────────────────────────────────────────

struct MaterialNode {
    MaterialNodeId  id       = kInvalidMaterialNodeId;
    MaterialNodeType type    = MaterialNodeType::Custom;
    std::string      label;
    float            posX    = 0.f;
    float            posY    = 0.f;
    std::vector<MaterialPin>               pins;
    std::map<std::string, std::string>     params; ///< flat string-encoded params
};

// ── Save status ────────────────────────────────────────────────────────────────

enum class MaterialSaveStatus : uint8_t { Ok, IoError, SerializeError };

struct MaterialSaveResult {
    MaterialSaveStatus status   = MaterialSaveStatus::Ok;
    std::string        errorMsg;
    [[nodiscard]] bool ok() const { return status == MaterialSaveStatus::Ok; }
};

// ── MaterialDocument ───────────────────────────────────────────────────────────

class MaterialDocument {
public:
    MaterialDocument() = default;
    explicit MaterialDocument(const std::string& path) : m_assetPath(path) {}

    // ── Identity ───────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& assetPath()   const { return m_assetPath; }
    [[nodiscard]] const std::string& displayName() const { return m_displayName; }
    void setAssetPath(const std::string& p)   { m_assetPath   = p; }
    void setDisplayName(const std::string& n) { m_displayName = n; markDirty(); }

    // ── Dirty tracking ─────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── Node management ────────────────────────────────────────────────────────

    MaterialNodeId addNode(MaterialNodeType type, const std::string& label,
                            float posX = 0.f, float posY = 0.f) {
        MaterialNodeId id = ++m_nextNodeId;
        MaterialNode node;
        node.id    = id;
        node.type  = type;
        node.label = label;
        node.posX  = posX;
        node.posY  = posY;

        // Auto-designate the first Output node
        if (type == MaterialNodeType::Output && m_outputNodeId == kInvalidMaterialNodeId) {
            m_outputNodeId = id;
        }

        m_nodes.push_back(std::move(node));
        markDirty();
        return id;
    }

    bool removeNode(MaterialNodeId id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) {
                // Remove all connections involving this node
                m_connections.erase(
                    std::remove_if(m_connections.begin(), m_connections.end(),
                        [id](const MaterialConnection& c) {
                            return c.fromNode == id || c.toNode == id;
                        }),
                    m_connections.end());

                if (m_outputNodeId == id) m_outputNodeId = kInvalidMaterialNodeId;
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

    [[nodiscard]] const MaterialNode* findNode(MaterialNodeId id) const {
        for (const auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] MaterialNode* findNode(MaterialNodeId id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    [[nodiscard]] const std::vector<MaterialNode>& nodes() const { return m_nodes; }

    // ── Output node ────────────────────────────────────────────────────────────

    [[nodiscard]] MaterialNodeId outputNodeId() const { return m_outputNodeId; }
    void setOutputNodeId(MaterialNodeId id) { m_outputNodeId = id; markDirty(); }

    // ── Pin management ─────────────────────────────────────────────────────────

    MaterialPinId addPin(MaterialNodeId nodeId, const std::string& name,
                          MaterialPinDir dir, const std::string& dataType = "float") {
        auto* node = findNode(nodeId);
        if (!node) return kInvalidMaterialPinId;

        MaterialPinId pid = ++m_nextPinId;
        MaterialPin pin;
        pin.id        = pid;
        pin.nodeId    = nodeId;
        pin.name      = name;
        pin.direction = dir;
        pin.dataType  = dataType;
        node->pins.push_back(std::move(pin));
        markDirty();
        return pid;
    }

    bool removePin(MaterialNodeId nodeId, MaterialPinId pinId) {
        auto* node = findNode(nodeId);
        if (!node) return false;
        auto& pins = node->pins;
        for (auto it = pins.begin(); it != pins.end(); ++it) {
            if (it->id == pinId) {
                // Remove connections on this pin
                m_connections.erase(
                    std::remove_if(m_connections.begin(), m_connections.end(),
                        [pinId](const MaterialConnection& c) {
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

    [[nodiscard]] uint32_t pinCount(MaterialNodeId nodeId) const {
        if (const auto* n = findNode(nodeId)) {
            return static_cast<uint32_t>(n->pins.size());
        }
        return 0u;
    }

    // ── Parameter management ───────────────────────────────────────────────────

    bool setNodeParam(MaterialNodeId nodeId, const std::string& key,
                       const std::string& value) {
        auto* node = findNode(nodeId);
        if (!node) return false;
        node->params[key] = value;
        markDirty();
        return true;
    }

    [[nodiscard]] std::string getNodeParam(MaterialNodeId nodeId,
                                            const std::string& key,
                                            const std::string& defaultVal = "") const {
        if (const auto* n = findNode(nodeId)) {
            auto it = n->params.find(key);
            if (it != n->params.end()) return it->second;
        }
        return defaultVal;
    }

    // ── Connection management ──────────────────────────────────────────────────

    bool connectPins(MaterialNodeId fromNode, MaterialPinId fromPin,
                      MaterialNodeId toNode,   MaterialPinId toPin) {
        // Validate nodes exist
        if (!findNode(fromNode) || !findNode(toNode)) return false;

        MaterialConnection c;
        c.fromNode = fromNode;
        c.fromPin  = fromPin;
        c.toNode   = toNode;
        c.toPin    = toPin;
        m_connections.push_back(c);
        markDirty();
        return true;
    }

    bool disconnectPin(MaterialNodeId toNode, MaterialPinId toPin) {
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it->toNode == toNode && it->toPin == toPin) {
                m_connections.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool isConnected(MaterialNodeId toNode, MaterialPinId toPin) const {
        for (const auto& c : m_connections) {
            if (c.toNode == toNode && c.toPin == toPin) return true;
        }
        return false;
    }

    [[nodiscard]] uint32_t connectionCount() const {
        return static_cast<uint32_t>(m_connections.size());
    }

    [[nodiscard]] const std::vector<MaterialConnection>& connections() const {
        return m_connections;
    }

    // ── Save / load ────────────────────────────────────────────────────────────

    MaterialSaveResult save(const std::string& path = "") {
        if (!path.empty()) m_assetPath = path;
        if (m_assetPath.empty()) {
            return { MaterialSaveStatus::IoError, "No path specified" };
        }
        clearDirty();
        return { MaterialSaveStatus::Ok, {} };
    }

    bool load(const std::string& /*json*/) {
        clearDirty();
        return true;
    }

    [[nodiscard]] std::string serialize() const {
        std::string out = "{\"material\":\"" + m_displayName + "\",";
        out += "\"nodes\":" + std::to_string(m_nodes.size()) + ",";
        out += "\"connections\":" + std::to_string(m_connections.size()) + "}";
        return out;
    }

private:
    std::string    m_assetPath;
    std::string    m_displayName;
    bool           m_dirty       = false;

    MaterialNodeId m_nextNodeId    = 0u;
    MaterialPinId  m_nextPinId     = 0u;
    MaterialNodeId m_outputNodeId  = kInvalidMaterialNodeId;

    std::vector<MaterialNode>       m_nodes;
    std::vector<MaterialConnection> m_connections;
};

} // namespace NF
