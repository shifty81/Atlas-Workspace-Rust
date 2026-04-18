#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AdvancedViewports.h"

using namespace NF;

// ── NFRenderViewport base ─────────────────────────────────────────

TEST_CASE("ViewportRenderMode names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Wireframe)) == "Wireframe");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Solid))     == "Solid");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Lit))       == "Lit");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Textured))  == "Textured");
    REQUIRE(std::string(viewportRenderModeName(ViewportRenderMode::Unlit))     == "Unlit");
}

TEST_CASE("ViewportGizmoMode names cover all 4 values", "[Editor][M1C]") {
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::None))      == "None");
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::Translate)) == "Translate");
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::Rotate))    == "Rotate");
    REQUIRE(std::string(viewportGizmoModeName(ViewportGizmoMode::Scale))     == "Scale");
}

TEST_CASE("ViewportCameraMode names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::FPS))        == "FPS");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::Orbit))      == "Orbit");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::Flythrough)) == "Flythrough");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::TopDown))    == "TopDown");
    REQUIRE(std::string(viewportCameraModeName(ViewportCameraMode::Cinematic))  == "Cinematic");
}

TEST_CASE("NFRenderViewport default construction and accessors", "[Editor][M1C]") {
    NFRenderViewport vp("TestViewport", 800, 600);
    REQUIRE(vp.name()       == "TestViewport");
    REQUIRE(vp.width()      == 800);
    REQUIRE(vp.height()     == 600);
    REQUIRE(vp.cameraMode() == ViewportCameraMode::FPS);
    REQUIRE(vp.renderMode() == ViewportRenderMode::Lit);
    REQUIRE(vp.gizmoMode()  == ViewportGizmoMode::None);
    REQUIRE(vp.fov()        == Catch::Approx(60.0f));
    REQUIRE(vp.nearPlane()  == Catch::Approx(0.1f));
    REQUIRE(vp.farPlane()   == Catch::Approx(1000.0f));
    REQUIRE(vp.gridVisible());
    REQUIRE_FALSE(vp.isGizmoActive());
    REQUIRE(vp.frameCount() == 0);
}

TEST_CASE("NFRenderViewport setters and camera control", "[Editor][M1C]") {
    NFRenderViewport vp("Cam");
    vp.setCameraMode(ViewportCameraMode::Orbit);
    vp.setRenderMode(ViewportRenderMode::Wireframe);
    vp.setGizmoMode(ViewportGizmoMode::Translate);
    vp.setCameraPosition(1.0f, 2.0f, 3.0f);
    vp.setCameraTarget(4.0f, 5.0f, 6.0f);
    vp.setCameraFOV(90.0f);
    vp.setNearFar(0.5f, 2000.0f);
    vp.setMoveSpeed(10.0f);
    vp.setLookSensitivity(0.3f);
    vp.setSprintMultiplier(3.0f);
    vp.setGridVisible(false);
    vp.setGridSize(2.0f);

    REQUIRE(vp.cameraMode() == ViewportCameraMode::Orbit);
    REQUIRE(vp.renderMode() == ViewportRenderMode::Wireframe);
    REQUIRE(vp.gizmoMode()  == ViewportGizmoMode::Translate);
    REQUIRE(vp.isGizmoActive());
    REQUIRE(vp.camX() == Catch::Approx(1.0f));
    REQUIRE(vp.camY() == Catch::Approx(2.0f));
    REQUIRE(vp.camZ() == Catch::Approx(3.0f));
    REQUIRE(vp.tgtX() == Catch::Approx(4.0f));
    REQUIRE(vp.fov()  == Catch::Approx(90.0f));
    REQUIRE(vp.nearPlane() == Catch::Approx(0.5f));
    REQUIRE(vp.farPlane()  == Catch::Approx(2000.0f));
    REQUIRE(vp.moveSpeed() == Catch::Approx(10.0f));
    REQUIRE(vp.lookSensitivity() == Catch::Approx(0.3f));
    REQUIRE(vp.sprintMultiplier() == Catch::Approx(3.0f));
    REQUIRE_FALSE(vp.gridVisible());
    REQUIRE(vp.gridSize() == Catch::Approx(2.0f));
}

