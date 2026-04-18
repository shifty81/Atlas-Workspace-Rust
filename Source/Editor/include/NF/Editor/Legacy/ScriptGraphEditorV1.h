#pragma once
// NF::Editor — Script graph editor v1: script node and connection management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Sgev1NodeType  : uint8_t { Entry, Exit, Branch, Loop, FunctionCall, Variable, Event, Comment };
enum class Sgev1NodeState : uint8_t { Inactive, Active, Error, Disabled, Breakpoint };

inline const char* sgev1NodeTypeName(Sgev1NodeType t) {
    switch (t) {
        case Sgev1NodeType::Entry:        return "Entry";
        case Sgev1NodeType::Exit:         return "Exit";
        case Sgev1NodeType::Branch:       return "Branch";
        case Sgev1NodeType::Loop:         return "Loop";
        case Sgev1NodeType::FunctionCall: return "FunctionCall";
        case Sgev1NodeType::Variable:     return "Variable";
        case Sgev1NodeType::Event:        return "Event";
        case Sgev1NodeType::Comment:      return "Comment";
    }
    return "Unknown";
}

inline const char* sgev1NodeStateName(Sgev1NodeState s) {
    switch (s) {
        case Sgev1NodeState::Inactive:    return "Inactive";
        case Sgev1NodeState::Active:      return "Active";
        case Sgev1NodeState::Error:       return "Error";
        case Sgev1NodeState::Disabled:    return "Disabled";
        case Sgev1NodeState::Breakpoint:  return "Breakpoint";
    }
    return "Unknown";
}

struct Sgev1Node {
    uint64_t       id        = 0;
    std::string    name;
    Sgev1NodeType  nodeType  = Sgev1NodeType::FunctionCall;
    Sgev1NodeState state     = Sgev1NodeState::Inactive;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()     const { return state == Sgev1NodeState::Active; }
    [[nodiscard]] bool isError()      const { return state == Sgev1NodeState::Error; }
    [[nodiscard]] bool isBreakpoint() const { return state == Sgev1NodeState::Breakpoint; }
};

struct Sgev1Connection {
    uint64_t id       = 0;
    uint64_t fromNode = 0;
    uint64_t toNode   = 0;

    [[nodiscard]] bool isValid() const { return id != 0 && fromNode != 0 && toNode != 0; }
};

using Sgev1ChangeCallback = std::function<void(uint64_t)>;

class ScriptGraphEditorV1 {
public:
    static constexpr size_t MAX_NODES       = 1024;
    static constexpr size_t MAX_CONNECTIONS = 4096;

    bool addNode(const Sgev1Node& node) {
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

    [[nodiscard]] Sgev1Node* findNode(uint64_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool addConnection(const Sgev1Connection& conn) {
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

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t errorCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isError()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Sgev1NodeType type) const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.nodeType == type) ++c; return c;
    }

    void setOnChange(Sgev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Sgev1Node>       m_nodes;
    std::vector<Sgev1Connection> m_connections;
    Sgev1ChangeCallback          m_onChange;
};

} // namespace NF
