#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/SliderManager.h"
#include "../engine/ui/ComboBoxManager.h"
#include <cassert>
#include <iostream>
#include <cmath>

// ============================================================
// Checkbox Widget Tests
// ============================================================

void test_checkbox_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Enable Vsync", 10, 10, 150, 20);
    const auto* w = screen.GetWidget(cb);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::Checkbox);
    assert(w->name == "Enable Vsync");
    assert(!w->isChecked);
    std::cout << "[PASS] test_checkbox_widget_creation" << std::endl;
}

void test_checkbox_toggle() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Grid", 10, 10, 150, 20);
    assert(!screen.IsChecked(cb));
    screen.SetChecked(cb, true);
    assert(screen.IsChecked(cb));
    screen.SetChecked(cb, false);
    assert(!screen.IsChecked(cb));
    std::cout << "[PASS] test_checkbox_toggle" << std::endl;
}

void test_checkbox_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Show Grid", 10, 10, 150, 20);
    screen.SetChecked(cb, true);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_checkbox_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Slider Widget Tests
// ============================================================

void test_slider_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Volume", 10, 10, 200, 24);
    const auto* w = screen.GetWidget(sl);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::Slider);
    assert(w->value == 0.0f);
    assert(w->minValue == 0.0f);
    assert(w->maxValue == 1.0f);
    std::cout << "[PASS] test_slider_widget_creation" << std::endl;
}

void test_slider_set_value() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Volume", 10, 10, 200, 24);
    screen.SetValue(sl, 0.5f);
    assert(std::fabs(screen.GetValue(sl) - 0.5f) < 0.001f);
    screen.SetValue(sl, 1.0f);
    assert(std::fabs(screen.GetValue(sl) - 1.0f) < 0.001f);
    std::cout << "[PASS] test_slider_set_value" << std::endl;
}

void test_slider_value_range() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Volume", 10, 10, 200, 24);
    screen.SetValueRange(sl, -10.0f, 10.0f);
    assert(std::fabs(screen.GetMinValue(sl) - (-10.0f)) < 0.001f);
    assert(std::fabs(screen.GetMaxValue(sl) - 10.0f) < 0.001f);
    std::cout << "[PASS] test_slider_value_range" << std::endl;
}

void test_slider_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Volume", 10, 10, 200, 24);
    screen.SetValue(sl, 0.75f);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_slider_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// ProgressBar Widget Tests
// ============================================================

void test_progressbar_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t pb = screen.AddWidget(atlas::ui::UIWidgetType::ProgressBar, "Loading...", 10, 10, 300, 20);
    const auto* w = screen.GetWidget(pb);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::ProgressBar);
    assert(w->value == 0.0f);
    std::cout << "[PASS] test_progressbar_widget_creation" << std::endl;
}

void test_progressbar_set_value() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t pb = screen.AddWidget(atlas::ui::UIWidgetType::ProgressBar, "Loading...", 10, 10, 300, 20);
    screen.SetValue(pb, 0.65f);
    assert(std::fabs(screen.GetValue(pb) - 0.65f) < 0.001f);
    std::cout << "[PASS] test_progressbar_set_value" << std::endl;
}

void test_progressbar_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t pb = screen.AddWidget(atlas::ui::UIWidgetType::ProgressBar, "Compiling...", 10, 10, 300, 20);
    screen.SetValue(pb, 0.33f);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_progressbar_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// ComboBox Widget Tests
// ============================================================

void test_combobox_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);
    const auto* w = screen.GetWidget(cb);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::ComboBox);
    assert(w->selectedIndex == -1);
    assert(!w->isOpen);
    std::cout << "[PASS] test_combobox_widget_creation" << std::endl;
}

void test_combobox_selected_index() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);
    screen.SetSelectedIndex(cb, 2);
    assert(screen.GetSelectedIndex(cb) == 2);
    screen.SetSelectedIndex(cb, -1);
    assert(screen.GetSelectedIndex(cb) == -1);
    std::cout << "[PASS] test_combobox_selected_index" << std::endl;
}

void test_combobox_open_state() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);
    assert(!screen.IsComboOpen(cb));
    screen.SetComboOpen(cb, true);
    assert(screen.IsComboOpen(cb));
    screen.SetComboOpen(cb, false);
    assert(!screen.IsComboOpen(cb));
    std::cout << "[PASS] test_combobox_open_state" << std::endl;
}

