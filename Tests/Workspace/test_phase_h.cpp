// Tests/Workspace/test_phase_h.cpp
//
// Phase H — UX Completion
//
// H.1  — PreferencesUIController
//   - Open/close/isOpen
//   - Category navigation
//   - loadFrom(PreferenceRegistry) snapshots entries
//   - setValue / getValue
//   - resetEntry / resetAll
//   - dirty tracking (isDirty, dirtyCount)
//   - applyAll / revertAll
//   - exportSettings / importSettings
//   - entriesForCategory / activeEntries
//   - scope management (setScope / scopeOf)
//
// H.2  — KeybindEditorController
//   - loadFrom(ShortcutContext) snapshots bindings
//   - startCapture / captureKey / confirmCapture
//   - cancelCapture
//   - resetBinding / resetAll
//   - detectConflicts
//   - isDirty / dirtyCount
//   - applyAll / revertAll
//   - entriesForCategory
//   - captureState names
//
// H.3  — LayoutPersistenceController
//   - Built-in presets seeded on construction (Default/Compact/Wide)
//   - savePreset / hasPreset / userPresetCount
//   - loadPreset round-trip
//   - renamePreset
//   - deletePreset (cannot delete built-ins)
//   - allPresetNames includes built-ins
//   - loadBuiltIn
//   - auto-save on close / restore on open
//
// H.4  — CommandPaletteController
//   - open / close / toggle
//   - setQuery / query
//   - results (fuzzy search)
//   - recentResults from history
//   - moveSelectionUp / moveSelectionDown / selectIndex / selectedIndex
//   - executeSelected dispatches command
//   - closeOnExecute behaviour
//   - registerCommand / unregisterCommand
//   - BuiltInLayoutPreset names
//   - ProjectOpenFlowState names
//
// H.5  — NotificationCenterController
//   - open / close / isOpen
//   - post to history + toast
//   - postToast (toast-only)
//   - tick expires toasts
//   - dismiss / dismissAll
//   - setFilter / filtered (newest-first, respects filter)
//   - allHistory
//   - navigateTo invokes action
//   - historyCount / toastCount / undismissedCount / errorCount
//   - onPost callback
//   - NotifSeverity names
//
// H.6  — ProjectOpenFlowController
//   - beginFileOpen transitions to ChoosingFile
//   - selectFile transitions to Validating
//   - validate() — valid .atlas path succeeds
//   - validate() — invalid path returns errors
//   - confirmOpen after valid validate succeeds
//   - cancelOpen returns to Idle
//   - openRecentProject opens immediately if .atlas path
//   - recentProjects list grows after open
//   - clearRecents
//   - beginNewProjectWizard / setWizardTemplate / setWizardProjectName / setWizardProjectPath
//   - isWizardReady / createProject
//   - cancelWizard
//   - ProjectOpenFlowState names
//   - ProjectTemplateKind names

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/Editor/PreferencesUIController.h"
#include "NF/Editor/KeybindEditorController.h"
#include "NF/Editor/LayoutPersistenceController.h"
#include "NF/Editor/CommandPaletteController.h"
#include "NF/Editor/NotificationCenterController.h"
#include "NF/Editor/ProjectOpenFlowController.h"

#include <string>
#include <vector>

using namespace NF;
using Catch::Approx;

// ═══════════════════════════════════════════════════════════════════════════
//  H.1 — PreferencesUIController
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("PreferencesUIController default state", "[phase_h][h1][prefs]") {
    PreferencesUIController ctrl;
    REQUIRE_FALSE(ctrl.isOpen());
    REQUIRE(ctrl.entryCount() == 0u);
    REQUIRE(ctrl.activeCategory() == PreferenceCategory::General);
    REQUIRE_FALSE(ctrl.isDirty());
}

TEST_CASE("PreferencesUIController open/close", "[phase_h][h1][prefs]") {
    PreferencesUIController ctrl;
    ctrl.open();
    REQUIRE(ctrl.isOpen());
    ctrl.close();
    REQUIRE_FALSE(ctrl.isOpen());
}

TEST_CASE("PreferencesUIController category navigation", "[phase_h][h1][prefs]") {
    PreferencesUIController ctrl;
    ctrl.setActiveCategory(PreferenceCategory::Appearance);
    REQUIRE(ctrl.activeCategory() == PreferenceCategory::Appearance);
    ctrl.setActiveCategory(PreferenceCategory::Keybindings);
    REQUIRE(ctrl.activeCategory() == PreferenceCategory::Keybindings);
}

TEST_CASE("PreferencesUIController loadFrom registry", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark",
                                                   PreferenceCategory::Appearance));
    reg.registerEntry(PreferenceEntry::makeBool("autosave", "Auto Save", true,
                                                 PreferenceCategory::General));
    reg.registerEntry(PreferenceEntry::makeInt("font_size", "Font Size", 14, 8, 72,
                                               PreferenceCategory::Appearance));

    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    REQUIRE(ctrl.entryCount() == 3u);
    REQUIRE(ctrl.getValue("theme") == "dark");
    REQUIRE(ctrl.getValue("autosave") == "true");
    REQUIRE(ctrl.getValue("font_size") == "14");
}

