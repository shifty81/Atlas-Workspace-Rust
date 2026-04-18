// Phase D — Runtime-Backed Viewport
//
// Tests for:
//   D.1: NovaForgePreviewWorld entity lifecycle, transforms, mesh/material, selection
//   D.1: NovaForgePreviewRuntime IViewportSceneProvider, fly-camera, gizmo, hierarchy
//   D.2: SceneEditorTool.provideScene() delegates to attached runtime
//   D.2: Fly-camera updates position on forward/strafe/up input
//   D.2: Gizmo state reflects selected entity position
//   D.2: selectedEntityProperties() returns expected keys
//   D.2: hierarchyOrder() puts parents before children
//   D.3: NovaForgeAssetPreview IViewportSceneProvider, bind/clear, dirty tracking
//   D.3: Asset preview editable fields (mesh, material, attachment, transform)
//   D.3: apply()/revert() baseline management
//   D.3: properties() returns all expected keys
//   D.3: AssetEditorTool.provideScene() delegates to attached asset preview

#include <catch2/catch_test_macros.hpp>

#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewRuntime.h"
#include "NovaForge/EditorAdapter/NovaForgeAssetPreview.h"
#include "NF/Editor/SceneEditorTool.h"
#include "NF/Editor/AssetEditorTool.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace NovaForge;

// ═══════════════════════════════════════════════════════════════════════════
//  D.1 — NovaForgePreviewWorld
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.1 PreviewWorld: starts empty", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    REQUIRE(world.entityCount() == 0);
    REQUIRE_FALSE(world.hasSelection());
    REQUIRE(world.selectedEntityId() == kInvalidEntityId);
    REQUIRE_FALSE(world.isDirty());
}

TEST_CASE("D.1 PreviewWorld: createEntity returns valid id", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("Root");
    REQUIRE(id != kInvalidEntityId);
    REQUIRE(world.entityCount() == 1);
    REQUIRE(world.hasEntity(id));
    REQUIRE(world.isDirty());
}

TEST_CASE("D.1 PreviewWorld: createEntity stores name", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("MyEntity");
    const PreviewEntity* e = world.find(id);
    REQUIRE(e != nullptr);
    REQUIRE(e->name == "MyEntity");
}

TEST_CASE("D.1 PreviewWorld: createEntity with parent stores parentId", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId parent = world.createEntity("Parent");
    EntityId child  = world.createEntity("Child", parent);
    const PreviewEntity* ce = world.find(child);
    REQUIRE(ce != nullptr);
    REQUIRE(ce->parentId == parent);
}

TEST_CASE("D.1 PreviewWorld: destroyEntity removes it", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    world.clearDirty();
    REQUIRE(world.destroyEntity(id));
    REQUIRE(world.entityCount() == 0);
    REQUIRE_FALSE(world.hasEntity(id));
    REQUIRE(world.isDirty());
}

TEST_CASE("D.1 PreviewWorld: destroyEntity non-existent returns false", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    REQUIRE_FALSE(world.destroyEntity(999));
}

TEST_CASE("D.1 PreviewWorld: destroyEntity deselects if selected", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    world.selectEntity(id);
    REQUIRE(world.hasSelection());
    world.destroyEntity(id);
    REQUIRE_FALSE(world.hasSelection());
}

TEST_CASE("D.1 PreviewWorld: clearEntities removes all", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    world.createEntity("A");
    world.createEntity("B");
    world.createEntity("C");
    world.clearEntities();
    REQUIRE(world.entityCount() == 0);
}

TEST_CASE("D.1 PreviewWorld: setTransform updates entity", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    PreviewTransform t;
    t.position = {1.f, 2.f, 3.f};
    t.rotation = {10.f, 20.f, 30.f};
    t.scale    = {2.f, 2.f, 2.f};
    REQUIRE(world.setTransform(id, t));
    const auto* e = world.find(id);
    REQUIRE(e->transform.position.x == 1.f);
    REQUIRE(e->transform.position.y == 2.f);
    REQUIRE(e->transform.position.z == 3.f);
    REQUIRE(e->transform.scale.x == 2.f);
}

