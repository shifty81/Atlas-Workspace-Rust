// Phase 6 — Build, Patch, and Release Pipeline
//
// Tests for:
//   - DependencyPolicy (classification, evaluation, canonical policy)
//   - PatchApplier (lifecycle state machine, apply/remove workflow, dependencies)
//   - RepoAuditReport (check accumulation, summary counters, pass/fail logic)
//   - ReleaseManifest + ReleaseManifestValidator (versioning, targets, gates)
//   - Integration: patch + gate + audit flow

#include <catch2/catch_test_macros.hpp>

#include "NF/Workspace/DependencyPolicy.h"
#include "NF/Workspace/PatchApplier.h"
#include "NF/Workspace/RepoAuditReport.h"
#include "NF/Workspace/ReleaseManifest.h"

#include <string>
#include <vector>

// ═════════════════════════════════════════════════════════════════════════════
// DependencyPolicy tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("DependencyPolicy: default is empty", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    CHECK(policy.count() == 0);
    CHECK(policy.all().empty());
}

TEST_CASE("DependencyPolicy: add increases count", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"Catch2", "3.4.0", NF::DependencyTier::Optional, NF::DependencySource::FetchContent, true, "Test"});
    CHECK(policy.count() == 1);
}

TEST_CASE("DependencyPolicy: find returns descriptor for known name", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"MyLib", "1.0.0", NF::DependencyTier::Required, NF::DependencySource::Vcpkg, false, ""});
    const auto* d = policy.find("MyLib");
    REQUIRE(d != nullptr);
    CHECK(d->name == "MyLib");
    CHECK(d->tier == NF::DependencyTier::Required);
    CHECK(d->source == NF::DependencySource::Vcpkg);
}

TEST_CASE("DependencyPolicy: find returns nullptr for unknown name", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    CHECK(policy.find("nonexistent") == nullptr);
}

TEST_CASE("DependencyPolicy: isDeclared reflects registration", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    CHECK_FALSE(policy.isDeclared("Catch2"));
    policy.add({"Catch2", "3.4.0", NF::DependencyTier::Optional, NF::DependencySource::FetchContent, true, ""});
    CHECK(policy.isDeclared("Catch2"));
}

TEST_CASE("DependencyPolicy: evaluate passes when all required deps present", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"LibA", "1.0.0", NF::DependencyTier::Required, NF::DependencySource::Vcpkg, false, ""});
    policy.add({"LibB", "2.0.0", NF::DependencyTier::Required, NF::DependencySource::Vcpkg, false, ""});
    auto report = policy.evaluate(false, {"LibA", "LibB"});
    CHECK(report.passed());
    CHECK(report.violationCount() == 0);
    CHECK(report.checkedCount() == 2);
}

TEST_CASE("DependencyPolicy: evaluate fails when required dep is absent", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"LibA", "1.0.0", NF::DependencyTier::Required, NF::DependencySource::Vcpkg, false, ""});
    auto report = policy.evaluate(false, {}); // LibA is absent
    CHECK_FALSE(report.passed());
    CHECK(report.violationCount() == 1);
    CHECK(report.violations[0].name == "LibA");
}

TEST_CASE("DependencyPolicy: evaluate fails when Forbidden dep is present", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"BadLib", "0.1.0", NF::DependencyTier::Forbidden, NF::DependencySource::Vendored, false, ""});
    auto report = policy.evaluate(false, {"BadLib"}); // Forbidden dep present
    CHECK_FALSE(report.passed());
    CHECK(report.violationCount() == 1);
    CHECK(report.violations[0].name == "BadLib");
}

TEST_CASE("DependencyPolicy: evaluate passes when Forbidden dep is absent", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"BadLib", "0.1.0", NF::DependencyTier::Forbidden, NF::DependencySource::Vendored, false, ""});
    auto report = policy.evaluate(false, {}); // Forbidden dep correctly absent
    CHECK(report.passed());
}

