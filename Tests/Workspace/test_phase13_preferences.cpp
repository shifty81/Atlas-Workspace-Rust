// Tests/Workspace/test_phase13_preferences.cpp
// Phase 13 — Workspace Preferences and Configuration
//
// Tests for:
//   1. PreferenceCategory / PreferenceType — enum names
//   2. PreferenceEntry    — construction, factories, validity, validation
//   3. PreferenceRegistry — register, find, category, validate, defaults
//   4. PreferenceController — set, get, typed access, reset, EventBus integration
//   5. PreferenceSerializer — serialize, deserialize, round-trip
//   6. Integration         — full preference pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspacePreferences.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — PreferenceCategory / PreferenceType
// ═════════════════════════════════════════════════════════════════

TEST_CASE("preferenceCategoryName returns correct strings", "[Phase13][Category]") {
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::General))     == "General");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::Appearance))  == "Appearance");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::Keybindings)) == "Keybindings");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::Editor))      == "Editor");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::Build))       == "Build");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::AI))          == "AI");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::Plugin))      == "Plugin");
    CHECK(std::string(preferenceCategoryName(PreferenceCategory::Custom))      == "Custom");
}

TEST_CASE("preferenceTypeName returns correct strings", "[Phase13][Type]") {
    CHECK(std::string(preferenceTypeName(PreferenceType::String)) == "String");
    CHECK(std::string(preferenceTypeName(PreferenceType::Bool))   == "Bool");
    CHECK(std::string(preferenceTypeName(PreferenceType::Int))    == "Int");
    CHECK(std::string(preferenceTypeName(PreferenceType::Float))  == "Float");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — PreferenceEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PreferenceEntry default is not valid", "[Phase13][Entry]") {
    PreferenceEntry e;
    CHECK_FALSE(e.isValid());
}

TEST_CASE("PreferenceEntry::makeString creates valid entry", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeString("theme", "Theme", "dark", PreferenceCategory::Appearance, "UI theme");
    CHECK(e.isValid());
    CHECK(e.key          == "theme");
    CHECK(e.displayName  == "Theme");
    CHECK(e.defaultValue == "dark");
    CHECK(e.category     == PreferenceCategory::Appearance);
    CHECK(e.type         == PreferenceType::String);
    CHECK(e.description  == "UI theme");
    CHECK_FALSE(e.hasRange);
}

TEST_CASE("PreferenceEntry::makeBool creates valid entry", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeBool("auto_save", "Auto Save", true, PreferenceCategory::General);
    CHECK(e.isValid());
    CHECK(e.type         == PreferenceType::Bool);
    CHECK(e.defaultValue == "true");
}

TEST_CASE("PreferenceEntry::makeInt creates valid entry with range", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeInt("font_size", "Font Size", 14, 8, 32, PreferenceCategory::Appearance);
    CHECK(e.isValid());
    CHECK(e.type     == PreferenceType::Int);
    CHECK(e.hasRange);
    CHECK(e.minValue == 8.f);
    CHECK(e.maxValue == 32.f);
}

TEST_CASE("PreferenceEntry::makeFloat creates valid entry with range", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeFloat("scale", "UI Scale", 1.0f, 0.5f, 3.0f);
    CHECK(e.isValid());
    CHECK(e.type     == PreferenceType::Float);
    CHECK(e.hasRange);
    CHECK(e.minValue == 0.5f);
    CHECK(e.maxValue == 3.0f);
}

TEST_CASE("PreferenceEntry::validate Bool type", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeBool("flag", "Flag", false);
    CHECK(e.validate("true"));
    CHECK(e.validate("false"));
    CHECK(e.validate("1"));
    CHECK(e.validate("0"));
    CHECK_FALSE(e.validate("yes"));
    CHECK_FALSE(e.validate("abc"));
}

TEST_CASE("PreferenceEntry::validate Int with range", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeInt("size", "Size", 10, 1, 100);
    CHECK(e.validate("1"));
    CHECK(e.validate("50"));
    CHECK(e.validate("100"));
    CHECK_FALSE(e.validate("0"));
    CHECK_FALSE(e.validate("101"));
    CHECK_FALSE(e.validate("abc"));
}