void test_combobox_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Resolution", 10, 10, 200, 24);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_combobox_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// TreeNode Widget Tests
// ============================================================

void test_treenode_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t tn = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "World", 10, 10, 200, 20);
    const auto* w = screen.GetWidget(tn);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::TreeNode);
    assert(!w->isExpanded);
    assert(w->treeDepth == 0);
    std::cout << "[PASS] test_treenode_widget_creation" << std::endl;
}

void test_treenode_expand_collapse() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t tn = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "World", 10, 10, 200, 20);
    assert(!screen.IsExpanded(tn));
    screen.SetExpanded(tn, true);
    assert(screen.IsExpanded(tn));
    screen.SetExpanded(tn, false);
    assert(!screen.IsExpanded(tn));
    std::cout << "[PASS] test_treenode_expand_collapse" << std::endl;
}

void test_treenode_depth() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t root = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "Root", 10, 10, 200, 20);
    uint32_t child = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "Child", 10, 30, 200, 20);
    uint32_t grandchild = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "GrandChild", 10, 50, 200, 20);
    screen.SetTreeDepth(root, 0);
    screen.SetTreeDepth(child, 1);
    screen.SetTreeDepth(grandchild, 2);
    assert(screen.GetTreeDepth(root) == 0);
    assert(screen.GetTreeDepth(child) == 1);
    assert(screen.GetTreeDepth(grandchild) == 2);
    std::cout << "[PASS] test_treenode_depth" << std::endl;
}

void test_treenode_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t tn = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "Entities", 10, 10, 200, 20);
    screen.SetExpanded(tn, true);
    screen.SetTreeDepth(tn, 1);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_treenode_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Splitter Widget Tests
// ============================================================

void test_splitter_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sp = screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "HSplit", 260, 30, 4, 690);
    const auto* w = screen.GetWidget(sp);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::Splitter);
    std::cout << "[PASS] test_splitter_widget_creation" << std::endl;
}

void test_splitter_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "Divider", 300, 0, 4, 600);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_splitter_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// ColorPicker Widget Tests
// ============================================================

void test_colorpicker_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);
    const auto* w = screen.GetWidget(cp);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::ColorPicker);
    assert(w->colorR == 255);
    assert(w->colorG == 255);
    assert(w->colorB == 255);
    assert(w->colorA == 255);
    std::cout << "[PASS] test_colorpicker_widget_creation" << std::endl;
}

void test_colorpicker_set_get_color() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);
    screen.SetColor(cp, 128, 64, 32, 200);
    uint8_t r, g, b, a;
    screen.GetColor(cp, r, g, b, a);
    assert(r == 128);
    assert(g == 64);
    assert(b == 32);
    assert(a == 200);
    std::cout << "[PASS] test_colorpicker_set_get_color" << std::endl;
}

void test_colorpicker_rendering_with_null_renderer() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);
    auto& screen = mgr.GetScreen();
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "BG Color", 10, 10, 24, 24);
    screen.SetColor(cp, 255, 0, 0, 255);
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer); // Should not crash
    std::cout << "[PASS] test_colorpicker_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// SliderManager Tests
// ============================================================

void test_slider_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    atlas::ui::SliderManager mgr;
    mgr.Init(&screen);
    assert(!mgr.IsDragging());
    assert(mgr.GetDraggingId() == 0);
    std::cout << "[PASS] test_slider_manager_init" << std::endl;
}

void test_slider_manager_click_sets_value() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Vol", 100, 10, 200, 24);
    screen.SetValue(sl, 0.0f);

    atlas::ui::SliderManager mgr;
    mgr.Init(&screen);

    bool callbackFired = false;
    float callbackValue = -1.0f;
    mgr.SetSliderChangedCallback([&](uint32_t id, float val) {
        callbackFired = true;
        callbackValue = val;
    });

    // Click at the middle of the slider (x=200 of a 100..300 range)
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 200;
    click.y = 18;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(mgr.IsDragging());
    assert(callbackFired);
    assert(std::fabs(callbackValue - 0.5f) < 0.01f);
    assert(std::fabs(screen.GetValue(sl) - 0.5f) < 0.01f);

    std::cout << "[PASS] test_slider_manager_click_sets_value" << std::endl;
}

