// Tests/Workspace/test_phase23_hotkeys.cpp
// Phase 23 — Workspace Hotkey Manager
//
// Tests for:
//   1. ModifierFlags — bitmask helpers
//   2. HotkeyChord   — modifier+key chord; toString, isValid, equality
//   3. HotkeyBinding — chord → command mapping; isValid
//   4. HotkeyConflict — overlapping binding pair
//   5. HotkeyManager — registration, lookup, conflict detection, observers
//   6. Integration   — full shortcut dispatch pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceHotkeys.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — ModifierFlags
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ModifierFlags None string", "[Phase23][ModifierFlags]") {
    CHECK(modifierFlagsString(ModifierFlags::None) == "None");
}

TEST_CASE("ModifierFlags Ctrl string", "[Phase23][ModifierFlags]") {
    CHECK(modifierFlagsString(ModifierFlags::Ctrl) == "Ctrl");
}

TEST_CASE("ModifierFlags Ctrl+Shift string", "[Phase23][ModifierFlags]") {
    auto f = ModifierFlags::Ctrl | ModifierFlags::Shift;
    std::string s = modifierFlagsString(f);
    CHECK(s.find("Ctrl") != std::string::npos);
    CHECK(s.find("Shift") != std::string::npos);
}

TEST_CASE("ModifierFlags hasModifier", "[Phase23][ModifierFlags]") {
    auto f = ModifierFlags::Ctrl | ModifierFlags::Alt;
    CHECK(hasModifier(f, ModifierFlags::Ctrl));
    CHECK(hasModifier(f, ModifierFlags::Alt));
    CHECK_FALSE(hasModifier(f, ModifierFlags::Shift));
    CHECK_FALSE(hasModifier(f, ModifierFlags::Meta));
}

