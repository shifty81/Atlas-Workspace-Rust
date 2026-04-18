// Tests/Workspace/test_phase63_command_palette.cpp
// Phase 63 — WorkspaceCommandPalette: CommandCategory, CommandEntry,
//             CommandHistory, CommandPalette
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceCommandPalette.h"

// ═════════════════════════════════════════════════════════════════
// CommandCategory
// ═════════════════════════════════════════════════════════════════

TEST_CASE("CommandCategory: name helpers", "[cmdpalette][category]") {
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::General))  == "General");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::Tool))     == "Tool");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::Edit))     == "Edit");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::View))     == "View");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::Build))    == "Build");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::Navigate)) == "Navigate");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::Debug))    == "Debug");
    REQUIRE(std::string(NF::commandCategoryName(NF::CommandCategory::Plugin))   == "Plugin");
}

// ═════════════════════════════════════════════════════════════════
// CommandEntry
// ═════════════════════════════════════════════════════════════════

static NF::CommandEntry makeCmd(const std::string& id, const std::string& label,
                                 NF::CommandCategory cat = NF::CommandCategory::General,
                                 bool success = true) {
    NF::CommandEntry cmd;
    cmd.id       = id;
    cmd.label    = label;
    cmd.category = cat;
    cmd.handler  = [success]() { return success; };
    return cmd;
}

TEST_CASE("CommandEntry: default invalid", "[cmdpalette][entry]") {
    NF::CommandEntry cmd;
    REQUIRE_FALSE(cmd.isValid());
}

TEST_CASE("CommandEntry: valid with id, label, handler", "[cmdpalette][entry]") {
    auto cmd = makeCmd("open", "Open File");
    REQUIRE(cmd.isValid());
}

TEST_CASE("CommandEntry: invalid without label", "[cmdpalette][entry]") {
    NF::CommandEntry cmd;
    cmd.id      = "x";
    cmd.handler = []() { return true; };
    REQUIRE_FALSE(cmd.isValid());
}

TEST_CASE("CommandEntry: invalid without handler", "[cmdpalette][entry]") {
    NF::CommandEntry cmd;
    cmd.id    = "x";
    cmd.label = "X";
    REQUIRE_FALSE(cmd.isValid());
}

TEST_CASE("CommandEntry: equality by id", "[cmdpalette][entry]") {
    auto a = makeCmd("a", "A");
    auto b = makeCmd("a", "B");
    auto c = makeCmd("c", "C");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("CommandEntry: enabled default true", "[cmdpalette][entry]") {
    auto cmd = makeCmd("x", "X");
    REQUIRE(cmd.enabled);
}

// ═════════════════════════════════════════════════════════════════
// CommandHistory
// ═════════════════════════════════════════════════════════════════

TEST_CASE("CommandHistory: default empty", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    REQUIRE(hist.empty());
    REQUIRE(hist.count() == 0);
    REQUIRE(hist.mostRecent() == nullptr);
}

TEST_CASE("CommandHistory: push and mostRecent", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    hist.push("cmd.open");
    hist.push("cmd.save");
    REQUIRE(hist.count() == 2);
    REQUIRE(*hist.mostRecent() == "cmd.save");
}

TEST_CASE("CommandHistory: push deduplicates to top", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    hist.push("cmd.open");
    hist.push("cmd.save");
    hist.push("cmd.open"); // re-push: moves to top
    REQUIRE(hist.count() == 2);
    REQUIRE(*hist.mostRecent() == "cmd.open");
}

TEST_CASE("CommandHistory: push empty string no-op", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    hist.push("");
    REQUIRE(hist.empty());
}

TEST_CASE("CommandHistory: contains", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    hist.push("cmd.open");
    REQUIRE(hist.contains("cmd.open"));
    REQUIRE_FALSE(hist.contains("cmd.close"));
}

TEST_CASE("CommandHistory: clear", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    hist.push("a");
    hist.push("b");
    hist.clear();
    REQUIRE(hist.empty());
}

TEST_CASE("CommandHistory: respects MAX_HISTORY", "[cmdpalette][history]") {
    NF::CommandHistory hist;
    for (int i = 0; i < NF::CommandHistory::MAX_HISTORY + 5; ++i) {
        hist.push("cmd." + std::to_string(i));
    }
    REQUIRE(hist.count() == NF::CommandHistory::MAX_HISTORY);
}

// ═════════════════════════════════════════════════════════════════
// CommandPalette
// ═════════════════════════════════════════════════════════════════

TEST_CASE("CommandPalette: default empty", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    REQUIRE(palette.commandCount() == 0);
    REQUIRE(palette.history().empty());
}

TEST_CASE("CommandPalette: registerCommand", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    REQUIRE(palette.registerCommand(makeCmd("open", "Open File")));
    REQUIRE(palette.commandCount() == 1);
    REQUIRE(palette.hasCommand("open"));
}

TEST_CASE("CommandPalette: registerCommand rejects invalid", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    NF::CommandEntry cmd;
    REQUIRE_FALSE(palette.registerCommand(cmd));
}

TEST_CASE("CommandPalette: registerCommand rejects duplicate", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("open", "Open"));
    REQUIRE_FALSE(palette.registerCommand(makeCmd("open", "Open2")));
}