TEST_CASE("D.1 PreviewWorld: setPosition updates position only", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    REQUIRE(world.setPosition(id, {5.f, 6.f, 7.f}));
    const auto* e = world.find(id);
    REQUIRE(e->transform.position.x == 5.f);
    REQUIRE(e->transform.position.y == 6.f);
    REQUIRE(e->transform.position.z == 7.f);
    REQUIRE(e->transform.scale.x == 1.f); // unchanged
}

TEST_CASE("D.1 PreviewWorld: setRotation and setScale work independently", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    REQUIRE(world.setRotation(id, {45.f, 0.f, 0.f}));
    REQUIRE(world.setScale(id, {3.f, 3.f, 3.f}));
    const auto* e = world.find(id);
    REQUIRE(e->transform.rotation.x == 45.f);
    REQUIRE(e->transform.scale.x == 3.f);
}

TEST_CASE("D.1 PreviewWorld: setMesh stores meshTag", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    REQUIRE(world.setMesh(id, "meshes/cube.nfm"));
    REQUIRE(world.find(id)->meshTag == "meshes/cube.nfm");
    REQUIRE(world.isDirty());
}

TEST_CASE("D.1 PreviewWorld: setMaterial stores materialTag", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    REQUIRE(world.setMaterial(id, "materials/metal.nfmat"));
    REQUIRE(world.find(id)->materialTag == "materials/metal.nfmat");
}

TEST_CASE("D.1 PreviewWorld: setVisible false hides entity", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    REQUIRE(world.setVisible(id, false));
    REQUIRE_FALSE(world.find(id)->visible);
}

TEST_CASE("D.1 PreviewWorld: setTransform on invalid id returns false", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    REQUIRE_FALSE(world.setTransform(999, {}));
}

TEST_CASE("D.1 PreviewWorld: selectEntity sets selection", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    REQUIRE(world.selectEntity(id));
    REQUIRE(world.hasSelection());
    REQUIRE(world.selectedEntityId() == id);
    REQUIRE(world.selectedEntity() == world.find(id));
}

TEST_CASE("D.1 PreviewWorld: selectEntity with kInvalidEntityId clears selection", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    world.selectEntity(id);
    REQUIRE(world.selectEntity(kInvalidEntityId));
    REQUIRE_FALSE(world.hasSelection());
}

TEST_CASE("D.1 PreviewWorld: selectEntity non-existent returns false", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    REQUIRE_FALSE(world.selectEntity(999));
}

TEST_CASE("D.1 PreviewWorld: deselectAll clears selection", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId id = world.createEntity("E");
    world.selectEntity(id);
    world.deselectAll();
    REQUIRE_FALSE(world.hasSelection());
}

TEST_CASE("D.1 PreviewWorld: clearDirty resets flag", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    world.createEntity("E");
    REQUIRE(world.isDirty());
    world.clearDirty();
    REQUIRE_FALSE(world.isDirty());
}

TEST_CASE("D.1 PreviewWorld: entities() returns all", "[phase_d][d1][world]") {
    NovaForgePreviewWorld world;
    EntityId a = world.createEntity("A");
    EntityId b = world.createEntity("B");
    const auto& all = world.entities();
    REQUIRE(all.size() == 2);
    bool foundA = false, foundB = false;
    for (const auto& e : all) {
        if (e.id == a) foundA = true;
        if (e.id == b) foundB = true;
    }
    REQUIRE(foundA);
    REQUIRE(foundB);
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.1 — NovaForgePreviewRuntime
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.1 Runtime: starts not running", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    REQUIRE_FALSE(rt.isRunning());
    REQUIRE(rt.elapsedSeconds() == 0.f);
}

TEST_CASE("D.1 Runtime: start/stop toggles isRunning", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    rt.start();
    REQUIRE(rt.isRunning());
    rt.stop();
    REQUIRE_FALSE(rt.isRunning());
}

TEST_CASE("D.1 Runtime: tick accumulates elapsed only while running", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    rt.tick(1.f);
    REQUIRE(rt.elapsedSeconds() == 0.f); // not running
    rt.start();
    rt.tick(1.5f);
    REQUIRE(rt.elapsedSeconds() == 1.5f);
    rt.stop();
    rt.tick(1.f);
    REQUIRE(rt.elapsedSeconds() == 1.5f); // stopped, no accumulation
}

TEST_CASE("D.1 Runtime: provideScene hasContent=false when stopped", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    NF::ViewportSlot slot;
    auto state = rt.provideScene(1, slot);
    REQUIRE_FALSE(state.hasContent);
}

