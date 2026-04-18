#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/MenuManager.h"
#include <cassert>
#include <iostream>

// Test menu creation and rendering
void test_menu_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    // Create a menu bar
    uint32_t menuBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "MenuBar", 0, 0, 800, 30);

    // Create File menu
    uint32_t fileMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    screen.SetParent(fileMenu, menuBar);

    // Add menu items
    uint32_t fileNew = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 30, 120, 24);
    screen.SetParent(fileNew, fileMenu);

    uint32_t fileOpen = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 54, 120, 24);
    screen.SetParent(fileOpen, fileMenu);

    uint32_t fileSep = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "", 10, 78, 120, 8);
    screen.SetParent(fileSep, fileMenu);
    screen.SetSeparator(fileSep, true);

    uint32_t fileExit = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Exit", 10, 86, 120, 24);
    screen.SetParent(fileExit, fileMenu);

    // Verify widgets were created with correct count
    assert(screen.WidgetCount() == 6);

    // Verify menu widget properties
    const auto* menu = screen.GetWidget(fileMenu);
    assert(menu != nullptr);
    assert(menu->type == atlas::ui::UIWidgetType::Menu);
    assert(menu->name == "File");
    assert(menu->x == 10.0f);
    assert(menu->y == 5.0f);
    assert(menu->width == 60.0f);
    assert(menu->height == 20.0f);
    assert(menu->parentId == menuBar);

    // Verify separator properties
    const auto* separator = screen.GetWidget(fileSep);
    assert(separator != nullptr);
    assert(separator->isSeparator);
    assert(separator->parentId == fileMenu);
    assert(separator->height == 8.0f);

    // Verify menu item properties
    const auto* newItem = screen.GetWidget(fileNew);
    assert(newItem != nullptr);
    assert(newItem->type == atlas::ui::UIWidgetType::MenuItem);
    assert(newItem->name == "New");
    assert(newItem->parentId == fileMenu);
    assert(newItem->width == 120.0f);
    assert(newItem->height == 24.0f);

    // Verify hierarchy
    auto children = screen.GetChildren(fileMenu);
    assert(children.size() == 4); // New, Open, separator, Exit
    assert(children[0] == fileNew);
    assert(children[1] == fileOpen);
    assert(children[2] == fileSep);
    assert(children[3] == fileExit);
}

// Test menu state management
void test_menu_state() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);

    // Test menu open/close
    assert(!screen.IsMenuOpen(menu));
    screen.SetMenuOpen(menu, true);
    assert(screen.IsMenuOpen(menu));
    screen.SetMenuOpen(menu, false);
    assert(!screen.IsMenuOpen(menu));

    // Test hover state
    assert(!screen.IsHovered(menu));
    screen.SetHovered(menu, true);
    assert(screen.IsHovered(menu));
}

// Test MenuManager interaction
void test_menu_manager() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    // Create menu
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 30, 120, 24);
    screen.SetParent(item1, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Test click detection
    bool callbackInvoked = false;
    uint32_t clickedMenu = 0;
    uint32_t clickedItem = 0;

    menuMgr.SetMenuItemCallback([&](uint32_t menuId, uint32_t itemId) {
        callbackInvoked = true;
        clickedMenu = menuId;
        clickedItem = itemId;
    });

    // Simulate clicking on menu to open it
    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 40; // Inside menu bounds
    clickEvent.y = 15;
    clickEvent.mouseButton = 0;

    bool consumed = menuMgr.HandleEvent(clickEvent);
    assert(consumed);
    assert(screen.IsMenuOpen(menu));

    // Simulate clicking on menu item
    clickEvent.x = 50; // Inside item bounds
    clickEvent.y = 40;
    consumed = menuMgr.HandleEvent(clickEvent);
    assert(consumed);
    assert(callbackInvoked);
    assert(clickedMenu == menu);
    assert(clickedItem == item1);
    assert(!screen.IsMenuOpen(menu)); // Menu should close after selection
}

// Test hover behavior
void test_menu_hover() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu1 = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t menu2 = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit", 80, 10, 60, 20);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu1
    screen.SetMenuOpen(menu1, true);

    // Hover over menu2 - should switch to it
    atlas::ui::UIEvent moveEvent;
    moveEvent.type = atlas::ui::UIEvent::Type::MouseMove;
    moveEvent.x = 110; // Inside menu2
    moveEvent.y = 15;

    menuMgr.HandleEvent(moveEvent);

    // After hovering, menu2 should be highlighted
    // (Implementation opens menu2 and closes menu1 on hover when another menu is open)
}

// Test UIManager integration
void test_ui_manager_integration() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = uiMgr.GetScreen();
    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);

    // Test that MenuManager is accessible
    auto& menuMgr = uiMgr.GetMenuManager();
    bool callbackSet = false;
    menuMgr.SetMenuItemCallback([&](uint32_t, uint32_t) {
        callbackSet = true;
    });

    // Test event dispatch
    atlas::ui::UIEvent event;
    event.type = atlas::ui::UIEvent::Type::MouseDown;
    event.x = 40;
    event.y = 15;
    event.mouseButton = 0;

    bool consumed = uiMgr.DispatchEvent(event);
    // Event should be consumed by menu system
    assert(consumed);
}
