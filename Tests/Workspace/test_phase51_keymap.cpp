// Tests/Workspace/test_phase51_keymap.cpp
// Phase 51 — WorkspaceKeymap: KeyModifiers, KeyChord, KeyAction,
//             KeymapLayer, KeymapManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceKeymap.h"

// ═════════════════════════════════════════════════════════════════
// KeyModifiers
// ═════════════════════════════════════════════════════════════════

TEST_CASE("KeyModifiers: default none", "[keymap][modifiers]") {
    NF::KeyModifiers m;
    REQUIRE(m.none());
    REQUIRE_FALSE(m.hasCtrl());
    REQUIRE_FALSE(m.hasShift());
    REQUIRE_FALSE(m.hasAlt());
    REQUIRE_FALSE(m.hasMeta());
    REQUIRE(m.toString().empty());
}

TEST_CASE("KeyModifiers: single flags", "[keymap][modifiers]") {
    NF::KeyModifiers ctrl(NF::KeyModifiers::Ctrl);
    REQUIRE(ctrl.hasCtrl());
    REQUIRE_FALSE(ctrl.hasShift());
    REQUIRE(ctrl.toString() == "Ctrl+");

    NF::KeyModifiers shift(NF::KeyModifiers::Shift);
    REQUIRE(shift.hasShift());
    REQUIRE(shift.toString() == "Shift+");

    NF::KeyModifiers alt(NF::KeyModifiers::Alt);
    REQUIRE(alt.hasAlt());
    REQUIRE(alt.toString() == "Alt+");

    NF::KeyModifiers meta(NF::KeyModifiers::Meta);
    REQUIRE(meta.hasMeta());
    REQUIRE(meta.toString() == "Meta+");
}

TEST_CASE("KeyModifiers: combined flags", "[keymap][modifiers]") {
    NF::KeyModifiers cs(NF::KeyModifiers::Ctrl | NF::KeyModifiers::Shift);
    REQUIRE(cs.hasCtrl());
    REQUIRE(cs.hasShift());
    REQUIRE_FALSE(cs.hasAlt());
    REQUIRE(cs.toString() == "Ctrl+Shift+");
}

