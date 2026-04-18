// Phase D Completion Tests — D.3 extended (collider/socket/anchor/PCG metadata) + D.4 Material Preview
//
// D.3 collider:
//   - setCollider / setColliderShape / setColliderExtents / setColliderIsTrigger / setColliderTag
//   - collider in properties(); apply()/revert() round-trips
// D.3 sockets:
//   - addSocket / removeSocket / setSocketTransform / socketCount
//   - socket operations require asset bound
// D.3 anchors:
//   - addAnchor / removeAnchor / setAnchorTransform / anchorCount
// D.3 PCG metadata:
//   - setPlacementTag / addGenerationTag / removeGenerationTag
//   - setPCGScaleRange / setPCGDensity / setPCGExclusionGroup
//   - PCG fields in properties()
// D.4 NovaForgeMaterialPreview:
//   - IViewportSceneProvider: provideScene with/without material bound
//   - bindMaterial / clearMaterial / hasMaterial
//   - setPreviewMesh (Sphere/Cube/Plane) reflected in world mesh tag
//   - setShaderTag, setParameter, removeParameter, resetParameterToDefault
//   - resetAllParametersToDefault
//   - apply()/revert() baseline management
//   - properties() returns materialPath, shaderTag, previewMesh, param.* entries
//   - parameterCount()
// D.4 MaterialEditorTool:
//   - attachMaterialPreviewProvider / materialPreviewProvider
//   - provideScene delegates when bound / stub when not bound / stub when detached

#include <catch2/catch_test_macros.hpp>

#include "NovaForge/EditorAdapter/NovaForgeAssetPreview.h"
#include "NovaForge/EditorAdapter/NovaForgeMaterialPreview.h"
#include "NF/Editor/MaterialEditorTool.h"

#include <algorithm>
#include <string>

using namespace NovaForge;

// ── Helper ────────────────────────────────────────────────────────────────────

static AssetPreviewDescriptor makeDesc(const std::string& path = "Assets/X.nfa") {
    AssetPreviewDescriptor d;
    d.assetPath = path;
    d.meshTag   = "mesh/x";
    return d;
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.3 Extended — Collider
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.3 Collider: default shape is Box", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.collider().shape == ColliderShape::Box);
}

TEST_CASE("D.3 Collider: setColliderShape changes shape", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setColliderShape(ColliderShape::Sphere));
    REQUIRE(preview.collider().shape == ColliderShape::Sphere);
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.3 Collider: setColliderExtents updates extents", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setColliderExtents({2.f, 3.f, 4.f}));
    REQUIRE(preview.collider().extents.x == 2.f);
    REQUIRE(preview.collider().extents.y == 3.f);
    REQUIRE(preview.collider().extents.z == 4.f);
}

TEST_CASE("D.3 Collider: setColliderIsTrigger sets trigger flag", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setColliderIsTrigger(true));
    REQUIRE(preview.collider().isTrigger);
}

TEST_CASE("D.3 Collider: setColliderTag sets tag", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setColliderTag("physics.ship_hull"));
    REQUIRE(preview.collider().tag == "physics.ship_hull");
}

TEST_CASE("D.3 Collider: setCollider stores full descriptor", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    ColliderDescriptor c;
    c.shape     = ColliderShape::Capsule;
    c.radius    = 2.5f;
    c.isTrigger = true;
    c.tag       = "physics.capsule";
    REQUIRE(preview.setCollider(c));
    REQUIRE(preview.collider().shape    == ColliderShape::Capsule);
    REQUIRE(preview.collider().radius   == 2.5f);
    REQUIRE(preview.collider().isTrigger);
    REQUIRE(preview.collider().tag      == "physics.capsule");
}

TEST_CASE("D.3 Collider: operations fail when no asset bound", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    REQUIRE_FALSE(preview.setColliderShape(ColliderShape::Sphere));
    REQUIRE_FALSE(preview.setColliderIsTrigger(true));
    REQUIRE_FALSE(preview.setColliderTag("x"));
}

