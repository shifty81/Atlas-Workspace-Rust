#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/StaticMeshEditor.h"
#include "NF/Editor/MeshOptimizer.h"
#include "NF/Editor/LODEditor.h"

using namespace NF;

// ── S87: LODEditor + MeshOptimizer + StaticMeshEditor ───────────

// ── LODEditor ────────────────────────────────────────────────────

TEST_CASE("LODTransitionMode names are correct", "[Editor][S87]") {
    REQUIRE(std::string(lodTransitionModeName(LODTransitionMode::Discrete))  == "Discrete");
    REQUIRE(std::string(lodTransitionModeName(LODTransitionMode::Blend))     == "Blend");
    REQUIRE(std::string(lodTransitionModeName(LODTransitionMode::Dithered))  == "Dithered");
    REQUIRE(std::string(lodTransitionModeName(LODTransitionMode::CrossFade)) == "CrossFade");
    REQUIRE(std::string(lodTransitionModeName(LODTransitionMode::Instant))   == "Instant");
}

TEST_CASE("LODReductionMethod names are correct", "[Editor][S87]") {
    REQUIRE(std::string(lodReductionMethodName(LODReductionMethod::None))        == "None");
    REQUIRE(std::string(lodReductionMethodName(LODReductionMethod::Quadric))     == "Quadric");
    REQUIRE(std::string(lodReductionMethodName(LODReductionMethod::Progressive)) == "Progressive");
    REQUIRE(std::string(lodReductionMethodName(LODReductionMethod::Impostor))    == "Impostor");
    REQUIRE(std::string(lodReductionMethodName(LODReductionMethod::Billboard))   == "Billboard");
}

TEST_CASE("LODBias names are correct", "[Editor][S87]") {
    REQUIRE(std::string(lodBiasName(LODBias::VeryHigh)) == "VeryHigh");
    REQUIRE(std::string(lodBiasName(LODBias::High))     == "High");
    REQUIRE(std::string(lodBiasName(LODBias::Medium))   == "Medium");
    REQUIRE(std::string(lodBiasName(LODBias::Low))      == "Low");
    REQUIRE(std::string(lodBiasName(LODBias::VeryLow))  == "VeryLow");
}

TEST_CASE("LODLevel stores properties", "[Editor][S87]") {
    LODLevel lod(1, 50.0f);
    lod.setTriangleCount(2000);
    lod.setReductionPct(50.0f);
    lod.setMethod(LODReductionMethod::Quadric);
    lod.setTransitionMode(LODTransitionMode::Blend);

    REQUIRE(lod.level()          == 1);
    REQUIRE(lod.screenSizePct()  == 50.0f);
    REQUIRE(lod.triangleCount()  == 2000);
    REQUIRE(lod.reductionPct()   == 50.0f);
    REQUIRE(lod.method()         == LODReductionMethod::Quadric);
    REQUIRE(lod.transitionMode() == LODTransitionMode::Blend);
    REQUIRE(lod.isEnabled());
}

TEST_CASE("LODEditor addLevel and findLevel", "[Editor][S87]") {
    LODEditor editor;
    LODLevel lod0(0, 100.0f);
    LODLevel lod1(1, 50.0f);
    REQUIRE(editor.addLevel(lod0));
    REQUIRE(editor.addLevel(lod1));
    REQUIRE(editor.levelCount() == 2);
    REQUIRE(editor.findLevel(1) != nullptr);
}

TEST_CASE("LODEditor rejects duplicate level index", "[Editor][S87]") {
    LODEditor editor;
    LODLevel lod(0, 100.0f);
    editor.addLevel(lod);
    REQUIRE_FALSE(editor.addLevel(lod));
}

TEST_CASE("LODEditor remove and countByMethod", "[Editor][S87]") {
    LODEditor editor;
    LODLevel l0(0, 100.0f); l0.setMethod(LODReductionMethod::Quadric);
    LODLevel l1(1, 50.0f);  l1.setMethod(LODReductionMethod::Progressive);
    LODLevel l2(2, 25.0f);  l2.setMethod(LODReductionMethod::Quadric);
    editor.addLevel(l0); editor.addLevel(l1); editor.addLevel(l2);
    REQUIRE(editor.countByMethod(LODReductionMethod::Quadric) == 2);
    editor.removeLevel(0);
    REQUIRE(editor.levelCount() == 2);
}

