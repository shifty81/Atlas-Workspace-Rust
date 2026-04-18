// Tests/Workspace/test_phase_g.cpp
//
// Phase G — Full Tool Wiring
//
// G.1  — SceneDocument (world/level document model)
//   - Construction, identity, dirty tracking
//   - Entity create/delete/duplicate
//   - Transform editing
//   - Component add/remove/edit
//   - Selection, hierarchy order
//   - Save / load / serialize
//
// G.2  — AssetDocument (asset metadata document model)
//   - Construction, identity, dirty tracking
//   - LOD management
//   - Variant management
//   - Dependency management
//   - Reimport settings
//   - Save / load / serialize
//
// G.3  — MaterialDocument (material graph document model)
//   - Construction, identity, dirty tracking
//   - Node add/remove/find
//   - Pin add/remove
//   - Node parameter get/set
//   - Connection connect/disconnect/isConnected
//   - Output node designation
//   - Save / load / serialize
//
// G.4  — AnimationDocument (animation clip document model)
//   - Construction, identity, dirty tracking
//   - Clip metadata (duration, fps, looping)
//   - Channel management
//   - Keyframe management (sorted by time)
//   - Save / load / serialize
//
// G.5  — GraphDocument (visual logic graph document model)
//   - Construction, identity, dirty tracking
//   - Node add/remove/find
//   - Pin add/remove
//   - Node property get/set
//   - Connection connect/disconnect
//   - Compile — error detection
//   - Save / load / serialize
//
// G.6  — BuildTaskGraph (build task DAG)
//   - Task add, dependency declaration
//   - Topological order (dependency-first)
//   - Task state transitions
//   - Build log (pushOutputLine, callback)
//   - Summary statistics
//
// G.7  — AIRequestContext (AtlasAI request and diff context)
//   - Context binding (document, object, errors)
//   - Conversation history (submit user/assistant messages)
//   - Diff proposals (add, apply, reject)
//   - Codex snippet library
//
// G.8  — DataTableDocument (schema-driven data table)
//   - Column add/remove/find
//   - Row add/remove/duplicate
//   - Cell get/set
//   - Validation (type, enum, required)
//   - CSV export
//   - Save / load / serialize

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/Editor/SceneDocument.h"
#include "NF/Editor/AssetDocument.h"
#include "NF/Editor/MaterialDocument.h"
#include "NF/Editor/AnimationDocument.h"
#include "NF/Editor/GraphDocument.h"
#include "NF/Editor/BuildTaskGraph.h"
#include "NF/Editor/AIRequestContext.h"
#include "NF/Editor/DataTableDocument.h"

#include <string>
#include <vector>

using namespace NF;
using Catch::Approx;

// ═══════════════════════════════════════════════════════════════════════════
//  G.1 — SceneDocument
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("SceneDocument default state is clean", "[phase_g][g1][scene]") {
    SceneDocument doc;
    REQUIRE(doc.scenePath().empty());
    REQUIRE(doc.sceneName().empty());
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.entityCount() == 0u);
}

TEST_CASE("SceneDocument construction with path", "[phase_g][g1][scene]") {
    SceneDocument doc("/project/scenes/world.scene");
    REQUIRE(doc.scenePath() == "/project/scenes/world.scene");
}

TEST_CASE("SceneDocument setSceneName marks dirty", "[phase_g][g1][scene]") {
    SceneDocument doc;
    doc.setSceneName("MainWorld");
    REQUIRE(doc.sceneName() == "MainWorld");
    REQUIRE(doc.isDirty());
}

TEST_CASE("SceneDocument createEntity returns valid ID", "[phase_g][g1][scene][entity]") {
    SceneDocument doc;
    EntityDocId id = doc.createEntity("Root");
    REQUIRE(id != kInvalidEntityDocId);
    REQUIRE(doc.hasEntity(id));
    REQUIRE(doc.entityCount() == 1u);
    REQUIRE(doc.isDirty());
}

TEST_CASE("SceneDocument createEntity with parent", "[phase_g][g1][scene][entity]") {
    SceneDocument doc;
    EntityDocId parent = doc.createEntity("Parent");
    EntityDocId child  = doc.createEntity("Child", parent);
    REQUIRE(doc.entityCount() == 2u);
    const auto* e = doc.findEntity(child);
    REQUIRE(e != nullptr);
    REQUIRE(e->parentId == parent);
}

TEST_CASE("SceneDocument destroyEntity removes entity and children", "[phase_g][g1][scene][entity]") {
    SceneDocument doc;
    EntityDocId root  = doc.createEntity("Root");
    EntityDocId child = doc.createEntity("Child", root);
    (void)child;
    doc.clearDirty();

    bool removed = doc.destroyEntity(root);
    REQUIRE(removed);
    REQUIRE(doc.entityCount() == 0u);
    REQUIRE(doc.isDirty());
}

TEST_CASE("SceneDocument destroyEntity on unknown ID returns false", "[phase_g][g1][scene][entity]") {
    SceneDocument doc;
    REQUIRE_FALSE(doc.destroyEntity(999u));
}

TEST_CASE("SceneDocument duplicateEntity creates copy", "[phase_g][g1][scene][entity]") {
    SceneDocument doc;
    EntityDocId orig = doc.createEntity("Hero");
    EntityDocId copy = doc.duplicateEntity(orig);
    REQUIRE(copy != kInvalidEntityDocId);
    REQUIRE(copy != orig);
    REQUIRE(doc.entityCount() == 2u);
    const auto* e = doc.findEntity(copy);
    REQUIRE(e != nullptr);
    REQUIRE(e->name == "Hero_copy");
}

TEST_CASE("SceneDocument setEntityTransform and retrieve", "[phase_g][g1][scene][transform]") {
    SceneDocument doc;
    EntityDocId id = doc.createEntity("Cube");
    DocTransform t;
    t.px = 1.f; t.py = 2.f; t.pz = 3.f;
    t.rx = 10.f; t.ry = 20.f; t.rz = 30.f;
    t.sx = 2.f; t.sy = 2.f; t.sz = 2.f;
    REQUIRE(doc.setEntityTransform(id, t));
    DocTransform r = doc.entityTransform(id);
    REQUIRE(r.px == Approx(1.f));
    REQUIRE(r.py == Approx(2.f));
    REQUIRE(r.pz == Approx(3.f));
    REQUIRE(r.rx == Approx(10.f));
    REQUIRE(r.sx == Approx(2.f));
}

TEST_CASE("SceneDocument addComponent and removeComponent", "[phase_g][g1][scene][component]") {
    SceneDocument doc;
    EntityDocId eid = doc.createEntity("Actor");
    ComponentDocId cid = doc.addComponent(eid, "MeshComponent");
    REQUIRE(cid != kInvalidComponentDocId);
    REQUIRE(doc.componentCount(eid) == 1u);

    REQUIRE(doc.removeComponent(eid, cid));
    REQUIRE(doc.componentCount(eid) == 0u);
}

TEST_CASE("SceneDocument setComponentProperty", "[phase_g][g1][scene][component]") {
    SceneDocument doc;
    EntityDocId eid = doc.createEntity("Actor");
    ComponentDocId cid = doc.addComponent(eid, "MeshComponent");
    REQUIRE(doc.setComponentProperty(eid, cid, "mesh", "Content/Meshes/cube.mesh"));
    const auto* c = doc.findComponent(eid, cid);
    REQUIRE(c != nullptr);
    REQUIRE(c->properties.at("mesh") == "Content/Meshes/cube.mesh");
}

