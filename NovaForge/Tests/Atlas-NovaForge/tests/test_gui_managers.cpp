#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/TabManager.h"
#include "../engine/ui/ScrollManager.h"
#include "../engine/ui/ToolbarManager.h"
#include "../engine/ui/UIEventRouter.h"
#include <cassert>
#include <iostream>

// ============================================================
// TabManager Tests
// ============================================================

void test_tab_manager_activate() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);
    screen.SetChecked(tab1, true);

    atlas::ui::TabManager mgr;
    mgr.Init(&screen);

    // Activate tab2
    bool changed = mgr.ActivateTab(tab2);
    assert(changed);
    assert(screen.IsChecked(tab2));
    assert(!screen.IsChecked(tab1));

    std::cout << "[PASS] test_tab_manager_activate" << std::endl;
}

void test_tab_manager_no_change_same_tab() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetChecked(tab1, true);

    atlas::ui::TabManager mgr;
    mgr.Init(&screen);

    // Activating already-active tab should return false
    bool changed = mgr.ActivateTab(tab1);
    assert(!changed);

    std::cout << "[PASS] test_tab_manager_no_change_same_tab" << std::endl;
}

void test_tab_manager_get_active() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);
    screen.SetChecked(tab1, true);

    atlas::ui::TabManager mgr;
    mgr.Init(&screen);

    assert(mgr.GetActiveTab(tabBar) == tab1);
    mgr.ActivateTab(tab2);
    assert(mgr.GetActiveTab(tabBar) == tab2);

    std::cout << "[PASS] test_tab_manager_get_active" << std::endl;
}

void test_tab_manager_content_visibility() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);
    screen.SetChecked(tab1, true);

    uint32_t panel1 = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Panel1", 0, 30, 400, 300);
    uint32_t panel2 = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Panel2", 0, 30, 400, 300);
    screen.SetVisible(panel2, false);

    atlas::ui::TabManager mgr;
    mgr.Init(&screen);
    mgr.SetTabContent(tab1, panel1);
    mgr.SetTabContent(tab2, panel2);

    // Switch to tab2 — panel2 visible, panel1 hidden
    mgr.ActivateTab(tab2);
    assert(screen.IsVisible(panel2));
    assert(!screen.IsVisible(panel1));

    // Switch back to tab1 — panel1 visible, panel2 hidden
    mgr.ActivateTab(tab1);
    assert(screen.IsVisible(panel1));
    assert(!screen.IsVisible(panel2));

    std::cout << "[PASS] test_tab_manager_content_visibility" << std::endl;
}

void test_tab_manager_callback() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);
    screen.SetChecked(tab1, true);

    atlas::ui::TabManager mgr;
    mgr.Init(&screen);

    uint32_t cbGroup = 0, cbNew = 0, cbOld = 0;
    mgr.SetTabChangedCallback([&](uint32_t g, uint32_t n, uint32_t o) {
        cbGroup = g; cbNew = n; cbOld = o;
    });

    mgr.ActivateTab(tab2);
    assert(cbGroup == tabBar);
    assert(cbNew == tab2);
    assert(cbOld == tab1);

    std::cout << "[PASS] test_tab_manager_callback" << std::endl;
}

void test_tab_manager_handle_click() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);
    screen.SetChecked(tab1, true);

    atlas::ui::TabManager mgr;
    mgr.Init(&screen);

    // Click on tab2
    bool consumed = mgr.HandleClick(100, 14);
    assert(consumed);
    assert(screen.IsChecked(tab2));
    assert(!screen.IsChecked(tab1));

    // Click outside any tab
    bool missed = mgr.HandleClick(500, 14);
    assert(!missed);

    std::cout << "[PASS] test_tab_manager_handle_click" << std::endl;
}

// ============================================================
// ScrollManager Tests
// ============================================================

