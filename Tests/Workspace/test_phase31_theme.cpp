// Tests/Workspace/test_phase31_theme.cpp
// Phase 31 — Workspace Theme System
//
// Tests for:
//   1. ThemeSlot       — enum name helpers
//   2. ThemeColorMap   — set/get/isDefined/reset/allDefined/definedCount
//   3. ThemeDescriptor — isValid; identity fields
//   4. ThemeEnforcer   — enforce pass / missing slots
//   5. ThemeRegistry   — register/unregister/find/apply/active/observers
//   6. Integration     — full theme pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceTheme.h"
#include <string>
#include <vector>

using namespace NF;

// Helper: build a fully-defined ThemeDescriptor
static ThemeDescriptor makeFullTheme(const std::string& id,
                                     const std::string& name = "Test Theme",
                                     bool builtIn = false) {
    ThemeDescriptor d;
    d.id          = id;
    d.displayName = name;
    d.author      = "TestAuthor";
    d.isBuiltIn   = builtIn;
    for (int i = 0; i < kThemeSlotCount; ++i)
        d.colorMap.set(static_cast<ThemeSlot>(i), 0xFF000000u + static_cast<uint32_t>(i));
    return d;
}

// ─────────────────────────────────────────────────────────────────
// 1. ThemeSlot enum name helpers
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ThemeSlot name helpers", "[ThemeSlot]") {
    CHECK(std::string(themeSlotName(ThemeSlot::Background))         == "Background");
    CHECK(std::string(themeSlotName(ThemeSlot::Surface))            == "Surface");
    CHECK(std::string(themeSlotName(ThemeSlot::Border))             == "Border");
    CHECK(std::string(themeSlotName(ThemeSlot::Accent))             == "Accent");
    CHECK(std::string(themeSlotName(ThemeSlot::AccentHover))        == "AccentHover");
    CHECK(std::string(themeSlotName(ThemeSlot::AccentActive))       == "AccentActive");
    CHECK(std::string(themeSlotName(ThemeSlot::TextPrimary))        == "TextPrimary");
    CHECK(std::string(themeSlotName(ThemeSlot::TextSecondary))      == "TextSecondary");
    CHECK(std::string(themeSlotName(ThemeSlot::TextDisabled))       == "TextDisabled");
    CHECK(std::string(themeSlotName(ThemeSlot::IconPrimary))        == "IconPrimary");
    CHECK(std::string(themeSlotName(ThemeSlot::IconSecondary))      == "IconSecondary");
    CHECK(std::string(themeSlotName(ThemeSlot::SelectionHighlight)) == "SelectionHighlight");
    CHECK(std::string(themeSlotName(ThemeSlot::ErrorColor))         == "ErrorColor");
    CHECK(std::string(themeSlotName(ThemeSlot::WarningColor))       == "WarningColor");
    CHECK(kThemeSlotCount == 14);
}

// ─────────────────────────────────────────────────────────────────
// 2. ThemeColorMap
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ThemeColorMap default state", "[ThemeColorMap]") {
    ThemeColorMap m;
    CHECK(m.definedCount() == 0);
    CHECK_FALSE(m.allDefined());
    CHECK_FALSE(m.isDefined(ThemeSlot::Background));
    CHECK(m.get(ThemeSlot::Background) == ThemeColorMap::kDefaultColor);
}

TEST_CASE("ThemeColorMap set and get", "[ThemeColorMap]") {
    ThemeColorMap m;
    m.set(ThemeSlot::Accent, 0xFF4488CCu);
    CHECK(m.isDefined(ThemeSlot::Accent));
    CHECK(m.get(ThemeSlot::Accent) == 0xFF4488CCu);
    CHECK(m.definedCount() == 1);
}

TEST_CASE("ThemeColorMap isDefined false for unset slot", "[ThemeColorMap]") {
    ThemeColorMap m;
    m.set(ThemeSlot::Background, 0xFF112233u);
    CHECK_FALSE(m.isDefined(ThemeSlot::Surface));
}

