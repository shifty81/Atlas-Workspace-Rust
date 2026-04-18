#include "../engine/ui/UIScreenGraph.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_ui_add_widget() {
    UIScreen screen;
    screen.Init("MainMenu");

    assert(screen.GetName() == "MainMenu");

    uint32_t id = screen.AddWidget(UIWidgetType::Button, "play_btn", 100.0f, 200.0f, 120.0f, 40.0f);
    assert(id > 0);
    assert(screen.WidgetCount() == 1);

    const UIWidget* w = screen.GetWidget(id);
    assert(w != nullptr);
    assert(w->type == UIWidgetType::Button);
    assert(w->name == "play_btn");

    std::cout << "[PASS] test_ui_add_widget" << std::endl;
}

void test_ui_visibility() {
    UIScreen screen;
    screen.Init("HUD");

    uint32_t id = screen.AddWidget(UIWidgetType::Panel, "health_bar", 10.0f, 10.0f, 200.0f, 20.0f);
    assert(screen.IsVisible(id) == true);

    screen.SetVisible(id, false);
    assert(screen.IsVisible(id) == false);

    screen.SetVisible(id, true);
    assert(screen.IsVisible(id) == true);

    std::cout << "[PASS] test_ui_visibility" << std::endl;
}

void test_ui_parent_child() {
    UIScreen screen;
    screen.Init("Inventory");

    uint32_t panel = screen.AddWidget(UIWidgetType::Panel, "main_panel", 0.0f, 0.0f, 800.0f, 600.0f);
    uint32_t btn1 = screen.AddWidget(UIWidgetType::Button, "btn1", 10.0f, 10.0f, 80.0f, 30.0f);
    uint32_t btn2 = screen.AddWidget(UIWidgetType::Button, "btn2", 10.0f, 50.0f, 80.0f, 30.0f);

    screen.SetParent(btn1, panel);
    screen.SetParent(btn2, panel);

    auto children = screen.GetChildren(panel);
    assert(children.size() == 2);

    const UIWidget* w = screen.GetWidget(btn1);
    assert(w != nullptr);
    assert(w->parentId == panel);

    std::cout << "[PASS] test_ui_parent_child" << std::endl;
}

void test_ui_remove_widget() {
    UIScreen screen;
    screen.Init("Settings");

    uint32_t id = screen.AddWidget(UIWidgetType::Text, "title", 100.0f, 50.0f, 200.0f, 30.0f);
    assert(screen.WidgetCount() == 1);

    screen.RemoveWidget(id);
    assert(screen.WidgetCount() == 0);
    assert(screen.GetWidget(id) == nullptr);

    std::cout << "[PASS] test_ui_remove_widget" << std::endl;
}
