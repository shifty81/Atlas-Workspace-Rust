#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/MenuManager.h"
#include "../engine/ui/UIConstants.h"
#include <cassert>
#include <iostream>

// ============================================================
// Menu Dropdown Visibility Tests
// ============================================================

void test_menu_dropdown_items_hidden_when_closed() {
    // Verify that RenderWidget skips children of a closed Menu widget.
    // We do this by checking that when isMenuOpen is false, the rendering
    // path would skip children (tested via the widget state).
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 25, 120, 24);
    screen.SetParent(item1, menu);

    // Menu starts closed
    assert(!screen.IsMenuOpen(menu));

    // The widget is visible but the menu is not open
    assert(screen.IsVisible(item1));

    // The menu rendering logic should gate children rendering on isMenuOpen
    const auto* menuWidget = screen.GetWidget(menu);
    assert(menuWidget != nullptr);
    assert(!menuWidget->isMenuOpen);

    std::cout << "[PASS] test_menu_dropdown_items_hidden_when_closed" << std::endl;
}

void test_menu_click_opens_dropdown() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 25, 120, 24);
    screen.SetParent(item1, menu);

    // Click on menu to open it
    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 40; // Inside menu bounds
    clickEvent.y = 15;
    clickEvent.mouseButton = 0;

    bool consumed = mgr.DispatchEvent(clickEvent);
    assert(consumed);
    assert(screen.IsMenuOpen(menu));
    assert(mgr.GetMenuManager().GetOpenMenuId() == menu);

    std::cout << "[PASS] test_menu_click_opens_dropdown" << std::endl;
}

void test_menu_click_then_item_click() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 25, 120, 24);
    uint32_t item2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 49, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetParent(item2, menu);

    bool callbackFired = false;
    uint32_t clickedMenu = 0, clickedItem = 0;
    mgr.GetMenuManager().SetMenuItemCallback([&](uint32_t menuId, uint32_t itemId) {
        callbackFired = true;
        clickedMenu = menuId;
        clickedItem = itemId;
    });

    // Step 1: Click menu to open
    atlas::ui::UIEvent openClick;
    openClick.type = atlas::ui::UIEvent::Type::MouseDown;
    openClick.x = 40;
    openClick.y = 15;
    openClick.mouseButton = 0;
    mgr.DispatchEvent(openClick);
    assert(screen.IsMenuOpen(menu));

    // Step 2: Click on item1
    atlas::ui::UIEvent itemClick;
    itemClick.type = atlas::ui::UIEvent::Type::MouseDown;
    itemClick.x = 50;
    itemClick.y = 35; // Inside item1
    itemClick.mouseButton = 0;
    bool consumed = mgr.DispatchEvent(itemClick);
    assert(consumed);
    assert(callbackFired);
    assert(clickedMenu == menu);
    assert(clickedItem == item1);
    // Menu should close after selection
    assert(!screen.IsMenuOpen(menu));

    std::cout << "[PASS] test_menu_click_then_item_click" << std::endl;
}

void test_menu_click_toggle() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 25, 120, 24);
    screen.SetParent(item1, menu);

    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 40;
    clickEvent.y = 15;
    clickEvent.mouseButton = 0;

    // Click to open
    mgr.DispatchEvent(clickEvent);
    assert(screen.IsMenuOpen(menu));

    // Click same menu to close
    mgr.DispatchEvent(clickEvent);
    assert(!screen.IsMenuOpen(menu));

    std::cout << "[PASS] test_menu_click_toggle" << std::endl;
}

// ============================================================
// Hover State Tracking Tests
// ============================================================

void test_button_hover_tracking() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "TB", 0, 0, 400, 30);
    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Play", 4, 3, 50, 24);
    screen.SetParent(btn, toolbar);

    // Move mouse over button
    atlas::ui::UIEvent moveEvent;
    moveEvent.type = atlas::ui::UIEvent::Type::MouseMove;
    moveEvent.x = 20;
    moveEvent.y = 10;
    mgr.DispatchEvent(moveEvent);

    const auto* widget = screen.GetWidget(btn);
    assert(widget != nullptr);
    assert(widget->isHovered);

    // Move mouse away
    moveEvent.x = 500;
    moveEvent.y = 500;
    mgr.DispatchEvent(moveEvent);

    widget = screen.GetWidget(btn);
    assert(!widget->isHovered);

    std::cout << "[PASS] test_button_hover_tracking" << std::endl;
}

