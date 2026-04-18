#pragma once
// NF::Editor — AI behavior tree editor
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

enum class BTNodeType : uint8_t {
    Selector, Sequence, Parallel, Decorator, Task, Condition, Root
};

inline const char* btNodeTypeName(BTNodeType t) {
    switch (t) {
        case BTNodeType::Selector:  return "Selector";
        case BTNodeType::Sequence:  return "Sequence";
        case BTNodeType::Parallel:  return "Parallel";
        case BTNodeType::Decorator: return "Decorator";
        case BTNodeType::Task:      return "Task";
        case BTNodeType::Condition: return "Condition";
        case BTNodeType::Root:      return "Root";
    }
    return "Unknown";
}

enum class BTNodeStatus : uint8_t {
    Idle, Running, Success, Failure, Aborted
};

inline const char* btNodeStatusName(BTNodeStatus s) {
    switch (s) {
        case BTNodeStatus::Idle:    return "Idle";
        case BTNodeStatus::Running: return "Running";
        case BTNodeStatus::Success: return "Success";
        case BTNodeStatus::Failure: return "Failure";
        case BTNodeStatus::Aborted: return "Aborted";
    }
    return "Unknown";
}

enum class BTAbortMode : uint8_t {
    None, Self, LowerPriority, Both
};

inline const char* btAbortModeName(BTAbortMode m) {
    switch (m) {
        case BTAbortMode::None:          return "None";
        case BTAbortMode::Self:          return "Self";
        case BTAbortMode::LowerPriority: return "LowerPriority";
        case BTAbortMode::Both:          return "Both";
    }
    return "Unknown";
}

class BTEditorNode {
public:
    explicit BTEditorNode(const std::string& name, BTNodeType type)
        : m_name(name), m_type(type) {}

    void setStatus(BTNodeStatus s)  { m_status    = s; }
    void setAbortMode(BTAbortMode m){ m_abortMode = m; }
    void setEnabled(bool v)         { m_enabled   = v; }
    void setChildCount(uint8_t c)   { m_childCount = c; }
    void setBreakpoint(bool v)      { m_breakpoint = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] BTNodeType         type()       const { return m_type;       }
    [[nodiscard]] BTNodeStatus       status()     const { return m_status;     }
    [[nodiscard]] BTAbortMode        abortMode()  const { return m_abortMode;  }
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;    }
    [[nodiscard]] uint8_t            childCount()   const { return m_childCount; }
    [[nodiscard]] bool               hasBreakpoint()const { return m_breakpoint; }

    [[nodiscard]] bool isRunning() const { return m_status == BTNodeStatus::Running; }
    [[nodiscard]] bool isLeaf()    const { return m_childCount == 0; }

private:
    std::string   m_name;
    BTNodeType    m_type;
    BTNodeStatus  m_status     = BTNodeStatus::Idle;
    BTAbortMode   m_abortMode  = BTAbortMode::None;
    uint8_t       m_childCount = 0;
    bool          m_enabled    = true;
    bool          m_breakpoint = false;
};

class AIBehaviorEditor {
public:
    static constexpr size_t MAX_NODES = 256;

    [[nodiscard]] bool addNode(const BTEditorNode& node) {
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

    [[nodiscard]] BTEditorNode* findNode(const std::string& name) {
        for (auto& n : m_nodes) if (n.name() == name) return &n;
        return nullptr;
    }

    [[nodiscard]] bool selectNode(const std::string& name) {
        for (auto& n : m_nodes) if (n.name() == name) { m_selectedNode = name; return true; }
        return false;
    }

    void setSimulating(bool v)              { m_simulating  = v; }
    void setTreeName(const std::string& n)  { m_treeName    = n; }

    [[nodiscard]] const std::string& selectedNode() const { return m_selectedNode; }
    [[nodiscard]] const std::string& treeName()     const { return m_treeName;     }
    [[nodiscard]] bool               isSimulating() const { return m_simulating;   }
    [[nodiscard]] size_t             nodeCount()    const { return m_nodes.size(); }

    [[nodiscard]] size_t runningCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.isRunning()) ++c; return c;
    }
    [[nodiscard]] size_t leafCount() const {
        size_t c = 0; for (auto& n : m_nodes) if (n.isLeaf()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(BTNodeType t) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByStatus(BTNodeStatus s) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.status() == s) ++c; return c;
    }

private:
    std::vector<BTEditorNode> m_nodes;
    std::string               m_selectedNode;
    std::string               m_treeName;
    bool                      m_simulating = false;
};

} // namespace NF
