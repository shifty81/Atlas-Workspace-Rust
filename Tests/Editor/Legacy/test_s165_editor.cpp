// S165 editor tests: AssetTagEditorV1, AssetDependencyV1, AssetMigratorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AssetMigratorV1.h"
#include "NF/Editor/AssetDependencyV1.h"
#include "NF/Editor/AssetTagEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── AssetTagEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Atv1Tag validity", "[Editor][S165]") {
    Atv1Tag t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Character";
    REQUIRE(t.isValid());
}

TEST_CASE("AssetTagEditorV1 addTag and tagCount", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    REQUIRE(ate.tagCount() == 0);
    Atv1Tag t; t.id = 1; t.name = "Hero";
    REQUIRE(ate.addTag(t));
    REQUIRE(ate.tagCount() == 1);
}

TEST_CASE("AssetTagEditorV1 addTag invalid fails", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    REQUIRE(!ate.addTag(Atv1Tag{}));
}

TEST_CASE("AssetTagEditorV1 addTag duplicate fails", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t; t.id = 1; t.name = "A";
    ate.addTag(t);
    REQUIRE(!ate.addTag(t));
}

TEST_CASE("AssetTagEditorV1 removeTag", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t; t.id = 2; t.name = "B";
    ate.addTag(t);
    REQUIRE(ate.removeTag(2));
    REQUIRE(ate.tagCount() == 0);
    REQUIRE(!ate.removeTag(2));
}

TEST_CASE("AssetTagEditorV1 findTag", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t; t.id = 3; t.name = "C";
    ate.addTag(t);
    REQUIRE(ate.findTag(3) != nullptr);
    REQUIRE(ate.findTag(99) == nullptr);
}

TEST_CASE("AssetTagEditorV1 tagAsset and assetCount", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t; t.id = 1; t.name = "X";
    ate.addTag(t);
    REQUIRE(ate.tagAsset("Assets/Mesh/hero.fbx", 1));
    REQUIRE(ate.assetCount() == 1);
}

TEST_CASE("AssetTagEditorV1 tagAsset unknown tag fails", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    REQUIRE(!ate.tagAsset("Assets/tex.png", 99));
}

TEST_CASE("AssetTagEditorV1 untagAsset", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t; t.id = 1; t.name = "X";
    ate.addTag(t);
    ate.tagAsset("Assets/a.png", 1);
    REQUIRE(ate.untagAsset("Assets/a.png", 1));
    REQUIRE(!ate.untagAsset("Assets/a.png", 1));
}

TEST_CASE("AssetTagEditorV1 countByCategory", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t1; t1.id = 1; t1.name = "A"; t1.category = Atv1TagCategory::Type;
    Atv1Tag t2; t2.id = 2; t2.name = "B"; t2.category = Atv1TagCategory::Status;
    ate.addTag(t1); ate.addTag(t2);
    REQUIRE(ate.countByCategory(Atv1TagCategory::Type)   == 1);
    REQUIRE(ate.countByCategory(Atv1TagCategory::Status) == 1);
}

TEST_CASE("AssetTagEditorV1 countByScope", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t1; t1.id = 1; t1.name = "A"; t1.scope = Atv1TagScope::Asset;
    Atv1Tag t2; t2.id = 2; t2.name = "B"; t2.scope = Atv1TagScope::Global;
    ate.addTag(t1); ate.addTag(t2);
    REQUIRE(ate.countByScope(Atv1TagScope::Asset)  == 1);
    REQUIRE(ate.countByScope(Atv1TagScope::Global) == 1);
}

TEST_CASE("AssetTagEditorV1 pinnedCount", "[Editor][S165]") {
    AssetTagEditorV1 ate;
    Atv1Tag t1; t1.id = 1; t1.name = "A"; t1.pinned = true;
    Atv1Tag t2; t2.id = 2; t2.name = "B"; t2.pinned = false;
    ate.addTag(t1); ate.addTag(t2);
    REQUIRE(ate.pinnedCount() == 1);
}

TEST_CASE("atv1TagCategoryName covers all values", "[Editor][S165]") {
    REQUIRE(std::string(atv1TagCategoryName(Atv1TagCategory::Type))   == "Type");
    REQUIRE(std::string(atv1TagCategoryName(Atv1TagCategory::Custom)) == "Custom");
}

// ── AssetDependencyV1 ────────────────────────────────────────────────────────

TEST_CASE("Adv1Node validity", "[Editor][S165]") {
    Adv1Node n;
    REQUIRE(!n.isValid());
    n.id = 1; n.assetPath = "Assets/mat.mat";
    REQUIRE(n.isValid());
}

TEST_CASE("Adv1Node isClean and isMissing", "[Editor][S165]") {
    Adv1Node n; n.id = 1; n.assetPath = "A";
    REQUIRE(n.isClean());
    n.status = Adv1NodeStatus::Missing;
    REQUIRE(n.isMissing());
}

