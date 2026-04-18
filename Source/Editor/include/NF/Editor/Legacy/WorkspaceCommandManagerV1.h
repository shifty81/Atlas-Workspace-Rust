#pragma once
// NF::Editor — Workspace command manager v1: command registration, execution, and undo/redo
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wcmv1CommandState : uint8_t { Idle, Executing, Done, Failed, Undone, Redone };
enum class Wcmv1CommandScope : uint8_t { Global, Tool, Panel, Selection, Project };

inline const char* wcmv1CommandStateName(Wcmv1CommandState s) {
    switch (s) {
        case Wcmv1CommandState::Idle:      return "Idle";
        case Wcmv1CommandState::Executing: return "Executing";
        case Wcmv1CommandState::Done:      return "Done";
        case Wcmv1CommandState::Failed:    return "Failed";
        case Wcmv1CommandState::Undone:    return "Undone";
        case Wcmv1CommandState::Redone:    return "Redone";
    }
    return "Unknown";
}

inline const char* wcmv1CommandScopeName(Wcmv1CommandScope s) {
    switch (s) {
        case Wcmv1CommandScope::Global:    return "Global";
        case Wcmv1CommandScope::Tool:      return "Tool";
        case Wcmv1CommandScope::Panel:     return "Panel";
        case Wcmv1CommandScope::Selection: return "Selection";
        case Wcmv1CommandScope::Project:   return "Project";
    }
    return "Unknown";
}

struct Wcmv1Command {
    uint64_t           id          = 0;
    std::string        name;
    std::string        description;
    Wcmv1CommandScope  scope       = Wcmv1CommandScope::Global;
    Wcmv1CommandState  state       = Wcmv1CommandState::Idle;
    bool               isUndoable  = true;
    bool               isDirty     = false;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isDone()      const { return state == Wcmv1CommandState::Done; }
    [[nodiscard]] bool hasFailed()   const { return state == Wcmv1CommandState::Failed; }
    [[nodiscard]] bool isUndone()    const { return state == Wcmv1CommandState::Undone; }
};

using Wcmv1ChangeCallback = std::function<void(uint64_t)>;

class WorkspaceCommandManagerV1 {
public:
    static constexpr size_t MAX_COMMANDS   = 512;
    static constexpr size_t MAX_UNDO_STACK = 64;

    bool registerCommand(const Wcmv1Command& cmd) {
        if (!cmd.isValid()) return false;
        for (const auto& c : m_commands) if (c.id == cmd.id) return false;
        if (m_commands.size() >= MAX_COMMANDS) return false;
        m_commands.push_back(cmd);
        return true;
    }

    bool unregisterCommand(uint64_t id) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->id == id) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Wcmv1Command* findCommand(uint64_t id) {
        for (auto& c : m_commands) if (c.id == id) return &c;
        return nullptr;
    }

    bool execute(uint64_t id) {
        auto* c = findCommand(id);
        if (!c) return false;
        c->state = Wcmv1CommandState::Done;
        c->isDirty = true;
        if (c->isUndoable) {
            if (m_undoStack.size() >= MAX_UNDO_STACK) m_undoStack.erase(m_undoStack.begin());
            m_undoStack.push_back(id);
            m_redoStack.clear();
        }
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool undo() {
        if (m_undoStack.empty()) return false;
        uint64_t id = m_undoStack.back();
        m_undoStack.pop_back();
        auto* c = findCommand(id);
        if (c) { c->state = Wcmv1CommandState::Undone; c->isDirty = true; if (m_onChange) m_onChange(id); }
        m_redoStack.push_back(id);
        return true;
    }

    bool redo() {
        if (m_redoStack.empty()) return false;
        uint64_t id = m_redoStack.back();
        m_redoStack.pop_back();
        auto* c = findCommand(id);
        if (c) { c->state = Wcmv1CommandState::Redone; c->isDirty = true; if (m_onChange) m_onChange(id); }
        m_undoStack.push_back(id);
        return true;
    }

    bool setState(uint64_t id, Wcmv1CommandState state) {
        auto* c = findCommand(id);
        if (!c) return false;
        c->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t commandCount()  const { return m_commands.size(); }
    [[nodiscard]] size_t undoDepth()     const { return m_undoStack.size(); }
    [[nodiscard]] size_t redoDepth()     const { return m_redoStack.size(); }
    [[nodiscard]] bool   canUndo()       const { return !m_undoStack.empty(); }
    [[nodiscard]] bool   canRedo()       const { return !m_redoStack.empty(); }
    [[nodiscard]] size_t doneCount()     const {
        size_t c = 0; for (const auto& cmd : m_commands) if (cmd.isDone())    ++c; return c;
    }
    [[nodiscard]] size_t failedCount()   const {
        size_t c = 0; for (const auto& cmd : m_commands) if (cmd.hasFailed()) ++c; return c;
    }
    [[nodiscard]] size_t countByScope(Wcmv1CommandScope scope) const {
        size_t c = 0; for (const auto& cmd : m_commands) if (cmd.scope == scope) ++c; return c;
    }

    void setOnChange(Wcmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wcmv1Command> m_commands;
    std::vector<uint64_t>     m_undoStack;
    std::vector<uint64_t>     m_redoStack;
    Wcmv1ChangeCallback       m_onChange;
};

} // namespace NF
