#pragma once
// NF::Editor — Event graph for scripting visual events
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

enum class EventGraphNodeKind : uint8_t {
    Event, Action, Branch, Loop, Variable, Function, Comment, Macro
};

inline const char* eventGraphNodeKindName(EventGraphNodeKind k) {
    switch (k) {
        case EventGraphNodeKind::Event:    return "Event";
        case EventGraphNodeKind::Action:   return "Action";
        case EventGraphNodeKind::Branch:   return "Branch";
        case EventGraphNodeKind::Loop:     return "Loop";
        case EventGraphNodeKind::Variable: return "Variable";
        case EventGraphNodeKind::Function: return "Function";
        case EventGraphNodeKind::Comment:  return "Comment";
        case EventGraphNodeKind::Macro:    return "Macro";
    }
    return "Unknown";
}

enum class EventGraphPinType : uint8_t {
    Exec, Bool, Int, Float, String, Object, Struct, Array
};

inline const char* eventGraphPinTypeName(EventGraphPinType t) {
    switch (t) {
        case EventGraphPinType::Exec:   return "Exec";
        case EventGraphPinType::Bool:   return "Bool";
        case EventGraphPinType::Int:    return "Int";
        case EventGraphPinType::Float:  return "Float";
        case EventGraphPinType::String: return "String";
        case EventGraphPinType::Object: return "Object";
        case EventGraphPinType::Struct: return "Struct";
        case EventGraphPinType::Array:  return "Array";
    }
    return "Unknown";
}

enum class EventGraphCompileState : uint8_t {
    Unknown, Clean, Warning, Error, Compiling
};

inline const char* eventGraphCompileStateName(EventGraphCompileState s) {
    switch (s) {
        case EventGraphCompileState::Unknown:   return "Unknown";
        case EventGraphCompileState::Clean:     return "Clean";
        case EventGraphCompileState::Warning:   return "Warning";
        case EventGraphCompileState::Error:     return "Error";
        case EventGraphCompileState::Compiling: return "Compiling";
    }
    return "Unknown";
}

class EventGraphNode {
public:
    explicit EventGraphNode(const std::string& name, EventGraphNodeKind kind)
        : m_name(name), m_kind(kind) {}

    void setComment(const std::string& c) { m_comment  = c; }
    void setEnabled(bool v)               { m_enabled  = v; }
    void setBreakpoint(bool v)            { m_breakpoint = v; }
    void setPinCount(uint8_t n)           { m_pinCount = n; }

    [[nodiscard]] const std::string& name()        const { return m_name;       }
    [[nodiscard]] EventGraphNodeKind kind()        const { return m_kind;       }
    [[nodiscard]] const std::string& comment()     const { return m_comment;    }
    [[nodiscard]] bool               isEnabled()   const { return m_enabled;    }
    [[nodiscard]] bool               hasBreakpoint()const { return m_breakpoint; }
    [[nodiscard]] uint8_t            pinCount()    const { return m_pinCount;   }

private:
    std::string       m_name;
    EventGraphNodeKind m_kind;
    std::string       m_comment;
    uint8_t           m_pinCount   = 0;
    bool              m_enabled    = true;
    bool              m_breakpoint = false;
};

class EventGraph {
public:
    static constexpr size_t MAX_NODES = 512;

    [[nodiscard]] bool addNode(const EventGraphNode& node) {
        for (auto& n : m_nodes) if (n.name() == node.name()) return false;
        if (m_nodes.size() >= MAX_NODES) return false;
        m_nodes.push_back(node);
        return true;
    }

    [[nodiscard]] bool removeNode(const std::string& name) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->name() == name) {
                if (m_selectedNode == name) m_selectedNode.clear();
                m_nodes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] EventGraphNode* findNode(const std::string& name) {
        for (auto& n : m_nodes) if (n.name() == name) return &n;
        return nullptr;
    }

    [[nodiscard]] bool selectNode(const std::string& name) {
        for (auto& n : m_nodes) if (n.name() == name) { m_selectedNode = name; return true; }
        return false;
    }

    void setCompileState(EventGraphCompileState s) { m_compileState = s; }
    void setGraphName(const std::string& n)        { m_graphName    = n; }

    [[nodiscard]] const std::string&    selectedNode()  const { return m_selectedNode;  }
    [[nodiscard]] const std::string&    graphName()     const { return m_graphName;     }
    [[nodiscard]] EventGraphCompileState compileState() const { return m_compileState;  }
    [[nodiscard]] size_t                nodeCount()     const { return m_nodes.size();  }

    [[nodiscard]] bool isClean() const { return m_compileState == EventGraphCompileState::Clean; }
    [[nodiscard]] size_t countByKind(EventGraphNodeKind k) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.kind() == k) ++c; return c;
    }
    [[nodiscard]] size_t breakpointCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.hasBreakpoint()) ++c; return c;
    }
    [[nodiscard]] size_t disabledCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (!n.isEnabled()) ++c; return c;
    }

private:
    std::vector<EventGraphNode> m_nodes;
    std::string                 m_selectedNode;
    std::string                 m_graphName;
    EventGraphCompileState      m_compileState = EventGraphCompileState::Unknown;
};

} // namespace NF