TEST_CASE("D.1 Runtime: provideScene hasContent=true when running with entities", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    rt.start();
    rt.world().createEntity("TestEntity");
    NF::ViewportSlot slot;
    auto state = rt.provideScene(1, slot);
    REQUIRE(state.hasContent);
}

TEST_CASE("D.1 Runtime: provideScene hasContent=true when stopped but has entities", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    // Not started — static preview mode
    rt.world().createEntity("StaticEntity");
    NF::ViewportSlot slot;
    auto state = rt.provideScene(1, slot);
    REQUIRE(state.hasContent);      // content exists even when paused
    REQUIRE_FALSE(state.overrideCamera); // camera is NOT authoritative when stopped
}

TEST_CASE("D.1 Runtime: provideScene overrideCamera=true only when running", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    rt.world().createEntity("E");
    NF::ViewportSlot slot;
    auto stopped = rt.provideScene(1, slot);
    REQUIRE_FALSE(stopped.overrideCamera);
    rt.start();
    auto running = rt.provideScene(1, slot);
    REQUIRE(running.overrideCamera);
}

TEST_CASE("D.1 Runtime: provideScene entityCount matches world", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    rt.start();
    rt.world().createEntity("A");
    rt.world().createEntity("B");
    NF::ViewportSlot slot;
    auto state = rt.provideScene(1, slot);
    REQUIRE(state.entityCount == 2);
}

TEST_CASE("D.1 Runtime: world() is accessible and mutable", "[phase_d][d1][runtime]") {
    NovaForgePreviewRuntime rt;
    EntityId id = rt.world().createEntity("TestEntity");
    REQUIRE(id != kInvalidEntityId);
    REQUIRE(rt.world().hasEntity(id));
}

// ── Fly-camera ────────────────────────────────────────────────────────────

TEST_CASE("D.2 Fly-camera: default camera state", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    const auto& cam = rt.cameraState();
    REQUIRE(cam.yaw == -90.f);
    REQUIRE(cam.pitch == 0.f);
    REQUIRE(cam.speed > 0.f);
}

TEST_CASE("D.2 Fly-camera: setCameraState applies state", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.x = 10.f; s.y = 5.f; s.z = -3.f;
    s.yaw = 45.f; s.pitch = 15.f;
    rt.setCameraState(s);
    REQUIRE(rt.cameraState().x == 10.f);
    REQUIRE(rt.cameraState().yaw == 45.f);
}

TEST_CASE("D.2 Fly-camera: setCameraSpeed updates speed", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    rt.setCameraSpeed(20.f);
    REQUIRE(rt.cameraState().speed == 20.f);
}

TEST_CASE("D.2 Fly-camera: setCameraSensitivity updates sensitivity", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    rt.setCameraSensitivity(0.5f);
    REQUIRE(rt.cameraState().sensitivity == 0.5f);
}

TEST_CASE("D.2 Fly-camera: moveForward changes position", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.x = 0.f; s.y = 0.f; s.z = 0.f;
    s.yaw = -90.f; s.pitch = 0.f; s.speed = 10.f;
    rt.setCameraState(s);

    CameraInput input;
    input.moveForward = true;
    rt.processCameraInput(input, 1.f);

    // At yaw=-90, pitch=0: forward is approximately (-1, 0, 0) direction
    // Position should change
    REQUIRE(rt.cameraState().x != 0.f);
}

TEST_CASE("D.2 Fly-camera: moveUp increases y position", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.x = 0.f; s.y = 0.f; s.z = 0.f; s.speed = 10.f;
    rt.setCameraState(s);

    CameraInput input;
    input.moveUp = true;
    rt.processCameraInput(input, 1.f);
    REQUIRE(rt.cameraState().y == 10.f);
}

TEST_CASE("D.2 Fly-camera: moveDown decreases y position", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.x = 0.f; s.y = 10.f; s.z = 0.f; s.speed = 5.f;
    rt.setCameraState(s);

    CameraInput input;
    input.moveDown = true;
    rt.processCameraInput(input, 1.f);
    REQUIRE(rt.cameraState().y == 5.f);
}

