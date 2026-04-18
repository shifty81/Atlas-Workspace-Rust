#pragma once
// NF::Editor — Asset dependency graph + tracker
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

enum class AssetDepType : uint8_t {
    Texture   = 0,
    Mesh      = 1,
    Shader    = 2,
    Script    = 3,
    Audio     = 4,
    Material  = 5,
    Animation = 6,
    Level     = 7,
};

inline const char* assetDepTypeName(AssetDepType t) {
    switch (t) {
        case AssetDepType::Texture:   return "Texture";
        case AssetDepType::Mesh:      return "Mesh";
        case AssetDepType::Shader:    return "Shader";
        case AssetDepType::Script:    return "Script";
        case AssetDepType::Audio:     return "Audio";
        case AssetDepType::Material:  return "Material";
        case AssetDepType::Animation: return "Animation";
        case AssetDepType::Level:     return "Level";
        default:                      return "Unknown";
    }
}

enum class AssetDepStatus : uint8_t {
    Unknown  = 0,
    Resolved = 1,
    Missing  = 2,
    Circular = 3,
};

struct AssetDepNode {
    std::string   assetId;
    std::string   assetPath;
    AssetDepType  type   = AssetDepType::Texture;
    AssetDepStatus status = AssetDepStatus::Unknown;

    std::vector<std::string> dependencies; // ids of direct deps

    [[nodiscard]] bool isResolved() const { return status == AssetDepStatus::Resolved; }
    [[nodiscard]] bool isMissing()  const { return status == AssetDepStatus::Missing;  }
    [[nodiscard]] bool isCircular() const { return status == AssetDepStatus::Circular; }

    bool addDependency(const std::string& depId) {
        if (depId == assetId) return false; // no self-dep
        for (auto& d : dependencies) if (d == depId) return false;
        dependencies.push_back(depId);
        return true;
    }

    [[nodiscard]] bool hasDependency(const std::string& depId) const {
        for (auto& d : dependencies) if (d == depId) return true;
        return false;
    }

    [[nodiscard]] size_t dependencyCount() const { return dependencies.size(); }
};

class AssetDepGraph {
public:
    static constexpr size_t MAX_NODES = 512;

    bool addNode(const AssetDepNode& node) {
        if (m_nodes.size() >= MAX_NODES) return false;
        for (auto& n : m_nodes) if (n.assetId == node.assetId) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(const std::string& assetId) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->assetId == assetId) {
                m_nodes.erase(it);
                // remove references to this node from other nodes
                for (auto& n : m_nodes) {
                    auto& deps = n.dependencies;
                    deps.erase(std::remove(deps.begin(), deps.end(), assetId), deps.end());
                }
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AssetDepNode* findNode(const std::string& assetId) {
        for (auto& n : m_nodes) if (n.assetId == assetId) return &n;
        return nullptr;
    }
    [[nodiscard]] const AssetDepNode* findNode(const std::string& assetId) const {
        for (auto& n : m_nodes) if (n.assetId == assetId) return &n;
        return nullptr;
    }

    bool addEdge(const std::string& sourceId, const std::string& depId) {
        AssetDepNode* src = findNode(sourceId);
        if (!src) return false;
        if (!findNode(depId)) return false;
        return src->addDependency(depId);
    }

    [[nodiscard]] bool hasEdge(const std::string& sourceId, const std::string& depId) const {
        const AssetDepNode* src = findNode(sourceId);
        if (!src) return false;
        return src->hasDependency(depId);
    }

    void resolveAll() {
        for (auto& n : m_nodes)
            if (n.status == AssetDepStatus::Unknown)
                n.status = AssetDepStatus::Resolved;
    }

    void detectCircular() {
        // Simple DFS cycle detection; mark involved nodes as Circular
        std::vector<std::string> visited;
        std::vector<std::string> stack;

        std::function<bool(const std::string&)> dfs = [&](const std::string& id) -> bool {
            visited.push_back(id);
            stack.push_back(id);
            const AssetDepNode* node = findNode(id);
            if (node) {
                for (auto& dep : node->dependencies) {
                    bool inStack = false;
                    for (auto& s : stack) if (s == dep) { inStack = true; break; }
                    if (inStack) {
                        // mark all nodes in the current stack as Circular
                        for (auto& s : stack) {
                            AssetDepNode* n = findNode(s);
                            if (n) n->status = AssetDepStatus::Circular;
                        }
                        AssetDepNode* n = findNode(dep);
                        if (n) n->status = AssetDepStatus::Circular;
                        stack.pop_back();
                        return true;
                    }
                    bool inVisited = false;
                    for (auto& v : visited) if (v == dep) { inVisited = true; break; }
                    if (!inVisited) dfs(dep);
                }
            }
            stack.pop_back();
            return false;
        };

        for (auto& n : m_nodes) {
            bool inVisited = false;
            for (auto& v : visited) if (v == n.assetId) { inVisited = true; break; }
            if (!inVisited) dfs(n.assetId);
        }
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }

    [[nodiscard]] size_t unresolvedCount() const {
        size_t c = 0;
        for (auto& n : m_nodes)
            if (n.status == AssetDepStatus::Unknown || n.status == AssetDepStatus::Missing)
                c++;
        return c;
    }

    [[nodiscard]] size_t totalEdgeCount() const {
        size_t c = 0;
        for (auto& n : m_nodes) c += n.dependencyCount();
        return c;
    }

    [[nodiscard]] const std::vector<AssetDepNode>& nodes() const { return m_nodes; }

private:
    std::vector<AssetDepNode> m_nodes;
};

class AssetDependencyTracker {
public:
    bool registerAsset(const std::string& assetId, const std::string& assetPath, AssetDepType type) {
        AssetDepNode node;
        node.assetId   = assetId;
        node.assetPath = assetPath;
        node.type      = type;
        node.status    = AssetDepStatus::Unknown;
        return m_graph.addNode(node);
    }

    bool unregisterAsset(const std::string& assetId) {
        return m_graph.removeNode(assetId);
    }

    bool addDependency(const std::string& sourceId, const std::string& depId) {
        return m_graph.addEdge(sourceId, depId);
    }

    [[nodiscard]] bool hasDependency(const std::string& sourceId, const std::string& depId) const {
        return m_graph.hasEdge(sourceId, depId);
    }

    void resolveAll() {
        m_graph.resolveAll();
    }

    void detectCircular() {
        m_graph.detectCircular();
    }

    [[nodiscard]] AssetDepNode* findAsset(const std::string& assetId) {
        return m_graph.findNode(assetId);
    }

    [[nodiscard]] size_t assetCount()        const { return m_graph.nodeCount(); }
    [[nodiscard]] size_t unresolvedCount()   const { return m_graph.unresolvedCount(); }
    [[nodiscard]] size_t totalDependencies() const { return m_graph.totalEdgeCount(); }

    [[nodiscard]] AssetDepGraph&       graph()       { return m_graph; }
    [[nodiscard]] const AssetDepGraph& graph() const { return m_graph; }

private:
    AssetDepGraph m_graph;
};

// ============================================================
// S18 — Build Configuration System
// ============================================================


} // namespace NF