TEST_CASE("ThemeColorMap reset single slot", "[ThemeColorMap]") {
    ThemeColorMap m;
    m.set(ThemeSlot::TextPrimary, 0xFFFFFFFFu);
    CHECK(m.isDefined(ThemeSlot::TextPrimary));
    m.reset(ThemeSlot::TextPrimary);
    CHECK_FALSE(m.isDefined(ThemeSlot::TextPrimary));
    CHECK(m.get(ThemeSlot::TextPrimary) == ThemeColorMap::kDefaultColor);
}

TEST_CASE("ThemeColorMap resetAll clears all slots", "[ThemeColorMap]") {
    ThemeColorMap m;
    for (int i = 0; i < kThemeSlotCount; ++i)
        m.set(static_cast<ThemeSlot>(i), 0xAABBCCDDu);
    CHECK(m.allDefined());
    m.resetAll();
    CHECK(m.definedCount() == 0);
    CHECK_FALSE(m.allDefined());
}

TEST_CASE("ThemeColorMap allDefined requires all 14 slots", "[ThemeColorMap]") {
    ThemeColorMap m;
    for (int i = 0; i < kThemeSlotCount - 1; ++i)
        m.set(static_cast<ThemeSlot>(i), 0x11223344u);
    CHECK_FALSE(m.allDefined());
    m.set(static_cast<ThemeSlot>(kThemeSlotCount - 1), 0x11223344u);
    CHECK(m.allDefined());
}

TEST_CASE("ThemeColorMap definedCount tracks correctly", "[ThemeColorMap]") {
    ThemeColorMap m;
    for (int i = 0; i < 5; ++i)
        m.set(static_cast<ThemeSlot>(i), 0xFFFFFFFFu);
    CHECK(m.definedCount() == 5);
}

// ─────────────────────────────────────────────────────────────────
// 3. ThemeDescriptor
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ThemeDescriptor default is invalid", "[ThemeDescriptor]") {
    ThemeDescriptor d;
    CHECK_FALSE(d.isValid());
}

TEST_CASE("ThemeDescriptor valid with id and displayName", "[ThemeDescriptor]") {
    ThemeDescriptor d;
    d.id          = "dark";
    d.displayName = "Dark Theme";
    CHECK(d.isValid());
}

TEST_CASE("ThemeDescriptor invalid without id", "[ThemeDescriptor]") {
    ThemeDescriptor d;
    d.displayName = "Dark Theme";
    CHECK_FALSE(d.isValid());
}

TEST_CASE("ThemeDescriptor invalid without displayName", "[ThemeDescriptor]") {
    ThemeDescriptor d;
    d.id = "dark";
    CHECK_FALSE(d.isValid());
}

TEST_CASE("ThemeDescriptor isBuiltIn flag", "[ThemeDescriptor]") {
    ThemeDescriptor d = makeFullTheme("builtin", "Built-In", true);
    CHECK(d.isBuiltIn);
}

// ─────────────────────────────────────────────────────────────────
// 4. ThemeEnforcer
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ThemeEnforcer passes fully-defined theme", "[ThemeEnforcer]") {
    ThemeEnforcer  enforcer;
    ThemeDescriptor d = makeFullTheme("dark", "Dark Theme");
    auto report = enforcer.enforce(d);
    CHECK(report.passed);
    CHECK(report.violationCount() == 0);
}

TEST_CASE("ThemeEnforcer reports violation for each missing slot", "[ThemeEnforcer]") {
    ThemeEnforcer  enforcer;
    ThemeDescriptor d;
    d.id          = "partial";
    d.displayName = "Partial";
    // define only half
    for (int i = 0; i < kThemeSlotCount / 2; ++i)
        d.colorMap.set(static_cast<ThemeSlot>(i), 0xFFFFFFFFu);
    auto report = enforcer.enforce(d);
    CHECK_FALSE(report.passed);
    CHECK(report.violationCount() == kThemeSlotCount - kThemeSlotCount / 2);
}

