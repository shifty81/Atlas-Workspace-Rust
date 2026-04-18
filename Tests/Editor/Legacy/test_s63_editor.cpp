#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AdvancedViewports.h"

using namespace NF;
using Catch::Approx;

// ── PropertyEditor::readProperty ─────────────────────────────────

TEST_CASE("PropertyEditor readProperty bool true", "[Editor][S63]") {
    struct TestObj { bool flag = true; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Bool;
    prop.offset = offsetof(TestObj, flag);
    JsonValue val = PropertyEditor::readProperty(&obj, prop);
    REQUIRE(val.isBool());
    REQUIRE(val.asBool() == true);
}

TEST_CASE("PropertyEditor readProperty bool false", "[Editor][S63]") {
    struct TestObj { bool flag = false; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Bool;
    prop.offset = offsetof(TestObj, flag);
    JsonValue val = PropertyEditor::readProperty(&obj, prop);
    REQUIRE(val.isBool());
    REQUIRE(val.asBool() == false);
}

TEST_CASE("PropertyEditor readProperty int32", "[Editor][S63]") {
    struct TestObj { int32_t count = 42; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Int32;
    prop.offset = offsetof(TestObj, count);
    JsonValue val = PropertyEditor::readProperty(&obj, prop);
    REQUIRE(val.isNumber());
    REQUIRE(val.asInt() == 42);
}

TEST_CASE("PropertyEditor readProperty float", "[Editor][S63]") {
    struct TestObj { float speed = 3.14f; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Float;
    prop.offset = offsetof(TestObj, speed);
    JsonValue val = PropertyEditor::readProperty(&obj, prop);
    REQUIRE(val.isNumber());
    REQUIRE(val.asFloat() == Approx(3.14f).epsilon(0.001f));
}

TEST_CASE("PropertyEditor readProperty string", "[Editor][S63]") {
    struct TestObj { std::string name = "Atlas"; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::String;
    prop.offset = offsetof(TestObj, name);
    JsonValue val = PropertyEditor::readProperty(&obj, prop);
    REQUIRE(val.isString());
    REQUIRE(val.asString() == "Atlas");
}

// ── PropertyEditor::writeProperty ────────────────────────────────

TEST_CASE("PropertyEditor writeProperty bool", "[Editor][S63]") {
    struct TestObj { bool flag = false; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Bool;
    prop.offset = offsetof(TestObj, flag);
    bool ok = PropertyEditor::writeProperty(&obj, prop, JsonValue(true));
    REQUIRE(ok);
    REQUIRE(obj.flag == true);
}

TEST_CASE("PropertyEditor writeProperty int32", "[Editor][S63]") {
    struct TestObj { int32_t value = 0; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Int32;
    prop.offset = offsetof(TestObj, value);
    bool ok = PropertyEditor::writeProperty(&obj, prop, JsonValue(99));
    REQUIRE(ok);
    REQUIRE(obj.value == 99);
}

TEST_CASE("PropertyEditor writeProperty float", "[Editor][S63]") {
    struct TestObj { float speed = 0.f; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Float;
    prop.offset = offsetof(TestObj, speed);
    bool ok = PropertyEditor::writeProperty(&obj, prop, JsonValue(2.5f));
    REQUIRE(ok);
    REQUIRE(obj.speed == Approx(2.5f));
}

TEST_CASE("PropertyEditor writeProperty string", "[Editor][S63]") {
    struct TestObj { std::string name; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::String;
    prop.offset = offsetof(TestObj, name);
    bool ok = PropertyEditor::writeProperty(&obj, prop, JsonValue(std::string("World")));
    REQUIRE(ok);
    REQUIRE(obj.name == "World");
}

TEST_CASE("PropertyEditor writeProperty type mismatch returns false", "[Editor][S63]") {
    struct TestObj { bool flag = false; };
    TestObj obj;
    PropertyInfo prop;
    prop.type = PropertyType::Bool;
    prop.offset = offsetof(TestObj, flag);
    bool ok = PropertyEditor::writeProperty(&obj, prop, JsonValue(42));
    REQUIRE_FALSE(ok);
}

// ── PropertyEditor::makeFloatChange ──────────────────────────────

TEST_CASE("PropertyEditor makeFloatChange creates undo command", "[Editor][S63]") {
    float val = 1.0f;
    auto cmd = PropertyEditor::makeFloatChange(&val, 5.0f, "Speed");
    REQUIRE(cmd != nullptr);
    REQUIRE_FALSE(cmd->description().empty());
    cmd->execute();
    REQUIRE(val == Approx(5.0f));
    cmd->undo();
    REQUIRE(val == Approx(1.0f));
}

TEST_CASE("PropertyEditor makeIntChange creates undo command", "[Editor][S63]") {
    int32_t val = 10;
    auto cmd = PropertyEditor::makeIntChange(&val, 20, "Count");
    cmd->execute();
    REQUIRE(val == 20);
    cmd->undo();
    REQUIRE(val == 10);
}

TEST_CASE("PropertyEditor makeBoolChange creates undo command", "[Editor][S63]") {
    bool val = false;
    auto cmd = PropertyEditor::makeBoolChange(&val, true, "Visible");
    cmd->execute();
    REQUIRE(val == true);
    cmd->undo();
    REQUIRE(val == false);
}

// ── MeshViewerAsset ───────────────────────────────────────────────

TEST_CASE("MeshViewerAsset defaults", "[Editor][S63]") {
    MeshViewerAsset asset;
    REQUIRE(asset.vertexCount   == 0);
    REQUIRE(asset.triangleCount == 0);
    REQUIRE(asset.lodCount      == 1);
    REQUIRE(asset.hasNormals);
    REQUIRE(asset.hasUVs);
    REQUIRE_FALSE(asset.loaded);
}

TEST_CASE("MeshViewerAsset isHighPoly threshold", "[Editor][S63]") {
    MeshViewerAsset asset;
    asset.triangleCount = 50000;
    REQUIRE_FALSE(asset.isHighPoly());
    asset.triangleCount = 100000;
    REQUIRE(asset.isHighPoly());
}

TEST_CASE("MeshViewerAsset isMultiLOD", "[Editor][S63]") {
    MeshViewerAsset asset;
    REQUIRE_FALSE(asset.isMultiLOD());
    asset.lodCount = 3;
    REQUIRE(asset.isMultiLOD());
}

TEST_CASE("MeshViewerAsset isComplete", "[Editor][S63]") {
    MeshViewerAsset asset;
    REQUIRE_FALSE(asset.isComplete());
    asset.loaded = true;
    REQUIRE(asset.isComplete());
}

// ── MeshViewerPanel ───────────────────────────────────────────────

TEST_CASE("MeshViewerPanel name is MeshViewer", "[Editor][S63]") {
    MeshViewerPanel panel;
    REQUIRE(panel.name() == "MeshViewer");
}

TEST_CASE("MeshViewerPanel starts empty", "[Editor][S63]") {
    MeshViewerPanel panel;
    REQUIRE(panel.meshCount() == 0);
}

TEST_CASE("MeshViewerPanel addMesh increases count", "[Editor][S63]") {
    MeshViewerPanel panel;
    MeshViewerAsset asset;
    asset.name = "Cube";
    REQUIRE(panel.addMesh(asset));
    REQUIRE(panel.meshCount() == 1);
}

TEST_CASE("MeshViewerPanel addMesh duplicate name rejected", "[Editor][S63]") {
    MeshViewerPanel panel;
    MeshViewerAsset asset;
    asset.name = "Cube";
    panel.addMesh(asset);
    REQUIRE_FALSE(panel.addMesh(asset));
    REQUIRE(panel.meshCount() == 1);
}

TEST_CASE("MeshViewerPanel removeMesh decreases count", "[Editor][S63]") {
    MeshViewerPanel panel;
    MeshViewerAsset asset;
    asset.name = "Sphere";
    panel.addMesh(asset);
    REQUIRE(panel.removeMesh("Sphere"));
    REQUIRE(panel.meshCount() == 0);
}

TEST_CASE("MeshViewerPanel setActiveMesh succeeds for known mesh", "[Editor][S63]") {
    MeshViewerPanel panel;
    MeshViewerAsset asset;
    asset.name = "Tree";
    panel.addMesh(asset);
    REQUIRE(panel.setActiveMesh("Tree"));
    REQUIRE(panel.activeMesh() == "Tree");
}

TEST_CASE("MeshViewerPanel setActiveMesh fails for unknown mesh", "[Editor][S63]") {
    MeshViewerPanel panel;
    REQUIRE_FALSE(panel.setActiveMesh("Unknown"));
}

TEST_CASE("MeshViewerPanel default display mode is Lit", "[Editor][S63]") {
    MeshViewerPanel panel;
    REQUIRE(panel.displayMode() == MeshDisplayMode::Lit);
}

TEST_CASE("MeshViewerPanel setDisplayMode updates mode", "[Editor][S63]") {
    MeshViewerPanel panel;
    panel.setDisplayMode(MeshDisplayMode::Wireframe);
    REQUIRE(panel.displayMode() == MeshDisplayMode::Wireframe);
}

TEST_CASE("MeshViewerPanel default camera mode is Orbit", "[Editor][S63]") {
    MeshViewerPanel panel;
    REQUIRE(panel.cameraMode() == ViewportCameraMode::Orbit);
}

TEST_CASE("MeshViewerPanel MAX_MESHES enforced", "[Editor][S63]") {
    MeshViewerPanel panel;
    for (size_t i = 0; i < MeshViewerPanel::MAX_MESHES; ++i) {
        MeshViewerAsset a;
        a.name = "mesh_" + std::to_string(i);
        panel.addMesh(a);
    }
    REQUIRE(panel.meshCount() == MeshViewerPanel::MAX_MESHES);
    MeshViewerAsset extra;
    extra.name = "overflow";
    REQUIRE_FALSE(panel.addMesh(extra));
}