TEST_CASE("PreferencesUIController setValue/getValue", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark",
                                                   PreferenceCategory::Appearance));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    REQUIRE(ctrl.setValue("theme", "light"));
    REQUIRE(ctrl.getValue("theme") == "light");
}

TEST_CASE("PreferencesUIController setValue unknown key returns false", "[phase_h][h1][prefs]") {
    PreferencesUIController ctrl;
    REQUIRE_FALSE(ctrl.setValue("nonexistent", "value"));
}

TEST_CASE("PreferencesUIController dirty tracking", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    reg.registerEntry(PreferenceEntry::makeBool("autosave", "Auto Save", true));

    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    REQUIRE_FALSE(ctrl.isDirty());
    REQUIRE(ctrl.dirtyCount() == 0u);

    ctrl.setValue("theme", "light");
    REQUIRE(ctrl.isDirty());
    REQUIRE(ctrl.dirtyCount() == 1u);
}

TEST_CASE("PreferencesUIController resetEntry", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    ctrl.setValue("theme", "light");
    REQUIRE(ctrl.isDirty());
    ctrl.resetEntry("theme");
    REQUIRE(ctrl.getValue("theme") == "dark");
    REQUIRE_FALSE(ctrl.isDirty());
}

TEST_CASE("PreferencesUIController resetAll", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    reg.registerEntry(PreferenceEntry::makeBool("autosave", "Auto Save", true));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    ctrl.setValue("theme", "light");
    ctrl.setValue("autosave", "false");
    REQUIRE(ctrl.dirtyCount() == 2u);
    ctrl.resetAll();
    REQUIRE_FALSE(ctrl.isDirty());
}

TEST_CASE("PreferencesUIController applyAll clears dirty", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    ctrl.setValue("theme", "light");
    REQUIRE(ctrl.isDirty());
    ctrl.applyAll();
    REQUIRE_FALSE(ctrl.isDirty());
    // After apply, value is "light" and committed
    REQUIRE(ctrl.getValue("theme") == "light");
}

TEST_CASE("PreferencesUIController revertAll restores committed", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    // Apply "light"
    ctrl.setValue("theme", "light");
    ctrl.applyAll();
    // Change to "solarized"
    ctrl.setValue("theme", "solarized");
    REQUIRE(ctrl.isDirty());
    // Revert goes back to committed = "light"
    ctrl.revertAll();
    REQUIRE(ctrl.getValue("theme") == "light");
    REQUIRE_FALSE(ctrl.isDirty());
}

TEST_CASE("PreferencesUIController exportSettings / importSettings", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    reg.registerEntry(PreferenceEntry::makeBool("autosave", "Auto Save", true));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    ctrl.setValue("theme", "light");
    ctrl.setValue("autosave", "false");

    std::string exported = ctrl.exportSettings();
    REQUIRE_FALSE(exported.empty());

    // Load fresh controller, import settings
    PreferencesUIController ctrl2;
    ctrl2.loadFrom(reg);
    REQUIRE(ctrl2.importSettings(exported));
    REQUIRE(ctrl2.getValue("theme") == "light");
    REQUIRE(ctrl2.getValue("autosave") == "false");
}

TEST_CASE("PreferencesUIController importSettings empty string returns false", "[phase_h][h1][prefs]") {
    PreferencesUIController ctrl;
    REQUIRE_FALSE(ctrl.importSettings(""));
}

TEST_CASE("PreferencesUIController entriesForCategory", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark",
                                                   PreferenceCategory::Appearance));
    reg.registerEntry(PreferenceEntry::makeString("lang", "Language", "en",
                                                   PreferenceCategory::General));
    reg.registerEntry(PreferenceEntry::makeInt("font_size", "Font Size", 14, 8, 72,
                                               PreferenceCategory::Appearance));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    auto appearance = ctrl.entriesForCategory(PreferenceCategory::Appearance);
    REQUIRE(appearance.size() == 2u);
    auto general = ctrl.entriesForCategory(PreferenceCategory::General);
    REQUIRE(general.size() == 1u);
}

TEST_CASE("PreferencesUIController scope management", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    REQUIRE(ctrl.scopeOf("theme") == PreferenceScopeType::User);
    ctrl.setScope("theme", PreferenceScopeType::Project);
    REQUIRE(ctrl.scopeOf("theme") == PreferenceScopeType::Project);
}

TEST_CASE("PreferencesUIController onApply callback fired", "[phase_h][h1][prefs]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    PreferencesUIController ctrl;
    ctrl.loadFrom(reg);
    int calls = 0;
    ctrl.setOnApply([&]{ ++calls; });
    ctrl.setValue("theme", "light");
    ctrl.applyAll();
    REQUIRE(calls == 1);
}

