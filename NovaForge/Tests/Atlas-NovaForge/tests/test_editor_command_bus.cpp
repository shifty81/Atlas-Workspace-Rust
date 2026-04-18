#include "../engine/tools/EditorCommandBus.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace atlas::tools;

// ---- Trivial test command that increments / decrements a counter ----

struct IncrementCommand : IEditorCommand {
    int& counter;
    int amount;

    IncrementCommand(int& c, int a) : counter(c), amount(a) {}

    std::string Description() const override {
        return "Increment by " + std::to_string(amount);
    }
    void Execute() override { counter += amount; }
    void Undo() override    { counter -= amount; }
};

// ---- Tests ----

void test_editor_command_bus_post_and_process() {
    EditorCommandBus bus;
    int counter = 0;

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 5));
    assert(bus.PendingCount() == 1);
    assert(counter == 0);  // not yet executed

    bus.ProcessCommands();
    assert(counter == 5);
    assert(bus.PendingCount() == 0);
    assert(bus.UndoCount() == 1);
    std::cout << "[PASS] test_editor_command_bus_post_and_process" << std::endl;
}

void test_editor_command_bus_undo() {
    EditorCommandBus bus;
    int counter = 0;

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 10));
    bus.ProcessCommands();
    assert(counter == 10);

    bool ok = bus.UndoLast();
    assert(ok);
    assert(counter == 0);
    assert(bus.UndoCount() == 0);
    assert(bus.RedoCount() == 1);
    std::cout << "[PASS] test_editor_command_bus_undo" << std::endl;
}

void test_editor_command_bus_redo() {
    EditorCommandBus bus;
    int counter = 0;

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 7));
    bus.ProcessCommands();
    bus.UndoLast();
    assert(counter == 0);

    bool ok = bus.RedoLast();
    assert(ok);
    assert(counter == 7);
    assert(bus.UndoCount() == 1);
    assert(bus.RedoCount() == 0);
    std::cout << "[PASS] test_editor_command_bus_redo" << std::endl;
}

void test_editor_command_bus_redo_cleared_on_new_command() {
    EditorCommandBus bus;
    int counter = 0;

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 3));
    bus.ProcessCommands();
    bus.UndoLast();
    assert(bus.RedoCount() == 1);

    // Posting a new command should clear the redo stack
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 5));
    bus.ProcessCommands();
    assert(bus.RedoCount() == 0);
    assert(counter == 5);
    std::cout << "[PASS] test_editor_command_bus_redo_cleared_on_new_command" << std::endl;
}

void test_editor_command_bus_multiple_undo_redo() {
    EditorCommandBus bus;
    int counter = 0;

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 1));
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 2));
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 3));
    bus.ProcessCommands();
    assert(counter == 6);
    assert(bus.UndoCount() == 3);

    bus.UndoLast();  // undo +3 → counter = 3
    assert(counter == 3);
    bus.UndoLast();  // undo +2 → counter = 1
    assert(counter == 1);

    bus.RedoLast();  // redo +2 → counter = 3
    assert(counter == 3);
    assert(bus.UndoCount() == 2);
    assert(bus.RedoCount() == 1);
    std::cout << "[PASS] test_editor_command_bus_multiple_undo_redo" << std::endl;
}

void test_editor_command_bus_undo_empty() {
    EditorCommandBus bus;
    assert(!bus.UndoLast());
    assert(!bus.RedoLast());
    std::cout << "[PASS] test_editor_command_bus_undo_empty" << std::endl;
}

void test_editor_command_bus_clear() {
    EditorCommandBus bus;
    int counter = 0;

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 1));
    bus.ProcessCommands();
    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 2));

    bus.Clear();
    assert(bus.PendingCount() == 0);
    assert(bus.UndoCount() == 0);
    assert(bus.RedoCount() == 0);
    std::cout << "[PASS] test_editor_command_bus_clear" << std::endl;
}

void test_editor_command_bus_listener() {
    EditorCommandBus bus;
    int counter = 0;
    int execNotify = 0;
    int undoNotify = 0;

    bus.SetListener([&](const IEditorCommand&, bool undone) {
        if (undone) undoNotify++;
        else execNotify++;
    });

    bus.PostCommand(std::make_unique<IncrementCommand>(counter, 1));
    bus.ProcessCommands();
    assert(execNotify == 1);

    bus.UndoLast();
    assert(undoNotify == 1);

    bus.RedoLast();
    assert(execNotify == 2);
    std::cout << "[PASS] test_editor_command_bus_listener" << std::endl;
}

void test_editor_command_bus_null_command() {
    EditorCommandBus bus;
    bus.PostCommand(nullptr);
    assert(bus.PendingCount() == 0);
    std::cout << "[PASS] test_editor_command_bus_null_command" << std::endl;
}
