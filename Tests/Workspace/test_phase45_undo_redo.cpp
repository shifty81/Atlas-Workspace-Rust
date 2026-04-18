// Tests/Workspace/test_phase45_undo_redo.cpp
// Phase 45 — UndoRedoSystem
//
// Tests for:
//   1. UndoActionType  — enum name helpers
//   2. UndoActionState — enum name helpers
//   3. UndoAction      — apply/undo/invalidate; isApplied/isUndone/isValid/canUndo/canRedo
//   4. UndoGroup       — addAction/removeAction/find/applyAll/undoAll; actionCount/appliedCount
//   5. UndoRedoSystem  — pushGroup/undo/redo; canUndo/canRedo/undoDepth/redoDepth/clear
//   6. Integration     — multi-step undo/redo, branching (redo cleared on new push)

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/UndoRedoSystem.h"
#include <string>

using namespace NF;

// ── Helper ────────────────────────────────────────────────────────

static UndoAction makeAction(const std::string& id,
                              UndoActionType type = UndoActionType::Modify) {
    UndoAction a;
    a.id          = id;
    a.description = id + "_desc";
    a.type        = type;
    return a;
}

static UndoGroup makeGroup(const std::string& name,
                            std::initializer_list<std::string> ids) {
    UndoGroup g(name);
    for (const auto& id : ids) {
        UndoAction a = makeAction(id);
        a.apply();
        g.addAction(std::move(a));
    }
    return g;
}

// ─────────────────────────────────────────────────────────────────
// 1. UndoActionType
// ─────────────────────────────────────────────────────────────────

TEST_CASE("UndoActionType – all values have names", "[phase45][UndoActionType]") {
    CHECK(std::string(undoActionTypeName(UndoActionType::Create))   == "Create");
    CHECK(std::string(undoActionTypeName(UndoActionType::Delete))   == "Delete");
    CHECK(std::string(undoActionTypeName(UndoActionType::Move))     == "Move");
    CHECK(std::string(undoActionTypeName(UndoActionType::Resize))   == "Resize");
    CHECK(std::string(undoActionTypeName(UndoActionType::Rename))   == "Rename");
    CHECK(std::string(undoActionTypeName(UndoActionType::Modify))   == "Modify");
    CHECK(std::string(undoActionTypeName(UndoActionType::Group))    == "Group");
    CHECK(std::string(undoActionTypeName(UndoActionType::Ungroup))  == "Ungroup");
}

// ─────────────────────────────────────────────────────────────────
// 2. UndoActionState
// ─────────────────────────────────────────────────────────────────

TEST_CASE("UndoActionState – all values have names", "[phase45][UndoActionState]") {
    CHECK(std::string(undoActionStateName(UndoActionState::Pending))  == "Pending");
    CHECK(std::string(undoActionStateName(UndoActionState::Applied))  == "Applied");
    CHECK(std::string(undoActionStateName(UndoActionState::Undone))   == "Undone");
    CHECK(std::string(undoActionStateName(UndoActionState::Invalid))  == "Invalid");
}

// ─────────────────────────────────────────────────────────────────
// 3. UndoAction
// ─────────────────────────────────────────────────────────────────

TEST_CASE("UndoAction – default state is Pending", "[phase45][UndoAction]") {
    UndoAction a;
    CHECK(a.state == UndoActionState::Pending);
    CHECK_FALSE(a.isApplied());
    CHECK_FALSE(a.isUndone());
    CHECK(a.isValid());
    CHECK_FALSE(a.canUndo());
    CHECK_FALSE(a.canRedo());
}

TEST_CASE("UndoAction – apply transitions to Applied", "[phase45][UndoAction]") {
    UndoAction a = makeAction("a1");
    a.apply();
    CHECK(a.isApplied());
    CHECK(a.canUndo());
    CHECK_FALSE(a.canRedo());
}

TEST_CASE("UndoAction – undo from Applied transitions to Undone", "[phase45][UndoAction]") {
    UndoAction a = makeAction("a1");
    a.apply();
    a.undo();
    CHECK(a.isUndone());
    CHECK(a.canRedo());
    CHECK_FALSE(a.canUndo());
}

