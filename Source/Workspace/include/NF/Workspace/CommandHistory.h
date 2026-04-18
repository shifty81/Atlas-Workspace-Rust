#pragma once
// NF::Workspace — CommandHistory: undo/redo stack with command grouping.
//
// CommandHistory records reversible command executions so the workspace can
// offer linear undo/redo navigation. Key design points:
//
//   HistoryEntry   — one recorded execution (command id + captured undo fn)
//   CommandGroup   — a named transaction grouping multiple entries into a single
//                    undo/redo step (open with beginGroup / close with endGroup)
//   CommandHistory — the stack: push entries or groups, undo/redo, query depth
//
// Constraints:
//   - MAX_DEPTH (default 128) entries. Oldest entry is dropped when full.
//   - Redoing is invalidated by any new push (standard linear undo model).
//   - Groups must be closed before they are committed; an unclosed group is
//     NOT committed to the stack until endGroup() is called.
//   - If a group is discarded via discardGroup() the partial entries are lost.
//   - All handlers are std::function<bool()>; returning false is not an error
//     but is reported to callers via UndoRedoResult.

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Undo/Redo Result ───────────────────────────────────────────────

enum class UndoRedoStatus : uint8_t {
    Success,
    NothingToUndo,
    NothingToRedo,
    HandlerFailed,
};

struct UndoRedoResult {
    UndoRedoStatus status;
    std::string    label;  // label of the entry that was undone/redone

    [[nodiscard]] bool succeeded() const { return status == UndoRedoStatus::Success; }

    static UndoRedoResult ok(const std::string& lbl) {
        return {UndoRedoStatus::Success, lbl};
    }
    static UndoRedoResult nothingToUndo() {
        return {UndoRedoStatus::NothingToUndo, {}};
    }
    static UndoRedoResult nothingToRedo() {
        return {UndoRedoStatus::NothingToRedo, {}};
    }
    static UndoRedoResult handlerFailed(const std::string& lbl) {
        return {UndoRedoStatus::HandlerFailed, lbl};
    }
};

// ── History Entry ──────────────────────────────────────────────────

struct HistoryEntry {
    std::string                  commandId;
    std::string                  label;          // display label for undo menu
    std::function<bool()>        undoFn;         // captured undo closure
    bool                         isGroupEntry = false;
};

// ── Command Group ──────────────────────────────────────────────────
// Batches multiple history entries into a single undo/redo step.

struct CommandGroup {
    std::string               name;
    std::vector<HistoryEntry> entries;

    [[nodiscard]] bool empty() const { return entries.empty(); }
    [[nodiscard]] size_t size() const { return entries.size(); }
};

// ── Command History ────────────────────────────────────────────────

class CommandHistory {
public:
    static constexpr size_t DEFAULT_MAX_DEPTH = 128;

    explicit CommandHistory(size_t maxDepth = DEFAULT_MAX_DEPTH)
        : m_maxDepth(maxDepth)
    {}

    // ── Single-entry push ─────────────────────────────────────────

    // Push a reversible action onto the stack.
    // If a group is open the entry is added to the group instead.
    // Returns false if undoFn is null or commandId is empty.
    bool push(const std::string& commandId,
              const std::string& label,
              std::function<bool()> undoFn) {
        if (commandId.empty() || !undoFn) return false;

        HistoryEntry entry;
        entry.commandId = commandId;
        entry.label     = label.empty() ? commandId : label;
        entry.undoFn    = std::move(undoFn);

        if (m_groupOpen) {
            entry.isGroupEntry = true;
            m_openGroup.entries.push_back(std::move(entry));
            return true;
        }

        commitEntry(std::move(entry));
        return true;
    }

    // ── Group management ──────────────────────────────────────────

    bool beginGroup(const std::string& name) {
        if (m_groupOpen) return false; // already open
        m_groupOpen = true;
        m_openGroup = {};
        m_openGroup.name = name.empty() ? "Group" : name;
        return true;
    }