TEST_CASE("DependencyPolicy: evaluate passes for Optional dep absent", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"OptLib", "1.0.0", NF::DependencyTier::Optional, NF::DependencySource::FetchContent, false, ""});
    auto report = policy.evaluate(false, {}); // Optional dep missing — OK
    CHECK(report.passed());
}

TEST_CASE("DependencyPolicy: online dep present with online disabled → violation", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"Catch2", "3.4.0", NF::DependencyTier::Optional, NF::DependencySource::FetchContent, true, ""});
    auto report = policy.evaluate(/*onlineEnabled=*/false, {"Catch2"});
    CHECK_FALSE(report.passed());
    CHECK(report.violationCount() == 1);
}

TEST_CASE("DependencyPolicy: online dep present with online enabled → no violation", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    policy.add({"Catch2", "3.4.0", NF::DependencyTier::Optional, NF::DependencySource::FetchContent, true, ""});
    auto report = policy.evaluate(/*onlineEnabled=*/true, {"Catch2"});
    CHECK(report.passed());
}

TEST_CASE("DependencyPolicy: evaluate records onlineEnabled flag", "[DependencyPolicy]") {
    NF::DependencyPolicy policy;
    auto r1 = policy.evaluate(true,  {});
    auto r2 = policy.evaluate(false, {});
    CHECK(r1.onlineEnabled == true);
    CHECK(r2.onlineEnabled == false);
}

TEST_CASE("DependencyPolicy: dependencyTierName covers all values", "[DependencyPolicy]") {
    CHECK(std::string(NF::dependencyTierName(NF::DependencyTier::Required))  == "Required");
    CHECK(std::string(NF::dependencyTierName(NF::DependencyTier::Optional))  == "Optional");
    CHECK(std::string(NF::dependencyTierName(NF::DependencyTier::Forbidden)) == "Forbidden");
}

TEST_CASE("DependencyPolicy: dependencySourceName covers all values", "[DependencyPolicy]") {
    CHECK(std::string(NF::dependencySourceName(NF::DependencySource::Vendored))       == "Vendored");
    CHECK(std::string(NF::dependencySourceName(NF::DependencySource::FetchContent))   == "FetchContent");
    CHECK(std::string(NF::dependencySourceName(NF::DependencySource::Vcpkg))          == "Vcpkg");
    CHECK(std::string(NF::dependencySourceName(NF::DependencySource::SystemProvided)) == "SystemProvided");
}

TEST_CASE("makeCanonicalDependencyPolicy returns non-empty policy", "[DependencyPolicy]") {
    auto policy = NF::makeCanonicalDependencyPolicy();
    CHECK(policy.count() > 0);
    CHECK(policy.isDeclared("Catch2"));
    CHECK(policy.isDeclared("nlohmann-json"));
    CHECK(policy.isDeclared("spdlog"));
    CHECK(policy.isDeclared("glm"));
}

TEST_CASE("makeCanonicalDependencyPolicy: Catch2 requires online deps", "[DependencyPolicy]") {
    auto policy = NF::makeCanonicalDependencyPolicy();
    const auto* d = policy.find("Catch2");
    REQUIRE(d != nullptr);
    CHECK(d->onlineRequired == true);
    CHECK(d->source == NF::DependencySource::FetchContent);
}

// ═════════════════════════════════════════════════════════════════════════════
// PatchApplier tests
// ═════════════════════════════════════════════════════════════════════════════

namespace {
NF::PatchRecord makePatch(const std::string& id, const std::string& displayName = "") {
    NF::PatchRecord rec;
    rec.id          = id;
    rec.displayName = displayName.empty() ? id : displayName;
    rec.version     = "1.0.0";
    return rec;
}
} // namespace

TEST_CASE("PatchApplier: default has no patches", "[PatchApplier]") {
    NF::PatchApplier applier;
    CHECK(applier.registeredCount() == 0);
    CHECK_FALSE(applier.isRegistered("any"));
}