TEST_CASE("UndoAction – undo from non-Applied has no effect", "[phase45][UndoAction]") {
    UndoAction a = makeAction("a1");
    a.undo();  // still Pending
    CHECK(a.state == UndoActionState::Pending);
}

TEST_CASE("UndoAction – invalidate marks action invalid", "[phase45][UndoAction]") {
    UndoAction a = makeAction("a1");
    a.apply();
    a.invalidate();
    CHECK_FALSE(a.isValid());
    CHECK(a.state == UndoActionState::Invalid);
}

// ─────────────────────────────────────────────────────────────────
// 4. UndoGroup
// ─────────────────────────────────────────────────────────────────

TEST_CASE("UndoGroup – constructed with name, initially empty", "[phase45][UndoGroup]") {
    UndoGroup g("move_objects");
    CHECK(g.name()        == "move_objects");
    CHECK(g.actionCount() == 0);
    CHECK(g.appliedCount()== 0);
}

TEST_CASE("UndoGroup – addAction succeeds", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    CHECK(g.addAction(makeAction("a1")));
    CHECK(g.actionCount() == 1);
}

TEST_CASE("UndoGroup – addAction rejects duplicate id", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    g.addAction(makeAction("a1"));
    CHECK_FALSE(g.addAction(makeAction("a1")));
    CHECK(g.actionCount() == 1);
}

TEST_CASE("UndoGroup – removeAction succeeds", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    g.addAction(makeAction("a1"));
    CHECK(g.removeAction("a1"));
    CHECK(g.actionCount() == 0);
}

TEST_CASE("UndoGroup – removeAction unknown returns false", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    CHECK_FALSE(g.removeAction("ghost"));
}

TEST_CASE("UndoGroup – find returns mutable pointer", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    g.addAction(makeAction("a1", UndoActionType::Create));
    UndoAction* a = g.find("a1");
    REQUIRE(a != nullptr);
    CHECK(a->type == UndoActionType::Create);
}

TEST_CASE("UndoGroup – find returns nullptr for unknown", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    CHECK(g.find("ghost") == nullptr);
}

TEST_CASE("UndoGroup – applyAll marks all actions Applied", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    g.addAction(makeAction("a1"));
    g.addAction(makeAction("a2"));
    g.applyAll();
    CHECK(g.appliedCount() == 2);
}

TEST_CASE("UndoGroup – undoAll reverts all Applied actions", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    UndoAction a1 = makeAction("a1"); a1.apply();
    UndoAction a2 = makeAction("a2"); a2.apply();
    g.addAction(a1);
    g.addAction(a2);
    g.undoAll();
    CHECK(g.appliedCount() == 0);
    CHECK(g.find("a1")->isUndone());
    CHECK(g.find("a2")->isUndone());
}

TEST_CASE("UndoGroup – undoAll skips non-Applied actions", "[phase45][UndoGroup]") {
    UndoGroup g("g");
    UndoAction applied = makeAction("applied"); applied.apply();
    UndoAction pending = makeAction("pending");   // still Pending
    g.addAction(applied);
    g.addAction(pending);
    g.undoAll();
    CHECK(g.find("applied")->isUndone());
    CHECK(g.find("pending")->state == UndoActionState::Pending);  // unchanged
}

// ─────────────────────────────────────────────────────────────────
// 5. UndoRedoSystem
// ─────────────────────────────────────────────────────────────────

TEST_CASE("UndoRedoSystem – default empty state", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    CHECK_FALSE(sys.canUndo());
    CHECK_FALSE(sys.canRedo());
    CHECK(sys.undoDepth() == 0);
    CHECK(sys.redoDepth() == 0);
}

TEST_CASE("UndoRedoSystem – pushGroup enables undo", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    CHECK(sys.pushGroup(makeGroup("g1", {"a1", "a2"})));
    CHECK(sys.canUndo());
    CHECK_FALSE(sys.canRedo());
    CHECK(sys.undoDepth() == 1);
}

TEST_CASE("UndoRedoSystem – undo returns true and moves to redo", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    sys.pushGroup(makeGroup("g1", {"a1"}));
    CHECK(sys.undo());
    CHECK_FALSE(sys.canUndo());
    CHECK(sys.canRedo());
    CHECK(sys.undoDepth() == 0);
    CHECK(sys.redoDepth() == 1);
}

