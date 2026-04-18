#include "../engine/command/CommandHistory.h"
#include <iostream>
#include <cassert>

using namespace atlas::command;

// Test command that increments/decrements a counter
class TestCounterCommand : public Command {
public:
    TestCounterCommand(int& counter, int delta)
        : m_counter(counter), m_delta(delta) {}

    void Execute() override { m_counter += m_delta; }
    void Undo() override { m_counter -= m_delta; }
    std::string Name() const override { return "TestCounter"; }

private:
    int& m_counter;
    int m_delta;
};

void test_command_execute() {
    int counter = 0;
    CommandHistory history;

    history.Execute(std::make_unique<TestCounterCommand>(counter, 5));
    assert(counter == 5);
    assert(history.UndoCount() == 1);
    assert(history.RedoCount() == 0);

    std::cout << "[PASS] test_command_execute" << std::endl;
}

void test_command_undo() {
    int counter = 0;
    CommandHistory history;

    history.Execute(std::make_unique<TestCounterCommand>(counter, 5));
    history.Execute(std::make_unique<TestCounterCommand>(counter, 3));
    assert(counter == 8);

    history.Undo();
    assert(counter == 5);
    assert(history.UndoCount() == 1);
    assert(history.RedoCount() == 1);

    std::cout << "[PASS] test_command_undo" << std::endl;
}

void test_command_redo() {
    int counter = 0;
    CommandHistory history;

    history.Execute(std::make_unique<TestCounterCommand>(counter, 5));
    history.Undo();
    assert(counter == 0);

    history.Redo();
    assert(counter == 5);
    assert(history.UndoCount() == 1);
    assert(history.RedoCount() == 0);

    std::cout << "[PASS] test_command_redo" << std::endl;
}

void test_command_redo_cleared_on_execute() {
    int counter = 0;
    CommandHistory history;

    history.Execute(std::make_unique<TestCounterCommand>(counter, 5));
    history.Undo();
    assert(history.CanRedo());

    // New command should clear redo stack
    history.Execute(std::make_unique<TestCounterCommand>(counter, 10));
    assert(!history.CanRedo());
    assert(counter == 10);

    std::cout << "[PASS] test_command_redo_cleared_on_execute" << std::endl;
}

void test_command_clear() {
    int counter = 0;
    CommandHistory history;

    history.Execute(std::make_unique<TestCounterCommand>(counter, 1));
    history.Execute(std::make_unique<TestCounterCommand>(counter, 2));
    history.Undo();

    history.Clear();
    assert(!history.CanUndo());
    assert(!history.CanRedo());
    assert(history.UndoCount() == 0);
    assert(history.RedoCount() == 0);

    std::cout << "[PASS] test_command_clear" << std::endl;
}

void test_command_last_executed() {
    int counter = 0;
    CommandHistory history;

    assert(history.LastExecuted() == nullptr);

    history.Execute(std::make_unique<TestCounterCommand>(counter, 1));
    assert(history.LastExecuted() != nullptr);
    assert(history.LastExecuted()->Name() == "TestCounter");

    std::cout << "[PASS] test_command_last_executed" << std::endl;
}

void test_command_multiple_undo_redo() {
    int counter = 0;
    CommandHistory history;

    history.Execute(std::make_unique<TestCounterCommand>(counter, 1));
    history.Execute(std::make_unique<TestCounterCommand>(counter, 2));
    history.Execute(std::make_unique<TestCounterCommand>(counter, 3));
    assert(counter == 6);

    history.Undo();
    history.Undo();
    history.Undo();
    assert(counter == 0);
    assert(!history.CanUndo());

    history.Redo();
    history.Redo();
    assert(counter == 3);

    std::cout << "[PASS] test_command_multiple_undo_redo" << std::endl;
}
