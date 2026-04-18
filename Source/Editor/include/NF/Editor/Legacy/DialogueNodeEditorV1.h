#pragma once
// NF::Editor — Dialogue node editor v1: dialogue node and branch management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Dnev1NodeType  : uint8_t { NPC, Player, Narrator, Choice, Action, Condition };
enum class Dnev1NodeState : uint8_t { Inactive, Active, Visited, Locked };

inline const char* dnev1NodeTypeName(Dnev1NodeType t) {
    switch (t) {
        case Dnev1NodeType::NPC:       return "NPC";
        case Dnev1NodeType::Player:    return "Player";
        case Dnev1NodeType::Narrator:  return "Narrator";
        case Dnev1NodeType::Choice:    return "Choice";
        case Dnev1NodeType::Action:    return "Action";
        case Dnev1NodeType::Condition: return "Condition";
    }
    return "Unknown";
}

inline const char* dnev1NodeStateName(Dnev1NodeState s) {
    switch (s) {
        case Dnev1NodeState::Inactive: return "Inactive";
        case Dnev1NodeState::Active:   return "Active";
        case Dnev1NodeState::Visited:  return "Visited";
        case Dnev1NodeState::Locked:   return "Locked";
    }
    return "Unknown";
}

struct Dnev1DialogueNode {
    uint64_t       id       = 0;
    std::string    name;
    std::string    text;
    Dnev1NodeType  nodeType = Dnev1NodeType::NPC;
    Dnev1NodeState state    = Dnev1NodeState::Inactive;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()  const { return state == Dnev1NodeState::Active; }
    [[nodiscard]] bool isVisited() const { return state == Dnev1NodeState::Visited; }
    [[nodiscard]] bool isLocked()  const { return state == Dnev1NodeState::Locked; }
};

struct Dnev1DialogueBranch {
    uint64_t    id         = 0;
    uint64_t    fromNodeId = 0;
    uint64_t    toNodeId   = 0;
    std::string condition;

    [[nodiscard]] bool isValid() const { return id != 0 && fromNodeId != 0 && toNodeId != 0; }
};

using Dnev1ChangeCallback = std::function<void(uint64_t)>;

class DialogueNodeEditorV1 {
public:
    static constexpr size_t MAX_NODES    = 2048;
    static constexpr size_t MAX_BRANCHES = 8192;

    bool addNode(const Dnev1DialogueNode& node) {
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

    [[nodiscard]] Dnev1DialogueNode* findNode(uint64_t id) {
        for (auto& n : m_nodes) if (n.id == id) return &n;
        return nullptr;
    }

    bool addBranch(const Dnev1DialogueBranch& branch) {
        if (!branch.isValid()) return false;
        for (const auto& b : m_branches) if (b.id == branch.id) return false;
        if (m_branches.size() >= MAX_BRANCHES) return false;
        m_branches.push_back(branch);
        if (m_onChange) m_onChange(branch.id);
        return true;
    }

    bool removeBranch(uint64_t id) {
        for (auto it = m_branches.begin(); it != m_branches.end(); ++it) {
            if (it->id == id) { m_branches.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t nodeCount()   const { return m_nodes.size(); }
    [[nodiscard]] size_t branchCount() const { return m_branches.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t visitedCount() const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.isVisited()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Dnev1NodeType type) const {
        size_t c = 0; for (const auto& n : m_nodes) if (n.nodeType == type) ++c; return c;
    }
    [[nodiscard]] size_t branchesFromNode(uint64_t nodeId) const {
        size_t c = 0; for (const auto& b : m_branches) if (b.fromNodeId == nodeId) ++c; return c;
    }

    void setOnChange(Dnev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Dnev1DialogueNode>   m_nodes;
    std::vector<Dnev1DialogueBranch> m_branches;
    Dnev1ChangeCallback              m_onChange;
};

} // namespace NF
