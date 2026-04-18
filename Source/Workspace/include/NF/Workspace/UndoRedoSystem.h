#pragma once
// NF::Editor — Undo group + undo/redo system
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

enum class UndoActionType : uint8_t { Create, Delete, Move, Resize, Rename, Modify, Group, Ungroup };

inline const char* undoActionTypeName(UndoActionType t) {
    switch (t) {
        case UndoActionType::Create:   return "Create";
        case UndoActionType::Delete:   return "Delete";
        case UndoActionType::Move:     return "Move";
        case UndoActionType::Resize:   return "Resize";
        case UndoActionType::Rename:   return "Rename";
        case UndoActionType::Modify:   return "Modify";
        case UndoActionType::Group:    return "Group";
        case UndoActionType::Ungroup:  return "Ungroup";
        default:                       return "Unknown";
    }
}

enum class UndoActionState : uint8_t { Pending, Applied, Undone, Invalid };

inline const char* undoActionStateName(UndoActionState s) {
    switch (s) {
        case UndoActionState::Pending:  return "Pending";
        case UndoActionState::Applied:  return "Applied";
        case UndoActionState::Undone:   return "Undone";
        case UndoActionState::Invalid:  return "Invalid";
        default:                        return "Unknown";
    }
}

struct UndoAction {
    std::string     id;
    std::string     description;
    UndoActionType  type  = UndoActionType::Create;
    UndoActionState state = UndoActionState::Pending;

    void apply()      { state = UndoActionState::Applied; }
    void undo()       { if (state == UndoActionState::Applied) state = UndoActionState::Undone; }
    void invalidate() { state = UndoActionState::Invalid; }

    [[nodiscard]] bool isApplied() const { return state == UndoActionState::Applied; }
    [[nodiscard]] bool isUndone()  const { return state == UndoActionState::Undone;  }
    [[nodiscard]] bool isValid()   const { return state != UndoActionState::Invalid; }
    [[nodiscard]] bool canUndo()   const { return isApplied(); }
    [[nodiscard]] bool canRedo()   const { return isUndone();  }
};

class UndoGroup {
public:
    explicit UndoGroup(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool addAction(UndoAction a) {
        for (auto& existing : m_actions) if (existing.id == a.id) return false;
        m_actions.push_back(std::move(a));
        return true;
    }

    bool removeAction(const std::string& id) {
        for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
            if (it->id == id) { m_actions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] UndoAction* find(const std::string& id) {
        for (auto& a : m_actions) if (a.id == id) return &a;
        return nullptr;
    }

    void applyAll() { for (auto& a : m_actions) a.apply(); }

    void undoAll() {
        for (auto& a : m_actions) if (a.isApplied()) a.undo();
    }

    [[nodiscard]] size_t actionCount() const { return m_actions.size(); }

    [[nodiscard]] size_t appliedCount() const {
        size_t c = 0;
        for (auto& a : m_actions) if (a.isApplied()) c++;
        return c;
    }

private:
    std::string              m_name;
    std::vector<UndoAction>  m_actions;
};

class UndoRedoSystem {
public:
    static constexpr size_t MAX_GROUPS = 64;

    bool pushGroup(UndoGroup g) {
        if (m_undoStack.size() >= MAX_GROUPS) return false;
        m_redoStack.clear();
        m_undoStack.push_back(std::move(g));
        return true;
    }

    bool undo() {
        if (m_undoStack.empty()) return false;
        UndoGroup g = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        g.undoAll();
        m_redoStack.push_back(std::move(g));
        return true;
    }

    bool redo() {
        if (m_redoStack.empty()) return false;
        UndoGroup g = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        g.applyAll();
        m_undoStack.push_back(std::move(g));
        return true;
    }

    [[nodiscard]] bool   canUndo()    const { return !m_undoStack.empty(); }
    [[nodiscard]] bool   canRedo()    const { return !m_redoStack.empty(); }
    [[nodiscard]] size_t undoDepth()  const { return m_undoStack.size();   }
    [[nodiscard]] size_t redoDepth()  const { return m_redoStack.size();   }

    void clear() { m_undoStack.clear(); m_redoStack.clear(); }

private:
    std::vector<UndoGroup> m_undoStack;
    std::vector<UndoGroup> m_redoStack;
};

// ============================================================
// S26 — Command Palette
// ============================================================


} // namespace NF