TEST_CASE("NFRenderViewport aspect ratio and resize", "[Editor][M1C]") {
    NFRenderViewport vp("AR", 1920, 1080);
    REQUIRE(vp.aspectRatio() == Catch::Approx(1920.0f / 1080.0f));
    vp.resize(800, 600);
    REQUIRE(vp.width()  == 800);
    REQUIRE(vp.height() == 600);
    REQUIRE(vp.aspectRatio() == Catch::Approx(800.0f / 600.0f));
}

TEST_CASE("NFRenderViewport frame tick counter", "[Editor][M1C]") {
    NFRenderViewport vp("Tick");
    REQUIRE(vp.frameCount() == 0);
    vp.tick(); vp.tick(); vp.tick();
    REQUIRE(vp.frameCount() == 3);
}

// ── MeshViewerPanel ───────────────────────────────────────────────

TEST_CASE("MeshDisplayMode names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Wireframe)) == "Wireframe");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Solid))     == "Solid");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Lit))       == "Lit");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::UV))        == "UV");
    REQUIRE(std::string(meshDisplayModeName(MeshDisplayMode::Normals))   == "Normals");
}

TEST_CASE("MeshViewerPanel inherits NFRenderViewport and defaults to Orbit cam", "[Editor][M1C]") {
    MeshViewerPanel panel;
    REQUIRE(panel.name()       == "MeshViewer");
    REQUIRE(panel.cameraMode() == ViewportCameraMode::Orbit);
    REQUIRE(panel.displayMode() == MeshDisplayMode::Lit);
    REQUIRE(panel.meshCount()  == 0);
    REQUIRE(panel.activeMesh().empty());
}

TEST_CASE("MeshViewerPanel add/remove/find meshes", "[Editor][M1C]") {
    MeshViewerPanel panel;
    MeshViewerAsset a; a.name = "Cube"; a.vertexCount = 8; a.triangleCount = 12; a.loaded = true;
    MeshViewerAsset b; b.name = "Sphere"; b.vertexCount = 2000; b.triangleCount = 150000; b.loaded = true;
    MeshViewerAsset dup; dup.name = "Cube";

    REQUIRE(panel.addMesh(a));
    REQUIRE(panel.addMesh(b));
    REQUIRE_FALSE(panel.addMesh(dup));
    REQUIRE(panel.meshCount() == 2);

    REQUIRE(panel.findMesh("Cube") != nullptr);
    REQUIRE(panel.findMesh("Missing") == nullptr);

    panel.setActiveMesh("Cube");
    REQUIRE(panel.activeMesh() == "Cube");

    REQUIRE(panel.removeMesh("Cube"));
    REQUIRE(panel.activeMesh().empty());
    REQUIRE(panel.meshCount() == 1);
}

TEST_CASE("MeshViewerPanel aggregate counts", "[Editor][M1C]") {
    MeshViewerPanel panel;
    MeshViewerAsset a; a.name = "Low"; a.triangleCount = 100; a.loaded = true; a.lodCount = 3;
    MeshViewerAsset b; b.name = "High"; b.triangleCount = 200000; b.loaded = false; b.lodCount = 1;
    MeshViewerAsset c; c.name = "Complete"; c.triangleCount = 50; c.loaded = true; c.hasNormals = true; c.hasUVs = true; c.lodCount = 2;

    REQUIRE(panel.addMesh(a));
    REQUIRE(panel.addMesh(b));
    REQUIRE(panel.addMesh(c));

    REQUIRE(panel.highPolyCount()  == 1); // High
    REQUIRE(panel.loadedCount()    == 2); // Low + Complete
    REQUIRE(panel.multiLODCount()  == 2); // Low + Complete
    REQUIRE(panel.completeCount()  == 2); // Low + Complete (both have normals, UVs, loaded)
}

