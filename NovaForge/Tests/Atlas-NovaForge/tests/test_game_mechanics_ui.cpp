#include "../editor/panels/GameMechanicsUIPanel.h"
#include <iostream>
#include <cassert>

void test_mechanics_ui_name() {
    atlas::editor::GameMechanicsUIPanel panel;
    assert(std::string(panel.Name()) == "Game Mechanics UI");
    std::cout << "[PASS] test_mechanics_ui_name" << std::endl;
}

void test_mechanics_ui_add_element() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement elem;
    elem.name = "health_bar";
    elem.elementType = "ProgressBar";
    uint64_t id = panel.AddElement(elem);
    assert(id > 0);
    assert(panel.ElementCount() == 1);
    std::cout << "[PASS] test_mechanics_ui_add_element" << std::endl;
}

void test_mechanics_ui_remove_element() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement elem;
    elem.name = "btn";
    uint64_t id = panel.AddElement(elem);
    assert(panel.ElementCount() == 1);
    assert(panel.RemoveElement(id));
    assert(panel.ElementCount() == 0);
    std::cout << "[PASS] test_mechanics_ui_remove_element" << std::endl;
}

void test_mechanics_ui_get_element() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement elem;
    elem.name = "inv_grid";
    elem.elementType = "Grid";
    uint64_t id = panel.AddElement(elem);
    auto* found = panel.GetElement(id);
    assert(found != nullptr);
    assert(found->name == "inv_grid");
    assert(panel.GetElement(999) == nullptr);
    std::cout << "[PASS] test_mechanics_ui_get_element" << std::endl;
}

void test_mechanics_ui_list_elements() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement e1; e1.name = "a";
    atlas::editor::MechanicsUIElement e2; e2.name = "b";
    panel.AddElement(e1);
    panel.AddElement(e2);
    auto list = panel.ListElements();
    assert(list.size() == 2);
    std::cout << "[PASS] test_mechanics_ui_list_elements" << std::endl;
}

void test_mechanics_ui_select_element() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement elem;
    elem.name = "sel";
    uint64_t id = panel.AddElement(elem);
    assert(panel.SelectedElementID() == 0);
    panel.SelectElement(id);
    assert(panel.SelectedElementID() == id);
    std::cout << "[PASS] test_mechanics_ui_select_element" << std::endl;
}

void test_mechanics_ui_preview_mode() {
    atlas::editor::GameMechanicsUIPanel panel;
    assert(!panel.IsPreviewMode());
    panel.SetPreviewMode(true);
    assert(panel.IsPreviewMode());
    panel.SetPreviewMode(false);
    assert(!panel.IsPreviewMode());
    std::cout << "[PASS] test_mechanics_ui_preview_mode" << std::endl;
}

void test_mechanics_ui_export() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement elem;
    elem.name = "hud";
    elem.elementType = "Panel";
    panel.AddElement(elem);
    std::string json = panel.ExportLayout();
    assert(!json.empty());
    assert(json.find("hud") != std::string::npos);
    std::cout << "[PASS] test_mechanics_ui_export" << std::endl;
}

void test_mechanics_ui_import() {
    atlas::editor::GameMechanicsUIPanel panel;
    assert(panel.ImportLayout("{\"elements\":[]}"));
    assert(!panel.ImportLayout(""));
    assert(!panel.ImportLayout("{}"));
    std::cout << "[PASS] test_mechanics_ui_import" << std::endl;
}

void test_mechanics_ui_clear() {
    atlas::editor::GameMechanicsUIPanel panel;
    atlas::editor::MechanicsUIElement elem;
    elem.name = "x";
    panel.AddElement(elem);
    panel.SetPreviewMode(true);
    panel.SelectElement(1);
    assert(panel.ElementCount() == 1);
    panel.Clear();
    assert(panel.ElementCount() == 0);
    assert(panel.SelectedElementID() == 0);
    assert(!panel.IsPreviewMode());
    std::cout << "[PASS] test_mechanics_ui_clear" << std::endl;
}
