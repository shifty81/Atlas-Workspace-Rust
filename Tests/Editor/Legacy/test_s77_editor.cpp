#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AdvancedViewports.h"

using namespace NF;

// ── S77: AdvancedViewports ───────────────────────────────────────

TEST_CASE("ViewportRenderMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Wireframe)) == "Wireframe");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Solid))     == "Solid");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Lit))       == "Lit");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Textured))  == "Textured");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Unlit))     == "Unlit");
}

TEST_CASE("ViewportGizmoMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::None))      == "None");
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::Translate)) == "Translate");
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::Rotate))    == "Rotate");
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::Scale))     == "Scale");
}

TEST_CASE("ViewportCameraMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::FPS))        == "FPS");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::Orbit))      == "Orbit");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::Flythrough)) == "Flythrough");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::TopDown))    == "TopDown");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::Cinematic))  == "Cinematic");
}

TEST_CASE("NFRenderViewport default dimensions are 1280x720", "[Editor][S77]") {
    NFRenderViewport vp("Scene");
    REQUIRE(vp.width()  == 1280);
    REQUIRE(vp.height() == 720);
    REQUIRE(vp.name()   == "Scene");
}

TEST_CASE("NFRenderViewport resize updates dimensions", "[Editor][S77]") {
    NFRenderViewport vp("Scene");
    vp.resize(1920, 1080);
    REQUIRE(vp.width()  == 1920);
    REQUIRE(vp.height() == 1080);
}

TEST_CASE("NFRenderViewport aspectRatio computed correctly", "[Editor][S77]") {
    NFRenderViewport vp("Scene", 1280, 720);
    float ratio = vp.aspectRatio();
    REQUIRE(ratio > 1.77f);
    REQUIRE(ratio < 1.78f);
}

TEST_CASE("NFRenderViewport default camera and render mode", "[Editor][S77]") {
    NFRenderViewport vp("V");
    REQUIRE(vp.cameraMode() == ViewportCameraMode::FPS);
    REQUIRE(vp.renderMode() == ViewportRenderMode::Lit);
    REQUIRE(vp.gizmoMode()  == ViewportGizmoMode::None);
    REQUIRE_FALSE(vp.isGizmoActive());
}

TEST_CASE("NFRenderViewport setCameraMode updates mode", "[Editor][S77]") {
    NFRenderViewport vp("V");
    vp.setCameraMode(ViewportCameraMode::Orbit);
    REQUIRE(vp.cameraMode() == ViewportCameraMode::Orbit);
}

TEST_CASE("NFRenderViewport setGizmoMode updates and isGizmoActive", "[Editor][S77]") {
    NFRenderViewport vp("V");
    vp.setGizmoMode(ViewportGizmoMode::Translate);
    REQUIRE(vp.isGizmoActive());
    vp.setGizmoMode(ViewportGizmoMode::None);
    REQUIRE_FALSE(vp.isGizmoActive());
}

TEST_CASE("NFRenderViewport setCameraPosition and Target", "[Editor][S77]") {
    NFRenderViewport vp("V");
    vp.setCameraPosition(1.f, 2.f, 3.f);
    REQUIRE(vp.camX() == 1.f);
    REQUIRE(vp.camY() == 2.f);
    REQUIRE(vp.camZ() == 3.f);
    vp.setCameraTarget(4.f, 5.f, 6.f);
    REQUIRE(vp.tgtX() == 4.f);
    REQUIRE(vp.tgtY() == 5.f);
    REQUIRE(vp.tgtZ() == 6.f);
}

TEST_CASE("NFRenderViewport tick increments frameCount", "[Editor][S77]") {
    NFRenderViewport vp("V");
    REQUIRE(vp.frameCount() == 0);
    vp.tick();
    vp.tick();
    REQUIRE(vp.frameCount() == 2);
}

TEST_CASE("NFRenderViewport setGridVisible and gridSize", "[Editor][S77]") {
    NFRenderViewport vp("V");
    REQUIRE(vp.gridVisible());
    vp.setGridVisible(false);
    REQUIRE_FALSE(vp.gridVisible());
    vp.setGridSize(2.5f);
    REQUIRE(vp.gridSize() == 2.5f);
}

TEST_CASE("MeshDisplayMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Wireframe)) == "Wireframe");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Solid))     == "Solid");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Lit))       == "Lit");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::UV))        == "UV");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Normals))   == "Normals");
}

TEST_CASE("MeshViewerAsset predicates work correctly", "[Editor][S77]") {
    MeshViewerAsset mesh;
    mesh.name = "cube";
    mesh.vertexCount = 8;
    mesh.triangleCount = 12;
    mesh.lodCount = 1;
    mesh.hasNormals = true;
    mesh.hasUVs = true;
    mesh.loaded = true;
    REQUIRE_FALSE(mesh.isHighPoly());
    REQUIRE_FALSE(mesh.isMultiLOD());
    REQUIRE(mesh.isComplete());
}

TEST_CASE("MeshViewerAsset isHighPoly for large triangle count", "[Editor][S77]") {
    MeshViewerAsset mesh;
    mesh.name = "hi";
    mesh.triangleCount = 200000;
    REQUIRE(mesh.isHighPoly());
}

