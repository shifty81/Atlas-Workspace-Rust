// S146 editor tests: WidgetKitV1, TooltipSystemV1, TabBarV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── WidgetKitV1 ───────────────────────────────────────────────────────────────

TEST_CASE("WidgetType and WidgetState names", "[Editor][S146]") {
    REQUIRE(std::string(widgetTypeName(WidgetType::Button))     == "Button");
    REQUIRE(std::string(widgetTypeName(WidgetType::TextInput))  == "TextInput");
    REQUIRE(std::string(widgetTypeName(WidgetType::Slider))     == "Slider");
    REQUIRE(std::string(widgetTypeName(WidgetType::ProgressBar))== "ProgressBar");

    REQUIRE(std::string(widgetStateName(WidgetState::Normal))   == "Normal");
    REQUIRE(std::string(widgetStateName(WidgetState::Disabled)) == "Disabled");
    REQUIRE(std::string(widgetStateName(WidgetState::Pressed))  == "Pressed");
}

TEST_CASE("WidgetSizeConstraint clamp", "[Editor][S146]") {
    WidgetSizeConstraint c;
    c.minW = 10.f; c.maxW = 200.f;
    c.minH = 5.f;  c.maxH = 100.f;
    REQUIRE(c.clampW(5.f)   == Approx(10.f));
    REQUIRE(c.clampW(150.f) == Approx(150.f));
    REQUIRE(c.clampW(300.f) == Approx(200.f));
    REQUIRE(c.clampH(0.f)   == Approx(5.f));
    REQUIRE(c.clampH(200.f) == Approx(100.f));
}

TEST_CASE("WidgetDescriptor validity and enable/disable", "[Editor][S146]") {
    WidgetDescriptor w;
    REQUIRE(!w.isValid());
    w.id = 1; w.type = WidgetType::Button; w.label = "OK";
    REQUIRE(w.isValid());
    REQUIRE(!w.isDisabled());
    w.disable();
    REQUIRE(w.isDisabled());
    w.enable();
    REQUIRE(!w.isDisabled());
}

TEST_CASE("WidgetContainer addWidget and removeWidget", "[Editor][S146]") {
    WidgetContainer c;
    c.id = 1; c.layout = WidgetLayout::Horizontal;

    WidgetDescriptor w1; w1.id = 10; w1.label = "Save";
    WidgetDescriptor w2; w2.id = 20; w2.label = "Cancel";
    REQUIRE(c.addWidget(w1));
    REQUIRE(c.addWidget(w2));
    REQUIRE(c.widgetCount() == 2u);
    REQUIRE(!c.addWidget(w1));  // duplicate

    REQUIRE(c.findWidget(10) != nullptr);
    REQUIRE(c.findWidget(99) == nullptr);

    REQUIRE(c.removeWidget(10));
    REQUIRE(c.widgetCount() == 1u);
    REQUIRE(!c.removeWidget(99));
}

TEST_CASE("WidgetKitV1 addContainer and reject invalid", "[Editor][S146]") {
    WidgetKitV1 kit;
    WidgetContainer c; c.id = 1;
    REQUIRE(kit.addContainer(c));
    REQUIRE(kit.containerCount() == 1u);
    REQUIRE(!kit.addContainer(c));  // duplicate

    WidgetContainer bad;
    REQUIRE(!kit.addContainer(bad));  // id==0
}

TEST_CASE("WidgetKitV1 simulateClick fires callback", "[Editor][S146]") {
    WidgetKitV1 kit;
    WidgetContainer c; c.id = 1;
    WidgetDescriptor btn; btn.id = 10; btn.label = "Click me";
    bool clicked = false;
    btn.onClick = [&]() { clicked = true; };
    c.addWidget(btn);
    kit.addContainer(c);

    REQUIRE(kit.simulateClick(1, 10));
    REQUIRE(clicked);
    REQUIRE(kit.interactionCount() == 1u);
}

TEST_CASE("WidgetKitV1 simulateClick on disabled widget fails", "[Editor][S146]") {
    WidgetKitV1 kit;
    WidgetContainer c; c.id = 1;
    WidgetDescriptor btn; btn.id = 10; btn.label = "X"; btn.state = WidgetState::Disabled;
    c.addWidget(btn);
    kit.addContainer(c);
    REQUIRE(!kit.simulateClick(1, 10));
    REQUIRE(kit.interactionCount() == 0u);
}