TEST_CASE("Adv1Node addDep and removeDep", "[Editor][S165]") {
    Adv1Node n; n.id = 1; n.assetPath = "A";
    Adv1Dep d; d.targetId = 2; d.type = Adv1DepType::Hard;
    REQUIRE(n.addDep(d));
    REQUIRE(n.deps.size() == 1);
    REQUIRE(!n.addDep(d)); // duplicate
    REQUIRE(n.removeDep(2));
    REQUIRE(n.deps.empty());
}

TEST_CASE("Adv1Node addDep self-reference fails", "[Editor][S165]") {
    Adv1Node n; n.id = 1; n.assetPath = "A";
    Adv1Dep d; d.targetId = 1;
    REQUIRE(!n.addDep(d));
}

TEST_CASE("AssetDependencyV1 addNode and nodeCount", "[Editor][S165]") {
    AssetDependencyV1 adv;
    REQUIRE(adv.nodeCount() == 0);
    Adv1Node n; n.id = 1; n.assetPath = "Assets/mesh.fbx";
    REQUIRE(adv.addNode(n));
    REQUIRE(adv.nodeCount() == 1);
}

TEST_CASE("AssetDependencyV1 addNode invalid fails", "[Editor][S165]") {
    AssetDependencyV1 adv;
    REQUIRE(!adv.addNode(Adv1Node{}));
}

TEST_CASE("AssetDependencyV1 addNode duplicate fails", "[Editor][S165]") {
    AssetDependencyV1 adv;
    Adv1Node n; n.id = 1; n.assetPath = "A";
    adv.addNode(n);
    REQUIRE(!adv.addNode(n));
}

TEST_CASE("AssetDependencyV1 removeNode", "[Editor][S165]") {
    AssetDependencyV1 adv;
    Adv1Node n; n.id = 2; n.assetPath = "B";
    adv.addNode(n);
    REQUIRE(adv.removeNode(2));
    REQUIRE(adv.nodeCount() == 0);
}

TEST_CASE("AssetDependencyV1 addDep and removeDep", "[Editor][S165]") {
    AssetDependencyV1 adv;
    Adv1Node n1; n1.id = 1; n1.assetPath = "A";
    Adv1Node n2; n2.id = 2; n2.assetPath = "B";
    adv.addNode(n1); adv.addNode(n2);
    Adv1Dep d; d.targetId = 2; d.type = Adv1DepType::Hard;
    REQUIRE(adv.addDep(1, d));
    REQUIRE(adv.findNode(1)->deps.size() == 1);
    REQUIRE(adv.removeDep(1, 2));
    REQUIRE(adv.findNode(1)->deps.empty());
}

TEST_CASE("AssetDependencyV1 setStatus cleanCount missingCount", "[Editor][S165]") {
    AssetDependencyV1 adv;
    Adv1Node n1; n1.id = 1; n1.assetPath = "A";
    Adv1Node n2; n2.id = 2; n2.assetPath = "B";
    adv.addNode(n1); adv.addNode(n2);
    adv.setStatus(2, Adv1NodeStatus::Missing);
    REQUIRE(adv.cleanCount()   == 1);
    REQUIRE(adv.missingCount() == 1);
}

TEST_CASE("AssetDependencyV1 countByStatus", "[Editor][S165]") {
    AssetDependencyV1 adv;
    Adv1Node n1; n1.id = 1; n1.assetPath = "A";
    Adv1Node n2; n2.id = 2; n2.assetPath = "B";
    adv.addNode(n1); adv.addNode(n2);
    adv.setStatus(1, Adv1NodeStatus::Broken);
    adv.setStatus(2, Adv1NodeStatus::Broken);
    REQUIRE(adv.countByStatus(Adv1NodeStatus::Broken) == 2);
}

TEST_CASE("adv1DepTypeName covers all values", "[Editor][S165]") {
    REQUIRE(std::string(adv1DepTypeName(Adv1DepType::Hard))     == "Hard");
    REQUIRE(std::string(adv1DepTypeName(Adv1DepType::Circular)) == "Circular");
}

// ── AssetMigratorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Amv1MigrateJob validity", "[Editor][S165]") {
    Amv1MigrateJob j;
    REQUIRE(!j.isValid());
    j.id = 1; j.name = "UpgradeTextures"; j.fromVersion = "1.0"; j.toVersion = "2.0";
    REQUIRE(j.isValid());
}

TEST_CASE("Amv1MigrateJob empty fromVersion invalid", "[Editor][S165]") {
    Amv1MigrateJob j; j.id = 1; j.name = "X"; j.fromVersion = ""; j.toVersion = "2.0";
    REQUIRE(!j.isValid());
}

TEST_CASE("Amv1MigrateJob isDone isRunning hasFailed", "[Editor][S165]") {
    Amv1MigrateJob j; j.id = 1; j.name = "J"; j.fromVersion = "1"; j.toVersion = "2";
    j.status = Amv1MigrateStatus::Done;
    REQUIRE(j.isDone());
    REQUIRE(!j.isRunning());
    j.status = Amv1MigrateStatus::Running;
    REQUIRE(j.isRunning());
    j.status = Amv1MigrateStatus::Failed;
    REQUIRE(j.hasFailed());
}

