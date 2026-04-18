// S169 editor tests: WorkspaceLayoutManagerV1, WorkspaceDockZoneV1, WorkspaceViewportManagerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WorkspaceLayoutManagerV1.h"
#include "NF/Editor/WorkspaceDockZoneV1.h"
#include "NF/Editor/WorkspaceViewportManagerV1.h"

using namespace NF;
using Catch::Approx;

// ── WorkspaceLayoutManagerV1 ─────────────────────────────────────────────────

TEST_CASE("Wlmv1LayoutPreset validity", "[Editor][S169]") {
    Wlmv1LayoutPreset p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "DefaultLayout";
    REQUIRE(p.isValid());
}

TEST_CASE("WorkspaceLayoutManagerV1 addPreset and presetCount", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    REQUIRE(wlm.presetCount() == 0);
    Wlmv1LayoutPreset p; p.id = 1; p.name = "P1";
    REQUIRE(wlm.addPreset(p));
    REQUIRE(wlm.presetCount() == 1);
}

TEST_CASE("WorkspaceLayoutManagerV1 addPreset invalid fails", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    REQUIRE(!wlm.addPreset(Wlmv1LayoutPreset{}));
}

TEST_CASE("WorkspaceLayoutManagerV1 addPreset duplicate fails", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A";
    wlm.addPreset(p);
    REQUIRE(!wlm.addPreset(p));
}

TEST_CASE("WorkspaceLayoutManagerV1 removePreset", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 2; p.name = "B";
    wlm.addPreset(p);
    REQUIRE(wlm.removePreset(2));
    REQUIRE(wlm.presetCount() == 0);
    REQUIRE(!wlm.removePreset(2));
}

TEST_CASE("WorkspaceLayoutManagerV1 activate sets activeId", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A";
    wlm.addPreset(p);
    REQUIRE(wlm.activate(1));
    REQUIRE(wlm.activeId() == 1);
    REQUIRE(wlm.findPreset(1)->isActive());
}

TEST_CASE("WorkspaceLayoutManagerV1 activate locked fails", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A"; p.state = Wlmv1LayoutState::Locked;
    wlm.addPreset(p);
    REQUIRE(!wlm.activate(1));
}

TEST_CASE("WorkspaceLayoutManagerV1 activate unknown fails", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    REQUIRE(!wlm.activate(99));
}

TEST_CASE("WorkspaceLayoutManagerV1 activate deactivates previous", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p1; p1.id = 1; p1.name = "A";
    Wlmv1LayoutPreset p2; p2.id = 2; p2.name = "B";
    wlm.addPreset(p1); wlm.addPreset(p2);
    wlm.activate(1);
    wlm.activate(2);
    REQUIRE(wlm.activeId() == 2);
    REQUIRE(wlm.findPreset(1)->state == Wlmv1LayoutState::Saved);
}

TEST_CASE("WorkspaceLayoutManagerV1 setDefault defaultCount", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A";
    wlm.addPreset(p);
    REQUIRE(wlm.setDefault(1, true));
    REQUIRE(wlm.defaultCount() == 1);
}

TEST_CASE("WorkspaceLayoutManagerV1 lockedCount", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p1; p1.id = 1; p1.name = "A"; p1.state = Wlmv1LayoutState::Locked;
    Wlmv1LayoutPreset p2; p2.id = 2; p2.name = "B";
    wlm.addPreset(p1); wlm.addPreset(p2);
    REQUIRE(wlm.lockedCount() == 1);
}

TEST_CASE("WorkspaceLayoutManagerV1 countByMode", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p1; p1.id = 1; p1.name = "A"; p1.mode = Wlmv1LayoutMode::Split2H;
    Wlmv1LayoutPreset p2; p2.id = 2; p2.name = "B"; p2.mode = Wlmv1LayoutMode::Grid4;
    wlm.addPreset(p1); wlm.addPreset(p2);
    REQUIRE(wlm.countByMode(Wlmv1LayoutMode::Split2H) == 1);
    REQUIRE(wlm.countByMode(Wlmv1LayoutMode::Grid4)   == 1);
}

TEST_CASE("WorkspaceLayoutManagerV1 addSlot and removeSlot", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A";
    wlm.addPreset(p);
    Wlmv1PanelSlot s; s.panelId = "inspector"; s.weight = 0.3f;
    REQUIRE(wlm.addSlot(1, s));
    REQUIRE(wlm.findPreset(1)->slots.size() == 1);
    REQUIRE(wlm.removeSlot(1, "inspector"));
    REQUIRE(wlm.findPreset(1)->slots.empty());
}