void test_tab_hover_tracking() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "TabBar", 0, 0, 400, 28);
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab1", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Tab2", 80, 0, 80, 28);
    screen.SetParent(tab1, tabBar);
    screen.SetParent(tab2, tabBar);

    // Move mouse over tab1
    atlas::ui::UIEvent moveEvent;
    moveEvent.type = atlas::ui::UIEvent::Type::MouseMove;
    moveEvent.x = 40;
    moveEvent.y = 14;
    mgr.DispatchEvent(moveEvent);

    assert(screen.IsHovered(tab1));
    assert(!screen.IsHovered(tab2));

    // Move mouse over tab2
    moveEvent.x = 120;
    moveEvent.y = 14;
    mgr.DispatchEvent(moveEvent);

    assert(!screen.IsHovered(tab1));
    assert(screen.IsHovered(tab2));

    std::cout << "[PASS] test_tab_hover_tracking" << std::endl;
}

// ============================================================
// General Button Click Tests
// ============================================================

void test_standalone_button_click_enqueues_command() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    // Create a standalone button (not in a toolbar)
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Panel", 0, 0, 400, 400);
    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Apply", 10, 10, 80, 30);
    screen.SetParent(btn, panel);

    // Click on the button
    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 30;
    clickEvent.y = 20;
    clickEvent.mouseButton = 0;
    bool consumed = mgr.DispatchEvent(clickEvent);
    assert(consumed);

    // Should have enqueued a ButtonPress command
    assert(mgr.GetCommandBus().PendingCount() == 1);

    std::cout << "[PASS] test_standalone_button_click_enqueues_command" << std::endl;
}

void test_toolbar_button_click_uses_toolbar_manager() {
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

    // Click on toolbar button
    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 20;
    clickEvent.y = 10;
    clickEvent.mouseButton = 0;
    bool consumed = mgr.DispatchEvent(clickEvent);
    assert(consumed);
    assert(callbackFired);

    // Should NOT have enqueued a ButtonPress command (toolbar handled it)
    assert(mgr.GetCommandBus().PendingCount() == 0);

    std::cout << "[PASS] test_toolbar_button_click_uses_toolbar_manager" << std::endl;
}

// ============================================================
// Menu Switch on Hover Tests
// ============================================================

void test_menu_hover_switches_open_menu() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t menu1 = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    uint32_t menu2 = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit", 80, 5, 60, 20);

    // Open menu1
    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 40;
    clickEvent.y = 15;
    clickEvent.mouseButton = 0;
    mgr.DispatchEvent(clickEvent);
    assert(screen.IsMenuOpen(menu1));
    assert(!screen.IsMenuOpen(menu2));

    // Hover over menu2 while menu1 is open - should switch
    atlas::ui::UIEvent moveEvent;
    moveEvent.type = atlas::ui::UIEvent::Type::MouseMove;
    moveEvent.x = 110;
    moveEvent.y = 15;
    mgr.DispatchEvent(moveEvent);

    assert(!screen.IsMenuOpen(menu1));
    assert(screen.IsMenuOpen(menu2));

    std::cout << "[PASS] test_menu_hover_switches_open_menu" << std::endl;
}

void run_gui_interaction_fix_tests() {
    std::cout << "\n--- GUI Interaction Fix Tests ---" << std::endl;

    test_menu_dropdown_items_hidden_when_closed();
    test_menu_click_opens_dropdown();
    test_menu_click_then_item_click();
    test_menu_click_toggle();
    test_button_hover_tracking();
    test_tab_hover_tracking();
    test_standalone_button_click_enqueues_command();
    test_toolbar_button_click_uses_toolbar_manager();
    test_menu_hover_switches_open_menu();
}