TEST_CASE("MeshViewerPanel MAX_MESHES limit enforced", "[Editor][M1C]") {
    MeshViewerPanel panel;
    for (size_t i = 0; i < MeshViewerPanel::MAX_MESHES; ++i) {
        MeshViewerAsset m; m.name = "M" + std::to_string(i);
        REQUIRE(panel.addMesh(m));
    }
    MeshViewerAsset overflow; overflow.name = "Overflow";
    REQUIRE_FALSE(panel.addMesh(overflow));
    REQUIRE(panel.meshCount() == MeshViewerPanel::MAX_MESHES);
}

// ── MaterialEditorPanel ───────────────────────────────────────────

TEST_CASE("MaterialPreviewShape names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Sphere))   == "Sphere");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Cube))     == "Cube");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Cylinder)) == "Cylinder");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Plane))    == "Plane");
    REQUIRE(std::string(materialPreviewShapeName(MaterialPreviewShape::Custom))   == "Custom");
}

TEST_CASE("MaterialEditorPanel inherits NFRenderViewport with live preview", "[Editor][M1C]") {
    MaterialEditorPanel panel;
    REQUIRE(panel.name() == "MaterialEditor");
    REQUIRE(panel.previewShape()  == MaterialPreviewShape::Sphere);
    REQUIRE(panel.autoRecompile());
    REQUIRE(panel.livePreview());
    REQUIRE(panel.materialCount() == 0);
}

TEST_CASE("MaterialEditorPanel add/remove/find materials", "[Editor][M1C]") {
    MaterialEditorPanel panel;
    MaterialAsset a("MatA"); a.setDirty(true);
    MaterialAsset b("MatB");

    REQUIRE(panel.addMaterial(a));
    REQUIRE(panel.addMaterial(b));
    REQUIRE_FALSE(panel.addMaterial(MaterialAsset("MatA")));
    REQUIRE(panel.materialCount() == 2);

    panel.setActiveMaterial("MatA");
    REQUIRE(panel.activeMaterial() == "MatA");

    REQUIRE(panel.removeMaterial("MatA"));
    REQUIRE(panel.activeMaterial().empty());
    REQUIRE(panel.dirtyCount() == 0);
}

TEST_CASE("MaterialEditorPanel PBR count and preview shape", "[Editor][M1C]") {
    MaterialEditorPanel panel;
    MaterialAsset a("A"); // default is PBR
    MaterialAsset b("B"); b.setShadingModel(MaterialShadingModel::Toon);
    MaterialAsset c("C"); // PBR

    REQUIRE(panel.addMaterial(a));
    REQUIRE(panel.addMaterial(b));
    REQUIRE(panel.addMaterial(c));

    REQUIRE(panel.pbrCount() == 2);
    panel.setPreviewShape(MaterialPreviewShape::Cube);
    REQUIRE(panel.previewShape() == MaterialPreviewShape::Cube);
}

// ── SkeletalEditorPanel ───────────────────────────────────────────

TEST_CASE("BoneDisplayMode names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Lines))      == "Lines");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Octahedral)) == "Octahedral");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Stick))      == "Stick");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::BBone))      == "BBone");
    REQUIRE(std::string(boneDisplayModeName(BoneDisplayMode::Envelope))   == "Envelope");
}

TEST_CASE("WeightPaintMode names cover all 6 values", "[Editor][M1C]") {
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Off))       == "Off");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Add))       == "Add");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Subtract))  == "Subtract");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Smooth))    == "Smooth");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Replace))   == "Replace");
    REQUIRE(std::string(weightPaintModeName(WeightPaintMode::Blur))      == "Blur");
}