TEST_CASE("AssetMigratorV1 addJob and jobCount", "[Editor][S165]") {
    AssetMigratorV1 am;
    REQUIRE(am.jobCount() == 0);
    Amv1MigrateJob j; j.id = 1; j.name = "A"; j.fromVersion = "1"; j.toVersion = "2";
    REQUIRE(am.addJob(j));
    REQUIRE(am.jobCount() == 1);
}

TEST_CASE("AssetMigratorV1 addJob invalid fails", "[Editor][S165]") {
    AssetMigratorV1 am;
    REQUIRE(!am.addJob(Amv1MigrateJob{}));
}

TEST_CASE("AssetMigratorV1 addJob duplicate fails", "[Editor][S165]") {
    AssetMigratorV1 am;
    Amv1MigrateJob j; j.id = 1; j.name = "A"; j.fromVersion = "1"; j.toVersion = "2";
    am.addJob(j);
    REQUIRE(!am.addJob(j));
}

TEST_CASE("AssetMigratorV1 removeJob", "[Editor][S165]") {
    AssetMigratorV1 am;
    Amv1MigrateJob j; j.id = 2; j.name = "B"; j.fromVersion = "1"; j.toVersion = "2";
    am.addJob(j);
    REQUIRE(am.removeJob(2));
    REQUIRE(am.jobCount() == 0);
    REQUIRE(!am.removeJob(2));
}

TEST_CASE("AssetMigratorV1 setStatus and counts", "[Editor][S165]") {
    AssetMigratorV1 am;
    Amv1MigrateJob j1; j1.id = 1; j1.name = "A"; j1.fromVersion = "1"; j1.toVersion = "2";
    Amv1MigrateJob j2; j2.id = 2; j2.name = "B"; j2.fromVersion = "1"; j2.toVersion = "2";
    Amv1MigrateJob j3; j3.id = 3; j3.name = "C"; j3.fromVersion = "1"; j3.toVersion = "2";
    am.addJob(j1); am.addJob(j2); am.addJob(j3);
    am.setStatus(1, Amv1MigrateStatus::Done);
    am.setStatus(2, Amv1MigrateStatus::Running);
    am.setStatus(3, Amv1MigrateStatus::Failed);
    REQUIRE(am.doneCount()    == 1);
    REQUIRE(am.runningCount() == 1);
    REQUIRE(am.failedCount()  == 1);
}

TEST_CASE("AssetMigratorV1 updateProgress", "[Editor][S165]") {
    AssetMigratorV1 am;
    Amv1MigrateJob j; j.id = 1; j.name = "A"; j.fromVersion = "1"; j.toVersion = "2"; j.assetsTotal = 100;
    am.addJob(j);
    REQUIRE(am.updateProgress(1, 0.5f, 50, 2));
    auto* found = am.findJob(1);
    REQUIRE(found->progress     == Approx(0.5f));
    REQUIRE(found->assetsDone   == 50);
    REQUIRE(found->assetsFailed == 2);
}

TEST_CASE("AssetMigratorV1 setConflictRule", "[Editor][S165]") {
    AssetMigratorV1 am;
    Amv1MigrateJob j; j.id = 1; j.name = "A"; j.fromVersion = "1"; j.toVersion = "2";
    am.addJob(j);
    REQUIRE(am.setConflictRule(1, Amv1ConflictRule::Skip));
    REQUIRE(am.findJob(1)->conflictRule == Amv1ConflictRule::Skip);
}

TEST_CASE("AssetMigratorV1 countByScope", "[Editor][S165]") {
    AssetMigratorV1 am;
    Amv1MigrateJob j1; j1.id = 1; j1.name = "A"; j1.fromVersion = "1"; j1.toVersion = "2"; j1.scope = Amv1MigrateScope::Project;
    Amv1MigrateJob j2; j2.id = 2; j2.name = "B"; j2.fromVersion = "1"; j2.toVersion = "2"; j2.scope = Amv1MigrateScope::Folder;
    am.addJob(j1); am.addJob(j2);
    REQUIRE(am.countByScope(Amv1MigrateScope::Project) == 1);
    REQUIRE(am.countByScope(Amv1MigrateScope::Folder)  == 1);
}

TEST_CASE("amv1MigrateStatusName covers all values", "[Editor][S165]") {
    REQUIRE(std::string(amv1MigrateStatusName(Amv1MigrateStatus::Pending))  == "Pending");
    REQUIRE(std::string(amv1MigrateStatusName(Amv1MigrateStatus::Rollback)) == "Rollback");
}

TEST_CASE("AssetMigratorV1 onChange callback", "[Editor][S165]") {
    AssetMigratorV1 am;
    uint64_t notified = 0;
    am.setOnChange([&](uint64_t id) { notified = id; });
    Amv1MigrateJob j; j.id = 7; j.name = "G"; j.fromVersion = "1"; j.toVersion = "2";
    am.addJob(j);
    am.setStatus(7, Amv1MigrateStatus::Done);
    REQUIRE(notified == 7);
}
