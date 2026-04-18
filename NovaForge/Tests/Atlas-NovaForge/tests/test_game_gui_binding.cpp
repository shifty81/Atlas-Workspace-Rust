#include "../engine/ui/GameGUIBinding.h"
#include <iostream>
#include <cassert>

void test_binding_add() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "hp_label";
    def.propertyName = "text";
    def.sourceComponent = "Health";
    def.sourceField = "current";
    uint64_t id = sys.AddBinding(def);
    assert(id > 0);
    assert(sys.BindingCount() == 1);
    std::cout << "[PASS] test_binding_add" << std::endl;
}

void test_binding_remove() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "w1";
    def.propertyName = "text";
    uint64_t id = sys.AddBinding(def);
    assert(sys.BindingCount() == 1);
    sys.RemoveBinding(id);
    assert(sys.BindingCount() == 0);
    std::cout << "[PASS] test_binding_remove" << std::endl;
}

void test_binding_get() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "w1";
    def.propertyName = "value";
    def.sourceComponent = "Stats";
    uint64_t id = sys.AddBinding(def);
    auto* found = sys.GetBinding(id);
    assert(found != nullptr);
    assert(found->widgetID == "w1");
    assert(sys.GetBinding(999) == nullptr);
    std::cout << "[PASS] test_binding_get" << std::endl;
}

void test_binding_list() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef d1; d1.widgetID = "w1"; d1.propertyName = "a";
    atlas::ui::BindingDef d2; d2.widgetID = "w2"; d2.propertyName = "b";
    sys.AddBinding(d1);
    sys.AddBinding(d2);
    auto list = sys.ListBindings();
    assert(list.size() == 2);
    std::cout << "[PASS] test_binding_list" << std::endl;
}

void test_binding_evaluate_direct() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "hp";
    def.propertyName = "text";
    def.sourceComponent = "Health";
    def.sourceField = "current";
    def.bindingType = atlas::ui::BindingType::Direct;
    uint64_t id = sys.AddBinding(def);

    std::unordered_map<std::string, std::string> data;
    data["Health.current"] = "100";
    auto update = sys.EvaluateBinding(id, data);
    assert(update.widgetID == "hp");
    assert(update.newValue == "100");
    std::cout << "[PASS] test_binding_evaluate_direct" << std::endl;
}

void test_binding_evaluate_formatted() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "hp";
    def.propertyName = "text";
    def.sourceComponent = "Health";
    def.sourceField = "current";
    def.bindingType = atlas::ui::BindingType::Formatted;
    def.formatString = "HP: {}";
    uint64_t id = sys.AddBinding(def);

    std::unordered_map<std::string, std::string> data;
    data["Health.current"] = "75";
    auto update = sys.EvaluateBinding(id, data);
    assert(update.newValue == "HP: 75");
    std::cout << "[PASS] test_binding_evaluate_formatted" << std::endl;
}

void test_binding_evaluate_computed() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "status";
    def.propertyName = "text";
    def.sourceComponent = "Health";
    def.sourceField = "current";
    def.bindingType = atlas::ui::BindingType::Computed;
    def.formatString = "status_fn";
    sys.SetFormatter("status_fn", [](const std::string& v) {
        return v == "0" ? "Dead" : "Alive";
    });
    uint64_t id = sys.AddBinding(def);

    std::unordered_map<std::string, std::string> data;
    data["Health.current"] = "50";
    auto update = sys.EvaluateBinding(id, data);
    assert(update.newValue == "Alive");
    std::cout << "[PASS] test_binding_evaluate_computed" << std::endl;
}

void test_binding_custom_formatter() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "gold";
    def.propertyName = "text";
    def.sourceComponent = "Inventory";
    def.sourceField = "gold";
    def.bindingType = atlas::ui::BindingType::Formatted;
    def.formatString = "gold_fmt";
    sys.SetFormatter("gold_fmt", [](const std::string& v) {
        return v + " gold";
    });
    uint64_t id = sys.AddBinding(def);

    std::unordered_map<std::string, std::string> data;
    data["Inventory.gold"] = "500";
    auto update = sys.EvaluateBinding(id, data);
    assert(update.newValue == "500 gold");
    std::cout << "[PASS] test_binding_custom_formatter" << std::endl;
}

void test_binding_validate() {
    atlas::ui::GameGUIBindingSystem sys;
    assert(sys.Validate()); // empty is valid
    atlas::ui::BindingDef def;
    def.widgetID = "w1";
    def.propertyName = "text";
    sys.AddBinding(def);
    assert(sys.Validate());
    atlas::ui::BindingDef bad;
    bad.widgetID = "";
    bad.propertyName = "text";
    sys.AddBinding(bad);
    assert(!sys.Validate());
    std::cout << "[PASS] test_binding_validate" << std::endl;
}

void test_binding_clear() {
    atlas::ui::GameGUIBindingSystem sys;
    atlas::ui::BindingDef def;
    def.widgetID = "w1";
    def.propertyName = "a";
    sys.AddBinding(def);
    assert(sys.BindingCount() == 1);
    sys.Clear();
    assert(sys.BindingCount() == 0);
    std::cout << "[PASS] test_binding_clear" << std::endl;
}
