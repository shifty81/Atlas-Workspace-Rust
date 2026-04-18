// Tests/Workspace/test_phase18_undo_redo.cpp
// Phase 18 — Workspace Undo/Redo Stack
//
// Tests for:
//   1. UndoActionType — enum name helpers
//   2. UndoAction — reversible action with do/undo handlers
//   3. UndoTransaction — grouped action sequence for atomic undo
//   4. UndoStack — linear undo/redo with transaction support
//   5. UndoManager — workspace-scoped undo management with observers
//   6. Integration — full undo pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceUndoRedo.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("undoActionTypeName returns correct strings", "[Phase18][ActionType]") {
    CHECK(std::string(undoActionTypeName(UndoActionType::Generic))   == "Generic");
    CHECK(std::string(undoActionTypeName(UndoActionType::Property))  == "Property");
    CHECK(std::string(undoActionTypeName(UndoActionType::Create))    == "Create");
    CHECK(std::string(undoActionTypeName(UndoActionType::Delete))    == "Delete");
    CHECK(std::string(undoActionTypeName(UndoActionType::Move))      == "Move");
    CHECK(std::string(undoActionTypeName(UndoActionType::Transform)) == "Transform");
    CHECK(std::string(undoActionTypeName(UndoActionType::Reparent))  == "Reparent");
    CHECK(std::string(undoActionTypeName(UndoActionType::Command))   == "Command");
    CHECK(std::string(undoActionTypeName(UndoActionType::Batch))     == "Batch");
    CHECK(std::string(undoActionTypeName(UndoActionType::Custom))    == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — UndoAction
// ═════════════════════════════════════════════════════════════════

TEST_CASE("UndoAction default is invalid", "[Phase18][Action]") {
    UndoAction action;
    CHECK_FALSE(action.isValid());
    CHECK(action.label().empty());
    CHECK(action.type() == UndoActionType::Generic);
}

TEST_CASE("UndoAction valid construction", "[Phase18][Action]") {
    int val = 0;
    UndoAction action("Set value", UndoActionType::Property,
        [&]() { val = 42; return true; },
        [&]() { val = 0;  return true; });

    CHECK(action.isValid());
    CHECK(action.label() == "Set value");
    CHECK(action.type() == UndoActionType::Property);
}

TEST_CASE("UndoAction execute and undo", "[Phase18][Action]") {
    int val = 0;
    UndoAction action("Set value", UndoActionType::Property,
        [&]() { val = 42; return true; },
        [&]() { val = 0;  return true; });

    CHECK(action.execute());
    CHECK(val == 42);
    CHECK(action.undo());
    CHECK(val == 0);
}

TEST_CASE("UndoAction without handler fails", "[Phase18][Action]") {
    UndoAction action;
    CHECK_FALSE(action.execute());
    CHECK_FALSE(action.undo());
}

TEST_CASE("UndoAction target id", "[Phase18][Action]") {
    UndoAction action("Test", UndoActionType::Generic,
        []() { return true; }, []() { return true; });
    CHECK(action.targetId().empty());
    action.setTargetId("obj_42");
    CHECK(action.targetId() == "obj_42");
}

TEST_CASE("UndoAction equality", "[Phase18][Action]") {
    auto make = [](const std::string& l, UndoActionType t) {
        return UndoAction(l, t, []() { return true; }, []() { return true; });
    };
    CHECK(make("A", UndoActionType::Create) == make("A", UndoActionType::Create));
    CHECK(make("A", UndoActionType::Create) != make("B", UndoActionType::Create));
    CHECK(make("A", UndoActionType::Create) != make("A", UndoActionType::Delete));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — UndoTransaction
// ═════════════════════════════════════════════════════════════════

TEST_CASE("UndoTransaction default state", "[Phase18][Transaction]") {
    UndoTransaction txn;
    CHECK_FALSE(txn.isValid());
    CHECK(txn.isEmpty());
    CHECK(txn.actionCount() == 0);
}

TEST_CASE("UndoTransaction valid construction", "[Phase18][Transaction]") {
    UndoTransaction txn("Batch edit");
    CHECK(txn.isValid());
    CHECK(txn.label() == "Batch edit");
    CHECK(txn.isEmpty());
}

TEST_CASE("UndoTransaction addAction", "[Phase18][Transaction]") {
    UndoTransaction txn("Batch");
    UndoAction a1("A", UndoActionType::Property,
        []() { return true; }, []() { return true; });
    UndoAction a2("B", UndoActionType::Create,
        []() { return true; }, []() { return true; });

    CHECK(txn.addAction(a1));
    CHECK(txn.addAction(a2));
    CHECK(txn.actionCount() == 2);
    CHECK_FALSE(txn.isEmpty());
}

TEST_CASE("UndoTransaction rejects invalid action", "[Phase18][Transaction]") {
    UndoTransaction txn("Batch");
    UndoAction invalid;
    CHECK_FALSE(txn.addAction(invalid));
}

TEST_CASE("UndoTransaction execute all actions", "[Phase18][Transaction]") {
    int x = 0, y = 0;
    UndoTransaction txn("Batch");
    txn.addAction(UndoAction("X", UndoActionType::Property,
        [&]() { x = 10; return true; }, [&]() { x = 0; return true; }));
    txn.addAction(UndoAction("Y", UndoActionType::Property,
        [&]() { y = 20; return true; }, [&]() { y = 0; return true; }));

    CHECK(txn.execute());
    CHECK(x == 10);
    CHECK(y == 20);
}

TEST_CASE("UndoTransaction undo in reverse", "[Phase18][Transaction]") {
    std::vector<int> order;
    UndoTransaction txn("Batch");
    txn.addAction(UndoAction("A", UndoActionType::Generic,
        [&]() { return true; }, [&]() { order.push_back(1); return true; }));
    txn.addAction(UndoAction("B", UndoActionType::Generic,
        [&]() { return true; }, [&]() { order.push_back(2); return true; }));
    txn.addAction(UndoAction("C", UndoActionType::Generic,
        [&]() { return true; }, [&]() { order.push_back(3); return true; }));

    txn.execute();
    CHECK(txn.undo());
    REQUIRE(order.size() == 3);
    CHECK(order[0] == 3);
    CHECK(order[1] == 2);
    CHECK(order[2] == 1);
}

TEST_CASE("UndoTransaction execute rollback on failure", "[Phase18][Transaction]") {
    int x = 0, y = 0;
    UndoTransaction txn("Batch");
    txn.addAction(UndoAction("X", UndoActionType::Property,
        [&]() { x = 10; return true; }, [&]() { x = 0; return true; }));
    txn.addAction(UndoAction("Y", UndoActionType::Property,
        [&]() { y = 20; return false; }, // Fails
        [&]() { y = 0; return true; }));

    CHECK_FALSE(txn.execute());
    CHECK(x == 0); // Rolled back
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — UndoStack
// ═════════════════════════════════════════════════════════════════

TEST_CASE("UndoStack starts empty", "[Phase18][Stack]") {
    UndoStack stack;
    CHECK(stack.isEmpty());
    CHECK(stack.depth() == 0);
    CHECK_FALSE(stack.canUndo());
    CHECK_FALSE(stack.canRedo());
    CHECK_FALSE(stack.isDirty());
    CHECK(stack.maxDepth() == UndoStack::DEFAULT_MAX_DEPTH);
}

TEST_CASE("UndoStack push and undo", "[Phase18][Stack]") {
    int val = 0;
    UndoStack stack;

    UndoAction action("Set 42", UndoActionType::Property,
        [&]() { val = 42; return true; },
        [&]() { val = 0;  return true; });

    CHECK(stack.push(action));
    CHECK(stack.depth() == 1);
    CHECK(stack.canUndo());
    CHECK_FALSE(stack.canRedo());
    CHECK(stack.isDirty());

    CHECK(stack.undo());
    CHECK(val == 0);
    CHECK_FALSE(stack.canUndo());
    CHECK(stack.canRedo());
}

TEST_CASE("UndoStack redo", "[Phase18][Stack]") {
    int val = 0;
    UndoStack stack;

    UndoAction action("Set 42", UndoActionType::Property,
        [&]() { val = 42; return true; },
        [&]() { val = 0;  return true; });

    stack.push(action);
    stack.undo();
    CHECK(val == 0);
    CHECK(stack.redo());
    CHECK(val == 42);
    CHECK(stack.canUndo());
    CHECK_FALSE(stack.canRedo());
}

TEST_CASE("UndoStack push clears redo", "[Phase18][Stack]") {
    int val = 0;
    UndoStack stack;

    stack.push(UndoAction("A", UndoActionType::Generic,
        [&]() { val = 1; return true; }, [&]() { val = 0; return true; }));
    stack.push(UndoAction("B", UndoActionType::Generic,
        [&]() { val = 2; return true; }, [&]() { val = 1; return true; }));

    stack.undo(); // Undo B
    CHECK(stack.canRedo());

    // Push new action clears redo
    stack.push(UndoAction("C", UndoActionType::Generic,
        [&]() { val = 3; return true; }, [&]() { val = 1; return true; }));
    CHECK_FALSE(stack.canRedo());
    CHECK(stack.depth() == 2); // A + C
}

TEST_CASE("UndoStack rejects invalid action", "[Phase18][Stack]") {
    UndoStack stack;
    UndoAction invalid;
    CHECK_FALSE(stack.push(invalid));
}

TEST_CASE("UndoStack labels", "[Phase18][Stack]") {
    UndoStack stack;
    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    stack.push(make("First"));
    stack.push(make("Second"));
    stack.push(make("Third"));

    CHECK(stack.nextUndoLabel() == "Third");
    CHECK(stack.nextRedoLabel().empty());

    auto undos = stack.undoLabels();
    REQUIRE(undos.size() == 3);
    CHECK(undos[0] == "Third");
    CHECK(undos[1] == "Second");
    CHECK(undos[2] == "First");

    stack.undo();
    CHECK(stack.nextRedoLabel() == "Third");
    auto redos = stack.redoLabels();
    REQUIRE(redos.size() == 1);
    CHECK(redos[0] == "Third");
}

TEST_CASE("UndoStack maxDepth trim", "[Phase18][Stack]") {
    UndoStack stack;
    stack.setMaxDepth(3);

    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    stack.push(make("A"));
    stack.push(make("B"));
    stack.push(make("C"));
    stack.push(make("D")); // Should trim A

    CHECK(stack.depth() == 3);
    auto labels = stack.undoLabels();
    REQUIRE(labels.size() == 3);
    CHECK(labels[0] == "D");
    CHECK(labels[1] == "C");
    CHECK(labels[2] == "B");
}

TEST_CASE("UndoStack dirty and markClean", "[Phase18][Stack]") {
    UndoStack stack;
    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    CHECK_FALSE(stack.isDirty());
    stack.push(make("A"));
    CHECK(stack.isDirty());

    stack.markClean();
    CHECK_FALSE(stack.isDirty());

    stack.push(make("B"));
    CHECK(stack.isDirty());

    stack.undo();
    CHECK_FALSE(stack.isDirty()); // Back to clean cursor
}

TEST_CASE("UndoStack transaction grouping", "[Phase18][Stack]") {
    int x = 0, y = 0;
    UndoStack stack;

    CHECK(stack.beginTransaction("Group"));
    CHECK(stack.isTransactionOpen());
    CHECK(stack.activeTransactionLabel() == "Group");

    stack.addToTransaction(UndoAction("X", UndoActionType::Property,
        [&]() { x = 10; return true; }, [&]() { x = 0; return true; }));
    stack.addToTransaction(UndoAction("Y", UndoActionType::Property,
        [&]() { y = 20; return true; }, [&]() { y = 0; return true; }));
    CHECK(stack.activeTransactionSize() == 2);

    CHECK(stack.commitTransaction());
    CHECK_FALSE(stack.isTransactionOpen());
    CHECK(stack.depth() == 1); // Single grouped entry
}

TEST_CASE("UndoStack transaction undo is atomic", "[Phase18][Stack]") {
    int x = 10, y = 20;
    UndoStack stack;

    stack.beginTransaction("Group");
    stack.addToTransaction(UndoAction("X", UndoActionType::Property,
        [&]() { x = 10; return true; }, [&]() { x = 0; return true; }));
    stack.addToTransaction(UndoAction("Y", UndoActionType::Property,
        [&]() { y = 20; return true; }, [&]() { y = 0; return true; }));
    stack.commitTransaction();

    CHECK(stack.undo());
    CHECK(x == 0);
    CHECK(y == 0);
}

TEST_CASE("UndoStack rejects double begin", "[Phase18][Stack]") {
    UndoStack stack;
    CHECK(stack.beginTransaction("A"));
    CHECK_FALSE(stack.beginTransaction("B")); // Already open
}

TEST_CASE("UndoStack discard transaction", "[Phase18][Stack]") {
    UndoStack stack;
    stack.beginTransaction("Group");
    stack.addToTransaction(UndoAction("X", UndoActionType::Generic,
        []() { return true; }, []() { return true; }));

    CHECK(stack.discardTransaction());
    CHECK_FALSE(stack.isTransactionOpen());
    CHECK(stack.depth() == 0); // Nothing pushed
}

TEST_CASE("UndoStack commit empty transaction fails", "[Phase18][Stack]") {
    UndoStack stack;
    stack.beginTransaction("Empty");
    CHECK_FALSE(stack.commitTransaction());
}

TEST_CASE("UndoStack statistics", "[Phase18][Stack]") {
    UndoStack stack;
    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    stack.push(make("A"));
    stack.push(make("B"));
    CHECK(stack.totalPushes() == 2);

    stack.undo();
    CHECK(stack.totalUndos() == 1);

    stack.redo();
    CHECK(stack.totalRedos() == 1);
}

TEST_CASE("UndoStack clear resets everything", "[Phase18][Stack]") {
    UndoStack stack;
    stack.push(UndoAction("A", UndoActionType::Generic,
        []() { return true; }, []() { return true; }));
    stack.undo();

    stack.clear();
    CHECK(stack.isEmpty());
    CHECK(stack.depth() == 0);
    CHECK_FALSE(stack.canUndo());
    CHECK_FALSE(stack.canRedo());
    CHECK(stack.totalPushes() == 0);
}

TEST_CASE("UndoStack undoDepth and redoDepth", "[Phase18][Stack]") {
    UndoStack stack;
    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    stack.push(make("A"));
    stack.push(make("B"));
    stack.push(make("C"));
    CHECK(stack.undoDepth() == 3);
    CHECK(stack.redoDepth() == 0);

    stack.undo();
    CHECK(stack.undoDepth() == 2);
    CHECK(stack.redoDepth() == 1);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — UndoManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("UndoManager starts empty", "[Phase18][Manager]") {
    UndoManager mgr;
    CHECK(mgr.stackCount() == 0);
    CHECK(mgr.activeStackName().empty());
    CHECK(mgr.activeStack() == nullptr);
    CHECK_FALSE(mgr.canUndo());
    CHECK_FALSE(mgr.canRedo());
}

TEST_CASE("UndoManager register and find stack", "[Phase18][Manager]") {
    UndoManager mgr;
    CHECK(mgr.registerStack("scene"));
    CHECK(mgr.isRegistered("scene"));
    CHECK(mgr.stackCount() == 1);
    CHECK(mgr.activeStackName() == "scene"); // First registered becomes active

    auto* stack = mgr.findStack("scene");
    REQUIRE(stack);
    CHECK(stack->isEmpty());
}

TEST_CASE("UndoManager rejects duplicate stack", "[Phase18][Manager]") {
    UndoManager mgr;
    mgr.registerStack("scene");
    CHECK_FALSE(mgr.registerStack("scene"));
}

TEST_CASE("UndoManager rejects empty name", "[Phase18][Manager]") {
    UndoManager mgr;
    CHECK_FALSE(mgr.registerStack(""));
}

TEST_CASE("UndoManager unregister stack", "[Phase18][Manager]") {
    UndoManager mgr;
    mgr.registerStack("a");
    mgr.registerStack("b");
    CHECK(mgr.activeStackName() == "a");

    CHECK(mgr.unregisterStack("a"));
    CHECK_FALSE(mgr.isRegistered("a"));
    CHECK(mgr.activeStackName() == "b"); // Fallback to first remaining
}

TEST_CASE("UndoManager set active stack", "[Phase18][Manager]") {
    UndoManager mgr;
    mgr.registerStack("a");
    mgr.registerStack("b");

    CHECK(mgr.setActiveStack("b"));
    CHECK(mgr.activeStackName() == "b");
    CHECK_FALSE(mgr.setActiveStack("nonexistent"));
}

TEST_CASE("UndoManager push/undo/redo on active stack", "[Phase18][Manager]") {
    int val = 0;
    UndoManager mgr;
    mgr.registerStack("main");

    UndoAction action("Set 42", UndoActionType::Property,
        [&]() { val = 42; return true; },
        [&]() { val = 0;  return true; });

    CHECK(mgr.push(action));
    CHECK(mgr.canUndo());

    CHECK(mgr.undo());
    CHECK(val == 0);
    CHECK(mgr.canRedo());

    CHECK(mgr.redo());
    CHECK(val == 42);
}

TEST_CASE("UndoManager stackNames", "[Phase18][Manager]") {
    UndoManager mgr;
    mgr.registerStack("a");
    mgr.registerStack("b");
    mgr.registerStack("c");

    auto names = mgr.stackNames();
    REQUIRE(names.size() == 3);
    CHECK(names[0] == "a");
    CHECK(names[1] == "b");
    CHECK(names[2] == "c");
}

TEST_CASE("UndoManager observers", "[Phase18][Manager]") {
    UndoManager mgr;
    mgr.registerStack("main");

    std::string lastLabel;
    bool lastIsUndo = false;

    size_t id = mgr.addObserver([&](const std::string& label, bool isUndo) {
        lastLabel = label;
        lastIsUndo = isUndo;
    });
    CHECK(id > 0);
    CHECK(mgr.observerCount() == 1);

    UndoAction action("Test", UndoActionType::Generic,
        []() { return true; }, []() { return true; });

    mgr.push(action);
    CHECK(lastLabel == "Test");
    CHECK_FALSE(lastIsUndo);

    mgr.undo();
    CHECK(lastLabel == "Test");
    CHECK(lastIsUndo);

    CHECK(mgr.removeObserver(id));
    CHECK(mgr.observerCount() == 0);
}

TEST_CASE("UndoManager clear resets all", "[Phase18][Manager]") {
    UndoManager mgr;
    mgr.registerStack("main");
    mgr.addObserver([](const std::string&, bool) {});

    mgr.clear();
    CHECK(mgr.stackCount() == 0);
    CHECK(mgr.observerCount() == 0);
    CHECK(mgr.activeStackName().empty());
}

TEST_CASE("UndoManager push without stack fails", "[Phase18][Manager]") {
    UndoManager mgr;
    UndoAction action("Test", UndoActionType::Generic,
        []() { return true; }, []() { return true; });
    CHECK_FALSE(mgr.push(action));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-step property undo", "[Phase18][Integration]") {
    int val = 0;
    UndoStack stack;

    auto makeSet = [&](int from, int to) {
        return UndoAction("Set " + std::to_string(to), UndoActionType::Property,
            [&, to]() { val = to; return true; },
            [&, from]() { val = from; return true; });
    };

    stack.push(makeSet(0, 10));
    stack.push(makeSet(10, 20));
    stack.push(makeSet(20, 30));
    CHECK(val == 0); // push doesn't execute

    // Manually set to simulate the actions having been applied
    val = 30;

    stack.undo(); CHECK(val == 20);
    stack.undo(); CHECK(val == 10);
    stack.undo(); CHECK(val == 0);
    CHECK_FALSE(stack.canUndo());

    stack.redo(); CHECK(val == 10);
    stack.redo(); CHECK(val == 20);
    stack.redo(); CHECK(val == 30);
    CHECK_FALSE(stack.canRedo());
}

TEST_CASE("Integration: transaction atomic undo with manager", "[Phase18][Integration]") {
    int x = 0, y = 0;
    UndoManager mgr;
    mgr.registerStack("scene");

    auto* stack = mgr.activeStack();
    REQUIRE(stack);

    stack->beginTransaction("Move Object");
    stack->addToTransaction(UndoAction("X", UndoActionType::Move,
        [&]() { x = 100; return true; }, [&]() { x = 0; return true; }));
    stack->addToTransaction(UndoAction("Y", UndoActionType::Move,
        [&]() { y = 200; return true; }, [&]() { y = 0; return true; }));
    stack->commitTransaction();

    x = 100; y = 200; // Simulate applied state

    CHECK(mgr.undo()); // Single undo undoes both
    CHECK(x == 0);
    CHECK(y == 0);

    CHECK(mgr.redo()); // Single redo redoes both
    CHECK(x == 100);
    CHECK(y == 200);
}

TEST_CASE("Integration: multi-stack manager", "[Phase18][Integration]") {
    UndoManager mgr;
    mgr.registerStack("scene");
    mgr.registerStack("material");

    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    // Push to scene stack
    mgr.setActiveStack("scene");
    mgr.push(make("Move A"));
    mgr.push(make("Move B"));

    // Push to material stack
    mgr.setActiveStack("material");
    mgr.push(make("Change Color"));

    // Verify independent stacks
    CHECK(mgr.findStack("scene")->depth() == 2);
    CHECK(mgr.findStack("material")->depth() == 1);

    // Undo only affects active stack (material)
    CHECK(mgr.undo());
    CHECK(mgr.findStack("material")->depth() == 1);
    CHECK(mgr.findStack("material")->undoDepth() == 0);
    CHECK(mgr.findStack("scene")->undoDepth() == 2); // Unchanged
}

TEST_CASE("Integration: observer notifications across operations", "[Phase18][Integration]") {
    UndoManager mgr;
    mgr.registerStack("main");

    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const std::string& label, bool isUndo) {
        log.push_back({label, isUndo});
    });

    auto make = [](const std::string& l) {
        return UndoAction(l, UndoActionType::Generic,
            []() { return true; }, []() { return true; });
    };

    mgr.push(make("A"));
    mgr.push(make("B"));
    mgr.undo();
    mgr.redo();

    REQUIRE(log.size() == 4);
    CHECK(log[0] == std::make_pair(std::string("A"), false));
    CHECK(log[1] == std::make_pair(std::string("B"), false));
    CHECK(log[2] == std::make_pair(std::string("B"), true));
    CHECK(log[3] == std::make_pair(std::string("B"), false));
}