void test_scroll_manager_register() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 0, 0, 200, 300);

    atlas::ui::ScrollManager mgr;
    mgr.Init(&screen);
    mgr.RegisterScrollView(sv, 600.0f);

    auto* state = mgr.GetScrollState(sv);
    assert(state != nullptr);
    assert(state->GetContentHeight() == 600.0f);
    assert(state->GetViewportHeight() == 300.0f);
    assert(state->IsScrollable());

    std::cout << "[PASS] test_scroll_manager_register" << std::endl;
}

void test_scroll_manager_scroll_wheel() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 10, 10, 200, 300);

    atlas::ui::ScrollManager mgr;
    mgr.Init(&screen);
    mgr.RegisterScrollView(sv, 600.0f);

    // Scroll inside the ScrollView
    bool consumed = mgr.HandleScrollWheel(50, 50, 3.0f);
    assert(consumed);

    auto* state = mgr.GetScrollState(sv);
    assert(state != nullptr);
    assert(state->GetOffset() > 0.0f);

    std::cout << "[PASS] test_scroll_manager_scroll_wheel" << std::endl;
}

void test_scroll_manager_scroll_outside() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 10, 10, 200, 300);

    atlas::ui::ScrollManager mgr;
    mgr.Init(&screen);
    mgr.RegisterScrollView(sv, 600.0f);

    // Scroll outside the ScrollView
    bool consumed = mgr.HandleScrollWheel(500, 500, 3.0f);
    assert(!consumed);

    std::cout << "[PASS] test_scroll_manager_scroll_outside" << std::endl;
}

void test_scroll_manager_not_scrollable() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    // Content height < viewport height = not scrollable
    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 10, 10, 200, 300);

    atlas::ui::ScrollManager mgr;
    mgr.Init(&screen);
    mgr.RegisterScrollView(sv, 100.0f);

    bool consumed = mgr.HandleScrollWheel(50, 50, 3.0f);
    assert(!consumed); // Not scrollable, so not consumed

    std::cout << "[PASS] test_scroll_manager_not_scrollable" << std::endl;
}

void test_scroll_manager_to_top_bottom() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 0, 0, 200, 300);

    atlas::ui::ScrollManager mgr;
    mgr.Init(&screen);
    mgr.RegisterScrollView(sv, 600.0f);

    mgr.ScrollToBottom(sv);
    auto* state = mgr.GetScrollState(sv);
    assert(state->GetOffset() == state->MaxOffset());

    mgr.ScrollToTop(sv);
    assert(state->GetOffset() == 0.0f);

    std::cout << "[PASS] test_scroll_manager_to_top_bottom" << std::endl;
}

void test_scroll_manager_set_content_height() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 0, 0, 200, 300);

    atlas::ui::ScrollManager mgr;
    mgr.Init(&screen);
    mgr.RegisterScrollView(sv, 600.0f);

    mgr.SetContentHeight(sv, 1000.0f);
    auto* state = mgr.GetScrollState(sv);
    assert(state->GetContentHeight() == 1000.0f);

    std::cout << "[PASS] test_scroll_manager_set_content_height" << std::endl;
}

// ============================================================
// ToolbarManager Tests
// ============================================================

void test_toolbar_manager_click() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "TB", 0, 0, 400, 30);
    uint32_t btn1 = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Play", 4, 3, 50, 24);
    uint32_t btn2 = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Stop", 58, 3, 50, 24);
    screen.SetParent(btn1, toolbar);
    screen.SetParent(btn2, toolbar);

    atlas::ui::ToolbarManager mgr;
    mgr.Init(&screen);

    uint32_t clickedToolbar = 0, clickedButton = 0;
    mgr.SetButtonCallback([&](uint32_t t, uint32_t b) {
        clickedToolbar = t; clickedButton = b;
    });

    // Click on btn1
    bool consumed = mgr.HandleClick(20, 10);
    assert(consumed);
    assert(clickedToolbar == toolbar);
    assert(clickedButton == btn1);

    std::cout << "[PASS] test_toolbar_manager_click" << std::endl;
}

