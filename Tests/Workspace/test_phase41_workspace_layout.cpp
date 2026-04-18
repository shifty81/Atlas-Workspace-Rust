// Tests/Workspace/test_phase41_workspace_layout.cpp
// Phase 41 — Workspace Layout Manager
//
// Tests for:
//   1. LayoutPanelType    — enum name helpers
//   2. LayoutDockZone     — enum name helpers
//   3. LayoutPanel        — show/hide/pin/unpin/hasSize helpers
//   4. LayoutSplit        — isValid, flipOrientation
//   5. WorkspaceLayout    — addPanel/removePanel/findPanel/addSplit; visible/pinned counts
//   6. WorkspaceLayoutManager — createLayout/removeLayout/findLayout/setActive/activeLayout
//   7. Integration        — multi-layout workflow, active switch, panel visibility reset

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceLayout.h"
#include <string>

using namespace NF;

// ── Helper ────────────────────────────────────────────────────────

static LayoutPanel makePanel(const std::string& id, LayoutPanelType type = LayoutPanelType::Custom) {
    LayoutPanel p;
    p.id    = id;
    p.title = id + "_title";
    p.type  = type;
    p.width = 200.f;
    p.height= 400.f;
    return p;
}

// ─────────────────────────────────────────────────────────────────
// 1. LayoutPanelType
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LayoutPanelType – all values have names", "[phase41][LayoutPanelType]") {
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Viewport))       == "Viewport");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Inspector))      == "Inspector");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Hierarchy))      == "Hierarchy");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::ContentBrowser)) == "ContentBrowser");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Console))        == "Console");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Profiler))       == "Profiler");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Timeline))       == "Timeline");
    CHECK(std::string(layoutPanelTypeName(LayoutPanelType::Custom))         == "Custom");
}

// ─────────────────────────────────────────────────────────────────
// 2. LayoutDockZone
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LayoutDockZone – all values have names", "[phase41][LayoutDockZone]") {
    CHECK(std::string(layoutDockZoneName(LayoutDockZone::Left))   == "Left");
    CHECK(std::string(layoutDockZoneName(LayoutDockZone::Right))  == "Right");
    CHECK(std::string(layoutDockZoneName(LayoutDockZone::Top))    == "Top");
    CHECK(std::string(layoutDockZoneName(LayoutDockZone::Bottom)) == "Bottom");
}

// ─────────────────────────────────────────────────────────────────
// 3. LayoutPanel
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LayoutPanel – default visible and not pinned", "[phase41][LayoutPanel]") {
    LayoutPanel p;
    CHECK(p.isVisible());
    CHECK_FALSE(p.isPinned());
}

TEST_CASE("LayoutPanel – hide and show", "[phase41][LayoutPanel]") {
    LayoutPanel p;
    p.hide();
    CHECK_FALSE(p.isVisible());
    p.show();
    CHECK(p.isVisible());
}

TEST_CASE("LayoutPanel – pin and unpin", "[phase41][LayoutPanel]") {
    LayoutPanel p;
    p.pin();
    CHECK(p.isPinned());
    p.unpin();
    CHECK_FALSE(p.isPinned());
}

TEST_CASE("LayoutPanel – hasSize true when both dimensions set", "[phase41][LayoutPanel]") {
    LayoutPanel p;
    CHECK_FALSE(p.hasSize());    // defaults 0, 0
    p.width  = 100.f;
    p.height = 200.f;
    CHECK(p.hasSize());
}

TEST_CASE("LayoutPanel – hasSize false when one dimension is zero", "[phase41][LayoutPanel]") {
    LayoutPanel p;
    p.width = 100.f;
    CHECK_FALSE(p.hasSize());
}

// ─────────────────────────────────────────────────────────────────
// 4. LayoutSplit
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LayoutSplit – invalid without panel ids", "[phase41][LayoutSplit]") {
    LayoutSplit s;
    CHECK_FALSE(s.isValid());
}

TEST_CASE("LayoutSplit – invalid when ratio is 0 or 1", "[phase41][LayoutSplit]") {
    LayoutSplit s;
    s.firstPanelId  = "a";
    s.secondPanelId = "b";
    s.ratio = 0.f;
    CHECK_FALSE(s.isValid());
    s.ratio = 1.f;
    CHECK_FALSE(s.isValid());
}