TEST_CASE("KeyModifiers: equality", "[keymap][modifiers]") {
    NF::KeyModifiers a(NF::KeyModifiers::Ctrl);
    NF::KeyModifiers b(NF::KeyModifiers::Ctrl);
    NF::KeyModifiers c(NF::KeyModifiers::Shift);
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// KeyChord
// ═════════════════════════════════════════════════════════════════

TEST_CASE("KeyChord: default invalid", "[keymap][chord]") {
    NF::KeyChord ch;
    REQUIRE_FALSE(ch.isValid());
}

TEST_CASE("KeyChord: valid with key only", "[keymap][chord]") {
    NF::KeyChord ch("F5");
    REQUIRE(ch.isValid());
    REQUIRE(ch.toString() == "F5");
}

TEST_CASE("KeyChord: with modifiers", "[keymap][chord]") {
    NF::KeyChord ch("P", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    REQUIRE(ch.isValid());
    REQUIRE(ch.toString() == "Ctrl+P");
}

TEST_CASE("KeyChord: equality", "[keymap][chord]") {
    NF::KeyChord a("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    NF::KeyChord b("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    NF::KeyChord c("S", NF::KeyModifiers(NF::KeyModifiers::Shift));
    NF::KeyChord d("A", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(a != d);
}

// ═════════════════════════════════════════════════════════════════
// KeyAction
// ═════════════════════════════════════════════════════════════════

TEST_CASE("KeyAction: default invalid", "[keymap][action]") {
    NF::KeyAction a;
    REQUIRE_FALSE(a.isValid());
}

TEST_CASE("KeyAction: invalid without chord key", "[keymap][action]") {
    NF::KeyAction a;
    a.id = "save";
    REQUIRE_FALSE(a.isValid());
}

TEST_CASE("KeyAction: valid with id and chord", "[keymap][action]") {
    NF::KeyAction a;
    a.id          = "save";
    a.chord       = NF::KeyChord("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    a.description = "Save file";
    REQUIRE(a.isValid());
}

TEST_CASE("KeyAction: equality by id", "[keymap][action]") {
    NF::KeyAction a, b, c;
    a.id = "save"; a.chord = NF::KeyChord("S");
    b.id = "save"; b.chord = NF::KeyChord("X");
    c.id = "undo"; c.chord = NF::KeyChord("Z");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// KeymapLayer
// ═════════════════════════════════════════════════════════════════

static NF::KeyAction makeAction(const std::string& id, const std::string& key,
                                uint8_t mods = NF::KeyModifiers::None,
                                const std::string& context = "") {
    NF::KeyAction a;
    a.id      = id;
    a.chord   = NF::KeyChord(key, NF::KeyModifiers(mods));
    a.context = context;
    return a;
}

TEST_CASE("KeymapLayer: default empty", "[keymap][layer]") {
    NF::KeymapLayer layer("user", "User", 10);
    REQUIRE(layer.id()       == "user");
    REQUIRE(layer.name()     == "User");
    REQUIRE(layer.priority() == 10);
    REQUIRE(layer.enabled());
    REQUIRE(layer.empty());
    REQUIRE(layer.count() == 0);
}

TEST_CASE("KeymapLayer: addAction valid", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    REQUIRE(layer.addAction(makeAction("save", "S", NF::KeyModifiers::Ctrl)));
    REQUIRE(layer.count() == 1);
    REQUIRE(layer.contains("save"));
}

TEST_CASE("KeymapLayer: addAction invalid rejected", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    NF::KeyAction bad; // no id, no chord
    REQUIRE_FALSE(layer.addAction(bad));
    REQUIRE(layer.empty());
}

TEST_CASE("KeymapLayer: addAction duplicate rejected", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    layer.addAction(makeAction("save", "S"));
    REQUIRE_FALSE(layer.addAction(makeAction("save", "X")));
    REQUIRE(layer.count() == 1);
}

TEST_CASE("KeymapLayer: removeAction", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    layer.addAction(makeAction("save", "S"));
    REQUIRE(layer.removeAction("save"));
    REQUIRE(layer.empty());
}

TEST_CASE("KeymapLayer: removeAction unknown returns false", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    REQUIRE_FALSE(layer.removeAction("no.such"));
}

TEST_CASE("KeymapLayer: findAction", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    layer.addAction(makeAction("save", "S", NF::KeyModifiers::Ctrl));
    REQUIRE(layer.findAction("save") != nullptr);
    REQUIRE(layer.findAction("missing") == nullptr);
}

TEST_CASE("KeymapLayer: findByChord matches chord", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    layer.addAction(makeAction("save", "S", NF::KeyModifiers::Ctrl));
    NF::KeyChord save("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    NF::KeyChord other("Z", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    REQUIRE(layer.findByChord(save) != nullptr);
    REQUIRE(layer.findByChord(other) == nullptr);
}

TEST_CASE("KeymapLayer: findByChord with context", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    layer.addAction(makeAction("del", "Delete", NF::KeyModifiers::None, "scene_editor"));
    NF::KeyChord del("Delete");
    // Matches when context matches
    REQUIRE(layer.findByChord(del, "scene_editor") != nullptr);
    // Matches when query context is empty (wildcard)
    REQUIRE(layer.findByChord(del, "") != nullptr);
    // No match when different context
    REQUIRE(layer.findByChord(del, "asset_browser") == nullptr);
}

TEST_CASE("KeymapLayer: setEnabled", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    REQUIRE(layer.enabled());
    layer.setEnabled(false);
    REQUIRE_FALSE(layer.enabled());
    layer.setEnabled(true);
    REQUIRE(layer.enabled());
}

TEST_CASE("KeymapLayer: clear", "[keymap][layer]") {
    NF::KeymapLayer layer("l", "L", 0);
    layer.addAction(makeAction("save", "S"));
    layer.clear();
    REQUIRE(layer.empty());
}

// ═════════════════════════════════════════════════════════════════
// KeymapManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("KeymapManager: default has default layer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE(mgr.layerCount() == 1);
    REQUIRE(mgr.hasLayer("default"));
    REQUIRE(mgr.findLayer("default") != nullptr);
}

TEST_CASE("KeymapManager: addLayer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE(mgr.addLayer("user", "User", 10) != nullptr);
    REQUIRE(mgr.layerCount() == 2);
    REQUIRE(mgr.hasLayer("user"));
}

TEST_CASE("KeymapManager: addLayer duplicate rejected", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.addLayer("user", "User", 10);
    REQUIRE(mgr.addLayer("user", "User2", 20) == nullptr);
    REQUIRE(mgr.layerCount() == 2);
}

TEST_CASE("KeymapManager: addLayer empty id rejected", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE(mgr.addLayer("", "Empty", 10) == nullptr);
}

TEST_CASE("KeymapManager: removeLayer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.addLayer("user", "User", 10);
    REQUIRE(mgr.removeLayer("user"));
    REQUIRE_FALSE(mgr.hasLayer("user"));
}

TEST_CASE("KeymapManager: cannot remove default layer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE_FALSE(mgr.removeLayer("default"));
    REQUIRE(mgr.hasLayer("default"));
}

TEST_CASE("KeymapManager: removeLayer unknown returns false", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE_FALSE(mgr.removeLayer("no.such"));
}

TEST_CASE("KeymapManager: registerAction adds to default layer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE(mgr.registerAction(makeAction("save", "S", NF::KeyModifiers::Ctrl)));
    REQUIRE(mgr.findLayer("default")->contains("save"));
}

TEST_CASE("KeymapManager: unregisterAction removes from default layer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.registerAction(makeAction("save", "S"));
    REQUIRE(mgr.unregisterAction("save"));
    REQUIRE_FALSE(mgr.findLayer("default")->contains("save"));
}

TEST_CASE("KeymapManager: findAction searches all layers", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.registerAction(makeAction("global.save", "S"));
    auto* ul = mgr.addLayer("user", "User", 10);
    ul->addAction(makeAction("user.open", "O"));
    REQUIRE(mgr.findAction("global.save") != nullptr);
    REQUIRE(mgr.findAction("user.open")   != nullptr);
    REQUIRE(mgr.findAction("missing")     == nullptr);
}

TEST_CASE("KeymapManager: lookup returns highest priority layer match", "[keymap][manager]") {
    NF::KeymapManager mgr;
    // Default layer has Ctrl+S = save
    mgr.registerAction(makeAction("default.save", "S", NF::KeyModifiers::Ctrl));
    // User layer (higher priority) also has Ctrl+S = user.quicksave
    auto* ul = mgr.addLayer("user", "User", 100);
    ul->addAction(makeAction("user.quicksave", "S", NF::KeyModifiers::Ctrl));

    NF::KeyChord chord("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    const auto* result = mgr.lookup(chord);
    REQUIRE(result != nullptr);
    REQUIRE(result->id == "user.quicksave"); // user layer wins
}

TEST_CASE("KeymapManager: lookup returns nullptr when no match", "[keymap][manager]") {
    NF::KeymapManager mgr;
    NF::KeyChord chord("X", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    REQUIRE(mgr.lookup(chord) == nullptr);
}

TEST_CASE("KeymapManager: disabled layer skipped in lookup", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.registerAction(makeAction("default.save", "S", NF::KeyModifiers::Ctrl));
    auto* ul = mgr.addLayer("user", "User", 100);
    ul->addAction(makeAction("user.quicksave", "S", NF::KeyModifiers::Ctrl));

    mgr.setLayerEnabled("user", false);

    NF::KeyChord chord("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    const auto* result = mgr.lookup(chord);
    REQUIRE(result != nullptr);
    REQUIRE(result->id == "default.save"); // user disabled, falls through to default
}

TEST_CASE("KeymapManager: lookupAll returns all matches across layers", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.registerAction(makeAction("a1", "S", NF::KeyModifiers::Ctrl));
    auto* ul = mgr.addLayer("user", "User", 100);
    ul->addAction(makeAction("a2", "S", NF::KeyModifiers::Ctrl));

    NF::KeyChord chord("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    auto all = mgr.lookupAll(chord);
    REQUIRE(all.size() == 2);
}

TEST_CASE("KeymapManager: observer fires on registerAction", "[keymap][manager]") {
    NF::KeymapManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.registerAction(makeAction("save", "S"));
    REQUIRE(count == 1);
}

TEST_CASE("KeymapManager: observer fires on addLayer", "[keymap][manager]") {
    NF::KeymapManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.addLayer("user", "User", 10);
    REQUIRE(count == 1);
}

TEST_CASE("KeymapManager: observer fires on setLayerEnabled", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.addLayer("user", "User", 10);
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.setLayerEnabled("user", false);
    REQUIRE(count == 1);
}

TEST_CASE("KeymapManager: clearObservers stops notifications", "[keymap][manager]") {
    NF::KeymapManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.registerAction(makeAction("save", "S"));
    REQUIRE(count == 1);
    mgr.clearObservers();
    mgr.registerAction(makeAction("undo", "Z"));
    REQUIRE(count == 1);
}

TEST_CASE("KeymapManager: serialize round-trip", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.registerAction(makeAction("save", "S", NF::KeyModifiers::Ctrl));
    auto* ul = mgr.addLayer("user", "User", 100);
    ul->addAction(makeAction("open", "O", NF::KeyModifiers::Ctrl, "editor"));

    std::string text = mgr.serialize();
    REQUIRE_FALSE(text.empty());

    NF::KeymapManager mgr2;
    int loaded = mgr2.deserialize(text);
    REQUIRE(loaded == 2);
    REQUIRE(mgr2.findAction("save")  != nullptr);
    REQUIRE(mgr2.findAction("open")  != nullptr);
    REQUIRE(mgr2.findAction("open")->context == "editor");
    REQUIRE(mgr2.findAction("save")->chord.modifiers.hasCtrl());
}

TEST_CASE("KeymapManager: deserialize empty returns 0", "[keymap][manager]") {
    NF::KeymapManager mgr;
    REQUIRE(mgr.deserialize("") == 0);
}

TEST_CASE("KeymapManager: clear resets to default layer only", "[keymap][manager]") {
    NF::KeymapManager mgr;
    mgr.addLayer("user", "User", 10);
    mgr.registerAction(makeAction("save", "S"));
    mgr.clear();
    REQUIRE(mgr.layerCount() == 1);
    REQUIRE(mgr.hasLayer("default"));
    REQUIRE(mgr.findLayer("default")->empty());
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Keymap integration: multi-layer priority resolution", "[keymap][integration]") {
    NF::KeymapManager mgr;

    // Default layer: common shortcuts
    mgr.registerAction(makeAction("file.save",  "S", NF::KeyModifiers::Ctrl));
    mgr.registerAction(makeAction("file.open",  "O", NF::KeyModifiers::Ctrl));
    mgr.registerAction(makeAction("edit.undo",  "Z", NF::KeyModifiers::Ctrl));

    // Tool-specific layer: overrides Ctrl+S in scene editor context
    auto* toolLayer = mgr.addLayer("scene_editor", "Scene Editor", 50);
    toolLayer->addAction(makeAction("scene.quicksave", "S", NF::KeyModifiers::Ctrl, "scene_editor"));

    // Ctrl+S in scene_editor context → tool layer wins
    NF::KeyChord ctrlS("S", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    const auto* r1 = mgr.lookup(ctrlS, "scene_editor");
    REQUIRE(r1 != nullptr);
    REQUIRE(r1->id == "scene.quicksave");

    // Ctrl+S in other context → default layer (tool action filtered by context)
    const auto* r2 = mgr.lookup(ctrlS, "asset_browser");
    REQUIRE(r2 != nullptr);
    REQUIRE(r2->id == "file.save");

    // Ctrl+O always comes from default (no override)
    NF::KeyChord ctrlO("O", NF::KeyModifiers(NF::KeyModifiers::Ctrl));
    const auto* r3 = mgr.lookup(ctrlO, "scene_editor");
    REQUIRE(r3 != nullptr);
    REQUIRE(r3->id == "file.open");
}

TEST_CASE("Keymap integration: full serialize/deserialize preserves layers", "[keymap][integration]") {
    NF::KeymapManager src;
    src.registerAction(makeAction("undo", "Z", NF::KeyModifiers::Ctrl));
    auto* ul = src.addLayer("user", "User", 20);
    ul->addAction(makeAction("custom.go", "G", NF::KeyModifiers::Alt, "navigator"));

    std::string text = src.serialize();

    NF::KeymapManager dst;
    dst.deserialize(text);

    const auto* undo = dst.findAction("undo");
    REQUIRE(undo != nullptr);
    REQUIRE(undo->chord.key == "Z");
    REQUIRE(undo->chord.modifiers.hasCtrl());

    const auto* go = dst.findAction("custom.go");
    REQUIRE(go != nullptr);
    REQUIRE(go->context == "navigator");
    REQUIRE(go->chord.modifiers.hasAlt());
}