TEST_CASE("PatchApplier: registerPatch succeeds for new id", "[PatchApplier]") {
    NF::PatchApplier applier;
    CHECK(applier.registerPatch(makePatch("patch-a")));
    CHECK(applier.isRegistered("patch-a"));
    CHECK(applier.registeredCount() == 1);
}

TEST_CASE("PatchApplier: registerPatch rejects duplicate id", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    CHECK_FALSE(applier.registerPatch(makePatch("patch-a")));
    CHECK(applier.registeredCount() == 1);
}

TEST_CASE("PatchApplier: registerPatch rejects empty id", "[PatchApplier]") {
    NF::PatchApplier applier;
    NF::PatchRecord bad;
    bad.id = "";
    CHECK_FALSE(applier.registerPatch(bad));
    CHECK(applier.registeredCount() == 0);
}

TEST_CASE("PatchApplier: stateOf returns Unavailable for unknown patch", "[PatchApplier]") {
    NF::PatchApplier applier;
    CHECK(applier.stateOf("unknown") == NF::PatchState::Unavailable);
}

TEST_CASE("PatchApplier: stateOf returns Available after registration", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Available);
}

TEST_CASE("PatchApplier: applyPatch with null callback succeeds", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    auto result = applier.applyPatch("patch-a"); // no callback
    CHECK(result.success);
    CHECK(result.patchId == "patch-a");
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Applied);
    CHECK(applier.isApplied("patch-a"));
}

TEST_CASE("PatchApplier: applyPatch with passing callback succeeds", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    bool callbackCalled = false;
    auto result = applier.applyPatch("patch-a", [&](const NF::PatchRecord&) {
        callbackCalled = true;
        return true;
    });
    CHECK(result.success);
    CHECK(callbackCalled);
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Applied);
}

TEST_CASE("PatchApplier: applyPatch with failing callback → Failed state", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    auto result = applier.applyPatch("patch-a", [](const NF::PatchRecord&) { return false; });
    CHECK_FALSE(result.success);
    CHECK_FALSE(result.errorMessage.empty());
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Failed);
}

TEST_CASE("PatchApplier: applyPatch on unknown patch returns error", "[PatchApplier]") {
    NF::PatchApplier applier;
    auto result = applier.applyPatch("nonexistent");
    CHECK_FALSE(result.success);
    CHECK_FALSE(result.errorMessage.empty());
}

TEST_CASE("PatchApplier: applyPatch on already-applied patch returns error", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    applier.applyPatch("patch-a");
    auto result = applier.applyPatch("patch-a"); // apply again
    CHECK_FALSE(result.success);
    CHECK_FALSE(result.errorMessage.empty());
}

TEST_CASE("PatchApplier: removePatch after apply succeeds", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    applier.applyPatch("patch-a");
    auto result = applier.removePatch("patch-a");
    CHECK(result.success);
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Available);
}

TEST_CASE("PatchApplier: removePatch on non-applied patch returns error", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    auto result = applier.removePatch("patch-a"); // not applied
    CHECK_FALSE(result.success);
    CHECK_FALSE(result.errorMessage.empty());
}

TEST_CASE("PatchApplier: removePatch with failing callback → Failed state", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    applier.applyPatch("patch-a");
    auto result = applier.removePatch("patch-a", [](const NF::PatchRecord&) { return false; });
    CHECK_FALSE(result.success);
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Failed);
}

TEST_CASE("PatchApplier: dependency enforcement prevents apply without dep", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("dep-patch"));

    NF::PatchRecord dependent = makePatch("child-patch");
    dependent.dependencies = {"dep-patch"};
    applier.registerPatch(dependent);

    // Apply child before dep → blocked
    auto result = applier.applyPatch("child-patch");
    CHECK_FALSE(result.success);
    CHECK_FALSE(result.errorMessage.empty());
}