TEST_CASE("LODEditor bias and autoGenerate", "[Editor][S87]") {
    LODEditor editor;
    editor.setBias(LODBias::High);
    editor.setAutoGenerate(true);
    REQUIRE(editor.bias() == LODBias::High);
    REQUIRE(editor.autoGenerate());
}

TEST_CASE("LODEditor MAX_LEVELS is 8", "[Editor][S87]") {
    REQUIRE(LODEditor::MAX_LEVELS == 8);
}

// ── MeshOptimizer ────────────────────────────────────────────────

TEST_CASE("MeshOptimizeTarget names are correct", "[Editor][S87]") {
    REQUIRE(std::string(meshOptimizeTargetName(MeshOptimizeTarget::Vertices))  == "Vertices");
    REQUIRE(std::string(meshOptimizeTargetName(MeshOptimizeTarget::Triangles)) == "Triangles");
    REQUIRE(std::string(meshOptimizeTargetName(MeshOptimizeTarget::DrawCalls)) == "DrawCalls");
    REQUIRE(std::string(meshOptimizeTargetName(MeshOptimizeTarget::Memory))    == "Memory");
    REQUIRE(std::string(meshOptimizeTargetName(MeshOptimizeTarget::All))       == "All");
}

TEST_CASE("MeshWeldMode names are correct", "[Editor][S87]") {
    REQUIRE(std::string(meshWeldModeName(MeshWeldMode::None))       == "None");
    REQUIRE(std::string(meshWeldModeName(MeshWeldMode::ByPosition)) == "ByPosition");
    REQUIRE(std::string(meshWeldModeName(MeshWeldMode::ByNormal))   == "ByNormal");
    REQUIRE(std::string(meshWeldModeName(MeshWeldMode::ByUV))       == "ByUV");
    REQUIRE(std::string(meshWeldModeName(MeshWeldMode::All))        == "All");
}

TEST_CASE("MeshOptimizeStatus names are correct", "[Editor][S87]") {
    REQUIRE(std::string(meshOptimizeStatusName(MeshOptimizeStatus::Idle))      == "Idle");
    REQUIRE(std::string(meshOptimizeStatusName(MeshOptimizeStatus::Running))   == "Running");
    REQUIRE(std::string(meshOptimizeStatusName(MeshOptimizeStatus::Done))      == "Done");
    REQUIRE(std::string(meshOptimizeStatusName(MeshOptimizeStatus::Failed))    == "Failed");
    REQUIRE(std::string(meshOptimizeStatusName(MeshOptimizeStatus::Cancelled)) == "Cancelled");
}

TEST_CASE("MeshOptimizeJob stores properties and status", "[Editor][S87]") {
    MeshOptimizeJob job("RockMesh", MeshOptimizeTarget::Triangles);
    job.setWeldMode(MeshWeldMode::ByPosition);
    job.setStatus(MeshOptimizeStatus::Running);
    job.setOriginalTriCount(10000);
    job.setResultTriCount(5000);

    REQUIRE(job.meshName()     == "RockMesh");
    REQUIRE(job.target()       == MeshOptimizeTarget::Triangles);
    REQUIRE(job.weldMode()     == MeshWeldMode::ByPosition);
    REQUIRE(job.isRunning());
    REQUIRE(job.originalTris() == 10000);
    REQUIRE(job.resultTris()   == 5000);
}

TEST_CASE("MeshOptimizerPanel add remove and complete count", "[Editor][S87]") {
    MeshOptimizerPanel panel;
    MeshOptimizeJob j1("Mesh1", MeshOptimizeTarget::Triangles);
    j1.setStatus(MeshOptimizeStatus::Done);
    MeshOptimizeJob j2("Mesh2", MeshOptimizeTarget::Memory);
    panel.addJob(j1);
    panel.addJob(j2);
    REQUIRE(panel.jobCount()       == 2);
    REQUIRE(panel.completedCount() == 1);
    panel.removeJob("Mesh1");
    REQUIRE(panel.jobCount() == 1);
}

TEST_CASE("MeshOptimizerPanel rejects duplicate meshName", "[Editor][S87]") {
    MeshOptimizerPanel panel;
    MeshOptimizeJob j("Mesh", MeshOptimizeTarget::All);
    panel.addJob(j);
    REQUIRE_FALSE(panel.addJob(j));
}

TEST_CASE("MeshOptimizerPanel MAX_JOBS is 128", "[Editor][S87]") {
    REQUIRE(MeshOptimizerPanel::MAX_JOBS == 128);
}

// ── StaticMeshEditor ─────────────────────────────────────────────