TEST_CASE("D.3 Collider: apply() preserves collider; revert() restores", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.setColliderShape(ColliderShape::Sphere);
    preview.apply();

    preview.setColliderShape(ColliderShape::TriangleMesh);
    REQUIRE(preview.collider().shape == ColliderShape::TriangleMesh);

    preview.revert();
    REQUIRE(preview.collider().shape == ColliderShape::Sphere);
}

TEST_CASE("D.3 Collider: collider shape in properties()", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.setColliderShape(ColliderShape::ConvexHull);
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "collider.shape"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "ConvexHull");
}

TEST_CASE("D.3 Collider: isTrigger in properties()", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.setColliderIsTrigger(true);
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "collider.isTrigger"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "true");
}

TEST_CASE("D.3 Collider: properties() has 20 entries", "[phase_d][d3][collider]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.properties().size() == 20);
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.3 Extended — Sockets
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.3 Socket: addSocket increases count", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    SocketDescriptor s;
    s.name       = "weapon_port_01";
    s.socketType = "weapon";
    REQUIRE(preview.addSocket(s));
    REQUIRE(preview.socketCount() == 1);
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.3 Socket: addSocket fails when no asset", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    SocketDescriptor s;
    s.name = "s1";
    REQUIRE_FALSE(preview.addSocket(s));
}

TEST_CASE("D.3 Socket: removeSocket by name", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    SocketDescriptor s;
    s.name = "engine_01";
    preview.addSocket(s);
    REQUIRE(preview.removeSocket("engine_01"));
    REQUIRE(preview.socketCount() == 0);
}

TEST_CASE("D.3 Socket: removeSocket non-existent returns false", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE_FALSE(preview.removeSocket("missing"));
}

TEST_CASE("D.3 Socket: setSocketTransform updates transform", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    SocketDescriptor s;
    s.name = "dock_01";
    preview.addSocket(s);
    PreviewTransform t;
    t.position = {1.f, 2.f, 3.f};
    REQUIRE(preview.setSocketTransform("dock_01", t));
    REQUIRE(preview.sockets()[0].localTransform.position.x == 1.f);
}

TEST_CASE("D.3 Socket: setSocketTransform fails for unknown socket", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE_FALSE(preview.setSocketTransform("ghost", {}));
}

TEST_CASE("D.3 Socket: multiple sockets tracked", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    SocketDescriptor s1; s1.name = "s1"; s1.socketType = "weapon";
    SocketDescriptor s2; s2.name = "s2"; s2.socketType = "engine";
    SocketDescriptor s3; s3.name = "s3"; s3.socketType = "cargo";
    preview.addSocket(s1);
    preview.addSocket(s2);
    preview.addSocket(s3);
    REQUIRE(preview.socketCount() == 3);
}

TEST_CASE("D.3 Socket: sockets cleared on clearAsset", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    SocketDescriptor s; s.name = "s1";
    preview.addSocket(s);
    preview.clearAsset();
    REQUIRE(preview.socketCount() == 0);
}

TEST_CASE("D.3 Socket: apply/revert round-trips sockets", "[phase_d][d3][socket]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    SocketDescriptor s; s.name = "s1"; s.socketType = "weapon";
    preview.addSocket(s);
    preview.apply();
    REQUIRE(preview.socketCount() == 1);

    preview.removeSocket("s1");
    REQUIRE(preview.socketCount() == 0);

    preview.revert();
    REQUIRE(preview.socketCount() == 1);
    REQUIRE(preview.sockets()[0].name == "s1");
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.3 Extended — Anchors
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.3 Anchor: addAnchor increases count", "[phase_d][d3][anchor]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    AnchorDescriptor a;
    a.name       = "dock_approach";
    a.anchorType = "dock";
    REQUIRE(preview.addAnchor(a));
    REQUIRE(preview.anchorCount() == 1);
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.3 Anchor: addAnchor fails when no asset", "[phase_d][d3][anchor]") {
    NovaForgeAssetPreview preview;
    AnchorDescriptor a; a.name = "a1";
    REQUIRE_FALSE(preview.addAnchor(a));
}

TEST_CASE("D.3 Anchor: removeAnchor by name", "[phase_d][d3][anchor]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    AnchorDescriptor a; a.name = "spawn_01";
    preview.addAnchor(a);
    REQUIRE(preview.removeAnchor("spawn_01"));
    REQUIRE(preview.anchorCount() == 0);
}