TEST_CASE("ModifierFlags all four bits", "[Phase23][ModifierFlags]") {
    auto f = ModifierFlags::Ctrl | ModifierFlags::Alt | ModifierFlags::Shift | ModifierFlags::Meta;
    CHECK(hasModifier(f, ModifierFlags::Ctrl));
    CHECK(hasModifier(f, ModifierFlags::Alt));
    CHECK(hasModifier(f, ModifierFlags::Shift));
    CHECK(hasModifier(f, ModifierFlags::Meta));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — HotkeyChord
// ═════════════════════════════════════════════════════════════════

TEST_CASE("HotkeyChord default is invalid", "[Phase23][HotkeyChord]") {
    HotkeyChord c;
    CHECK_FALSE(c.isValid());
    CHECK(c.key.empty());
}

TEST_CASE("HotkeyChord valid with key only", "[Phase23][HotkeyChord]") {
    HotkeyChord c{ModifierFlags::None, "F5"};
    CHECK(c.isValid());
    CHECK(c.toString() == "F5");
}

TEST_CASE("HotkeyChord toString with modifiers", "[Phase23][HotkeyChord]") {
    HotkeyChord c{ModifierFlags::Ctrl, "S"};
    CHECK(c.toString() == "Ctrl+S");
}

TEST_CASE("HotkeyChord toString Ctrl+Shift+Z", "[Phase23][HotkeyChord]") {
    HotkeyChord c{ModifierFlags::Ctrl | ModifierFlags::Shift, "Z"};
    std::string s = c.toString();
    CHECK(s.find("Ctrl") != std::string::npos);
    CHECK(s.find("Shift") != std::string::npos);
    CHECK(s.find('Z') != std::string::npos);
}

TEST_CASE("HotkeyChord equality", "[Phase23][HotkeyChord]") {
    HotkeyChord a{ModifierFlags::Ctrl, "Z"};
    HotkeyChord b{ModifierFlags::Ctrl, "Z"};
    HotkeyChord c{ModifierFlags::Alt,  "Z"};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — HotkeyBinding
// ═════════════════════════════════════════════════════════════════

TEST_CASE("HotkeyBinding default is invalid", "[Phase23][HotkeyBinding]") {
    HotkeyBinding b;
    CHECK_FALSE(b.isValid());
}

TEST_CASE("HotkeyBinding valid construction", "[Phase23][HotkeyBinding]") {
    HotkeyBinding b{"bind_save", {ModifierFlags::Ctrl, "S"}, "cmd.save", "global", true};
    CHECK(b.isValid());
    CHECK(b.id == "bind_save");
    CHECK(b.commandId == "cmd.save");
    CHECK(b.scopeId == "global");
    CHECK(b.enabled);
}

TEST_CASE("HotkeyBinding invalid without id", "[Phase23][HotkeyBinding]") {
    HotkeyBinding b{"", {ModifierFlags::Ctrl, "S"}, "cmd.save", "", true};
    CHECK_FALSE(b.isValid());
}

TEST_CASE("HotkeyBinding invalid without commandId", "[Phase23][HotkeyBinding]") {
    HotkeyBinding b{"b1", {ModifierFlags::Ctrl, "S"}, "", "", true};
    CHECK_FALSE(b.isValid());
}

TEST_CASE("HotkeyBinding equality by id", "[Phase23][HotkeyBinding]") {
    HotkeyBinding a{"bind_1", {ModifierFlags::Ctrl, "Z"}, "cmd.undo", "", true};
    HotkeyBinding b{"bind_1", {ModifierFlags::Ctrl, "Y"}, "cmd.redo", "", true};
    HotkeyBinding c{"bind_2", {ModifierFlags::Ctrl, "Z"}, "cmd.undo", "", true};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — HotkeyConflict
// ═════════════════════════════════════════════════════════════════

TEST_CASE("HotkeyConflict default is invalid", "[Phase23][HotkeyConflict]") {
    HotkeyConflict c;
    CHECK_FALSE(c.isValid());
}

TEST_CASE("HotkeyConflict valid construction", "[Phase23][HotkeyConflict]") {
    HotkeyChord chord{ModifierFlags::Ctrl, "Z"};
    HotkeyConflict c{"bind_a", "bind_b", chord, ""};
    CHECK(c.isValid());
    CHECK(c.bindingIdA == "bind_a");
    CHECK(c.bindingIdB == "bind_b");
    CHECK(c.chord == chord);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — HotkeyManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("HotkeyManager empty state", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    CHECK(m.bindingCount() == 0);
    CHECK(m.allBindingIds().empty());
}

TEST_CASE("HotkeyManager registerBinding", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyBinding b{"b1", {ModifierFlags::Ctrl, "S"}, "cmd.save", "", true};
    CHECK(m.registerBinding(b));
    CHECK(m.isRegistered("b1"));
    CHECK(m.bindingCount() == 1);
}

TEST_CASE("HotkeyManager duplicate id rejected", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyBinding b{"b1", {ModifierFlags::Ctrl, "S"}, "cmd.save", "", true};
    CHECK(m.registerBinding(b));
    CHECK_FALSE(m.registerBinding(b));
    CHECK(m.bindingCount() == 1);
}

TEST_CASE("HotkeyManager invalid binding rejected", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyBinding b;  // invalid
    CHECK_FALSE(m.registerBinding(b));
}

TEST_CASE("HotkeyManager unregisterBinding", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyBinding b{"b1", {ModifierFlags::Ctrl, "S"}, "cmd.save", "", true};
    m.registerBinding(b);
    CHECK(m.unregisterBinding("b1"));
    CHECK_FALSE(m.isRegistered("b1"));
}

TEST_CASE("HotkeyManager unregister unknown fails", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    CHECK_FALSE(m.unregisterBinding("nope"));
}

TEST_CASE("HotkeyManager findById", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyBinding b{"b1", {ModifierFlags::Ctrl, "Z"}, "cmd.undo", "", true};
    m.registerBinding(b);
    const HotkeyBinding* found = m.findById("b1");
    CHECK(found != nullptr);
    CHECK(found->commandId == "cmd.undo");
}

TEST_CASE("HotkeyManager findByChord — global scope", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "Z"};
    HotkeyBinding b{"b1", chord, "cmd.undo", "", true};
    m.registerBinding(b);
    auto* found = m.findByChord(chord);
    CHECK(found != nullptr);
    CHECK(found->commandId == "cmd.undo");
}

TEST_CASE("HotkeyManager findByChord — scope-exact match preferred", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "D"};
    HotkeyBinding global{"bg", chord, "cmd.global", "", true};
    HotkeyBinding scoped{"bs", chord, "cmd.scoped", "tool_scene", true};
    m.registerBinding(global);
    m.registerBinding(scoped);
    auto* found = m.findByChord(chord, "tool_scene");
    CHECK(found != nullptr);
    CHECK(found->commandId == "cmd.scoped");
}

TEST_CASE("HotkeyManager findByChord — falls back to global", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "P"};
    HotkeyBinding global{"bg", chord, "cmd.print", "", true};
    m.registerBinding(global);
    auto* found = m.findByChord(chord, "some_scope");
    CHECK(found != nullptr);
    CHECK(found->commandId == "cmd.print");
}

TEST_CASE("HotkeyManager findByCommand returns all matches", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    m.registerBinding({"b1", {ModifierFlags::Ctrl, "Z"}, "cmd.undo", "", true});
    m.registerBinding({"b2", {ModifierFlags::Alt,  "Z"}, "cmd.undo", "", true});
    m.registerBinding({"b3", {ModifierFlags::Ctrl, "S"}, "cmd.save", "", true});
    auto results = m.findByCommand("cmd.undo");
    CHECK(results.size() == 2);
}

TEST_CASE("HotkeyManager detectConflicts finds same chord in same scope", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "Z"};
    m.registerBinding({"b1", chord, "cmd.undo", "", true});
    m.registerBinding({"b2", chord, "cmd.something", "", true});
    auto conflicts = m.detectConflicts();
    CHECK(conflicts.size() == 1);
    CHECK(conflicts[0].chord == chord);
}