TEST_CASE("WidgetKitV1 setValue fires onChange", "[Editor][S146]") {
    WidgetKitV1 kit;
    WidgetContainer c; c.id = 1;
    WidgetDescriptor input; input.id = 10; input.label = "Name"; input.value = "";
    std::string changed;
    input.onChange = [&](const std::string& v) { changed = v; };
    c.addWidget(input);
    kit.addContainer(c);

    REQUIRE(kit.setValue(1, 10, "Atlas"));
    REQUIRE(changed == "Atlas");
    REQUIRE(kit.findContainer(1)->findWidget(10)->value == "Atlas");
}

TEST_CASE("WidgetKitV1 setWidgetEnabled", "[Editor][S146]") {
    WidgetKitV1 kit;
    WidgetContainer c; c.id = 1;
    WidgetDescriptor w; w.id = 10; w.label = "X";
    c.addWidget(w);
    kit.addContainer(c);
    REQUIRE(kit.setWidgetEnabled(1, 10, false));
    REQUIRE(kit.findContainer(1)->findWidget(10)->isDisabled());
    REQUIRE(kit.setWidgetEnabled(1, 10, true));
    REQUIRE(!kit.findContainer(1)->findWidget(10)->isDisabled());
}

TEST_CASE("WidgetKitV1 removeContainer", "[Editor][S146]") {
    WidgetKitV1 kit;
    WidgetContainer c; c.id = 1;
    kit.addContainer(c);
    REQUIRE(kit.removeContainer(1));
    REQUIRE(kit.containerCount() == 0u);
    REQUIRE(!kit.removeContainer(99));
}

// ── TooltipSystemV1 ───────────────────────────────────────────────────────────

TEST_CASE("TooltipPosition names", "[Editor][S146]") {
    REQUIRE(std::string(tooltipPositionName(TooltipPosition::Auto))  == "Auto");
    REQUIRE(std::string(tooltipPositionName(TooltipPosition::Above)) == "Above");
    REQUIRE(std::string(tooltipPositionName(TooltipPosition::Below)) == "Below");
    REQUIRE(std::string(tooltipPositionName(TooltipPosition::Left))  == "Left");
    REQUIRE(std::string(tooltipPositionName(TooltipPosition::Right)) == "Right");
}

TEST_CASE("TooltipShowState names", "[Editor][S146]") {
    REQUIRE(std::string(tooltipShowStateName(TooltipShowState::Hidden))    == "Hidden");
    REQUIRE(std::string(tooltipShowStateName(TooltipShowState::Pending))   == "Pending");
    REQUIRE(std::string(tooltipShowStateName(TooltipShowState::Visible))   == "Visible");
    REQUIRE(std::string(tooltipShowStateName(TooltipShowState::Dismissed)) == "Dismissed");
}

TEST_CASE("TooltipContent isEmpty", "[Editor][S146]") {
    TooltipContent c;
    REQUIRE(c.isEmpty());
    c.title = "Save";
    REQUIRE(!c.isEmpty());
}

TEST_CASE("TooltipTarget isValid and resolveContent static", "[Editor][S146]") {
    TooltipTarget t;
    REQUIRE(!t.isValid());
    t.id = 1; t.ownerId = "save-button";
    t.staticContent.title = "Save file";
    t.staticContent.body  = "Ctrl+S";
    REQUIRE(t.isValid());
    REQUIRE(!t.hasDynamicContent());
    auto c = t.resolveContent();
    REQUIRE(c.title == "Save file");
}

TEST_CASE("TooltipTarget dynamic content provider", "[Editor][S146]") {
    TooltipTarget t;
    t.id = 1; t.ownerId = "btn";
    t.provider = []() -> TooltipContent {
        TooltipContent c; c.title = "Dynamic"; return c;
    };
    REQUIRE(t.hasDynamicContent());
    REQUIRE(t.resolveContent().title == "Dynamic");
}

TEST_CASE("TooltipSystemV1 registerTarget and unregister", "[Editor][S146]") {
    TooltipSystemV1 sys;
    TooltipTarget t; t.id = 1; t.ownerId = "btn";
    REQUIRE(sys.registerTarget(t));
    REQUIRE(sys.targetCount() == 1u);
    REQUIRE(!sys.registerTarget(t));  // duplicate

    REQUIRE(sys.unregisterTarget(1));
    REQUIRE(sys.targetCount() == 0u);
}

