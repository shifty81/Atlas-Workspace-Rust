// Tests/Workspace/test_phase11_command_bus.cpp
// Phase 11 — Command Bus and Action System
//
// Tests for:
//   1. WorkspaceCommand — descriptor, state, handlers, execute/undo, validity
//   2. CommandRegistry  — register, find, execute, hooks, state mutation, batch
//   3. CommandHistory   — push, undo, redo, groups, depth limit, labels
//   4. ActionBinding    — gesture types, ActionMap register/lookup/remove/serialize
//   5. Integration      — command bus pipeline: register → bind → execute → undo/redo

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceCommand.h"
#include "NF/Workspace/CommandRegistry.h"
#include "NF/Workspace/CommandHistory.h"
#include "NF/Workspace/ActionBinding.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — WorkspaceCommand
// ═════════════════════════════════════════════════════════════════

TEST_CASE("commandCategoryName returns correct strings", "[Phase11][Command]") {
    CHECK(std::string(commandCategoryName(CommandCategory::File))   == "File");
    CHECK(std::string(commandCategoryName(CommandCategory::Edit))   == "Edit");
    CHECK(std::string(commandCategoryName(CommandCategory::View))   == "View");
    CHECK(std::string(commandCategoryName(CommandCategory::Tools))  == "Tools");
    CHECK(std::string(commandCategoryName(CommandCategory::Window)) == "Window");
    CHECK(std::string(commandCategoryName(CommandCategory::Help))   == "Help");
    CHECK(std::string(commandCategoryName(CommandCategory::Custom)) == "Custom");
}

TEST_CASE("CommandState equality", "[Phase11][Command]") {
    CommandState a{true, true, false};
    CommandState b{true, true, false};
    CommandState c{false, true, false};
    CHECK(a == b);
    CHECK(a != c);
}

TEST_CASE("WorkspaceCommand default-constructed is not valid", "[Phase11][Command]") {
    WorkspaceCommand cmd;
    CHECK_FALSE(cmd.isValid());
}

TEST_CASE("WorkspaceCommand constructed with id+label is valid", "[Phase11][Command]") {
    WorkspaceCommand cmd{"file.save", "Save", CommandCategory::File};
    CHECK(cmd.isValid());
    CHECK(cmd.id()       == "file.save");
    CHECK(cmd.label()    == "Save");
    CHECK(cmd.category() == CommandCategory::File);
}

TEST_CASE("WorkspaceCommand setters update fields", "[Phase11][Command]") {
    WorkspaceCommand cmd{"v", "L"};
    cmd.setLabel("New Label");
    cmd.setTooltip("Save the current file");
    cmd.setShortcut("Ctrl+S");
    cmd.setIconKey("icon_save");
    cmd.setCategory(CommandCategory::File);

    CHECK(cmd.label()    == "New Label");
    CHECK(cmd.tooltip()  == "Save the current file");
    CHECK(cmd.shortcut() == "Ctrl+S");
    CHECK(cmd.iconKey()  == "icon_save");
    CHECK(cmd.category() == CommandCategory::File);
}

TEST_CASE("WorkspaceCommand default state is enabled and visible", "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    CHECK(cmd.isEnabled());
    CHECK(cmd.isVisible());
    CHECK_FALSE(cmd.isChecked());
}

TEST_CASE("WorkspaceCommand setState updates all flags", "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    cmd.setState({false, false, true});
    CHECK_FALSE(cmd.isEnabled());
    CHECK_FALSE(cmd.isVisible());
    CHECK(cmd.isChecked());
}

TEST_CASE("WorkspaceCommand setEnabled/setVisible/setChecked", "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    cmd.setEnabled(false);
    cmd.setVisible(false);
    cmd.setChecked(true);
    CHECK_FALSE(cmd.isEnabled());
    CHECK_FALSE(cmd.isVisible());
    CHECK(cmd.isChecked());
}