TEST_CASE("CommandPalette: unregisterCommand", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("open", "Open"));
    REQUIRE(palette.unregisterCommand("open"));
    REQUIRE(palette.commandCount() == 0);
}

TEST_CASE("CommandPalette: unregisterCommand unknown", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    REQUIRE_FALSE(palette.unregisterCommand("missing"));
}

TEST_CASE("CommandPalette: setEnabled", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("open", "Open"));
    REQUIRE(palette.setEnabled("open", false));
    REQUIRE_FALSE(palette.find("open")->enabled);
    REQUIRE(palette.setEnabled("open", true));
    REQUIRE(palette.find("open")->enabled);
}

TEST_CASE("CommandPalette: setEnabled unknown", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    REQUIRE_FALSE(palette.setEnabled("missing", false));
}

TEST_CASE("CommandPalette: execute success", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("open", "Open", NF::CommandCategory::General, true));
    REQUIRE(palette.execute("open"));
    REQUIRE(palette.history().count() == 1);
    REQUIRE(*palette.history().mostRecent() == "open");
}

TEST_CASE("CommandPalette: execute failure", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("fail", "Fail", NF::CommandCategory::General, false));
    REQUIRE_FALSE(palette.execute("fail"));
    // Still recorded in history
    REQUIRE(palette.history().count() == 1);
}

TEST_CASE("CommandPalette: execute unknown", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    REQUIRE_FALSE(palette.execute("missing"));
}

TEST_CASE("CommandPalette: execute disabled command", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("save", "Save"));
    palette.setEnabled("save", false);
    REQUIRE_FALSE(palette.execute("save"));
    REQUIRE(palette.history().empty()); // disabled commands not added to history
}

TEST_CASE("CommandPalette: searchByLabel", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("file.open", "Open File", NF::CommandCategory::Tool));
    palette.registerCommand(makeCmd("file.save", "Save File", NF::CommandCategory::Tool));
    palette.registerCommand(makeCmd("edit.copy", "Copy Selection", NF::CommandCategory::Edit));

    auto results = palette.searchByLabel("file");
    REQUIRE(results.size() == 2);
}

TEST_CASE("CommandPalette: searchByLabel case-insensitive", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("build.run", "Run Build", NF::CommandCategory::Build));
    auto results = palette.searchByLabel("BUILD");
    REQUIRE(results.size() == 1);
}

TEST_CASE("CommandPalette: searchByLabel empty query", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("x", "X"));
    REQUIRE(palette.searchByLabel("").empty());
}

TEST_CASE("CommandPalette: filterByCategory", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("edit.cut",  "Cut",  NF::CommandCategory::Edit));
    palette.registerCommand(makeCmd("edit.copy", "Copy", NF::CommandCategory::Edit));
    palette.registerCommand(makeCmd("view.zoom", "Zoom", NF::CommandCategory::View));

    auto edits = palette.filterByCategory(NF::CommandCategory::Edit);
    REQUIRE(edits.size() == 2);
}

TEST_CASE("CommandPalette: findByShortcut", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    auto cmd = makeCmd("save", "Save");
    cmd.shortcut = "Ctrl+S";
    palette.registerCommand(cmd);

    auto results = palette.findByShortcut("Ctrl+S");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0]->id == "save");
}

TEST_CASE("CommandPalette: findByShortcut not found", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    auto results = palette.findByShortcut("Ctrl+Z");
    REQUIRE(results.empty());
}

TEST_CASE("CommandPalette: observer on execute", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("open", "Open", NF::CommandCategory::General, true));

    std::string lastCmd;
    bool lastResult = false;
    palette.addObserver([&](const std::string& id, bool ok) {
        lastCmd = id; lastResult = ok;
    });

    palette.execute("open");
    REQUIRE(lastCmd == "open");
    REQUIRE(lastResult == true);
}

TEST_CASE("CommandPalette: observer on execute failure", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("fail", "Fail", NF::CommandCategory::General, false));

    bool observedResult = true;
    palette.addObserver([&](const std::string&, bool ok) { observedResult = ok; });
    palette.execute("fail");
    REQUIRE_FALSE(observedResult);
}

TEST_CASE("CommandPalette: clearObservers", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("x", "X"));
    int calls = 0;
    palette.addObserver([&](const std::string&, bool) { ++calls; });
    palette.clearObservers();
    palette.execute("x");
    REQUIRE(calls == 0);
}