TEST_CASE("LayoutSplit – valid with both ids and 0 < ratio < 1", "[phase41][LayoutSplit]") {
    LayoutSplit s;
    s.firstPanelId  = "left";
    s.secondPanelId = "right";
    s.ratio = 0.5f;
    CHECK(s.isValid());
}

TEST_CASE("LayoutSplit – flipOrientation toggles isHorizontal", "[phase41][LayoutSplit]") {
    LayoutSplit s;
    s.isHorizontal = true;
    s.flipOrientation();
    CHECK_FALSE(s.isHorizontal);
    s.flipOrientation();
    CHECK(s.isHorizontal);
}

// ─────────────────────────────────────────────────────────────────
// 5. WorkspaceLayout
// ─────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceLayout – constructed with name, initially empty", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("Default");
    CHECK(layout.name()        == "Default");
    CHECK(layout.panelCount()  == 0);
    CHECK(layout.splitCount()  == 0);
}

TEST_CASE("WorkspaceLayout – addPanel succeeds", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    CHECK(layout.addPanel(makePanel("viewport")));
    CHECK(layout.panelCount() == 1);
}

TEST_CASE("WorkspaceLayout – addPanel rejects duplicate id", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("viewport"));
    CHECK_FALSE(layout.addPanel(makePanel("viewport")));
    CHECK(layout.panelCount() == 1);
}

TEST_CASE("WorkspaceLayout – removePanel succeeds", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("inspector"));
    CHECK(layout.removePanel("inspector"));
    CHECK(layout.panelCount() == 0);
}

TEST_CASE("WorkspaceLayout – removePanel unknown returns false", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    CHECK_FALSE(layout.removePanel("ghost"));
}

TEST_CASE("WorkspaceLayout – findPanel returns pointer", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("console", LayoutPanelType::Console));
    LayoutPanel* p = layout.findPanel("console");
    REQUIRE(p != nullptr);
    CHECK(p->type == LayoutPanelType::Console);
}

TEST_CASE("WorkspaceLayout – findPanel returns nullptr for unknown", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    CHECK(layout.findPanel("nope") == nullptr);
}

TEST_CASE("WorkspaceLayout – findPanel mutation persists", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("panel1"));
    LayoutPanel* p = layout.findPanel("panel1");
    REQUIRE(p != nullptr);
    p->hide();
    CHECK_FALSE(layout.findPanel("panel1")->isVisible());
}

TEST_CASE("WorkspaceLayout – addSplit rejects invalid split", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    LayoutSplit s; // invalid: empty ids
    CHECK_FALSE(layout.addSplit(s));
    CHECK(layout.splitCount() == 0);
}

TEST_CASE("WorkspaceLayout – addSplit accepts valid split", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    LayoutSplit s;
    s.firstPanelId  = "left";
    s.secondPanelId = "right";
    s.ratio = 0.5f;
    CHECK(layout.addSplit(s));
    CHECK(layout.splitCount() == 1);
}

TEST_CASE("WorkspaceLayout – visiblePanelCount", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("a"));
    layout.addPanel(makePanel("b"));
    layout.findPanel("b")->hide();
    CHECK(layout.visiblePanelCount() == 1);
}

TEST_CASE("WorkspaceLayout – pinnedPanelCount", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("a"));
    layout.addPanel(makePanel("b"));
    layout.findPanel("a")->pin();
    CHECK(layout.pinnedPanelCount() == 1);
}

TEST_CASE("WorkspaceLayout – showAll and hideAll", "[phase41][WorkspaceLayout]") {
    WorkspaceLayout layout("L");
    layout.addPanel(makePanel("a"));
    layout.addPanel(makePanel("b"));
    layout.hideAll();
    CHECK(layout.visiblePanelCount() == 0);
    layout.showAll();
    CHECK(layout.visiblePanelCount() == 2);
}

// ─────────────────────────────────────────────────────────────────
// 6. WorkspaceLayoutManager
// ─────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceLayoutManager – default empty, no active", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    CHECK(mgr.layoutCount() == 0);
    CHECK_FALSE(mgr.hasActive());
    CHECK(mgr.activeName().empty());
}

TEST_CASE("WorkspaceLayoutManager – createLayout returns pointer", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    WorkspaceLayout* l = mgr.createLayout("Default");
    REQUIRE(l != nullptr);
    CHECK(l->name() == "Default");
    CHECK(mgr.layoutCount() == 1);
}

