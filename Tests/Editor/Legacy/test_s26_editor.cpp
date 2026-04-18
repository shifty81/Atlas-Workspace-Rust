#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- CommandPaletteCategory names ---

TEST_CASE("CommandPaletteCategory names cover all 8 values", "[Editor][S26]") {
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::File))     == "File");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::Edit))     == "Edit");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::View))     == "View");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::Navigate)) == "Navigate");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::Debug))    == "Debug");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::Build))    == "Build");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::Tools))    == "Tools");
    REQUIRE(std::string(commandPaletteCategoryName(CommandPaletteCategory::Help))     == "Help");
}

// --- CommandPaletteState names ---

TEST_CASE("CommandPaletteState names cover all 4 values", "[Editor][S26]") {
    REQUIRE(std::string(commandPaletteStateName(CommandPaletteState::Idle))      == "Idle");
    REQUIRE(std::string(commandPaletteStateName(CommandPaletteState::Open))      == "Open");
    REQUIRE(std::string(commandPaletteStateName(CommandPaletteState::Searching)) == "Searching");
    REQUIRE(std::string(commandPaletteStateName(CommandPaletteState::Executing)) == "Executing");
}

// --- PaletteCommand execute ---

TEST_CASE("PaletteCommand execute increments executeCount only when enabled", "[Editor][S26]") {
    PaletteCommand cmd;
    cmd.id = "cmd-1"; cmd.label = "Open File";

    REQUIRE(cmd.executeCount == 0);
    cmd.execute();
    REQUIRE(cmd.executeCount == 1);

    cmd.disable();
    cmd.execute();
    REQUIRE(cmd.executeCount == 1); // no increment when disabled
}

// --- PaletteCommand disable/enable ---

TEST_CASE("PaletteCommand disable/enable toggles isEnabled", "[Editor][S26]") {
    PaletteCommand cmd;
    cmd.id = "cmd-2"; cmd.label = "Save";

    REQUIRE(cmd.isEnabled());
    cmd.disable();
    REQUIRE_FALSE(cmd.isEnabled());
    cmd.enable();
    REQUIRE(cmd.isEnabled());
}

// --- PaletteCommand hasBeenExecuted ---

TEST_CASE("PaletteCommand hasBeenExecuted is false until first execute", "[Editor][S26]") {
    PaletteCommand cmd;
    cmd.id = "cmd-3"; cmd.label = "Build";

    REQUIRE_FALSE(cmd.hasBeenExecuted());
    cmd.execute();
    REQUIRE(cmd.hasBeenExecuted());
}

// --- PaletteCommand timesExecuted ---

TEST_CASE("PaletteCommand timesExecuted increments per call", "[Editor][S26]") {
    PaletteCommand cmd;
    cmd.id = "cmd-4"; cmd.label = "Run";

    REQUIRE(cmd.timesExecuted() == 0);
    cmd.execute();
    cmd.execute();
    cmd.execute();
    REQUIRE(cmd.timesExecuted() == 3);
}

// --- PaletteCommandGroup ---

TEST_CASE("PaletteCommandGroup addCommand and duplicate rejection", "[Editor][S26]") {
    PaletteCommandGroup grp("file-group");

    PaletteCommand a; a.id = "open";  a.label = "Open";
    PaletteCommand b; b.id = "close"; b.label = "Close";
    PaletteCommand dup; dup.id = "open"; dup.label = "Open Again";

    REQUIRE(grp.addCommand(a));
    REQUIRE(grp.addCommand(b));
    REQUIRE_FALSE(grp.addCommand(dup));
    REQUIRE(grp.commandCount() == 2);
}