TEST_CASE("PatchApplier: dependency enforcement allows apply after dep applied", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("dep-patch"));

    NF::PatchRecord dependent = makePatch("child-patch");
    dependent.dependencies = {"dep-patch"};
    applier.registerPatch(dependent);

    applier.applyPatch("dep-patch");     // apply dep first
    auto result = applier.applyPatch("child-patch"); // now OK
    CHECK(result.success);
}

TEST_CASE("PatchApplier: cannot remove dep while dependent is applied", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("dep-patch"));

    NF::PatchRecord dependent = makePatch("child-patch");
    dependent.dependencies = {"dep-patch"};
    applier.registerPatch(dependent);

    applier.applyPatch("dep-patch");
    applier.applyPatch("child-patch");

    // Remove dep while child is applied → blocked
    auto result = applier.removePatch("dep-patch");
    CHECK_FALSE(result.success);
    CHECK_FALSE(result.errorMessage.empty());
}

TEST_CASE("PatchApplier: resetPatch clears Failed state", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    applier.applyPatch("patch-a", [](const NF::PatchRecord&) { return false; }); // → Failed
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Failed);
    CHECK(applier.resetPatch("patch-a"));
    CHECK(applier.stateOf("patch-a") == NF::PatchState::Available);
}

TEST_CASE("PatchApplier: resetPatch returns false for non-failed patch", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    CHECK_FALSE(applier.resetPatch("patch-a")); // Available, not Failed
}

TEST_CASE("PatchApplier: appliedIds lists applied patches", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    applier.registerPatch(makePatch("patch-b"));
    applier.applyPatch("patch-a");
    auto ids = applier.appliedIds();
    CHECK(ids.size() == 1);
    CHECK(ids[0] == "patch-a");
}

TEST_CASE("PatchApplier: availableIds lists unapplied patches", "[PatchApplier]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("patch-a"));
    applier.registerPatch(makePatch("patch-b"));
    applier.applyPatch("patch-a");
    auto ids = applier.availableIds();
    CHECK(ids.size() == 1);
    CHECK(ids[0] == "patch-b");
}

TEST_CASE("PatchApplier: patchStateName covers all values", "[PatchApplier]") {
    CHECK(std::string(NF::patchStateName(NF::PatchState::Unavailable)) == "Unavailable");
    CHECK(std::string(NF::patchStateName(NF::PatchState::Available))   == "Available");
    CHECK(std::string(NF::patchStateName(NF::PatchState::Applying))    == "Applying");
    CHECK(std::string(NF::patchStateName(NF::PatchState::Applied))     == "Applied");
    CHECK(std::string(NF::patchStateName(NF::PatchState::Removing))    == "Removing");
    CHECK(std::string(NF::patchStateName(NF::PatchState::Failed))      == "Failed");
}

TEST_CASE("PatchApplier: patchFileOpName covers all values", "[PatchApplier]") {
    CHECK(std::string(NF::patchFileOpName(NF::PatchFileOp::Add))    == "Add");
    CHECK(std::string(NF::patchFileOpName(NF::PatchFileOp::Modify)) == "Modify");
    CHECK(std::string(NF::patchFileOpName(NF::PatchFileOp::Remove)) == "Remove");
}

// ═════════════════════════════════════════════════════════════════════════════
// RepoAuditReport tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("RepoAuditReport: default is empty", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    CHECK(r.totalChecks() == 0);
    CHECK(r.passed());
    CHECK(r.failCount() == 0);
}

TEST_CASE("RepoAuditReport: pass() adds passing check", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.pass("source_exists", "Source/ directory exists", NF::AuditCategory::Structure);
    CHECK(r.totalChecks() == 1);
    CHECK(r.passCount() == 1);
    CHECK(r.passed());
}