TEST_CASE("D.2 Fly-camera: mouse look updates yaw and pitch", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.yaw = 0.f; s.pitch = 0.f; s.sensitivity = 1.f;
    rt.setCameraState(s);

    CameraInput input;
    input.mouseButtonHeld = true;
    input.mouseDeltaX = 30.f;
    input.mouseDeltaY = 10.f;
    rt.processCameraInput(input, 0.016f);
    REQUIRE(rt.cameraState().yaw == 30.f);
    REQUIRE(rt.cameraState().pitch == -10.f);
}

TEST_CASE("D.2 Fly-camera: pitch clamped to ±89 degrees", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.pitch = 0.f; s.sensitivity = 1.f;
    rt.setCameraState(s);

    CameraInput input;
    input.mouseButtonHeld = true;
    input.mouseDeltaY = -200.f; // would push pitch > 89
    rt.processCameraInput(input, 0.016f);
    REQUIRE(rt.cameraState().pitch <= 89.f);
    REQUIRE(rt.cameraState().pitch >= -89.f);
}

TEST_CASE("D.2 Fly-camera: mouse look ignored when button not held", "[phase_d][d2][camera]") {
    NovaForgePreviewRuntime rt;
    FlyCameraState s;
    s.yaw = 0.f; s.pitch = 0.f; s.sensitivity = 1.f;
    rt.setCameraState(s);

    CameraInput input;
    input.mouseButtonHeld = false;
    input.mouseDeltaX = 50.f;
    rt.processCameraInput(input, 0.016f);
    REQUIRE(rt.cameraState().yaw == 0.f);
}

// ── Gizmo ─────────────────────────────────────────────────────────────────

TEST_CASE("D.2 Gizmo: default mode is Translate", "[phase_d][d2][gizmo]") {
    NovaForgePreviewRuntime rt;
    REQUIRE(rt.gizmoMode() == GizmoMode::Translate);
}

TEST_CASE("D.2 Gizmo: setGizmoMode changes mode", "[phase_d][d2][gizmo]") {
    NovaForgePreviewRuntime rt;
    rt.setGizmoMode(GizmoMode::Rotate);
    REQUIRE(rt.gizmoMode() == GizmoMode::Rotate);
    rt.setGizmoMode(GizmoMode::Scale);
    REQUIRE(rt.gizmoMode() == GizmoMode::Scale);
}

TEST_CASE("D.2 Gizmo: no selection — gizmo invisible", "[phase_d][d2][gizmo]") {
    NovaForgePreviewRuntime rt;
    auto gs = rt.gizmoState();
    REQUIRE_FALSE(gs.visible);
    REQUIRE(gs.entityId == kInvalidEntityId);
}

TEST_CASE("D.2 Gizmo: selected entity — gizmo visible at entity position", "[phase_d][d2][gizmo]") {
    NovaForgePreviewRuntime rt;
    EntityId id = rt.world().createEntity("Cube");
    rt.world().setPosition(id, {3.f, 7.f, -2.f});
    rt.world().selectEntity(id);

    auto gs = rt.gizmoState();
    REQUIRE(gs.visible);
    REQUIRE(gs.entityId == id);
    REQUIRE(gs.position.x == 3.f);
    REQUIRE(gs.position.y == 7.f);
    REQUIRE(gs.position.z == -2.f);
}

TEST_CASE("D.2 Gizmo: gizmo mode preserved in state", "[phase_d][d2][gizmo]") {
    NovaForgePreviewRuntime rt;
    rt.setGizmoMode(GizmoMode::Scale);
    EntityId id = rt.world().createEntity("E");
    rt.world().selectEntity(id);
    REQUIRE(rt.gizmoState().mode == GizmoMode::Scale);
}

// ── Inspector data ─────────────────────────────────────────────────────────

TEST_CASE("D.2 Inspector: no selection returns empty properties", "[phase_d][d2][inspector]") {
    NovaForgePreviewRuntime rt;
    auto props = rt.selectedEntityProperties();
    REQUIRE(props.empty());
}

TEST_CASE("D.2 Inspector: selected entity has name property", "[phase_d][d2][inspector]") {
    NovaForgePreviewRuntime rt;
    EntityId id = rt.world().createEntity("Spaceship");
    rt.world().selectEntity(id);
    auto props = rt.selectedEntityProperties();

    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "name"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "Spaceship");
}

