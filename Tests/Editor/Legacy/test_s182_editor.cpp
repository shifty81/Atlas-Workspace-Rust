// S182 editor tests: UIDesignEditorV1, UndoRedoSystemV1, TypographySystemV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/UIDesignEditorV1.h"
#include "NF/Editor/UndoRedoSystemV1.h"
#include "NF/Editor/TypographySystemV1.h"

using namespace NF;
using Catch::Approx;

// ── UIDesignEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Uidv1Element validity", "[Editor][S182]") {
    Uidv1Element e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "MainPanel";
    REQUIRE(e.isValid());
}

TEST_CASE("UIDesignEditorV1 addElement and elementCount", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    REQUIRE(uid.elementCount() == 0);
    Uidv1Element e; e.id = 1; e.name = "Root";
    REQUIRE(uid.addElement(e));
    REQUIRE(uid.elementCount() == 1);
}

TEST_CASE("UIDesignEditorV1 addElement invalid fails", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    REQUIRE(!uid.addElement(Uidv1Element{}));
}

TEST_CASE("UIDesignEditorV1 addElement duplicate fails", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Element e; e.id = 1; e.name = "A";
    uid.addElement(e);
    REQUIRE(!uid.addElement(e));
}

TEST_CASE("UIDesignEditorV1 removeElement", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Element e; e.id = 2; e.name = "B";
    uid.addElement(e);
    REQUIRE(uid.removeElement(2));
    REQUIRE(uid.elementCount() == 0);
    REQUIRE(!uid.removeElement(2));
}

TEST_CASE("UIDesignEditorV1 addBinding and bindingCount", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Binding b; b.id = 1; b.elementId = 10; b.property = "text"; b.source = "model.label";
    REQUIRE(uid.addBinding(b));
    REQUIRE(uid.bindingCount() == 1);
}

TEST_CASE("UIDesignEditorV1 addBinding invalid fails", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    REQUIRE(!uid.addBinding(Uidv1Binding{}));
}

TEST_CASE("UIDesignEditorV1 removeBinding", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Binding b; b.id = 1; b.elementId = 5; b.property = "visible";
    uid.addBinding(b);
    REQUIRE(uid.removeBinding(1));
    REQUIRE(uid.bindingCount() == 0);
    REQUIRE(!uid.removeBinding(1));
}

TEST_CASE("UIDesignEditorV1 activeCount and lockedCount", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Element e1; e1.id = 1; e1.name = "A"; e1.state = Uidv1ElementState::Active;
    Uidv1Element e2; e2.id = 2; e2.name = "B"; e2.state = Uidv1ElementState::Locked;
    uid.addElement(e1); uid.addElement(e2);
    REQUIRE(uid.activeCount() == 1);
    REQUIRE(uid.lockedCount() == 1);
}

TEST_CASE("UIDesignEditorV1 countByElementType", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Element e1; e1.id = 1; e1.name = "A"; e1.type = Uidv1ElementType::Button;
    Uidv1Element e2; e2.id = 2; e2.name = "B"; e2.type = Uidv1ElementType::Label;
    uid.addElement(e1); uid.addElement(e2);
    REQUIRE(uid.countByElementType(Uidv1ElementType::Button) == 1);
    REQUIRE(uid.countByElementType(Uidv1ElementType::Label)  == 1);
}

TEST_CASE("UIDesignEditorV1 findElement returns ptr", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    Uidv1Element e; e.id = 5; e.name = "Header";
    uid.addElement(e);
    REQUIRE(uid.findElement(5) != nullptr);
    REQUIRE(uid.findElement(5)->name == "Header");
    REQUIRE(uid.findElement(99) == nullptr);
}

TEST_CASE("uidv1ElementTypeName covers all values", "[Editor][S182]") {
    REQUIRE(std::string(uidv1ElementTypeName(Uidv1ElementType::Panel))      == "Panel");
    REQUIRE(std::string(uidv1ElementTypeName(Uidv1ElementType::InputField)) == "InputField");
    REQUIRE(std::string(uidv1ElementTypeName(Uidv1ElementType::Dropdown))   == "Dropdown");
}

