#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── DockSlot ─────────────────────────────────────────────────────

TEST_CASE("DockSlot enum values exist", "[Editor][S56]") {
    DockSlot s = DockSlot::Left;
    REQUIRE(s == DockSlot::Left);
    s = DockSlot::Right;
    REQUIRE(s == DockSlot::Right);
    s = DockSlot::Top;
    REQUIRE(s == DockSlot::Top);
    s = DockSlot::Bottom;
    REQUIRE(s == DockSlot::Bottom);
    s = DockSlot::Center;
    REQUIRE(s == DockSlot::Center);
}

// ── DockPanel ────────────────────────────────────────────────────

TEST_CASE("DockPanel defaults", "[Editor][S56]") {
    DockPanel dp;
    REQUIRE(dp.name.empty());
    REQUIRE(dp.slot == DockSlot::Center);
    REQUIRE(dp.visible);
}

TEST_CASE("DockPanel name and slot assignment", "[Editor][S56]") {
    DockPanel dp;
    dp.name = "Scene";
    dp.slot = DockSlot::Left;
    dp.visible = false;
    REQUIRE(dp.name == "Scene");
    REQUIRE(dp.slot == DockSlot::Left);
    REQUIRE_FALSE(dp.visible);
}

TEST_CASE("DockPanel bounds assignment", "[Editor][S56]") {
    DockPanel dp;
    dp.bounds = {0.f, 0.f, 300.f, 600.f};
    REQUIRE(dp.bounds.w == 300.f);
    REQUIRE(dp.bounds.h == 600.f);
}

// ── SelectionService ─────────────────────────────────────────────

TEST_CASE("SelectionService starts empty", "[Editor][S56]") {
    SelectionService svc;
    REQUIRE(svc.selectionCount() == 0);
    REQUIRE_FALSE(svc.hasSelection());
    REQUIRE(svc.primarySelection() == INVALID_ENTITY);
}

TEST_CASE("SelectionService select single entity", "[Editor][S56]") {
    SelectionService svc;
    svc.select(42u);
    REQUIRE(svc.hasSelection());
    REQUIRE(svc.selectionCount() == 1);
    REQUIRE(svc.primarySelection() == 42u);
    REQUIRE(svc.isSelected(42u));
}

TEST_CASE("SelectionService select multiple entities", "[Editor][S56]") {
    SelectionService svc;
    svc.select(1u);
    svc.select(2u);
    svc.select(3u);
    REQUIRE(svc.selectionCount() == 3);
    REQUIRE(svc.isSelected(1u));
    REQUIRE(svc.isSelected(2u));
    REQUIRE(svc.isSelected(3u));
}

TEST_CASE("SelectionService deselect removes entity", "[Editor][S56]") {
    SelectionService svc;
    svc.select(10u);
    svc.select(20u);
    svc.deselect(10u);
    REQUIRE(svc.selectionCount() == 1);
    REQUIRE_FALSE(svc.isSelected(10u));
    REQUIRE(svc.isSelected(20u));
}

TEST_CASE("SelectionService clearSelection empties set", "[Editor][S56]") {
    SelectionService svc;
    svc.select(1u);
    svc.select(2u);
    svc.clearSelection();
    REQUIRE(svc.selectionCount() == 0);
    REQUIRE_FALSE(svc.hasSelection());
    REQUIRE(svc.primarySelection() == INVALID_ENTITY);
}

TEST_CASE("SelectionService toggleSelect adds and removes", "[Editor][S56]") {
    SelectionService svc;
    svc.toggleSelect(5u);
    REQUIRE(svc.isSelected(5u));
    svc.toggleSelect(5u);
    REQUIRE_FALSE(svc.isSelected(5u));
}

TEST_CASE("SelectionService selectExclusive replaces previous selection", "[Editor][S56]") {
    SelectionService svc;
    svc.select(1u);
    svc.select(2u);
    svc.selectExclusive(99u);
    REQUIRE(svc.selectionCount() == 1);
    REQUIRE(svc.isSelected(99u));
    REQUIRE_FALSE(svc.isSelected(1u));
    REQUIRE(svc.primarySelection() == 99u);
}

TEST_CASE("SelectionService version increments on change", "[Editor][S56]") {
    SelectionService svc;
    uint64_t v0 = svc.version();
    svc.select(1u);
    REQUIRE(svc.version() > v0);
    uint64_t v1 = svc.version();
    svc.deselect(1u);
    REQUIRE(svc.version() > v1);
}

TEST_CASE("SelectionService primarySelection updates after deselect", "[Editor][S56]") {
    SelectionService svc;
    svc.select(7u);
    REQUIRE(svc.primarySelection() == 7u);
    svc.deselect(7u);
    REQUIRE(svc.primarySelection() == INVALID_ENTITY);
}

TEST_CASE("SelectionService selection set reference is stable", "[Editor][S56]") {
    SelectionService svc;
    svc.select(3u);
    svc.select(4u);
    const auto& sel = svc.selection();
    REQUIRE(sel.count(3u) == 1);
    REQUIRE(sel.count(4u) == 1);
    REQUIRE(sel.size() == 2);
}
