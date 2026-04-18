#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- ShortcutCategory names ---

TEST_CASE("ShortcutCategory names cover all 8 values", "[Editor][S23]") {
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::File))     == "File");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::Edit))     == "Edit");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::View))     == "View");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::Navigate)) == "Navigate");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::Select))   == "Select");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::Debug))    == "Debug");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::Tool))     == "Tool");
    REQUIRE(std::string(shortcutCategoryName(ShortcutCategory::Custom))   == "Custom");
}

// --- ShortcutState names ---

TEST_CASE("ShortcutState names cover all 4 values", "[Editor][S23]") {
    REQUIRE(std::string(shortcutStateName(ShortcutState::Inactive)) == "Inactive");
    REQUIRE(std::string(shortcutStateName(ShortcutState::Active))   == "Active");
    REQUIRE(std::string(shortcutStateName(ShortcutState::Pressed))  == "Pressed");
    REQUIRE(std::string(shortcutStateName(ShortcutState::Blocked))  == "Blocked");
}

// --- ShortcutBinding ---

TEST_CASE("ShortcutBinding enable/disable/trigger/reset", "[Editor][S23]") {
    ShortcutBinding b;
    b.id = "save"; b.name = "Save"; b.key = "S";

    REQUIRE(b.isEnabled());
    b.disable();
    REQUIRE_FALSE(b.isEnabled());
    b.enable();
    REQUIRE(b.isEnabled());

    b.trigger();
    REQUIRE(b.state == ShortcutState::Pressed);
    b.reset();
    REQUIRE(b.state == ShortcutState::Inactive);
}

TEST_CASE("ShortcutBinding isActive only when Pressed", "[Editor][S23]") {
    ShortcutBinding b;
    b.id = "undo"; b.key = "Z";

    REQUIRE_FALSE(b.isActive());
    b.trigger();
    REQUIRE(b.isActive());
    b.reset();
    REQUIRE_FALSE(b.isActive());
}

TEST_CASE("ShortcutBinding hasKey returns false for empty key", "[Editor][S23]") {
    ShortcutBinding b;
    b.id = "noop";
    REQUIRE_FALSE(b.hasKey());
    b.key = "F5";
    REQUIRE(b.hasKey());
}

// --- ShortcutContext ---

TEST_CASE("ShortcutContext addBinding and duplicate rejection", "[Editor][S23]") {
    ShortcutContext ctx("main");
    ShortcutBinding b1; b1.id = "save"; b1.key = "S";
    ShortcutBinding b2; b2.id = "open"; b2.key = "O";
    ShortcutBinding dup; dup.id = "save"; dup.key = "S";

    REQUIRE(ctx.addBinding(b1));
    REQUIRE(ctx.addBinding(b2));
    REQUIRE_FALSE(ctx.addBinding(dup));
    REQUIRE(ctx.bindingCount() == 2);
}

TEST_CASE("ShortcutContext removeBinding by id", "[Editor][S23]") {
    ShortcutContext ctx("main");
    ShortcutBinding b; b.id = "cut"; b.key = "X";
    ctx.addBinding(b);
    REQUIRE(ctx.bindingCount() == 1);
    REQUIRE(ctx.removeBinding("cut"));
    REQUIRE(ctx.bindingCount() == 0);
    REQUIRE_FALSE(ctx.removeBinding("cut"));
}

TEST_CASE("ShortcutContext activeCount counts enabled bindings", "[Editor][S23]") {
    ShortcutContext ctx("editor");
    ShortcutBinding a; a.id = "a"; a.key = "A"; a.enabled = true;
    ShortcutBinding b; b.id = "b"; b.key = "B"; b.enabled = false;
    ctx.addBinding(a);
    ctx.addBinding(b);
    REQUIRE(ctx.activeCount() == 1);
}

TEST_CASE("ShortcutContext enableAll and disableAll", "[Editor][S23]") {
    ShortcutContext ctx("editor");
    ShortcutBinding a; a.id = "a"; a.key = "A";
    ShortcutBinding b; b.id = "b"; b.key = "B";
    ctx.addBinding(a);
    ctx.addBinding(b);

    ctx.disableAll();
    REQUIRE(ctx.activeCount() == 0);
    ctx.enableAll();
    REQUIRE(ctx.activeCount() == 2);
}

// --- ShortcutManager ---

TEST_CASE("ShortcutManager createContext and duplicate rejection", "[Editor][S23]") {
    ShortcutManager mgr;
    REQUIRE(mgr.createContext("global")  != nullptr);
    REQUIRE(mgr.createContext("scene")   != nullptr);
    REQUIRE(mgr.createContext("global")  == nullptr); // duplicate
    REQUIRE(mgr.contextCount() == 2);
}

TEST_CASE("ShortcutManager setActiveContext + hasActive + activeName", "[Editor][S23]") {
    ShortcutManager mgr;
    mgr.createContext("global");
    REQUIRE_FALSE(mgr.hasActive());
    REQUIRE(mgr.setActiveContext("global"));
    REQUIRE(mgr.hasActive());
    REQUIRE(mgr.activeName() == "global");
    REQUIRE(mgr.activeContext() != nullptr);
}

TEST_CASE("ShortcutManager setActiveContext rejects unknown context", "[Editor][S23]") {
    ShortcutManager mgr;
    mgr.createContext("global");
    REQUIRE_FALSE(mgr.setActiveContext("unknown"));
    REQUIRE_FALSE(mgr.hasActive());
}

TEST_CASE("ShortcutManager removeContext clears active if removed", "[Editor][S23]") {
    ShortcutManager mgr;
    mgr.createContext("ctx");
    mgr.setActiveContext("ctx");
    REQUIRE(mgr.hasActive());
    REQUIRE(mgr.removeContext("ctx"));
    REQUIRE_FALSE(mgr.hasActive());
    REQUIRE(mgr.contextCount() == 0);
}

TEST_CASE("ShortcutManager MAX_CONTEXTS limit", "[Editor][S23]") {
    ShortcutManager mgr;
    for (size_t i = 0; i < ShortcutManager::MAX_CONTEXTS; ++i) {
        REQUIRE(mgr.createContext("ctx" + std::to_string(i)) != nullptr);
    }
    REQUIRE(mgr.createContext("overflow") == nullptr);
    REQUIRE(mgr.contextCount() == ShortcutManager::MAX_CONTEXTS);
}