TEST_CASE("PreferenceEntry::validate Float with range", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeFloat("scale", "Scale", 1.0f, 0.5f, 3.0f);
    CHECK(e.validate("0.5"));
    CHECK(e.validate("1.5"));
    CHECK(e.validate("3.0"));
    CHECK_FALSE(e.validate("0.1"));
    CHECK_FALSE(e.validate("3.1"));
    CHECK_FALSE(e.validate("abc"));
}

TEST_CASE("PreferenceEntry::validate String always accepts", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeString("name", "Name", "default");
    CHECK(e.validate("anything"));
    CHECK(e.validate(""));
}

TEST_CASE("PreferenceEntry::validate empty value is always valid", "[Phase13][Entry]") {
    auto e = PreferenceEntry::makeInt("size", "Size", 10, 1, 100);
    CHECK(e.validate(""));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — PreferenceRegistry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PreferenceRegistry empty on construction", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    CHECK(reg.empty());
    CHECK(reg.count() == 0);
}

TEST_CASE("PreferenceRegistry register and find", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    auto entry = PreferenceEntry::makeString("theme", "Theme", "dark");
    CHECK(reg.registerEntry(entry));
    CHECK(reg.count() == 1);
    CHECK(reg.isRegistered("theme"));

    auto* found = reg.find("theme");
    REQUIRE(found != nullptr);
    CHECK(found->displayName == "Theme");
}

TEST_CASE("PreferenceRegistry rejects duplicate key", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("key", "K", "v"));
    CHECK_FALSE(reg.registerEntry(PreferenceEntry::makeString("key", "K2", "v2")));
    CHECK(reg.count() == 1);
}

TEST_CASE("PreferenceRegistry rejects invalid entry", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    PreferenceEntry invalid; // empty key
    CHECK_FALSE(reg.registerEntry(invalid));
}

TEST_CASE("PreferenceRegistry unregister", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("key", "K", "v"));
    CHECK(reg.unregisterEntry("key"));
    CHECK(reg.count() == 0);
    CHECK_FALSE(reg.unregisterEntry("nonexistent"));
}

TEST_CASE("PreferenceRegistry findByCategory", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("a", "A", "v", PreferenceCategory::General));
    reg.registerEntry(PreferenceEntry::makeString("b", "B", "v", PreferenceCategory::Appearance));
    reg.registerEntry(PreferenceEntry::makeString("c", "C", "v", PreferenceCategory::General));

    auto general = reg.findByCategory(PreferenceCategory::General);
    CHECK(general.size() == 2);

    auto appearance = reg.findByCategory(PreferenceCategory::Appearance);
    CHECK(appearance.size() == 1);
}

TEST_CASE("PreferenceRegistry countByCategory", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("a", "A", "v", PreferenceCategory::Build));
    reg.registerEntry(PreferenceEntry::makeString("b", "B", "v", PreferenceCategory::Build));
    reg.registerEntry(PreferenceEntry::makeString("c", "C", "v", PreferenceCategory::AI));

    CHECK(reg.countByCategory(PreferenceCategory::Build) == 2);
    CHECK(reg.countByCategory(PreferenceCategory::AI)    == 1);
    CHECK(reg.countByCategory(PreferenceCategory::Plugin) == 0);
}

TEST_CASE("PreferenceRegistry validate delegates to entry", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeInt("size", "Size", 10, 1, 100));

    CHECK(reg.validate("size", "50"));
    CHECK_FALSE(reg.validate("size", "200"));
    CHECK_FALSE(reg.validate("nonexistent", "50"));
}

TEST_CASE("PreferenceRegistry populateDefaults fills SettingsStore", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeBool("auto_save", "Auto Save", true));
    reg.registerEntry(PreferenceEntry::makeInt("font_size", "Font Size", 14, 8, 32));

    SettingsStore store;
    reg.populateDefaults(store);

    CHECK(store.get("auto_save") == "true");
    CHECK(store.get("font_size") == "14");
}