// ═══════════════════════════════════════════════════════════════════════════
//  H.2 — KeybindEditorController
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("KeybindCaptureState names", "[phase_h][h2][keybind]") {
    REQUIRE(std::string(keybindCaptureStateName(KeybindCaptureState::Idle))      == "Idle");
    REQUIRE(std::string(keybindCaptureStateName(KeybindCaptureState::Capturing)) == "Capturing");
    REQUIRE(std::string(keybindCaptureStateName(KeybindCaptureState::Captured))  == "Captured");
    REQUIRE(std::string(keybindCaptureStateName(KeybindCaptureState::Cancelled)) == "Cancelled");
}

TEST_CASE("KeybindEditorController default state", "[phase_h][h2][keybind]") {
    KeybindEditorController ctrl;
    REQUIRE(ctrl.entryCount() == 0u);
    REQUIRE_FALSE(ctrl.isDirty());
    REQUIRE(ctrl.captureState() == KeybindCaptureState::Idle);
}

TEST_CASE("KeybindEditorController loadFrom ShortcutContext", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b1; b1.id = "save";    b1.name = "Save";    b1.key = "S"; b1.modifiers = 1;
    ShortcutBinding b2; b2.id = "undo";    b2.name = "Undo";    b2.key = "Z"; b2.modifiers = 1;
    ShortcutBinding b3; b3.id = "redo";    b3.name = "Redo";    b3.key = "Y"; b3.modifiers = 1;
    ctx.addBinding(b1); ctx.addBinding(b2); ctx.addBinding(b3);

    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);
    REQUIRE(ctrl.entryCount() == 3u);
    REQUIRE(ctrl.contextName() == "global");
    REQUIRE_FALSE(ctrl.isDirty()); // snapshot = defaults, no changes yet
}

TEST_CASE("KeybindEditorController startCapture / captureKey / confirmCapture", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b; b.id = "save"; b.name = "Save"; b.key = "S"; b.modifiers = 1;
    ctx.addBinding(b);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);

    REQUIRE(ctrl.startCapture("save"));
    REQUIRE(ctrl.captureState() == KeybindCaptureState::Capturing);
    REQUIRE(ctrl.captureTarget() == "save");

    REQUIRE(ctrl.captureKey("W", 0));
    REQUIRE(ctrl.captureState() == KeybindCaptureState::Captured);
    REQUIRE(ctrl.pendingKey() == "W");

    REQUIRE(ctrl.confirmCapture());
    REQUIRE(ctrl.captureState() == KeybindCaptureState::Idle);
    REQUIRE(ctrl.captureTarget().empty());

    // The binding should now have key "W"
    const auto& entries = ctrl.entries();
    REQUIRE(entries[0].currentKey == "W");
    REQUIRE(ctrl.isDirty()); // changed from default "S"
}

TEST_CASE("KeybindEditorController cancelCapture", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b; b.id = "save"; b.name = "Save"; b.key = "S"; b.modifiers = 1;
    ctx.addBinding(b);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);

    ctrl.startCapture("save");
    ctrl.captureKey("X", 0);
    ctrl.cancelCapture();
    REQUIRE(ctrl.captureState() == KeybindCaptureState::Idle);
    // Key should remain at default "S" (cancel did not confirm)
    REQUIRE(ctrl.entries()[0].currentKey == "S");
}

TEST_CASE("KeybindEditorController resetBinding", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b; b.id = "save"; b.name = "Save"; b.key = "S"; b.modifiers = 1;
    ctx.addBinding(b);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);

    ctrl.startCapture("save");
    ctrl.captureKey("W", 0);
    ctrl.confirmCapture();
    REQUIRE(ctrl.isDirty());

    ctrl.resetBinding("save");
    REQUIRE(ctrl.entries()[0].currentKey == "S");
    REQUIRE_FALSE(ctrl.isDirty());
}

TEST_CASE("KeybindEditorController resetAll", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b1; b1.id = "save"; b1.name = "Save"; b1.key = "S"; b1.modifiers = 1;
    ShortcutBinding b2; b2.id = "undo"; b2.name = "Undo"; b2.key = "Z"; b2.modifiers = 1;
    ctx.addBinding(b1); ctx.addBinding(b2);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);

    ctrl.startCapture("save"); ctrl.captureKey("A", 0); ctrl.confirmCapture();
    ctrl.startCapture("undo"); ctrl.captureKey("B", 0); ctrl.confirmCapture();
    REQUIRE(ctrl.dirtyCount() == 2u);
    ctrl.resetAll();
    REQUIRE_FALSE(ctrl.isDirty());
}