TEST_CASE("MeshShadingMode names are correct", "[Editor][S87]") {
    REQUIRE(std::string(meshShadingModeName(MeshShadingMode::Flat))      == "Flat");
    REQUIRE(std::string(meshShadingModeName(MeshShadingMode::Smooth))    == "Smooth");
    REQUIRE(std::string(meshShadingModeName(MeshShadingMode::Wireframe)) == "Wireframe");
    REQUIRE(std::string(meshShadingModeName(MeshShadingMode::Normals))   == "Normals");
    REQUIRE(std::string(meshShadingModeName(MeshShadingMode::UV))        == "UV");
}

TEST_CASE("MeshColliderShape names are correct", "[Editor][S87]") {
    REQUIRE(std::string(meshColliderShapeName(MeshColliderShape::None))         == "None");
    REQUIRE(std::string(meshColliderShapeName(MeshColliderShape::Box))          == "Box");
    REQUIRE(std::string(meshColliderShapeName(MeshColliderShape::ConvexHull))   == "ConvexHull");
    REQUIRE(std::string(meshColliderShapeName(MeshColliderShape::TriangleMesh)) == "TriangleMesh");
}

TEST_CASE("MeshImportScale names are correct", "[Editor][S87]") {
    REQUIRE(std::string(meshImportScaleName(MeshImportScale::CM))     == "CM");
    REQUIRE(std::string(meshImportScaleName(MeshImportScale::M))      == "M");
    REQUIRE(std::string(meshImportScaleName(MeshImportScale::Inches)) == "Inches");
    REQUIRE(std::string(meshImportScaleName(MeshImportScale::Feet))   == "Feet");
}

TEST_CASE("StaticMeshAsset stores properties", "[Editor][S87]") {
    StaticMeshAsset mesh("CubeMesh");
    mesh.setVertexCount(24);
    mesh.setTriangleCount(12);
    mesh.setSubmeshCount(2);
    mesh.setShadingMode(MeshShadingMode::Smooth);
    mesh.setColliderShape(MeshColliderShape::Box);
    mesh.setCastsShadow(true);
    mesh.setDirty(true);

    REQUIRE(mesh.name()          == "CubeMesh");
    REQUIRE(mesh.vertexCount()   == 24);
    REQUIRE(mesh.triangleCount() == 12);
    REQUIRE(mesh.submeshCount()  == 2);
    REQUIRE(mesh.hasCollider());
    REQUIRE(mesh.castsShadow());
    REQUIRE(mesh.isDirty());
}

TEST_CASE("StaticMeshEditor add setActive and remove", "[Editor][S87]") {
    StaticMeshEditor editor;
    StaticMeshAsset m1("MeshA");
    StaticMeshAsset m2("MeshB");
    editor.addMesh(m1);
    editor.addMesh(m2);
    REQUIRE(editor.meshCount() == 2);
    REQUIRE(editor.setActiveMesh("MeshA"));
    REQUIRE(editor.activeMesh() == "MeshA");
    editor.removeMesh("MeshA");
    REQUIRE(editor.activeMesh().empty());
}

TEST_CASE("StaticMeshEditor rejects duplicate name", "[Editor][S87]") {
    StaticMeshEditor editor;
    StaticMeshAsset m("MeshX");
    editor.addMesh(m);
    REQUIRE_FALSE(editor.addMesh(m));
}

TEST_CASE("StaticMeshEditor countByShading and colliderCount", "[Editor][S87]") {
    StaticMeshEditor editor;
    StaticMeshAsset m1("A"); m1.setShadingMode(MeshShadingMode::Smooth); m1.setColliderShape(MeshColliderShape::Box);
    StaticMeshAsset m2("B"); m2.setShadingMode(MeshShadingMode::Wireframe);
    StaticMeshAsset m3("C"); m3.setShadingMode(MeshShadingMode::Smooth); m3.setColliderShape(MeshColliderShape::Sphere);
    editor.addMesh(m1); editor.addMesh(m2); editor.addMesh(m3);
    REQUIRE(editor.countByShading(MeshShadingMode::Smooth)    == 2);
    REQUIRE(editor.countByShading(MeshShadingMode::Wireframe) == 1);
    REQUIRE(editor.colliderCount() == 2);
}

TEST_CASE("StaticMeshEditor MAX_MESHES is 256", "[Editor][S87]") {
    REQUIRE(StaticMeshEditor::MAX_MESHES == 256);
}