TEST_CASE("D.2 Inspector: selected entity has position properties", "[phase_d][d2][inspector]") {
    NovaForgePreviewRuntime rt;
    EntityId id = rt.world().createEntity("E");
    rt.world().setPosition(id, {1.f, 2.f, 3.f});
    rt.world().selectEntity(id);
    auto props = rt.selectedEntityProperties();

    auto find = [&](const std::string& key) {
        return std::find_if(props.begin(), props.end(),
            [&](const auto& p){ return p.first == key; });
    };
    REQUIRE(find("position.x") != props.end());
    REQUIRE(find("position.y") != props.end());
    REQUIRE(find("position.z") != props.end());
}

TEST_CASE("D.2 Inspector: selected entity has mesh and material properties", "[phase_d][d2][inspector]") {
    NovaForgePreviewRuntime rt;
    EntityId id = rt.world().createEntity("E");
    rt.world().setMesh(id, "mesh/asteroid.nfm");
    rt.world().setMaterial(id, "mat/rock.nfmat");
    rt.world().selectEntity(id);
    auto props = rt.selectedEntityProperties();

    auto find = [&](const std::string& key) -> std::string {
        auto it = std::find_if(props.begin(), props.end(),
            [&](const auto& p){ return p.first == key; });
        return it != props.end() ? it->second : "";
    };
    REQUIRE(find("mesh")     == "mesh/asteroid.nfm");
    REQUIRE(find("material") == "mat/rock.nfmat");
}

TEST_CASE("D.2 Inspector: 13 properties returned for selected entity", "[phase_d][d2][inspector]") {
    NovaForgePreviewRuntime rt;
    EntityId id = rt.world().createEntity("E");
    rt.world().selectEntity(id);
    REQUIRE(rt.selectedEntityProperties().size() == 13);
}

// ── Hierarchy order ────────────────────────────────────────────────────────

TEST_CASE("D.2 Hierarchy: empty world returns empty order", "[phase_d][d2][hierarchy]") {
    NovaForgePreviewRuntime rt;
    REQUIRE(rt.hierarchyOrder().empty());
}

TEST_CASE("D.2 Hierarchy: root-only entities returned in order", "[phase_d][d2][hierarchy]") {
    NovaForgePreviewRuntime rt;
    EntityId a = rt.world().createEntity("A");
    EntityId b = rt.world().createEntity("B");
    auto order = rt.hierarchyOrder();
    REQUIRE(order.size() == 2);
    bool foundA = std::find(order.begin(), order.end(), a) != order.end();
    bool foundB = std::find(order.begin(), order.end(), b) != order.end();
    REQUIRE(foundA);
    REQUIRE(foundB);
}

TEST_CASE("D.2 Hierarchy: parent appears before its children", "[phase_d][d2][hierarchy]") {
    NovaForgePreviewRuntime rt;
    EntityId root  = rt.world().createEntity("Root");
    EntityId child = rt.world().createEntity("Child", root);
    EntityId grand = rt.world().createEntity("Grandchild", child);

    auto order = rt.hierarchyOrder();
    REQUIRE(order.size() == 3);

    auto idx = [&](EntityId id) {
        return std::distance(order.begin(), std::find(order.begin(), order.end(), id));
    };
    REQUIRE(idx(root) < idx(child));
    REQUIRE(idx(child) < idx(grand));
}

TEST_CASE("D.2 Hierarchy: multiple roots with children maintain parent-before-child", "[phase_d][d2][hierarchy]") {
    NovaForgePreviewRuntime rt;
    EntityId r1 = rt.world().createEntity("Root1");
    EntityId r2 = rt.world().createEntity("Root2");
    EntityId c1 = rt.world().createEntity("Child1", r1);
    EntityId c2 = rt.world().createEntity("Child2", r2);

    auto order = rt.hierarchyOrder();
    REQUIRE(order.size() == 4);

    auto idx = [&](EntityId id) {
        return std::distance(order.begin(), std::find(order.begin(), order.end(), id));
    };
    REQUIRE(idx(r1) < idx(c1));
    REQUIRE(idx(r2) < idx(c2));
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.2 — SceneEditorTool delegates provideScene() to attached runtime
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.2 SceneEditorTool: no provider — provideScene returns stub state", "[phase_d][d2][tool]") {
    NF::SceneEditorTool tool;
    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    // Stub: hasContent depends on entityCount (0 by default)
    REQUIRE_FALSE(state.hasContent);
}

