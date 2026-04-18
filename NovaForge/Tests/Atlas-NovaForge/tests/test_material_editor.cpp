#include "../editor/panels/MaterialEditorPanel.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::procedural;

void test_material_editor_name() {
    MaterialEditorPanel panel;
    assert(std::string(panel.Name()) == "Material Editor");
    std::cout << "[PASS] test_material_editor_name" << std::endl;
}

void test_material_editor_no_material() {
    MaterialEditorPanel panel;
    assert(!panel.HasMaterial());
    assert(panel.Summary() == "No material loaded");
    assert(panel.SelectedParameter() == -1);
    std::cout << "[PASS] test_material_editor_no_material" << std::endl;
}

void test_material_editor_load_material() {
    MaterialEditorPanel panel;

    MaterialData mat;
    mat.width = 2;
    mat.height = 2;
    mat.albedo.resize(16, 1.0f);   // 4 pixels * RGBA
    mat.normal.resize(12, 0.0f);   // 4 pixels * XYZ
    mat.roughness.resize(4, 0.5f); // 4 pixels
    mat.metallic.resize(4, 0.0f);  // 4 pixels

    panel.LoadMaterial(mat);
    assert(panel.HasMaterial());
    assert(panel.GetMaterial().width == 2);
    assert(panel.GetMaterial().height == 2);

    std::cout << "[PASS] test_material_editor_load_material" << std::endl;
}

void test_material_editor_add_parameter() {
    MaterialEditorPanel panel;

    MaterialParameter p;
    p.name = "roughness";
    p.type = "float";
    p.value = "0.5";
    panel.AddParameter(p);

    assert(panel.Parameters().size() == 1);
    assert(panel.IsDirty());

    std::cout << "[PASS] test_material_editor_add_parameter" << std::endl;
}

void test_material_editor_remove_parameter() {
    MaterialEditorPanel panel;

    MaterialParameter p1;
    p1.name = "roughness";
    p1.type = "float";
    p1.value = "0.5";
    panel.AddParameter(p1);

    MaterialParameter p2;
    p2.name = "metallic";
    p2.type = "float";
    p2.value = "0.0";
    panel.AddParameter(p2);

    assert(panel.Parameters().size() == 2);

    panel.RemoveParameter("roughness");
    assert(panel.Parameters().size() == 1);
    assert(panel.Parameters()[0].name == "metallic");

    std::cout << "[PASS] test_material_editor_remove_parameter" << std::endl;
}

void test_material_editor_get_parameter() {
    MaterialEditorPanel panel;

    MaterialParameter p;
    p.name = "color";
    p.type = "color";
    p.value = "#FF0000";
    panel.AddParameter(p);

    const auto* found = panel.GetParameter("color");
    assert(found != nullptr);
    assert(found->value == "#FF0000");

    assert(panel.GetParameter("nonexistent") == nullptr);

    std::cout << "[PASS] test_material_editor_get_parameter" << std::endl;
}

void test_material_editor_set_parameter() {
    MaterialEditorPanel panel;

    MaterialParameter p;
    p.name = "roughness";
    p.type = "float";
    p.value = "0.5";
    panel.AddParameter(p);

    panel.MarkClean();
    assert(!panel.IsDirty());

    panel.SetParameter("roughness", "0.8");
    assert(panel.GetParameter("roughness")->value == "0.8");
    assert(panel.IsDirty());

    std::cout << "[PASS] test_material_editor_set_parameter" << std::endl;
}

void test_material_editor_select_parameter() {
    MaterialEditorPanel panel;

    MaterialParameter p;
    p.name = "roughness";
    p.type = "float";
    p.value = "0.5";
    panel.AddParameter(p);

    assert(panel.SelectedParameter() == -1);

    panel.SelectParameter(0);
    assert(panel.SelectedParameter() == 0);

    // Out of range
    panel.SelectParameter(5);
    assert(panel.SelectedParameter() == -1);

    std::cout << "[PASS] test_material_editor_select_parameter" << std::endl;
}

void test_material_editor_preview_mode() {
    MaterialEditorPanel panel;
    assert(!panel.PreviewMode());

    panel.SetPreviewMode(true);
    assert(panel.PreviewMode());

    panel.SetPreviewMode(false);
    assert(!panel.PreviewMode());

    std::cout << "[PASS] test_material_editor_preview_mode" << std::endl;
}

void test_material_editor_summary() {
    MaterialEditorPanel panel;

    MaterialData mat;
    mat.width = 4;
    mat.height = 4;
    mat.albedo.resize(64, 1.0f);
    mat.normal.resize(48, 0.0f);
    mat.roughness.resize(16, 0.5f);
    mat.metallic.resize(16, 0.0f);

    panel.LoadMaterial(mat);

    MaterialParameter p;
    p.name = "color";
    p.type = "color";
    p.value = "#FFFFFF";
    panel.AddParameter(p);

    std::string summary = panel.Summary();
    assert(summary.find("4x4") != std::string::npos);
    assert(summary.find("1 params") != std::string::npos);
    assert(summary.find("modified") != std::string::npos);

    std::cout << "[PASS] test_material_editor_summary" << std::endl;
}

void test_material_editor_clear() {
    MaterialEditorPanel panel;

    MaterialData mat;
    mat.width = 2;
    mat.height = 2;
    mat.albedo.resize(16, 1.0f);
    mat.normal.resize(12, 0.0f);
    mat.roughness.resize(4, 0.5f);
    mat.metallic.resize(4, 0.0f);
    panel.LoadMaterial(mat);

    MaterialParameter p;
    p.name = "test";
    p.type = "float";
    p.value = "1.0";
    panel.AddParameter(p);

    panel.SelectParameter(0);

    panel.Clear();
    assert(!panel.HasMaterial());
    assert(panel.Parameters().empty());
    assert(panel.SelectedParameter() == -1);
    assert(!panel.IsDirty());
    assert(panel.Summary() == "No material loaded");

    std::cout << "[PASS] test_material_editor_clear" << std::endl;
}
