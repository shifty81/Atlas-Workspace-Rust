#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── SettingEntry ─────────────────────────────────────────────────

TEST_CASE("SettingEntry bool type", "[Editor][S54]") {
    SettingEntry e;
    e.key = "test.bool";
    e.label = "Test Bool";
    e.value = true;
    e.defaultValue = true;
    REQUIRE(e.isBool());
    REQUIRE_FALSE(e.isInt());
    REQUIRE_FALSE(e.isFloat());
    REQUIRE_FALSE(e.isString());
    REQUIRE(e.isDefault());
}

TEST_CASE("SettingEntry int type", "[Editor][S54]") {
    SettingEntry e;
    e.key = "test.int";
    e.value = 42;
    e.defaultValue = 42;
    REQUIRE(e.isInt());
    REQUIRE(e.isDefault());
}

TEST_CASE("SettingEntry float type", "[Editor][S54]") {
    SettingEntry e;
    e.key = "test.float";
    e.value = 3.14f;
    e.defaultValue = 3.14f;
    REQUIRE(e.isFloat());
}

TEST_CASE("SettingEntry string type", "[Editor][S54]") {
    SettingEntry e;
    e.key = "test.str";
    e.value = std::string("hello");
    e.defaultValue = std::string("hello");
    REQUIRE(e.isString());
}

TEST_CASE("SettingEntry resetToDefault", "[Editor][S54]") {
    SettingEntry e;
    e.key = "test";
    e.value = 10;
    e.defaultValue = 5;
    REQUIRE_FALSE(e.isDefault());
    e.resetToDefault();
    REQUIRE(e.isDefault());
    REQUIRE(std::get<int>(e.value) == 5);
}

// ── SettingsCategory ─────────────────────────────────────────────

TEST_CASE("SettingsCategory starts empty", "[Editor][S54]") {
    SettingsCategory cat;
    cat.name = "General";
    REQUIRE(cat.empty());
    REQUIRE(cat.entryCount() == 0);
}

TEST_CASE("SettingsCategory findEntry", "[Editor][S54]") {
    SettingsCategory cat;
    cat.name = "General";
    SettingEntry e;
    e.key = "test.key";
    e.label = "Test";
    e.value = true;
    e.defaultValue = true;
    cat.entries.push_back(e);
    REQUIRE(cat.findEntry("test.key") != nullptr);
    REQUIRE(cat.findEntry("nope") == nullptr);
}

// ── SettingsRegistry ─────────────────────────────────────────────

TEST_CASE("SettingsRegistry starts empty", "[Editor][S54]") {
    SettingsRegistry reg;
    REQUIRE(reg.categoryCount() == 0);
    REQUIRE(reg.totalSettings() == 0);
    REQUIRE(reg.changeCount() == 0);
}

TEST_CASE("SettingsRegistry addCategory", "[Editor][S54]") {
    SettingsRegistry reg;
    auto* cat = reg.addCategory("General");
    REQUIRE(cat != nullptr);
    REQUIRE(reg.categoryCount() == 1);
    // Adding same name returns existing
    auto* cat2 = reg.addCategory("General");
    REQUIRE(cat2 == cat);
    REQUIRE(reg.categoryCount() == 1);
}

TEST_CASE("SettingsRegistry removeCategory", "[Editor][S54]") {
    SettingsRegistry reg;
    reg.addCategory("Test");
    REQUIRE(reg.removeCategory("Test"));
    REQUIRE(reg.categoryCount() == 0);
    REQUIRE_FALSE(reg.removeCategory("Test"));
}

TEST_CASE("SettingsRegistry addSetting", "[Editor][S54]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "gen.dark";
    e.label = "Dark Mode";
    e.category = "General";
    e.value = true;
    e.defaultValue = true;
    REQUIRE(reg.addSetting("General", e));
    REQUIRE(reg.totalSettings() == 1);
    REQUIRE(reg.findSetting("gen.dark") != nullptr);
}

TEST_CASE("SettingsRegistry addSetting duplicate key fails", "[Editor][S54]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "gen.dark";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    REQUIRE_FALSE(reg.addSetting("General", e));
}

TEST_CASE("SettingsRegistry setSetting", "[Editor][S54]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "gen.dark";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    REQUIRE(reg.setSetting("gen.dark", false));
    REQUIRE(std::get<bool>(reg.findSetting("gen.dark")->value) == false);
    REQUIRE(reg.changeCount() == 1);
}

TEST_CASE("SettingsRegistry setSetting nonexistent fails", "[Editor][S54]") {
    SettingsRegistry reg;
    REQUIRE_FALSE(reg.setSetting("nope", true));
}

TEST_CASE("SettingsRegistry setSetting readOnly fails", "[Editor][S54]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "gen.version";
    e.value = std::string("1.0");
    e.defaultValue = std::string("1.0");
    e.readOnly = true;
    reg.addSetting("General", e);
    REQUIRE_FALSE(reg.setSetting("gen.version", std::string("2.0")));
}

TEST_CASE("SettingsRegistry resetAll", "[Editor][S54]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "gen.dark";
    e.value = false;
    e.defaultValue = true;
    reg.addSetting("General", e);
    reg.resetAll();
    REQUIRE(std::get<bool>(reg.findSetting("gen.dark")->value) == true);
}

TEST_CASE("SettingsRegistry loadDefaults", "[Editor][S54]") {
    SettingsRegistry reg;
    reg.loadDefaults();
    REQUIRE(reg.totalSettings() == 8);
    REQUIRE(reg.findSetting("general.dark_mode") != nullptr);
    REQUIRE(reg.findSetting("viewport.show_grid") != nullptr);
    REQUIRE(reg.findSetting("viewport.camera_speed") != nullptr);
    REQUIRE(reg.findSetting("input.snap_enabled") != nullptr);
}

TEST_CASE("SettingsRegistry findCategory", "[Editor][S54]") {
    SettingsRegistry reg;
    reg.loadDefaults();
    REQUIRE(reg.findCategory("General") != nullptr);
    REQUIRE(reg.findCategory("Viewport") != nullptr);
    REQUIRE(reg.findCategory("Input") != nullptr);
    REQUIRE(reg.findCategory("Nonexistent") == nullptr);
}

// ── EditorSettingsPanel ──────────────────────────────────────────

TEST_CASE("EditorSettingsPanel name and slot", "[Editor][S54]") {
    EditorSettingsPanel panel;
    REQUIRE(panel.name() == "Settings");
    REQUIRE(panel.slot() == DockSlot::Right);
}

TEST_CASE("EditorSettingsPanel registry access", "[Editor][S54]") {
    EditorSettingsPanel panel;
    panel.registry().loadDefaults();
    REQUIRE(panel.registry().totalSettings() == 8);
}

TEST_CASE("EditorSettingsPanel search filter", "[Editor][S54]") {
    EditorSettingsPanel panel;
    panel.setSearchFilter("dark");
    REQUIRE(panel.searchFilter() == "dark");
}

TEST_CASE("SettingsRegistry MAX_CATEGORIES limit", "[Editor][S54]") {
    SettingsRegistry reg;
    for (size_t i = 0; i < SettingsRegistry::MAX_CATEGORIES; ++i) {
        reg.addCategory("Cat" + std::to_string(i));
    }
    REQUIRE(reg.addCategory("Extra") == nullptr);
}