TEST_CASE("uidv1ElementStateName covers all values", "[Editor][S182]") {
    REQUIRE(std::string(uidv1ElementStateName(Uidv1ElementState::Draft))      == "Draft");
    REQUIRE(std::string(uidv1ElementStateName(Uidv1ElementState::Hidden))     == "Hidden");
    REQUIRE(std::string(uidv1ElementStateName(Uidv1ElementState::Deprecated)) == "Deprecated");
}

TEST_CASE("Uidv1Element state helpers", "[Editor][S182]") {
    Uidv1Element e; e.id = 1; e.name = "X";
    e.state = Uidv1ElementState::Active;
    REQUIRE(e.isActive());
    e.state = Uidv1ElementState::Locked;
    REQUIRE(e.isLocked());
}

TEST_CASE("UIDesignEditorV1 onChange callback", "[Editor][S182]") {
    UIDesignEditorV1 uid;
    uint64_t notified = 0;
    uid.setOnChange([&](uint64_t id) { notified = id; });
    Uidv1Element e; e.id = 8; e.name = "Footer";
    uid.addElement(e);
    REQUIRE(notified == 8);
}

// ── UndoRedoSystemV1 ─────────────────────────────────────────────────────────

TEST_CASE("Ursv1Action validity", "[Editor][S182]") {
    Ursv1Action a;
    REQUIRE(!a.isValid());
    a.id = 1; a.name = "MoveNode";
    REQUIRE(a.isValid());
}

TEST_CASE("UndoRedoSystemV1 addAction and actionCount", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    REQUIRE(urs.actionCount() == 0);
    Ursv1Action a; a.id = 1; a.name = "Create";
    REQUIRE(urs.addAction(a));
    REQUIRE(urs.actionCount() == 1);
}

TEST_CASE("UndoRedoSystemV1 addAction invalid fails", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    REQUIRE(!urs.addAction(Ursv1Action{}));
}

TEST_CASE("UndoRedoSystemV1 canUndo and canRedo initial state", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    REQUIRE(!urs.canUndo());
    REQUIRE(!urs.canRedo());
}

TEST_CASE("UndoRedoSystemV1 canUndo after addAction", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    Ursv1Action a; a.id = 1; a.name = "A";
    urs.addAction(a);
    REQUIRE(urs.canUndo());
    REQUIRE(!urs.canRedo());
}

TEST_CASE("UndoRedoSystemV1 undo reduces undoDepth", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    Ursv1Action a; a.id = 1; a.name = "A";
    urs.addAction(a);
    REQUIRE(urs.undoDepth() == 1);
    REQUIRE(urs.undo());
    REQUIRE(urs.undoDepth() == 0);
    REQUIRE(!urs.canUndo());
}

TEST_CASE("UndoRedoSystemV1 redo after undo", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    Ursv1Action a; a.id = 1; a.name = "A";
    urs.addAction(a);
    urs.undo();
    REQUIRE(urs.canRedo());
    REQUIRE(urs.redoDepth() == 1);
    REQUIRE(urs.redo());
    REQUIRE(!urs.canRedo());
}

TEST_CASE("UndoRedoSystemV1 undo fails when empty", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    REQUIRE(!urs.undo());
}

TEST_CASE("UndoRedoSystemV1 redo fails when empty", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    REQUIRE(!urs.redo());
}

TEST_CASE("UndoRedoSystemV1 addAction clears redo stack", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    Ursv1Action a1; a1.id = 1; a1.name = "A";
    Ursv1Action a2; a2.id = 2; a2.name = "B";
    urs.addAction(a1);
    urs.undo();
    REQUIRE(urs.canRedo());
    urs.addAction(a2);
    REQUIRE(!urs.canRedo());
}

