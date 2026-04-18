#pragma once
#include "EditorCommandBus.h"
#include "../command/CommandHistory.h"
#include <memory>

namespace atlas::tools {

/// An editor command that can be undone / redone.
///
/// Concrete implementations override DoExecute() and DoUndo() to perform
/// the actual work.  The UndoableCommandBus integrates with the engine's
/// CommandHistory to provide undo/redo and also dispatches through the
/// EditorCommandBus so listeners are notified.
class UndoableEditorCommand : public atlas::command::Command {
public:
    explicit UndoableEditorCommand(std::string name,
                                   EditorCommand cmd = {})
        : m_name(std::move(name))
        , m_cmd(std::move(cmd)) {}

    void Execute() override { DoExecute(); }
    void Undo() override    { DoUndo(); }
    std::string Name() const override { return m_name; }

    const EditorCommand& Cmd() const { return m_cmd; }

protected:
    virtual void DoExecute() = 0;
    virtual void DoUndo() = 0;

    std::string m_name;
    EditorCommand m_cmd;
};

/// Combines an EditorCommandBus (FIFO dispatch) with an undo/redo stack.
///
/// Commands executed through Execute() are both pushed onto the undo stack
/// and dispatched to registered handlers on the EditorCommandBus.
class UndoableCommandBus {
public:
    /// Execute an undoable command (pushes to undo stack + dispatches).
    void Execute(std::unique_ptr<UndoableEditorCommand> cmd);

    bool CanUndo() const;
    bool CanRedo() const;

    void Undo();
    void Redo();

    void Clear();

    size_t UndoCount() const;
    size_t RedoCount() const;

    const command::Command* LastExecuted() const;

    /// Access the underlying command bus for handler registration.
    EditorCommandBus& Bus();

private:
    command::CommandHistory m_history;
    EditorCommandBus m_bus;
};

} // namespace atlas::tools
