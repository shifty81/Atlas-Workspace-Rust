#pragma once
// NF::Editor — VFX graph editor v1: VFX node and link management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Vfxv1NodeType  : uint8_t { Emitter, Force, Collision, ColorOverLife, SizeOverLife, Output };
enum class Vfxv1NodeState : uint8_t { Disabled, Enabled, Preview, Locked };

inline const char* vfxv1NodeTypeName(Vfxv1NodeType t) {
    switch (t) {
        case Vfxv1NodeType::Emitter:      return "Emitter";
        case Vfxv1NodeType::Force:        return "Force";
        case Vfxv1NodeType::Collision:    return "Collision";
        case Vfxv1NodeType::ColorOverLife:return "ColorOverLife";
        case Vfxv1NodeType::SizeOverLife: return "SizeOverLife";
        case Vfxv1NodeType::Output:       return "Output";
    }
    return "Unknown";
}

inline const char* vfxv1NodeStateName(Vfxv1NodeState s) {
    switch (s) {
        case Vfxv1NodeState::Disabled: return "Disabled";
        case Vfxv1NodeState::Enabled:  return "Enabled";
        case Vfxv1NodeState::Preview:  return "Preview";
        case Vfxv1NodeState::Locked:   return "Locked";
    }
    return "Unknown";
}

struct Vfxv1Node {
    uint64_t        id       = 0;
    std::string     name;
    Vfxv1NodeType   nodeType = Vfxv1NodeType::Emitter;
    Vfxv1NodeState  state    = Vfxv1NodeState::Disabled;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEnabled() const { return state == Vfxv1NodeState::Enabled; }
    [[nodiscard]] bool isLocked()  const { return state == Vfxv1NodeState::Locked; }
};

struct Vfxv1Link {
    uint64_t id       = 0;
    uint64_t fromNode = 0;
    uint64_t toNode   = 0;

    [[nodiscard]] bool isValid() const { return id != 0 && fromNode != 0 && toNode != 0; }
};

using Vfxv1ChangeCallback = std::function<void(uint64_t)>;

class VFXGraphEditorV1 {
public:
    static constexpr size_t MAX_NODES = 2048;
    static constexpr size_t MAX_LINKS = 8192;

    bool addNode(const Vfxv1Node& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        m_nodes.push_back(node);
        if (m_onChange) m_onChange(node.id);
        return true;
    }

    bool removeNode(uint64_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Vfxv1Node* findNode(uint64_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool addLink(const Vfxv1Link& link) {
        if (!link.isValid()) return false;
        for (const auto& l : m_links) if (l.id == link.id) return false;
        if (m_links.size() >= MAX_LINKS) return false;
        m_links.push_back(link);
        if (m_onChange) m_onChange(link.id);
        return true;
    }

    bool removeLink(uint64_t id) {
        for (auto it = m_links.begin(); it != m_links.end(); ++it) {
            if (it->id == id) { m_links.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t linkCount() const { return m_links.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByNodeType(Vfxv1NodeType type) const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.nodeType == type) ++c; return c;
    }

    void setOnChange(Vfxv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Vfxv1Node> m_nodes;
    std::vector<Vfxv1Link> m_links;
    Vfxv1ChangeCallback    m_onChange;
};

} // namespace NF