TEST_CASE("UndoRedoSystemV1 countByActionType", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    Ursv1Action a1; a1.id = 1; a1.name = "A"; a1.type = Ursv1ActionType::Create;
    Ursv1Action a2; a2.id = 2; a2.name = "B"; a2.type = Ursv1ActionType::Move;
    urs.addAction(a1); urs.addAction(a2);
    REQUIRE(urs.countByActionType(Ursv1ActionType::Create) == 1);
    REQUIRE(urs.countByActionType(Ursv1ActionType::Move)   == 1);
}

TEST_CASE("UndoRedoSystemV1 clear resets all state", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    Ursv1Action a; a.id = 1; a.name = "A";
    urs.addAction(a);
    urs.clear();
    REQUIRE(urs.actionCount() == 0);
    REQUIRE(!urs.canUndo());
    REQUIRE(!urs.canRedo());
}

TEST_CASE("ursv1ActionTypeName covers all values", "[Editor][S182]") {
    REQUIRE(std::string(ursv1ActionTypeName(Ursv1ActionType::Delete))  == "Delete");
    REQUIRE(std::string(ursv1ActionTypeName(Ursv1ActionType::Modify))  == "Modify");
    REQUIRE(std::string(ursv1ActionTypeName(Ursv1ActionType::Group))   == "Group");
    REQUIRE(std::string(ursv1ActionTypeName(Ursv1ActionType::Ungroup)) == "Ungroup");
}

TEST_CASE("ursv1ActionStateName covers all values", "[Editor][S182]") {
    REQUIRE(std::string(ursv1ActionStateName(Ursv1ActionState::Pending)) == "Pending");
    REQUIRE(std::string(ursv1ActionStateName(Ursv1ActionState::Undone))  == "Undone");
    REQUIRE(std::string(ursv1ActionStateName(Ursv1ActionState::Merged))  == "Merged");
}

TEST_CASE("UndoRedoSystemV1 onChange callback", "[Editor][S182]") {
    UndoRedoSystemV1 urs;
    uint64_t notified = 0;
    urs.setOnChange([&](uint64_t id) { notified = id; });
    Ursv1Action a; a.id = 4; a.name = "D";
    urs.addAction(a);
    REQUIRE(notified == 4);
}

// ── TypographySystemV1 ───────────────────────────────────────────────────────

TEST_CASE("Typv1FontEntry validity", "[Editor][S182]") {
    Typv1FontEntry f;
    REQUIRE(!f.isValid());
    f.id = 1; f.family = "Inter";
    REQUIRE(f.isValid());
}

TEST_CASE("TypographySystemV1 addFont and fontCount", "[Editor][S182]") {
    TypographySystemV1 typ;
    REQUIRE(typ.fontCount() == 0);
    Typv1FontEntry f; f.id = 1; f.family = "Roboto";
    REQUIRE(typ.addFont(f));
    REQUIRE(typ.fontCount() == 1);
}

TEST_CASE("TypographySystemV1 addFont invalid fails", "[Editor][S182]") {
    TypographySystemV1 typ;
    REQUIRE(!typ.addFont(Typv1FontEntry{}));
}

TEST_CASE("TypographySystemV1 addFont duplicate fails", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1FontEntry f; f.id = 1; f.family = "A";
    typ.addFont(f);
    REQUIRE(!typ.addFont(f));
}

TEST_CASE("TypographySystemV1 removeFont", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1FontEntry f; f.id = 2; f.family = "B";
    typ.addFont(f);
    REQUIRE(typ.removeFont(2));
    REQUIRE(typ.fontCount() == 0);
    REQUIRE(!typ.removeFont(2));
}

TEST_CASE("TypographySystemV1 addScale and scaleCount", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1Scale s; s.id = 1; s.entryId = 10; s.sizePx = 18.f; s.lineHeight = 1.4f;
    REQUIRE(typ.addScale(s));
    REQUIRE(typ.scaleCount() == 1);
}

TEST_CASE("TypographySystemV1 addScale invalid fails", "[Editor][S182]") {
    TypographySystemV1 typ;
    REQUIRE(!typ.addScale(Typv1Scale{}));
}