TEST_CASE("KeybindEditorController detectConflicts", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b1; b1.id = "save"; b1.name = "Save"; b1.key = "S"; b1.modifiers = 1;
    ShortcutBinding b2; b2.id = "undo"; b2.name = "Undo"; b2.key = "Z"; b2.modifiers = 1;
    ctx.addBinding(b1); ctx.addBinding(b2);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);

    // No conflict initially
    REQUIRE(ctrl.detectConflicts().empty());
    REQUIRE_FALSE(ctrl.hasConflicts());

    // Make both bindings use "S" with modifiers=1 → conflict
    ctrl.startCapture("undo"); ctrl.captureKey("S", 1); ctrl.confirmCapture();
    auto conflicts = ctrl.detectConflicts();
    REQUIRE(conflicts.size() == 1u);
    REQUIRE(ctrl.hasConflicts());
    REQUIRE(ctrl.entries()[0].hasConflict);
    REQUIRE(ctrl.entries()[1].hasConflict);
}

TEST_CASE("KeybindEditorController applyAll commits", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b; b.id = "save"; b.name = "Save"; b.key = "S"; b.modifiers = 1;
    ctx.addBinding(b);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);
    ctrl.startCapture("save"); ctrl.captureKey("W", 0); ctrl.confirmCapture();
    REQUIRE(ctrl.isDirty());
    ctrl.applyAll();
    REQUIRE_FALSE(ctrl.isDirty()); // W is now the committed default
}

TEST_CASE("KeybindEditorController revertAll restores defaults", "[phase_h][h2][keybind]") {
    ShortcutContext ctx("global");
    ShortcutBinding b; b.id = "save"; b.name = "Save"; b.key = "S"; b.modifiers = 1;
    ctx.addBinding(b);
    KeybindEditorController ctrl;
    ctrl.loadFrom(ctx);
    ctrl.startCapture("save"); ctrl.captureKey("W", 0); ctrl.confirmCapture();
    ctrl.revertAll();
    REQUIRE(ctrl.entries()[0].currentKey == "S");
    REQUIRE_FALSE(ctrl.isDirty());
}

// ═══════════════════════════════════════════════════════════════════════════
//  H.3 — LayoutPersistenceController
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("BuiltInLayoutPreset names", "[phase_h][h3][layout]") {
    REQUIRE(std::string(builtInLayoutPresetName(BuiltInLayoutPreset::Default)) == "Default");
    REQUIRE(std::string(builtInLayoutPresetName(BuiltInLayoutPreset::Compact)) == "Compact");
    REQUIRE(std::string(builtInLayoutPresetName(BuiltInLayoutPreset::Wide))    == "Wide");
}

TEST_CASE("LayoutPersistenceController built-in presets on construction", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    REQUIRE(ctrl.hasPreset("Default"));
    REQUIRE(ctrl.hasPreset("Compact"));
    REQUIRE(ctrl.hasPreset("Wide"));
    REQUIRE(ctrl.userPresetCount() == 0u);
    REQUIRE(ctrl.totalPresetCount() == 3u);
}

TEST_CASE("LayoutPersistenceController allPresetNames includes built-ins", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    auto names = ctrl.allPresetNames();
    REQUIRE(names.size() == 3u);
    REQUIRE(std::find(names.begin(), names.end(), "Default") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "Compact") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "Wide")    != names.end());
}

TEST_CASE("LayoutPersistenceController savePreset and hasPreset", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    WorkspaceLayout layout("MyLayout");
    LayoutPanel p; p.id = "vp"; p.title = "Viewport"; p.type = LayoutPanelType::Viewport;
    layout.addPanel(p);
    REQUIRE(ctrl.savePreset("MyLayout", layout));
    REQUIRE(ctrl.hasPreset("MyLayout"));
    REQUIRE(ctrl.userPresetCount() == 1u);
}

TEST_CASE("LayoutPersistenceController cannot save to built-in name", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    WorkspaceLayout layout("Default");
    REQUIRE_FALSE(ctrl.savePreset("Default", layout));
    REQUIRE_FALSE(ctrl.savePreset("Compact", layout));
}

TEST_CASE("LayoutPersistenceController loadPreset round-trip", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    WorkspaceLayout layout("MyLayout");
    ctrl.savePreset("MyLayout", layout);

    WorkspaceLayout out("tmp");
    REQUIRE(ctrl.loadPreset("MyLayout", out));
}

TEST_CASE("LayoutPersistenceController loadBuiltIn", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    WorkspaceLayout out("tmp");
    REQUIRE(ctrl.loadBuiltIn(BuiltInLayoutPreset::Default, out));
    REQUIRE(ctrl.loadBuiltIn(BuiltInLayoutPreset::Compact, out));
    REQUIRE(ctrl.loadBuiltIn(BuiltInLayoutPreset::Wide,    out));
}

TEST_CASE("LayoutPersistenceController renamePreset", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    WorkspaceLayout layout("Original");
    ctrl.savePreset("Original", layout);
    REQUIRE(ctrl.hasPreset("Original"));
    REQUIRE(ctrl.renamePreset("Original", "Renamed"));
    REQUIRE_FALSE(ctrl.hasPreset("Original"));
    REQUIRE(ctrl.hasPreset("Renamed"));
}

