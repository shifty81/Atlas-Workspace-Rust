#pragma once
// NF::Editor — Undo/redo system v1: action stack with undo and redo support
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ursv1ActionType  : uint8_t { Create, Delete, Move, Modify, Group, Ungroup };
enum class Ursv1ActionState : uint8_t { Pending, Applied, Undone, Merged };

inline const char* ursv1ActionTypeName(Ursv1ActionType t) {
    switch (t) {
        case Ursv1ActionType::Create:  return "Create";
        case Ursv1ActionType::Delete:  return "Delete";
        case Ursv1ActionType::Move:    return "Move";
        case Ursv1ActionType::Modify:  return "Modify";
        case Ursv1ActionType::Group:   return "Group";
        case Ursv1ActionType::Ungroup: return "Ungroup";
    }
    return "Unknown";
}

inline const char* ursv1ActionStateName(Ursv1ActionState s) {
    switch (s) {
        case Ursv1ActionState::Pending: return "Pending";
        case Ursv1ActionState::Applied: return "Applied";
        case Ursv1ActionState::Undone:  return "Undone";
        case Ursv1ActionState::Merged:  return "Merged";
    }
    return "Unknown";
}

struct Ursv1Action {
    uint64_t         id    = 0;
    std::string      name;
    Ursv1ActionType  type  = Ursv1ActionType::Modify;
    Ursv1ActionState state = Ursv1ActionState::Pending;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isApplied() const { return state == Ursv1ActionState::Applied; }
    [[nodiscard]] bool isUndone()  const { return state == Ursv1ActionState::Undone; }
};

using Ursv1ChangeCallback = std::function<void(uint64_t)>;

class UndoRedoSystemV1 {
public:
    static constexpr size_t MAX_HISTORY = 1024;

    bool addAction(const Ursv1Action& action) {
        if (!action.isValid()) return false;
        for (const auto& a : m_history) if (a.id == action.id) return false;
        if (m_history.size() >= MAX_HISTORY) m_history.erase(m_history.begin());
        m_history.push_back(action);
        m_redoStack.clear();
        m_undoPtr = m_history.size();
        if (m_onChange) m_onChange(action.id);
        return true;
    }

    [[nodiscard]] bool canUndo() const { return m_undoPtr > 0; }
    [[nodiscard]] bool canRedo() const { return !m_redoStack.empty(); }

    bool undo() {
        if (!canUndo()) return false;
        --m_undoPtr;
        m_history[m_undoPtr].state = Ursv1ActionState::Undone;
        m_redoStack.push_back(m_history[m_undoPtr]);
        if (m_onChange) m_onChange(m_history[m_undoPtr].id);
        return true;
    }

    bool redo() {
        if (!canRedo()) return false;
        Ursv1Action act = m_redoStack.back();
        m_redoStack.pop_back();
        act.state = Ursv1ActionState::Applied;
        m_history[m_undoPtr] = act;
        ++m_undoPtr;
        if (m_onChange) m_onChange(act.id);
        return true;
    }

    [[nodiscard]] size_t actionCount() const { return m_history.size(); }
    [[nodiscard]] size_t undoDepth()   const { return m_undoPtr; }
    [[nodiscard]] size_t redoDepth()   const { return m_redoStack.size(); }

    [[nodiscard]] size_t appliedCount() const {
        size_t c = 0; for (const auto& a : m_history) if (a.isApplied()) ++c; return c;
    }
    [[nodiscard]] size_t undoneCount() const {
        size_t c = 0; for (const auto& a : m_history) if (a.isUndone()) ++c; return c;
    }
    [[nodiscard]] size_t countByActionType(Ursv1ActionType type) const {
        size_t c = 0; for (const auto& a : m_history) if (a.type == type) ++c; return c;
    }

    void clear() { m_history.clear(); m_redoStack.clear(); m_undoPtr = 0; }

    void setOnChange(Ursv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ursv1Action> m_history;
    std::vector<Ursv1Action> m_redoStack;
    size_t                   m_undoPtr = 0;
    Ursv1ChangeCallback      m_onChange;
};

} // namespace NF
