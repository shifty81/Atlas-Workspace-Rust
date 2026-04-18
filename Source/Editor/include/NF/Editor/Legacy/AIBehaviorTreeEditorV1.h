#pragma once
// NF::Editor — AI behavior tree editor v1: AI behavior tree node management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Abtv1NodeKind   : uint8_t { Root, Selector, Sequence, Parallel, Decorator, Leaf };
enum class Abtv1NodeStatus : uint8_t { Idle, Running, Success, Failure, Aborted };

inline const char* abtv1NodeKindName(Abtv1NodeKind k) {
    switch (k) {
        case Abtv1NodeKind::Root:      return "Root";
        case Abtv1NodeKind::Selector:  return "Selector";
        case Abtv1NodeKind::Sequence:  return "Sequence";
        case Abtv1NodeKind::Parallel:  return "Parallel";
        case Abtv1NodeKind::Decorator: return "Decorator";
        case Abtv1NodeKind::Leaf:      return "Leaf";
    }
    return "Unknown";
}

inline const char* abtv1NodeStatusName(Abtv1NodeStatus s) {
    switch (s) {
        case Abtv1NodeStatus::Idle:    return "Idle";
        case Abtv1NodeStatus::Running: return "Running";
        case Abtv1NodeStatus::Success: return "Success";
        case Abtv1NodeStatus::Failure: return "Failure";
        case Abtv1NodeStatus::Aborted: return "Aborted";
    }
    return "Unknown";
}

struct Abtv1TreeNode {
    uint64_t        id     = 0;
    std::string     name;
    Abtv1NodeKind   kind   = Abtv1NodeKind::Leaf;
    Abtv1NodeStatus status = Abtv1NodeStatus::Idle;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRunning() const { return status == Abtv1NodeStatus::Running; }
    [[nodiscard]] bool isSuccess() const { return status == Abtv1NodeStatus::Success; }
};

struct Abtv1Connection {
    uint64_t id       = 0;
    uint64_t parentId = 0;
    uint64_t childId  = 0;

    [[nodiscard]] bool isValid() const { return id != 0 && parentId != 0 && childId != 0; }
};

using Abtv1ChangeCallback = std::function<void(uint64_t)>;

class AIBehaviorTreeEditorV1 {
public:
    static constexpr size_t MAX_NODES       = 4096;
    static constexpr size_t MAX_CONNECTIONS = 8192;

    bool addNode(const Abtv1TreeNode& node) {
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

    [[nodiscard]] Abtv1TreeNode* findNode(uint64_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool addConnection(const Abtv1Connection& conn) {
        if (!conn.isValid()) return false;
        for (const auto& c : m_connections) if (c.id == conn.id) return false;
        if (m_connections.size() >= MAX_CONNECTIONS) return false;
        m_connections.push_back(conn);
        if (m_onChange) m_onChange(conn.id);
        return true;
    }

    bool removeConnection(uint64_t id) {
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            if (it->id == id) { m_connections.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t nodeCount()       const { return m_nodes.size(); }
    [[nodiscard]] size_t connectionCount() const { return m_connections.size(); }

    [[nodiscard]] size_t runningCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isRunning()) ++c; return c;
    }
    [[nodiscard]] size_t successCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isSuccess()) ++c; return c;
    }
    [[nodiscard]] size_t countByNodeKind(Abtv1NodeKind kind) const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.kind == kind) ++c; return c;
    }

    void setOnChange(Abtv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Abtv1TreeNode>   m_nodes;
    std::vector<Abtv1Connection> m_connections;
    Abtv1ChangeCallback          m_onChange;
};

} // namespace NF