TEST_CASE("SceneDocument hierarchyOrder is parent-before-child", "[phase_g][g1][scene][hierarchy]") {
    SceneDocument doc;
    EntityDocId root   = doc.createEntity("Root");
    EntityDocId childA = doc.createEntity("ChildA", root);
    EntityDocId childB = doc.createEntity("ChildB", root);
    (void)childA; (void)childB;

    auto order = doc.hierarchyOrder();
    REQUIRE(order.size() == 3u);
    REQUIRE(order[0] == root);
}

TEST_CASE("SceneDocument selection", "[phase_g][g1][scene][selection]") {
    SceneDocument doc;
    REQUIRE_FALSE(doc.hasSelection());
    EntityDocId id = doc.createEntity("E");
    REQUIRE(doc.selectEntity(id));
    REQUIRE(doc.hasSelection());
    REQUIRE(doc.selectedEntityId() == id);
    doc.clearSelection();
    REQUIRE_FALSE(doc.hasSelection());
}

TEST_CASE("SceneDocument save with path", "[phase_g][g1][scene][save]") {
    SceneDocument doc;
    doc.createEntity("E");
    auto result = doc.save("/project/scenes/world.scene");
    REQUIRE(result.ok());
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("SceneDocument save without path returns IoError", "[phase_g][g1][scene][save]") {
    SceneDocument doc;
    doc.createEntity("E");
    auto result = doc.save();
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.status == SceneSaveStatus::IoError);
}

TEST_CASE("SceneDocument serialize produces non-empty JSON", "[phase_g][g1][scene][serialize]") {
    SceneDocument doc;
    doc.setSceneName("MainWorld");
    doc.createEntity("Player");
    std::string json = doc.serialize();
    REQUIRE_FALSE(json.empty());
    REQUIRE(json.find("MainWorld") != std::string::npos);
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.2 — AssetDocument
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("AssetDocument default state is clean", "[phase_g][g2][asset]") {
    AssetDocument doc;
    REQUIRE(doc.guid().empty());
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.lodCount()        == 0u);
    REQUIRE(doc.variantCount()    == 0u);
    REQUIRE(doc.dependencyCount() == 0u);
}

TEST_CASE("AssetDocument construction with guid and path", "[phase_g][g2][asset]") {
    AssetDocument doc("guid-001", "/content/hero.asset", AssetDocType::StaticMesh);
    REQUIRE(doc.guid() == "guid-001");
    REQUIRE(doc.assetPath() == "/content/hero.asset");
    REQUIRE(doc.type() == AssetDocType::StaticMesh);
}

TEST_CASE("AssetDocument setDisplayName marks dirty", "[phase_g][g2][asset]") {
    AssetDocument doc("g", "/p");
    doc.setDisplayName("HeroMesh");
    REQUIRE(doc.displayName() == "HeroMesh");
    REQUIRE(doc.isDirty());
}

TEST_CASE("AssetDocument LOD add/remove/find", "[phase_g][g2][asset][lod]") {
    AssetDocument doc("g", "/p");
    AssetLODEntry lod0;
    lod0.lodIndex      = 0;
    lod0.meshTag       = "lod0.mesh";
    lod0.screenPercent = 100.f;
    doc.addLOD(lod0);
    REQUIRE(doc.lodCount() == 1u);
    REQUIRE(doc.findLOD(0) != nullptr);
    REQUIRE(doc.findLOD(0)->meshTag == "lod0.mesh");
    REQUIRE(doc.removeLOD(0));
    REQUIRE(doc.lodCount() == 0u);
}

TEST_CASE("AssetDocument setLODScreenPercent", "[phase_g][g2][asset][lod]") {
    AssetDocument doc("g", "/p");
    AssetLODEntry lod;
    lod.lodIndex      = 0;
    lod.screenPercent = 100.f;
    doc.addLOD(lod);
    doc.clearDirty();
    REQUIRE(doc.setLODScreenPercent(0, 50.f));
    REQUIRE(doc.findLOD(0)->screenPercent == Approx(50.f));
    REQUIRE(doc.isDirty());
}

TEST_CASE("AssetDocument variant add/remove/find", "[phase_g][g2][asset][variant]") {
    AssetDocument doc("g", "/p");
    AssetVariantEntry v;
    v.variantId   = "red";
    v.displayName = "Red Variant";
    v.overrides["diffuse"] = "red_diffuse.tex";
    doc.addVariant(v);
    REQUIRE(doc.variantCount() == 1u);
    const auto* found = doc.findVariant("red");
    REQUIRE(found != nullptr);
    REQUIRE(found->displayName == "Red Variant");
    REQUIRE(doc.removeVariant("red"));
    REQUIRE(doc.variantCount() == 0u);
}

TEST_CASE("AssetDocument dependency add/remove", "[phase_g][g2][asset][dependency]") {
    AssetDocument doc("g", "/p");
    AssetDependency dep;
    dep.guid         = "dep-guid-001";
    dep.relationship = "material";
    doc.addDependency(dep);
    REQUIRE(doc.dependencyCount() == 1u);
    REQUIRE(doc.removeDependency("dep-guid-001"));
    REQUIRE(doc.dependencyCount() == 0u);
}

TEST_CASE("AssetDocument setSourcePath marks dirty", "[phase_g][g2][asset][reimport]") {
    AssetDocument doc("g", "/p");
    doc.clearDirty();
    doc.setSourcePath("/sources/hero.fbx");
    REQUIRE(doc.importSettings().sourcePath == "/sources/hero.fbx");
    REQUIRE(doc.isDirty());
}

TEST_CASE("AssetDocument save and load clear dirty", "[phase_g][g2][asset][save]") {
    AssetDocument doc("guid-001", "/content/hero.asset");
    doc.setDisplayName("Hero");
    REQUIRE(doc.save());
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.load());
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("AssetDocument serialize produces valid JSON stub", "[phase_g][g2][asset][serialize]") {
    AssetDocument doc("guid-001", "/p", AssetDocType::StaticMesh);
    std::string json = doc.serialize();
    REQUIRE(json.find("guid-001") != std::string::npos);
    REQUIRE(json.find("StaticMesh") != std::string::npos);
}

TEST_CASE("AssetDocType names are correct", "[phase_g][g2][asset][types]") {
    REQUIRE(std::string(assetDocTypeName(AssetDocType::StaticMesh))    == "StaticMesh");
    REQUIRE(std::string(assetDocTypeName(AssetDocType::SkeletalMesh))  == "SkeletalMesh");
    REQUIRE(std::string(assetDocTypeName(AssetDocType::Material))      == "Material");
    REQUIRE(std::string(assetDocTypeName(AssetDocType::AnimationClip)) == "AnimationClip");
    REQUIRE(std::string(assetDocTypeName(AssetDocType::Unknown))       == "Unknown");
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.3 — MaterialDocument
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("MaterialDocument default state is clean", "[phase_g][g3][material]") {
    MaterialDocument doc;
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.nodeCount()       == 0u);
    REQUIRE(doc.connectionCount() == 0u);
    REQUIRE(doc.outputNodeId()    == kInvalidMaterialNodeId);
}

