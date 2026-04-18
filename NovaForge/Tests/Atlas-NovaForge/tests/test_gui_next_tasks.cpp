#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/MenuManager.h"
#include <cassert>
#include <iostream>

// ============================================================
// Checkmark / Toggle Tests
// ============================================================

void test_checkable_default_false() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Show Grid", 10, 30, 120, 24);
    assert(!screen.IsCheckable(item));
    assert(!screen.IsChecked(item));
    std::cout << "[PASS] test_checkable_default_false" << std::endl;
}

void test_set_checkable() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Show Grid", 10, 30, 120, 24);
    screen.SetCheckable(item, true);
    assert(screen.IsCheckable(item));
    screen.SetCheckable(item, false);
    assert(!screen.IsCheckable(item));
    std::cout << "[PASS] test_set_checkable" << std::endl;
}

void test_set_checked() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Show Grid", 10, 30, 120, 24);
    screen.SetCheckable(item, true);
    screen.SetChecked(item, true);
    assert(screen.IsChecked(item));
    screen.SetChecked(item, false);
    assert(!screen.IsChecked(item));
    std::cout << "[PASS] test_set_checked" << std::endl;
}

void test_checkable_toggle_on_click() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "View", 10, 10, 60, 20);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Grid", 10, 30, 120, 24);
    screen.SetParent(item, menu);
    screen.SetCheckable(item, true);

    atlas::ui::MenuManager mgr;
    mgr.Init(&screen);
    bool callbackCalled = false;
    mgr.SetMenuItemCallback([&](uint32_t, uint32_t) { callbackCalled = true; });

    // Open menu
    atlas::ui::UIEvent openMenu;
    openMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    openMenu.x = 40; openMenu.y = 15; openMenu.mouseButton = 0;
    mgr.HandleEvent(openMenu);
    assert(screen.IsMenuOpen(menu));

    // Click checkable item
    atlas::ui::UIEvent clickItem;
    clickItem.type = atlas::ui::UIEvent::Type::MouseDown;
    clickItem.x = 50; clickItem.y = 38; clickItem.mouseButton = 0;
    mgr.HandleEvent(clickItem);
    assert(screen.IsChecked(item));
    assert(callbackCalled);

    std::cout << "[PASS] test_checkable_toggle_on_click" << std::endl;
}

void test_checkable_toggle_via_keyboard() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "View", 10, 10, 60, 20);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Grid", 10, 30, 120, 24);
    screen.SetParent(item, menu);
    screen.SetCheckable(item, true);

    atlas::ui::MenuManager mgr;
    mgr.Init(&screen);
    bool callbackCalled = false;
    mgr.SetMenuItemCallback([&](uint32_t, uint32_t) { callbackCalled = true; });

    // Open menu
    atlas::ui::UIEvent openMenu;
    openMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    openMenu.x = 40; openMenu.y = 15; openMenu.mouseButton = 0;
    mgr.HandleEvent(openMenu);

    // Navigate down then enter
    atlas::ui::UIEvent downKey;
    downKey.type = atlas::ui::UIEvent::Type::KeyDown;
    downKey.keyCode = 0x28; // Down
    mgr.HandleEvent(downKey);

    atlas::ui::UIEvent enterKey;
    enterKey.type = atlas::ui::UIEvent::Type::KeyDown;
    enterKey.keyCode = 0x0D; // Enter
    mgr.HandleEvent(enterKey);

    assert(screen.IsChecked(item));
    assert(callbackCalled);
    std::cout << "[PASS] test_checkable_toggle_via_keyboard" << std::endl;
}

// ============================================================
// Icon Tests
// ============================================================

void test_icon_id_default_zero() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 30, 120, 24);
    assert(screen.GetIconId(item) == 0);
    std::cout << "[PASS] test_icon_id_default_zero" << std::endl;
}

void test_set_icon_id() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 30, 120, 24);
    screen.SetIconId(item, 42);
    assert(screen.GetIconId(item) == 42);
    screen.SetIconId(item, 0);
    assert(screen.GetIconId(item) == 0);
    std::cout << "[PASS] test_set_icon_id" << std::endl;
}

void test_icon_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 0, 0, 50, 24);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 0, 24, 120, 24);
    screen.SetParent(item, menu);
    screen.SetMenuOpen(menu, true);
    screen.SetIconId(item, 5);

    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_icon_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Tooltip Widget Tests
// ============================================================

void test_tooltip_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Click to open file", 100, 200, 150, 24);
    const auto* w = screen.GetWidget(tip);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::Tooltip);
    assert(w->name == "Click to open file");
    std::cout << "[PASS] test_tooltip_widget_creation" << std::endl;
}

void test_tooltip_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Hint text", 50, 50, 100, 20);

    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_tooltip_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Tab Widget Tests
// ============================================================

void test_tab_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t tab = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Scene", 0, 0, 80, 28);
    const auto* w = screen.GetWidget(tab);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::Tab);
    assert(w->name == "Scene");
    std::cout << "[PASS] test_tab_widget_creation" << std::endl;
}

void test_tab_active_state() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t tab1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Scene", 0, 0, 80, 28);
    uint32_t tab2 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Game", 80, 0, 80, 28);
    // Use isChecked to indicate active tab
    screen.SetChecked(tab1, true);
    assert(screen.IsChecked(tab1));
    assert(!screen.IsChecked(tab2));
    std::cout << "[PASS] test_tab_active_state" << std::endl;
}

void test_tab_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t t1 = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Scene", 0, 0, 80, 28);
    screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Game", 80, 0, 80, 28);
    screen.SetChecked(t1, true);

    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_tab_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// ScrollView Widget Tests
// ============================================================

void test_scrollview_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sv = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "EntityScroll", 0, 0, 200, 400);
    const auto* w = screen.GetWidget(sv);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::ScrollView);
    std::cout << "[PASS] test_scrollview_widget_creation" << std::endl;
}

void test_scrollview_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "Scroll", 10, 10, 200, 300);

    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_scrollview_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// DockArea Widget Tests
// ============================================================

void test_dockarea_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "MainDock", 0, 30, 1280, 690);
    const auto* w = screen.GetWidget(dock);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::DockArea);
    std::cout << "[PASS] test_dockarea_widget_creation" << std::endl;
}

void test_dockarea_with_panels() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "MainDock", 0, 30, 1280, 690);
    uint32_t left = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Assets", 0, 30, 260, 690);
    uint32_t center = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Viewport", 260, 30, 760, 690);
    screen.SetParent(left, dock);
    screen.SetParent(center, dock);

    auto children = screen.GetChildren(dock);
    assert(children.size() == 2);
    std::cout << "[PASS] test_dockarea_with_panels" << std::endl;
}

void test_dockarea_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);
    uint32_t child = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Child", 0, 0, 200, 600);
    screen.SetParent(child, dock);

    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_dockarea_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Editor Integration Test
// ============================================================

void test_editor_checkable_view_items() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t viewMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "View", 0, 0, 50, 24);
    uint32_t assets = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Asset Browser", 0, 24, 140, 24);
    uint32_t inspector = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Inspector", 0, 48, 140, 24);
    screen.SetParent(assets, viewMenu);
    screen.SetParent(inspector, viewMenu);
    screen.SetCheckable(assets, true);
    screen.SetChecked(assets, true);
    screen.SetCheckable(inspector, true);
    screen.SetChecked(inspector, true);

    assert(screen.IsCheckable(assets));
    assert(screen.IsChecked(assets));
    assert(screen.IsCheckable(inspector));
    assert(screen.IsChecked(inspector));
    std::cout << "[PASS] test_editor_checkable_view_items" << std::endl;
}