TEST_CASE("TooltipSystemV1 hover enter/exit and update", "[Editor][S146]") {
    TooltipSystemV1 sys;
    TooltipTarget t; t.id = 1; t.ownerId = "widget"; t.delayMs = 300.f;
    t.staticContent.title = "Hint";
    sys.registerTarget(t);

    sys.onHoverEnter(1);
    REQUIRE(sys.isPending());
    REQUIRE(!sys.isVisible());

    sys.update(100.f);
    REQUIRE(sys.isPending());  // not enough time

    sys.update(200.f);
    REQUIRE(sys.isVisible());
    REQUIRE(sys.showCount() == 1u);
    REQUIRE(sys.currentContent().title == "Hint");
}

TEST_CASE("TooltipSystemV1 hover exit cancels pending", "[Editor][S146]") {
    TooltipSystemV1 sys;
    TooltipTarget t; t.id = 1; t.ownerId = "w"; t.delayMs = 500.f;
    sys.registerTarget(t);
    sys.onHoverEnter(1);
    REQUIRE(sys.isPending());
    sys.onHoverExit(1);
    REQUIRE(sys.showState() == TooltipShowState::Hidden);
}

TEST_CASE("TooltipSystemV1 dismiss", "[Editor][S146]") {
    TooltipSystemV1 sys;
    TooltipTarget t; t.id = 1; t.ownerId = "w"; t.delayMs = 0.f;
    sys.registerTarget(t);
    sys.onHoverEnter(1);
    sys.update(1.f);
    REQUIRE(sys.isVisible());
    sys.dismiss();
    REQUIRE(sys.showState() == TooltipShowState::Dismissed);
}

TEST_CASE("TooltipSystemV1 setTargetEnabled", "[Editor][S146]") {
    TooltipSystemV1 sys;
    TooltipTarget t; t.id = 1; t.ownerId = "w"; t.delayMs = 100.f;
    sys.registerTarget(t);
    REQUIRE(sys.setTargetEnabled(1, false));
    sys.onHoverEnter(1);
    REQUIRE(sys.showState() == TooltipShowState::Hidden);  // disabled, so no hover effect
    REQUIRE(sys.setTargetEnabled(1, true));
    sys.onHoverEnter(1);
    REQUIRE(sys.isPending());
}

TEST_CASE("TooltipSystemV1 unregisterByOwner", "[Editor][S146]") {
    TooltipSystemV1 sys;
    TooltipTarget t1; t1.id = 1; t1.ownerId = "panel-A";
    TooltipTarget t2; t2.id = 2; t2.ownerId = "panel-A";
    TooltipTarget t3; t3.id = 3; t3.ownerId = "panel-B";
    sys.registerTarget(t1); sys.registerTarget(t2); sys.registerTarget(t3);
    REQUIRE(sys.unregisterByOwner("panel-A"));
    REQUIRE(sys.targetCount() == 1u);
    REQUIRE(!sys.unregisterByOwner("nonexistent"));
}

// ── TabBarV1 ──────────────────────────────────────────────────────────────────

TEST_CASE("TabState names", "[Editor][S146]") {
    REQUIRE(std::string(tabStateName(TabState::Normal))   == "Normal");
    REQUIRE(std::string(tabStateName(TabState::Modified)) == "Modified");
    REQUIRE(std::string(tabStateName(TabState::Loading))  == "Loading");
    REQUIRE(std::string(tabStateName(TabState::Error))    == "Error");
    REQUIRE(std::string(tabStateName(TabState::Disabled)) == "Disabled");
}

TEST_CASE("TabDescriptor isValid", "[Editor][S146]") {
    TabDescriptor t;
    REQUIRE(!t.isValid());
    t.id = 1; t.label = "Scene.atlas";
    REQUIRE(t.isValid());
    REQUIRE(!t.isModified());
    t.state = TabState::Modified;
    REQUIRE(t.isModified());
}

TEST_CASE("TabBarV1 addTab and active tracking", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t; t.id = 1; t.label = "Main.cpp";
    REQUIRE(bar.addTab(t));
    REQUIRE(bar.tabCount() == 1u);
    REQUIRE(bar.activeTabId() == 1u);
    REQUIRE(!bar.addTab(t));  // duplicate
}

TEST_CASE("TabBarV1 addTab invalid rejected", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor bad;
    REQUIRE(!bar.addTab(bad));
}