TEST_CASE("D.3 Anchor: removeAnchor non-existent returns false", "[phase_d][d3][anchor]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE_FALSE(preview.removeAnchor("ghost"));
}

TEST_CASE("D.3 Anchor: setAnchorTransform updates transform", "[phase_d][d3][anchor]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    AnchorDescriptor a; a.name = "interact_01";
    preview.addAnchor(a);
    PreviewTransform t;
    t.position = {5.f, 0.f, 0.f};
    REQUIRE(preview.setAnchorTransform("interact_01", t));
    REQUIRE(preview.anchors()[0].localTransform.position.x == 5.f);
}

TEST_CASE("D.3 Anchor: apply/revert round-trips anchors", "[phase_d][d3][anchor]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    AnchorDescriptor a; a.name = "a1"; a.anchorType = "dock";
    preview.addAnchor(a);
    preview.apply();
    preview.removeAnchor("a1");
    REQUIRE(preview.anchorCount() == 0);
    preview.revert();
    REQUIRE(preview.anchorCount() == 1);
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.3 Extended — PCG Metadata
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.3 PCGMeta: setPlacementTag sets tag", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setPlacementTag("pcg.asteroid_cluster"));
    REQUIRE(preview.pcgMetadata().placementTag == "pcg.asteroid_cluster");
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.3 PCGMeta: addGenerationTag adds tag", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.addGenerationTag("ring.outer"));
    REQUIRE(preview.pcgMetadata().generationTags.size() == 1);
    REQUIRE(preview.pcgMetadata().generationTags[0] == "ring.outer");
}

TEST_CASE("D.3 PCGMeta: removeGenerationTag removes it", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.addGenerationTag("tag.alpha");
    preview.addGenerationTag("tag.beta");
    REQUIRE(preview.removeGenerationTag("tag.alpha"));
    REQUIRE(preview.pcgMetadata().generationTags.size() == 1);
    REQUIRE(preview.pcgMetadata().generationTags[0] == "tag.beta");
}

TEST_CASE("D.3 PCGMeta: removeGenerationTag non-existent returns false", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE_FALSE(preview.removeGenerationTag("ghost"));
}

TEST_CASE("D.3 PCGMeta: setPCGScaleRange stores min/max", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setPCGScaleRange(0.5f, 2.0f));
    REQUIRE(preview.pcgMetadata().minScale == 0.5f);
    REQUIRE(preview.pcgMetadata().maxScale == 2.0f);
}

TEST_CASE("D.3 PCGMeta: setPCGScaleRange rejects min>max", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE_FALSE(preview.setPCGScaleRange(2.0f, 0.5f));
}

TEST_CASE("D.3 PCGMeta: setPCGDensity stores density", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setPCGDensity(0.75f));
    REQUIRE(preview.pcgMetadata().density == 0.75f);
}

TEST_CASE("D.3 PCGMeta: setPCGDensity rejects negative", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE_FALSE(preview.setPCGDensity(-1.f));
}

TEST_CASE("D.3 PCGMeta: setPCGExclusionGroup stores group", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    REQUIRE(preview.setPCGExclusionGroup("asteroids.large"));
    REQUIRE(preview.pcgMetadata().exclusionGroup == "asteroids.large");
}

TEST_CASE("D.3 PCGMeta: pcg.placementTag in properties()", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.setPlacementTag("pcg.ship");
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "pcg.placementTag"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "pcg.ship");
}

TEST_CASE("D.3 PCGMeta: pcg.density in properties()", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.setPCGDensity(0.5f);
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "pcg.density"; });
    REQUIRE(it != props.end());
}

TEST_CASE("D.3 PCGMeta: apply/revert round-trips PCG metadata", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    preview.setPlacementTag("pcg.station");
    preview.setPCGDensity(2.f);
    preview.apply();

    preview.setPlacementTag("pcg.changed");
    preview.setPCGDensity(0.1f);

    preview.revert();
    REQUIRE(preview.pcgMetadata().placementTag == "pcg.station");
    REQUIRE(preview.pcgMetadata().density == 2.f);
}

