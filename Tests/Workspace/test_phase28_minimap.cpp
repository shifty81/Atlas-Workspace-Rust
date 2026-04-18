// Tests/Workspace/test_phase28_minimap.cpp
// Phase 28 — Workspace Minimap / Overview
//
// Tests for:
//   1. MinimapRect     — normalized rect; isValid, equality
//   2. MinimapRegion   — labeled minimap area; isValid, equality
//   3. MinimapViewport — visible window descriptor; isValid, locked flag
//   4. MinimapManager  — region registry with viewport scroll and observers
//   5. Integration     — full minimap pipeline

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Workspace/WorkspaceMinimap.h"
#include <string>
#include <vector>

using Catch::Matchers::WithinAbs;

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — MinimapRect
// ═════════════════════════════════════════════════════════════════

TEST_CASE("MinimapRect default is invalid", "[Phase28][MinimapRect]") {
    MinimapRect r;
    CHECK_FALSE(r.isValid());
    CHECK(r.x == 0.0f);
    CHECK(r.y == 0.0f);
    CHECK(r.w == 0.0f);
    CHECK(r.h == 0.0f);
}

TEST_CASE("MinimapRect valid with positive size", "[Phase28][MinimapRect]") {
    MinimapRect r{0.1f, 0.2f, 0.5f, 0.4f};
    CHECK(r.isValid());
}

TEST_CASE("MinimapRect invalid with zero width", "[Phase28][MinimapRect]") {
    MinimapRect r{0.0f, 0.0f, 0.0f, 0.5f};
    CHECK_FALSE(r.isValid());
}

