#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── MenuItem ────────────────────────────────────────────────────

TEST_CASE("MenuItem separator creates a separator item", "[Editor][S47]") {
    MenuItem sep = MenuItem::separator();
    REQUIRE(sep.isSeparator == true);
    REQUIRE(sep.name.empty());
    REQUIRE(sep.command.empty());
    REQUIRE(sep.enabled == true);
}

TEST_CASE("MenuItem defaults", "[Editor][S47]") {
    MenuItem m;
    REQUIRE(m.name.empty());
    REQUIRE(m.command.empty());
    REQUIRE(m.hotkey.empty());
    REQUIRE(m.enabled == true);
    REQUIRE(m.isSeparator == false);
    REQUIRE(m.children.empty());
}

// ── MenuCategory ────────────────────────────────────────────────

TEST_CASE("MenuCategory addItem populates items list", "[Editor][S47]") {
    MenuCategory cat;
    cat.name = "File";
    cat.addItem("New", "cmd.new", "Ctrl+N");
    cat.addItem("Open", "cmd.open", "Ctrl+O");

    REQUIRE(cat.items.size() == 2);
    REQUIRE(cat.items[0].name == "New");
    REQUIRE(cat.items[0].command == "cmd.new");
    REQUIRE(cat.items[0].hotkey == "Ctrl+N");
    REQUIRE(cat.items[0].enabled == true);
    REQUIRE(cat.items[1].name == "Open");
}

TEST_CASE("MenuCategory addItem with disabled flag", "[Editor][S47]") {
    MenuCategory cat;
    cat.name = "Edit";
    cat.addItem("Paste", "cmd.paste", "Ctrl+V", false);

    REQUIRE(cat.items.size() == 1);
    REQUIRE(cat.items[0].enabled == false);
}

TEST_CASE("MenuCategory addSeparator inserts separator item", "[Editor][S47]") {
    MenuCategory cat;
    cat.name = "View";
    cat.addItem("ZoomIn", "cmd.zoomIn");
    cat.addSeparator();
    cat.addItem("ZoomOut", "cmd.zoomOut");

    REQUIRE(cat.items.size() == 3);
    REQUIRE(cat.items[1].isSeparator == true);
}

// ── MenuBar ─────────────────────────────────────────────────────

TEST_CASE("MenuBar addCategory and categoryCount", "[Editor][S47]") {
    MenuBar bar;
    REQUIRE(bar.categoryCount() == 0);

    bar.addCategory("File");
    bar.addCategory("Edit");
    REQUIRE(bar.categoryCount() == 2);
}

TEST_CASE("MenuBar findCategory returns pointer or nullptr", "[Editor][S47]") {
    MenuBar bar;
    auto& fileCat = bar.addCategory("File");
    fileCat.addItem("Save", "cmd.save", "Ctrl+S");

    REQUIRE(bar.findCategory("File") != nullptr);
    REQUIRE(bar.findCategory("File")->name == "File");
    REQUIRE(bar.findCategory("File")->items.size() == 1);
    REQUIRE(bar.findCategory("Missing") == nullptr);
}

TEST_CASE("MenuBar const findCategory", "[Editor][S47]") {
    MenuBar bar;
    bar.addCategory("Help");
    const MenuBar& cbar = bar;

    REQUIRE(cbar.findCategory("Help") != nullptr);
    REQUIRE(cbar.findCategory("Help")->name == "Help");
    REQUIRE(cbar.findCategory("Ghost") == nullptr);
}

TEST_CASE("MenuBar categories returns full list", "[Editor][S47]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.addCategory("Edit");
    bar.addCategory("View");

    const auto& cats = bar.categories();
    REQUIRE(cats.size() == 3);
    REQUIRE(cats[0].name == "File");
    REQUIRE(cats[1].name == "Edit");
    REQUIRE(cats[2].name == "View");
}

// ── StatusBarState ──────────────────────────────────────────────

TEST_CASE("StatusBarState default values", "[Editor][S47]") {
    StatusBarState state;
    REQUIRE(state.modeName.empty());
    REQUIRE(state.worldPath.empty());
    REQUIRE(state.isDirty == false);
    REQUIRE(state.selectionCount == 0);
    REQUIRE(state.fps == 0.f);
    REQUIRE(state.statusMessage.empty());
}

// ── EditorStatusBar ─────────────────────────────────────────────

TEST_CASE("EditorStatusBar update sets all fields", "[Editor][S47]") {
    EditorStatusBar bar;
    bar.update("Sculpt", "/world/main.nfw", true, 5, 60.f, "Ready");

    const auto& s = bar.state();
    REQUIRE(s.modeName == "Sculpt");
    REQUIRE(s.worldPath == "/world/main.nfw");
    REQUIRE(s.isDirty == true);
    REQUIRE(s.selectionCount == 5);
    REQUIRE(s.fps == 60.f);
    REQUIRE(s.statusMessage == "Ready");
}

TEST_CASE("EditorStatusBar buildText format with all fields", "[Editor][S47]") {
    EditorStatusBar bar;
    bar.update("Edit", "/project/level.nfw", true, 3, 60.f, "Saved");

    std::string text = bar.buildText();
    // mode | worldPath * | N selected | FPS | statusMessage
    REQUIRE(text.find("Edit") != std::string::npos);
    REQUIRE(text.find("/project/level.nfw") != std::string::npos);
    REQUIRE(text.find("*") != std::string::npos);
    REQUIRE(text.find("3 selected") != std::string::npos);
    REQUIRE(text.find("60 FPS") != std::string::npos);
    REQUIRE(text.find("Saved") != std::string::npos);
}

TEST_CASE("EditorStatusBar buildText without dirty flag", "[Editor][S47]") {
    EditorStatusBar bar;
    bar.update("Play", "/world/test.nfw", false, 0, 30.f);

    std::string text = bar.buildText();
    REQUIRE(text.find("Play") != std::string::npos);
    REQUIRE(text.find("*") == std::string::npos);
    REQUIRE(text.find("selected") == std::string::npos);
    REQUIRE(text.find("30 FPS") != std::string::npos);
}

TEST_CASE("EditorStatusBar buildText without world path", "[Editor][S47]") {
    EditorStatusBar bar;
    bar.update("Browse", "", false, 0, 45.f);

    std::string text = bar.buildText();
    REQUIRE(text.find("Browse") != std::string::npos);
    REQUIRE(text.find("45 FPS") != std::string::npos);
}
