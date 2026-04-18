#include "../engine/ui/GameGUIAsset.h"
#include <iostream>
#include <cassert>

void test_gui_asset_create_widget() {
    atlas::ui::GameGUIAsset asset;
    uint64_t id = asset.AddWidget(atlas::ui::WidgetType::Panel, "root");
    assert(id > 0);
    assert(asset.WidgetCount() == 1);
    std::cout << "[PASS] test_gui_asset_create_widget" << std::endl;
}

void test_gui_asset_add_child() {
    atlas::ui::GameGUIAsset asset;
    uint64_t parent = asset.AddWidget(atlas::ui::WidgetType::Panel, "root");
    uint64_t child = asset.AddChild(parent, atlas::ui::WidgetType::Label, "title");
    assert(child > 0);
    assert(asset.WidgetCount() == 2);
    std::cout << "[PASS] test_gui_asset_add_child" << std::endl;
}

void test_gui_asset_find_widget() {
    atlas::ui::GameGUIAsset asset;
    uint64_t id = asset.AddWidget(atlas::ui::WidgetType::Button, "btn");
    auto* found = asset.FindWidget(id);
    assert(found != nullptr);
    assert(found->name == "btn");
    assert(asset.FindWidget(999) == nullptr);
    std::cout << "[PASS] test_gui_asset_find_widget" << std::endl;
}

void test_gui_asset_remove_widget() {
    atlas::ui::GameGUIAsset asset;
    uint64_t id = asset.AddWidget(atlas::ui::WidgetType::Panel, "root");
    assert(asset.WidgetCount() == 1);
    assert(asset.RemoveWidget(id));
    assert(asset.WidgetCount() == 0);
    std::cout << "[PASS] test_gui_asset_remove_widget" << std::endl;
}

void test_gui_asset_list_by_type() {
    atlas::ui::GameGUIAsset asset;
    asset.AddWidget(atlas::ui::WidgetType::Label, "l1");
    asset.AddWidget(atlas::ui::WidgetType::Button, "b1");
    asset.AddWidget(atlas::ui::WidgetType::Label, "l2");
    auto labels = asset.ListWidgetsByType(atlas::ui::WidgetType::Label);
    assert(labels.size() == 2);
    auto buttons = asset.ListWidgetsByType(atlas::ui::WidgetType::Button);
    assert(buttons.size() == 1);
    std::cout << "[PASS] test_gui_asset_list_by_type" << std::endl;
}

void test_gui_asset_properties() {
    atlas::ui::GameGUIAsset asset;
    uint64_t id = asset.AddWidget(atlas::ui::WidgetType::Label, "lbl");
    asset.SetProperty(id, "text", "Hello");
    assert(asset.GetProperty(id, "text") == "Hello");
    assert(asset.GetProperty(id, "missing") == "");
    std::cout << "[PASS] test_gui_asset_properties" << std::endl;
}

void test_gui_asset_bindings() {
    atlas::ui::GameGUIAsset asset;
    uint64_t id = asset.AddWidget(atlas::ui::WidgetType::Label, "hp");
    asset.AddBinding(id, "text", "player.health");
    auto* w = asset.FindWidget(id);
    assert(w != nullptr);
    assert(w->bindings.size() == 1);
    assert(w->bindings[0].bindingKey == "player.health");
    std::cout << "[PASS] test_gui_asset_bindings" << std::endl;
}

void test_gui_asset_validate() {
    atlas::ui::GameGUIAsset asset;
    assert(asset.Validate()); // empty is valid
    uint64_t id = asset.AddWidget(atlas::ui::WidgetType::Panel, "root");
    assert(asset.Validate());
    std::cout << "[PASS] test_gui_asset_validate" << std::endl;
}

void test_gui_asset_export_json() {
    atlas::ui::GameGUIAsset asset;
    asset.AddWidget(atlas::ui::WidgetType::Panel, "root");
    std::string json = asset.ExportJSON();
    assert(!json.empty());
    assert(json.find("root") != std::string::npos);
    std::cout << "[PASS] test_gui_asset_export_json" << std::endl;
}

void test_gui_asset_clear() {
    atlas::ui::GameGUIAsset asset;
    asset.AddWidget(atlas::ui::WidgetType::Panel, "a");
    asset.AddWidget(atlas::ui::WidgetType::Panel, "b");
    assert(asset.WidgetCount() == 2);
    asset.Clear();
    assert(asset.WidgetCount() == 0);
    std::cout << "[PASS] test_gui_asset_clear" << std::endl;
}