TEST_CASE("HotkeyManager detectConflicts no conflict different scopes", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "Z"};
    m.registerBinding({"b1", chord, "cmd.undo", "scope_a", true});
    m.registerBinding({"b2", chord, "cmd.other", "scope_b", true});
    auto conflicts = m.detectConflicts();
    CHECK(conflicts.empty());
}

TEST_CASE("HotkeyManager enableBinding / disableBinding", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "H"};
    m.registerBinding({"b1", chord, "cmd.help", "", true});
    CHECK(m.disableBinding("b1"));
    CHECK(m.findByChord(chord) == nullptr);  // disabled not found
    CHECK(m.enableBinding("b1"));
    CHECK(m.findByChord(chord) != nullptr);
}

TEST_CASE("HotkeyManager activate notifies observer", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "S"};
    m.registerBinding({"b1", chord, "cmd.save", "", true});
    std::string fired;
    m.addObserver([&](const HotkeyBinding& b) { fired = b.commandId; });
    CHECK(m.activate(chord));
    CHECK(fired == "cmd.save");
}

TEST_CASE("HotkeyManager activate returns false for unknown chord", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Alt, "X"};
    CHECK_FALSE(m.activate(chord));
}

TEST_CASE("HotkeyManager removeObserver stops notifications", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "S"};
    m.registerBinding({"b1", chord, "cmd.save", "", true});
    int calls = 0;
    uint32_t id = m.addObserver([&](const HotkeyBinding&) { ++calls; });
    m.removeObserver(id);
    m.activate(chord);
    CHECK(calls == 0);
}

TEST_CASE("HotkeyManager allBindingIds", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    m.registerBinding({"b1", {ModifierFlags::Ctrl, "A"}, "cmd.a", "", true});
    m.registerBinding({"b2", {ModifierFlags::Ctrl, "B"}, "cmd.b", "", true});
    auto ids = m.allBindingIds();
    CHECK(ids.size() == 2);
}