TEST_CASE("PaletteCommandGroup enableAll / disableAll and enabledCount", "[Editor][S26]") {
    PaletteCommandGroup grp("edit-group");

    PaletteCommand a; a.id = "cut";   a.label = "Cut";
    PaletteCommand b; b.id = "copy";  b.label = "Copy";
    PaletteCommand c; c.id = "paste"; c.label = "Paste";
    grp.addCommand(a);
    grp.addCommand(b);
    grp.addCommand(c);

    REQUIRE(grp.enabledCount() == 3);
    grp.disableAll();
    REQUIRE(grp.enabledCount() == 0);
    grp.enableAll();
    REQUIRE(grp.enabledCount() == 3);
}

TEST_CASE("PaletteCommandGroup find returns correct pointer or nullptr", "[Editor][S26]") {
    PaletteCommandGroup grp("nav-group");

    PaletteCommand a; a.id = "goto-line"; a.label = "Go to Line";
    grp.addCommand(a);

    REQUIRE(grp.find("goto-line") != nullptr);
    REQUIRE(grp.find("goto-line")->label == "Go to Line");
    REQUIRE(grp.find("missing") == nullptr);
}

// --- CommandPalette ---

TEST_CASE("CommandPalette registerCommand and duplicate rejection", "[Editor][S26]") {
    CommandPalette palette;

    PaletteCommand a; a.id = "new-file";  a.label = "New File";
    PaletteCommand b; b.id = "new-file";  b.label = "New File Dup";
    PaletteCommand c; c.id = "open-file"; c.label = "Open File";

    REQUIRE(palette.registerCommand(a));
    REQUIRE(palette.registerCommand(c));
    // duplicate id
    PaletteCommand dup; dup.id = "open-file"; dup.label = "Open Again";
    REQUIRE_FALSE(palette.registerCommand(dup));
    REQUIRE(palette.commandCount() == 2);
}

TEST_CASE("CommandPalette execute by id and returns false for unknown", "[Editor][S26]") {
    CommandPalette palette;

    PaletteCommand cmd; cmd.id = "save"; cmd.label = "Save";
    palette.registerCommand(cmd);

    REQUIRE(palette.execute("save"));
    REQUIRE(palette.find("save")->timesExecuted() == 1);
    REQUIRE_FALSE(palette.execute("nonexistent"));
}

TEST_CASE("CommandPalette search returns matching commands", "[Editor][S26]") {
    CommandPalette palette;

    PaletteCommand a; a.id = "new-file";  a.label = "New File";
    PaletteCommand b; b.id = "new-proj";  b.label = "New Project";
    PaletteCommand c; c.id = "open-file"; c.label = "Open File";
    palette.registerCommand(a);
    palette.registerCommand(b);
    palette.registerCommand(c);

    auto results = palette.search("New");
    REQUIRE(results.size() == 2);

    auto single = palette.search("Open");
    REQUIRE(single.size() == 1);
    REQUIRE(single[0]->id == "open-file");

    auto none = palette.search("Zzz");
    REQUIRE(none.empty());
}

TEST_CASE("CommandPalette open/close changes state", "[Editor][S26]") {
    CommandPalette palette;

    REQUIRE(palette.state() == CommandPaletteState::Idle);
    palette.open();
    REQUIRE(palette.state() == CommandPaletteState::Open);
    palette.close();
    REQUIRE(palette.state() == CommandPaletteState::Idle);
}

TEST_CASE("CommandPalette enabledCount tracks enabled commands", "[Editor][S26]") {
    CommandPalette palette;

    PaletteCommand a; a.id = "cmd-a"; a.label = "Alpha";
    PaletteCommand b; b.id = "cmd-b"; b.label = "Beta";
    PaletteCommand c; c.id = "cmd-c"; c.label = "Gamma";
    palette.registerCommand(a);
    palette.registerCommand(b);
    palette.registerCommand(c);

    REQUIRE(palette.enabledCount() == 3);
    palette.find("cmd-b")->disable();
    REQUIRE(palette.enabledCount() == 2);
    palette.find("cmd-b")->enable();
    REQUIRE(palette.enabledCount() == 3);
}
