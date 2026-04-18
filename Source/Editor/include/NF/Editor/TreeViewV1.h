#pragma once
// NF::Editor — Tree view v1: hierarchical node model for inspector/scene-hierarchy panels
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Tree Node Flags ───────────────────────────────────────────────

enum class TreeNodeFlag : uint8_t {
    None        = 0,
    Expanded    = 1 << 0,
    Selected    = 1 << 1,
    Disabled    = 1 << 2,
    Highlighted = 1 << 3,
    Leaf        = 1 << 4,  // no children allowed
};

inline TreeNodeFlag operator|(TreeNodeFlag a, TreeNodeFlag b) {
    return static_cast<TreeNodeFlag>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline bool hasFlag(TreeNodeFlag flags, TreeNodeFlag flag) {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

// ── Tree Node ─────────────────────────────────────────────────────

struct TreeNode {
    uint32_t             id        = 0;
    std::string          label;
    std::string          iconId;   // icon registry id
    TreeNodeFlag         flags     = TreeNodeFlag::None;
    uint32_t             parentId  = 0;  // 0 = root level
    std::vector<uint32_t> childIds;
    void*                userData  = nullptr;  // external payload

    [[nodiscard]] bool isValid()    const { return id != 0 && !label.empty(); }
    [[nodiscard]] bool isExpanded() const { return hasFlag(flags, TreeNodeFlag::Expanded);    }
    [[nodiscard]] bool isSelected() const { return hasFlag(flags, TreeNodeFlag::Selected);    }
    [[nodiscard]] bool isDisabled() const { return hasFlag(flags, TreeNodeFlag::Disabled);    }
    [[nodiscard]] bool isLeaf()     const { return hasFlag(flags, TreeNodeFlag::Leaf);        }
    [[nodiscard]] bool isRoot()     const { return parentId == 0;                             }
    [[nodiscard]] size_t childCount() const { return childIds.size(); }

    void setFlag(TreeNodeFlag f)   { flags = flags | f; }
    void clearFlag(TreeNodeFlag f) {
        flags = static_cast<TreeNodeFlag>(
            static_cast<uint8_t>(flags) & ~static_cast<uint8_t>(f));
    }
};

// ── Tree View V1 ──────────────────────────────────────────────────

using TreeSelectCallback  = std::function<void(uint32_t nodeId)>;
using TreeExpandCallback  = std::function<void(uint32_t nodeId, bool expanded)>;

class TreeViewV1 {
public:
    static constexpr size_t MAX_NODES = 2048;

    bool addNode(const TreeNode& node) {
        if (!node.isValid()) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;

        m_nodes.push_back(node);

        // Register as child of parent
        if (node.parentId != 0) {
            auto* parent = findNodeMut(node.parentId);
            if (parent) {
                parent->childIds.push_back(node.id);
            }
        }
        return true;
    }

    bool removeNode(uint32_t id) {
        auto* node = findNodeMut(id);
        if (!node) return false;

        // Remove from parent's child list
        if (node->parentId != 0) {
            auto* parent = findNodeMut(node->parentId);
            if (parent) {
                parent->childIds.erase(
                    std::remove(parent->childIds.begin(), parent->childIds.end(), id),
                    parent->childIds.end());
            }
        }

        m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(),
            [id](const TreeNode& n) { return n.id == id; }), m_nodes.end());
        return true;
    }

    [[nodiscard]] const TreeNode* findNode(uint32_t id) const {
        for (const auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool select(uint32_t id, bool exclusive = true) {
        auto* node = findNodeMut(id);
        if (!node || node->isDisabled()) return false;

        if (exclusive) {
            for (auto& n : m_nodes)
                n.clearFlag(TreeNodeFlag::Selected);
            m_selectedIds.clear();
        }

        node->setFlag(TreeNodeFlag::Selected);
        m_selectedIds.push_back(id);
        ++m_selectCount;
        if (m_onSelect) m_onSelect(id);
        return true;
    }

    bool deselect(uint32_t id) {
        auto* node = findNodeMut(id);
        if (!node) return false;
        node->clearFlag(TreeNodeFlag::Selected);
        m_selectedIds.erase(
            std::remove(m_selectedIds.begin(), m_selectedIds.end(), id),
            m_selectedIds.end());
        return true;
    }

    void clearSelection() {
        for (auto& n : m_nodes) n.clearFlag(TreeNodeFlag::Selected);
        m_selectedIds.clear();
    }

    bool expand(uint32_t id, bool expanded = true) {
        auto* node = findNodeMut(id);
        if (!node || node->isLeaf()) return false;
        if (expanded) node->setFlag(TreeNodeFlag::Expanded);
        else node->clearFlag(TreeNodeFlag::Expanded);
        if (m_onExpand) m_onExpand(id, expanded);
        return true;
    }

    void expandAll() {
        for (auto& n : m_nodes)
            if (!n.isLeaf()) n.setFlag(TreeNodeFlag::Expanded);
    }

    void collapseAll() {
        for (auto& n : m_nodes)
            n.clearFlag(TreeNodeFlag::Expanded);
    }

    // Rename a node
    bool rename(uint32_t id, const std::string& newLabel) {
        auto* node = findNodeMut(id);
        if (!node || newLabel.empty()) return false;
        node->label = newLabel;
        return true;
    }

    // Get root-level nodes (parentId == 0)
    [[nodiscard]] std::vector<uint32_t> rootIds() const {
        std::vector<uint32_t> ids;
        for (const auto& n : m_nodes) if (n.isRoot()) ids.push_back(n.id);
        return ids;
    }

    // Depth-first traversal
    void traverseDepthFirst(uint32_t startId, const std::function<bool(const TreeNode&, int depth)>& visitor) const {
        const auto* node = findNode(startId);
        if (!node) return;
        traverseImpl(*node, visitor, 0);
    }

    // Filter nodes by label substring
    [[nodiscard]] std::vector<uint32_t> filterByLabel(const std::string& query) const {
        std::vector<uint32_t> result;
        for (const auto& n : m_nodes) {
            if (n.label.find(query) != std::string::npos)
                result.push_back(n.id);
        }
        return result;
    }

    void setOnSelect(TreeSelectCallback cb)  { m_onSelect  = std::move(cb); }
    void setOnExpand(TreeExpandCallback cb)  { m_onExpand  = std::move(cb); }

    [[nodiscard]] size_t nodeCount()     const { return m_nodes.size();     }
    [[nodiscard]] size_t selectedCount() const { return m_selectedIds.size(); }
    [[nodiscard]] size_t selectCount()   const { return m_selectCount;        }
    [[nodiscard]] const std::vector<uint32_t>& selectedIds() const { return m_selectedIds; }
    [[nodiscard]] const std::vector<TreeNode>& nodes()       const { return m_nodes; }

private:
    TreeNode* findNodeMut(uint32_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    void traverseImpl(const TreeNode& node,
                      const std::function<bool(const TreeNode&, int)>& visitor,
                      int depth) const {
        if (!visitor(node, depth)) return;
        for (uint32_t childId : node.childIds) {
            const auto* child = findNode(childId);
            if (child) traverseImpl(*child, visitor, depth + 1);
        }
    }

    std::vector<TreeNode> m_nodes;
    std::vector<uint32_t> m_selectedIds;
    TreeSelectCallback    m_onSelect;
    TreeExpandCallback    m_onExpand;
    size_t                m_selectCount = 0;
};

} // namespace NF