TEST_CASE("D.3 PCGMeta: operations fail when no asset bound", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    REQUIRE_FALSE(preview.setPlacementTag("x"));
    REQUIRE_FALSE(preview.addGenerationTag("y"));
    REQUIRE_FALSE(preview.setPCGDensity(1.f));
    REQUIRE_FALSE(preview.setPCGExclusionGroup("g"));
}

TEST_CASE("D.3 PCGMeta: setPCGMetadata sets full metadata block", "[phase_d][d3][pcg_meta]") {
    NovaForgeAssetPreview preview;
    preview.bindAsset(makeDesc());
    AssetPCGMetadata meta;
    meta.placementTag   = "pcg.fighter";
    meta.minScale       = 0.9f;
    meta.maxScale       = 1.1f;
    meta.density        = 3.f;
    meta.exclusionGroup = "fighters";
    REQUIRE(preview.setPCGMetadata(meta));
    REQUIRE(preview.pcgMetadata().placementTag   == "pcg.fighter");
    REQUIRE(preview.pcgMetadata().exclusionGroup == "fighters");
    REQUIRE(preview.pcgMetadata().density        == 3.f);
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.4 — NovaForgeMaterialPreview
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.4 MaterialPreview: starts with no material", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    REQUIRE_FALSE(preview.hasMaterial());
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.4 MaterialPreview: provideScene hasContent=false with no material", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    NF::ViewportSlot slot;
    REQUIRE_FALSE(preview.provideScene(1, slot).hasContent);
}

TEST_CASE("D.4 MaterialPreview: bindMaterial sets hasMaterial=true", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/Metal.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.hasMaterial());
}

TEST_CASE("D.4 MaterialPreview: not dirty after bindMaterial", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.4 MaterialPreview: provideScene hasContent=true after bindMaterial", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    NF::ViewportSlot slot;
    REQUIRE(preview.provideScene(1, slot).hasContent);
}

TEST_CASE("D.4 MaterialPreview: provideScene entityCount=1 after bind", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    NF::ViewportSlot slot;
    REQUIRE(preview.provideScene(1, slot).entityCount == 1);
}

TEST_CASE("D.4 MaterialPreview: clearMaterial resets state", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    preview.clearMaterial();
    REQUIRE_FALSE(preview.hasMaterial());
    REQUIRE(preview.previewWorld().entityCount() == 0);
}

TEST_CASE("D.4 MaterialPreview: setPreviewMesh changes mesh tag", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.setPreviewMesh(PreviewMeshType::Cube));
    REQUIRE(preview.previewMesh() == PreviewMeshType::Cube);
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.4 MaterialPreview: default preview mesh is Sphere", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.previewMesh() == PreviewMeshType::Sphere);
}

TEST_CASE("D.4 MaterialPreview: all 3 mesh types cycle correctly", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.setPreviewMesh(PreviewMeshType::Plane));
    REQUIRE(preview.previewMesh() == PreviewMeshType::Plane);
    REQUIRE(preview.setPreviewMesh(PreviewMeshType::Cube));
    REQUIRE(preview.previewMesh() == PreviewMeshType::Cube);
    REQUIRE(preview.setPreviewMesh(PreviewMeshType::Sphere));
    REQUIRE(preview.previewMesh() == PreviewMeshType::Sphere);
}

TEST_CASE("D.4 MaterialPreview: setPreviewMesh fails when no material bound", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    REQUIRE_FALSE(preview.setPreviewMesh(PreviewMeshType::Cube));
}

TEST_CASE("D.4 MaterialPreview: setShaderTag stores tag", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.setShaderTag("shaders/pbr_metallic"));
    REQUIRE(preview.shaderTag() == "shaders/pbr_metallic");
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.4 MaterialPreview: setShaderTag fails when no material bound", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    REQUIRE_FALSE(preview.setShaderTag("shaders/pbr"));
}

// ── Parameters ────────────────────────────────────────────────────────────