TEST_CASE("PreferenceRegistry loadWorkspaceDefaults adds standard entries", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.loadWorkspaceDefaults();

    CHECK(reg.count() > 0);
    CHECK(reg.isRegistered("general.auto_save"));
    CHECK(reg.isRegistered("appearance.theme"));
    CHECK(reg.isRegistered("editor.tab_size"));
    CHECK(reg.isRegistered("build.parallel"));
    CHECK(reg.isRegistered("ai.enabled"));
}

TEST_CASE("PreferenceRegistry clear removes all", "[Phase13][Registry]") {
    PreferenceRegistry reg;
    reg.loadWorkspaceDefaults();
    CHECK(reg.count() > 0);
    reg.clear();
    CHECK(reg.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — PreferenceController
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PreferenceController set and get", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    SettingsStore store;
    PreferenceController ctrl(reg, store);
    ctrl.initialize();

    CHECK(ctrl.get("theme") == "dark"); // default
    CHECK(ctrl.set("theme", "light"));
    CHECK(ctrl.get("theme") == "light");
}

TEST_CASE("PreferenceController rejects unregistered key", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    SettingsStore store;
    PreferenceController ctrl(reg, store);

    CHECK_FALSE(ctrl.set("nonexistent", "value"));
}

TEST_CASE("PreferenceController rejects readOnly", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    auto entry = PreferenceEntry::makeString("version", "Version", "1.0");
    entry.readOnly = true;
    reg.registerEntry(entry);
    SettingsStore store;
    PreferenceController ctrl(reg, store);

    CHECK_FALSE(ctrl.set("version", "2.0"));
}

TEST_CASE("PreferenceController validates before setting", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeInt("size", "Size", 10, 1, 100));
    SettingsStore store;
    PreferenceController ctrl(reg, store);
    ctrl.initialize();

    CHECK(ctrl.set("size", "50"));
    CHECK_FALSE(ctrl.set("size", "200"));
    CHECK(ctrl.getInt("size") == 50);
}

TEST_CASE("PreferenceController typed getters", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeBool("flag", "Flag", true));
    reg.registerEntry(PreferenceEntry::makeInt("count", "Count", 42, 0, 100));
    reg.registerEntry(PreferenceEntry::makeFloat("scale", "Scale", 1.5f, 0.f, 5.f));
    SettingsStore store;
    PreferenceController ctrl(reg, store);
    ctrl.initialize();

    CHECK(ctrl.getBool("flag") == true);
    CHECK(ctrl.getInt("count") == 42);
    CHECK(ctrl.getFloat("scale") > 1.4f);
    CHECK(ctrl.getFloat("scale") < 1.6f);
}

TEST_CASE("PreferenceController resetToDefault", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    SettingsStore store;
    PreferenceController ctrl(reg, store);
    ctrl.initialize();

    ctrl.set("theme", "light");
    CHECK(ctrl.get("theme") == "light");

    CHECK(ctrl.resetToDefault("theme"));
    CHECK(ctrl.get("theme") == "dark"); // back to default
}

TEST_CASE("PreferenceController resetAll clears user and project layers", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("a", "A", "default_a"));
    reg.registerEntry(PreferenceEntry::makeString("b", "B", "default_b"));
    SettingsStore store;
    PreferenceController ctrl(reg, store);
    ctrl.initialize();

    ctrl.set("a", "custom_a");
    ctrl.set("b", "custom_b");
    ctrl.resetAll();

    CHECK(ctrl.get("a") == "default_a");
    CHECK(ctrl.get("b") == "default_b");
}

TEST_CASE("PreferenceController fires EventBus events on set", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    SettingsStore store;
    WorkspaceEventBus bus;

    int eventCount = 0;
    bus.subscribe(WorkspaceEventType::System, [&](const WorkspaceEvent& ev) {
        CHECK(ev.source == "preferences");
        ++eventCount;
    });

    PreferenceController ctrl(reg, store, &bus);
    ctrl.initialize();
    ctrl.set("theme", "light");

    CHECK(eventCount == 1);
}

