#include "../engine/core/Engine.h"
#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/MenuManager.h"
#include <cassert>
#include <iostream>

// Test that Engine::RequestExit() stops the engine
void test_engine_request_exit() {
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;
    cfg.headless = true;
    atlas::Engine engine(cfg);
    engine.InitCore();

    assert(engine.Running());
    engine.RequestExit();
    assert(!engine.Running());
    std::cout << "[PASS] test_engine_request_exit" << std::endl;
}

// Test that menu callback can toggle panel visibility
void test_menu_toggle_panel_visibility() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    // Create menu and panel
    uint32_t viewMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "View", 0, 0, 50, 24);
    uint32_t viewItem = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Panel", 0, 24, 120, 24);
    screen.SetParent(viewItem, viewMenu);
    screen.SetCheckable(viewItem, true);
    screen.SetChecked(viewItem, true);

    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "MyPanel", 0, 60, 200, 400);

    // Set up MenuManager with a callback that toggles panel visibility
    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    uint32_t panelId = panel;
    uint32_t viewItemId = viewItem;
    menuMgr.SetMenuItemCallback([&screen, panelId, viewItemId](uint32_t, uint32_t itemId) {
        if (itemId == viewItemId) {
            const atlas::ui::UIWidget* w = screen.GetWidget(panelId);
            if (w) {
                screen.SetVisible(panelId, !w->visible);
            }
        }
    });

    // Panel should be visible initially
    assert(screen.IsVisible(panel));

    // Open menu and click the item
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 25;
    clickMenu.y = 12;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);
    assert(screen.IsMenuOpen(viewMenu));

    // Click the menu item
    atlas::ui::UIEvent clickItem;
    clickItem.type = atlas::ui::UIEvent::Type::MouseDown;
    clickItem.x = 60;
    clickItem.y = 36;
    clickItem.mouseButton = 0;
    menuMgr.HandleEvent(clickItem);

    // Panel should now be hidden
    assert(!screen.IsVisible(panel));
    std::cout << "[PASS] test_menu_toggle_panel_visibility" << std::endl;
}

// Test that menu callback receives correct item IDs for different menus
void test_menu_action_item_dispatch() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t fileMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 0, 0, 50, 24);
    uint32_t fileSave = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Save", 0, 24, 120, 24);
    screen.SetParent(fileSave, fileMenu);
    uint32_t fileExit = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Exit", 0, 48, 120, 24);
    screen.SetParent(fileExit, fileMenu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    uint32_t lastClickedItem = 0;
    uint32_t saveId = fileSave;
    uint32_t exitId = fileExit;
    menuMgr.SetMenuItemCallback([&lastClickedItem](uint32_t, uint32_t itemId) {
        lastClickedItem = itemId;
    });

    // Open file menu
    atlas::ui::UIEvent openMenu;
    openMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    openMenu.x = 25;
    openMenu.y = 12;
    openMenu.mouseButton = 0;
    menuMgr.HandleEvent(openMenu);

    // Click Save
    atlas::ui::UIEvent clickSave;
    clickSave.type = atlas::ui::UIEvent::Type::MouseDown;
    clickSave.x = 60;
    clickSave.y = 36;
    clickSave.mouseButton = 0;
    menuMgr.HandleEvent(clickSave);
    assert(lastClickedItem == saveId);

    // Re-open menu and click Exit
    menuMgr.HandleEvent(openMenu);
    atlas::ui::UIEvent clickExit;
    clickExit.type = atlas::ui::UIEvent::Type::MouseDown;
    clickExit.x = 60;
    clickExit.y = 60;
    clickExit.mouseButton = 0;
    menuMgr.HandleEvent(clickExit);
    assert(lastClickedItem == exitId);
    std::cout << "[PASS] test_menu_action_item_dispatch" << std::endl;
}

// Test status bar text is updateable
void test_status_bar_update() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t statusBar = screen.AddWidget(atlas::ui::UIWidgetType::StatusBar, "Ready", 0, 0, 800, 28);

    const atlas::ui::UIWidget* w = screen.GetWidget(statusBar);
    assert(w != nullptr);
    assert(w->name == "Ready");

    // Simulate status update via mutable access
    atlas::ui::UIWidget* mw = screen.GetWidgetMutable(statusBar);
    assert(mw != nullptr);
    mw->name = "Project saved";

    w = screen.GetWidget(statusBar);
    assert(w->name == "Project saved");
    std::cout << "[PASS] test_status_bar_update" << std::endl;
}
