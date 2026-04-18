#include "../editor/panels/PrefabEditorPanel.h"
#include <iostream>
#include <cassert>
#include <cstdint>

using namespace atlas::editor;

void test_prefab_editor_name() {
    PrefabEditorPanel panel;
    assert(std::string(panel.Name()) == "Prefab Editor");
    std::cout << "[PASS] test_prefab_editor_name" << std::endl;
}

void test_prefab_editor_empty() {
    PrefabEditorPanel panel;
    assert(panel.EntityCount() == 0);
    assert(panel.RootEntities().empty());
    assert(panel.Summary() == "Empty prefab");
    assert(panel.SelectedEntity() == 0);
    assert(!panel.IsDirty());
    std::cout << "[PASS] test_prefab_editor_empty" << std::endl;
}

void test_prefab_editor_add_entity() {
    PrefabEditorPanel panel;

    uint32_t id = panel.AddEntity("Player");
    assert(id > 0);
    assert(panel.EntityCount() == 1);
    assert(panel.IsDirty());

    const auto* entity = panel.GetEntity(id);
    assert(entity != nullptr);
    assert(entity->name == "Player");
    assert(entity->parent == 0);

    std::cout << "[PASS] test_prefab_editor_add_entity" << std::endl;
}

void test_prefab_editor_remove_entity() {
    PrefabEditorPanel panel;

    uint32_t id1 = panel.AddEntity("Entity1");
    uint32_t id2 = panel.AddEntity("Entity2");
    assert(panel.EntityCount() == 2);

    panel.RemoveEntity(id1);
    assert(panel.EntityCount() == 1);
    assert(panel.GetEntity(id1) == nullptr);
    assert(panel.GetEntity(id2) != nullptr);

    std::cout << "[PASS] test_prefab_editor_remove_entity" << std::endl;
}

void test_prefab_editor_parent_child() {
    PrefabEditorPanel panel;

    uint32_t parent = panel.AddEntity("Ship");
    uint32_t child1 = panel.AddEntity("Turret1");
    uint32_t child2 = panel.AddEntity("Turret2");

    panel.SetParent(child1, parent);
    panel.SetParent(child2, parent);

    auto roots = panel.RootEntities();
    assert(roots.size() == 1);
    assert(roots[0] == parent);

    auto children = panel.Children(parent);
    assert(children.size() == 2);

    std::cout << "[PASS] test_prefab_editor_parent_child" << std::endl;
}

void test_prefab_editor_remove_parent_removes_children() {
    PrefabEditorPanel panel;

    uint32_t parent = panel.AddEntity("Ship");
    uint32_t child = panel.AddEntity("Turret");
    panel.SetParent(child, parent);

    assert(panel.EntityCount() == 2);

    panel.RemoveEntity(parent);
    assert(panel.EntityCount() == 0);

    std::cout << "[PASS] test_prefab_editor_remove_parent_removes_children" << std::endl;
}

void test_prefab_editor_remove_grandchildren() {
    PrefabEditorPanel panel;

    uint32_t root = panel.AddEntity("Root");
    uint32_t child = panel.AddEntity("Child");
    uint32_t grandchild = panel.AddEntity("Grandchild");
    panel.SetParent(child, root);
    panel.SetParent(grandchild, child);

    assert(panel.EntityCount() == 3);

    panel.RemoveEntity(root);
    assert(panel.EntityCount() == 0);
    assert(panel.GetEntity(grandchild) == nullptr);

    std::cout << "[PASS] test_prefab_editor_remove_grandchildren" << std::endl;
}

void test_prefab_editor_remove_clears_selected_child() {
    PrefabEditorPanel panel;

    uint32_t parent = panel.AddEntity("Ship");
    uint32_t child = panel.AddEntity("Turret");
    panel.SetParent(child, parent);

    panel.SelectEntity(child);
    assert(panel.SelectedEntity() == child);

    panel.RemoveEntity(parent);
    assert(panel.SelectedEntity() == 0);

    std::cout << "[PASS] test_prefab_editor_remove_clears_selected_child" << std::endl;
}

void test_prefab_editor_add_component() {
    PrefabEditorPanel panel;
    uint32_t id = panel.AddEntity("Player");

    PrefabComponent comp;
    comp.type = "Transform";
    comp.properties = {{"x", "0"}, {"y", "0"}, {"z", "0"}};
    panel.AddComponent(id, comp);

    const auto* entity = panel.GetEntity(id);
    assert(entity->components.size() == 1);
    assert(entity->components[0].type == "Transform");
    assert(entity->components[0].GetProperty("x") == "0");

    std::cout << "[PASS] test_prefab_editor_add_component" << std::endl;
}

void test_prefab_editor_remove_component() {
    PrefabEditorPanel panel;
    uint32_t id = panel.AddEntity("Player");

    PrefabComponent t;
    t.type = "Transform";
    panel.AddComponent(id, t);

    PrefabComponent h;
    h.type = "Health";
    h.properties = {{"max", "100"}};
    panel.AddComponent(id, h);

    assert(panel.GetEntity(id)->components.size() == 2);

    panel.RemoveComponent(id, "Transform");
    assert(panel.GetEntity(id)->components.size() == 1);
    assert(panel.GetEntity(id)->components[0].type == "Health");

    std::cout << "[PASS] test_prefab_editor_remove_component" << std::endl;
}

void test_prefab_editor_select_entity() {
    PrefabEditorPanel panel;
    uint32_t id = panel.AddEntity("Player");

    assert(panel.SelectedEntity() == 0);

    panel.SelectEntity(id);
    assert(panel.SelectedEntity() == id);

    // Removing selected entity should deselect
    panel.RemoveEntity(id);
    assert(panel.SelectedEntity() == 0);

    std::cout << "[PASS] test_prefab_editor_select_entity" << std::endl;
}

void test_prefab_editor_summary() {
    PrefabEditorPanel panel;
    uint32_t id = panel.AddEntity("Ship");

    PrefabComponent t;
    t.type = "Transform";
    panel.AddComponent(id, t);

    PrefabComponent r;
    r.type = "Renderer";
    panel.AddComponent(id, r);

    std::string summary = panel.Summary();
    assert(summary.find("Entities: 1") != std::string::npos);
    assert(summary.find("Components: 2") != std::string::npos);
    assert(summary.find("modified") != std::string::npos);

    std::cout << "[PASS] test_prefab_editor_summary" << std::endl;
}

void test_prefab_editor_clear() {
    PrefabEditorPanel panel;
    panel.AddEntity("A");
    panel.AddEntity("B");
    panel.SelectEntity(1);

    panel.Clear();
    assert(panel.EntityCount() == 0);
    assert(panel.SelectedEntity() == 0);
    assert(!panel.IsDirty());
    assert(panel.Summary() == "Empty prefab");

    std::cout << "[PASS] test_prefab_editor_clear" << std::endl;
}