TEST_CASE("RepoAuditReport: fail() adds failing check and fails the report", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.fail("cmake_missing", "CMakeLists.txt is missing", NF::AuditCategory::Build, "File not found");
    CHECK(r.totalChecks() == 1);
    CHECK(r.failCount() == 1);
    CHECK_FALSE(r.passed());
}

TEST_CASE("RepoAuditReport: warn() does not fail the report", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.warn("no_changelog", "No CHANGELOG.md found", NF::AuditCategory::Docs, "Advisory only");
    CHECK(r.warnCount() == 1);
    CHECK(r.failCount() == 0);
    CHECK(r.passed());
}

TEST_CASE("RepoAuditReport: skip() counts separately", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.skip("windows_only_check", "DirectX headers", NF::AuditCategory::Structure, "Linux: skip");
    CHECK(r.skipCount() == 1);
    CHECK(r.totalChecks() == 1);
    CHECK(r.passed());
}

TEST_CASE("RepoAuditReport: mixed checks give correct counters", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.pass("a", "A", NF::AuditCategory::Structure);
    r.pass("b", "B", NF::AuditCategory::Structure);
    r.warn("c", "C", NF::AuditCategory::Docs, "warning");
    r.fail("d", "D", NF::AuditCategory::Build, "fail reason");
    r.skip("e", "E", NF::AuditCategory::Security);
    CHECK(r.totalChecks() == 5);
    CHECK(r.passCount() == 2);
    CHECK(r.warnCount() == 1);
    CHECK(r.failCount() == 1);
    CHECK(r.skipCount() == 1);
    CHECK_FALSE(r.passed());
}

TEST_CASE("RepoAuditReport: failures() returns only Fail entries", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.pass("a", "A", NF::AuditCategory::Structure);
    r.fail("b", "B", NF::AuditCategory::Build, "problem");
    r.warn("c", "C", NF::AuditCategory::Docs, "advisory");
    auto fails = r.failures();
    REQUIRE(fails.size() == 1);
    CHECK(fails[0]->id == "b");
}

TEST_CASE("RepoAuditReport: warnings() returns only Warn entries", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.pass("a", "A", NF::AuditCategory::Structure);
    r.warn("b", "B", NF::AuditCategory::Docs, "advisory");
    auto warns = r.warnings();
    REQUIRE(warns.size() == 1);
    CHECK(warns[0]->id == "b");
}

TEST_CASE("RepoAuditReport: countByCategory is accurate", "[RepoAuditReport]") {
    NF::RepoAuditReport r;
    r.pass("s1", "S1", NF::AuditCategory::Structure);
    r.pass("s2", "S2", NF::AuditCategory::Structure);
    r.pass("b1", "B1", NF::AuditCategory::Build);
    r.fail("t1", "T1", NF::AuditCategory::Tests, "test fail");
    CHECK(r.countByCategory(NF::AuditCategory::Structure) == 2);
    CHECK(r.countByCategory(NF::AuditCategory::Build)     == 1);
    CHECK(r.countByCategory(NF::AuditCategory::Tests)     == 1);
    CHECK(r.countByCategory(NF::AuditCategory::Docs)      == 0);
}

TEST_CASE("RepoAuditReport: auditCheckStatusName covers all values", "[RepoAuditReport]") {
    CHECK(std::string(NF::auditCheckStatusName(NF::AuditCheckStatus::Pass)) == "Pass");
    CHECK(std::string(NF::auditCheckStatusName(NF::AuditCheckStatus::Warn)) == "Warn");
    CHECK(std::string(NF::auditCheckStatusName(NF::AuditCheckStatus::Fail)) == "Fail");
    CHECK(std::string(NF::auditCheckStatusName(NF::AuditCheckStatus::Skip)) == "Skip");
}

