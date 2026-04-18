#pragma once
// NF::Editor — Dock tree serializer: persist and restore dock layout tree
#include "NF/Core/Core.h"
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ── Dock Split Orientation ────────────────────────────────────────

enum class DockSplitOrientation : uint8_t { Horizontal, Vertical };

inline const char* dockSplitOrientationName(DockSplitOrientation o) {
    return o == DockSplitOrientation::Horizontal ? "Horizontal" : "Vertical";
}

// ── Dock Node Kind ────────────────────────────────────────────────

enum class DockNodeKind : uint8_t { Split, TabStack };

inline const char* dockNodeKindName(DockNodeKind k) {
    return k == DockNodeKind::Split ? "Split" : "TabStack";
}

// ── Dock Tree Node ────────────────────────────────────────────────

struct DockTreeNode {
    uint32_t              id          = 0;
    DockNodeKind          kind        = DockNodeKind::TabStack;
    DockSplitOrientation  orientation = DockSplitOrientation::Horizontal;
    float                 splitRatio  = 0.5f;   // only for Split nodes
    uint32_t              firstChild  = 0;       // only for Split nodes
    uint32_t              secondChild = 0;       // only for Split nodes
    std::vector<std::string> panelIds;           // only for TabStack nodes
    int                   activeTab   = 0;       // index into panelIds

    [[nodiscard]] bool isValid()    const { return id != 0; }
    [[nodiscard]] bool isSplit()    const { return kind == DockNodeKind::Split; }
    [[nodiscard]] bool isTabStack() const { return kind == DockNodeKind::TabStack; }

    void addPanel(const std::string& panelId) {
        panelIds.push_back(panelId);
    }
    bool removePanel(const std::string& panelId) {
        for (auto it = panelIds.begin(); it != panelIds.end(); ++it) {
            if (*it == panelId) { panelIds.erase(it); return true; }
        }
        return false;
    }
};

// ── Dock Tree ─────────────────────────────────────────────────────

class DockTree {
public:
    bool addNode(const DockTreeNode& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        m_nodes.push_back(node);
        if (m_rootId == 0) m_rootId = node.id;
        return true;
    }

    bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const DockTreeNode* findNode(uint32_t id) const {
        for (const auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }
    DockTreeNode* findNodeMut(uint32_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    void setRootId(uint32_t id) { m_rootId = id; }
    [[nodiscard]] uint32_t    rootId()    const { return m_rootId;       }
    [[nodiscard]] size_t      nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] const std::vector<DockTreeNode>& nodes() const { return m_nodes; }
    void clear() { m_nodes.clear(); m_rootId = 0; }

private:
    std::vector<DockTreeNode> m_nodes;
    uint32_t m_rootId = 0;
};

// ── Dock Tree Serializer ──────────────────────────────────────────
// Text format:
//   root:<id>
//   node:<id>|split|<orientation>|<ratio>|<first>|<second>
//   node:<id>|tabs|<active>|<p0>,<p1>,...

class DockTreeSerializer {
public:
    static std::string serialize(const DockTree& tree) {
        std::ostringstream out;
        out << "root:" << tree.rootId() << "\n";
        for (const auto& n : tree.nodes()) {
            if (n.isSplit()) {
                out << "node:" << n.id
                    << "|split|"
                    << dockSplitOrientationName(n.orientation) << "|"
                    << n.splitRatio << "|"
                    << n.firstChild << "|"
                    << n.secondChild << "\n";
            } else {
                out << "node:" << n.id << "|tabs|" << n.activeTab;
                for (const auto& pid : n.panelIds) out << "|" << pid;
                out << "\n";
            }
        }
        return out.str();
    }

    static bool deserialize(const std::string& data, DockTree& out) {
        out.clear();
        if (data.empty()) return false;

        std::istringstream in(data);
        std::string line;
        bool foundRoot = false;

        while (std::getline(in, line)) {
            if (line.empty()) continue;

            if (line.substr(0, 5) == "root:") {
                try {
                    out.setRootId(static_cast<uint32_t>(std::stoul(line.substr(5))));
                    foundRoot = true;
                } catch (...) {}
            } else if (line.substr(0, 5) == "node:") {
                auto parts = splitOn(line.substr(5), '|');
                if (parts.size() < 2) continue;
                DockTreeNode node;
                try { node.id = static_cast<uint32_t>(std::stoul(parts[0])); } catch (...) { continue; }

                if (parts[1] == "split" && parts.size() >= 6) {
                    node.kind = DockNodeKind::Split;
                    node.orientation = (parts[2] == "Horizontal")
                        ? DockSplitOrientation::Horizontal
                        : DockSplitOrientation::Vertical;
                    try {
                        node.splitRatio  = std::stof(parts[3]);
                        node.firstChild  = static_cast<uint32_t>(std::stoul(parts[4]));
                        node.secondChild = static_cast<uint32_t>(std::stoul(parts[5]));
                    } catch (...) {}
                } else if (parts[1] == "tabs" && parts.size() >= 3) {
                    node.kind = DockNodeKind::TabStack;
                    try { node.activeTab = std::stoi(parts[2]); } catch (...) {}
                    for (size_t i = 3; i < parts.size(); ++i)
                        if (!parts[i].empty()) node.panelIds.push_back(parts[i]);
                }
                out.addNode(node);
            }
        }
        return foundRoot && out.nodeCount() > 0;
    }

private:
    static std::vector<std::string> splitOn(const std::string& s, char delim) {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : s) {
            if (c == delim) { parts.push_back(cur); cur.clear(); }
            else cur += c;
        }
        parts.push_back(cur);
        return parts;
    }
};

} // namespace NF