TEST_CASE("D.2 SceneEditorTool: attachSceneProvider returns provider pointer", "[phase_d][d2][tool]") {
    NF::SceneEditorTool tool;
    NovaForgePreviewRuntime rt;
    tool.attachSceneProvider(&rt);
    REQUIRE(tool.sceneProvider() == &rt);
}

TEST_CASE("D.2 SceneEditorTool: provideScene delegates to attached runtime when running", "[phase_d][d2][tool]") {
    NF::SceneEditorTool tool;
    NovaForgePreviewRuntime rt;
    rt.start();
    rt.world().createEntity("TestEntity");
    tool.attachSceneProvider(&rt);

    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE(state.hasContent);
    REQUIRE(state.entityCount == 1);
}

TEST_CASE("D.2 SceneEditorTool: detach provider reverts to stub", "[phase_d][d2][tool]") {
    NF::SceneEditorTool tool;
    NovaForgePreviewRuntime rt;
    rt.start();
    tool.attachSceneProvider(&rt);
    tool.attachSceneProvider(nullptr);
    REQUIRE(tool.sceneProvider() == nullptr);

    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE_FALSE(state.hasContent);
}

TEST_CASE("D.2 SceneEditorTool: provideScene entityCount from runtime world", "[phase_d][d2][tool]") {
    NF::SceneEditorTool tool;
    NovaForgePreviewRuntime rt;
    rt.start();
    rt.world().createEntity("A");
    rt.world().createEntity("B");
    rt.world().createEntity("C");
    tool.attachSceneProvider(&rt);

    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE(state.entityCount == 3);
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.3 — NovaForgeAssetPreview
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.3 AssetPreview: starts with no asset", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    REQUIRE_FALSE(preview.hasAsset());
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.3 AssetPreview: provideScene hasContent=false with no asset", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    NF::ViewportSlot slot;
    auto state = preview.provideScene(1, slot);
    REQUIRE_FALSE(state.hasContent);
}

TEST_CASE("D.3 AssetPreview: bindAsset sets hasAsset=true", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/Ships/Raider.nfa";
    desc.meshTag   = "mesh/raider";
    desc.materialTag = "mat/hull";
    preview.bindAsset(desc);
    REQUIRE(preview.hasAsset());
}

TEST_CASE("D.3 AssetPreview: bindAsset not dirty after bind", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.3 AssetPreview: provideScene hasContent=true after bind", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    NF::ViewportSlot slot;
    auto state = preview.provideScene(1, slot);
    REQUIRE(state.hasContent);
}

TEST_CASE("D.3 AssetPreview: provideScene entityCount=1 after bind", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    NF::ViewportSlot slot;
    REQUIRE(preview.provideScene(1, slot).entityCount == 1);
}

TEST_CASE("D.3 AssetPreview: clearAsset removes asset", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    preview.clearAsset();
    REQUIRE_FALSE(preview.hasAsset());
}

TEST_CASE("D.3 AssetPreview: setMeshTag marks dirty", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    REQUIRE(preview.setMeshTag("mesh/newmesh"));
    REQUIRE(preview.isDirty());
    REQUIRE(preview.descriptor().meshTag == "mesh/newmesh");
}

TEST_CASE("D.3 AssetPreview: setMaterialTag marks dirty", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    REQUIRE(preview.setMaterialTag("mat/chrome"));
    REQUIRE(preview.isDirty());
    REQUIRE(preview.descriptor().materialTag == "mat/chrome");
}

TEST_CASE("D.3 AssetPreview: setAttachmentTag marks dirty", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    REQUIRE(preview.setAttachmentTag("cannon_slot_01"));
    REQUIRE(preview.isDirty());
    REQUIRE(preview.descriptor().attachmentTag == "cannon_slot_01");
}

TEST_CASE("D.3 AssetPreview: setTransform marks dirty", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    PreviewTransform t;
    t.position = {5.f, 0.f, 0.f};
    REQUIRE(preview.setTransform(t));
    REQUIRE(preview.isDirty());
    REQUIRE(preview.descriptor().transform.position.x == 5.f);
}