TEST_CASE("RepoAuditReport: auditCategoryName covers all values", "[RepoAuditReport]") {
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Structure))    == "Structure");
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Naming))       == "Naming");
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Dependencies)) == "Dependencies");
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Tests))        == "Tests");
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Docs))         == "Docs");
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Build))        == "Build");
    CHECK(std::string(NF::auditCategoryName(NF::AuditCategory::Security))     == "Security");
}

// ═════════════════════════════════════════════════════════════════════════════
// ReleaseManifest tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("SemanticVersion: toString basic", "[ReleaseManifest]") {
    NF::SemanticVersion v{1, 2, 3};
    CHECK(v.toString() == "1.2.3");
}

TEST_CASE("SemanticVersion: toString with preRelease", "[ReleaseManifest]") {
    NF::SemanticVersion v{1, 0, 0, "alpha.1"};
    CHECK(v.toString() == "1.0.0-alpha.1");
}

TEST_CASE("SemanticVersion: toString with buildMeta", "[ReleaseManifest]") {
    NF::SemanticVersion v{2, 0, 0, "", "abc123"};
    CHECK(v.toString() == "2.0.0+abc123");
}

TEST_CASE("SemanticVersion: toString with preRelease and buildMeta", "[ReleaseManifest]") {
    NF::SemanticVersion v{1, 0, 0, "rc.1", "build99"};
    CHECK(v.toString() == "1.0.0-rc.1+build99");
}

TEST_CASE("SemanticVersion: isStable for release version", "[ReleaseManifest]") {
    NF::SemanticVersion v{1, 2, 3};
    CHECK(v.isStable());
}

TEST_CASE("SemanticVersion: isStable false for pre-release", "[ReleaseManifest]") {
    NF::SemanticVersion v{1, 2, 3, "beta.1"};
    CHECK_FALSE(v.isStable());
}

TEST_CASE("SemanticVersion: equality comparison", "[ReleaseManifest]") {
    NF::SemanticVersion a{1, 2, 3};
    NF::SemanticVersion b{1, 2, 3};
    NF::SemanticVersion c{1, 2, 4};
    CHECK(a == b);
    CHECK(a != c);
}

TEST_CASE("ReleaseManifest: default is empty", "[ReleaseManifest]") {
    NF::ReleaseManifest m;
    CHECK(m.targetCount() == 0);
    CHECK(m.totalArtifactCount() == 0);
    CHECK_FALSE(m.hasTarget("windows-x64"));
}

TEST_CASE("ReleaseManifest: findTarget returns correct target", "[ReleaseManifest]") {
    NF::ReleaseManifest m;
    NF::ReleaseTarget t;
    t.platform = "windows-x64";
    t.config   = NF::BuildConfiguration::Release;
    t.artifacts.push_back({"AtlasWorkspace.exe", "bin/", true, ""});
    m.targets.push_back(t);

    CHECK(m.targetCount() == 1);
    CHECK(m.hasTarget("windows-x64"));
    REQUIRE(m.findTarget("windows-x64") != nullptr);
    CHECK(m.findTarget("windows-x64")->platform == "windows-x64");
    CHECK(m.findTarget("linux-x64") == nullptr);
}

TEST_CASE("ReleaseManifest: totalArtifactCount sums across targets", "[ReleaseManifest]") {
    NF::ReleaseManifest m;
    NF::ReleaseTarget t1;
    t1.platform = "windows-x64";
    t1.artifacts.push_back({"AtlasWorkspace.exe", "bin/", true, ""});
    t1.artifacts.push_back({"AtlasWorkspace.pdb", "bin/", false, ""});
    NF::ReleaseTarget t2;
    t2.platform = "linux-x64";
    t2.artifacts.push_back({"AtlasWorkspace", "bin/", true, ""});
    m.targets.push_back(t1);
    m.targets.push_back(t2);

    CHECK(m.totalArtifactCount() == 3);
}