TEST_CASE("WorkspaceCommand execute returns false when disabled", "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    cmd.setHandler([] { return true; });
    cmd.setEnabled(false);
    CHECK_FALSE(cmd.execute());
}

TEST_CASE("WorkspaceCommand execute returns false with no handler", "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    CHECK_FALSE(cmd.hasHandler());
    CHECK_FALSE(cmd.execute());
}

TEST_CASE("WorkspaceCommand execute invokes handler and returns result",
          "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    int calls = 0;
    cmd.setHandler([&] { ++calls; return true; });
    CHECK(cmd.execute());
    CHECK(calls == 1);
}

TEST_CASE("WorkspaceCommand execute returns handler's false result",
          "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    cmd.setHandler([] { return false; });
    CHECK_FALSE(cmd.execute());
}

TEST_CASE("WorkspaceCommand isReversible and undo handler", "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    CHECK_FALSE(cmd.isReversible());
    cmd.setUndoHandler([] { return true; });
    CHECK(cmd.isReversible());
    CHECK(cmd.undo());
}

TEST_CASE("WorkspaceCommand undo returns false when no undo handler",
          "[Phase11][Command]") {
    WorkspaceCommand cmd{"x", "X"};
    CHECK_FALSE(cmd.undo());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — CommandRegistry
// ═════════════════════════════════════════════════════════════════

static WorkspaceCommand makeCommand(const std::string& id,
                                    const std::string& label = "L",
                                    CommandCategory cat = CommandCategory::Edit) {
    return WorkspaceCommand{id, label, cat};
}

static WorkspaceCommand makeCommandWithHandler(const std::string& id, bool result = true) {
    WorkspaceCommand cmd{id, id, CommandCategory::Edit};
    cmd.setHandler([result] { return result; });
    return cmd;
}

TEST_CASE("executeStatusName returns correct strings", "[Phase11][Registry]") {
    CHECK(std::string(executeStatusName(ExecuteStatus::Success))       == "Success");
    CHECK(std::string(executeStatusName(ExecuteStatus::NotFound))      == "NotFound");
    CHECK(std::string(executeStatusName(ExecuteStatus::Disabled))      == "Disabled");
    CHECK(std::string(executeStatusName(ExecuteStatus::NoHandler))     == "NoHandler");
    CHECK(std::string(executeStatusName(ExecuteStatus::HandlerFailed)) == "HandlerFailed");
}

TEST_CASE("CommandExecuteResult factory methods", "[Phase11][Registry]") {
    auto ok  = CommandExecuteResult::ok("cmd");
    auto nf  = CommandExecuteResult::notFound("cmd");
    auto dis = CommandExecuteResult::disabled("cmd");
    auto noh = CommandExecuteResult::noHandler("cmd");
    auto hf  = CommandExecuteResult::handlerFailed("cmd");

    CHECK(ok.succeeded());
    CHECK_FALSE(nf.succeeded());
    CHECK(nf.status == ExecuteStatus::NotFound);
    CHECK(dis.status == ExecuteStatus::Disabled);
    CHECK(noh.status == ExecuteStatus::NoHandler);
    CHECK(hf.status  == ExecuteStatus::HandlerFailed);
}

TEST_CASE("CommandRegistry empty on construction", "[Phase11][Registry]") {
    CommandRegistry reg;
    CHECK(reg.count() == 0);
    CHECK(reg.findById("x") == nullptr);
}

TEST_CASE("CommandRegistry registerCommand succeeds for valid command",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    CHECK(reg.registerCommand(makeCommand("edit.cut", "Cut")));
    CHECK(reg.count() == 1);
    CHECK(reg.isRegistered("edit.cut"));
}

TEST_CASE("CommandRegistry registerCommand rejects invalid command",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    CHECK_FALSE(reg.registerCommand(WorkspaceCommand{})); // no id or label
}

TEST_CASE("CommandRegistry registerCommand rejects duplicate id",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("x"));
    CHECK_FALSE(reg.registerCommand(makeCommand("x")));
    CHECK(reg.count() == 1);
}