TEST_CASE("Wlmv1PanelSlot invalid when empty", "[Editor][S169]") {
    Wlmv1PanelSlot s;
    REQUIRE(!s.isValid());
    s.panelId = "outliner"; s.weight = 0.25f;
    REQUIRE(s.isValid());
}

TEST_CASE("Wlmv1PanelSlot duplicate panelId fails", "[Editor][S169]") {
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A";
    Wlmv1PanelSlot s; s.panelId = "panel1"; s.weight = 0.5f;
    p.addSlot(s);
    REQUIRE(!p.addSlot(s));
}

TEST_CASE("wlmv1LayoutModeName covers all values", "[Editor][S169]") {
    REQUIRE(std::string(wlmv1LayoutModeName(Wlmv1LayoutMode::Single)) == "Single");
    REQUIRE(std::string(wlmv1LayoutModeName(Wlmv1LayoutMode::Custom)) == "Custom");
}

TEST_CASE("WorkspaceLayoutManagerV1 removePreset clears activeId", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    Wlmv1LayoutPreset p; p.id = 1; p.name = "A";
    wlm.addPreset(p);
    wlm.activate(1);
    wlm.removePreset(1);
    REQUIRE(wlm.activeId() == 0);
}

TEST_CASE("WorkspaceLayoutManagerV1 onChange callback", "[Editor][S169]") {
    WorkspaceLayoutManagerV1 wlm;
    uint64_t notified = 0;
    wlm.setOnChange([&](uint64_t id) { notified = id; });
    Wlmv1LayoutPreset p; p.id = 5; p.name = "X";
    wlm.addPreset(p);
    wlm.activate(5);
    REQUIRE(notified == 5);
}

// ── WorkspaceDockZoneV1 ──────────────────────────────────────────────────────

TEST_CASE("Wdzv1DockZone validity", "[Editor][S169]") {
    Wdzv1DockZone z;
    REQUIRE(!z.isValid());
    z.id = 1; z.name = "LeftPanel";
    REQUIRE(z.isValid());
}

TEST_CASE("WorkspaceDockZoneV1 addZone and zoneCount", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    REQUIRE(wdz.zoneCount() == 0);
    Wdzv1DockZone z; z.id = 1; z.name = "Z1";
    REQUIRE(wdz.addZone(z));
    REQUIRE(wdz.zoneCount() == 1);
}

TEST_CASE("WorkspaceDockZoneV1 addZone invalid fails", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    REQUIRE(!wdz.addZone(Wdzv1DockZone{}));
}

TEST_CASE("WorkspaceDockZoneV1 addZone duplicate fails", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z; z.id = 1; z.name = "A";
    wdz.addZone(z);
    REQUIRE(!wdz.addZone(z));
}

TEST_CASE("WorkspaceDockZoneV1 removeZone", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z; z.id = 2; z.name = "B";
    wdz.addZone(z);
    REQUIRE(wdz.removeZone(2));
    REQUIRE(wdz.zoneCount() == 0);
    REQUIRE(!wdz.removeZone(2));
}

TEST_CASE("WorkspaceDockZoneV1 setState visibleCount", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z1; z1.id = 1; z1.name = "A";
    Wdzv1DockZone z2; z2.id = 2; z2.name = "B";
    wdz.addZone(z1); wdz.addZone(z2);
    wdz.setState(2, Wdzv1ZoneState::Hidden);
    REQUIRE(wdz.visibleCount() == 1);
}

TEST_CASE("WorkspaceDockZoneV1 pinnedCount", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z; z.id = 1; z.name = "A";
    wdz.addZone(z);
    wdz.setState(1, Wdzv1ZoneState::Pinned);
    REQUIRE(wdz.pinnedCount() == 1);
    REQUIRE(wdz.findZone(1)->isPinned());
}

TEST_CASE("WorkspaceDockZoneV1 collapsedCount", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z; z.id = 1; z.name = "A";
    wdz.addZone(z);
    wdz.setState(1, Wdzv1ZoneState::Collapsed);
    REQUIRE(wdz.collapsedCount() == 1);
    REQUIRE(wdz.findZone(1)->isCollapsed());
}