TEST_CASE("LayoutPersistenceController cannot rename built-in", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    REQUIRE_FALSE(ctrl.renamePreset("Default", "Custom"));
    REQUIRE(ctrl.hasPreset("Default")); // still exists
}

TEST_CASE("LayoutPersistenceController deletePreset", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    WorkspaceLayout layout("Temp");
    ctrl.savePreset("Temp", layout);
    REQUIRE(ctrl.hasPreset("Temp"));
    REQUIRE(ctrl.deletePreset("Temp"));
    REQUIRE_FALSE(ctrl.hasPreset("Temp"));
    REQUIRE(ctrl.userPresetCount() == 0u);
}

TEST_CASE("LayoutPersistenceController cannot delete built-in", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    REQUIRE_FALSE(ctrl.deletePreset("Default"));
    REQUIRE(ctrl.hasPreset("Default"));
}

TEST_CASE("LayoutPersistenceController auto-save on close / restore on open", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    ctrl.setAutoSaveEnabled(true);
    REQUIRE(ctrl.isAutoSaveEnabled());

    WorkspaceLayout layout("session");
    LayoutPanel p; p.id = "vp"; p.title = "Viewport"; p.type = LayoutPanelType::Viewport;
    layout.addPanel(p);

    REQUIRE(ctrl.onBeforeClose(layout));

    WorkspaceLayout restored("tmp");
    REQUIRE(ctrl.onAfterOpen(restored));
}

TEST_CASE("LayoutPersistenceController auto-save disabled", "[phase_h][h3][layout]") {
    LayoutPersistenceController ctrl;
    ctrl.setAutoSaveEnabled(false);
    WorkspaceLayout layout("session");
    REQUIRE_FALSE(ctrl.onBeforeClose(layout)); // disabled → no-op
}

// ═══════════════════════════════════════════════════════════════════════════
//  H.4 — CommandPaletteController
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("CommandPaletteController default state", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    REQUIRE_FALSE(ctrl.isOpen());
    REQUIRE(ctrl.query().empty());
    REQUIRE(ctrl.resultCount() == 0);
    REQUIRE(ctrl.selectedIndex() == 0);
}

TEST_CASE("CommandPaletteController open/close", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    ctrl.open();
    REQUIRE(ctrl.isOpen());
    ctrl.close();
    REQUIRE_FALSE(ctrl.isOpen());
}

TEST_CASE("CommandPaletteController toggle", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    ctrl.toggle(); REQUIRE(ctrl.isOpen());
    ctrl.toggle(); REQUIRE_FALSE(ctrl.isOpen());
}

TEST_CASE("CommandPaletteController registerCommand and search", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    int callCount = 0;
    CommandEntry e;
    e.id       = "file.save";
    e.label    = "Save File";
    e.category = CommandCategory::Edit;
    e.handler  = [&]{ ++callCount; return true; };
    REQUIRE(ctrl.registerCommand(e));

    CommandEntry e2;
    e2.id       = "file.open";
    e2.label    = "Open File";
    e2.category = CommandCategory::Edit;
    e2.handler  = [&]{ ++callCount; return true; };
    REQUIRE(ctrl.registerCommand(e2));

    ctrl.open();
    REQUIRE(ctrl.resultCount() == 2); // all results with empty query

    ctrl.setQuery("save");
    REQUIRE(ctrl.resultCount() >= 1);
    REQUIRE(ctrl.results()[0]->label == "Save File");
}

TEST_CASE("CommandPaletteController fuzzy search matches subsequence", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    CommandEntry e;
    e.id = "nav.goto"; e.label = "Go To Definition"; e.handler = []{ return true; };
    ctrl.registerCommand(e);
    ctrl.open();
    ctrl.setQuery("gtd"); // subsequence of "Go To Definition"
    REQUIRE(ctrl.resultCount() == 1);
}

TEST_CASE("CommandPaletteController keyboard navigation", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    for (int i = 0; i < 3; ++i) {
        CommandEntry e;
        e.id = "cmd." + std::to_string(i);
        e.label = "Command " + std::to_string(i);
        e.handler = []{ return true; };
        ctrl.registerCommand(e);
    }
    ctrl.open();
    REQUIRE(ctrl.selectedIndex() == 0);
    ctrl.moveSelectionDown();
    REQUIRE(ctrl.selectedIndex() == 1);
    ctrl.moveSelectionDown();
    REQUIRE(ctrl.selectedIndex() == 2);
    ctrl.moveSelectionDown(); // capped at end
    REQUIRE(ctrl.selectedIndex() == 2);
    ctrl.moveSelectionUp();
    REQUIRE(ctrl.selectedIndex() == 1);
    ctrl.selectIndex(0);
    REQUIRE(ctrl.selectedIndex() == 0);
}

