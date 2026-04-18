#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- LayoutPanelType names ---

TEST_CASE("LayoutPanelType names cover all 8 values", "[Editor][S22]") {
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Viewport))       == "Viewport");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Inspector))      == "Inspector");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Hierarchy))      == "Hierarchy");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::ContentBrowser)) == "ContentBrowser");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Console))        == "Console");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Profiler))       == "Profiler");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Timeline))       == "Timeline");
    REQUIRE(std::string(layoutPanelTypeName(LayoutPanelType::Custom))         == "Custom");
}

// --- LayoutDockZone names ---

TEST_CASE("LayoutDockZone names cover all 4 values", "[Editor][S22]") {
    REQUIRE(std::string(layoutDockZoneName(LayoutDockZone::Left))   == "Left");
    REQUIRE(std::string(layoutDockZoneName(LayoutDockZone::Right))  == "Right");
    REQUIRE(std::string(layoutDockZoneName(LayoutDockZone::Top))    == "Top");
    REQUIRE(std::string(layoutDockZoneName(LayoutDockZone::Bottom)) == "Bottom");
}

// --- LayoutPanel ---

TEST_CASE("LayoutPanel show/hide/pin/unpin", "[Editor][S22]") {
    LayoutPanel p;
    p.id = "vp"; p.title = "Viewport"; p.type = LayoutPanelType::Viewport;
    REQUIRE(p.isVisible());
    p.hide();
    REQUIRE_FALSE(p.isVisible());
    p.show();
    REQUIRE(p.isVisible());

    REQUIRE_FALSE(p.isPinned());
    p.pin();
    REQUIRE(p.isPinned());
    p.unpin();
    REQUIRE_FALSE(p.isPinned());
}

TEST_CASE("LayoutPanel hasSize checks both dimensions positive", "[Editor][S22]") {
    LayoutPanel p;
    REQUIRE_FALSE(p.hasSize());
    p.width = 400.f; p.height = 300.f;
    REQUIRE(p.hasSize());
    p.width = 0.f;
    REQUIRE_FALSE(p.hasSize());
}

// --- LayoutSplit ---

TEST_CASE("LayoutSplit isValid rejects empty ids and bad ratio", "[Editor][S22]") {
    LayoutSplit s;
    REQUIRE_FALSE(s.isValid());
    s.firstPanelId = "a"; s.secondPanelId = "b"; s.ratio = 0.5f;
    REQUIRE(s.isValid());
    s.ratio = 0.f;
    REQUIRE_FALSE(s.isValid());
    s.ratio = 1.f;
    REQUIRE_FALSE(s.isValid());
}

TEST_CASE("LayoutSplit flipOrientation toggles horizontal flag", "[Editor][S22]") {
    LayoutSplit s;
    REQUIRE(s.isHorizontal);
    s.flipOrientation();
    REQUIRE_FALSE(s.isHorizontal);
    s.flipOrientation();
    REQUIRE(s.isHorizontal);
}

// --- WorkspaceLayout ---

TEST_CASE("WorkspaceLayout addPanel + duplicate rejection", "[Editor][S22]") {
    WorkspaceLayout layout("Default");
    LayoutPanel p1; p1.id = "vp"; p1.title = "Viewport";
    LayoutPanel dup; dup.id = "vp";
    REQUIRE(layout.addPanel(p1));
    REQUIRE_FALSE(layout.addPanel(dup));
    REQUIRE(layout.panelCount() == 1);
}

TEST_CASE("WorkspaceLayout removePanel by id", "[Editor][S22]") {
    WorkspaceLayout layout("L");
    LayoutPanel p; p.id = "c"; p.title = "Console";
    layout.addPanel(p);
    REQUIRE(layout.removePanel("c"));
    REQUIRE(layout.panelCount() == 0);
    REQUIRE_FALSE(layout.removePanel("c")); // already removed
}

TEST_CASE("WorkspaceLayout visiblePanelCount and pinnedPanelCount", "[Editor][S22]") {
    WorkspaceLayout layout("L");
    LayoutPanel a; a.id = "a"; a.visible = true;  a.pinned = true;
    LayoutPanel b; b.id = "b"; b.visible = false; b.pinned = false;
    LayoutPanel c; c.id = "c"; c.visible = true;  c.pinned = false;
    layout.addPanel(a); layout.addPanel(b); layout.addPanel(c);
    REQUIRE(layout.visiblePanelCount() == 2);
    REQUIRE(layout.pinnedPanelCount()  == 1);
}

TEST_CASE("WorkspaceLayout showAll and hideAll", "[Editor][S22]") {
    WorkspaceLayout layout("L");
    LayoutPanel a; a.id = "a"; a.visible = false;
    LayoutPanel b; b.id = "b"; b.visible = false;
    layout.addPanel(a); layout.addPanel(b);
    layout.showAll();
    REQUIRE(layout.visiblePanelCount() == 2);
    layout.hideAll();
    REQUIRE(layout.visiblePanelCount() == 0);
}

TEST_CASE("WorkspaceLayout addSplit rejects invalid splits", "[Editor][S22]") {
    WorkspaceLayout layout("L");
    LayoutSplit bad; // empty ids
    REQUIRE_FALSE(layout.addSplit(bad));

    LayoutSplit good; good.firstPanelId = "a"; good.secondPanelId = "b"; good.ratio = 0.4f;
    REQUIRE(layout.addSplit(good));
    REQUIRE(layout.splitCount() == 1);
}

// --- WorkspaceLayoutManager ---

TEST_CASE("WorkspaceLayoutManager createLayout + duplicate rejection", "[Editor][S22]") {
    WorkspaceLayoutManager mgr;
    REQUIRE(mgr.createLayout("Default") != nullptr);
    REQUIRE(mgr.createLayout("Cinematic") != nullptr);
    REQUIRE(mgr.createLayout("Default") == nullptr); // duplicate
    REQUIRE(mgr.layoutCount() == 2);
}

TEST_CASE("WorkspaceLayoutManager setActive + hasActive + activeName", "[Editor][S22]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("Default");
    REQUIRE_FALSE(mgr.hasActive());
    REQUIRE(mgr.setActive("Default"));
    REQUIRE(mgr.hasActive());
    REQUIRE(mgr.activeName() == "Default");
    REQUIRE(mgr.activeLayout() != nullptr);
}

TEST_CASE("WorkspaceLayoutManager setActive rejects unknown layout", "[Editor][S22]") {
    WorkspaceLayoutManager mgr;
    REQUIRE_FALSE(mgr.setActive("Ghost"));
    REQUIRE_FALSE(mgr.hasActive());
}

TEST_CASE("WorkspaceLayoutManager removeLayout clears active if removed", "[Editor][S22]") {
    WorkspaceLayoutManager mgr;
    mgr.createLayout("A");
    mgr.setActive("A");
    REQUIRE(mgr.removeLayout("A"));
    REQUIRE_FALSE(mgr.hasActive());
    REQUIRE(mgr.layoutCount() == 0);
}