TEST_CASE("D.4 MaterialPreview: setParameter adds new parameter", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.setParameter("roughness", "0.5"));
    REQUIRE(preview.parameterCount() == 1);
    REQUIRE(preview.getParameter("roughness") == "0.5");
    REQUIRE(preview.isDirty());
}

TEST_CASE("D.4 MaterialPreview: setParameter updates existing parameter", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setParameter("roughness", "0.5");
    REQUIRE(preview.setParameter("roughness", "0.9"));
    REQUIRE(preview.parameterCount() == 1);
    REQUIRE(preview.getParameter("roughness") == "0.9");
}

TEST_CASE("D.4 MaterialPreview: setParameter with type", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.setParameter("base_color", "1,0,0,1", MaterialParameterType::Vec4));
    REQUIRE(preview.parameters()[0].type == MaterialParameterType::Vec4);
}

TEST_CASE("D.4 MaterialPreview: removeParameter removes it", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setParameter("metallic", "0.0");
    REQUIRE(preview.removeParameter("metallic"));
    REQUIRE(preview.parameterCount() == 0);
}

TEST_CASE("D.4 MaterialPreview: removeParameter non-existent returns false", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE_FALSE(preview.removeParameter("ghost"));
}

TEST_CASE("D.4 MaterialPreview: getParameter fallback for missing key", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.getParameter("missing", "fallback") == "fallback");
}

TEST_CASE("D.4 MaterialPreview: resetParameterToDefault restores default", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setParameter("roughness", "0.5"); // initial sets default=0.5
    preview.setParameter("roughness", "0.9"); // updates to 0.9
    REQUIRE(preview.getParameter("roughness") == "0.9");
    REQUIRE(preview.resetParameterToDefault("roughness"));
    REQUIRE(preview.getParameter("roughness") == "0.5");
}

TEST_CASE("D.4 MaterialPreview: resetAllParametersToDefault restores all", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setParameter("roughness", "0.5");
    preview.setParameter("metallic",  "0.0");
    preview.setParameter("roughness", "0.9");
    preview.setParameter("metallic",  "1.0");
    REQUIRE(preview.resetAllParametersToDefault());
    REQUIRE(preview.getParameter("roughness") == "0.5");
    REQUIRE(preview.getParameter("metallic")  == "0.0");
}

TEST_CASE("D.4 MaterialPreview: setParameter fails when no material bound", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    REQUIRE_FALSE(preview.setParameter("roughness", "0.5"));
}

// ── apply / revert ────────────────────────────────────────────────────────

TEST_CASE("D.4 MaterialPreview: apply() clears dirty", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setShaderTag("shaders/pbr");
    REQUIRE(preview.apply());
    REQUIRE_FALSE(preview.isDirty());
}

TEST_CASE("D.4 MaterialPreview: revert() restores shaderTag", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath = "M/X.nfmat";
    desc.shaderTag    = "shaders/original";
    preview.bindMaterial(desc);
    preview.setShaderTag("shaders/modified");
    REQUIRE(preview.revert());
    REQUIRE(preview.shaderTag() == "shaders/original");
}

TEST_CASE("D.4 MaterialPreview: revert() restores preview mesh", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setPreviewMesh(PreviewMeshType::Cube);
    preview.apply();
    preview.setPreviewMesh(PreviewMeshType::Plane);
    preview.revert();
    REQUIRE(preview.previewMesh() == PreviewMeshType::Cube);
}

TEST_CASE("D.4 MaterialPreview: apply on no-material returns false", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    REQUIRE_FALSE(preview.apply());
}

TEST_CASE("D.4 MaterialPreview: revert on no-material returns false", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    REQUIRE_FALSE(preview.revert());
}

// ── properties() ─────────────────────────────────────────────────────────

TEST_CASE("D.4 MaterialPreview: properties() has 3 base entries", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    // base entries: materialPath, shaderTag, previewMesh
    REQUIRE(preview.properties().size() >= 3);
}

TEST_CASE("D.4 MaterialPreview: properties() includes param.* entries", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setParameter("roughness", "0.5");
    preview.setParameter("metallic",  "0.0");
    auto props = preview.properties();
    REQUIRE(props.size() == 5); // materialPath + shaderTag + previewMesh + 2 params
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "param.roughness"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "0.5");
}