void test_slider_manager_drag_updates_value() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Vol", 100, 10, 200, 24);

    atlas::ui::SliderManager mgr;
    mgr.Init(&screen);

    float lastValue = -1.0f;
    mgr.SetSliderChangedCallback([&](uint32_t, float val) { lastValue = val; });

    // Click to start drag at left edge
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 100;
    click.y = 18;
    mgr.HandleEvent(click);
    assert(mgr.IsDragging());

    // Drag to 75% (x=250)
    atlas::ui::UIEvent drag;
    drag.type = atlas::ui::UIEvent::Type::MouseMove;
    drag.x = 250;
    drag.y = 18;
    bool consumed = mgr.HandleEvent(drag);
    assert(consumed);
    assert(std::fabs(lastValue - 0.75f) < 0.01f);

    // Release
    atlas::ui::UIEvent release;
    release.type = atlas::ui::UIEvent::Type::MouseUp;
    release.x = 250;
    release.y = 18;
    mgr.HandleEvent(release);
    assert(!mgr.IsDragging());

    std::cout << "[PASS] test_slider_manager_drag_updates_value" << std::endl;
}

void test_slider_manager_clamps_value() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Vol", 100, 10, 200, 24);

    atlas::ui::SliderManager mgr;
    mgr.Init(&screen);

    // Click to start drag
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 150;
    click.y = 18;
    mgr.HandleEvent(click);

    // Drag past the right edge (x=400, past 300)
    atlas::ui::UIEvent drag;
    drag.type = atlas::ui::UIEvent::Type::MouseMove;
    drag.x = 400;
    drag.y = 18;
    mgr.HandleEvent(drag);
    assert(std::fabs(screen.GetValue(sl) - 1.0f) < 0.001f);

    // Drag past the left edge (x=0, before 100)
    drag.x = 0;
    mgr.HandleEvent(drag);
    assert(std::fabs(screen.GetValue(sl)) < 0.001f);

    std::cout << "[PASS] test_slider_manager_clamps_value" << std::endl;
}

void test_slider_manager_click_outside_ignored() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Vol", 100, 10, 200, 24);

    atlas::ui::SliderManager mgr;
    mgr.Init(&screen);

    // Click outside the slider
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 500;
    click.y = 500;
    bool consumed = mgr.HandleEvent(click);
    assert(!consumed);
    assert(!mgr.IsDragging());

    std::cout << "[PASS] test_slider_manager_click_outside_ignored" << std::endl;
}

// ============================================================
// ComboBoxManager Tests
// ============================================================

void test_combobox_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);
    assert(mgr.GetOpenComboId() == 0);
    std::cout << "[PASS] test_combobox_manager_init" << std::endl;
}

void test_combobox_manager_set_items() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);

    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);
    mgr.SetItems(cb, {"640x480", "1280x720", "1920x1080"});

    auto& items = mgr.GetItems(cb);
    assert(items.size() == 3);
    assert(items[0] == "640x480");
    assert(items[2] == "1920x1080");

    std::cout << "[PASS] test_combobox_manager_set_items" << std::endl;
}

void test_combobox_manager_click_opens() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);

    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);
    mgr.SetItems(cb, {"A", "B", "C"});

    // Click on the combo box
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 50;
    click.y = 18;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(mgr.GetOpenComboId() == cb);
    assert(screen.IsComboOpen(cb));

    std::cout << "[PASS] test_combobox_manager_click_opens" << std::endl;
}

void test_combobox_manager_select_item() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);

    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);
    mgr.SetItems(cb, {"Low", "Medium", "High"});

    bool callbackFired = false;
    int32_t selectedIdx = -1;
    mgr.SetSelectionCallback([&](uint32_t, int32_t idx) {
        callbackFired = true;
        selectedIdx = idx;
    });

    // Open the combo
    atlas::ui::UIEvent openClick;
    openClick.type = atlas::ui::UIEvent::Type::MouseDown;
    openClick.mouseButton = 0;
    openClick.x = 50;
    openClick.y = 18;
    mgr.HandleEvent(openClick);
    assert(mgr.GetOpenComboId() == cb);

    // Click second item (items start at y = 10 + 24 = 34, item height = 20)
    // Item 0: y=34..54, Item 1: y=54..74
    atlas::ui::UIEvent selectClick;
    selectClick.type = atlas::ui::UIEvent::Type::MouseDown;
    selectClick.mouseButton = 0;
    selectClick.x = 50;
    selectClick.y = 55; // Inside second item
    mgr.HandleEvent(selectClick);

    assert(callbackFired);
    assert(selectedIdx == 1);
    assert(screen.GetSelectedIndex(cb) == 1);
    assert(!screen.IsComboOpen(cb));
    assert(mgr.GetOpenComboId() == 0);
    assert(mgr.GetSelectedText(cb) == "Medium");

    std::cout << "[PASS] test_combobox_manager_select_item" << std::endl;
}