TEST_CASE("MeshViewerPanel has Orbit camera by default", "[Editor][S77]") {
    MeshViewerPanel panel;
    REQUIRE(panel.cameraMode() == ViewportCameraMode::Orbit);
    REQUIRE(panel.name() == "MeshViewer");
}

TEST_CASE("MeshViewerPanel addMesh succeeds", "[Editor][S77]") {
    MeshViewerPanel panel;
    MeshViewerAsset mesh;
    mesh.name = "cube";
    REQUIRE(panel.addMesh(mesh));
    REQUIRE(panel.meshCount() == 1);
}

TEST_CASE("MeshViewerPanel addMesh rejects duplicate name", "[Editor][S77]") {
    MeshViewerPanel panel;
    MeshViewerAsset mesh; mesh.name = "cube";
    panel.addMesh(mesh);
    REQUIRE_FALSE(panel.addMesh(mesh));
    REQUIRE(panel.meshCount() == 1);
}

TEST_CASE("MeshViewerPanel removeMesh removes entry", "[Editor][S77]") {
    MeshViewerPanel panel;
    MeshViewerAsset mesh; mesh.name = "cube";
    panel.addMesh(mesh);
    REQUIRE(panel.removeMesh("cube"));
    REQUIRE(panel.meshCount() == 0);
}

TEST_CASE("MeshViewerPanel setActiveMesh updates active", "[Editor][S77]") {
    MeshViewerPanel panel;
    MeshViewerAsset mesh; mesh.name = "sphere";
    panel.addMesh(mesh);
    REQUIRE(panel.setActiveMesh("sphere"));
    REQUIRE(panel.activeMesh() == "sphere");
}

TEST_CASE("MeshViewerPanel loadedCount and completeCount", "[Editor][S77]") {
    MeshViewerPanel panel;
    MeshViewerAsset m1; m1.name = "a"; m1.loaded = true; m1.hasNormals = true; m1.hasUVs = true;
    MeshViewerAsset m2; m2.name = "b"; m2.loaded = false;
    panel.addMesh(m1);
    panel.addMesh(m2);
    REQUIRE(panel.loadedCount() == 1);
    REQUIRE(panel.completeCount() == 1);
}

TEST_CASE("MaterialPreviewShape names are correct", "[Editor][S77]") {
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Sphere))   == "Sphere");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Cube))     == "Cube");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Cylinder)) == "Cylinder");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Plane))    == "Plane");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Custom))   == "Custom");
}

TEST_CASE("MaterialEditorPanel default name is MaterialEditor", "[Editor][S77]") {
    MaterialEditorPanel panel;
    REQUIRE(panel.name() == "MaterialEditor");
}

TEST_CASE("MaterialEditorPanel autoRecompile and livePreview default true", "[Editor][S77]") {
    MaterialEditorPanel panel;
    REQUIRE(panel.autoRecompile());
    REQUIRE(panel.livePreview());
}

TEST_CASE("MaterialEditorPanel setPreviewShape updates shape", "[Editor][S77]") {
    MaterialEditorPanel panel;
    panel.setPreviewShape(MaterialPreviewShape::Cube);
    REQUIRE(panel.previewShape() == MaterialPreviewShape::Cube);
}

TEST_CASE("BoneDisplayMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Lines))      == "Lines");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Octahedral)) == "Octahedral");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Stick))      == "Stick");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::BBone))      == "BBone");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Envelope))   == "Envelope");
}

TEST_CASE("WeightPaintMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Off))      == "Off");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Add))      == "Add");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Subtract)) == "Subtract");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Smooth))   == "Smooth");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Replace))  == "Replace");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Blur))     == "Blur");
}

TEST_CASE("SkeletalBone isRoot true when parentIndex is -1", "[Editor][S77]") {
    SkeletalBone bone;
    bone.name = "Root";
    bone.parentIndex = -1;
    REQUIRE(bone.isRoot());
    bone.parentIndex = 0;
    REQUIRE_FALSE(bone.isRoot());
}

TEST_CASE("SkeletalAsset addBone and boneCount", "[Editor][S77]") {
    SkeletalAsset skel("Hero");
    SkeletalBone root; root.name = "Root"; root.parentIndex = -1;
    REQUIRE(skel.addBone(root));
    REQUIRE(skel.boneCount() == 1);
}

TEST_CASE("SkeletalAsset addBone rejects duplicate name", "[Editor][S77]") {
    SkeletalAsset skel("Hero");
    SkeletalBone root; root.name = "Root";
    skel.addBone(root);
    REQUIRE_FALSE(skel.addBone(root));
}

TEST_CASE("SkeletalAsset rootCount and selectedCount", "[Editor][S77]") {
    SkeletalAsset skel("Hero");
    SkeletalBone root; root.name = "Root"; root.parentIndex = -1; root.selected = true;
    SkeletalBone child; child.name = "Spine"; child.parentIndex = 0; child.selected = false;
    skel.addBone(root);
    skel.addBone(child);
    REQUIRE(skel.rootCount() == 1);
    REQUIRE(skel.selectedCount() == 1);
}