TEST_CASE("PreferenceController fires EventBus events on reset", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark"));
    SettingsStore store;
    WorkspaceEventBus bus;

    int eventCount = 0;
    bus.subscribe(WorkspaceEventType::System, [&](const WorkspaceEvent&) { ++eventCount; });

    PreferenceController ctrl(reg, store, &bus);
    ctrl.initialize();
    ctrl.set("theme", "light");
    ctrl.resetToDefault("theme");
    ctrl.resetAll();

    CHECK(eventCount == 3); // set + resetToDefault + resetAll
}

TEST_CASE("PreferenceController getOr returns fallback", "[Phase13][Controller]") {
    PreferenceRegistry reg;
    SettingsStore store;
    PreferenceController ctrl(reg, store);

    CHECK(ctrl.getOr("nonexistent", "fallback") == "fallback");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — PreferenceSerializer
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PreferenceSerializeResult factories", "[Phase13][Serializer]") {
    auto ok = PreferenceSerializeResult::ok(5);
    CHECK(ok.succeeded);
    CHECK(ok.entryCount == 5);

    auto fail = PreferenceSerializeResult::fail("oops");
    CHECK_FALSE(fail.succeeded);
    CHECK(fail.errorMessage == "oops");
}

TEST_CASE("PreferenceSerializer serialize writes section", "[Phase13][Serializer]") {
    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark", PreferenceCategory::Appearance));
    reg.registerEntry(PreferenceEntry::makeInt("size", "Size", 14, 8, 32, PreferenceCategory::Appearance));

    WorkspaceProjectFile file;
    file.setProjectId("test");
    file.setProjectName("Test");

    auto res = PreferenceSerializer::serializeRegistry(reg, file);
    CHECK(res.succeeded);
    CHECK(res.entryCount == 2);

    CHECK(file.hasSection(PreferenceSerializer::SECTION_REGISTRY));
}

TEST_CASE("PreferenceSerializer deserialize reads entries", "[Phase13][Serializer]") {
    PreferenceRegistry source;
    source.registerEntry(PreferenceEntry::makeString("a", "A", "val_a", PreferenceCategory::General));
    source.registerEntry(PreferenceEntry::makeBool("b", "B", true, PreferenceCategory::Editor));

    WorkspaceProjectFile file;
    file.setProjectId("test");
    file.setProjectName("Test");
    PreferenceSerializer::serializeRegistry(source, file);

    // Serialize to text and parse back
    std::string text = file.serialize();
    WorkspaceProjectFile parsed;
    REQUIRE(WorkspaceProjectFile::parse(text, parsed));

    PreferenceRegistry dest;
    auto res = PreferenceSerializer::deserializeRegistry(dest, parsed);
    CHECK(res.succeeded);
    CHECK(res.entryCount == 2);
    CHECK(dest.count()   == 2);
    CHECK(dest.isRegistered("a"));
    CHECK(dest.isRegistered("b"));
}

TEST_CASE("PreferenceSerializer deserialize missing section fails", "[Phase13][Serializer]") {
    WorkspaceProjectFile file;
    file.setProjectId("test");
    file.setProjectName("Test");

    PreferenceRegistry dest;
    auto res = PreferenceSerializer::deserializeRegistry(dest, file);
    CHECK_FALSE(res.succeeded);
    CHECK(res.errorMessage == "missing section");
}

TEST_CASE("PreferenceSerializer roundTrip preserves entries", "[Phase13][Serializer]") {
    PreferenceRegistry source;
    source.registerEntry(PreferenceEntry::makeString("theme", "Theme", "dark", PreferenceCategory::Appearance, "UI theme"));
    source.registerEntry(PreferenceEntry::makeInt("size", "Size", 14, 8, 32, PreferenceCategory::Appearance));
    source.registerEntry(PreferenceEntry::makeBool("auto", "Auto Save", true, PreferenceCategory::General));
    source.registerEntry(PreferenceEntry::makeFloat("scale", "Scale", 1.0f, 0.5f, 3.0f, PreferenceCategory::Appearance));

    PreferenceRegistry dest;
    auto res = PreferenceSerializer::roundTrip(source, dest);
    CHECK(res.succeeded);
    CHECK(dest.count() == 4);

    auto* theme = dest.find("theme");
    REQUIRE(theme != nullptr);
    CHECK(theme->displayName  == "Theme");
    CHECK(theme->defaultValue == "dark");
    CHECK(theme->category     == PreferenceCategory::Appearance);

    auto* size = dest.find("size");
    REQUIRE(size != nullptr);
    CHECK(size->type     == PreferenceType::Int);
    CHECK(size->hasRange);
}