TEST_CASE("MaterialDocument construction with path", "[phase_g][g3][material]") {
    MaterialDocument doc("/content/mat.mat");
    REQUIRE(doc.assetPath() == "/content/mat.mat");
}

TEST_CASE("MaterialDocument addNode returns valid ID", "[phase_g][g3][material][node]") {
    MaterialDocument doc;
    MaterialNodeId id = doc.addNode(MaterialNodeType::Constant, "ConstR");
    REQUIRE(id != kInvalidMaterialNodeId);
    REQUIRE(doc.nodeCount() == 1u);
    REQUIRE(doc.isDirty());
    const auto* n = doc.findNode(id);
    REQUIRE(n != nullptr);
    REQUIRE(n->label == "ConstR");
    REQUIRE(n->type  == MaterialNodeType::Constant);
}

TEST_CASE("MaterialDocument first Output node becomes outputNodeId", "[phase_g][g3][material][node]") {
    MaterialDocument doc;
    MaterialNodeId out = doc.addNode(MaterialNodeType::Output, "Surface");
    REQUIRE(doc.outputNodeId() == out);
}

TEST_CASE("MaterialDocument removeNode also removes connections", "[phase_g][g3][material][node]") {
    MaterialDocument doc;
    MaterialNodeId n1 = doc.addNode(MaterialNodeType::Constant, "C1");
    MaterialNodeId n2 = doc.addNode(MaterialNodeType::Output,   "Out");
    MaterialPinId  p1 = doc.addPin(n1, "Value", MaterialPinDir::Output, "float");
    MaterialPinId  p2 = doc.addPin(n2, "Color", MaterialPinDir::Input,  "float");
    doc.connectPins(n1, p1, n2, p2);
    REQUIRE(doc.connectionCount() == 1u);
    REQUIRE(doc.removeNode(n1));
    REQUIRE(doc.nodeCount()       == 1u);
    REQUIRE(doc.connectionCount() == 0u);
}

TEST_CASE("MaterialDocument setNodeParam and getNodeParam", "[phase_g][g3][material][param]") {
    MaterialDocument doc;
    MaterialNodeId id = doc.addNode(MaterialNodeType::TextureSample, "Tex");
    REQUIRE(doc.setNodeParam(id, "texture", "Content/T_Rock.tex"));
    REQUIRE(doc.getNodeParam(id, "texture") == "Content/T_Rock.tex");
    REQUIRE(doc.getNodeParam(id, "missing", "default") == "default");
}

TEST_CASE("MaterialDocument addPin increments count", "[phase_g][g3][material][pin]") {
    MaterialDocument doc;
    MaterialNodeId nid = doc.addNode(MaterialNodeType::MathAdd, "Add");
    MaterialPinId p1 = doc.addPin(nid, "A", MaterialPinDir::Input,  "float");
    MaterialPinId p2 = doc.addPin(nid, "B", MaterialPinDir::Input,  "float");
    MaterialPinId p3 = doc.addPin(nid, "Out", MaterialPinDir::Output, "float");
    REQUIRE(p1 != kInvalidMaterialPinId);
    REQUIRE(p2 != kInvalidMaterialPinId);
    REQUIRE(p3 != kInvalidMaterialPinId);
    REQUIRE(doc.pinCount(nid) == 3u);
}

TEST_CASE("MaterialDocument removePin disconnects connections", "[phase_g][g3][material][pin]") {
    MaterialDocument doc;
    MaterialNodeId n1 = doc.addNode(MaterialNodeType::Constant, "C");
    MaterialNodeId n2 = doc.addNode(MaterialNodeType::Output,   "Out");
    MaterialPinId  p1 = doc.addPin(n1, "V",     MaterialPinDir::Output, "float");
    MaterialPinId  p2 = doc.addPin(n2, "Color", MaterialPinDir::Input,  "float");
    doc.connectPins(n1, p1, n2, p2);
    REQUIRE(doc.connectionCount() == 1u);
    doc.removePin(n1, p1);
    REQUIRE(doc.connectionCount() == 0u);
}

TEST_CASE("MaterialDocument connectPins and isConnected", "[phase_g][g3][material][connection]") {
    MaterialDocument doc;
    MaterialNodeId n1 = doc.addNode(MaterialNodeType::Constant, "C");
    MaterialNodeId n2 = doc.addNode(MaterialNodeType::Output,   "Out");
    MaterialPinId  p1 = doc.addPin(n1, "V",     MaterialPinDir::Output, "float");
    MaterialPinId  p2 = doc.addPin(n2, "Color", MaterialPinDir::Input,  "float");
    REQUIRE(doc.connectPins(n1, p1, n2, p2));
    REQUIRE(doc.connectionCount() == 1u);
    REQUIRE(doc.isConnected(n2, p2));
}

TEST_CASE("MaterialDocument disconnectPin removes connection", "[phase_g][g3][material][connection]") {
    MaterialDocument doc;
    MaterialNodeId n1 = doc.addNode(MaterialNodeType::Constant, "C");
    MaterialNodeId n2 = doc.addNode(MaterialNodeType::Output,   "Out");
    MaterialPinId  p1 = doc.addPin(n1, "V",     MaterialPinDir::Output, "float");
    MaterialPinId  p2 = doc.addPin(n2, "Color", MaterialPinDir::Input,  "float");
    doc.connectPins(n1, p1, n2, p2);
    REQUIRE(doc.disconnectPin(n2, p2));
    REQUIRE_FALSE(doc.isConnected(n2, p2));
    REQUIRE(doc.connectionCount() == 0u);
}

TEST_CASE("MaterialDocument connectPins with invalid node returns false", "[phase_g][g3][material][connection]") {
    MaterialDocument doc;
    MaterialNodeId n1 = doc.addNode(MaterialNodeType::Constant, "C");
    REQUIRE_FALSE(doc.connectPins(n1, 1u, 999u, 1u));
}

TEST_CASE("MaterialDocument save with path clears dirty", "[phase_g][g3][material][save]") {
    MaterialDocument doc("/content/mat.mat");
    doc.addNode(MaterialNodeType::Output, "Out");
    auto r = doc.save();
    REQUIRE(r.ok());
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("MaterialDocument save without path returns IoError", "[phase_g][g3][material][save]") {
    MaterialDocument doc;
    doc.addNode(MaterialNodeType::Output, "Out");
    auto r = doc.save();
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.status == MaterialSaveStatus::IoError);
}

TEST_CASE("MaterialDocument serialize produces non-empty output", "[phase_g][g3][material][serialize]") {
    MaterialDocument doc;
    doc.setDisplayName("RockMaterial");
    doc.addNode(MaterialNodeType::Output, "Out");
    std::string json = doc.serialize();
    REQUIRE_FALSE(json.empty());
    REQUIRE(json.find("RockMaterial") != std::string::npos);
}

