#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- UndoActionType names ---

TEST_CASE("UndoActionType names cover all 8 values", "[Editor][S25]") {
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Create))   == "Create");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Delete))   == "Delete");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Move))     == "Move");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Resize))   == "Resize");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Rename))   == "Rename");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Modify))   == "Modify");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Group))    == "Group");
    REQUIRE(std::string(undoActionTypeName(UndoActionType::Ungroup))  == "Ungroup");
}

// --- UndoActionState names ---

TEST_CASE("UndoActionState names cover all 4 values", "[Editor][S25]") {
    REQUIRE(std::string(undoActionStateName(UndoActionState::Pending))  == "Pending");
    REQUIRE(std::string(undoActionStateName(UndoActionState::Applied))  == "Applied");
    REQUIRE(std::string(undoActionStateName(UndoActionState::Undone))   == "Undone");
    REQUIRE(std::string(undoActionStateName(UndoActionState::Invalid))  == "Invalid");
}

// --- UndoAction state transitions ---

TEST_CASE("UndoAction apply/undo state transitions", "[Editor][S25]") {
    UndoAction a;
    a.id = "act-1"; a.description = "Create node";

    REQUIRE(a.state == UndoActionState::Pending);
    a.apply();
    REQUIRE(a.state == UndoActionState::Applied);
    a.undo();
    REQUIRE(a.state == UndoActionState::Undone);
}

TEST_CASE("UndoAction canUndo only when Applied", "[Editor][S25]") {
    UndoAction a;
    a.id = "act-2";

    REQUIRE_FALSE(a.canUndo());
    a.apply();
    REQUIRE(a.canUndo());
    a.undo();
    REQUIRE_FALSE(a.canUndo());
}

TEST_CASE("UndoAction canRedo only when Undone", "[Editor][S25]") {
    UndoAction a;
    a.id = "act-3";

    REQUIRE_FALSE(a.canRedo());
    a.apply();
    REQUIRE_FALSE(a.canRedo());
    a.undo();
    REQUIRE(a.canRedo());
}

TEST_CASE("UndoAction invalidate makes isValid false", "[Editor][S25]") {
    UndoAction a;
    a.id = "act-4";

    REQUIRE(a.isValid());
    a.apply();
    REQUIRE(a.isValid());
    a.invalidate();
    REQUIRE_FALSE(a.isValid());
}

// --- UndoGroup ---

TEST_CASE("UndoGroup addAction and duplicate rejection", "[Editor][S25]") {
    UndoGroup g("group-1");
    UndoAction a; a.id = "a";
    UndoAction b; b.id = "b";
    UndoAction dup; dup.id = "a";

    REQUIRE(g.addAction(a));
    REQUIRE(g.addAction(b));
    REQUIRE_FALSE(g.addAction(dup));
    REQUIRE(g.actionCount() == 2);
}

TEST_CASE("UndoGroup applyAll sets all actions Applied", "[Editor][S25]") {
    UndoGroup g("group-2");
    UndoAction a; a.id = "a";
    UndoAction b; b.id = "b";
    g.addAction(a);
    g.addAction(b);

    g.applyAll();
    REQUIRE(g.find("a")->isApplied());
    REQUIRE(g.find("b")->isApplied());
}

TEST_CASE("UndoGroup undoAll sets Applied actions to Undone", "[Editor][S25]") {
    UndoGroup g("group-3");
    UndoAction a; a.id = "a";
    UndoAction b; b.id = "b";
    g.addAction(a);
    g.addAction(b);
    g.applyAll();

    g.undoAll();
    REQUIRE(g.find("a")->isUndone());
    REQUIRE(g.find("b")->isUndone());
}

TEST_CASE("UndoGroup appliedCount counts Applied actions", "[Editor][S25]") {
    UndoGroup g("group-4");
    UndoAction a; a.id = "a";
    UndoAction b; b.id = "b";
    UndoAction c; c.id = "c";
    g.addAction(a);
    g.addAction(b);
    g.addAction(c);

    REQUIRE(g.appliedCount() == 0);
    g.find("a")->apply();
    g.find("b")->apply();
    REQUIRE(g.appliedCount() == 2);
}

// --- UndoRedoSystem ---

TEST_CASE("UndoRedoSystem pushGroup + undo + redo cycle", "[Editor][S25]") {
    UndoRedoSystem sys;

    UndoGroup g("op-1");
    UndoAction a; a.id = "a"; a.apply();
    g.addAction(a);

    REQUIRE(sys.pushGroup(g));
    REQUIRE(sys.undoDepth() == 1);

    REQUIRE(sys.undo());
    REQUIRE(sys.undoDepth() == 0);
    REQUIRE(sys.redoDepth() == 1);

    REQUIRE(sys.redo());
    REQUIRE(sys.undoDepth() == 1);
    REQUIRE(sys.redoDepth() == 0);
}

TEST_CASE("UndoRedoSystem canUndo / canRedo state", "[Editor][S25]") {
    UndoRedoSystem sys;

    REQUIRE_FALSE(sys.canUndo());
    REQUIRE_FALSE(sys.canRedo());

    UndoGroup g("op-2");
    sys.pushGroup(g);
    REQUIRE(sys.canUndo());
    REQUIRE_FALSE(sys.canRedo());

    sys.undo();
    REQUIRE_FALSE(sys.canUndo());
    REQUIRE(sys.canRedo());
}

TEST_CASE("UndoRedoSystem redo stack clears on new pushGroup", "[Editor][S25]") {
    UndoRedoSystem sys;

    UndoGroup g1("op-1");
    sys.pushGroup(g1);
    sys.undo();
    REQUIRE(sys.redoDepth() == 1);

    UndoGroup g2("op-2");
    sys.pushGroup(g2);
    REQUIRE(sys.redoDepth() == 0);
}

TEST_CASE("UndoRedoSystem clear empties both stacks", "[Editor][S25]") {
    UndoRedoSystem sys;

    sys.pushGroup(UndoGroup("a"));
    sys.pushGroup(UndoGroup("b"));
    sys.undo();

    REQUIRE(sys.undoDepth() > 0);
    REQUIRE(sys.redoDepth() > 0);

    sys.clear();
    REQUIRE(sys.undoDepth() == 0);
    REQUIRE(sys.redoDepth() == 0);
}