TEST_CASE("WorkspaceDockZoneV1 setSizeRatio validation", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z; z.id = 1; z.name = "A";
    wdz.addZone(z);
    REQUIRE(!wdz.setSizeRatio(1, 0.f));
    REQUIRE(!wdz.setSizeRatio(1, 1.5f));
    REQUIRE(wdz.setSizeRatio(1, 0.3f));
    REQUIRE(wdz.findZone(1)->sizeRatio == Approx(0.3f));
}

TEST_CASE("WorkspaceDockZoneV1 addTab and removeTab", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z; z.id = 1; z.name = "A";
    wdz.addZone(z);
    Wdzv1TabEntry tab; tab.panelId = "inspector"; tab.title = "Inspector";
    REQUIRE(wdz.addTab(1, tab));
    REQUIRE(wdz.findZone(1)->tabs.size() == 1);
    REQUIRE(wdz.removeTab(1, "inspector"));
    REQUIRE(wdz.findZone(1)->tabs.empty());
}

TEST_CASE("Wdzv1TabEntry invalid when empty", "[Editor][S169]") {
    Wdzv1TabEntry t;
    REQUIRE(!t.isValid());
    t.panelId = "p"; t.title = "T";
    REQUIRE(t.isValid());
}

TEST_CASE("WorkspaceDockZoneV1 countBySide", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    Wdzv1DockZone z1; z1.id = 1; z1.name = "A"; z1.side = Wdzv1DockSide::Left;
    Wdzv1DockZone z2; z2.id = 2; z2.name = "B"; z2.side = Wdzv1DockSide::Right;
    wdz.addZone(z1); wdz.addZone(z2);
    REQUIRE(wdz.countBySide(Wdzv1DockSide::Left)  == 1);
    REQUIRE(wdz.countBySide(Wdzv1DockSide::Right) == 1);
}

TEST_CASE("Wdzv1DockZone dirtyTabCount", "[Editor][S169]") {
    Wdzv1DockZone z; z.id = 1; z.name = "A";
    Wdzv1TabEntry t1; t1.panelId = "a"; t1.title = "A"; t1.isDirty = true;
    Wdzv1TabEntry t2; t2.panelId = "b"; t2.title = "B"; t2.isDirty = false;
    z.addTab(t1); z.addTab(t2);
    REQUIRE(z.dirtyTabCount() == 1);
}

TEST_CASE("wdzv1DockSideName covers all values", "[Editor][S169]") {
    REQUIRE(std::string(wdzv1DockSideName(Wdzv1DockSide::Left))     == "Left");
    REQUIRE(std::string(wdzv1DockSideName(Wdzv1DockSide::Floating)) == "Floating");
}

TEST_CASE("WorkspaceDockZoneV1 onChange callback", "[Editor][S169]") {
    WorkspaceDockZoneV1 wdz;
    uint64_t notified = 0;
    wdz.setOnChange([&](uint64_t id) { notified = id; });
    Wdzv1DockZone z; z.id = 3; z.name = "C";
    wdz.addZone(z);
    wdz.setState(3, Wdzv1ZoneState::Pinned);
    REQUIRE(notified == 3);
}

// ── WorkspaceViewportManagerV1 ───────────────────────────────────────────────

TEST_CASE("Wvmv1Viewport validity", "[Editor][S169]") {
    Wvmv1Viewport v;
    REQUIRE(!v.isValid());
    v.id = 1; v.name = "MainViewport"; v.fov = 60.f;
    REQUIRE(v.isValid());
}

TEST_CASE("Wvmv1Viewport zero fov invalid", "[Editor][S169]") {
    Wvmv1Viewport v; v.id = 1; v.name = "X"; v.fov = 0.f;
    REQUIRE(!v.isValid());
}

TEST_CASE("WorkspaceViewportManagerV1 addViewport and viewportCount", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    REQUIRE(wvm.viewportCount() == 0);
    Wvmv1Viewport v; v.id = 1; v.name = "V1"; v.fov = 60.f;
    REQUIRE(wvm.addViewport(v));
    REQUIRE(wvm.viewportCount() == 1);
}

TEST_CASE("WorkspaceViewportManagerV1 addViewport invalid fails", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    REQUIRE(!wvm.addViewport(Wvmv1Viewport{}));
}

TEST_CASE("WorkspaceViewportManagerV1 addViewport duplicate fails", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    REQUIRE(!wvm.addViewport(v));
}