TEST_CASE("MaterialNodeType names are correct", "[phase_g][g3][material][types]") {
    REQUIRE(std::string(materialNodeTypeName(MaterialNodeType::Output))        == "Output");
    REQUIRE(std::string(materialNodeTypeName(MaterialNodeType::TextureSample)) == "TextureSample");
    REQUIRE(std::string(materialNodeTypeName(MaterialNodeType::MathAdd))       == "MathAdd");
    REQUIRE(std::string(materialNodeTypeName(MaterialNodeType::Lerp))          == "Lerp");
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.4 — AnimationDocument
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("AnimationDocument default state is clean", "[phase_g][g4][animation]") {
    AnimationDocument doc;
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.channelCount() == 0u);
    REQUIRE(doc.durationMs()   == Approx(0.f));
    REQUIRE(doc.fps()          == Approx(30.f));
    REQUIRE_FALSE(doc.isLooping());
}

TEST_CASE("AnimationDocument construction with clip name", "[phase_g][g4][animation]") {
    AnimationDocument doc("RunCycle");
    REQUIRE(doc.clipName() == "RunCycle");
}

TEST_CASE("AnimationDocument setDurationMs marks dirty", "[phase_g][g4][animation][meta]") {
    AnimationDocument doc("Run");
    doc.setDurationMs(1000.f);
    REQUIRE(doc.durationMs() == Approx(1000.f));
    REQUIRE(doc.isDirty());
}

TEST_CASE("AnimationDocument setLooping", "[phase_g][g4][animation][meta]") {
    AnimationDocument doc("Run");
    doc.setLooping(true);
    REQUIRE(doc.isLooping());
    REQUIRE(doc.isDirty());
}

TEST_CASE("AnimationDocument addChannel returns valid ID", "[phase_g][g4][animation][channel]") {
    AnimationDocument doc("Run");
    AnimChannelId id = doc.addChannel("Spine", AnimChannelType::Rotation);
    REQUIRE(id != kInvalidAnimChannelId);
    REQUIRE(doc.channelCount() == 1u);
    const auto* ch = doc.findChannel(id);
    REQUIRE(ch != nullptr);
    REQUIRE(ch->boneName == "Spine");
    REQUIRE(ch->type == AnimChannelType::Rotation);
}

TEST_CASE("AnimationDocument removeChannel", "[phase_g][g4][animation][channel]") {
    AnimationDocument doc("Run");
    AnimChannelId id = doc.addChannel("Hip", AnimChannelType::Translation);
    REQUIRE(doc.removeChannel(id));
    REQUIRE(doc.channelCount() == 0u);
}

TEST_CASE("AnimationDocument findChannelByBone", "[phase_g][g4][animation][channel]") {
    AnimationDocument doc("Run");
    AnimChannelId id = doc.addChannel("Spine", AnimChannelType::Rotation);
    REQUIRE(doc.findChannelByBone("Spine") == id);
    REQUIRE(doc.findChannelByBone("Missing") == kInvalidAnimChannelId);
}

TEST_CASE("AnimationDocument addKeyframe returns valid ID", "[phase_g][g4][animation][keyframe]") {
    AnimationDocument doc("Run");
    AnimChannelId chId = doc.addChannel("Hip", AnimChannelType::Translation);
    float val[4] = { 1.f, 2.f, 3.f, 0.f };
    AnimKeyframeId kid = doc.addKeyframe(chId, 0.f, val);
    REQUIRE(kid != kInvalidAnimKeyframeId);
    REQUIRE(doc.keyframeCount(chId) == 1u);
    REQUIRE(doc.totalKeyframeCount() == 1u);
}

TEST_CASE("AnimationDocument keyframes sorted by time", "[phase_g][g4][animation][keyframe]") {
    AnimationDocument doc("Run");
    AnimChannelId chId = doc.addChannel("Hip", AnimChannelType::Translation);
    float v[4] = {};
    doc.addKeyframe(chId, 500.f, v);
    doc.addKeyframe(chId, 100.f, v);
    doc.addKeyframe(chId, 300.f, v);
    const auto* ch = doc.findChannel(chId);
    REQUIRE(ch->keyframes[0].timeMs == Approx(100.f));
    REQUIRE(ch->keyframes[1].timeMs == Approx(300.f));
    REQUIRE(ch->keyframes[2].timeMs == Approx(500.f));
}

TEST_CASE("AnimationDocument removeKeyframe", "[phase_g][g4][animation][keyframe]") {
    AnimationDocument doc("Run");
    AnimChannelId chId = doc.addChannel("Hip", AnimChannelType::Translation);
    float v[4] = {};
    AnimKeyframeId kid = doc.addKeyframe(chId, 0.f, v);
    REQUIRE(doc.removeKeyframe(chId, kid));
    REQUIRE(doc.keyframeCount(chId) == 0u);
}

