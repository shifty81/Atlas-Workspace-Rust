#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S67: SettingsPanel ───────────────────────────────────────────

TEST_CASE("SettingEntry bool isDefault when unchanged", "[Editor][S67]") {
    SettingEntry e;
    e.value = true;
    e.defaultValue = true;
    REQUIRE(e.isDefault());
}

TEST_CASE("SettingEntry isDefault false when changed", "[Editor][S67]") {
    SettingEntry e;
    e.value = false;
    e.defaultValue = true;
    REQUIRE_FALSE(e.isDefault());
}

TEST_CASE("SettingEntry resetToDefault restores value", "[Editor][S67]") {
    SettingEntry e;
    e.value = false;
    e.defaultValue = true;
    e.resetToDefault();
    REQUIRE(e.isDefault());
}

TEST_CASE("SettingEntry isBool true for bool variant", "[Editor][S67]") {
    SettingEntry e;
    e.value = true;
    REQUIRE(e.isBool());
    REQUIRE_FALSE(e.isInt());
    REQUIRE_FALSE(e.isFloat());
    REQUIRE_FALSE(e.isString());
}

TEST_CASE("SettingEntry isInt true for int variant", "[Editor][S67]") {
    SettingEntry e;
    e.value = 42;
    REQUIRE(e.isInt());
    REQUIRE_FALSE(e.isBool());
}

TEST_CASE("SettingEntry isFloat true for float variant", "[Editor][S67]") {
    SettingEntry e;
    e.value = 3.14f;
    REQUIRE(e.isFloat());
}

TEST_CASE("SettingEntry isString true for string variant", "[Editor][S67]") {
    SettingEntry e;
    e.value = std::string("hello");
    REQUIRE(e.isString());
}

TEST_CASE("SettingsCategory starts empty", "[Editor][S67]") {
    SettingsCategory cat;
    cat.name = "General";
    REQUIRE(cat.empty());
    REQUIRE(cat.entryCount() == 0);
}

TEST_CASE("SettingsCategory findEntry returns nullptr when missing", "[Editor][S67]") {
    SettingsCategory cat;
    cat.name = "General";
    REQUIRE(cat.findEntry("missing") == nullptr);
}

TEST_CASE("SettingsCategory findEntry returns entry after add", "[Editor][S67]") {
    SettingsCategory cat;
    cat.name = "General";
    SettingEntry e;
    e.key = "dark_mode";
    e.value = true;
    cat.entries.push_back(e);
    REQUIRE(cat.findEntry("dark_mode") != nullptr);
    REQUIRE(cat.entryCount() == 1);
}

TEST_CASE("SettingsRegistry starts with no categories", "[Editor][S67]") {
    SettingsRegistry reg;
    REQUIRE(reg.categoryCount() == 0);
    REQUIRE(reg.totalSettings() == 0);
}

TEST_CASE("SettingsRegistry addSetting creates category", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "dark_mode";
    e.label = "Dark Mode";
    e.value = true;
    e.defaultValue = true;
    bool added = reg.addSetting("General", e);
    REQUIRE(added);
    REQUIRE(reg.categoryCount() == 1);
    REQUIRE(reg.totalSettings() == 1);
}

TEST_CASE("SettingsRegistry addSetting rejects duplicate key", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "dark_mode";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    bool second = reg.addSetting("General", e);
    REQUIRE_FALSE(second);
    REQUIRE(reg.totalSettings() == 1);
}

TEST_CASE("SettingsRegistry findSetting returns entry", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "autosave";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    const auto* found = reg.findSetting("autosave");
    REQUIRE(found != nullptr);
    REQUIRE(found->key == "autosave");
}

TEST_CASE("SettingsRegistry findSetting returns nullptr for missing key", "[Editor][S67]") {
    SettingsRegistry reg;
    REQUIRE(reg.findSetting("nonexistent") == nullptr);
}

TEST_CASE("SettingsRegistry setSetting updates value", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "dark_mode";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    bool ok = reg.setSetting("dark_mode", false);
    REQUIRE(ok);
    const auto* found = reg.findSetting("dark_mode");
    REQUIRE(std::get<bool>(found->value) == false);
}