TEST_CASE("MinimapRect equality", "[Phase28][MinimapRect]") {
    MinimapRect a{0.1f, 0.2f, 0.3f, 0.4f};
    MinimapRect b{0.1f, 0.2f, 0.3f, 0.4f};
    MinimapRect c{0.0f, 0.0f, 1.0f, 1.0f};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — MinimapRegion
// ═════════════════════════════════════════════════════════════════

TEST_CASE("MinimapRegion default is invalid", "[Phase28][MinimapRegion]") {
    MinimapRegion r;
    CHECK_FALSE(r.isValid());
    CHECK(r.id.empty());
    CHECK(r.visible);
}

TEST_CASE("MinimapRegion valid construction", "[Phase28][MinimapRegion]") {
    MinimapRegion r{"region_a", "Scene", {0.0f, 0.0f, 0.5f, 0.5f}, 0xFF0000FF, true};
    CHECK(r.isValid());
    CHECK(r.id    == "region_a");
    CHECK(r.label == "Scene");
    CHECK(r.color == 0xFF0000FF);
    CHECK(r.visible);
}

TEST_CASE("MinimapRegion invalid without id", "[Phase28][MinimapRegion]") {
    MinimapRegion r{"", "Label", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true};
    CHECK_FALSE(r.isValid());
}

TEST_CASE("MinimapRegion invalid with zero-size rect", "[Phase28][MinimapRegion]") {
    MinimapRegion r{"region_b", "B", {0.0f, 0.0f, 0.0f, 0.0f}, 0, true};
    CHECK_FALSE(r.isValid());
}

TEST_CASE("MinimapRegion equality by id", "[Phase28][MinimapRegion]") {
    MinimapRegion a{"id_1", "A", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true};
    MinimapRegion b{"id_1", "B", {0.1f, 0.1f, 0.2f, 0.2f}, 1, false};
    MinimapRegion c{"id_2", "A", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — MinimapViewport
// ═════════════════════════════════════════════════════════════════

TEST_CASE("MinimapViewport default is invalid", "[Phase28][MinimapViewport]") {
    MinimapViewport vp;
    CHECK_FALSE(vp.isValid());
    CHECK_FALSE(vp.locked);
}

TEST_CASE("MinimapViewport valid with positive rect", "[Phase28][MinimapViewport]") {
    MinimapViewport vp{{0.0f, 0.0f, 0.5f, 0.5f}, false};
    CHECK(vp.isValid());
    CHECK_FALSE(vp.locked);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — MinimapManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("MinimapManager empty state", "[Phase28][MinimapManager]") {
    MinimapManager m;
    CHECK(m.empty());
    CHECK(m.regionCount() == 0);
}

TEST_CASE("MinimapManager addRegion", "[Phase28][MinimapManager]") {
    MinimapManager m;
    MinimapRegion r{"r1", "Region", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true};
    CHECK(m.addRegion(r));
    CHECK(m.isRegistered("r1"));
    CHECK(m.regionCount() == 1);
}

TEST_CASE("MinimapManager duplicate addRegion fails", "[Phase28][MinimapManager]") {
    MinimapManager m;
    MinimapRegion r{"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true};
    CHECK(m.addRegion(r));
    CHECK_FALSE(m.addRegion(r));
}

TEST_CASE("MinimapManager invalid region rejected", "[Phase28][MinimapManager]") {
    MinimapManager m;
    MinimapRegion r; // invalid
    CHECK_FALSE(m.addRegion(r));
}

TEST_CASE("MinimapManager removeRegion", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    CHECK(m.removeRegion("r1"));
    CHECK(m.empty());
}

TEST_CASE("MinimapManager removeRegion unknown fails", "[Phase28][MinimapManager]") {
    MinimapManager m;
    CHECK_FALSE(m.removeRegion("nope"));
}

TEST_CASE("MinimapManager updateRegion", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.addRegion({"r1", "Old", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    MinimapRegion updated{"r1", "New", {0.1f, 0.1f, 0.4f, 0.4f}, 0xFF, true};
    CHECK(m.updateRegion(updated));
    CHECK(m.findRegion("r1")->label == "New");
}

TEST_CASE("MinimapManager updateRegion unknown fails", "[Phase28][MinimapManager]") {
    MinimapManager m;
    MinimapRegion r{"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true};
    CHECK_FALSE(m.updateRegion(r));
}

TEST_CASE("MinimapManager setVisible", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    CHECK(m.setVisible("r1", false));
    CHECK_FALSE(m.findRegion("r1")->visible);
    CHECK(m.setVisible("r1", true));
    CHECK(m.findRegion("r1")->visible);
}

TEST_CASE("MinimapManager visibleRegions filters by visibility", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.addRegion({"r1", "R1", {0.0f, 0.0f, 0.3f, 0.3f}, 0, true});
    m.addRegion({"r2", "R2", {0.3f, 0.0f, 0.3f, 0.3f}, 0, true});
    m.addRegion({"r3", "R3", {0.6f, 0.0f, 0.3f, 0.3f}, 0, true});
    m.setVisible("r2", false);
    CHECK(m.visibleRegions().size() == 2);
}

TEST_CASE("MinimapManager setViewport", "[Phase28][MinimapManager]") {
    MinimapManager m;
    MinimapViewport vp{{0.0f, 0.0f, 0.25f, 0.25f}, false};
    CHECK(m.setViewport(vp));
    CHECK(m.viewport().rect == vp.rect);
}

TEST_CASE("MinimapManager setViewport invalid fails", "[Phase28][MinimapManager]") {
    MinimapManager m;
    MinimapViewport vp;  // invalid
    CHECK_FALSE(m.setViewport(vp));
}

TEST_CASE("MinimapManager scrollViewport", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.setViewport({{0.0f, 0.0f, 0.25f, 0.25f}, false});
    CHECK(m.scrollViewport(0.1f, 0.1f));
    CHECK_THAT(m.viewport().rect.x, WithinAbs(0.1f, 1e-5f));
    CHECK_THAT(m.viewport().rect.y, WithinAbs(0.1f, 1e-5f));
}

TEST_CASE("MinimapManager scrollViewport clamped at bounds", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.setViewport({{0.5f, 0.5f, 0.25f, 0.25f}, false});
    m.scrollViewport(1.0f, 1.0f); // should clamp to 0.75
    CHECK_THAT(m.viewport().rect.x, WithinAbs(0.75f, 1e-5f));
    CHECK_THAT(m.viewport().rect.y, WithinAbs(0.75f, 1e-5f));
}

TEST_CASE("MinimapManager scrollViewport locked fails", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.setViewport({{0.0f, 0.0f, 0.25f, 0.25f}, false});
    m.lockViewport();
    CHECK_FALSE(m.scrollViewport(0.1f, 0.1f));
}

TEST_CASE("MinimapManager lockViewport / unlockViewport", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.setViewport({{0.0f, 0.0f, 0.25f, 0.25f}, false});
    m.lockViewport();
    CHECK(m.viewport().locked);
    m.unlockViewport();
    CHECK_FALSE(m.viewport().locked);
}

TEST_CASE("MinimapManager region observer fires on add", "[Phase28][MinimapManager]") {
    MinimapManager m;
    std::string lastId; bool lastAdded = false;
    m.addRegionObserver([&](const MinimapRegion& r, bool added) { lastId = r.id; lastAdded = added; });
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    CHECK(lastId    == "r1");
    CHECK(lastAdded == true);
}

TEST_CASE("MinimapManager region observer fires on remove", "[Phase28][MinimapManager]") {
    MinimapManager m;
    bool lastAdded = true;
    m.addRegionObserver([&](const MinimapRegion&, bool added) { lastAdded = added; });
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    m.removeRegion("r1");
    CHECK_FALSE(lastAdded);
}

TEST_CASE("MinimapManager viewport observer fires on setViewport", "[Phase28][MinimapManager]") {
    MinimapManager m;
    int calls = 0;
    m.addViewportObserver([&](const MinimapViewport&) { ++calls; });
    m.setViewport({{0.0f, 0.0f, 0.25f, 0.25f}, false});
    CHECK(calls == 1);
}

TEST_CASE("MinimapManager viewport observer fires on scroll", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.setViewport({{0.0f, 0.0f, 0.25f, 0.25f}, false});
    int calls = 0;
    m.addViewportObserver([&](const MinimapViewport&) { ++calls; });
    m.scrollViewport(0.1f, 0.0f);
    CHECK(calls == 1);
}

TEST_CASE("MinimapManager removeObserver stops notifications", "[Phase28][MinimapManager]") {
    MinimapManager m;
    int calls = 0;
    uint32_t id = m.addRegionObserver([&](const MinimapRegion&, bool) { ++calls; });
    m.removeObserver(id);
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    CHECK(calls == 0);
}

TEST_CASE("MinimapManager clear empties regions", "[Phase28][MinimapManager]") {
    MinimapManager m;
    m.addRegion({"r1", "R1", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    m.addRegion({"r2", "R2", {0.5f, 0.0f, 0.5f, 0.5f}, 0, true});
    m.clear();
    CHECK(m.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full minimap pipeline: add regions, set viewport, scroll, observe", "[Phase28][Integration]") {
    MinimapManager m;

    std::vector<std::pair<std::string,bool>> regionEvents;
    int viewportEvents = 0;

    m.addRegionObserver([&](const MinimapRegion& r, bool added) {
        regionEvents.push_back({r.id, added});
    });
    m.addViewportObserver([&](const MinimapViewport&) { ++viewportEvents; });

    m.addRegion({"scene",  "Scene",  {0.0f, 0.0f, 0.5f, 1.0f}, 0xFFFF0000, true});
    m.addRegion({"ui",     "UI",     {0.5f, 0.0f, 0.5f, 1.0f}, 0xFF00FF00, true});
    m.setViewport({{0.0f, 0.0f, 0.3f, 0.3f}, false});
    m.scrollViewport(0.2f, 0.0f);

    CHECK(regionEvents.size()  == 2);
    CHECK(viewportEvents       == 2); // setViewport + scroll
    CHECK(m.regionCount()      == 2);
    CHECK_THAT(m.viewport().rect.x, WithinAbs(0.2f, 1e-5f));
}

TEST_CASE("Visible region filter hides invisible", "[Phase28][Integration]") {
    MinimapManager m;
    m.addRegion({"r1", "A", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    m.addRegion({"r2", "B", {0.5f, 0.0f, 0.5f, 0.5f}, 0, true});
    m.addRegion({"r3", "C", {0.0f, 0.5f, 0.5f, 0.5f}, 0, true});
    m.setVisible("r2", false);
    CHECK(m.visibleRegions().size() == 2);
    m.setVisible("r2", true);
    CHECK(m.visibleRegions().size() == 3);
}

TEST_CASE("clearObservers stops all notifications", "[Phase28][Integration]") {
    MinimapManager m;
    int calls = 0;
    m.addRegionObserver([&](const MinimapRegion&, bool) { ++calls; });
    m.addViewportObserver([&](const MinimapViewport&) { ++calls; });
    m.clearObservers();
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    m.setViewport({{0.0f, 0.0f, 0.25f, 0.25f}, false});
    CHECK(calls == 0);
}

TEST_CASE("Multiple observers all fire", "[Phase28][Integration]") {
    MinimapManager m;
    int c1 = 0, c2 = 0;
    m.addRegionObserver([&](const MinimapRegion&, bool) { ++c1; });
    m.addRegionObserver([&](const MinimapRegion&, bool) { ++c2; });
    m.addRegion({"r1", "R", {0.0f, 0.0f, 0.5f, 0.5f}, 0, true});
    CHECK(c1 == 1);
    CHECK(c2 == 1);
}