TEST_CASE("CommandRegistry unregisterCommand removes command", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("edit.cut"));
    CHECK(reg.unregisterCommand("edit.cut"));
    CHECK_FALSE(reg.isRegistered("edit.cut"));
    CHECK(reg.count() == 0);
}

TEST_CASE("CommandRegistry unregisterCommand returns false for unknown id",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    CHECK_FALSE(reg.unregisterCommand("no-such-command"));
}

TEST_CASE("CommandRegistry findById returns correct pointer", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("file.save", "Save", CommandCategory::File));
    const auto* cmd = reg.findById("file.save");
    REQUIRE(cmd != nullptr);
    CHECK(cmd->label() == "Save");
}

TEST_CASE("CommandRegistry findByShortcut resolves enabled+visible command",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    auto cmd = makeCommand("file.save", "Save");
    cmd.setShortcut("Ctrl+S");
    reg.registerCommand(std::move(cmd));
    const auto* found = reg.findByShortcut("Ctrl+S");
    REQUIRE(found != nullptr);
    CHECK(found->id() == "file.save");
}

TEST_CASE("CommandRegistry findByShortcut returns null for disabled command",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    auto cmd = makeCommand("x");
    cmd.setShortcut("Ctrl+X");
    reg.registerCommand(std::move(cmd));
    reg.setEnabled("x", false);
    CHECK(reg.findByShortcut("Ctrl+X") == nullptr);
}

TEST_CASE("CommandRegistry findByCategory returns matching commands",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("file.save",  "Save",    CommandCategory::File));
    reg.registerCommand(makeCommand("file.open",  "Open",    CommandCategory::File));
    reg.registerCommand(makeCommand("edit.undo",  "Undo",    CommandCategory::Edit));
    auto fileCmds = reg.findByCategory(CommandCategory::File);
    CHECK(fileCmds.size() == 2);
    CHECK(reg.findByCategory(CommandCategory::View).empty());
}

TEST_CASE("CommandRegistry setEnabled/setVisible/setChecked", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("x"));
    CHECK(reg.setEnabled("x", false));
    CHECK_FALSE(reg.findById("x")->isEnabled());
    CHECK(reg.setVisible("x", false));
    CHECK_FALSE(reg.findById("x")->isVisible());
    CHECK(reg.setChecked("x", true));
    CHECK(reg.findById("x")->isChecked());
}

TEST_CASE("CommandRegistry setEnabled returns false for unknown id",
          "[Phase11][Registry]") {
    CommandRegistry reg;
    CHECK_FALSE(reg.setEnabled("no-such", true));
}

TEST_CASE("CommandRegistry execute success", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommandWithHandler("file.save"));
    auto res = reg.execute("file.save");
    CHECK(res.succeeded());
    CHECK(res.commandId == "file.save");
}

TEST_CASE("CommandRegistry execute NotFound", "[Phase11][Registry]") {
    CommandRegistry reg;
    auto res = reg.execute("missing");
    CHECK(res.status == ExecuteStatus::NotFound);
}

TEST_CASE("CommandRegistry execute Disabled", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommandWithHandler("x"));
    reg.setEnabled("x", false);
    CHECK(reg.execute("x").status == ExecuteStatus::Disabled);
}

TEST_CASE("CommandRegistry execute NoHandler", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("x"));
    CHECK(reg.execute("x").status == ExecuteStatus::NoHandler);
}

TEST_CASE("CommandRegistry execute HandlerFailed", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommandWithHandler("x", false));
    CHECK(reg.execute("x").status == ExecuteStatus::HandlerFailed);
}

