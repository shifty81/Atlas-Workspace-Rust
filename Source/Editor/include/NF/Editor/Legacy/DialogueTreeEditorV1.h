#pragma once
// NF::Editor — Dialogue tree editor v1: dialogue node and edge management for branching dialogue
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Dtev1NodeType  : uint8_t { Entry, Line, Choice, Condition, Action, Exit };
enum class Dtev1NodeState : uint8_t { Draft, Review, Approved, Locked, Deprecated };

inline const char* dtev1NodeTypeName(Dtev1NodeType t) {
    switch (t) {
        case Dtev1NodeType::Entry:     return "Entry";
        case Dtev1NodeType::Line:      return "Line";
        case Dtev1NodeType::Choice:    return "Choice";
        case Dtev1NodeType::Condition: return "Condition";
        case Dtev1NodeType::Action:    return "Action";
        case Dtev1NodeType::Exit:      return "Exit";
    }
    return "Unknown";
}

inline const char* dtev1NodeStateName(Dtev1NodeState s) {
    switch (s) {
        case Dtev1NodeState::Draft:      return "Draft";
        case Dtev1NodeState::Review:     return "Review";
        case Dtev1NodeState::Approved:   return "Approved";
        case Dtev1NodeState::Locked:     return "Locked";
        case Dtev1NodeState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

struct Dtev1Node {
    uint64_t       id    = 0;
    std::string    name;
    Dtev1NodeType  type  = Dtev1NodeType::Line;
    Dtev1NodeState state = Dtev1NodeState::Draft;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isApproved()   const { return state == Dtev1NodeState::Approved; }
    [[nodiscard]] bool isLocked()     const { return state == Dtev1NodeState::Locked; }
};

struct Dtev1Edge {
    uint64_t    id       = 0;
    uint64_t    fromNode = 0;
    uint64_t    toNode   = 0;
    std::string label;

    [[nodiscard]] bool isValid() const { return id != 0 && fromNode != 0 && toNode != 0; }
};

using Dtev1ChangeCallback = std::function<void(uint64_t)>;

class DialogueTreeEditorV1 {
public:
    static constexpr size_t MAX_NODES = 4096;
    static constexpr size_t MAX_EDGES = 16384;

    bool addNode(const Dtev1Node& node) {
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

    [[nodiscard]] Dtev1Node* findNode(uint64_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool addEdge(const Dtev1Edge& edge) {
        if (!edge.isValid()) return false;
        for (const auto& e : m_edges) if (e.id == edge.id) return false;
        if (m_edges.size() >= MAX_EDGES) return false;
        m_edges.push_back(edge);
        if (m_onChange) m_onChange(edge.id);
        return true;
    }

    bool removeEdge(uint64_t id) {
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            if (it->id == id) { m_edges.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t nodeCount()  const { return m_nodes.size(); }
    [[nodiscard]] size_t edgeCount()  const { return m_edges.size(); }

    [[nodiscard]] size_t approvedCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isApproved()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByNodeType(Dtev1NodeType type) const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.type == type) ++c; return c;
    }

    void setOnChange(Dtev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Dtev1Node>    m_nodes;
    std::vector<Dtev1Edge>    m_edges;
    Dtev1ChangeCallback       m_onChange;
};

} // namespace NF