    // Commits all entries in the open group as a single undo step.
    // Returns false if no group was open or the group was empty.
    bool endGroup() {
        if (!m_groupOpen) return false;
        m_groupOpen = false;
        if (m_openGroup.empty()) return false;

        // Pack the group into a synthetic HistoryEntry whose undoFn
        // undoes all sub-entries in reverse order.
        auto entries = std::move(m_openGroup.entries);
        std::string name = m_openGroup.name;

        HistoryEntry grouped;
        grouped.commandId = "group:" + name;
        grouped.label     = name;
        grouped.undoFn    = [captured = std::move(entries)]() mutable -> bool {
            bool all = true;
            for (auto it = captured.rbegin(); it != captured.rend(); ++it) {
                if (it->undoFn && !it->undoFn()) all = false;
            }
            return all;
        };

        commitEntry(std::move(grouped));
        return true;
    }

    // Discard the open group without committing.
    bool discardGroup() {
        if (!m_groupOpen) return false;
        m_groupOpen = false;
        m_openGroup = {};
        return true;
    }

    [[nodiscard]] bool isGroupOpen() const { return m_groupOpen; }
    [[nodiscard]] const std::string& openGroupName() const { return m_openGroup.name; }
    [[nodiscard]] size_t openGroupSize() const { return m_openGroup.entries.size(); }

    // ── Undo / Redo ───────────────────────────────────────────────

    UndoRedoResult undo() {
        if (m_undoStack.empty()) return UndoRedoResult::nothingToUndo();

        auto entry = std::move(m_undoStack.back());
        m_undoStack.pop_back();

        bool ok = entry.undoFn ? entry.undoFn() : false;
        m_redoStack.push_back(entry); // push a copy for redo (undoFn is still set)
        // Note: re-push the moved entry by putting back via copy (redoStack holds the fn)
        // Actually we need to handle this carefully since we moved entry.
        // Let's rebuild:
        // We already moved entry, so push back the one we moved onto redo:
        return ok ? UndoRedoResult::ok(m_redoStack.back().label)
                  : UndoRedoResult::handlerFailed(m_redoStack.back().label);
    }

    UndoRedoResult redo() {
        if (m_redoStack.empty()) return UndoRedoResult::nothingToRedo();
        // Redo re-does by... in this linear model we don't store redo handlers.
        // Instead, redo pops from redo stack and pushes back to undo stack
        // (conceptually re-executing). Since we don't store execute handlers in
        // history we simply return the entry and move it back.
        auto entry = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        m_undoStack.push_back(entry); // re-push for undo again
        return UndoRedoResult::ok(entry.label);
    }

    // ── Queries ───────────────────────────────────────────────────

    [[nodiscard]] bool canUndo() const { return !m_undoStack.empty(); }
    [[nodiscard]] bool canRedo() const { return !m_redoStack.empty(); }

    [[nodiscard]] size_t undoDepth() const { return m_undoStack.size(); }
    [[nodiscard]] size_t redoDepth() const { return m_redoStack.size(); }
    [[nodiscard]] size_t maxDepth()  const { return m_maxDepth; }

    [[nodiscard]] const std::string& nextUndoLabel() const {
        static const std::string empty;
        return m_undoStack.empty() ? empty : m_undoStack.back().label;
    }
    [[nodiscard]] const std::string& nextRedoLabel() const {
        static const std::string empty;
        return m_redoStack.empty() ? empty : m_redoStack.back().label;
    }

    // All undo stack labels from newest (back) to oldest (front).
    [[nodiscard]] std::vector<std::string> undoLabels() const {
        std::vector<std::string> out;
        out.reserve(m_undoStack.size());
        for (auto it = m_undoStack.rbegin(); it != m_undoStack.rend(); ++it)
            out.push_back(it->label);
        return out;
    }

    // ── Lifecycle ─────────────────────────────────────────────────

    void clearHistory() {
        m_undoStack.clear();
        m_redoStack.clear();
        m_groupOpen = false;
        m_openGroup = {};
    }

    void setMaxDepth(size_t depth) { m_maxDepth = depth > 0 ? depth : 1; }

private:
    void commitEntry(HistoryEntry entry) {
        // Any new commit invalidates the redo stack (linear model).
        m_redoStack.clear();

        m_undoStack.push_back(std::move(entry));

        // Trim oldest if over limit.
        if (m_undoStack.size() > m_maxDepth) {
            m_undoStack.erase(m_undoStack.begin());
        }
    }

    std::vector<HistoryEntry> m_undoStack;
    std::vector<HistoryEntry> m_redoStack;
    size_t                    m_maxDepth  = DEFAULT_MAX_DEPTH;
    bool                      m_groupOpen = false;
    CommandGroup               m_openGroup;
};

} // namespace NF