TEST_CASE("CommandPalette: serialize empty", "[cmdpalette][serial]") {
    NF::CommandPalette palette;
    REQUIRE(palette.serialize().empty());
}

TEST_CASE("CommandPalette: serialize round-trip metadata", "[cmdpalette][serial]") {
    NF::CommandPalette palette;
    auto cmd1 = makeCmd("open", "Open File", NF::CommandCategory::Tool);
    cmd1.shortcut    = "Ctrl+O";
    cmd1.description = "Opens a file";
    palette.registerCommand(cmd1);

    auto cmd2 = makeCmd("save", "Save");
    cmd2.shortcut = "Ctrl+S";
    cmd2.enabled  = false;
    palette.registerCommand(cmd2);

    std::string data = palette.serialize();
    REQUIRE_FALSE(data.empty());

    // Create a fresh palette with the same handlers and update metadata
    NF::CommandPalette palette2;
    palette2.registerCommand(makeCmd("open", "Open File"));
    palette2.registerCommand(makeCmd("save", "Save"));
    REQUIRE(palette2.deserializeMetadata(data));

    REQUIRE(palette2.find("open")->shortcut    == "Ctrl+O");
    REQUIRE(palette2.find("open")->description == "Opens a file");
    REQUIRE(palette2.find("open")->category    == NF::CommandCategory::Tool);
    REQUIRE(palette2.find("save")->enabled     == false);
}

TEST_CASE("CommandPalette: deserializeMetadata empty", "[cmdpalette][serial]") {
    NF::CommandPalette palette;
    REQUIRE(palette.deserializeMetadata(""));
}

TEST_CASE("CommandPalette: serialize pipe in label", "[cmdpalette][serial]") {
    NF::CommandPalette palette;
    auto cmd = makeCmd("x", "Option A|B");
    palette.registerCommand(cmd);

    std::string data = palette.serialize();
    NF::CommandPalette palette2;
    palette2.registerCommand(makeCmd("x", "placeholder"));
    palette2.deserializeMetadata(data);
    REQUIRE(palette2.find("x")->label == "Option A|B");
}

TEST_CASE("CommandPalette: clear", "[cmdpalette][main]") {
    NF::CommandPalette palette;
    palette.registerCommand(makeCmd("x", "X"));
    palette.execute("x");
    palette.clear();
    REQUIRE(palette.commandCount() == 0);
    REQUIRE(palette.history().empty());
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: command palette workflow", "[cmdpalette][integration]") {
    NF::CommandPalette palette;

    auto openCmd = makeCmd("file.open",  "Open File",    NF::CommandCategory::Tool, true);
    auto saveCmd = makeCmd("file.save",  "Save File",    NF::CommandCategory::Tool, true);
    auto copyCmd = makeCmd("edit.copy",  "Copy",         NF::CommandCategory::Edit, true);
    auto buildCmd = makeCmd("build.run", "Run Build",    NF::CommandCategory::Build, true);

    openCmd.shortcut  = "Ctrl+O";
    saveCmd.shortcut  = "Ctrl+S";
    copyCmd.shortcut  = "Ctrl+C";
    buildCmd.shortcut = "F5";

    palette.registerCommand(openCmd);
    palette.registerCommand(saveCmd);
    palette.registerCommand(copyCmd);
    palette.registerCommand(buildCmd);

    // Execute some commands
    palette.execute("file.open");
    palette.execute("file.save");
    palette.execute("file.open"); // duplicate: moves to front

    REQUIRE(palette.commandCount() == 4);
    REQUIRE(palette.history().count() == 2); // deduplicated
    REQUIRE(*palette.history().mostRecent() == "file.open");

    // Search
    REQUIRE(palette.searchByLabel("file").size() == 2);
    REQUIRE(palette.filterByCategory(NF::CommandCategory::Tool).size() == 2);
    REQUIRE(palette.findByShortcut("F5").size() == 1);
}

TEST_CASE("Integration: serialize/deserialize updates metadata", "[cmdpalette][integration]") {
    NF::CommandPalette palette;
    auto cmd = makeCmd("nav.goto", "Go To Definition", NF::CommandCategory::Navigate);
    cmd.shortcut    = "F12";
    cmd.description = "Navigate to symbol definition";
    palette.registerCommand(cmd);

    std::string data = palette.serialize();

    NF::CommandPalette palette2;
    palette2.registerCommand(makeCmd("nav.goto", "Go To Definition"));
    palette2.deserializeMetadata(data);

    REQUIRE(palette2.find("nav.goto")->shortcut    == "F12");
    REQUIRE(palette2.find("nav.goto")->description == "Navigate to symbol definition");
    REQUIRE(palette2.find("nav.goto")->category    == NF::CommandCategory::Navigate);
}