TEST_CASE("SkeletalAsset bone management", "[Editor][M1C]") {
    SkeletalAsset skel("Humanoid");
    SkeletalBone root; root.name = "Root"; root.parentIndex = -1;
    SkeletalBone spine; spine.name = "Spine"; spine.parentIndex = 0;
    SkeletalBone dup; dup.name = "Root";

    REQUIRE(skel.addBone(root));
    REQUIRE(skel.addBone(spine));
    REQUIRE_FALSE(skel.addBone(dup));
    REQUIRE(skel.boneCount() == 2);
    REQUIRE(skel.rootCount() == 1);

    REQUIRE(skel.findBone("Root") != nullptr);
    REQUIRE(skel.findBone("Root")->isRoot());
    REQUIRE_FALSE(skel.findBone("Spine")->isRoot());

    REQUIRE(skel.removeBone("Root"));
    REQUIRE(skel.boneCount() == 1);
}

TEST_CASE("SkeletalAsset complexity threshold", "[Editor][M1C]") {
    SkeletalAsset skel("Test");
    for (int i = 0; i < 49; ++i) {
        SkeletalBone b; b.name = "B" + std::to_string(i);
        skel.addBone(b);
    }
    REQUIRE_FALSE(skel.isComplex());

    SkeletalBone b50; b50.name = "B49";
    skel.addBone(b50);
    REQUIRE(skel.isComplex());
}

TEST_CASE("SkeletalEditorPanel add/remove/find skeletons and weight paint", "[Editor][M1C]") {
    SkeletalEditorPanel panel;
    REQUIRE(panel.name() == "SkeletalEditor");
    REQUIRE(panel.boneDisplayMode() == BoneDisplayMode::Octahedral);
    REQUIRE(panel.weightPaintMode() == WeightPaintMode::Off);
    REQUIRE_FALSE(panel.isPainting());

    SkeletalAsset a("Humanoid"); a.dirty = true; a.loaded = true;
    SkeletalAsset b("Animal"); b.loaded = true;
    for (int i = 0; i < 55; ++i) { SkeletalBone bone; bone.name = "B" + std::to_string(i); b.addBone(bone); }

    REQUIRE(panel.addSkeleton(a));
    REQUIRE(panel.addSkeleton(b));
    REQUIRE_FALSE(panel.addSkeleton(SkeletalAsset("Humanoid")));
    REQUIRE(panel.skeletonCount() == 2);

    REQUIRE(panel.dirtyCount()   == 1);
    REQUIRE(panel.loadedCount()  == 2);
    REQUIRE(panel.complexCount() == 1);

    panel.setWeightPaintMode(WeightPaintMode::Add);
    REQUIRE(panel.isPainting());
    panel.setBrushRadius(10.0f);
    REQUIRE(panel.brushRadius() == Catch::Approx(10.0f));

    panel.setActiveSkeleton("Humanoid");
    panel.removeSkeleton("Humanoid");
    REQUIRE(panel.activeSkeleton().empty());
}

TEST_CASE("SkeletalEditorPanel openAsset sets active", "[Editor][M1C]") {
    SkeletalEditorPanel panel;
    SkeletalAsset skel("CharacterRig_01");
    REQUIRE(panel.addSkeleton(skel));
    panel.openAsset("CharacterRig_01");
    REQUIRE(panel.activeSkeleton() == "CharacterRig_01");
}

// ── AnimationEditorPanel ──────────────────────────────────────────

TEST_CASE("AnimPlaybackState names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Stopped)) == "Stopped");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Playing)) == "Playing");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Paused))  == "Paused");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Looping)) == "Looping");
    REQUIRE(std::string(animPlaybackStateName(AnimPlaybackState::Reverse)) == "Reverse");
}

TEST_CASE("AnimBlendTreeType names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Simple))   == "Simple");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Additive)) == "Additive");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Layered))  == "Layered");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Override)) == "Override");
    REQUIRE(std::string(animBlendTreeTypeName(AnimBlendTreeType::Masked))   == "Masked");
}