TEST_CASE("D.4 MaterialPreview: properties() materialPath present", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "Materials/Plasma.nfmat";
    preview.bindMaterial(desc);
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "materialPath"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "Materials/Plasma.nfmat");
}

TEST_CASE("D.4 MaterialPreview: properties() previewMesh name", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setPreviewMesh(PreviewMeshType::Plane);
    auto props = preview.properties();
    auto it = std::find_if(props.begin(), props.end(),
        [](const auto& p){ return p.first == "previewMesh"; });
    REQUIRE(it != props.end());
    REQUIRE(it->second == "Plane");
}

// ── previewWorld() ────────────────────────────────────────────────────────

TEST_CASE("D.4 MaterialPreview: previewWorld has 1 entity after bind", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    REQUIRE(preview.previewWorld().entityCount() == 1);
}

TEST_CASE("D.4 MaterialPreview: previewWorld entity uses previewMeshTypeTag", "[phase_d][d4][material_preview]") {
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc;
    desc.materialPath  = "M/X.nfmat";
    desc.previewMesh   = PreviewMeshType::Cube;
    preview.bindMaterial(desc);
    const auto& entities = preview.previewWorld().entities();
    REQUIRE_FALSE(entities.empty());
    REQUIRE(entities[0].meshTag == std::string(previewMeshTypeTag(PreviewMeshType::Cube)));
}

// ═══════════════════════════════════════════════════════════════════════════
//  D.4 — MaterialEditorTool wiring
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("D.4 MaterialEditorTool: no provider — provideScene hasContent=false", "[phase_d][d4][tool]") {
    NF::MaterialEditorTool tool;
    NF::ViewportSlot slot;
    REQUIRE_FALSE(tool.provideScene(1, slot).hasContent);
}

TEST_CASE("D.4 MaterialEditorTool: attachMaterialPreviewProvider stores pointer", "[phase_d][d4][tool]") {
    NF::MaterialEditorTool tool;
    NovaForgeMaterialPreview preview;
    tool.attachMaterialPreviewProvider(&preview);
    REQUIRE(tool.materialPreviewProvider() == &preview);
}

TEST_CASE("D.4 MaterialEditorTool: provideScene delegates when material bound", "[phase_d][d4][tool]") {
    NF::MaterialEditorTool tool;
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "Materials/Metal.nfmat";
    preview.bindMaterial(desc);
    tool.attachMaterialPreviewProvider(&preview);
    NF::ViewportSlot slot;
    auto state = tool.provideScene(1, slot);
    REQUIRE(state.hasContent);
    REQUIRE(state.entityCount == 1);
}

TEST_CASE("D.4 MaterialEditorTool: detach provider reverts to stub", "[phase_d][d4][tool]") {
    NF::MaterialEditorTool tool;
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    tool.attachMaterialPreviewProvider(&preview);
    tool.attachMaterialPreviewProvider(nullptr);
    NF::ViewportSlot slot;
    REQUIRE_FALSE(tool.provideScene(1, slot).hasContent);
}

TEST_CASE("D.4 MaterialEditorTool: provideScene not-content when material cleared", "[phase_d][d4][tool]") {
    NF::MaterialEditorTool tool;
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "Materials/X.nfmat";
    preview.bindMaterial(desc);
    tool.attachMaterialPreviewProvider(&preview);
    preview.clearMaterial();
    NF::ViewportSlot slot;
    REQUIRE_FALSE(tool.provideScene(1, slot).hasContent);
}

TEST_CASE("D.4 MaterialEditorTool: previewMesh type from preview reflected in entityCount", "[phase_d][d4][tool]") {
    NF::MaterialEditorTool tool;
    NovaForgeMaterialPreview preview;
    MaterialPreviewDescriptor desc; desc.materialPath = "M/X.nfmat";
    preview.bindMaterial(desc);
    preview.setPreviewMesh(PreviewMeshType::Plane);
    tool.attachMaterialPreviewProvider(&preview);
    NF::ViewportSlot slot;
    REQUIRE(tool.provideScene(1, slot).entityCount == 1);
}