TEST_CASE("D.3 AssetPreview: setMeshTag on no-asset returns false", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    REQUIRE_FALSE(preview.setMeshTag("mesh/x"));
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.3 AssetPreview: apply() clears dirty", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    preview.setMeshTag("mesh/newmesh");
    REQUIRE(preview.apply());
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.3 AssetPreview: revert() restores pre-edit descriptor", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    desc.meshTag   = "mesh/original";
    preview.bindAsset(desc);

    preview.setMeshTag("mesh/changed");
    REQUIRE(preview.descriptor().meshTag == "mesh/changed");

    REQUIRE(preview.revert());
    REQUIRE(preview.descriptor().meshTag == "mesh/original");
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.3 AssetPreview: apply then revert keeps applied state", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    desc.meshTag   = "mesh/original";
    preview.bindAsset(desc);

    preview.setMeshTag("mesh/v2");
    preview.apply();

    preview.setMeshTag("mesh/v3");
    preview.revert();

    REQUIRE(preview.descriptor().meshTag == "mesh/v2");
}

TEST_CASE("D.3 AssetPreview: revert on no-asset returns false", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    REQUIRE_FALSE(preview.revert());
}

TEST_CASE("D.3 AssetPreview: apply on no-asset returns false", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    REQUIRE_FALSE(preview.apply());
}

TEST_CASE("D.3 AssetPreview: properties() returns 20 entries", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    REQUIRE(preview.properties().size() == 20);
}

TEST_CASE("D.3 AssetPreview: properties() contains assetPath", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/Ships/Raider.nfa";
    preview.bindAsset(desc);
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "assetPath"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "Assets/Ships/Raider.nfa");
}

TEST_CASE("D.3 AssetPreview: properties() contains meshTag and materialTag", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath   = "Assets/X.nfa";
    desc.meshTag     = "mesh/fighter";
    desc.materialTag = "mat/metal";
    preview.bindAsset(desc);

    auto props = preview.properties();
    auto find = [&](const std::string& key) {
        return std::find_if(props.begin(), props.end(),
            [&](const auto& p){ return p.first == key; });
    };
    REQUIRE(find("meshTag")     != props.end());
    REQUIRE(find("materialTag") != props.end());
    REQUIRE(find("meshTag")->second     == "mesh/fighter");
    REQUIRE(find("materialTag")->second == "mat/metal");
}

TEST_CASE("D.3 AssetPreview: preview world has one entity after bind", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    REQUIRE(preview.previewWorld().entityCount() == 1);
}

TEST_CASE("D.3 AssetPreview: preview world cleared after clearAsset", "[phase_d][d3][asset_preview]") {
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    preview.clearAsset();
    REQUIRE(preview.previewWorld().entityCount() == 0);
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.3 — AssetEditorTool delegates provideScene() to attached preview
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.3 AssetEditorTool: no provider — provideScene hasContent=false", "[phase_d][d3][tool]") {
    NF::AssetEditorTool tool;
    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE_FALSE(state.hasContent);
}

TEST_CASE("D.3 AssetEditorTool: attachAssetPreviewProvider stores pointer", "[phase_d][d3][tool]") {
    NF::AssetEditorTool tool;
    NovaForgeAssetPreview preview;
    tool.attachAssetPreviewProvider(&preview);
    REQUIRE(tool.assetPreviewProvider() == &preview);
}

TEST_CASE("D.3 AssetEditorTool: provideScene delegates when asset bound", "[phase_d][d3][tool]") {
    NF::AssetEditorTool tool;
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/Ships/Destroyer.nfa";
    preview.bindAsset(desc);
    tool.attachAssetPreviewProvider(&preview);

    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE(state.hasContent);
    REQUIRE(state.entityCount == 1);
}

TEST_CASE("D.3 AssetEditorTool: detach provider reverts to stub", "[phase_d][d3][tool]") {
    NF::AssetEditorTool tool;
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    tool.attachAssetPreviewProvider(&preview);
    tool.attachAssetPreviewProvider(nullptr);

    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE_FALSE(state.hasContent);
}

TEST_CASE("D.3 AssetEditorTool: provideScene not-content when asset cleared", "[phase_d][d3][tool]") {
    NF::AssetEditorTool tool;
    NovaForgeAssetPreview preview;
    AssetPreviewDescriptor desc;
    desc.assetPath = "Assets/X.nfa";
    preview.bindAsset(desc);
    tool.attachAssetPreviewProvider(&preview);
    preview.clearAsset();

    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE_FALSE(state.hasContent);
}