TEST_CASE("PreferenceSerializer roundTrip preserves readOnly", "[Phase13][Serializer]") {
    PreferenceRegistry source;
    auto entry = PreferenceEntry::makeString("version", "Version", "1.0");
    entry.readOnly = true;
    source.registerEntry(entry);

    PreferenceRegistry dest;
    auto res = PreferenceSerializer::roundTrip(source, dest);
    CHECK(res.succeeded);
    auto* v = dest.find("version");
    REQUIRE(v != nullptr);
    CHECK(v->readOnly);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration Tests
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: full preference lifecycle", "[Phase13][Integration]") {
    // 1. Create registry and load workspace defaults
    PreferenceRegistry reg;
    reg.loadWorkspaceDefaults();

    // 2. Create settings store and controller with event bus
    SettingsStore store;
    WorkspaceEventBus bus;
    int eventCount = 0;
    bus.subscribe(WorkspaceEventType::System, [&](const WorkspaceEvent&) { ++eventCount; });

    PreferenceController ctrl(reg, store, &bus);
    ctrl.initialize();

    // 3. Read defaults
    CHECK(ctrl.getBool("general.auto_save") == true);
    CHECK(ctrl.getInt("appearance.font_size") == 14);
    CHECK(ctrl.get("appearance.theme") == "dark");

    // 4. Modify preferences
    CHECK(ctrl.set("appearance.theme", "light"));
    CHECK(ctrl.set("appearance.font_size", "18"));
    CHECK(eventCount == 2);

    CHECK(ctrl.get("appearance.theme") == "light");
    CHECK(ctrl.getInt("appearance.font_size") == 18);

    // 5. Validate rejects out-of-range
    CHECK_FALSE(ctrl.set("appearance.font_size", "100"));

    // 6. Reset single preference
    ctrl.resetToDefault("appearance.theme");
    CHECK(ctrl.get("appearance.theme") == "dark");

    // 7. Reset all
    ctrl.resetAll();
    CHECK(ctrl.getInt("appearance.font_size") == 14);
}

TEST_CASE("Integration: preferences survive serialization round-trip", "[Phase13][Integration]") {
    PreferenceRegistry source;
    source.loadWorkspaceDefaults();

    PreferenceRegistry dest;
    auto res = PreferenceSerializer::roundTrip(source, dest);
    CHECK(res.succeeded);
    CHECK(dest.count() == source.count());

    // Verify some specific entries survived
    CHECK(dest.isRegistered("general.auto_save"));
    CHECK(dest.isRegistered("appearance.font_size"));
    CHECK(dest.isRegistered("ai.enabled"));

    auto* fontSize = dest.find("appearance.font_size");
    REQUIRE(fontSize != nullptr);
    CHECK(fontSize->type     == PreferenceType::Int);
    CHECK(fontSize->hasRange);
}

TEST_CASE("Integration: preferences + event bus + notification bus", "[Phase13][Integration]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    PreferenceRegistry reg;
    reg.registerEntry(PreferenceEntry::makeInt("size", "Size", 10, 1, 100));
    SettingsStore store;
    PreferenceController ctrl(reg, store, &bus);
    ctrl.initialize();

    // Subscribe to system events from preferences
    std::vector<std::string> events;
    bus.subscribe(WorkspaceEventType::System, [&](const WorkspaceEvent& ev) {
        events.push_back(ev.payload);
    });

    ctrl.set("size", "20");
    ctrl.set("size", "30");

    CHECK(events.size() == 2);
    CHECK(events[0] == "size=20");
    CHECK(events[1] == "size=30");

    // Notification bus still works alongside
    notifBus.info("Settings", "Updated");
    CHECK(notifBus.historySize() == 1);
}