TEST_CASE("AnimClipAsset defaults and computed properties", "[Editor][M1C]") {
    AnimClipAsset clip("Walk");
    REQUIRE(clip.name == "Walk");
    REQUIRE(clip.duration == Catch::Approx(0.0f));
    REQUIRE(clip.totalFrames() == 0);
    REQUIRE_FALSE(clip.isLong());
    REQUIRE_FALSE(clip.isDense());
    REQUIRE_FALSE(clip.isReady());

    clip.duration = 12.0f;
    clip.frameRate = 30.0f;
    clip.keyframeCount = 150;
    clip.loaded = true;
    clip.dirty = false;

    REQUIRE(clip.totalFrames() == 360);
    REQUIRE(clip.isLong());
    REQUIRE(clip.isDense());
    REQUIRE(clip.isReady());
}

TEST_CASE("AnimationEditorPanel add/remove/find clips", "[Editor][M1C]") {
    AnimationEditorPanel panel;
    REQUIRE(panel.name() == "AnimationEditor");
    REQUIRE(panel.playbackState() == AnimPlaybackState::Stopped);
    REQUIRE_FALSE(panel.isPlaying());

    AnimClipAsset a("Run"); a.dirty = true; a.looping = true; a.loaded = true;
    AnimClipAsset b("Jump"); b.duration = 15.0f; b.keyframeCount = 200; b.loaded = true;
    AnimClipAsset c("Idle"); c.blendType = AnimBlendTreeType::Additive;

    REQUIRE(panel.addClip(a));
    REQUIRE(panel.addClip(b));
    REQUIRE(panel.addClip(c));
    REQUIRE_FALSE(panel.addClip(AnimClipAsset("Run")));
    REQUIRE(panel.clipCount() == 3);

    REQUIRE(panel.dirtyCount()      == 1);
    REQUIRE(panel.loadedCount()     == 2);
    REQUIRE(panel.loopingClipCount() == 1);
    REQUIRE(panel.longClipCount()    == 1);
    REQUIRE(panel.denseClipCount()   == 1);
    REQUIRE(panel.countByBlendType(AnimBlendTreeType::Simple)   == 2);
    REQUIRE(panel.countByBlendType(AnimBlendTreeType::Additive) == 1);

    panel.setActiveClip("Run");
    REQUIRE(panel.activeClip() == "Run");
    panel.removeClip("Run");
    REQUIRE(panel.activeClip().empty());
}

TEST_CASE("AnimationEditorPanel playback and timeline controls", "[Editor][M1C]") {
    AnimationEditorPanel panel;
    panel.setPlaybackState(AnimPlaybackState::Playing);
    REQUIRE(panel.isPlaying());
    panel.setPlaybackState(AnimPlaybackState::Looping);
    REQUIRE(panel.isPlaying());
    panel.setPlaybackState(AnimPlaybackState::Paused);
    REQUIRE_FALSE(panel.isPlaying());

    panel.setPlaybackSpeed(2.0f);
    panel.setCurrentFrame(42);
    panel.setTimelineZoom(3.0f);
    panel.setOnionSkinning(true);
    panel.setShowBlendTree(true);

    REQUIRE(panel.playbackSpeed()  == Catch::Approx(2.0f));
    REQUIRE(panel.currentFrame()   == 42);
    REQUIRE(panel.timelineZoom()   == Catch::Approx(3.0f));
    REQUIRE(panel.onionSkinning());
    REQUIRE(panel.showBlendTree());
}

TEST_CASE("AnimationEditorPanel openAsset sets active clip", "[Editor][M1C]") {
    AnimationEditorPanel panel;
    AnimClipAsset clip("PlayerRun");
    REQUIRE(panel.addClip(clip));
    panel.openAsset("PlayerRun");
    REQUIRE(panel.activeClip() == "PlayerRun");
}

// ── PrefabEditorPanel ─────────────────────────────────────────────