TEST_CASE("SettingsRegistry setSetting increments changeCount", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "dark_mode";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    size_t before = reg.changeCount();
    reg.setSetting("dark_mode", false);
    REQUIRE(reg.changeCount() == before + 1);
}

TEST_CASE("SettingsRegistry setSetting fails on readOnly", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "engine_ver";
    e.value = std::string("1.0");
    e.defaultValue = std::string("1.0");
    e.readOnly = true;
    reg.addSetting("Engine", e);
    bool ok = reg.setSetting("engine_ver", std::string("2.0"));
    REQUIRE_FALSE(ok);
}

TEST_CASE("SettingsRegistry setSetting fails for missing key", "[Editor][S67]") {
    SettingsRegistry reg;
    bool ok = reg.setSetting("missing", true);
    REQUIRE_FALSE(ok);
}

TEST_CASE("SettingsRegistry resetAll restores defaults", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "dark_mode";
    e.value = true;
    e.defaultValue = true;
    reg.addSetting("General", e);
    reg.setSetting("dark_mode", false);
    reg.resetAll();
    const auto* found = reg.findSetting("dark_mode");
    REQUIRE(std::get<bool>(found->value) == true);
}

TEST_CASE("SettingsRegistry resetAll increments changeCount", "[Editor][S67]") {
    SettingsRegistry reg;
    SettingEntry e;
    e.key = "x";
    e.value = 1;
    e.defaultValue = 1;
    reg.addSetting("A", e);
    size_t before = reg.changeCount();
    reg.resetAll();
    REQUIRE(reg.changeCount() > before);
}

TEST_CASE("SettingsRegistry MAX_CATEGORIES is 32", "[Editor][S67]") {
    REQUIRE(SettingsRegistry::MAX_CATEGORIES == 32);
}

TEST_CASE("SettingsRegistry loadDefaults populates categories", "[Editor][S67]") {
    SettingsRegistry reg;
    reg.loadDefaults();
    REQUIRE(reg.categoryCount() > 0);
    REQUIRE(reg.totalSettings() > 0);
}

TEST_CASE("SettingsRegistry loadDefaults has General category", "[Editor][S67]") {
    SettingsRegistry reg;
    reg.loadDefaults();
    REQUIRE(reg.findCategory("General") != nullptr);
}

TEST_CASE("SettingsRegistry loadDefaults has dark_mode setting", "[Editor][S67]") {
    SettingsRegistry reg;
    reg.loadDefaults();
    REQUIRE(reg.findSetting("general.dark_mode") != nullptr);
}

TEST_CASE("EditorSettingsPanel name is Settings", "[Editor][S67]") {
    EditorSettingsPanel panel;
    REQUIRE(panel.name() == "Settings");
}

TEST_CASE("EditorSettingsPanel slot is Right", "[Editor][S67]") {
    EditorSettingsPanel panel;
    REQUIRE(panel.slot() == DockSlot::Right);
}

TEST_CASE("EditorSettingsPanel registry starts empty", "[Editor][S67]") {
    EditorSettingsPanel panel;
    REQUIRE(panel.registry().categoryCount() == 0);
}

TEST_CASE("EditorSettingsPanel registry is mutable", "[Editor][S67]") {
    EditorSettingsPanel panel;
    SettingEntry e;
    e.key = "test_key";
    e.value = 42;
    e.defaultValue = 42;
    panel.registry().addSetting("Test", e);
    REQUIRE(panel.registry().totalSettings() == 1);
}

TEST_CASE("EditorSettingsPanel searchFilter starts empty", "[Editor][S67]") {
    EditorSettingsPanel panel;
    REQUIRE(panel.searchFilter().empty());
}

TEST_CASE("EditorSettingsPanel setSearchFilter stores value", "[Editor][S67]") {
    EditorSettingsPanel panel;
    panel.setSearchFilter("dark");
    REQUIRE(panel.searchFilter() == "dark");
}

TEST_CASE("EditorSettingsPanel is an EditorPanel", "[Editor][S67]") {
    EditorSettingsPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base != nullptr);
}
