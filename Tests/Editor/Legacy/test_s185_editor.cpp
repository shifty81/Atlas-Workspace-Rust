// S185 editor tests: LODEditorV1, MeshBatchEditorV1, ProceduralMeshEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LODEditorV1.h"
#include "NF/Editor/MeshBatchEditorV1.h"
#include "NF/Editor/ProceduralMeshEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── LODEditorV1 ──────────────────────────────────────────────────────────────

TEST_CASE("Lodev1Level validity", "[Editor][S185]") {
    Lodev1Level l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "LOD0";
    REQUIRE(l.isValid());
}

TEST_CASE("LODEditorV1 addLevel and levelCount", "[Editor][S185]") {
    LODEditorV1 le;
    REQUIRE(le.levelCount() == 0);
    Lodev1Level l; l.id = 1; l.name = "LOD0";
    REQUIRE(le.addLevel(l));
    REQUIRE(le.levelCount() == 1);
}

TEST_CASE("LODEditorV1 addLevel invalid fails", "[Editor][S185]") {
    LODEditorV1 le;
    REQUIRE(!le.addLevel(Lodev1Level{}));
}

TEST_CASE("LODEditorV1 addLevel duplicate fails", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Level l; l.id = 1; l.name = "A";
    le.addLevel(l);
    REQUIRE(!le.addLevel(l));
}

TEST_CASE("LODEditorV1 removeLevel", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Level l; l.id = 2; l.name = "B";
    le.addLevel(l);
    REQUIRE(le.removeLevel(2));
    REQUIRE(le.levelCount() == 0);
    REQUIRE(!le.removeLevel(2));
}

TEST_CASE("LODEditorV1 findLevel", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Level l; l.id = 3; l.name = "C";
    le.addLevel(l);
    REQUIRE(le.findLevel(3) != nullptr);
    REQUIRE(le.findLevel(99) == nullptr);
}

TEST_CASE("LODEditorV1 addGroup and groupCount", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Group g; g.id = 1; g.name = "Trees";
    REQUIRE(le.addGroup(g));
    REQUIRE(le.groupCount() == 1);
}

TEST_CASE("LODEditorV1 removeGroup", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Group g; g.id = 1; g.name = "Rocks";
    le.addGroup(g);
    REQUIRE(le.removeGroup(1));
    REQUIRE(le.groupCount() == 0);
}

TEST_CASE("LODEditorV1 activeCount", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Level l1; l1.id = 1; l1.name = "A"; l1.state = Lodev1LodState::Active;
    Lodev1Level l2; l2.id = 2; l2.name = "B";
    le.addLevel(l1); le.addLevel(l2);
    REQUIRE(le.activeCount() == 1);
}

TEST_CASE("LODEditorV1 countByStrategy", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Level l1; l1.id = 1; l1.name = "A"; l1.strategy = Lodev1LodStrategy::Distance;
    Lodev1Level l2; l2.id = 2; l2.name = "B"; l2.strategy = Lodev1LodStrategy::ScreenSize;
    Lodev1Level l3; l3.id = 3; l3.name = "C"; l3.strategy = Lodev1LodStrategy::Distance;
    le.addLevel(l1); le.addLevel(l2); le.addLevel(l3);
    REQUIRE(le.countByStrategy(Lodev1LodStrategy::Distance) == 2);
    REQUIRE(le.countByStrategy(Lodev1LodStrategy::ScreenSize) == 1);
}

TEST_CASE("LODEditorV1 totalPolyCount", "[Editor][S185]") {
    LODEditorV1 le;
    Lodev1Level l1; l1.id = 1; l1.name = "A"; l1.polyCount = 1000;
    Lodev1Level l2; l2.id = 2; l2.name = "B"; l2.polyCount = 500;
    le.addLevel(l1); le.addLevel(l2);
    REQUIRE(le.totalPolyCount() == 1500);
}

TEST_CASE("LODEditorV1 onChange callback", "[Editor][S185]") {
    LODEditorV1 le;
    uint64_t notified = 0;
    le.setOnChange([&](uint64_t id) { notified = id; });
    Lodev1Level l; l.id = 4; l.name = "D";
    le.addLevel(l);
    REQUIRE(notified == 4);
}

TEST_CASE("Lodev1Level state helpers", "[Editor][S185]") {
    Lodev1Level l; l.id = 1; l.name = "X";
    l.state = Lodev1LodState::Override; REQUIRE(l.isOverride());
    l.state = Lodev1LodState::Disabled; REQUIRE(l.isDisabled());
    l.state = Lodev1LodState::Active;   REQUIRE(l.isActive());
}

TEST_CASE("lodev1LodStrategyName all values", "[Editor][S185]") {
    REQUIRE(std::string(lodev1LodStrategyName(Lodev1LodStrategy::Distance))   == "Distance");
    REQUIRE(std::string(lodev1LodStrategyName(Lodev1LodStrategy::ScreenSize)) == "ScreenSize");
    REQUIRE(std::string(lodev1LodStrategyName(Lodev1LodStrategy::Manual))     == "Manual");
    REQUIRE(std::string(lodev1LodStrategyName(Lodev1LodStrategy::Automatic))  == "Automatic");
}

// ── MeshBatchEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Mbev1Batch validity", "[Editor][S185]") {
    Mbev1Batch b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "StaticBatch";
    REQUIRE(b.isValid());
}

TEST_CASE("MeshBatchEditorV1 addBatch and batchCount", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    REQUIRE(mbe.batchCount() == 0);
    Mbev1Batch b; b.id = 1; b.name = "B1";
    REQUIRE(mbe.addBatch(b));
    REQUIRE(mbe.batchCount() == 1);
}

TEST_CASE("MeshBatchEditorV1 addBatch invalid fails", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    REQUIRE(!mbe.addBatch(Mbev1Batch{}));
}

TEST_CASE("MeshBatchEditorV1 addBatch duplicate fails", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1Batch b; b.id = 1; b.name = "A";
    mbe.addBatch(b);
    REQUIRE(!mbe.addBatch(b));
}

TEST_CASE("MeshBatchEditorV1 removeBatch", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1Batch b; b.id = 2; b.name = "B";
    mbe.addBatch(b);
    REQUIRE(mbe.removeBatch(2));
    REQUIRE(mbe.batchCount() == 0);
    REQUIRE(!mbe.removeBatch(2));
}

TEST_CASE("MeshBatchEditorV1 findBatch", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1Batch b; b.id = 3; b.name = "C";
    mbe.addBatch(b);
    REQUIRE(mbe.findBatch(3) != nullptr);
    REQUIRE(mbe.findBatch(99) == nullptr);
}

TEST_CASE("MeshBatchEditorV1 addGroup and groupCount", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1RenderGroup g; g.id = 1; g.name = "Group1";
    REQUIRE(mbe.addGroup(g));
    REQUIRE(mbe.groupCount() == 1);
}

TEST_CASE("MeshBatchEditorV1 removeGroup", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1RenderGroup g; g.id = 1; g.name = "G";
    mbe.addGroup(g);
    REQUIRE(mbe.removeGroup(1));
    REQUIRE(mbe.groupCount() == 0);
}

TEST_CASE("MeshBatchEditorV1 readyCount", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1Batch b1; b1.id = 1; b1.name = "A"; b1.state = Mbev1BatchState::Ready;
    Mbev1Batch b2; b2.id = 2; b2.name = "B";
    mbe.addBatch(b1); mbe.addBatch(b2);
    REQUIRE(mbe.readyCount() == 1);
}

TEST_CASE("MeshBatchEditorV1 countByMode", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1Batch b1; b1.id = 1; b1.name = "A"; b1.mode = Mbev1BatchMode::Static;
    Mbev1Batch b2; b2.id = 2; b2.name = "B"; b2.mode = Mbev1BatchMode::Instanced;
    Mbev1Batch b3; b3.id = 3; b3.name = "C"; b3.mode = Mbev1BatchMode::Static;
    mbe.addBatch(b1); mbe.addBatch(b2); mbe.addBatch(b3);
    REQUIRE(mbe.countByMode(Mbev1BatchMode::Static) == 2);
    REQUIRE(mbe.countByMode(Mbev1BatchMode::Instanced) == 1);
}

TEST_CASE("MeshBatchEditorV1 totalDrawCalls", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    Mbev1Batch b1; b1.id = 1; b1.name = "A"; b1.drawCalls = 10;
    Mbev1Batch b2; b2.id = 2; b2.name = "B"; b2.drawCalls = 5;
    mbe.addBatch(b1); mbe.addBatch(b2);
    REQUIRE(mbe.totalDrawCalls() == 15);
}

TEST_CASE("MeshBatchEditorV1 onChange callback", "[Editor][S185]") {
    MeshBatchEditorV1 mbe;
    uint64_t notified = 0;
    mbe.setOnChange([&](uint64_t id) { notified = id; });
    Mbev1Batch b; b.id = 6; b.name = "F";
    mbe.addBatch(b);
    REQUIRE(notified == 6);
}

TEST_CASE("Mbev1Batch state helpers", "[Editor][S185]") {
    Mbev1Batch b; b.id = 1; b.name = "X";
    b.state = Mbev1BatchState::Ready;     REQUIRE(b.isReady());
    b.state = Mbev1BatchState::Submitted; REQUIRE(b.isSubmitted());
    b.state = Mbev1BatchState::Failed;    REQUIRE(b.isFailed());
}

TEST_CASE("mbev1BatchModeName all values", "[Editor][S185]") {
    REQUIRE(std::string(mbev1BatchModeName(Mbev1BatchMode::Static))    == "Static");
    REQUIRE(std::string(mbev1BatchModeName(Mbev1BatchMode::Dynamic))   == "Dynamic");
    REQUIRE(std::string(mbev1BatchModeName(Mbev1BatchMode::Skinned))   == "Skinned");
    REQUIRE(std::string(mbev1BatchModeName(Mbev1BatchMode::Instanced)) == "Instanced");
}

// ── ProceduralMeshEditorV1 ───────────────────────────────────────────────────