TEST_CASE("TabBarV1 activateTab fires callback", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A";
    TabDescriptor t2; t2.id = 2; t2.label = "B";
    bar.addTab(t1); bar.addTab(t2);

    uint32_t activated = 0;
    bar.setOnActivate([&](uint32_t id) { activated = id; });

    REQUIRE(bar.activateTab(2));
    REQUIRE(bar.activeTabId() == 2u);
    REQUIRE(activated == 2u);
    REQUIRE(bar.activationCount() == 1u);
}

TEST_CASE("TabBarV1 closeTab fires callback and can cancel", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A"; t1.closable = true;
    TabDescriptor t2; t2.id = 2; t2.label = "B"; t2.closable = true;
    bar.addTab(t1); bar.addTab(t2);

    bool allow = true;
    bar.setOnClose([&](uint32_t) -> bool { return allow; });

    allow = false;
    REQUIRE(!bar.closeTab(1));
    REQUIRE(bar.tabCount() == 2u);

    allow = true;
    REQUIRE(bar.closeTab(1));
    REQUIRE(bar.tabCount() == 1u);
}

TEST_CASE("TabBarV1 pinTab moves to front", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A";
    TabDescriptor t2; t2.id = 2; t2.label = "B";
    TabDescriptor t3; t3.id = 3; t3.label = "C";
    bar.addTab(t1); bar.addTab(t2); bar.addTab(t3);
    REQUIRE(bar.pinTab(3, true));
    REQUIRE(bar.tabs()[0].id == 3u);
    REQUIRE(bar.pinnedCount() == 1u);
}

TEST_CASE("TabBarV1 pinned tab cannot be closed normally", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t; t.id = 1; t.label = "Locked"; t.pinned = true; t.closable = true;
    bar.addTab(t);
    REQUIRE(!bar.removeTab(1));
    REQUIRE(bar.tabCount() == 1u);
}

TEST_CASE("TabBarV1 forceRemoveTab removes pinned", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t; t.id = 1; t.label = "Pinned"; t.pinned = true;
    bar.addTab(t);
    REQUIRE(bar.forceRemoveTab(1));
    REQUIRE(bar.tabCount() == 0u);
}

TEST_CASE("TabBarV1 setTabLabel", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t; t.id = 1; t.label = "Old";
    bar.addTab(t);
    REQUIRE(bar.setTabLabel(1, "New"));
    REQUIRE(bar.tabs()[0].label == "New");
    REQUIRE(!bar.setTabLabel(1, ""));  // empty rejected
}

TEST_CASE("TabBarV1 reorder tabs", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A";
    TabDescriptor t2; t2.id = 2; t2.label = "B";
    TabDescriptor t3; t3.id = 3; t3.label = "C";
    bar.addTab(t1); bar.addTab(t2); bar.addTab(t3);
    REQUIRE(bar.reorder(3, 0));
    REQUIRE(bar.tabs()[0].id == 3u);
}

TEST_CASE("TabBarV1 nextTab and prevTab", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A";
    TabDescriptor t2; t2.id = 2; t2.label = "B";
    TabDescriptor t3; t3.id = 3; t3.label = "C";
    bar.addTab(t1); bar.addTab(t2); bar.addTab(t3);
    bar.activateTab(1);
    REQUIRE(bar.nextTab() == 2u);
    bar.activateTab(2);
    REQUIRE(bar.nextTab() == 3u);
    REQUIRE(bar.prevTab() == 1u);
}

TEST_CASE("TabBarV1 modifiedCount", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A"; t1.state = TabState::Modified;
    TabDescriptor t2; t2.id = 2; t2.label = "B"; t2.state = TabState::Normal;
    bar.addTab(t1); bar.addTab(t2);
    REQUIRE(bar.modifiedCount() == 1u);
    REQUIRE(bar.setTabState(2, TabState::Modified));
    REQUIRE(bar.modifiedCount() == 2u);
}

TEST_CASE("TabBarV1 setTabVisible hides tab and reactivates", "[Editor][S146]") {
    TabBarV1 bar;
    TabDescriptor t1; t1.id = 1; t1.label = "A";
    TabDescriptor t2; t2.id = 2; t2.label = "B";
    bar.addTab(t1); bar.addTab(t2);
    bar.activateTab(1);
    REQUIRE(bar.setTabVisible(1, false));
    REQUIRE(bar.activeTabId() == 2u);
}

TEST_CASE("TabBarV1 triggerAdd callback", "[Editor][S146]") {
    TabBarV1 bar;
    bool addFired = false;
    bar.setOnAdd([&]() { addFired = true; });
    bar.triggerAdd();
    REQUIRE(addFired);
}
