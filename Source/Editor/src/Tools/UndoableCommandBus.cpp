#include "UndoableCommandBus.h"

namespace atlas::tools {

void UndoableCommandBus::Execute(std::unique_ptr<UndoableEditorCommand> cmd) {
    // Notify listeners via the bus.
    m_bus.Enqueue(cmd->Cmd());
    m_bus.Dispatch();

    // Push onto the undo stack (calls cmd->Execute() internally).
    m_history.Execute(std::move(cmd));
}

bool UndoableCommandBus::CanUndo() const { return m_history.CanUndo(); }
bool UndoableCommandBus::CanRedo() const { return m_history.CanRedo(); }

void UndoableCommandBus::Undo() { m_history.Undo(); }
void UndoableCommandBus::Redo() { m_history.Redo(); }

void UndoableCommandBus::Clear() { m_history.Clear(); }

size_t UndoableCommandBus::UndoCount() const { return m_history.UndoCount(); }
size_t UndoableCommandBus::RedoCount() const { return m_history.RedoCount(); }

const command::Command* UndoableCommandBus::LastExecuted() const {
    return m_history.LastExecuted();
}

EditorCommandBus& UndoableCommandBus::Bus() { return m_bus; }

} // namespace atlas::tools
