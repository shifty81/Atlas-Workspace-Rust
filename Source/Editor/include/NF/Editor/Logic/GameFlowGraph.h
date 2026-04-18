#pragma once
// NF::Editor — Game flow graph editor (high-level game loop state machine)
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

enum class GameFlowNodeKind : uint8_t {
    Start, End, GameState, Transition, Condition, SubFlow, Event, Delay
};

inline const char* gameFlowNodeKindName(GameFlowNodeKind k) {
    switch (k) {
        case GameFlowNodeKind::Start:      return "Start";
        case GameFlowNodeKind::End:        return "End";
        case GameFlowNodeKind::GameState:  return "GameState";
        case GameFlowNodeKind::Transition: return "Transition";
        case GameFlowNodeKind::Condition:  return "Condition";
        case GameFlowNodeKind::SubFlow:    return "SubFlow";
        case GameFlowNodeKind::Event:      return "Event";
        case GameFlowNodeKind::Delay:      return "Delay";
    }
    return "Unknown";
}

enum class GameFlowCompileState : uint8_t {
    Dirty, Compiling, Ok, Error
};

inline const char* gameFlowCompileStateName(GameFlowCompileState s) {
    switch (s) {
        case GameFlowCompileState::Dirty:     return "Dirty";
        case GameFlowCompileState::Compiling: return "Compiling";
        case GameFlowCompileState::Ok:        return "Ok";
        case GameFlowCompileState::Error:     return "Error";
    }
    return "Unknown";
}

enum class GameFlowPinDir : uint8_t {
    Input, Output
};

inline const char* gameFlowPinDirName(GameFlowPinDir d) {
    return d == GameFlowPinDir::Input ? "Input" : "Output";
}

class GameFlowNode {
public:
    explicit GameFlowNode(uint32_t id, GameFlowNodeKind kind)
        : m_id(id), m_kind(kind) {}

    void setLabel(const std::string& l)  { m_label    = l; }
    void setComment(const std::string& c){ m_comment   = c; }
    void setEnabled(bool v)              { m_enabled   = v; }
    void setBreakpoint(bool v)           { m_breakpoint = v; }

    [[nodiscard]] uint32_t          id()          const { return m_id;         }
    [[nodiscard]] GameFlowNodeKind  kind()        const { return m_kind;       }
    [[nodiscard]] const std::string& label()      const { return m_label;      }
    [[nodiscard]] const std::string& comment()    const { return m_comment;    }
    [[nodiscard]] bool               isEnabled()  const { return m_enabled;    }
    [[nodiscard]] bool               hasBreakpoint() const { return m_breakpoint; }

private:
    uint32_t         m_id;
    GameFlowNodeKind m_kind;
    std::string      m_label;
    std::string      m_comment;
    bool             m_enabled    = true;
    bool             m_breakpoint = false;
};

class GameFlowGraph {
public:
    explicit GameFlowGraph(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addNode(const GameFlowNode& node) {
        for (auto& n : m_nodes) if (n.id() == node.id()) return false;
        m_nodes.push_back(node); return true;
    }

    [[nodiscard]] bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id() == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] GameFlowNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }

    void setCompileState(GameFlowCompileState s) { m_compileState = s; }

    [[nodiscard]] const std::string&   name()         const { return m_name;         }
    [[nodiscard]] GameFlowCompileState compileState() const { return m_compileState; }
    [[nodiscard]] size_t               nodeCount()    const { return m_nodes.size(); }
    [[nodiscard]] bool                 isCompiled()   const {
        return m_compileState == GameFlowCompileState::Ok;
    }
    [[nodiscard]] size_t countByKind(GameFlowNodeKind k) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.kind() == k) ++c; return c;
    }
    [[nodiscard]] size_t breakpointCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.hasBreakpoint()) ++c; return c;
    }
    [[nodiscard]] size_t enabledNodeCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.isEnabled()) ++c; return c;
    }

private:
    std::string              m_name;
    std::vector<GameFlowNode> m_nodes;
    GameFlowCompileState     m_compileState = GameFlowCompileState::Dirty;
};

} // namespace NF
