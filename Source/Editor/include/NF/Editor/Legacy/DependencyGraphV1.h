#pragma once
// NF::Editor — Dependency graph v1: directed dependency graph with cycle detection
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace NF {

struct DgNode {
    uint32_t              id      = 0;
    std::string           name;
    std::string           version;
    std::vector<uint32_t> dependsOn;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

class DependencyGraphV1 {
public:
    bool addNode(const DgNode& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    bool addDependency(uint32_t from, uint32_t to) {
        DgNode* n = findNode_(from);
        if (!n || !findNode_(to)) return false;
        for (auto d : n->dependsOn) if (d == to) return false;
        n->dependsOn.push_back(to);
        return true;
    }

    bool removeDependency(uint32_t from, uint32_t to) {
        DgNode* n = findNode_(from);
        if (!n) return false;
        auto it = std::find(n->dependsOn.begin(), n->dependsOn.end(), to);
        if (it == n->dependsOn.end()) return false;
        n->dependsOn.erase(it);
        return true;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }

    [[nodiscard]] std::vector<uint32_t> getDirectDeps(uint32_t id) const {
        for (const auto& n : m_nodes) if (n.id == id) return n.dependsOn;
        return {};
    }

    [[nodiscard]] bool hasCycle() const {
        std::vector<uint32_t> visited, stack;
        for (const auto& n : m_nodes) {
            if (dfsCycle(n.id, visited, stack)) return true;
        }
        return false;
    }

    [[nodiscard]] std::vector<uint32_t> topologicalOrder() const {
        std::vector<uint32_t> result;
        std::vector<uint32_t> visited;
        for (const auto& n : m_nodes)
            dfsTopoSort(n.id, visited, result);
        std::reverse(result.begin(), result.end());
        std::vector<uint32_t> deduped;
        for (auto id : result) {
            bool found = false;
            for (auto d : deduped) if (d == id) { found = true; break; }
            if (!found) deduped.push_back(id);
        }
        return deduped;
    }

private:
    DgNode* findNode_(uint32_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }
    const DgNode* findNode_(uint32_t id) const {
        for (const auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool dfsCycle(uint32_t id, std::vector<uint32_t>& visited,
                  std::vector<uint32_t>& stack) const {
        for (auto s : stack) if (s == id) return true;
        for (auto v : visited) if (v == id) return false;
        visited.push_back(id);
        stack.push_back(id);
        const DgNode* n = findNode_(id);
        if (n) {
            for (auto dep : n->dependsOn)
                if (dfsCycle(dep, visited, stack)) return true;
        }
        stack.pop_back();
        return false;
    }

    void dfsTopoSort(uint32_t id, std::vector<uint32_t>& visited,
                     std::vector<uint32_t>& result) const {
        for (auto v : visited) if (v == id) return;
        visited.push_back(id);
        const DgNode* n = findNode_(id);
        if (n) for (auto dep : n->dependsOn) dfsTopoSort(dep, visited, result);
        result.push_back(id);
    }

    std::vector<DgNode> m_nodes;
};

} // namespace NF