TEST_CASE("HotkeyManager clear removes all bindings", "[Phase23][HotkeyManager]") {
    HotkeyManager m;
    m.registerBinding({"b1", {ModifierFlags::Ctrl, "A"}, "cmd.a", "", true});
    m.clear();
    CHECK(m.bindingCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full hotkey dispatch: register → activate → observer fires command", "[Phase23][Integration]") {
    HotkeyManager m;
    m.registerBinding({"undo",  {ModifierFlags::Ctrl, "Z"}, "cmd.undo", "", true});
    m.registerBinding({"redo",  {ModifierFlags::Ctrl | ModifierFlags::Shift, "Z"}, "cmd.redo", "", true});
    m.registerBinding({"save",  {ModifierFlags::Ctrl, "S"}, "cmd.save", "", true});

    std::vector<std::string> fired;
    m.addObserver([&](const HotkeyBinding& b) { fired.push_back(b.commandId); });

    m.activate({ModifierFlags::Ctrl, "Z"});
    m.activate({ModifierFlags::Ctrl, "S"});
    m.activate({ModifierFlags::Ctrl | ModifierFlags::Shift, "Z"});

    CHECK(fired.size() == 3);
    CHECK(fired[0] == "cmd.undo");
    CHECK(fired[1] == "cmd.save");
    CHECK(fired[2] == "cmd.redo");
}

TEST_CASE("Scope isolation: scoped binding shadows global in its scope", "[Phase23][Integration]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "D"};
    m.registerBinding({"global_d", chord, "cmd.deselect",    "",           true});
    m.registerBinding({"scene_d",  chord, "cmd.duplicate",   "tool_scene", true});

    // In tool_scene scope → scoped wins
    auto* inScene = m.findByChord(chord, "tool_scene");
    CHECK(inScene != nullptr);
    CHECK(inScene->commandId == "cmd.duplicate");

    // In another scope → global wins
    auto* inOther = m.findByChord(chord, "tool_material");
    CHECK(inOther != nullptr);
    CHECK(inOther->commandId == "cmd.deselect");
}

TEST_CASE("Conflict detection across multiple pairs", "[Phase23][Integration]") {
    HotkeyManager m;
    HotkeyChord c1{ModifierFlags::Ctrl, "Z"};
    HotkeyChord c2{ModifierFlags::Ctrl, "Y"};
    m.registerBinding({"a1", c1, "cmd.a", "", true});
    m.registerBinding({"a2", c1, "cmd.b", "", true});
    m.registerBinding({"a3", c2, "cmd.c", "", true});
    m.registerBinding({"a4", c2, "cmd.d", "", true});
    auto conflicts = m.detectConflicts();
    CHECK(conflicts.size() == 2);
}

TEST_CASE("Disabled binding not activated", "[Phase23][Integration]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "S"};
    m.registerBinding({"b1", chord, "cmd.save", "", true});
    m.disableBinding("b1");
    int calls = 0;
    m.addObserver([&](const HotkeyBinding&) { ++calls; });
    CHECK_FALSE(m.activate(chord));
    CHECK(calls == 0);
}

TEST_CASE("clearObservers stops all notifications", "[Phase23][Integration]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "Z"};
    m.registerBinding({"b1", chord, "cmd.undo", "", true});
    int calls = 0;
    m.addObserver([&](const HotkeyBinding&) { ++calls; });
    m.clearObservers();
    m.activate(chord);
    CHECK(calls == 0);
}

TEST_CASE("Multiple observers all fire on activate", "[Phase23][Integration]") {
    HotkeyManager m;
    HotkeyChord chord{ModifierFlags::Ctrl, "O"};
    m.registerBinding({"b1", chord, "cmd.open", "", true});
    int c1 = 0, c2 = 0;
    m.addObserver([&](const HotkeyBinding&) { ++c1; });
    m.addObserver([&](const HotkeyBinding&) { ++c2; });
    m.activate(chord);
    CHECK(c1 == 1);
    CHECK(c2 == 1);
}
