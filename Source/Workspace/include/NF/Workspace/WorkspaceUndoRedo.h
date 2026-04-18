#pragma once
// NF::Workspace — Phase 18: Workspace Undo/Redo Stack
//
// Workspace-level undo/redo with transactional grouping:
//   UndoActionType      — action type classification
//   UndoAction          — reversible action with do/undo handlers
//   UndoTransaction     — grouped action sequence for atomic undo
//   UndoStack           — linear undo/redo with transaction support
//   UndoManager         — workspace-scoped undo management with observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// UndoActionType — action type classification
// ═════════════════════════════════════════════════════════════════

enum class UndoActionType : uint8_t {
    Generic     = 0,   // Unclassified
    Property    = 1,   // Property value change
    Create      = 2,   // Object creation
    Delete      = 3,   // Object deletion
    Move        = 4,   // Object repositioning
    Transform   = 5,   // Object transformation
    Reparent    = 6,   // Hierarchy change
    Command     = 7,   // Command execution
    Batch       = 8,   // Batched multi-action
    Custom      = 9,   // User-defined
};

inline const char* undoActionTypeName(UndoActionType t) {
    switch (t) {
        case UndoActionType::Generic:   return "Generic";
        case UndoActionType::Property:  return "Property";
        case UndoActionType::Create:    return "Create";
        case UndoActionType::Delete:    return "Delete";
        case UndoActionType::Move:      return "Move";
        case UndoActionType::Transform: return "Transform";
        case UndoActionType::Reparent:  return "Reparent";
        case UndoActionType::Command:   return "Command";
        case UndoActionType::Batch:     return "Batch";
        case UndoActionType::Custom:    return "Custom";
        default:                        return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// UndoAction — reversible action with do/undo handlers
// ═════════════════════════════════════════════════════════════════

class UndoAction {
public:
    using Handler = std::function<bool()>;

    UndoAction() = default;

    UndoAction(const std::string& label, UndoActionType type,
               Handler doHandler, Handler undoHandler)
        : label_(label), type_(type),
          doHandler_(std::move(doHandler)),
          undoHandler_(std::move(undoHandler)) {}

    // ── Identity ─────────────────────────────────────────────────
    const std::string& label() const { return label_; }
    UndoActionType     type()  const { return type_; }

    bool isValid() const {
        return !label_.empty() && doHandler_ && undoHandler_;
    }

    // ── Execution ────────────────────────────────────────────────
    bool execute() {
        if (!doHandler_) return false;
        return doHandler_();
    }

    bool undo() {
        if (!undoHandler_) return false;
        return undoHandler_();
    }

    // ── Target (optional) ────────────────────────────────────────
    void   setTargetId(const std::string& id) { targetId_ = id; }
    const std::string& targetId() const       { return targetId_; }

    bool operator==(const UndoAction& o) const {
        return label_ == o.label_ && type_ == o.type_;
    }
    bool operator!=(const UndoAction& o) const { return !(*this == o); }

private:
    std::string    label_;
    UndoActionType type_        = UndoActionType::Generic;
    Handler        doHandler_;
    Handler        undoHandler_;
    std::string    targetId_;
};

// ═════════════════════════════════════════════════════════════════
// UndoTransaction — grouped action sequence for atomic undo
// ═════════════════════════════════════════════════════════════════

class UndoTransaction {
public:
    UndoTransaction() = default;

    explicit UndoTransaction(const std::string& label)
        : label_(label) {}

    // ── Identity ─────────────────────────────────────────────────
    const std::string& label() const { return label_; }
    bool isValid() const { return !label_.empty(); }
    bool isEmpty() const { return actions_.empty(); }
    size_t actionCount() const { return actions_.size(); }

    // ── Action management ────────────────────────────────────────
    bool addAction(const UndoAction& action) {
        if (!action.isValid()) return false;
        if (actions_.size() >= MAX_ACTIONS) return false;
        actions_.push_back(action);
        return true;
    }

    const std::vector<UndoAction>& actions() const { return actions_; }

    // ── Execution ────────────────────────────────────────────────
    bool execute() {
        for (auto& action : actions_) {
            if (!action.execute()) {
                // Rollback previously executed actions in reverse
                rollbackFrom(static_cast<int>(&action - &actions_[0]) - 1);
                return false;
            }
        }
        return true;
    }

    bool undo() {
        // Undo in reverse order
        for (int i = static_cast<int>(actions_.size()) - 1; i >= 0; --i) {
            if (!actions_[static_cast<size_t>(i)].undo()) return false;
        }
        return true;
    }

    static constexpr size_t MAX_ACTIONS = 256;

private:
    void rollbackFrom(int lastSuccessIdx) {
        for (int i = lastSuccessIdx; i >= 0; --i) {
            actions_[static_cast<size_t>(i)].undo();
        }
    }

    std::string              label_;
    std::vector<UndoAction>  actions_;
};

// ═════════════════════════════════════════════════════════════════
// UndoStack — linear undo/redo with transaction support
// ═════════════════════════════════════════════════════════════════

class UndoStack {
public:
    static constexpr size_t DEFAULT_MAX_DEPTH = 128;

    // ── Configuration ────────────────────────────────────────────
    void setMaxDepth(size_t n) { maxDepth_ = n; trimStack(); }
    size_t maxDepth() const { return maxDepth_; }

    // ── Push ─────────────────────────────────────────────────────
    bool push(const UndoAction& action) {
        if (!action.isValid()) return false;

        UndoTransaction txn(action.label());
        txn.addAction(action);
        return pushTransaction(txn);
    }

    bool pushTransaction(const UndoTransaction& txn) {
        if (!txn.isValid() || txn.isEmpty()) return false;

        // Clear redo stack on new push
        if (cursor_ < stack_.size()) {
            stack_.erase(stack_.begin() + static_cast<ptrdiff_t>(cursor_), stack_.end());
        }

        stack_.push_back(txn);
        cursor_ = stack_.size();
        trimStack();
        ++totalPushes_;
        return true;
    }

    // ── Undo / Redo ──────────────────────────────────────────────
    bool canUndo() const { return cursor_ > 0; }
    bool canRedo() const { return cursor_ < stack_.size(); }

    bool undo() {
        if (!canUndo()) return false;
        --cursor_;
        bool ok = stack_[cursor_].undo();
        if (ok) ++totalUndos_;
        return ok;
    }

    bool redo() {
        if (!canRedo()) return false;
        bool ok = stack_[cursor_].execute();
        if (ok) { ++cursor_; ++totalRedos_; }
        return ok;
    }

    // ── Labels ───────────────────────────────────────────────────
    std::string nextUndoLabel() const {
        return canUndo() ? stack_[cursor_ - 1].label() : "";
    }

    std::string nextRedoLabel() const {
        return canRedo() ? stack_[cursor_].label() : "";
    }

    std::vector<std::string> undoLabels() const {
        std::vector<std::string> labels;
        for (size_t i = cursor_; i > 0; --i) {
            labels.push_back(stack_[i - 1].label());
        }
        return labels;
    }

    std::vector<std::string> redoLabels() const {
        std::vector<std::string> labels;
        for (size_t i = cursor_; i < stack_.size(); ++i) {
            labels.push_back(stack_[i].label());
        }
        return labels;
    }

    // ── Transaction grouping ─────────────────────────────────────
    bool beginTransaction(const std::string& label) {
        if (activeTransaction_) return false; // Already open
        pendingTxn_ = UndoTransaction(label);
        activeTransaction_ = true;
        return true;
    }

    bool addToTransaction(const UndoAction& action) {
        if (!activeTransaction_) return false;
        return pendingTxn_.addAction(action);
    }

    bool commitTransaction() {
        if (!activeTransaction_) return false;
        activeTransaction_ = false;
        if (pendingTxn_.isEmpty()) return false;
        return pushTransaction(pendingTxn_);
    }

    bool discardTransaction() {
        if (!activeTransaction_) return false;
        activeTransaction_ = false;
        pendingTxn_ = UndoTransaction();
        return true;
    }

    bool isTransactionOpen() const { return activeTransaction_; }

    std::string activeTransactionLabel() const {
        return activeTransaction_ ? pendingTxn_.label() : "";
    }

    size_t activeTransactionSize() const {
        return activeTransaction_ ? pendingTxn_.actionCount() : 0;
    }

    // ── State ────────────────────────────────────────────────────
    size_t depth()        const { return stack_.size(); }
    size_t undoDepth()    const { return cursor_; }
    size_t redoDepth()    const { return stack_.size() - cursor_; }
    bool   isEmpty()      const { return stack_.empty(); }
    bool   isDirty()      const { return cursor_ != cleanCursor_; }

    void markClean() { cleanCursor_ = cursor_; }

    // ── Statistics ───────────────────────────────────────────────
    size_t totalPushes() const { return totalPushes_; }
    size_t totalUndos()  const { return totalUndos_; }
    size_t totalRedos()  const { return totalRedos_; }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() {
        stack_.clear();
        cursor_        = 0;
        cleanCursor_   = 0;
        totalPushes_   = 0;
        totalUndos_    = 0;
        totalRedos_    = 0;
        activeTransaction_ = false;
        pendingTxn_    = UndoTransaction();
    }

private:
    void trimStack() {
        while (stack_.size() > maxDepth_ && cursor_ > 0) {
            stack_.erase(stack_.begin());
            --cursor_;
            if (cleanCursor_ > 0) --cleanCursor_;
        }
    }

    std::vector<UndoTransaction> stack_;
    size_t                       cursor_            = 0;
    size_t                       cleanCursor_       = 0;
    size_t                       maxDepth_          = DEFAULT_MAX_DEPTH;
    size_t                       totalPushes_       = 0;
    size_t                       totalUndos_        = 0;
    size_t                       totalRedos_        = 0;
    bool                         activeTransaction_ = false;
    UndoTransaction              pendingTxn_;
};

// ═════════════════════════════════════════════════════════════════
// UndoManager — workspace-scoped undo management with observers
// ═════════════════════════════════════════════════════════════════

class UndoManager {
public:
    using Observer = std::function<void(const std::string& label, bool isUndo)>;
    static constexpr size_t MAX_STACKS    = 64;
    static constexpr size_t MAX_OBSERVERS = 32;

    // ── Stack management ─────────────────────────────────────────
    bool registerStack(const std::string& name) {
        if (name.empty()) return false;
        for (auto& s : stacks_) if (s.first == name) return false;
        if (stacks_.size() >= MAX_STACKS) return false;
        stacks_.push_back({name, UndoStack()});
        if (activeStack_.empty()) activeStack_ = name;
        return true;
    }

    bool unregisterStack(const std::string& name) {
        for (auto it = stacks_.begin(); it != stacks_.end(); ++it) {
            if (it->first == name) {
                stacks_.erase(it);
                if (activeStack_ == name) {
                    activeStack_ = stacks_.empty() ? "" : stacks_[0].first;
                }
                return true;
            }
        }
        return false;
    }

    bool setActiveStack(const std::string& name) {
        for (auto& s : stacks_) {
            if (s.first == name) { activeStack_ = name; return true; }
        }
        return false;
    }

    const std::string& activeStackName() const { return activeStack_; }

    UndoStack* activeStack() {
        return findStack(activeStack_);
    }

    const UndoStack* activeStack() const {
        return findStack(activeStack_);
    }

    UndoStack* findStack(const std::string& name) {
        for (auto& s : stacks_) if (s.first == name) return &s.second;
        return nullptr;
    }

    const UndoStack* findStack(const std::string& name) const {
        for (auto& s : stacks_) if (s.first == name) return &s.second;
        return nullptr;
    }

    bool isRegistered(const std::string& name) const {
        for (auto& s : stacks_) if (s.first == name) return true;
        return false;
    }

    size_t stackCount() const { return stacks_.size(); }

    std::vector<std::string> stackNames() const {
        std::vector<std::string> names;
        for (auto& s : stacks_) names.push_back(s.first);
        return names;
    }

    // ── Convenience: operate on active stack ─────────────────────
    bool push(const UndoAction& action) {
        auto* stack = activeStack();
        if (!stack) return false;
        bool ok = stack->push(action);
        if (ok) notifyObservers(action.label(), false);
        return ok;
    }

    bool undo() {
        auto* stack = activeStack();
        if (!stack) return false;
        std::string label = stack->nextUndoLabel();
        bool ok = stack->undo();
        if (ok) notifyObservers(label, true);
        return ok;
    }

    bool redo() {
        auto* stack = activeStack();
        if (!stack) return false;
        std::string label = stack->nextRedoLabel();
        bool ok = stack->redo();
        if (ok) notifyObservers(label, false);
        return ok;
    }

    bool canUndo() const {
        auto* stack = activeStack();
        return stack && stack->canUndo();
    }

    bool canRedo() const {
        auto* stack = activeStack();
        return stack && stack->canRedo();
    }

    // ── Observers ────────────────────────────────────────────────
    size_t addObserver(Observer observer) {
        if (observers_.size() >= MAX_OBSERVERS) return 0;
        size_t id = ++nextObserverId_;
        observers_.push_back({id, std::move(observer)});
        return id;
    }

    bool removeObserver(size_t id) {
        for (auto it = observers_.begin(); it != observers_.end(); ++it) {
            if (it->first == id) { observers_.erase(it); return true; }
        }
        return false;
    }

    size_t observerCount() const { return observers_.size(); }

    void clearObservers() { observers_.clear(); }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() {
        stacks_.clear();
        activeStack_.clear();
        observers_.clear();
        nextObserverId_ = 0;
    }

private:
    void notifyObservers(const std::string& label, bool isUndo) {
        for (auto& obs : observers_) {
            if (obs.second) obs.second(label, isUndo);
        }
    }

    std::vector<std::pair<std::string, UndoStack>>    stacks_;
    std::string                                        activeStack_;
    std::vector<std::pair<size_t, Observer>>            observers_;
    size_t                                             nextObserverId_ = 0;
};

} // namespace NF