TEST_CASE("CommandPaletteController executeSelected", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    ctrl.setCloseOnExecute(false);
    int calls = 0;
    CommandEntry e;
    e.id = "test.cmd"; e.label = "Test Command";
    e.handler = [&]{ ++calls; return true; };
    ctrl.registerCommand(e);
    ctrl.open();
    ctrl.selectIndex(0);
    REQUIRE(ctrl.executeSelected());
    REQUIRE(calls == 1);
    REQUIRE(ctrl.isOpen()); // closeOnExecute=false
}

TEST_CASE("CommandPaletteController closeOnExecute default behaviour", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    CommandEntry e;
    e.id = "test.cmd"; e.label = "Test Command";
    e.handler = []{ return true; };
    ctrl.registerCommand(e);
    ctrl.open();
    ctrl.selectIndex(0);
    ctrl.executeSelected();
    REQUIRE_FALSE(ctrl.isOpen()); // default closeOnExecute=true
}

TEST_CASE("CommandPaletteController recentResults after execute", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    ctrl.setCloseOnExecute(false);
    CommandEntry e;
    e.id = "test.cmd"; e.label = "Test"; e.handler = []{ return true; };
    ctrl.registerCommand(e);
    ctrl.open();
    ctrl.selectIndex(0);
    ctrl.executeSelected();
    auto recents = ctrl.recentResults();
    REQUIRE(recents.size() == 1u);
    REQUIRE(recents[0]->id == "test.cmd");
}

TEST_CASE("CommandPaletteController unregisterCommand", "[phase_h][h4][cmdpalette]") {
    CommandPaletteController ctrl;
    CommandEntry e;
    e.id = "test.cmd"; e.label = "Test"; e.handler = []{ return true; };
    ctrl.registerCommand(e);
    ctrl.open();
    REQUIRE(ctrl.resultCount() == 1);
    REQUIRE(ctrl.unregisterCommand("test.cmd"));
    ctrl.setQuery(""); // force rebuild
    ctrl.open(); // re-open to rebuild
    REQUIRE(ctrl.resultCount() == 0);
}

// ═══════════════════════════════════════════════════════════════════════════
//  H.5 — NotificationCenterController
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("NotifSeverity names", "[phase_h][h5][notif]") {
    REQUIRE(std::string(notifSeverityName(NotifSeverity::Info))     == "Info");
    REQUIRE(std::string(notifSeverityName(NotifSeverity::Success))  == "Success");
    REQUIRE(std::string(notifSeverityName(NotifSeverity::Warning))  == "Warning");
    REQUIRE(std::string(notifSeverityName(NotifSeverity::Error))    == "Error");
    REQUIRE(std::string(notifSeverityName(NotifSeverity::Critical)) == "Critical");
}

TEST_CASE("NotificationCenterController default state", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    REQUIRE_FALSE(ctrl.isOpen());
    REQUIRE(ctrl.historyCount() == 0u);
    REQUIRE(ctrl.toastCount() == 0u);
    REQUIRE(ctrl.undismissedCount() == 0u);
}

TEST_CASE("NotificationCenterController open/close", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    ctrl.open(); REQUIRE(ctrl.isOpen());
    ctrl.close(); REQUIRE_FALSE(ctrl.isOpen());
}

TEST_CASE("NotificationCenterController post adds to history and toast", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    NotificationRecord rec;
    rec.id = "n1"; rec.title = "Build Done"; rec.message = "Build succeeded";
    rec.severity = NotifSeverity::Success; rec.hasToast = true;
    REQUIRE(ctrl.post(rec));
    REQUIRE(ctrl.historyCount() == 1u);
    REQUIRE(ctrl.toastCount() == 1u);
    REQUIRE(ctrl.undismissedCount() == 1u);
}

TEST_CASE("NotificationCenterController post deduplicates by id", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    NotificationRecord rec; rec.id = "n1"; rec.message = "msg"; rec.hasToast = false;
    ctrl.post(rec);
    REQUIRE_FALSE(ctrl.post(rec)); // duplicate
    REQUIRE(ctrl.historyCount() == 1u);
}

TEST_CASE("NotificationCenterController postToast (toast only)", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    ctrl.postToast("t1", "Quick message", NotifSeverity::Info, 5.f);
    REQUIRE(ctrl.toastCount() == 1u);
    REQUIRE(ctrl.historyCount() == 0u); // no history entry
}

TEST_CASE("NotificationCenterController tick expires toasts", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    ctrl.postToast("t1", "msg", NotifSeverity::Info, 2.f);
    REQUIRE(ctrl.toastCount() == 1u);
    ctrl.tick(1.f);
    REQUIRE(ctrl.toastCount() == 1u); // still alive at 1s of 2s
    ctrl.tick(1.5f);
    REQUIRE(ctrl.toastCount() == 0u); // expired at 2.5s total
}

TEST_CASE("NotificationCenterController dismiss by id", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    NotificationRecord rec; rec.id = "n1"; rec.message = "msg"; rec.hasToast = false;
    ctrl.post(rec);
    REQUIRE(ctrl.undismissedCount() == 1u);
    ctrl.dismiss("n1");
    REQUIRE(ctrl.undismissedCount() == 0u);
    REQUIRE(ctrl.historyCount() == 1u); // still in history
}