TEST_CASE("Pmev1Rule validity", "[Editor][S185]") {
    Pmev1Rule r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "Extrude01";
    REQUIRE(r.isValid());
}

TEST_CASE("ProceduralMeshEditorV1 addRule and ruleCount", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    REQUIRE(pme.ruleCount() == 0);
    Pmev1Rule r; r.id = 1; r.name = "R1";
    REQUIRE(pme.addRule(r));
    REQUIRE(pme.ruleCount() == 1);
}

TEST_CASE("ProceduralMeshEditorV1 addRule invalid fails", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    REQUIRE(!pme.addRule(Pmev1Rule{}));
}

TEST_CASE("ProceduralMeshEditorV1 addRule duplicate fails", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Rule r; r.id = 1; r.name = "A";
    pme.addRule(r);
    REQUIRE(!pme.addRule(r));
}

TEST_CASE("ProceduralMeshEditorV1 removeRule", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Rule r; r.id = 2; r.name = "B";
    pme.addRule(r);
    REQUIRE(pme.removeRule(2));
    REQUIRE(pme.ruleCount() == 0);
    REQUIRE(!pme.removeRule(2));
}

TEST_CASE("ProceduralMeshEditorV1 findRule", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Rule r; r.id = 3; r.name = "C";
    pme.addRule(r);
    REQUIRE(pme.findRule(3) != nullptr);
    REQUIRE(pme.findRule(99) == nullptr);
}

TEST_CASE("ProceduralMeshEditorV1 addVariant and variantCount", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Variant v; v.id = 1; v.name = "RoundedV1";
    REQUIRE(pme.addVariant(v));
    REQUIRE(pme.variantCount() == 1);
}

TEST_CASE("ProceduralMeshEditorV1 removeVariant", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Variant v; v.id = 1; v.name = "V";
    pme.addVariant(v);
    REQUIRE(pme.removeVariant(1));
    REQUIRE(pme.variantCount() == 0);
}

TEST_CASE("ProceduralMeshEditorV1 activeCount", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Rule r1; r1.id = 1; r1.name = "A"; r1.state = Pmev1RuleState::Active;
    Pmev1Rule r2; r2.id = 2; r2.name = "B";
    pme.addRule(r1); pme.addRule(r2);
    REQUIRE(pme.activeCount() == 1);
}

TEST_CASE("ProceduralMeshEditorV1 bakedCount", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Rule r1; r1.id = 1; r1.name = "A"; r1.state = Pmev1RuleState::Baked;
    Pmev1Rule r2; r2.id = 2; r2.name = "B"; r2.state = Pmev1RuleState::Active;
    pme.addRule(r1); pme.addRule(r2);
    REQUIRE(pme.bakedCount() == 1);
}

TEST_CASE("ProceduralMeshEditorV1 countByType", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    Pmev1Rule r1; r1.id = 1; r1.name = "A"; r1.ruleType = Pmev1RuleType::Extrude;
    Pmev1Rule r2; r2.id = 2; r2.name = "B"; r2.ruleType = Pmev1RuleType::Bevel;
    Pmev1Rule r3; r3.id = 3; r3.name = "C"; r3.ruleType = Pmev1RuleType::Extrude;
    pme.addRule(r1); pme.addRule(r2); pme.addRule(r3);
    REQUIRE(pme.countByType(Pmev1RuleType::Extrude) == 2);
    REQUIRE(pme.countByType(Pmev1RuleType::Bevel) == 1);
}

TEST_CASE("ProceduralMeshEditorV1 onChange callback", "[Editor][S185]") {
    ProceduralMeshEditorV1 pme;
    uint64_t notified = 0;
    pme.setOnChange([&](uint64_t id) { notified = id; });
    Pmev1Rule r; r.id = 8; r.name = "H";
    pme.addRule(r);
    REQUIRE(notified == 8);
}

TEST_CASE("Pmev1Rule state helpers", "[Editor][S185]") {
    Pmev1Rule r; r.id = 1; r.name = "X";
    r.state = Pmev1RuleState::Active;  REQUIRE(r.isActive());
    r.state = Pmev1RuleState::Preview; REQUIRE(r.isPreview());
    r.state = Pmev1RuleState::Baked;   REQUIRE(r.isBaked());
}

TEST_CASE("pmev1RuleTypeName all values", "[Editor][S185]") {
    REQUIRE(std::string(pmev1RuleTypeName(Pmev1RuleType::Extrude))   == "Extrude");
    REQUIRE(std::string(pmev1RuleTypeName(Pmev1RuleType::Lathe))     == "Lathe");
    REQUIRE(std::string(pmev1RuleTypeName(Pmev1RuleType::Sweep))     == "Sweep");
    REQUIRE(std::string(pmev1RuleTypeName(Pmev1RuleType::Subdivide)) == "Subdivide");
    REQUIRE(std::string(pmev1RuleTypeName(Pmev1RuleType::Weld))      == "Weld");
    REQUIRE(std::string(pmev1RuleTypeName(Pmev1RuleType::Bevel))     == "Bevel");
}