TEST_CASE("CommandRegistry pre/post hooks fire on success", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommandWithHandler("x"));
    std::vector<std::string> log;
    reg.setPreHook([&](const std::string& id)  { log.push_back("pre:" + id); });
    reg.setPostHook([&](const std::string& id, ExecuteStatus s) {
        log.push_back("post:" + id + "=" + executeStatusName(s));
    });
    reg.execute("x");
    REQUIRE(log.size() == 2);
    CHECK(log[0] == "pre:x");
    CHECK(log[1] == "post:x=Success");
}

TEST_CASE("CommandRegistry hooks do NOT fire for NotFound", "[Phase11][Registry]") {
    CommandRegistry reg;
    int hookCalls = 0;
    reg.setPreHook([&](const std::string&) { ++hookCalls; });
    reg.execute("missing");
    CHECK(hookCalls == 0);
}

TEST_CASE("CommandRegistry clearHooks removes hooks", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommandWithHandler("x"));
    int pre = 0;
    reg.setPreHook([&](const std::string&) { ++pre; });
    reg.clearHooks();
    reg.execute("x");
    CHECK(pre == 0);
}

TEST_CASE("CommandRegistry enableAll/disableAll", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("a"));
    reg.registerCommand(makeCommand("b"));
    reg.disableAll();
    CHECK_FALSE(reg.findById("a")->isEnabled());
    CHECK_FALSE(reg.findById("b")->isEnabled());
    reg.enableAll();
    CHECK(reg.findById("a")->isEnabled());
    CHECK(reg.findById("b")->isEnabled());
}