TEST_CASE("ReleaseTarget: requiredArtifactCount counts only required", "[ReleaseManifest]") {
    NF::ReleaseTarget t;
    t.platform = "windows-x64";
    t.artifacts.push_back({"main.exe",  "bin/", true,  ""});
    t.artifacts.push_back({"debug.pdb", "bin/", false, ""}); // optional
    t.artifacts.push_back({"readme.txt","",     true,  ""});
    CHECK(t.requiredArtifactCount() == 2);
}

TEST_CASE("buildConfigurationName covers all values", "[ReleaseManifest]") {
    CHECK(std::string(NF::buildConfigurationName(NF::BuildConfiguration::Debug))          == "Debug");
    CHECK(std::string(NF::buildConfigurationName(NF::BuildConfiguration::Release))        == "Release");
    CHECK(std::string(NF::buildConfigurationName(NF::BuildConfiguration::RelWithDebInfo)) == "RelWithDebInfo");
    CHECK(std::string(NF::buildConfigurationName(NF::BuildConfiguration::MinSizeRel))     == "MinSizeRel");
}

TEST_CASE("ReleaseManifestValidator: no gates always passes", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    NF::ReleaseManifest m;
    auto result = v.validate(m);
    CHECK(result.passed);
    CHECK(result.isPublishable());
    CHECK(result.gateResults.empty());
}

TEST_CASE("ReleaseManifestValidator: passing gate → passed", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    v.addGate("always_pass", [](const NF::ReleaseManifest&) {
        return NF::ReleaseGateResult{true};
    });
    NF::ReleaseManifest m;
    auto result = v.validate(m);
    CHECK(result.passed);
    CHECK(result.gateResults.size() == 1);
    CHECK(result.gateResults[0].passed);
}

TEST_CASE("ReleaseManifestValidator: failing gate → not passed", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    v.addGate("version_gate", [](const NF::ReleaseManifest& m) {
        if (!m.version.isStable())
            return NF::ReleaseGateResult{false, "version_gate", "Only stable versions can be released."};
        return NF::ReleaseGateResult{true};
    });
    NF::ReleaseManifest m;
    m.version = {1, 0, 0, "beta.1"}; // pre-release
    auto result = v.validate(m);
    CHECK_FALSE(result.passed);
    CHECK_FALSE(result.isPublishable());
    CHECK(result.failures.size() == 1);
}

TEST_CASE("ReleaseManifestValidator: gate receives manifest data", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    std::string capturedName;
    v.addGate("name_checker", [&](const NF::ReleaseManifest& m) {
        capturedName = m.projectName;
        return NF::ReleaseGateResult{true};
    });
    NF::ReleaseManifest m;
    m.projectName = "AtlasWorkspace";
    static_cast<void>(v.validate(m));
    CHECK(capturedName == "AtlasWorkspace");
}

TEST_CASE("ReleaseManifestValidator: clearGates removes all gates", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    v.addGate("g1", [](const NF::ReleaseManifest&) { return NF::ReleaseGateResult{false}; });
    CHECK(v.gateCount() == 1);
    v.clearGates();
    CHECK(v.gateCount() == 0);
    NF::ReleaseManifest m;
    CHECK(v.validate(m).passed); // no gates → passes
}

TEST_CASE("ReleaseManifestValidator: rejects empty gateId", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    v.addGate("", [](const NF::ReleaseManifest&) { return NF::ReleaseGateResult{false}; });
    CHECK(v.gateCount() == 0);
}

TEST_CASE("ReleaseManifestValidator: rejects null gate function", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    v.addGate("null_gate", nullptr);
    CHECK(v.gateCount() == 0);
}

TEST_CASE("ReleaseManifestValidator: gate requiring at least one target", "[ReleaseManifest]") {
    NF::ReleaseManifestValidator v;
    v.addGate("has_targets", [](const NF::ReleaseManifest& m) {
        if (m.targetCount() == 0)
            return NF::ReleaseGateResult{false, "has_targets", "Release must have at least one target."};
        return NF::ReleaseGateResult{true};
    });
    NF::ReleaseManifest empty;
    CHECK_FALSE(v.validate(empty).passed);

    NF::ReleaseManifest withTarget;
    NF::ReleaseTarget t;
    t.platform = "linux-x64";
    withTarget.targets.push_back(t);
    CHECK(v.validate(withTarget).passed);
}

