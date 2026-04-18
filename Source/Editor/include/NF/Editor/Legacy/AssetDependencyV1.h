#pragma once
// NF::Editor — Asset dependency v1: dependency graph node authoring and cycle detection
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

enum class Adv1DepType      : uint8_t { Hard, Soft, Optional, Circular };
enum class Adv1NodeStatus   : uint8_t { Clean, Dirty, Missing, Broken, Excluded };

inline const char* adv1DepTypeName(Adv1DepType t) {
    switch (t) {
        case Adv1DepType::Hard:     return "Hard";
        case Adv1DepType::Soft:     return "Soft";
        case Adv1DepType::Optional: return "Optional";
        case Adv1DepType::Circular: return "Circular";
    }
    return "Unknown";
}

inline const char* adv1NodeStatusName(Adv1NodeStatus s) {
    switch (s) {
        case Adv1NodeStatus::Clean:    return "Clean";
        case Adv1NodeStatus::Dirty:    return "Dirty";
        case Adv1NodeStatus::Missing:  return "Missing";
        case Adv1NodeStatus::Broken:   return "Broken";
        case Adv1NodeStatus::Excluded: return "Excluded";
    }
    return "Unknown";
}

struct Adv1Dep {
    uint64_t    targetId = 0;
    Adv1DepType type     = Adv1DepType::Hard;
};

struct Adv1Node {
    uint64_t              id = 0;
    std::string           assetPath;
    Adv1NodeStatus        status = Adv1NodeStatus::Clean;
    std::vector<Adv1Dep>  deps;

    [[nodiscard]] bool isValid()   const { return id != 0 && !assetPath.empty(); }
    [[nodiscard]] bool isClean()   const { return status == Adv1NodeStatus::Clean; }
    [[nodiscard]] bool isMissing() const { return status == Adv1NodeStatus::Missing; }

    bool addDep(const Adv1Dep& dep) {
        if (dep.targetId == 0 || dep.targetId == id) return false;
        for (const auto& d : deps) if (d.targetId == dep.targetId) return false;
        deps.push_back(dep);
        return true;
    }

    bool removeDep(uint64_t targetId) {
        for (auto it = deps.begin(); it != deps.end(); ++it) {
            if (it->targetId == targetId) { deps.erase(it); return true; }
        }
        return false;
    }
};

using Adv1ChangeCallback = std::function<void(uint64_t)>;

class AssetDependencyV1 {
public:
    static constexpr size_t MAX_NODES = 8192;

    bool addNode(const Adv1Node& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(uint64_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Adv1Node* findNode(uint64_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool addDep(uint64_t fromId, const Adv1Dep& dep) {
        auto* n = findNode(fromId);
        if (!n) return false;
        bool ok = n->addDep(dep);
        if (ok && m_onChange) m_onChange(fromId);
        return ok;
    }

    bool removeDep(uint64_t fromId, uint64_t toId) {
        auto* n = findNode(fromId);
        if (!n) return false;
        bool ok = n->removeDep(toId);
        if (ok && m_onChange) m_onChange(fromId);
        return ok;
    }

    bool setStatus(uint64_t id, Adv1NodeStatus status) {
        auto* n = findNode(id);
        if (!n) return false;
        n->status = status;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t nodeCount()   const { return m_nodes.size(); }
    [[nodiscard]] size_t cleanCount()  const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isClean())   ++c; return c;
    }
    [[nodiscard]] size_t missingCount()const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isMissing()) ++c; return c;
    }
    [[nodiscard]] size_t countByStatus(Adv1NodeStatus s) const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.status == s) ++c; return c;
    }

    void setOnChange(Adv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Adv1Node> m_nodes;
    Adv1ChangeCallback    m_onChange;
};

} // namespace NF