TEST_CASE("ThemeEnforcer reports violation for invalid descriptor", "[ThemeEnforcer]") {
    ThemeEnforcer   enforcer;
    ThemeDescriptor d; // invalid: no id or displayName
    auto report = enforcer.enforce(d);
    CHECK_FALSE(report.passed);
    CHECK(report.violationCount() >= 1);
}

TEST_CASE("ThemeEnforcer violation carries slot info", "[ThemeEnforcer]") {
    ThemeEnforcer   enforcer;
    ThemeDescriptor d;
    d.id          = "missing_warning";
    d.displayName = "Missing Warning";
    // define all except WarningColor
    for (int i = 0; i < kThemeSlotCount; ++i) {
        auto s = static_cast<ThemeSlot>(i);
        if (s != ThemeSlot::WarningColor)
            d.colorMap.set(s, 0xFFFFFFFFu);
    }
    auto report = enforcer.enforce(d);
    CHECK_FALSE(report.passed);
    REQUIRE(report.violationCount() == 1);
    CHECK(report.violations[0].slot == ThemeSlot::WarningColor);
}

// ─────────────────────────────────────────────────────────────────
// 5. ThemeRegistry
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ThemeRegistry empty state", "[ThemeRegistry]") {
    ThemeRegistry reg;
    CHECK(reg.empty());
    CHECK(reg.count() == 0);
    CHECK(reg.activeThemeId().empty());
    CHECK(reg.activeTheme() == nullptr);
}

TEST_CASE("ThemeRegistry register theme", "[ThemeRegistry]") {
    ThemeRegistry reg;
    auto d = makeFullTheme("dark", "Dark Theme");
    CHECK(reg.registerTheme(d));
    CHECK(reg.count() == 1);
    CHECK(reg.contains("dark"));
}

TEST_CASE("ThemeRegistry duplicate registration fails", "[ThemeRegistry]") {
    ThemeRegistry reg;
    auto d = makeFullTheme("dark", "Dark Theme");
    CHECK(reg.registerTheme(d));
    CHECK_FALSE(reg.registerTheme(d));
    CHECK(reg.count() == 1);
}

TEST_CASE("ThemeRegistry invalid registration fails", "[ThemeRegistry]") {
    ThemeRegistry   reg;
    ThemeDescriptor d; // invalid
    CHECK_FALSE(reg.registerTheme(d));
    CHECK(reg.empty());
}

TEST_CASE("ThemeRegistry find returns correct descriptor", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark", "Dark Theme"));
    reg.registerTheme(makeFullTheme("light", "Light Theme"));
    const auto* p = reg.find("light");
    REQUIRE(p != nullptr);
    CHECK(p->displayName == "Light Theme");
}

TEST_CASE("ThemeRegistry find returns null for unknown id", "[ThemeRegistry]") {
    ThemeRegistry reg;
    CHECK(reg.find("nope") == nullptr);
}

TEST_CASE("ThemeRegistry apply theme sets active", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark", "Dark Theme"));
    CHECK(reg.applyTheme("dark"));
    CHECK(reg.activeThemeId() == "dark");
    REQUIRE(reg.activeTheme() != nullptr);
    CHECK(reg.activeTheme()->id == "dark");
}

TEST_CASE("ThemeRegistry apply unknown theme fails", "[ThemeRegistry]") {
    ThemeRegistry reg;
    CHECK_FALSE(reg.applyTheme("nope"));
    CHECK(reg.activeThemeId().empty());
}

TEST_CASE("ThemeRegistry unregister removes theme", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark",  "Dark Theme"));
    reg.registerTheme(makeFullTheme("light", "Light Theme"));
    CHECK(reg.unregisterTheme("light"));
    CHECK_FALSE(reg.contains("light"));
    CHECK(reg.count() == 1);
}

TEST_CASE("ThemeRegistry cannot unregister active theme", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark", "Dark Theme"));
    reg.applyTheme("dark");
    CHECK_FALSE(reg.unregisterTheme("dark"));
    CHECK(reg.contains("dark"));
}