TEST_CASE("WorkspaceViewportManagerV1 removeViewport", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 2; v.name = "B"; v.fov = 60.f;
    wvm.addViewport(v);
    REQUIRE(wvm.removeViewport(2));
    REQUIRE(wvm.viewportCount() == 0);
    REQUIRE(!wvm.removeViewport(2));
}

TEST_CASE("WorkspaceViewportManagerV1 setFocus focusedId", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    REQUIRE(wvm.setFocus(1));
    REQUIRE(wvm.focusedId() == 1);
    REQUIRE(wvm.findViewport(1)->isFocused());
}

TEST_CASE("WorkspaceViewportManagerV1 setFocus locked fails", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f; v.state = Wvmv1ViewportState::Locked;
    wvm.addViewport(v);
    REQUIRE(!wvm.setFocus(1));
}

TEST_CASE("WorkspaceViewportManagerV1 setFocus deactivates previous", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v1; v1.id = 1; v1.name = "A"; v1.fov = 60.f;
    Wvmv1Viewport v2; v2.id = 2; v2.name = "B"; v2.fov = 60.f;
    wvm.addViewport(v1); wvm.addViewport(v2);
    wvm.setFocus(1);
    wvm.setFocus(2);
    REQUIRE(wvm.findViewport(1)->state == Wvmv1ViewportState::Idle);
    REQUIRE(wvm.focusedId() == 2);
}

TEST_CASE("WorkspaceViewportManagerV1 setState maximizedCount", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    wvm.setState(1, Wvmv1ViewportState::Maximized);
    REQUIRE(wvm.maximizedCount() == 1);
    REQUIRE(wvm.findViewport(1)->isMaximized());
}

TEST_CASE("WorkspaceViewportManagerV1 setShadingMode countByShading", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    wvm.setShadingMode(1, Wvmv1ShadingMode::Wireframe);
    REQUIRE(wvm.countByShading(Wvmv1ShadingMode::Wireframe) == 1);
}

TEST_CASE("WorkspaceViewportManagerV1 bindCamera", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    REQUIRE(wvm.bindCamera(1, "MainCamera"));
    REQUIRE(wvm.findViewport(1)->boundCamera == "MainCamera");
}

TEST_CASE("WorkspaceViewportManagerV1 setFOV validation", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    REQUIRE(!wvm.setFOV(1, 0.f));
    REQUIRE(wvm.setFOV(1, 90.f));
    REQUIRE(wvm.findViewport(1)->fov == Approx(90.f));
}

TEST_CASE("WorkspaceViewportManagerV1 countByType", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v1; v1.id = 1; v1.name = "A"; v1.fov = 60.f; v1.viewportType = Wvmv1ViewportType::Scene3D;
    Wvmv1Viewport v2; v2.id = 2; v2.name = "B"; v2.fov = 60.f; v2.viewportType = Wvmv1ViewportType::UV;
    wvm.addViewport(v1); wvm.addViewport(v2);
    REQUIRE(wvm.countByType(Wvmv1ViewportType::Scene3D) == 1);
    REQUIRE(wvm.countByType(Wvmv1ViewportType::UV)      == 1);
}

TEST_CASE("wvmv1ViewportTypeName covers all values", "[Editor][S169]") {
    REQUIRE(std::string(wvmv1ViewportTypeName(Wvmv1ViewportType::Scene3D)) == "Scene3D");
    REQUIRE(std::string(wvmv1ViewportTypeName(Wvmv1ViewportType::Custom))  == "Custom");
}

TEST_CASE("WorkspaceViewportManagerV1 removeViewport clears focusedId", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    Wvmv1Viewport v; v.id = 1; v.name = "A"; v.fov = 60.f;
    wvm.addViewport(v);
    wvm.setFocus(1);
    wvm.removeViewport(1);
    REQUIRE(wvm.focusedId() == 0);
}

TEST_CASE("WorkspaceViewportManagerV1 onChange callback", "[Editor][S169]") {
    WorkspaceViewportManagerV1 wvm;
    uint64_t notified = 0;
    wvm.setOnChange([&](uint64_t id) { notified = id; });
    Wvmv1Viewport v; v.id = 7; v.name = "G"; v.fov = 45.f;
    wvm.addViewport(v);
    wvm.setState(7, Wvmv1ViewportState::Maximized);
    REQUIRE(notified == 7);
}