void test_toolbar_manager_toggle() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "TB", 0, 0, 400, 30);
    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Grid", 4, 3, 50, 24);
    screen.SetParent(btn, toolbar);

    atlas::ui::ToolbarManager mgr;
    mgr.Init(&screen);
    mgr.SetToggleButton(btn, true);
    assert(mgr.IsToggleButton(btn));

    // Click toggles the button
    mgr.HandleClick(20, 10);
    assert(screen.IsChecked(btn));

    // Click again un-toggles
    mgr.HandleClick(20, 10);
    assert(!screen.IsChecked(btn));

    std::cout << "[PASS] test_toolbar_manager_toggle" << std::endl;
}

void test_toolbar_manager_miss() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "TB", 0, 0, 400, 30);
    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Play", 4, 3, 50, 24);
    screen.SetParent(btn, toolbar);

    atlas::ui::ToolbarManager mgr;
    mgr.Init(&screen);

    // Click outside toolbar buttons
    bool consumed = mgr.HandleClick(500, 500);
    assert(!consumed);

    std::cout << "[PASS] test_toolbar_manager_miss" << std::endl;
}

// ============================================================
// ScrollWheel Event Type Test
// ============================================================

void test_scroll_wheel_event_type() {
    atlas::ui::UIEvent event;
    event.type = atlas::ui::UIEvent::Type::ScrollWheel;
    event.scrollDelta = 3.0f;
    event.x = 100;
    event.y = 100;

    assert(event.type == atlas::ui::UIEvent::Type::ScrollWheel);
    assert(event.scrollDelta == 3.0f);

    std::cout << "[PASS] test_scroll_wheel_event_type" << std::endl;
}

// ============================================================
// UIManager Integration Tests
// ============================================================

void test_ui_manager_tab_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);
    screen.SetChecked(tab1, true);

    uint32_t panel1 = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "P1", 0, 30, 400, 300);
    uint32_t panel2 = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "P2", 0, 30, 400, 300);
    screen.SetVisible(panel2, false);

    mgr.GetTabManager().SetTabContent(tab1, panel1);
    mgr.GetTabManager().SetTabContent(tab2, panel2);

    // Dispatch a mouse click on tab2
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 100;
    click.y = 14;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(screen.IsChecked(tab2));
    assert(!screen.IsChecked(tab1));
    assert(screen.IsVisible(panel2));
    assert(!screen.IsVisible(panel1));

    std::cout << "[PASS] test_ui_manager_tab_integration" << std::endl;
}

void test_ui_manager_scroll_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SV", 10, 10, 200, 300);
    mgr.GetScrollManager().RegisterScrollView(sv, 600.0f);

    // Dispatch scroll wheel event
    atlas::ui::UIEvent scroll;
    scroll.type = atlas::ui::UIEvent::Type::ScrollWheel;
    scroll.x = 50;
    scroll.y = 50;
    scroll.scrollDelta = 2.0f;
    bool consumed = mgr.DispatchEvent(scroll);
    assert(consumed);

    auto* state = mgr.GetScrollManager().GetScrollState(sv);
    assert(state != nullptr);
    assert(state->GetOffset() > 0.0f);

    std::cout << "[PASS] test_ui_manager_scroll_integration" << std::endl;
}

void test_ui_manager_toolbar_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "TB", 0, 0, 400, 30);
    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Play", 4, 3, 50, 24);
    screen.SetParent(btn, toolbar);

    bool callbackFired = false;
    mgr.GetToolbarManager().SetButtonCallback([&](uint32_t, uint32_t) {
        callbackFired = true;
    });

    // Dispatch click on toolbar button
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 20;
    click.y = 10;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(callbackFired);

    std::cout << "[PASS] test_ui_manager_toolbar_integration" << std::endl;
}