TEST_CASE("ThemeRegistry unregister unknown fails", "[ThemeRegistry]") {
    ThemeRegistry reg;
    CHECK_FALSE(reg.unregisterTheme("nope"));
}

TEST_CASE("ThemeRegistry allIds returns all registered ids", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark",  "Dark Theme"));
    reg.registerTheme(makeFullTheme("light", "Light Theme"));
    auto ids = reg.allIds();
    CHECK(ids.size() == 2);
    bool hasDark  = std::find(ids.begin(), ids.end(), "dark")  != ids.end();
    bool hasLight = std::find(ids.begin(), ids.end(), "light") != ids.end();
    CHECK(hasDark);
    CHECK(hasLight);
}

TEST_CASE("ThemeRegistry observer fires on applyTheme", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark", "Dark Theme"));
    std::string notified;
    reg.addObserver([&](const std::string& id){ notified = id; });
    reg.applyTheme("dark");
    CHECK(notified == "dark");
}

TEST_CASE("ThemeRegistry observer fires on theme switch", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark",  "Dark Theme"));
    reg.registerTheme(makeFullTheme("light", "Light Theme"));
    reg.applyTheme("dark");
    std::string last;
    reg.addObserver([&](const std::string& id){ last = id; });
    reg.applyTheme("light");
    CHECK(last == "light");
}

TEST_CASE("ThemeRegistry clearObservers removes all observers", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark", "Dark Theme"));
    int calls = 0;
    reg.addObserver([&](const std::string&){ ++calls; });
    reg.clearObservers();
    reg.applyTheme("dark");
    CHECK(calls == 0);
}

TEST_CASE("ThemeRegistry clear resets registry", "[ThemeRegistry]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("dark", "Dark Theme"));
    reg.applyTheme("dark");
    reg.clear();
    CHECK(reg.empty());
    CHECK(reg.activeThemeId().empty());
}

// ─────────────────────────────────────────────────────────────────
// 6. Integration — full theme pipeline
// ─────────────────────────────────────────────────────────────────
TEST_CASE("Theme integration: register, enforce, apply, switch", "[ThemeIntegration]") {
    ThemeRegistry  reg;
    ThemeEnforcer  enforcer;

    auto dark  = makeFullTheme("dark",  "Dark Theme");
    auto light = makeFullTheme("light", "Light Theme");

    CHECK(enforcer.enforce(dark).passed);
    CHECK(enforcer.enforce(light).passed);

    reg.registerTheme(dark);
    reg.registerTheme(light);

    reg.applyTheme("dark");
    CHECK(reg.activeThemeId() == "dark");

    reg.applyTheme("light");
    CHECK(reg.activeThemeId() == "light");

    CHECK_FALSE(reg.unregisterTheme("light")); // active
    CHECK(reg.contains("light"));
}

TEST_CASE("Theme integration: invalid theme rejected by enforcer", "[ThemeIntegration]") {
    ThemeEnforcer   enforcer;
    ThemeDescriptor partial;
    partial.id          = "partial";
    partial.displayName = "Partial";
    partial.colorMap.set(ThemeSlot::Background, 0xFF111111u);
    auto report = enforcer.enforce(partial);
    CHECK_FALSE(report.passed);
    CHECK(report.violationCount() == kThemeSlotCount - 1);
}

TEST_CASE("Theme integration: multiple observers fire in order", "[ThemeIntegration]") {
    ThemeRegistry reg;
    reg.registerTheme(makeFullTheme("a", "Theme A"));
    std::vector<std::string> log;
    reg.addObserver([&](const std::string& id){ log.push_back("obs1:" + id); });
    reg.addObserver([&](const std::string& id){ log.push_back("obs2:" + id); });
    reg.applyTheme("a");
    REQUIRE(log.size() == 2);
    CHECK(log[0] == "obs1:a");
    CHECK(log[1] == "obs2:a");
}