// ═════════════════════════════════════════════════════════════════════════════
// Integration tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("Phase6 integration: patch + audit + release gate full flow", "[Phase6][integration]") {
    // 1. Apply a patch
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("hotfix-1.2.3", "Hotfix 1.2.3"));
    auto patchResult = applier.applyPatch("hotfix-1.2.3");
    CHECK(patchResult.success);
    CHECK(applier.isApplied("hotfix-1.2.3"));

    // 2. Run a repo audit — pass structure checks, fail a naming check
    NF::RepoAuditReport audit;
    audit.pass("cmake_exists",  "CMakeLists.txt exists",   NF::AuditCategory::Structure);
    audit.pass("readme_exists", "README.md exists",        NF::AuditCategory::Docs);
    audit.warn("changelog",     "No CHANGELOG.md",         NF::AuditCategory::Docs, "Advisory");
    CHECK(audit.passed());

    // 3. Build a release manifest and validate
    NF::ReleaseManifest manifest;
    manifest.projectName = "AtlasWorkspace";
    manifest.version     = {1, 2, 3};

    NF::ReleaseTarget target;
    target.platform = "linux-x64";
    target.config   = NF::BuildConfiguration::Release;
    target.artifacts.push_back({"AtlasWorkspace", "bin/", true, ""});
    manifest.targets.push_back(target);

    NF::ReleaseManifestValidator validator;
    validator.addGate("stable_version", [](const NF::ReleaseManifest& m) {
        if (!m.version.isStable())
            return NF::ReleaseGateResult{false, "stable_version", "Pre-release versions cannot be published."};
        return NF::ReleaseGateResult{true};
    });
    validator.addGate("audit_passed", [&audit](const NF::ReleaseManifest&) {
        if (!audit.passed())
            return NF::ReleaseGateResult{false, "audit_passed", "Repo audit has failures."};
        return NF::ReleaseGateResult{true};
    });

    auto releaseResult = validator.validate(manifest);
    CHECK(releaseResult.isPublishable());
}

TEST_CASE("Phase6 integration: dependency policy gates online-dep build", "[Phase6][integration]") {
    auto policy = NF::makeCanonicalDependencyPolicy();

    // Without Catch2 present and online disabled → no violation (Catch2 is Optional)
    auto r1 = policy.evaluate(false, {});
    CHECK(r1.passed()); // All are Optional, none Forbidden

    // With Catch2 present and online disabled → violation
    auto r2 = policy.evaluate(false, {"Catch2"});
    CHECK_FALSE(r2.passed());

    // With Catch2 present and online enabled → passes
    auto r3 = policy.evaluate(true, {"Catch2"});
    CHECK(r3.passed());
}

TEST_CASE("Phase6 integration: patch dep chain apply and remove", "[Phase6][integration]") {
    NF::PatchApplier applier;
    applier.registerPatch(makePatch("base"));

    NF::PatchRecord middle = makePatch("middle");
    middle.dependencies = {"base"};
    applier.registerPatch(middle);

    NF::PatchRecord top = makePatch("top");
    top.dependencies = {"middle"};
    applier.registerPatch(top);

    // Apply in order
    CHECK(applier.applyPatch("base").success);
    CHECK(applier.applyPatch("middle").success);
    CHECK(applier.applyPatch("top").success);

    // Remove in reverse order
    CHECK(applier.removePatch("top").success);
    CHECK(applier.removePatch("middle").success);
    CHECK(applier.removePatch("base").success);

    CHECK(applier.appliedIds().empty());
}