TEST_CASE("TypographySystemV1 removeScale", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1Scale s; s.id = 1; s.entryId = 5; s.sizePx = 12.f;
    typ.addScale(s);
    REQUIRE(typ.removeScale(1));
    REQUIRE(typ.scaleCount() == 0);
    REQUIRE(!typ.removeScale(1));
}

TEST_CASE("TypographySystemV1 boldCount", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1FontEntry f1; f1.id = 1; f1.family = "A"; f1.weight = Typv1FontWeight::Bold;
    Typv1FontEntry f2; f2.id = 2; f2.family = "B"; f2.weight = Typv1FontWeight::Regular;
    typ.addFont(f1); typ.addFont(f2);
    REQUIRE(typ.boldCount() == 1);
}

TEST_CASE("TypographySystemV1 countByRole", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1FontEntry f1; f1.id = 1; f1.family = "A"; f1.role = Typv1TypeRole::Heading;
    Typv1FontEntry f2; f2.id = 2; f2.family = "B"; f2.role = Typv1TypeRole::Code;
    typ.addFont(f1); typ.addFont(f2);
    REQUIRE(typ.countByRole(Typv1TypeRole::Heading) == 1);
    REQUIRE(typ.countByRole(Typv1TypeRole::Code)    == 1);
}

TEST_CASE("TypographySystemV1 countByWeight", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1FontEntry f1; f1.id = 1; f1.family = "A"; f1.weight = Typv1FontWeight::Thin;
    Typv1FontEntry f2; f2.id = 2; f2.family = "B"; f2.weight = Typv1FontWeight::Medium;
    typ.addFont(f1); typ.addFont(f2);
    REQUIRE(typ.countByWeight(Typv1FontWeight::Thin)   == 1);
    REQUIRE(typ.countByWeight(Typv1FontWeight::Medium) == 1);
}

TEST_CASE("TypographySystemV1 findFont returns ptr", "[Editor][S182]") {
    TypographySystemV1 typ;
    Typv1FontEntry f; f.id = 7; f.family = "Poppins";
    typ.addFont(f);
    REQUIRE(typ.findFont(7) != nullptr);
    REQUIRE(typ.findFont(7)->family == "Poppins");
    REQUIRE(typ.findFont(99) == nullptr);
}

TEST_CASE("typv1FontWeightName covers all values", "[Editor][S182]") {
    REQUIRE(std::string(typv1FontWeightName(Typv1FontWeight::Thin))    == "Thin");
    REQUIRE(std::string(typv1FontWeightName(Typv1FontWeight::Light))   == "Light");
    REQUIRE(std::string(typv1FontWeightName(Typv1FontWeight::Black))   == "Black");
}

TEST_CASE("typv1TypeRoleName covers all values", "[Editor][S182]") {
    REQUIRE(std::string(typv1TypeRoleName(Typv1TypeRole::Display)) == "Display");
    REQUIRE(std::string(typv1TypeRoleName(Typv1TypeRole::Body))    == "Body");
    REQUIRE(std::string(typv1TypeRoleName(Typv1TypeRole::Caption)) == "Caption");
    REQUIRE(std::string(typv1TypeRoleName(Typv1TypeRole::Code))    == "Code");
}

TEST_CASE("Typv1FontEntry isBold helper", "[Editor][S182]") {
    Typv1FontEntry f; f.id = 1; f.family = "X";
    f.weight = Typv1FontWeight::Bold;
    REQUIRE(f.isBold());
    f.weight = Typv1FontWeight::Black;
    REQUIRE(f.isBold());
    f.weight = Typv1FontWeight::Regular;
    REQUIRE(!f.isBold());
}

TEST_CASE("TypographySystemV1 onChange callback", "[Editor][S182]") {
    TypographySystemV1 typ;
    uint64_t notified = 0;
    typ.setOnChange([&](uint64_t id) { notified = id; });
    Typv1FontEntry f; f.id = 5; f.family = "Ubuntu";
    typ.addFont(f);
    REQUIRE(notified == 5);
}