TEST_CASE("AnimationDocument save clears dirty", "[phase_g][g4][animation][save]") {
    AnimationDocument doc("Run");
    doc.setAssetPath("/content/run.anim");
    doc.setDurationMs(1000.f);
    REQUIRE(doc.save());
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("AnimationDocument save without path fails", "[phase_g][g4][animation][save]") {
    AnimationDocument doc("Run");
    REQUIRE_FALSE(doc.save());
}

TEST_CASE("AnimationDocument serialize produces non-empty output", "[phase_g][g4][animation][serialize]") {
    AnimationDocument doc("RunCycle");
    doc.addChannel("Hip", AnimChannelType::Translation);
    std::string json = doc.serialize();
    REQUIRE_FALSE(json.empty());
    REQUIRE(json.find("RunCycle") != std::string::npos);
}

TEST_CASE("AnimChannelType names are correct", "[phase_g][g4][animation][types]") {
    REQUIRE(std::string(animChannelTypeName(AnimChannelType::Translation)) == "Translation");
    REQUIRE(std::string(animChannelTypeName(AnimChannelType::Rotation))    == "Rotation");
    REQUIRE(std::string(animChannelTypeName(AnimChannelType::Event))       == "Event");
}

TEST_CASE("AnimInterpolation names are correct", "[phase_g][g4][animation][types]") {
    REQUIRE(std::string(animInterpolationName(AnimInterpolation::Step))   == "Step");
    REQUIRE(std::string(animInterpolationName(AnimInterpolation::Linear)) == "Linear");
    REQUIRE(std::string(animInterpolationName(AnimInterpolation::Bezier)) == "Bezier");
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.5 — GraphDocument
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("GraphDocument default state is clean", "[phase_g][g5][graph]") {
    GraphDocument doc;
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.nodeCount()       == 0u);
    REQUIRE(doc.connectionCount() == 0u);
}

TEST_CASE("GraphDocument construction with graph name", "[phase_g][g5][graph]") {
    GraphDocument doc("PlayerController");
    REQUIRE(doc.graphName() == "PlayerController");
}

TEST_CASE("GraphDocument addNode returns valid ID", "[phase_g][g5][graph][node]") {
    GraphDocument doc("G");
    GraphNodeId id = doc.addNode("OnGameStart", "On Game Start");
    REQUIRE(id != kInvalidGraphNodeId);
    REQUIRE(doc.nodeCount() == 1u);
    REQUIRE(doc.isDirty());
    const auto* n = doc.findNode(id);
    REQUIRE(n != nullptr);
    REQUIRE(n->typeName == "OnGameStart");
    REQUIRE(n->title    == "On Game Start");
}

TEST_CASE("GraphDocument removeNode removes connections", "[phase_g][g5][graph][node]") {
    GraphDocument doc("G");
    GraphNodeId n1 = doc.addNode("Start",  "Start");
    GraphNodeId n2 = doc.addNode("Branch", "Branch");
    GraphPinId  p1 = doc.addPin(n1, "Out", GraphPinDir::Output, GraphPinCategory::Execution);
    GraphPinId  p2 = doc.addPin(n2, "In",  GraphPinDir::Input,  GraphPinCategory::Execution);
    doc.connect(n1, p1, n2, p2);
    REQUIRE(doc.connectionCount() == 1u);
    REQUIRE(doc.removeNode(n1));
    REQUIRE(doc.nodeCount()       == 1u);
    REQUIRE(doc.connectionCount() == 0u);
}

TEST_CASE("GraphDocument setNodeProperty and getNodeProperty", "[phase_g][g5][graph][property]") {
    GraphDocument doc("G");
    GraphNodeId id = doc.addNode("Print", "Print String");
    REQUIRE(doc.setNodeProperty(id, "message", "Hello World"));
    REQUIRE(doc.getNodeProperty(id, "message") == "Hello World");
    REQUIRE(doc.getNodeProperty(id, "missing",  "def") == "def");
}

TEST_CASE("GraphDocument addPin increments count", "[phase_g][g5][graph][pin]") {
    GraphDocument doc("G");
    GraphNodeId n = doc.addNode("Branch", "Branch");
    doc.addPin(n, "Condition", GraphPinDir::Input,  GraphPinCategory::Data, "bool");
    doc.addPin(n, "True",      GraphPinDir::Output, GraphPinCategory::Execution);
    doc.addPin(n, "False",     GraphPinDir::Output, GraphPinCategory::Execution);
    REQUIRE(doc.pinCount(n) == 3u);
}

TEST_CASE("GraphDocument connect and isConnected", "[phase_g][g5][graph][connection]") {
    GraphDocument doc("G");
    GraphNodeId n1 = doc.addNode("A", "A");
    GraphNodeId n2 = doc.addNode("B", "B");
    GraphPinId  p1 = doc.addPin(n1, "Out", GraphPinDir::Output, GraphPinCategory::Execution);
    GraphPinId  p2 = doc.addPin(n2, "In",  GraphPinDir::Input,  GraphPinCategory::Execution);
    GraphConnId cid = doc.connect(n1, p1, n2, p2);
    REQUIRE(cid != kInvalidGraphConnId);
    REQUIRE(doc.connectionCount() == 1u);
    REQUIRE(doc.isConnected(n2, p2));
}

TEST_CASE("GraphDocument disconnect removes connection", "[phase_g][g5][graph][connection]") {
    GraphDocument doc("G");
    GraphNodeId n1 = doc.addNode("A", "A");
    GraphNodeId n2 = doc.addNode("B", "B");
    GraphPinId  p1 = doc.addPin(n1, "Out", GraphPinDir::Output, GraphPinCategory::Execution);
    GraphPinId  p2 = doc.addPin(n2, "In",  GraphPinDir::Input,  GraphPinCategory::Execution);
    GraphConnId cid = doc.connect(n1, p1, n2, p2);
    REQUIRE(doc.disconnect(cid));
    REQUIRE(doc.connectionCount() == 0u);
    REQUIRE_FALSE(doc.isConnected(n2, p2));
}

TEST_CASE("GraphDocument compile detects unconnected input exec pins as warnings", "[phase_g][g5][graph][compile]") {
    GraphDocument doc("G");
    GraphNodeId n = doc.addNode("Branch", "Branch");
    doc.addPin(n, "In", GraphPinDir::Input, GraphPinCategory::Execution);
    GraphCompileResult r = doc.compile();
    REQUIRE(r.warningCount() >= 1u);
}

TEST_CASE("GraphDocument compile succeeds when all exec pins connected", "[phase_g][g5][graph][compile]") {
    GraphDocument doc("G");
    GraphNodeId n1 = doc.addNode("Start",  "Start");
    GraphNodeId n2 = doc.addNode("Branch", "Branch");
    GraphPinId  p1 = doc.addPin(n1, "Out", GraphPinDir::Output, GraphPinCategory::Execution);
    GraphPinId  p2 = doc.addPin(n2, "In",  GraphPinDir::Input,  GraphPinCategory::Execution);
    doc.connect(n1, p1, n2, p2);
    GraphCompileResult r = doc.compile();
    REQUIRE(r.success);
    REQUIRE(r.errorCount() == 0u);
}

TEST_CASE("GraphDocument save with path clears dirty", "[phase_g][g5][graph][save]") {
    GraphDocument doc("G");
    doc.setAssetPath("/scripts/controller.graph");
    doc.addNode("Start", "Start");
    REQUIRE(doc.save());
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("GraphDocument serialize produces non-empty output", "[phase_g][g5][graph][serialize]") {
    GraphDocument doc("PlayerController");
    doc.addNode("Start", "Start");
    std::string json = doc.serialize();
    REQUIRE_FALSE(json.empty());
    REQUIRE(json.find("PlayerController") != std::string::npos);
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.6 — BuildTaskGraph
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("BuildTaskGraph default is empty", "[phase_g][g6][build]") {
    BuildTaskGraph g;
    REQUIRE(g.taskCount()      == 0u);
    REQUIRE(g.outputLineCount() == 0u);
    REQUIRE(g.totalErrors()    == 0u);
    REQUIRE(g.totalWarnings()  == 0u);
}

TEST_CASE("BuildTaskGraph addTask returns valid ID", "[phase_g][g6][build][task]") {
    BuildTaskGraph g;
    BuildTaskId id = g.addTask("CompileShaders");
    REQUIRE(id != kInvalidBuildTaskId);
    REQUIRE(g.taskCount() == 1u);
    const auto* t = g.findTask(id);
    REQUIRE(t != nullptr);
    REQUIRE(t->name  == "CompileShaders");
    REQUIRE(t->state == BuildTaskState::Pending);
}

TEST_CASE("BuildTaskGraph depends declares dependency", "[phase_g][g6][build][task]") {
    BuildTaskGraph g;
    BuildTaskId t1 = g.addTask("CompileShaders");
    BuildTaskId t2 = g.addTask("PackageAssets");
    REQUIRE(g.depends(t2, t1));
    const auto* task2 = g.findTask(t2);
    REQUIRE(task2->dependencies.size() == 1u);
    REQUIRE(task2->dependencies[0] == t1);
}

TEST_CASE("BuildTaskGraph topologicalOrder is dependency-first", "[phase_g][g6][build][topo]") {
    BuildTaskGraph g;
    BuildTaskId t1 = g.addTask("Step1");
    BuildTaskId t2 = g.addTask("Step2");
    BuildTaskId t3 = g.addTask("Step3");
    g.depends(t2, t1);
    g.depends(t3, t2);
    auto order = g.topologicalOrder();
    REQUIRE(order.size() == 3u);
    // t1 must come before t2, t2 before t3
    size_t pos1 = 0, pos2 = 0, pos3 = 0;
    for (size_t i = 0; i < order.size(); ++i) {
        if (order[i] == t1) pos1 = i;
        if (order[i] == t2) pos2 = i;
        if (order[i] == t3) pos3 = i;
    }
    REQUIRE(pos1 < pos2);
    REQUIRE(pos2 < pos3);
}

TEST_CASE("BuildTaskGraph setTaskState transitions correctly", "[phase_g][g6][build][state]") {
    BuildTaskGraph g;
    BuildTaskId id = g.addTask("Compile");
    REQUIRE(g.setTaskState(id, BuildTaskState::Running));
    REQUIRE(g.findTask(id)->state == BuildTaskState::Running);
    REQUIRE(g.setTaskState(id, BuildTaskState::Succeeded, 250.f));
    REQUIRE(g.findTask(id)->state == BuildTaskState::Succeeded);
    REQUIRE(g.findTask(id)->durationMs == Approx(250.f));
}

TEST_CASE("BuildTaskGraph setTaskCounts tracks errors and warnings", "[phase_g][g6][build][counts]") {
    BuildTaskGraph g;
    BuildTaskId id = g.addTask("Lint");
    g.setTaskCounts(id, 2u, 5u);
    REQUIRE(g.totalErrors()   == 2u);
    REQUIRE(g.totalWarnings() == 5u);
}

TEST_CASE("BuildTaskGraph pushOutputLine appends to log", "[phase_g][g6][build][log]") {
    BuildTaskGraph g;
    BuildTaskId id = g.addTask("Build");
    g.pushOutputLine(id, BuildLineType::Info,    "Compiling file.cpp...");
    g.pushOutputLine(id, BuildLineType::Warning, "Deprecated API used");
    g.pushOutputLine(id, BuildLineType::Error,   "Undefined symbol 'Foo'");
    REQUIRE(g.outputLineCount() == 3u);
}

TEST_CASE("BuildTaskGraph output callback fires per line", "[phase_g][g6][build][callback]") {
    BuildTaskGraph g;
    BuildTaskId id = g.addTask("Build");
    int callCount = 0;
    g.setOutputCallback([&](const BuildOutputLine&) { ++callCount; });
    g.pushOutputLine(id, BuildLineType::Info, "Line 1");
    g.pushOutputLine(id, BuildLineType::Info, "Line 2");
    REQUIRE(callCount == 2);
}

TEST_CASE("BuildTaskGraph summarize counts success/failed/skipped", "[phase_g][g6][build][summary]") {
    BuildTaskGraph g;
    BuildTaskId t1 = g.addTask("A");
    BuildTaskId t2 = g.addTask("B");
    BuildTaskId t3 = g.addTask("C");
    g.setTaskState(t1, BuildTaskState::Succeeded, 100.f);
    g.setTaskState(t2, BuildTaskState::Failed,    50.f);
    g.setTaskState(t3, BuildTaskState::Skipped,   0.f);
    BuildGraphResult r = g.summarize();
    REQUIRE(r.tasksSucceeded == 1u);
    REQUIRE(r.tasksFailed    == 1u);
    REQUIRE(r.tasksSkipped   == 1u);
    REQUIRE_FALSE(r.success);
    REQUIRE(r.totalDurationMs == Approx(150.f));
}

TEST_CASE("BuildTaskGraph resetAllToState clears log", "[phase_g][g6][build][reset]") {
    BuildTaskGraph g;
    BuildTaskId id = g.addTask("Build");
    g.setTaskState(id, BuildTaskState::Succeeded, 100.f);
    g.pushOutputLine(id, BuildLineType::Info, "Done");
    g.resetAllToState(BuildTaskState::Pending);
    REQUIRE(g.findTask(id)->state == BuildTaskState::Pending);
    REQUIRE(g.outputLineCount() == 0u);
}

TEST_CASE("BuildTaskState names are correct", "[phase_g][g6][build][types]") {
    REQUIRE(std::string(buildTaskStateName(BuildTaskState::Pending))   == "Pending");
    REQUIRE(std::string(buildTaskStateName(BuildTaskState::Running))   == "Running");
    REQUIRE(std::string(buildTaskStateName(BuildTaskState::Succeeded)) == "Succeeded");
    REQUIRE(std::string(buildTaskStateName(BuildTaskState::Failed))    == "Failed");
    REQUIRE(std::string(buildTaskStateName(BuildTaskState::Skipped))   == "Skipped");
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.7 — AIRequestContext
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("AIRequestContext default state has no context", "[phase_g][g7][ai]") {
    AIRequestContext ctx;
    REQUIRE_FALSE(ctx.hasContext());
    REQUIRE(ctx.messageCount()        == 0u);
    REQUIRE(ctx.proposalCount()       == 0u);
    REQUIRE(ctx.pendingProposalCount() == 0u);
    REQUIRE(ctx.snippetCount()         == 0u);
}

TEST_CASE("AIRequestContext setActiveDocumentId sets context", "[phase_g][g7][ai][context]") {
    AIRequestContext ctx;
    ctx.setActiveDocumentId("doc-001");
    REQUIRE(ctx.activeDocumentId() == "doc-001");
    REQUIRE(ctx.hasContext());
}

TEST_CASE("AIRequestContext addContextError and clearContextErrors", "[phase_g][g7][ai][context]") {
    AIRequestContext ctx;
    ctx.addContextError("Undefined symbol 'Foo'");
    ctx.addContextError("Missing return statement");
    REQUIRE(ctx.contextErrors().size() == 2u);
    ctx.clearContextErrors();
    REQUIRE(ctx.contextErrors().empty());
}

TEST_CASE("AIRequestContext clearContext resets all context", "[phase_g][g7][ai][context]") {
    AIRequestContext ctx;
    ctx.setActiveDocumentId("doc-001");
    ctx.setActiveObjectId("entity-42");
    ctx.addContextError("err");
    ctx.clearContext();
    REQUIRE_FALSE(ctx.hasContext());
    REQUIRE(ctx.contextErrors().empty());
}

TEST_CASE("AIRequestContext submitUserMessage records message", "[phase_g][g7][ai][message]") {
    AIRequestContext ctx;
    uint32_t id = ctx.submitUserMessage("How do I add a physics component?");
    REQUIRE(id != 0u);
    REQUIRE(ctx.messageCount() == 1u);
    const auto* m = ctx.findMessage(id);
    REQUIRE(m != nullptr);
    REQUIRE(m->role    == AIMessageRole::User);
    REQUIRE(m->content == "How do I add a physics component?");
}

TEST_CASE("AIRequestContext submitAssistantMessage records message", "[phase_g][g7][ai][message]") {
    AIRequestContext ctx;
    ctx.submitUserMessage("Hello");
    uint32_t id = ctx.submitAssistantMessage("I can help with that.");
    REQUIRE(ctx.messageCount() == 2u);
    const auto* m = ctx.findMessage(id);
    REQUIRE(m != nullptr);
    REQUIRE(m->role == AIMessageRole::Assistant);
}

TEST_CASE("AIRequestContext message callback fires per message", "[phase_g][g7][ai][callback]") {
    AIRequestContext ctx;
    int count = 0;
    ctx.setMessageCallback([&](const AIMessage&) { ++count; });
    ctx.submitUserMessage("Q1");
    ctx.submitAssistantMessage("A1");
    REQUIRE(count == 2);
}

TEST_CASE("AIRequestContext clearConversation empties messages", "[phase_g][g7][ai][message]") {
    AIRequestContext ctx;
    ctx.submitUserMessage("Hello");
    ctx.clearConversation();
    REQUIRE(ctx.messageCount() == 0u);
}

TEST_CASE("AIRequestContext addProposal records pending proposal", "[phase_g][g7][ai][proposal]") {
    AIRequestContext ctx;
    std::vector<DiffProposalLine> lines = {
        { DiffLineAction::Context, "int x = 0;" },
        { DiffLineAction::Remove,  "int y = 0;" },
        { DiffLineAction::Add,     "float y = 0.f;" },
    };
    uint32_t pid = ctx.addProposal("src/game.cpp", std::move(lines));
    REQUIRE(pid != 0u);
    REQUIRE(ctx.proposalCount()        == 1u);
    REQUIRE(ctx.pendingProposalCount() == 1u);
    const auto* p = ctx.findProposal(pid);
    REQUIRE(p != nullptr);
    REQUIRE(p->status       == DiffProposalStatus::Pending);
    REQUIRE(p->filename     == "src/game.cpp");
    REQUIRE(p->addedLines()  == 1u);
    REQUIRE(p->removedLines() == 1u);
}

TEST_CASE("AIRequestContext applyProposal changes status to Applied", "[phase_g][g7][ai][proposal]") {
    AIRequestContext ctx;
    uint32_t pid = ctx.addProposal("file.cpp", {});
    REQUIRE(ctx.applyProposal(pid));
    REQUIRE(ctx.findProposal(pid)->status == DiffProposalStatus::Applied);
    REQUIRE(ctx.pendingProposalCount() == 0u);
}

TEST_CASE("AIRequestContext rejectProposal changes status to Rejected", "[phase_g][g7][ai][proposal]") {
    AIRequestContext ctx;
    uint32_t pid = ctx.addProposal("file.cpp", {});
    REQUIRE(ctx.rejectProposal(pid));
    REQUIRE(ctx.findProposal(pid)->status == DiffProposalStatus::Rejected);
    REQUIRE(ctx.pendingProposalCount() == 0u);
}

TEST_CASE("AIRequestContext applyProposal on already-applied returns false", "[phase_g][g7][ai][proposal]") {
    AIRequestContext ctx;
    uint32_t pid = ctx.addProposal("file.cpp", {});
    ctx.applyProposal(pid);
    REQUIRE_FALSE(ctx.applyProposal(pid));
}

TEST_CASE("AIRequestContext proposal callback fires on add", "[phase_g][g7][ai][callback]") {
    AIRequestContext ctx;
    int count = 0;
    ctx.setProposalCallback([&](const DiffProposal&) { ++count; });
    ctx.addProposal("a.cpp", {});
    ctx.addProposal("b.cpp", {});
    REQUIRE(count == 2);
}

TEST_CASE("AIRequestContext addCodexSnippet stores snippet", "[phase_g][g7][ai][codex]") {
    AIRequestContext ctx;
    uint32_t id = ctx.addCodexSnippet("Spawn entity", "cpp", "world.spawn(e);", "ecs,spawn");
    REQUIRE(id != 0u);
    REQUIRE(ctx.snippetCount() == 1u);
    const auto* s = ctx.findSnippet(id);
    REQUIRE(s != nullptr);
    REQUIRE(s->title    == "Spawn entity");
    REQUIRE(s->language == "cpp");
    REQUIRE(s->tags     == "ecs,spawn");
}

TEST_CASE("AIRequestContext removeCodexSnippet removes snippet", "[phase_g][g7][ai][codex]") {
    AIRequestContext ctx;
    uint32_t id = ctx.addCodexSnippet("Snippet", "lua", "print('hi')");
    REQUIRE(ctx.removeCodexSnippet(id));
    REQUIRE(ctx.snippetCount() == 0u);
}

TEST_CASE("AIMessageRole names are correct", "[phase_g][g7][ai][types]") {
    REQUIRE(std::string(aiMessageRoleName(AIMessageRole::User))      == "User");
    REQUIRE(std::string(aiMessageRoleName(AIMessageRole::Assistant)) == "Assistant");
    REQUIRE(std::string(aiMessageRoleName(AIMessageRole::System))    == "System");
}

TEST_CASE("DiffLineAction names are correct", "[phase_g][g7][ai][types]") {
    REQUIRE(std::string(diffLineActionName(DiffLineAction::Context)) == "Context");
    REQUIRE(std::string(diffLineActionName(DiffLineAction::Add))     == "Add");
    REQUIRE(std::string(diffLineActionName(DiffLineAction::Remove))  == "Remove");
}

TEST_CASE("DiffProposalStatus names are correct", "[phase_g][g7][ai][types]") {
    REQUIRE(std::string(diffProposalStatusName(DiffProposalStatus::Pending))  == "Pending");
    REQUIRE(std::string(diffProposalStatusName(DiffProposalStatus::Applied))  == "Applied");
    REQUIRE(std::string(diffProposalStatusName(DiffProposalStatus::Rejected)) == "Rejected");
}

// ═══════════════════════════════════════════════════════════════════════════
//  G.8 — DataTableDocument
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("DataTableDocument default state is clean", "[phase_g][g8][data]") {
    DataTableDocument doc;
    REQUIRE_FALSE(doc.isDirty());
    REQUIRE(doc.columnCount() == 0u);
    REQUIRE(doc.rowCount()    == 0u);
}

TEST_CASE("DataTableDocument construction with table name", "[phase_g][g8][data]") {
    DataTableDocument doc("ItemTable");
    REQUIRE(doc.tableName() == "ItemTable");
}

TEST_CASE("DataTableDocument addColumn returns valid ID", "[phase_g][g8][data][column]") {
    DataTableDocument doc("T");
    DataColumnId id = doc.addColumn("name", DataColumnType::String, "Unknown");
    REQUIRE(id != kInvalidDataColumnId);
    REQUIRE(doc.columnCount() == 1u);
    REQUIRE(doc.isDirty());
    const auto* col = doc.findColumn(id);
    REQUIRE(col != nullptr);
    REQUIRE(col->name == "name");
    REQUIRE(col->type == DataColumnType::String);
}

TEST_CASE("DataTableDocument removeColumn removes cell data", "[phase_g][g8][data][column]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("name", DataColumnType::String);
    DataRowId    rowId = doc.addRow();
    doc.setCell(rowId, colId, "Sword");
    REQUIRE(doc.getCell(rowId, colId) == "Sword");
    REQUIRE(doc.removeColumn(colId));
    REQUIRE(doc.columnCount() == 0u);
    // Cell data should be gone (defaults to empty)
    REQUIRE(doc.getCell(rowId, colId).empty());
}

TEST_CASE("DataTableDocument findColumnByName", "[phase_g][g8][data][column]") {
    DataTableDocument doc("T");
    DataColumnId id = doc.addColumn("damage", DataColumnType::Float);
    REQUIRE(doc.findColumnByName("damage") == id);
    REQUIRE(doc.findColumnByName("missing") == kInvalidDataColumnId);
}

TEST_CASE("DataTableDocument addEnumOption on enum column", "[phase_g][g8][data][column]") {
    DataTableDocument doc("T");
    DataColumnId id = doc.addColumn("rarity", DataColumnType::Enum);
    REQUIRE(doc.addEnumOption(id, "Common"));
    REQUIRE(doc.addEnumOption(id, "Rare"));
    REQUIRE(doc.addEnumOption(id, "Legendary"));
    REQUIRE(doc.findColumn(id)->enumOptions.size() == 3u);
}

TEST_CASE("DataTableDocument addEnumOption on non-Enum column fails", "[phase_g][g8][data][column]") {
    DataTableDocument doc("T");
    DataColumnId id = doc.addColumn("name", DataColumnType::String);
    REQUIRE_FALSE(doc.addEnumOption(id, "Option"));
}

TEST_CASE("DataTableDocument addRow initializes cells with defaults", "[phase_g][g8][data][row]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("level", DataColumnType::Int, "1");
    DataRowId    rowId = doc.addRow();
    REQUIRE(rowId != kInvalidDataRowId);
    REQUIRE(doc.rowCount() == 1u);
    REQUIRE(doc.getCell(rowId, colId) == "1");
}

TEST_CASE("DataTableDocument removeRow", "[phase_g][g8][data][row]") {
    DataTableDocument doc("T");
    DataRowId rowId = doc.addRow();
    REQUIRE(doc.removeRow(rowId));
    REQUIRE(doc.rowCount() == 0u);
}

TEST_CASE("DataTableDocument duplicateRow copies cell values", "[phase_g][g8][data][row]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("hp", DataColumnType::Int, "100");
    DataRowId    src   = doc.addRow();
    doc.setCell(src, colId, "200");
    DataRowId copy = doc.duplicateRow(src);
    REQUIRE(copy != kInvalidDataRowId);
    REQUIRE(copy != src);
    REQUIRE(doc.getCell(copy, colId) == "200");
    REQUIRE(doc.rowCount() == 2u);
}

TEST_CASE("DataTableDocument setCell and getCell", "[phase_g][g8][data][cell]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("name", DataColumnType::String);
    DataRowId    rowId = doc.addRow();
    doc.clearDirty();
    REQUIRE(doc.setCell(rowId, colId, "IronSword"));
    REQUIRE(doc.getCell(rowId, colId) == "IronSword");
    REQUIRE(doc.isDirty());
}

TEST_CASE("DataTableDocument setCell on invalid row/column returns false", "[phase_g][g8][data][cell]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("name", DataColumnType::String);
    REQUIRE_FALSE(doc.setCell(999u, colId, "x"));
    DataRowId rowId = doc.addRow();
    REQUIRE_FALSE(doc.setCell(rowId, 999u, "x"));
}

TEST_CASE("DataTableDocument validateCell Ok for valid values", "[phase_g][g8][data][validation]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("name", DataColumnType::String);
    DataRowId    rowId = doc.addRow();
    doc.setCell(rowId, colId, "Sword");
    auto r = doc.validateCell(rowId, colId);
    REQUIRE(r.ok());
}

TEST_CASE("DataTableDocument validateCell detects missing required field", "[phase_g][g8][data][validation]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("name", DataColumnType::String, "");
    doc.findColumn(colId)->required = true;
    DataRowId rowId = doc.addRow();
    doc.setCell(rowId, colId, ""); // empty required field
    auto r = doc.validateCell(rowId, colId);
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.status == DataCellValidation::Empty);
}

TEST_CASE("DataTableDocument validateCell detects invalid enum", "[phase_g][g8][data][validation]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("rarity", DataColumnType::Enum);
    doc.addEnumOption(colId, "Common");
    doc.addEnumOption(colId, "Rare");
    DataRowId rowId = doc.addRow();
    doc.setCell(rowId, colId, "Legendary"); // not an option
    auto r = doc.validateCell(rowId, colId);
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.status == DataCellValidation::InvalidEnum);
}

TEST_CASE("DataTableDocument validateAll counts all validation errors", "[phase_g][g8][data][validation]") {
    DataTableDocument doc("T");
    DataColumnId colId = doc.addColumn("rarity", DataColumnType::Enum);
    doc.addEnumOption(colId, "Common");
    DataRowId r1 = doc.addRow();
    DataRowId r2 = doc.addRow();
    doc.setCell(r1, colId, "Invalid1");
    doc.setCell(r2, colId, "Invalid2");
    REQUIRE(doc.validateAll() == 2u);
}

TEST_CASE("DataTableDocument exportCsv produces header and rows", "[phase_g][g8][data][csv]") {
    DataTableDocument doc("T");
    DataColumnId nameId  = doc.addColumn("name",  DataColumnType::String);
    DataColumnId levelId = doc.addColumn("level", DataColumnType::Int, "1");
    DataRowId rowId = doc.addRow();
    doc.setCell(rowId, nameId,  "Sword");
    doc.setCell(rowId, levelId, "5");
    std::string csv = doc.exportCsv();
    REQUIRE(csv.find("name")  != std::string::npos);
    REQUIRE(csv.find("level") != std::string::npos);
    REQUIRE(csv.find("Sword") != std::string::npos);
    REQUIRE(csv.find("5")     != std::string::npos);
}

TEST_CASE("DataTableDocument importCsv marks dirty", "[phase_g][g8][data][csv]") {
    DataTableDocument doc("T");
    doc.clearDirty();
    REQUIRE(doc.importCsv("name,level\nSword,5\n"));
    REQUIRE(doc.isDirty());
}

TEST_CASE("DataTableDocument save with path clears dirty", "[phase_g][g8][data][save]") {
    DataTableDocument doc("T");
    doc.setAssetPath("/data/items.json");
    doc.addRow();
    REQUIRE(doc.save());
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("DataTableDocument save without path fails", "[phase_g][g8][data][save]") {
    DataTableDocument doc("T");
    REQUIRE_FALSE(doc.save());
}

TEST_CASE("DataTableDocument serialize produces non-empty output", "[phase_g][g8][data][serialize]") {
    DataTableDocument doc("ItemTable");
    doc.addColumn("name", DataColumnType::String);
    doc.addRow();
    std::string json = doc.serialize();
    REQUIRE_FALSE(json.empty());
    REQUIRE(json.find("ItemTable") != std::string::npos);
}

TEST_CASE("DataColumnType names are correct", "[phase_g][g8][data][types]") {
    REQUIRE(std::string(dataColumnTypeName(DataColumnType::String)) == "String");
    REQUIRE(std::string(dataColumnTypeName(DataColumnType::Int))    == "Int");
    REQUIRE(std::string(dataColumnTypeName(DataColumnType::Float))  == "Float");
    REQUIRE(std::string(dataColumnTypeName(DataColumnType::Bool))   == "Bool");
    REQUIRE(std::string(dataColumnTypeName(DataColumnType::Enum))   == "Enum");
}

TEST_CASE("DataCellValidation names are correct", "[phase_g][g8][data][types]") {
    REQUIRE(std::string(dataCellValidationName(DataCellValidation::Ok))           == "Ok");
    REQUIRE(std::string(dataCellValidationName(DataCellValidation::TypeMismatch)) == "TypeMismatch");
    REQUIRE(std::string(dataCellValidationName(DataCellValidation::InvalidEnum))  == "InvalidEnum");
    REQUIRE(std::string(dataCellValidationName(DataCellValidation::Empty))        == "Empty");
}