TEST_CASE("UndoRedoSystem – redo returns true and moves back to undo", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    sys.pushGroup(makeGroup("g1", {"a1"}));
    sys.undo();
    CHECK(sys.redo());
    CHECK(sys.canUndo());
    CHECK_FALSE(sys.canRedo());
    CHECK(sys.undoDepth() == 1);
    CHECK(sys.redoDepth() == 0);
}

TEST_CASE("UndoRedoSystem – undo when empty returns false", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    CHECK_FALSE(sys.undo());
}

TEST_CASE("UndoRedoSystem – redo when empty returns false", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    CHECK_FALSE(sys.redo());
}

TEST_CASE("UndoRedoSystem – pushGroup clears redo stack", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    sys.pushGroup(makeGroup("g1", {"a1"}));
    sys.undo();  // now redo has 1
    CHECK(sys.redoDepth() == 1);
    sys.pushGroup(makeGroup("g2", {"a2"}));  // must clear redo
    CHECK(sys.redoDepth() == 0);
    CHECK(sys.undoDepth() == 1);
}

TEST_CASE("UndoRedoSystem – multiple pushes stack correctly", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    sys.pushGroup(makeGroup("g1", {"a1"}));
    sys.pushGroup(makeGroup("g2", {"a2"}));
    sys.pushGroup(makeGroup("g3", {"a3"}));
    CHECK(sys.undoDepth() == 3);
}

TEST_CASE("UndoRedoSystem – undo moves from top of stack", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    sys.pushGroup(makeGroup("first",  {"a1"}));
    sys.pushGroup(makeGroup("second", {"a2"}));
    sys.undo();  // pops "second"
    CHECK(sys.undoDepth() == 1);
    CHECK(sys.redoDepth() == 1);
}

TEST_CASE("UndoRedoSystem – clear empties both stacks", "[phase45][UndoRedoSystem]") {
    UndoRedoSystem sys;
    sys.pushGroup(makeGroup("g1", {"a1"}));
    sys.pushGroup(makeGroup("g2", {"a2"}));
    sys.undo();
    sys.clear();
    CHECK(sys.undoDepth() == 0);
    CHECK(sys.redoDepth() == 0);
    CHECK_FALSE(sys.canUndo());
    CHECK_FALSE(sys.canRedo());
}

// ─────────────────────────────────────────────────────────────────
// Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("UndoRedoSystem integration – multi-step undo/redo cycle", "[phase45][integration]") {
    UndoRedoSystem sys;

    sys.pushGroup(makeGroup("step1", {"a1", "a2"}));
    sys.pushGroup(makeGroup("step2", {"b1"}));
    sys.pushGroup(makeGroup("step3", {"c1", "c2", "c3"}));

    CHECK(sys.undoDepth() == 3);

    // Undo all
    sys.undo(); sys.undo(); sys.undo();
    CHECK(sys.undoDepth() == 0);
    CHECK(sys.redoDepth() == 3);

    // Redo two
    sys.redo(); sys.redo();
    CHECK(sys.undoDepth() == 2);
    CHECK(sys.redoDepth() == 1);

    // Push new group – redo stack must be cleared
    sys.pushGroup(makeGroup("new_branch", {"x1"}));
    CHECK(sys.redoDepth() == 0);
    CHECK(sys.undoDepth() == 3);
}

TEST_CASE("UndoRedoSystem integration – group applyAll/undoAll through system", "[phase45][integration]") {
    UndoRedoSystem sys;

    UndoGroup g("move");
    UndoAction a1 = makeAction("m1"); a1.apply();
    UndoAction a2 = makeAction("m2"); a2.apply();
    g.addAction(a1);
    g.addAction(a2);

    sys.pushGroup(std::move(g));
    CHECK(sys.canUndo());

    // Undo should call undoAll on the group
    sys.undo();
    CHECK_FALSE(sys.canUndo());
    CHECK(sys.canRedo());

    // Redo should call applyAll on the group
    sys.redo();
    CHECK(sys.canUndo());
    CHECK_FALSE(sys.canRedo());
}