TEST_CASE("SkeletalEditorPanel default name and boneDisplay", "[Editor][S77]") {
    SkeletalEditorPanel panel;
    REQUIRE(panel.name() == "SkeletalEditor");
    REQUIRE(panel.boneDisplayMode() == BoneDisplayMode::Octahedral);
    REQUIRE_FALSE(panel.isPainting());
}

TEST_CASE("SkeletalEditorPanel addSkeleton and setActiveSkeleton", "[Editor][S77]") {
    SkeletalEditorPanel panel;
    SkeletalAsset skel("Hero");
    REQUIRE(panel.addSkeleton(skel));
    REQUIRE(panel.skeletonCount() == 1);
    REQUIRE(panel.setActiveSkeleton("Hero"));
    REQUIRE(panel.activeSkeleton() == "Hero");
}

TEST_CASE("SkeletalEditorPanel setWeightPaintMode enables painting", "[Editor][S77]") {
    SkeletalEditorPanel panel;
    panel.setWeightPaintMode(WeightPaintMode::Add);
    REQUIRE(panel.isPainting());
    REQUIRE(panel.weightPaintMode() == WeightPaintMode::Add);
}

TEST_CASE("AnimPlaybackState names are correct", "[Editor][S77]") {
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Stopped)) == "Stopped");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Playing)) == "Playing");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Paused))  == "Paused");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Looping)) == "Looping");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Reverse)) == "Reverse");
}

TEST_CASE("AnimBlendTreeType names are correct", "[Editor][S77]") {
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Simple))   == "Simple");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Additive)) == "Additive");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Layered))  == "Layered");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Override)) == "Override");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Masked))   == "Masked");
}

TEST_CASE("AnimClipAsset totalFrames computation", "[Editor][S77]") {
    AnimClipAsset clip("Run");
    clip.duration = 2.0f;
    clip.frameRate = 30.0f;
    REQUIRE(clip.totalFrames() == 60);
}

TEST_CASE("AnimClipAsset isLong and isDense predicates", "[Editor][S77]") {
    AnimClipAsset clip("Epic");
    clip.duration = 15.0f;
    clip.keyframeCount = 200;
    REQUIRE(clip.isLong());
    REQUIRE(clip.isDense());
}

TEST_CASE("AnimationEditorPanel addClip and removeClip", "[Editor][S77]") {
    AnimationEditorPanel panel;
    AnimClipAsset clip("Walk");
    REQUIRE(panel.addClip(clip));
    REQUIRE(panel.clipCount() == 1);
    REQUIRE(panel.removeClip("Walk"));
    REQUIRE(panel.clipCount() == 0);
}

TEST_CASE("AnimationEditorPanel setPlaybackState and isPlaying", "[Editor][S77]") {
    AnimationEditorPanel panel;
    REQUIRE_FALSE(panel.isPlaying());
    panel.setPlaybackState(AnimPlaybackState::Playing);
    REQUIRE(panel.isPlaying());
    panel.setPlaybackState(AnimPlaybackState::Looping);
    REQUIRE(panel.isPlaying());
    panel.setPlaybackState(AnimPlaybackState::Paused);
    REQUIRE_FALSE(panel.isPlaying());
}

TEST_CASE("PrefabEditMode names are correct", "[Editor][S77]") {
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::View))      == "View");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Place))     == "Place");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Delete))    == "Delete");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Transform)) == "Transform");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Connect))   == "Connect");
}

TEST_CASE("PrefabInstance isModified and isScaled", "[Editor][S77]") {
    PrefabInstance inst;
    inst.name = "Wall1";
    inst.overridden = true;
    inst.scale = 2.0f;
    REQUIRE(inst.isModified());
    REQUIRE(inst.isScaled());
}

TEST_CASE("PrefabEditorPanel addInstance and instanceCount", "[Editor][S77]") {
    PrefabEditorPanel panel;
    PrefabInstance inst; inst.name = "Wall1"; inst.prefabSource = "Wall";
    REQUIRE(panel.addInstance(inst));
    REQUIRE(panel.instanceCount() == 1);
}

TEST_CASE("PrefabEditorPanel visibleCount and lockedCount", "[Editor][S77]") {
    PrefabEditorPanel panel;
    PrefabInstance a; a.name = "a"; a.visible = true;  a.locked = true;
    PrefabInstance b; b.name = "b"; b.visible = false; b.locked = false;
    PrefabInstance c; c.name = "c"; c.visible = true;  c.locked = false;
    panel.addInstance(a); panel.addInstance(b); panel.addInstance(c);
    REQUIRE(panel.visibleCount() == 2);
    REQUIRE(panel.lockedCount() == 1);
}

TEST_CASE("PrefabEditorPanel snapToGrid defaults to true", "[Editor][S77]") {
    PrefabEditorPanel panel;
    REQUIRE(panel.snapToGrid());
    panel.setSnapToGrid(false);
    REQUIRE_FALSE(panel.snapToGrid());
}