TEST_CASE("NotificationCenterController dismissAll", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    for (int i = 0; i < 3; ++i) {
        NotificationRecord rec;
        rec.id = "n" + std::to_string(i); rec.message = "m"; rec.hasToast = false;
        ctrl.post(rec);
    }
    REQUIRE(ctrl.undismissedCount() == 3u);
    ctrl.dismissAll();
    REQUIRE(ctrl.undismissedCount() == 0u);
}

TEST_CASE("NotificationCenterController filtered by severity", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    NotificationRecord r1; r1.id="n1"; r1.message="info"; r1.severity=NotifSeverity::Info; r1.hasToast=false;
    NotificationRecord r2; r2.id="n2"; r2.message="err";  r2.severity=NotifSeverity::Error; r2.hasToast=false;
    NotificationRecord r3; r3.id="n3"; r3.message="warn"; r3.severity=NotifSeverity::Warning; r3.hasToast=false;
    ctrl.post(r1); ctrl.post(r2); ctrl.post(r3);

    ctrl.setFilter(NotifSeverity::Error);
    auto filtered = ctrl.filtered();
    REQUIRE(filtered.size() == 1u);
    REQUIRE(filtered[0]->severity == NotifSeverity::Error);

    ctrl.clearFilter();
    REQUIRE(ctrl.filtered().size() == 3u);
}

TEST_CASE("NotificationCenterController filtered returns newest-first", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    for (int i = 0; i < 3; ++i) {
        NotificationRecord r;
        r.id = "n" + std::to_string(i); r.message = "m" + std::to_string(i); r.hasToast = false;
        ctrl.post(r);
    }
    auto f = ctrl.filtered();
    REQUIRE(f.size() == 3u);
    REQUIRE(f[0]->id == "n2"); // newest first
    REQUIRE(f[2]->id == "n0"); // oldest last
}

TEST_CASE("NotificationCenterController navigateTo invokes action", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    bool navigated = false;
    NotificationRecord rec;
    rec.id = "n1"; rec.message = "err"; rec.hasToast = false;
    rec.navigateTo = [&]{ navigated = true; };
    ctrl.post(rec);
    REQUIRE(rec.isActionable());
    REQUIRE(ctrl.navigateTo("n1"));
    REQUIRE(navigated);
}

TEST_CASE("NotificationCenterController navigateTo non-actionable returns false", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    NotificationRecord rec; rec.id = "n1"; rec.message = "info"; rec.hasToast = false;
    ctrl.post(rec);
    REQUIRE_FALSE(ctrl.navigateTo("n1")); // no navigateTo action
}

TEST_CASE("NotificationCenterController errorCount", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    NotificationRecord r1; r1.id="n1"; r1.message="info"; r1.severity=NotifSeverity::Info; r1.hasToast=false;
    NotificationRecord r2; r2.id="n2"; r2.message="err";  r2.severity=NotifSeverity::Error; r2.hasToast=false;
    NotificationRecord r3; r3.id="n3"; r3.message="crit"; r3.severity=NotifSeverity::Critical; r3.hasToast=false;
    ctrl.post(r1); ctrl.post(r2); ctrl.post(r3);
    REQUIRE(ctrl.errorCount() == 2u); // Error + Critical
}

TEST_CASE("NotificationCenterController onPost callback", "[phase_h][h5][notif]") {
    NotificationCenterController ctrl;
    int calls = 0;
    ctrl.setOnPost([&](const NotificationRecord&){ ++calls; });
    NotificationRecord rec; rec.id = "n1"; rec.message = "msg"; rec.hasToast = false;
    ctrl.post(rec);
    REQUIRE(calls == 1);
}

// ═══════════════════════════════════════════════════════════════════════════
//  H.6 — ProjectOpenFlowController
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("ProjectOpenFlowState names", "[phase_h][h6][project_flow]") {
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::Idle))         == "Idle");
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::ChoosingFile)) == "ChoosingFile");
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::Validating))   == "Validating");
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::Opening))      == "Opening");
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::Open))         == "Open");
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::NewWizard))    == "NewWizard");
    REQUIRE(std::string(projectOpenFlowStateName(ProjectOpenFlowState::Error))        == "Error");
}

TEST_CASE("ProjectTemplateKind names", "[phase_h][h6][project_flow]") {
    REQUIRE(std::string(projectTemplateKindName(ProjectTemplateKind::Blank))     == "Blank");
    REQUIRE(std::string(projectTemplateKindName(ProjectTemplateKind::NovaForge)) == "NovaForge");
    REQUIRE(std::string(projectTemplateKindName(ProjectTemplateKind::Minimal))   == "Minimal");
}

TEST_CASE("ProjectOpenFlowController default state", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Idle);
    REQUIRE(ctrl.pendingPath().empty());
    REQUIRE(ctrl.recentProjects().empty());
}

