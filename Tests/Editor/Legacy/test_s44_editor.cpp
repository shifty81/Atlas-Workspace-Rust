#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── EditorSettings defaults ──────────────────────────────────────

TEST_CASE("EditorSettings default values", "[Editor][S44]") {
    EditorSettings s;
    REQUIRE(s.darkMode == true);
    REQUIRE(s.showGrid == true);
    REQUIRE(s.showGizmos == true);
    REQUIRE(s.cameraSpeed == 10.f);
    REQUIRE(s.autosave == true);
    REQUIRE(s.autosaveIntervalSecs == 300.f);
    REQUIRE(s.undoHistorySize == 100);
}

// ── EditorSettingsService ────────────────────────────────────────

TEST_CASE("EditorSettingsService reset restores defaults", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setDarkMode(false);
    svc.setCameraSpeed(50.f);
    svc.setShowGrid(false);
    svc.setSnapEnabled(true);

    svc.reset();

    REQUIRE(svc.settings().darkMode == true);
    REQUIRE(svc.settings().cameraSpeed == 10.f);
    REQUIRE(svc.settings().showGrid == true);
}

TEST_CASE("EditorSettingsService setDarkMode", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setDarkMode(false);
    REQUIRE(svc.settings().darkMode == false);
    svc.setDarkMode(true);
    REQUIRE(svc.settings().darkMode == true);
}

TEST_CASE("EditorSettingsService setShowGrid", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setShowGrid(false);
    REQUIRE(svc.settings().showGrid == false);
    svc.setShowGrid(true);
    REQUIRE(svc.settings().showGrid == true);
}

TEST_CASE("EditorSettingsService setSnapEnabled", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setSnapEnabled(true);
    REQUIRE(svc.settings().snap.enabled == true);
    svc.setSnapEnabled(false);
    REQUIRE(svc.settings().snap.enabled == false);
}

TEST_CASE("EditorSettingsService setCameraSpeed", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setCameraSpeed(25.f);
    REQUIRE(svc.settings().cameraSpeed == 25.f);
}

TEST_CASE("EditorSettingsService applyTheme dark mode", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setDarkMode(true);
    EditorTheme theme;
    svc.applyTheme(theme);
    EditorTheme darkTheme = EditorTheme::dark();
    REQUIRE(theme.panelBackground == darkTheme.panelBackground);
}

TEST_CASE("EditorSettingsService applyTheme light mode", "[Editor][S44]") {
    EditorSettingsService svc;
    svc.setDarkMode(false);
    EditorTheme theme;
    svc.applyTheme(theme);
    EditorTheme lightTheme = EditorTheme::light();
    REQUIRE(theme.panelBackground == lightTheme.panelBackground);
}

TEST_CASE("EditorSettingsService const settings accessor", "[Editor][S44]") {
    EditorSettingsService svc;
    const auto& csvc = svc;
    REQUIRE(csvc.settings().darkMode == true);
    REQUIRE(csvc.settings().cameraSpeed == 10.f);
}

// ── HotkeyBinding defaults ──────────────────────────────────────

TEST_CASE("HotkeyBinding default values", "[Editor][S44]") {
    HotkeyBinding b;
    REQUIRE(b.hotkey.empty());
    REQUIRE(b.commandName.empty());
}

// ── HotkeyDispatcher ────────────────────────────────────────────

TEST_CASE("HotkeyDispatcher bind and findCommand", "[Editor][S44]") {
    HotkeyDispatcher disp;
    disp.bind("Ctrl+Z", "Undo");
    disp.bind("Ctrl+Y", "Redo");

    REQUIRE(disp.findCommand("Ctrl+Z") == "Undo");
    REQUIRE(disp.findCommand("Ctrl+Y") == "Redo");
    REQUIRE(disp.findCommand("F12").empty());
}

TEST_CASE("HotkeyDispatcher unbind removes binding", "[Editor][S44]") {
    HotkeyDispatcher disp;
    disp.bind("Ctrl+S", "Save");
    disp.bind("Ctrl+Z", "Undo");

    disp.unbind("Ctrl+S");
    REQUIRE(disp.findCommand("Ctrl+S").empty());
    REQUIRE(disp.findCommand("Ctrl+Z") == "Undo");
}

TEST_CASE("HotkeyDispatcher bindingCount tracks entries", "[Editor][S44]") {
    HotkeyDispatcher disp;
    REQUIRE(disp.bindingCount() == 0);

    disp.bind("F5", "Run");
    disp.bind("F6", "Debug");
    REQUIRE(disp.bindingCount() == 2);

    disp.unbind("F5");
    REQUIRE(disp.bindingCount() == 1);
}

TEST_CASE("HotkeyDispatcher bindings returns vector", "[Editor][S44]") {
    HotkeyDispatcher disp;
    disp.bind("Ctrl+Shift+S", "SaveAll");
    disp.bind("F12", "GoToDefinition");

    const auto& bindings = disp.bindings();
    REQUIRE(bindings.size() == 2);
    REQUIRE(bindings[0].hotkey == "Ctrl+Shift+S");
    REQUIRE(bindings[0].commandName == "SaveAll");
    REQUIRE(bindings[1].hotkey == "F12");
    REQUIRE(bindings[1].commandName == "GoToDefinition");
}

TEST_CASE("HotkeyDispatcher unbind non-existent key is safe", "[Editor][S44]") {
    HotkeyDispatcher disp;
    disp.bind("Ctrl+A", "SelectAll");
    disp.unbind("Ctrl+B"); // no-op
    REQUIRE(disp.bindingCount() == 1);
    REQUIRE(disp.findCommand("Ctrl+A") == "SelectAll");
}
