#pragma once
// NF::Editor — VFX graph editor
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

enum class VFXNodeType : uint8_t {
    Emitter, Force, Collision, ColorOverLife, SizeOverLife,
    Velocity, Spawn, Solver, Noise, Output
};

inline const char* vfxNodeTypeName(VFXNodeType t) {
    switch (t) {
        case VFXNodeType::Emitter:      return "Emitter";
        case VFXNodeType::Force:        return "Force";
        case VFXNodeType::Collision:    return "Collision";
        case VFXNodeType::ColorOverLife:return "ColorOverLife";
        case VFXNodeType::SizeOverLife: return "SizeOverLife";
        case VFXNodeType::Velocity:     return "Velocity";
        case VFXNodeType::Spawn:        return "Spawn";
        case VFXNodeType::Solver:       return "Solver";
        case VFXNodeType::Noise:        return "Noise";
        case VFXNodeType::Output:       return "Output";
    }
    return "Unknown";
}

enum class VFXBlendMode : uint8_t {
    Additive, AlphaBlend, Multiply, Screen, Premultiplied, Opaque
};

inline const char* vfxBlendModeName(VFXBlendMode m) {
    switch (m) {
        case VFXBlendMode::Additive:      return "Additive";
        case VFXBlendMode::AlphaBlend:    return "AlphaBlend";
        case VFXBlendMode::Multiply:      return "Multiply";
        case VFXBlendMode::Screen:        return "Screen";
        case VFXBlendMode::Premultiplied: return "Premultiplied";
        case VFXBlendMode::Opaque:        return "Opaque";
    }
    return "Unknown";
}

enum class VFXSimulationSpace : uint8_t {
    World, Local, Custom
};

inline const char* vfxSimulationSpaceName(VFXSimulationSpace s) {
    switch (s) {
        case VFXSimulationSpace::World:  return "World";
        case VFXSimulationSpace::Local:  return "Local";
        case VFXSimulationSpace::Custom: return "Custom";
    }
    return "Unknown";
}

class VFXNode {
public:
    explicit VFXNode(uint32_t id, const std::string& name, VFXNodeType nodeType)
        : m_id(id), m_name(name), m_nodeType(nodeType) {}

    void setPosX(float v)              { m_posX = v; }
    void setPosY(float v)              { m_posY = v; }
    void setEnabled(bool v)            { m_isEnabled = v; }
    void setBlendMode(VFXBlendMode v)  { m_blendMode = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] VFXNodeType        nodeType()  const { return m_nodeType; }
    [[nodiscard]] float              posX()      const { return m_posX; }
    [[nodiscard]] float              posY()      const { return m_posY; }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }
    [[nodiscard]] VFXBlendMode       blendMode() const { return m_blendMode; }

private:
    uint32_t     m_id;
    std::string  m_name;
    VFXNodeType  m_nodeType  = VFXNodeType::Emitter;
    float        m_posX      = 0.0f;
    float        m_posY      = 0.0f;
    bool         m_isEnabled = true;
    VFXBlendMode m_blendMode = VFXBlendMode::AlphaBlend;
};

class VFXGraphEditor {
public:
    bool addNode(const VFXNode& node) {
        for (const auto& n : m_nodes)
            if (n.id() == node.id()) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id() == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] VFXNode* findNode(uint32_t id) {
        for (auto& n : m_nodes)
            if (n.id() == id) return &n;
        return nullptr;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }

    [[nodiscard]] size_t countByType(VFXNodeType t) const {
        size_t n = 0;
        for (const auto& nd : m_nodes) if (nd.nodeType() == t) ++n;
        return n;
    }

    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0;
        for (const auto& nd : m_nodes) if (nd.isEnabled()) ++n;
        return n;
    }

    void setSimulationSpace(VFXSimulationSpace v) { m_simulationSpace = v; }
    void setShowGrid(bool v)                       { m_isShowGrid = v; }
    void setShowMinimap(bool v)                    { m_isShowMinimap = v; }
    void setPreviewDuration(float v)               { m_previewDuration = v; }

    [[nodiscard]] VFXSimulationSpace simulationSpace()  const { return m_simulationSpace; }
    [[nodiscard]] bool               isShowGrid()       const { return m_isShowGrid; }
    [[nodiscard]] bool               isShowMinimap()    const { return m_isShowMinimap; }
    [[nodiscard]] float              previewDuration()  const { return m_previewDuration; }

private:
    std::vector<VFXNode>  m_nodes;
    VFXSimulationSpace m_simulationSpace = VFXSimulationSpace::World;
    bool               m_isShowGrid      = true;
    bool               m_isShowMinimap   = true;
    float              m_previewDuration = 5.0f;
};

} // namespace NF