TEST_CASE("ProjectOpenFlowController beginFileOpen transitions to ChoosingFile", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    bool pickerRequested = false;
    ctrl.setOnRequestFilePicker([&]{ pickerRequested = true; });
    ctrl.beginFileOpen();
    REQUIRE(ctrl.state() == ProjectOpenFlowState::ChoosingFile);
    REQUIRE(pickerRequested);
}

TEST_CASE("ProjectOpenFlowController selectFile transitions to Validating", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    REQUIRE(ctrl.selectFile("/projects/MyGame.atlas"));
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Validating);
    REQUIRE(ctrl.pendingPath() == "/projects/MyGame.atlas");
}

TEST_CASE("ProjectOpenFlowController selectFile empty path cancels", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    REQUIRE_FALSE(ctrl.selectFile(""));
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Idle);
}

TEST_CASE("ProjectOpenFlowController validate valid .atlas path", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile("/projects/MyGame.atlas");
    auto result = ctrl.validate();
    REQUIRE(result.success);
    REQUIRE_FALSE(result.hasErrors());
    REQUIRE(result.projectName == "MyGame");
}

TEST_CASE("ProjectOpenFlowController validate invalid extension", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile("/projects/MyGame.json");
    auto result = ctrl.validate();
    REQUIRE_FALSE(result.success);
    REQUIRE(result.hasErrors());
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Error);
}

TEST_CASE("ProjectOpenFlowController confirmOpen after valid validate", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    bool opened = false;
    ctrl.setOnProjectOpened([&](const std::string&){ opened = true; });
    ctrl.beginFileOpen();
    ctrl.selectFile("/projects/MyGame.atlas");
    (void)ctrl.validate();
    REQUIRE(ctrl.confirmOpen());
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Open);
    REQUIRE(opened);
}

TEST_CASE("ProjectOpenFlowController cancelOpen returns to Idle", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile("/projects/MyGame.atlas");
    ctrl.cancelOpen();
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Idle);
    REQUIRE(ctrl.pendingPath().empty());
}

TEST_CASE("ProjectOpenFlowController openRecentProject succeeds for .atlas", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    bool opened = false;
    ctrl.setOnProjectOpened([&](const std::string&){ opened = true; });
    REQUIRE(ctrl.openRecentProject("/projects/MyGame.atlas"));
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Open);
    REQUIRE(opened);
}

TEST_CASE("ProjectOpenFlowController openRecentProject fails for non-.atlas", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    REQUIRE_FALSE(ctrl.openRecentProject("/projects/MyGame.json"));
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Error);
}

TEST_CASE("ProjectOpenFlowController recentProjects grows after open", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.openRecentProject("/projects/MyGame.atlas");
    REQUIRE_FALSE(ctrl.recentProjects().empty());
    REQUIRE(ctrl.recentProjects()[0].path == "/projects/MyGame.atlas");
}

TEST_CASE("ProjectOpenFlowController clearRecents", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.openRecentProject("/projects/MyGame.atlas");
    REQUIRE_FALSE(ctrl.recentProjects().empty());
    ctrl.clearRecents();
    REQUIRE(ctrl.recentProjects().empty());
}

TEST_CASE("ProjectOpenFlowController new project wizard flow", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginNewProjectWizard();
    REQUIRE(ctrl.state() == ProjectOpenFlowState::NewWizard);
    REQUIRE_FALSE(ctrl.isWizardReady()); // name+path not set

    ctrl.setWizardTemplate(ProjectTemplateKind::NovaForge);
    ctrl.setWizardProjectName("MyNewGame");
    ctrl.setWizardProjectPath("/projects");
    REQUIRE(ctrl.wizardTemplate() == ProjectTemplateKind::NovaForge);
    REQUIRE(ctrl.wizardProjectName() == "MyNewGame");
    REQUIRE(ctrl.wizardProjectPath() == "/projects");
    REQUIRE(ctrl.isWizardReady());

    bool opened = false;
    ctrl.setOnProjectOpened([&](const std::string&){ opened = true; });
    REQUIRE(ctrl.createProject());
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Open);
    REQUIRE(opened);
}

TEST_CASE("ProjectOpenFlowController wizard not ready without name", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginNewProjectWizard();
    ctrl.setWizardProjectPath("/projects");
    // no name set
    REQUIRE_FALSE(ctrl.isWizardReady());
    REQUIRE_FALSE(ctrl.createProject());
}

TEST_CASE("ProjectOpenFlowController cancelWizard returns to Idle", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.beginNewProjectWizard();
    ctrl.cancelWizard();
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Idle);
}

TEST_CASE("ProjectOpenFlowController closeProject returns to Idle", "[phase_h][h6][project_flow]") {
    ProjectOpenFlowController ctrl;
    ctrl.openRecentProject("/projects/MyGame.atlas");
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Open);
    REQUIRE(ctrl.closeProject());
    REQUIRE(ctrl.state() == ProjectOpenFlowState::Idle);
}
