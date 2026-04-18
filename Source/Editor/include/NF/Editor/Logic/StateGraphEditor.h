#pragma once
// NF::Editor — State graph editor (per-object FSM authoring)
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

enum class StateGraphNodeRole : uint8_t {
    Entry, State, AnyState, Exit, Conduit
};

inline const char* stateGraphNodeRoleName(StateGraphNodeRole r) {
    switch (r) {
        case StateGraphNodeRole::Entry:    return "Entry";
        case StateGraphNodeRole::State:    return "State";
        case StateGraphNodeRole::AnyState: return "AnyState";
        case StateGraphNodeRole::Exit:     return "Exit";
        case StateGraphNodeRole::Conduit:  return "Conduit";
    }
    return "Unknown";
}

enum class StateTransitionCondOp : uint8_t {
    Equal, NotEqual, GreaterThan, LessThan, GreaterEqual, LessEqual, Always
};

inline const char* stateTransitionCondOpName(StateTransitionCondOp op) {
    switch (op) {
        case StateTransitionCondOp::Equal:        return "Equal";
        case StateTransitionCondOp::NotEqual:     return "NotEqual";
        case StateTransitionCondOp::GreaterThan:  return "GreaterThan";
        case StateTransitionCondOp::LessThan:     return "LessThan";
        case StateTransitionCondOp::GreaterEqual: return "GreaterEqual";
        case StateTransitionCondOp::LessEqual:    return "LessEqual";
        case StateTransitionCondOp::Always:       return "Always";
    }
    return "Unknown";
}

enum class StateGraphCompileResult : uint8_t {
    Ok, HasErrors, HasWarnings, NotCompiled
};

inline const char* stateGraphCompileResultName(StateGraphCompileResult r) {
    switch (r) {
        case StateGraphCompileResult::Ok:          return "Ok";
        case StateGraphCompileResult::HasErrors:   return "HasErrors";
        case StateGraphCompileResult::HasWarnings: return "HasWarnings";
        case StateGraphCompileResult::NotCompiled: return "NotCompiled";
    }
    return "Unknown";
}

class StateGraphTransition {
public:
    explicit StateGraphTransition(uint32_t fromId, uint32_t toId, StateTransitionCondOp op)
        : m_fromId(fromId), m_toId(toId), m_op(op) {}

    void setEnabled(bool v)           { m_enabled     = v; }
    void setPriority(int p)           { m_priority    = p; }
    void setHasExitTime(bool v)       { m_hasExitTime = v; }

    [[nodiscard]] uint32_t               fromId()      const { return m_fromId;     }
    [[nodiscard]] uint32_t               toId()        const { return m_toId;       }
    [[nodiscard]] StateTransitionCondOp  op()          const { return m_op;         }
    [[nodiscard]] bool                   isEnabled()   const { return m_enabled;    }
    [[nodiscard]] int                    priority()    const { return m_priority;   }
    [[nodiscard]] bool                   hasExitTime() const { return m_hasExitTime;}

private:
    uint32_t               m_fromId;
    uint32_t               m_toId;
    StateTransitionCondOp  m_op;
    int                    m_priority    = 0;
    bool                   m_enabled     = true;
    bool                   m_hasExitTime = false;
};

class StateGraphNode {
public:
    explicit StateGraphNode(uint32_t id, StateGraphNodeRole role)
        : m_id(id), m_role(role) {}

    void setName(const std::string& n)  { m_name    = n; }
    void setEnabled(bool v)             { m_enabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;      }
    [[nodiscard]] StateGraphNodeRole role()      const { return m_role;    }
    [[nodiscard]] const std::string& name()      const { return m_name;    }
    [[nodiscard]] bool               isEnabled() const { return m_enabled; }

private:
    uint32_t          m_id;
    StateGraphNodeRole m_role;
    std::string       m_name;
    bool              m_enabled = true;
};

class StateGraphEditor {
public:
    static constexpr size_t MAX_NODES       = 256;
    static constexpr size_t MAX_TRANSITIONS = 1024;

    [[nodiscard]] bool addNode(const StateGraphNode& node) {
        for (auto& n : m_nodes) if (n.id() == node.id()) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        m_nodes.push_back(node); return true;
    }

    [[nodiscard]] bool addTransition(const StateGraphTransition& t) {
        if (m_transitions.size() >= MAX_TRANSITIONS) return false;
        m_transitions.push_back(t); return true;
    }

    [[nodiscard]] StateGraphNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }

    void setCompileResult(StateGraphCompileResult r) { m_compileResult = r; }

    [[nodiscard]] StateGraphCompileResult compileResult()   const { return m_compileResult; }
    [[nodiscard]] size_t                  nodeCount()       const { return m_nodes.size(); }
    [[nodiscard]] size_t                  transitionCount() const { return m_transitions.size(); }
    [[nodiscard]] bool                    isValid()         const {
        return m_compileResult == StateGraphCompileResult::Ok;
    }
    [[nodiscard]] size_t countByRole(StateGraphNodeRole r) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.role() == r) ++c; return c;
    }
    [[nodiscard]] size_t enabledTransitionCount() const {
        size_t c = 0; for (auto& t : m_transitions) if (t.isEnabled()) ++c; return c;
    }

private:
    std::vector<StateGraphNode>       m_nodes;
    std::vector<StateGraphTransition> m_transitions;
    StateGraphCompileResult           m_compileResult = StateGraphCompileResult::NotCompiled;
};

} // namespace NF