TEST_CASE("CommandRegistry clear removes all commands", "[Phase11][Registry]") {
    CommandRegistry reg;
    reg.registerCommand(makeCommand("a"));
    reg.registerCommand(makeCommand("b"));
    reg.clear();
    CHECK(reg.count() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — CommandHistory
// ═════════════════════════════════════════════════════════════════

TEST_CASE("UndoRedoResult factory methods", "[Phase11][History]") {
    auto ok  = UndoRedoResult::ok("Save");
    auto nu  = UndoRedoResult::nothingToUndo();
    auto nr  = UndoRedoResult::nothingToRedo();
    auto hf  = UndoRedoResult::handlerFailed("X");

    CHECK(ok.succeeded());
    CHECK(ok.label == "Save");
    CHECK(nu.status == UndoRedoStatus::NothingToUndo);
    CHECK(nr.status == UndoRedoStatus::NothingToRedo);
    CHECK(hf.status == UndoRedoStatus::HandlerFailed);
}

TEST_CASE("CommandHistory empty on construction", "[Phase11][History]") {
    CommandHistory h;
    CHECK_FALSE(h.canUndo());
    CHECK_FALSE(h.canRedo());
    CHECK(h.undoDepth() == 0);
    CHECK(h.redoDepth() == 0);
}

TEST_CASE("CommandHistory push increases undoDepth", "[Phase11][History]") {
    CommandHistory h;
    h.push("cmd", "Save", [] { return true; });
    CHECK(h.canUndo());
    CHECK(h.undoDepth() == 1);
}

TEST_CASE("CommandHistory push rejects empty commandId", "[Phase11][History]") {
    CommandHistory h;
    CHECK_FALSE(h.push("", "label", [] { return true; }));
}

TEST_CASE("CommandHistory push rejects null undoFn", "[Phase11][History]") {
    CommandHistory h;
    CHECK_FALSE(h.push("cmd", "label", nullptr));
}

TEST_CASE("CommandHistory undo invokes undoFn and moves to redo",
          "[Phase11][History]") {
    CommandHistory h;
    bool undone = false;
    h.push("cmd", "Save", [&] { undone = true; return true; });
    auto res = h.undo();
    CHECK(res.succeeded());
    CHECK(undone);
    CHECK_FALSE(h.canUndo());
    CHECK(h.canRedo());
}

TEST_CASE("CommandHistory undo returns NothingToUndo when empty",
          "[Phase11][History]") {
    CommandHistory h;
    auto res = h.undo();
    CHECK(res.status == UndoRedoStatus::NothingToUndo);
}

TEST_CASE("CommandHistory redo moves entry back to undo stack", "[Phase11][History]") {
    CommandHistory h;
    h.push("cmd", "label", [] { return true; });
    h.undo();
    auto res = h.redo();
    CHECK(res.succeeded());
    CHECK(h.canUndo());
    CHECK_FALSE(h.canRedo());
}

TEST_CASE("CommandHistory redo returns NothingToRedo when empty",
          "[Phase11][History]") {
    CommandHistory h;
    CHECK(h.redo().status == UndoRedoStatus::NothingToRedo);
}

TEST_CASE("CommandHistory new push clears redo stack", "[Phase11][History]") {
    CommandHistory h;
    h.push("a", "A", [] { return true; });
    h.undo();
    CHECK(h.canRedo());
    h.push("b", "B", [] { return true; });
    CHECK_FALSE(h.canRedo());
}

TEST_CASE("CommandHistory nextUndoLabel and nextRedoLabel", "[Phase11][History]") {
    CommandHistory h;
    h.push("a", "Action A", [] { return true; });
    h.push("b", "Action B", [] { return true; });
    CHECK(h.nextUndoLabel() == "Action B");
    h.undo();
    CHECK(h.nextRedoLabel() == "Action B");
}

TEST_CASE("CommandHistory undoLabels returns newest-first", "[Phase11][History]") {
    CommandHistory h;
    h.push("a", "Alpha", [] { return true; });
    h.push("b", "Beta",  [] { return true; });
    h.push("c", "Gamma", [] { return true; });
    auto labels = h.undoLabels();
    REQUIRE(labels.size() == 3);
    CHECK(labels[0] == "Gamma"); // newest first
    CHECK(labels[2] == "Alpha");
}

TEST_CASE("CommandHistory max depth trims oldest entry", "[Phase11][History]") {
    CommandHistory h(3);
    h.push("a", "A", [] { return true; });
    h.push("b", "B", [] { return true; });
    h.push("c", "C", [] { return true; });
    h.push("d", "D", [] { return true; }); // should drop A
    CHECK(h.undoDepth() == 3);
    auto labels = h.undoLabels();
    CHECK(labels.back() == "B"); // A was dropped
}

TEST_CASE("CommandHistory clearHistory empties both stacks", "[Phase11][History]") {
    CommandHistory h;
    h.push("x", "X", [] { return true; });
    h.clearHistory();
    CHECK_FALSE(h.canUndo());
    CHECK_FALSE(h.canRedo());
}

TEST_CASE("CommandHistory setMaxDepth", "[Phase11][History]") {
    CommandHistory h(10);
    h.setMaxDepth(2);
    CHECK(h.maxDepth() == 2);
}

// ── Group tests ────────────────────────────────────────────────────

TEST_CASE("CommandHistory beginGroup / endGroup commits as single undo step",
          "[Phase11][History]") {
    CommandHistory h;
    std::vector<std::string> undone;
    h.beginGroup("Transform");
    h.push("move", "Move", [&] { undone.push_back("move"); return true; });
    h.push("scale", "Scale", [&] { undone.push_back("scale"); return true; });
    CHECK(h.endGroup());

    CHECK(h.undoDepth() == 1); // one group entry

    auto res = h.undo();
    CHECK(res.succeeded());
    // Both sub-entries should be undone (in reverse: scale then move)
    REQUIRE(undone.size() == 2);
    CHECK(undone[0] == "scale");
    CHECK(undone[1] == "move");
}

TEST_CASE("CommandHistory beginGroup rejects double-open", "[Phase11][History]") {
    CommandHistory h;
    CHECK(h.beginGroup("A"));
    CHECK_FALSE(h.beginGroup("B"));
    h.discardGroup();
}

TEST_CASE("CommandHistory endGroup returns false when no group open",
          "[Phase11][History]") {
    CommandHistory h;
    CHECK_FALSE(h.endGroup());
}

TEST_CASE("CommandHistory endGroup returns false for empty group",
          "[Phase11][History]") {
    CommandHistory h;
    h.beginGroup("empty");
    CHECK_FALSE(h.endGroup());
    CHECK_FALSE(h.isGroupOpen());
}

TEST_CASE("CommandHistory discardGroup clears without committing",
          "[Phase11][History]") {
    CommandHistory h;
    h.beginGroup("G");
    h.push("x", "X", [] { return true; });
    CHECK(h.discardGroup());
    CHECK_FALSE(h.isGroupOpen());
    CHECK(h.undoDepth() == 0);
}

TEST_CASE("CommandHistory openGroupName and openGroupSize", "[Phase11][History]") {
    CommandHistory h;
    h.beginGroup("MyGroup");
    h.push("x", "X", [] { return true; });
    h.push("y", "Y", [] { return true; });
    CHECK(h.openGroupName() == "MyGroup");
    CHECK(h.openGroupSize() == 2);
    h.discardGroup();
}

TEST_CASE("CommandHistory push to open group does not increment undoDepth",
          "[Phase11][History]") {
    CommandHistory h;
    h.beginGroup("G");
    h.push("x", "X", [] { return true; });
    CHECK(h.undoDepth() == 0); // not committed yet
    h.endGroup();
    CHECK(h.undoDepth() == 1);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — ActionBinding / ActionMap
// ═════════════════════════════════════════════════════════════════

TEST_CASE("gestureTypeName returns correct strings", "[Phase11][Action]") {
    CHECK(std::string(gestureTypeName(GestureType::Keyboard)) == "keyboard");
    CHECK(std::string(gestureTypeName(GestureType::Toolbar))  == "toolbar");
    CHECK(std::string(gestureTypeName(GestureType::MenuItem)) == "menu");
}

TEST_CASE("ActionBinding isValid", "[Phase11][Action]") {
    ActionBinding valid{"cmd.x", GestureType::Keyboard, "Ctrl+X"};
    ActionBinding noId {"",      GestureType::Keyboard, "Ctrl+X"};
    ActionBinding noKey{"cmd.x", GestureType::Keyboard, ""};
    CHECK(valid.isValid());
    CHECK_FALSE(noId.isValid());
    CHECK_FALSE(noKey.isValid());
}

TEST_CASE("ActionMap empty on construction", "[Phase11][Action]") {
    ActionMap m;
    CHECK(m.empty());
    CHECK(m.count() == 0);
}

TEST_CASE("ActionMap addKeyboardBinding", "[Phase11][Action]") {
    ActionMap m;
    CHECK(m.addKeyboardBinding("file.save", "Ctrl+S"));
    CHECK(m.count() == 1);
    CHECK(m.hasBinding("file.save"));
}

TEST_CASE("ActionMap addMenuBinding", "[Phase11][Action]") {
    ActionMap m;
    CHECK(m.addMenuBinding("file.save", "File/Save"));
    CHECK(m.count() == 1);
    CHECK(m.resolveMenu("File/Save") == "file.save");
}

TEST_CASE("ActionMap addToolbarBinding", "[Phase11][Action]") {
    ActionMap m;
    CHECK(m.addToolbarBinding("file.save", "main_toolbar", 0));
    CHECK(m.count() == 1);
    CHECK(m.resolveToolbar("main_toolbar", 0) == "file.save");
}

TEST_CASE("ActionMap rejects duplicate binding", "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("x", "Ctrl+X");
    CHECK_FALSE(m.addKeyboardBinding("x", "Ctrl+X")); // exact duplicate
    CHECK(m.count() == 1);
}

TEST_CASE("ActionMap allows multiple bindings for same command",
          "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("edit.undo", "Ctrl+Z");
    m.addMenuBinding("edit.undo", "Edit/Undo");
    CHECK(m.count() == 2);
    auto bindings = m.bindingsForCommand("edit.undo");
    CHECK(bindings.size() == 2);
}

TEST_CASE("ActionMap resolveKeyboard returns correct command id",
          "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("file.save", "Ctrl+S");
    m.addKeyboardBinding("edit.undo", "Ctrl+Z");
    CHECK(m.resolveKeyboard("Ctrl+S") == "file.save");
    CHECK(m.resolveKeyboard("Ctrl+Z") == "edit.undo");
    CHECK(m.resolveKeyboard("Ctrl+Q").empty());
}

TEST_CASE("ActionMap resolveGesture general lookup", "[Phase11][Action]") {
    ActionMap m;
    m.addMenuBinding("file.open", "File/Open");
    CHECK(m.resolveGesture(GestureType::MenuItem, "File/Open") == "file.open");
    CHECK(m.resolveGesture(GestureType::Keyboard, "File/Open").empty());
}

TEST_CASE("ActionMap removeBindingsForCommand removes all for that command",
          "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("x", "Ctrl+X");
    m.addMenuBinding("x", "Edit/X");
    m.addKeyboardBinding("y", "Ctrl+Y");
    size_t removed = m.removeBindingsForCommand("x");
    CHECK(removed == 2);
    CHECK(m.count() == 1);
    CHECK(m.hasBinding("y"));
    CHECK_FALSE(m.hasBinding("x"));
}

TEST_CASE("ActionMap removeBinding removes exact binding", "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("x", "Ctrl+X");
    m.addMenuBinding("x", "Edit/X");
    ActionBinding toRemove{"x", GestureType::Keyboard, "Ctrl+X"};
    CHECK(m.removeBinding(toRemove));
    CHECK(m.count() == 1);
    CHECK(m.resolveKeyboard("Ctrl+X").empty());
}

TEST_CASE("ActionMap bindingsByType filters correctly", "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("a", "Ctrl+A");
    m.addMenuBinding("b", "File/B");
    m.addKeyboardBinding("c", "Ctrl+C");
    auto kb = m.bindingsByType(GestureType::Keyboard);
    CHECK(kb.size() == 2);
    CHECK(m.bindingsByType(GestureType::Toolbar).empty());
}

TEST_CASE("ActionMap serialize and deserialize round-trip", "[Phase11][Action]") {
    ActionMap src;
    src.addKeyboardBinding("file.save",  "Ctrl+S");
    src.addMenuBinding("file.open",      "File/Open");
    src.addToolbarBinding("file.save",   "main_bar", 0);

    std::string text = src.serialize();

    ActionMap dst;
    CHECK(ActionMap::deserialize(text, dst));
    CHECK(dst.count() == 3);
    CHECK(dst.resolveKeyboard("Ctrl+S") == "file.save");
    CHECK(dst.resolveMenu("File/Open")  == "file.open");
    CHECK(dst.resolveToolbar("main_bar", 0) == "file.save");
}

TEST_CASE("ActionMap deserialize returns false for empty input",
          "[Phase11][Action]") {
    ActionMap m;
    CHECK_FALSE(ActionMap::deserialize("", m));
}

TEST_CASE("ActionMap clear removes all bindings", "[Phase11][Action]") {
    ActionMap m;
    m.addKeyboardBinding("x", "Ctrl+X");
    m.clear();
    CHECK(m.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: register → execute → undo via history",
          "[Phase11][Integration]") {
    // Setup
    CommandRegistry registry;
    CommandHistory   history;

    std::string state = "original";

    // Build a reversible "rename" command
    WorkspaceCommand rename{"scene.rename", "Rename Scene", CommandCategory::Edit};
    rename.setShortcut("F2");
    rename.setHandler([&] { state = "renamed"; return true; });
    rename.setUndoHandler([&] { state = "original"; return true; });

    registry.registerCommand(std::move(rename));

    // Execute via registry
    auto res = registry.execute("scene.rename");
    REQUIRE(res.succeeded());
    CHECK(state == "renamed");

    // Record in history using a captured undo fn
    const auto* cmd = registry.findById("scene.rename");
    REQUIRE(cmd != nullptr);
    history.push(cmd->id(), cmd->label(), [&] {
        return registry.findById("scene.rename")->undo();
    });

    // Undo
    auto undoRes = history.undo();
    CHECK(undoRes.succeeded());
    CHECK(state == "original");

    // Redo brings state back
    auto redoRes = history.redo();
    CHECK(redoRes.succeeded());
    // (Redo in linear model just re-queues for undo — state remains unchanged
    //  because redo here only repositions the stack pointer, not re-executes)
}

TEST_CASE("Integration: keyboard shortcut → command lookup → execute",
          "[Phase11][Integration]") {
    CommandRegistry registry;
    ActionMap       actionMap;

    int saved = 0;
    WorkspaceCommand save{"file.save", "Save", CommandCategory::File};
    save.setHandler([&] { ++saved; return true; });
    registry.registerCommand(std::move(save));

    actionMap.addKeyboardBinding("file.save", "Ctrl+S");

    // Simulate a key press
    std::string cmdId = actionMap.resolveKeyboard("Ctrl+S");
    REQUIRE(cmdId == "file.save");
    auto res = registry.execute(cmdId);
    CHECK(res.succeeded());
    CHECK(saved == 1);
}

TEST_CASE("Integration: group undo collapses multiple actions",
          "[Phase11][Integration]") {
    CommandHistory history;
    std::vector<std::string> undone;

    history.beginGroup("Bulk Edit");
    history.push("edit.a", "Edit A", [&] { undone.push_back("A"); return true; });
    history.push("edit.b", "Edit B", [&] { undone.push_back("B"); return true; });
    history.push("edit.c", "Edit C", [&] { undone.push_back("C"); return true; });
    history.endGroup();

    CHECK(history.undoDepth() == 1); // one group

    auto res = history.undo();
    CHECK(res.succeeded());
    CHECK(undone.size() == 3);
    // Reverse order: C, B, A
    CHECK(undone[0] == "C");
    CHECK(undone[1] == "B");
    CHECK(undone[2] == "A");
}

TEST_CASE("Integration: execute hook logs command execution with status",
           "[Phase11][Integration]") {
    CommandRegistry registry;
    std::vector<std::string> log;

    registry.setPostHook([&](const std::string& id, ExecuteStatus status) {
        log.push_back(id + ":" + executeStatusName(status));
    });

    WorkspaceCommand ok_cmd{"ok", "OK"};
    ok_cmd.setHandler([] { return true; });
    WorkspaceCommand bad_cmd{"bad", "Bad"};
    bad_cmd.setHandler([] { return false; });

    registry.registerCommand(std::move(ok_cmd));
    registry.registerCommand(std::move(bad_cmd));

    registry.execute("ok");
    registry.execute("bad");
    registry.execute("missing");  // NotFound — hook should NOT fire

    CHECK(log.size() == 2);
    CHECK(log[0] == "ok:Success");
    CHECK(log[1] == "bad:HandlerFailed");
}

TEST_CASE("Integration: action map serialize → deserialize → resolve → execute",
          "[Phase11][Integration]") {
    // Serialize an action map
    ActionMap src;
    src.addKeyboardBinding("view.fullscreen", "F11");
    std::string text = src.serialize();

    // Restore on the other side
    ActionMap dst;
    REQUIRE(ActionMap::deserialize(text, dst));

    CommandRegistry registry;
    bool toggled = false;
    WorkspaceCommand fullscreen{"view.fullscreen", "Full Screen", CommandCategory::View};
    fullscreen.setHandler([&] { toggled = true; return true; });
    registry.registerCommand(std::move(fullscreen));

    std::string cmdId = dst.resolveKeyboard("F11");
    REQUIRE(cmdId == "view.fullscreen");
    CHECK(registry.execute(cmdId).succeeded());
    CHECK(toggled);
}