TEST_CASE("WorkspaceLayoutManager – createLayout rejects duplicate name", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Default");
    CHECK(mgr.createLayout("Default") == nullptr);
    CHECK(mgr.layoutCount() == 1);
}

TEST_CASE("WorkspaceLayoutManager – removeLayout succeeds", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Game");
    CHECK(mgr.removeLayout("Game"));
    CHECK(mgr.layoutCount() == 0);
}

TEST_CASE("WorkspaceLayoutManager – removeLayout unknown returns false", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    CHECK_FALSE(mgr.removeLayout("Missing"));
}

TEST_CASE("WorkspaceLayoutManager – findLayout returns pointer", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Cinematic");
    CHECK(mgr.findLayout("Cinematic") != nullptr);
}

TEST_CASE("WorkspaceLayoutManager – findLayout returns nullptr for unknown", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    CHECK(mgr.findLayout("ghost") == nullptr);
}

TEST_CASE("WorkspaceLayoutManager – setActive succeeds for existing layout", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Debug");
    CHECK(mgr.setActive("Debug"));
    CHECK(mgr.hasActive());
    CHECK(mgr.activeName() == "Debug");
}

TEST_CASE("WorkspaceLayoutManager – setActive fails for unknown layout", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    CHECK_FALSE(mgr.setActive("Unknown"));
    CHECK_FALSE(mgr.hasActive());
}

TEST_CASE("WorkspaceLayoutManager – activeLayout returns correct pointer", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Production");
    mgr.setActive("Production");
    WorkspaceLayout* active = mgr.activeLayout();
    REQUIRE(active != nullptr);
    CHECK(active->name() == "Production");
}

TEST_CASE("WorkspaceLayoutManager – activeLayout returns nullptr when no active", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Production");
    CHECK(mgr.activeLayout() == nullptr);
}

TEST_CASE("WorkspaceLayoutManager – removing active layout clears active name", "[phase41][WorkspaceLayoutManager]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Temp");
    mgr.setActive("Temp");
    mgr.removeLayout("Temp");
    CHECK_FALSE(mgr.hasActive());
}

// ─────────────────────────────────────────────────────────────────
// 7. Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceLayout integration – build full layout, switch active", "[phase41][integration]") {
    WorkspaceLayoutManager mgr;

    // Create two layouts
    mgr.createLayout("Game");
    mgr.createLayout("Cinematic");

    // Populate game layout (fetch pointer after all creates to avoid realloc invalidation)
    WorkspaceLayout* game = mgr.findLayout("Game");
    WorkspaceLayout* cinematic = mgr.findLayout("Cinematic");
    REQUIRE(game != nullptr);
    REQUIRE(cinematic != nullptr);

    // Populate game layout
    game->addPanel(makePanel("viewport",  LayoutPanelType::Viewport));
    game->addPanel(makePanel("inspector", LayoutPanelType::Inspector));
    game->addPanel(makePanel("hierarchy", LayoutPanelType::Hierarchy));

    // Populate cinematic layout
    cinematic->addPanel(makePanel("viewport",  LayoutPanelType::Viewport));
    cinematic->addPanel(makePanel("timeline",  LayoutPanelType::Timeline));

    // Pin viewport in game layout
    game->findPanel("viewport")->pin();

    CHECK(game->panelCount()      == 3);
    CHECK(game->pinnedPanelCount() == 1);
    CHECK(cinematic->panelCount() == 2);

    // Switch active
    mgr.setActive("Game");
    CHECK(mgr.activeName() == "Game");
    mgr.setActive("Cinematic");
    CHECK(mgr.activeName() == "Cinematic");

    // Active layout is cinematic
    WorkspaceLayout* active = mgr.activeLayout();
    REQUIRE(active != nullptr);
    CHECK(active->panelCount() == 2);
}

TEST_CASE("WorkspaceLayout integration – hide all, show all, count panels", "[phase41][integration]") {
    WorkspaceLayoutManager mgr;
    WorkspaceLayout* layout = mgr.createLayout("UI");
    layout->addPanel(makePanel("a"));
    layout->addPanel(makePanel("b"));
    layout->addPanel(makePanel("c"));

    layout->hideAll();
    CHECK(layout->visiblePanelCount() == 0);

    layout->showAll();
    CHECK(layout->visiblePanelCount() == 3);

    // Hide one manually
    layout->findPanel("b")->hide();
    CHECK(layout->visiblePanelCount() == 2);
}