TEST_CASE("PrefabEditMode names cover all 5 values", "[Editor][M1C]") {
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::View))      == "View");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Place))     == "Place");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Delete))    == "Delete");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Transform)) == "Transform");
    REQUIRE(std::string(prefabEditModeName(PrefabEditMode::Connect))   == "Connect");
}

TEST_CASE("PrefabEditorPanel inherits NFRenderViewport and manages instances", "[Editor][M1C]") {
    PrefabEditorPanel panel;
    REQUIRE(panel.name() == "PrefabEditor");
    REQUIRE(panel.editMode()      == PrefabEditMode::View);
    REQUIRE(panel.snapToGrid());
    REQUIRE(panel.snapSize()      == Catch::Approx(0.5f));
    REQUIRE_FALSE(panel.isolationMode());

    PrefabInstance a; a.name = "Tree01"; a.visible = true; a.overridden = true; a.scale = 2.0f;
    PrefabInstance b; b.name = "Rock01"; b.locked = true;
    PrefabInstance c; c.name = "Bush01"; c.visible = false;

    REQUIRE(panel.addInstance(a));
    REQUIRE(panel.addInstance(b));
    REQUIRE(panel.addInstance(c));
    REQUIRE_FALSE(panel.addInstance(PrefabInstance{.name = "Tree01"}));
    REQUIRE(panel.instanceCount() == 3);

    REQUIRE(panel.visibleCount()    == 2);
    REQUIRE(panel.lockedCount()     == 1);
    REQUIRE(panel.overriddenCount() == 1);
    REQUIRE(panel.scaledCount()     == 1);

    panel.setActiveInstance("Tree01");
    REQUIRE(panel.activeInstance() == "Tree01");
    panel.removeInstance("Tree01");
    REQUIRE(panel.activeInstance().empty());

    panel.setEditMode(PrefabEditMode::Place);
    panel.setSnapSize(1.0f);
    panel.setIsolationMode(true);
    REQUIRE(panel.editMode()      == PrefabEditMode::Place);
    REQUIRE(panel.snapSize()      == Catch::Approx(1.0f));
    REQUIRE(panel.isolationMode());
}

TEST_CASE("PrefabEditorPanel MAX_INSTANCES limit enforced", "[Editor][M1C]") {
    PrefabEditorPanel panel;
    for (size_t i = 0; i < PrefabEditorPanel::MAX_INSTANCES; ++i) {
        PrefabInstance inst; inst.name = "I" + std::to_string(i);
        REQUIRE(panel.addInstance(inst));
    }
    PrefabInstance overflow; overflow.name = "Overflow";
    REQUIRE_FALSE(panel.addInstance(overflow));
    REQUIRE(panel.instanceCount() == PrefabEditorPanel::MAX_INSTANCES);
}

// ── Cross-editor NFRenderViewport integration ─────────────────────

TEST_CASE("All M1-C editors inherit NFRenderViewport with gizmo and render mode", "[Editor][M1C]") {
    MeshViewerPanel mesh;
    MaterialEditorPanel mat;
    SkeletalEditorPanel skel;
    AnimationEditorPanel anim;
    PrefabEditorPanel pref;

    // All share the NFRenderViewport interface
    mesh.setGizmoMode(ViewportGizmoMode::Translate);
    mat.setGizmoMode(ViewportGizmoMode::Rotate);
    skel.setGizmoMode(ViewportGizmoMode::Scale);
    anim.setRenderMode(ViewportRenderMode::Wireframe);
    pref.setRenderMode(ViewportRenderMode::Unlit);

    REQUIRE(mesh.isGizmoActive());
    REQUIRE(mat.isGizmoActive());
    REQUIRE(skel.isGizmoActive());
    REQUIRE(anim.renderMode() == ViewportRenderMode::Wireframe);
    REQUIRE(pref.renderMode() == ViewportRenderMode::Unlit);

    // All support frame ticking
    mesh.tick(); mesh.tick();
    REQUIRE(mesh.frameCount() == 2);
    mat.tick();
    REQUIRE(mat.frameCount() == 1);
}