void test_combobox_manager_click_outside_closes() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);

    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);
    mgr.SetItems(cb, {"X", "Y"});

    // Open
    atlas::ui::UIEvent openClick;
    openClick.type = atlas::ui::UIEvent::Type::MouseDown;
    openClick.mouseButton = 0;
    openClick.x = 50;
    openClick.y = 18;
    mgr.HandleEvent(openClick);
    assert(mgr.GetOpenComboId() == cb);

    // Click outside
    atlas::ui::UIEvent outsideClick;
    outsideClick.type = atlas::ui::UIEvent::Type::MouseDown;
    outsideClick.mouseButton = 0;
    outsideClick.x = 500;
    outsideClick.y = 500;
    mgr.HandleEvent(outsideClick);
    assert(mgr.GetOpenComboId() == 0);
    assert(!screen.IsComboOpen(cb));

    std::cout << "[PASS] test_combobox_manager_click_outside_closes" << std::endl;
}

void test_combobox_manager_get_selected_text_no_selection() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);

    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);
    mgr.SetItems(cb, {"A", "B"});

    assert(mgr.GetSelectedText(cb).empty());

    std::cout << "[PASS] test_combobox_manager_get_selected_text_no_selection" << std::endl;
}

void test_combobox_manager_empty_items() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    atlas::ui::ComboBoxManager mgr;
    mgr.Init(&screen);

    auto& items = mgr.GetItems(999);
    assert(items.empty());

    std::cout << "[PASS] test_combobox_manager_empty_items" << std::endl;
}

// ============================================================
// UIManager Integration Tests
// ============================================================

void test_ui_manager_slider_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Vol", 100, 10, 200, 24);

    bool sliderChanged = false;
    mgr.GetSliderManager().SetSliderChangedCallback([&](uint32_t, float) {
        sliderChanged = true;
    });

    // Click on the slider via UIManager dispatch
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 200;
    click.y = 18;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(sliderChanged);
    assert(std::fabs(screen.GetValue(sl) - 0.5f) < 0.01f);

    std::cout << "[PASS] test_ui_manager_slider_integration" << std::endl;
}

void test_ui_manager_combobox_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Select...", 10, 10, 200, 24);
    mgr.GetComboBoxManager().SetItems(cb, {"Alpha", "Beta", "Gamma"});

    // Click to open the combo
    atlas::ui::UIEvent openClick;
    openClick.type = atlas::ui::UIEvent::Type::MouseDown;
    openClick.mouseButton = 0;
    openClick.x = 50;
    openClick.y = 18;
    mgr.DispatchEvent(openClick);
    assert(screen.IsComboOpen(cb));

    std::cout << "[PASS] test_ui_manager_combobox_integration" << std::endl;
}

void test_ui_manager_new_widget_rendering() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    // Create one of each new widget type
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Test CB", 10, 10, 150, 20);
    screen.SetChecked(cb, true);
    uint32_t sl = screen.AddWidget(atlas::ui::UIWidgetType::Slider, "Slider", 10, 40, 200, 24);
    screen.SetValue(sl, 0.3f);
    uint32_t pb = screen.AddWidget(atlas::ui::UIWidgetType::ProgressBar, "Progress", 10, 70, 300, 20);
    screen.SetValue(pb, 0.7f);
    screen.AddWidget(atlas::ui::UIWidgetType::ComboBox, "Combo", 10, 100, 200, 24);
    uint32_t tn = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "Tree", 10, 130, 200, 20);
    screen.SetExpanded(tn, true);
    screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "Split", 260, 0, 4, 600);
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Color", 10, 160, 24, 24);
    screen.SetColor(cp, 255, 128, 0, 255);

    // Render all with null renderer — should not crash
    atlas::ui::NullUIRenderer nullRenderer;
    mgr.Render(&nullRenderer);

    std::cout << "[PASS] test_ui_manager_new_widget_rendering" << std::endl;
}
