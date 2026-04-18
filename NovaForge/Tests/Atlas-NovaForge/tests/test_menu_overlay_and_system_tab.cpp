#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/UIRenderer.h"
#include "../engine/core/Logger.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

// --- Counting renderer that records DrawRect calls ---

struct DrawCall {
    atlas::ui::UIRect rect;
    std::string text;
    enum Kind { Rect, Text, Border, Icon, Image } kind;
};

class CountingRenderer : public atlas::ui::UIRenderer {
public:
    std::vector<DrawCall> calls;

    void BeginFrame() override {}
    void EndFrame() override {}

    void DrawRect(const atlas::ui::UIRect& rect, const atlas::ui::UIColor&) override {
        calls.push_back({rect, "", DrawCall::Rect});
    }
    void DrawText(const atlas::ui::UIRect& rect, const std::string& text, const atlas::ui::UIColor&) override {
        calls.push_back({rect, text, DrawCall::Text});
    }
    void DrawIcon(const atlas::ui::UIRect& rect, uint32_t, const atlas::ui::UIColor&) override {
        calls.push_back({rect, "", DrawCall::Icon});
    }
    void DrawBorder(const atlas::ui::UIRect& rect, int32_t, const atlas::ui::UIColor&) override {
        calls.push_back({rect, "", DrawCall::Border});
    }
    void DrawImage(const atlas::ui::UIRect& rect, uint32_t, const atlas::ui::UIColor&) override {
        calls.push_back({rect, "", DrawCall::Image});
    }
};

// Test that open menu dropdown items are rendered AFTER other widgets (overlay pass).
void test_menu_overlay_renders_on_top() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = uiMgr.GetScreen();

    // Menu bar
    uint32_t menuBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "MenuBar",
                                         0, 0, 800, 28);

    // File menu
    uint32_t fileMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File",
                                          4, 2, 50, 24);
    screen.SetParent(fileMenu, menuBar);

    // Menu items that drop down BELOW the menu bar
    uint32_t fileNew = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",
                                         4, 28, 120, 24);
    screen.SetParent(fileNew, fileMenu);

    uint32_t fileOpen = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open",
                                          4, 52, 120, 24);
    screen.SetParent(fileOpen, fileMenu);

    // Toolbar that overlaps with dropdown area
    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "Toolbar",
                                         0, 28, 800, 30);

    uint32_t tbPlay = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Play",
                                        4, 31, 50, 24);
    screen.SetParent(tbPlay, toolbar);

    // Open the File menu
    screen.SetMenuOpen(fileMenu, true);

    // Render and check ordering
    CountingRenderer renderer;
    uiMgr.Render(&renderer);

    // Find last draw call that contains the text "New" (menu item)
    // and last draw call that contains the text "Play" (toolbar button)
    int lastMenuItemIdx = -1;
    int lastToolbarBtnIdx = -1;
    for (size_t i = 0; i < renderer.calls.size(); ++i) {
        if (renderer.calls[i].text == "New") lastMenuItemIdx = static_cast<int>(i);
        if (renderer.calls[i].text == "Play") lastToolbarBtnIdx = static_cast<int>(i);
    }

    assert(lastMenuItemIdx > 0 && "Menu item 'New' should have been rendered");
    assert(lastToolbarBtnIdx > 0 && "Toolbar button 'Play' should have been rendered");

    // The menu item must be rendered AFTER the toolbar button (overlay pass)
    assert(lastMenuItemIdx > lastToolbarBtnIdx &&
           "Menu dropdown items must render on top of toolbar (after it in draw order)");

    std::cout << "[PASS] test_menu_overlay_renders_on_top" << std::endl;
}

// Test that closed menus do NOT produce an overlay pass.
void test_menu_overlay_not_rendered_when_closed() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = uiMgr.GetScreen();

    uint32_t menuBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "MenuBar",
                                         0, 0, 800, 28);
    uint32_t fileMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File",
                                          4, 2, 50, 24);
    screen.SetParent(fileMenu, menuBar);
    uint32_t fileNew = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",
                                         4, 28, 120, 24);
    screen.SetParent(fileNew, fileMenu);

    // Menu is closed
    assert(!screen.IsMenuOpen(fileMenu));

    CountingRenderer renderer;
    uiMgr.Render(&renderer);

    // "New" should NOT be rendered at all (menu is closed)
    bool foundNew = false;
    for (auto& call : renderer.calls) {
        if (call.text == "New") { foundNew = true; break; }
    }
    assert(!foundNew && "Closed menu should not render dropdown items");

    std::cout << "[PASS] test_menu_overlay_not_rendered_when_closed" << std::endl;
}

// Test that Logger sink callback is invoked.
void test_logger_sink_callback() {
    atlas::Logger::Init();

    std::vector<std::string> captured;
    atlas::Logger::SetSink([&captured](const std::string& line) {
        captured.push_back(line);
    });

    atlas::Logger::Info("test-sink-message");

    assert(!captured.empty());
    assert(captured.back().find("test-sink-message") != std::string::npos);

    // Clear the sink
    atlas::Logger::SetSink(nullptr);
    atlas::Logger::Shutdown();

    std::cout << "[PASS] test_logger_sink_callback" << std::endl;
}

// Test that System tab widgets are created and tab switching works.
void test_system_tab_creation() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = uiMgr.GetScreen();

    // Simulate the console tab structure from BuildEditorUI
    uint32_t bottomPanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ConsoleArea",
                                             262, 512, 756, 178);

    uint32_t consoleTabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ConsoleTabBar",
                                               262, 512, 756, 26);
    screen.SetParent(consoleTabBar, bottomPanel);

    uint32_t consoleTab = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Console",
                                            264, 514, 80, 22);
    screen.SetParent(consoleTab, consoleTabBar);
    screen.SetChecked(consoleTab, true);

    uint32_t systemTab = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "System",
                                           348, 514, 80, 22);
    screen.SetParent(systemTab, consoleTabBar);

    uint32_t consoleContent = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ConsoleContent",
                                                262, 538, 756, 152);
    uint32_t systemContent = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "SystemContent",
                                               262, 538, 756, 152);
    screen.SetVisible(systemContent, false);

    // Verify Console tab is checked, System tab is not
    assert(screen.IsChecked(consoleTab));
    assert(!screen.IsChecked(systemTab));

    // Verify Console content is visible, System content is hidden
    assert(screen.IsVisible(consoleContent));
    assert(!screen.IsVisible(systemContent));

    // Set up tab manager for switching
    auto& tabMgr = uiMgr.GetTabManager();
    tabMgr.SetTabContent(consoleTab, consoleContent);
    tabMgr.SetTabContent(systemTab, systemContent);

    // Simulate clicking the System tab
    atlas::ui::UIEvent clickEvent;
    clickEvent.type = atlas::ui::UIEvent::Type::MouseDown;
    clickEvent.x = 380;  // Inside System tab bounds
    clickEvent.y = 520;
    clickEvent.mouseButton = 0;

    uiMgr.DispatchEvent(clickEvent);

    // After clicking System tab, it should be checked and its content visible
    assert(screen.IsChecked(systemTab));
    assert(!screen.IsChecked(consoleTab));
    assert(screen.IsVisible(systemContent));
    assert(!screen.IsVisible(consoleContent));

    std::cout << "[PASS] test_system_tab_creation" << std::endl;
}
