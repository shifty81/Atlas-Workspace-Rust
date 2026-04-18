#pragma once
// NF::Editor — Scene tree editor v1: hierarchical entity/group/prefab tree management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class SteNodeType : uint8_t { Entity, Group, Prefab, Camera, Light, Volume };

inline const char* steNodeTypeName(SteNodeType t) {
    switch(t){
        case SteNodeType::Entity:  return "Entity";
        case SteNodeType::Group:   return "Group";
        case SteNodeType::Prefab:  return "Prefab";
        case SteNodeType::Camera:  return "Camera";
        case SteNodeType::Light:   return "Light";
        case SteNodeType::Volume:  return "Volume";
    }
    return "Unknown";
}

struct SteNode {
    uint32_t              id       = 0;
    SteNodeType           type     = SteNodeType::Entity;
    std::string           name;
    uint32_t              parentId = 0;
    std::vector<uint32_t> children;
    bool                  visible  = true;
    bool                  locked   = false;
    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRoot()   const { return parentId == 0; }
};

using SteSelectCallback = std::function<void(uint32_t)>;

class SceneTreeEditorV1 {
public:
    static constexpr size_t MAX_NODES = 1024;

    bool addNode(const SteNode& node) {
        if (!node.isValid()) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        m_nodes.push_back(node);
        if (node.parentId != 0) {
            SteNode* parent = findNode_(node.parentId);
            if (parent) parent->children.push_back(node.id);
        }
        return true;
    }

    bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) {
                if (it->parentId != 0) {
                    SteNode* parent = findNode_(it->parentId);
                    if (parent) {
                        auto& ch = parent->children;
                        auto cit = std::find(ch.begin(), ch.end(), id);
                        if (cit != ch.end()) ch.erase(cit);
                    }
                }
                m_nodes.erase(it);
                return true;
            }
        }
        return false;
    }

    bool reparent(uint32_t id, uint32_t newParent) {
        SteNode* node = findNode_(id);
        if (!node) return false;
        if (node->parentId != 0) {
            SteNode* oldParent = findNode_(node->parentId);
            if (oldParent) {
                auto& ch = oldParent->children;
                auto it = std::find(ch.begin(), ch.end(), id);
                if (it != ch.end()) ch.erase(it);
            }
        }
        node->parentId = newParent;
        if (newParent != 0) {
            SteNode* p = findNode_(newParent);
            if (p) p->children.push_back(id);
        }
        return true;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }

    const SteNode* findNode(const std::string& name) const {
        for (const auto& n : m_nodes) if (n.name == name) return &n;
        return nullptr;
    }

    std::vector<SteNode*> rootNodes() {
        std::vector<SteNode*> roots;
        for (auto& n : m_nodes) if (n.isRoot()) roots.push_back(&n);
        return roots;
    }

    bool setVisibility(uint32_t id, bool visible) {
        SteNode* n = findNode_(id);
        if (!n) return false;
        n->visible = visible;
        return true;
    }

    bool toggleLock(uint32_t id) {
        SteNode* n = findNode_(id);
        if (!n) return false;
        n->locked = !n->locked;
        return true;
    }

    void setOnSelect(SteSelectCallback cb) { m_onSelect = std::move(cb); }

    void selectNode(uint32_t id) { if (m_onSelect) m_onSelect(id); }

private:
    SteNode* findNode_(uint32_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    std::vector<SteNode> m_nodes;
    SteSelectCallback    m_onSelect;
};

} // namespace NF
