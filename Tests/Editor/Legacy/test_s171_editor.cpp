// S171 editor tests: PlatformPublishEditorV1, BuildArtifactManagerV1, DeploymentTargetEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PlatformPublishEditorV1.h"
#include "NF/Editor/BuildArtifactManagerV1.h"
#include "NF/Editor/DeploymentTargetEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── PlatformPublishEditorV1 ──────────────────────────────────────────────────

TEST_CASE("Ppev1StoreListing validity", "[Editor][S171]") {
    Ppev1StoreListing l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "AtlasGame"; l.version = "1.0.0";
    REQUIRE(l.isValid());
}

TEST_CASE("PlatformPublishEditorV1 addListing and listingCount", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    REQUIRE(ppe.listingCount() == 0);
    Ppev1StoreListing l; l.id = 1; l.name = "G"; l.version = "1.0";
    REQUIRE(ppe.addListing(l));
    REQUIRE(ppe.listingCount() == 1);
}

TEST_CASE("PlatformPublishEditorV1 addListing invalid fails", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    REQUIRE(!ppe.addListing(Ppev1StoreListing{}));
}

TEST_CASE("PlatformPublishEditorV1 addListing duplicate fails", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    REQUIRE(!ppe.addListing(l));
}

TEST_CASE("PlatformPublishEditorV1 removeListing", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 2; l.name = "B"; l.version = "1.0";
    ppe.addListing(l);
    REQUIRE(ppe.removeListing(2));
    REQUIRE(ppe.listingCount() == 0);
    REQUIRE(!ppe.removeListing(2));
}

TEST_CASE("PlatformPublishEditorV1 setState liveCount", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    REQUIRE(ppe.setState(1, Ppev1PublishState::Live));
    REQUIRE(ppe.liveCount() == 1);
    REQUIRE(ppe.findListing(1)->isLive());
}

TEST_CASE("PlatformPublishEditorV1 approvedCount", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    ppe.setState(1, Ppev1PublishState::Approved);
    REQUIRE(ppe.approvedCount() == 1);
    REQUIRE(ppe.findListing(1)->isApproved());
}

TEST_CASE("PlatformPublishEditorV1 rejectedCount", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    ppe.setState(1, Ppev1PublishState::Rejected);
    REQUIRE(ppe.rejectedCount() == 1);
    REQUIRE(ppe.findListing(1)->isRejected());
}

TEST_CASE("PlatformPublishEditorV1 submitForReview transitions from Draft", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    REQUIRE(ppe.submitForReview(1));
    REQUIRE(ppe.findListing(1)->state == Ppev1PublishState::Review);
}

TEST_CASE("PlatformPublishEditorV1 submitForReview non-Draft fails", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    ppe.setState(1, Ppev1PublishState::Live);
    REQUIRE(!ppe.submitForReview(1));
}

TEST_CASE("PlatformPublishEditorV1 setAgeRated", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l; l.id = 1; l.name = "A"; l.version = "1.0";
    ppe.addListing(l);
    REQUIRE(ppe.setAgeRated(1, true));
    REQUIRE(ppe.findListing(1)->ageRated);
}

TEST_CASE("PlatformPublishEditorV1 countByPlatform", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l1; l1.id = 1; l1.name = "A"; l1.version = "1.0"; l1.platform = Ppev1Platform::PC;
    Ppev1StoreListing l2; l2.id = 2; l2.name = "B"; l2.version = "1.0"; l2.platform = Ppev1Platform::Mobile;
    ppe.addListing(l1); ppe.addListing(l2);
    REQUIRE(ppe.countByPlatform(Ppev1Platform::PC)     == 1);
    REQUIRE(ppe.countByPlatform(Ppev1Platform::Mobile) == 1);
}

TEST_CASE("PlatformPublishEditorV1 countByRegion", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    Ppev1StoreListing l1; l1.id = 1; l1.name = "A"; l1.version = "1.0"; l1.region = Ppev1StoreRegion::NA;
    Ppev1StoreListing l2; l2.id = 2; l2.name = "B"; l2.version = "1.0"; l2.region = Ppev1StoreRegion::EU;
    ppe.addListing(l1); ppe.addListing(l2);
    REQUIRE(ppe.countByRegion(Ppev1StoreRegion::NA) == 1);
    REQUIRE(ppe.countByRegion(Ppev1StoreRegion::EU) == 1);
}

TEST_CASE("ppev1PlatformName covers all values", "[Editor][S171]") {
    REQUIRE(std::string(ppev1PlatformName(Ppev1Platform::PC))             == "PC");
    REQUIRE(std::string(ppev1PlatformName(Ppev1Platform::CloudStreaming)) == "CloudStreaming");
}

TEST_CASE("PlatformPublishEditorV1 onChange callback", "[Editor][S171]") {
    PlatformPublishEditorV1 ppe;
    uint64_t notified = 0;
    ppe.setOnChange([&](uint64_t id) { notified = id; });
    Ppev1StoreListing l; l.id = 3; l.name = "C"; l.version = "1.0";
    ppe.addListing(l);
    ppe.setState(3, Ppev1PublishState::Approved);
    REQUIRE(notified == 3);
}

// ── BuildArtifactManagerV1 ───────────────────────────────────────────────────

TEST_CASE("Bamv1Artifact validity", "[Editor][S171]") {
    Bamv1Artifact a;
    REQUIRE(!a.isValid());
    a.id = 1; a.name = "GameBin"; a.version = "1.0.0";
    REQUIRE(a.isValid());
}

TEST_CASE("BuildArtifactManagerV1 addArtifact and artifactCount", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    REQUIRE(bam.artifactCount() == 0);
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    REQUIRE(bam.addArtifact(a));
    REQUIRE(bam.artifactCount() == 1);
}

TEST_CASE("BuildArtifactManagerV1 addArtifact invalid fails", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    REQUIRE(!bam.addArtifact(Bamv1Artifact{}));
}

TEST_CASE("BuildArtifactManagerV1 addArtifact duplicate fails", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    REQUIRE(!bam.addArtifact(a));
}

TEST_CASE("BuildArtifactManagerV1 removeArtifact", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 2; a.name = "B"; a.version = "1.0";
    bam.addArtifact(a);
    REQUIRE(bam.removeArtifact(2));
    REQUIRE(bam.artifactCount() == 0);
    REQUIRE(!bam.removeArtifact(2));
}

TEST_CASE("BuildArtifactManagerV1 setState readyCount", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    REQUIRE(bam.setState(1, Bamv1ArtifactState::Ready));
    REQUIRE(bam.readyCount() == 1);
    REQUIRE(bam.findArtifact(1)->isReady());
}

TEST_CASE("BuildArtifactManagerV1 verifiedCount", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    bam.setState(1, Bamv1ArtifactState::Verified);
    REQUIRE(bam.verifiedCount() == 1);
    REQUIRE(bam.findArtifact(1)->isVerified());
}

TEST_CASE("BuildArtifactManagerV1 publishedCount", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    bam.setState(1, Bamv1ArtifactState::Published);
    REQUIRE(bam.publishedCount() == 1);
    REQUIRE(bam.findArtifact(1)->isPublished());
}

TEST_CASE("BuildArtifactManagerV1 failedCount", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    bam.setState(1, Bamv1ArtifactState::Failed);
    REQUIRE(bam.failedCount() == 1);
    REQUIRE(bam.findArtifact(1)->hasFailed());
}

TEST_CASE("BuildArtifactManagerV1 setChecksum", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    REQUIRE(bam.setChecksum(1, "abc123"));
    REQUIRE(bam.findArtifact(1)->checksum == "abc123");
    REQUIRE(!bam.setChecksum(1, ""));
}

TEST_CASE("BuildArtifactManagerV1 setSize", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    REQUIRE(bam.setSize(1, 1048576));
    REQUIRE(bam.findArtifact(1)->sizeBytes == 1048576);
}

TEST_CASE("BuildArtifactManagerV1 setSigned", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a; a.id = 1; a.name = "A"; a.version = "1.0";
    bam.addArtifact(a);
    REQUIRE(bam.setSigned(1, true));
    REQUIRE(bam.findArtifact(1)->signed_);
}

TEST_CASE("BuildArtifactManagerV1 countByType", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a1; a1.id = 1; a1.name = "A"; a1.version = "1.0"; a1.artifactType = Bamv1ArtifactType::Executable;
    Bamv1Artifact a2; a2.id = 2; a2.name = "B"; a2.version = "1.0"; a2.artifactType = Bamv1ArtifactType::Symbols;
    bam.addArtifact(a1); bam.addArtifact(a2);
    REQUIRE(bam.countByType(Bamv1ArtifactType::Executable) == 1);
    REQUIRE(bam.countByType(Bamv1ArtifactType::Symbols)    == 1);
}

TEST_CASE("BuildArtifactManagerV1 countByCompression", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    Bamv1Artifact a1; a1.id = 1; a1.name = "A"; a1.version = "1.0"; a1.compression = Bamv1Compression::LZ4;
    Bamv1Artifact a2; a2.id = 2; a2.name = "B"; a2.version = "1.0"; a2.compression = Bamv1Compression::Brotli;
    bam.addArtifact(a1); bam.addArtifact(a2);
    REQUIRE(bam.countByCompression(Bamv1Compression::LZ4)    == 1);
    REQUIRE(bam.countByCompression(Bamv1Compression::Brotli) == 1);
}

TEST_CASE("bamv1ArtifactTypeName covers all values", "[Editor][S171]") {
    REQUIRE(std::string(bamv1ArtifactTypeName(Bamv1ArtifactType::Executable)) == "Executable");
    REQUIRE(std::string(bamv1ArtifactTypeName(Bamv1ArtifactType::Symbols))    == "Symbols");
}

TEST_CASE("BuildArtifactManagerV1 onChange callback", "[Editor][S171]") {
    BuildArtifactManagerV1 bam;
    uint64_t notified = 0;
    bam.setOnChange([&](uint64_t id) { notified = id; });
    Bamv1Artifact a; a.id = 8; a.name = "H"; a.version = "2.0";
    bam.addArtifact(a);
    bam.setState(8, Bamv1ArtifactState::Ready);
    REQUIRE(notified == 8);
}

// ── DeploymentTargetEditorV1 ─────────────────────────────────────────────────

TEST_CASE("Dtev1DeployTarget validity", "[Editor][S171]") {
    Dtev1DeployTarget t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "DevDevice01";
    REQUIRE(t.isValid());
}

TEST_CASE("DeploymentTargetEditorV1 addTarget and targetCount", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    REQUIRE(dte.targetCount() == 0);
    Dtev1DeployTarget t; t.id = 1; t.name = "T1";
    REQUIRE(dte.addTarget(t));
    REQUIRE(dte.targetCount() == 1);
}

TEST_CASE("DeploymentTargetEditorV1 addTarget invalid fails", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    REQUIRE(!dte.addTarget(Dtev1DeployTarget{}));
}

TEST_CASE("DeploymentTargetEditorV1 addTarget duplicate fails", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 1; t.name = "A";
    dte.addTarget(t);
    REQUIRE(!dte.addTarget(t));
}

TEST_CASE("DeploymentTargetEditorV1 removeTarget", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 2; t.name = "B";
    dte.addTarget(t);
    REQUIRE(dte.removeTarget(2));
    REQUIRE(dte.targetCount() == 0);
    REQUIRE(!dte.removeTarget(2));
}

TEST_CASE("DeploymentTargetEditorV1 setState completeCount", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 1; t.name = "A";
    dte.addTarget(t);
    REQUIRE(dte.setState(1, Dtev1DeployState::Complete));
    REQUIRE(dte.completeCount() == 1);
    REQUIRE(dte.findTarget(1)->isComplete());
}

TEST_CASE("DeploymentTargetEditorV1 failedCount", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 1; t.name = "A";
    dte.addTarget(t);
    dte.setState(1, Dtev1DeployState::Failed);
    REQUIRE(dte.failedCount() == 1);
    REQUIRE(dte.findTarget(1)->hasFailed());
}

TEST_CASE("DeploymentTargetEditorV1 activeCount", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 1; t.name = "A";
    dte.addTarget(t);
    dte.setState(1, Dtev1DeployState::Uploading);
    REQUIRE(dte.activeCount() == 1);
    REQUIRE(dte.findTarget(1)->isActive());
}

TEST_CASE("DeploymentTargetEditorV1 setProgress validation", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 1; t.name = "A";
    dte.addTarget(t);
    REQUIRE(!dte.setProgress(1, -0.1f));
    REQUIRE(!dte.setProgress(1, 1.5f));
    REQUIRE(dte.setProgress(1, 0.5f));
    REQUIRE(dte.findTarget(1)->progress == Approx(0.5f));
}

TEST_CASE("DeploymentTargetEditorV1 setEnabled enabledCount", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t1; t1.id = 1; t1.name = "A";
    Dtev1DeployTarget t2; t2.id = 2; t2.name = "B"; t2.isEnabled = false;
    dte.addTarget(t1); dte.addTarget(t2);
    REQUIRE(dte.enabledCount() == 1);
    dte.setEnabled(2, true);
    REQUIRE(dte.enabledCount() == 2);
}

TEST_CASE("DeploymentTargetEditorV1 setRequiresVPN", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t; t.id = 1; t.name = "A";
    dte.addTarget(t);
    REQUIRE(dte.setRequiresVPN(1, true));
    REQUIRE(dte.findTarget(1)->requiresVPN);
}

TEST_CASE("DeploymentTargetEditorV1 countByType", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t1; t1.id = 1; t1.name = "A"; t1.targetType = Dtev1TargetType::LocalDevice;
    Dtev1DeployTarget t2; t2.id = 2; t2.name = "B"; t2.targetType = Dtev1TargetType::CDN;
    dte.addTarget(t1); dte.addTarget(t2);
    REQUIRE(dte.countByType(Dtev1TargetType::LocalDevice) == 1);
    REQUIRE(dte.countByType(Dtev1TargetType::CDN)         == 1);
}

TEST_CASE("DeploymentTargetEditorV1 countByEnvironment", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    Dtev1DeployTarget t1; t1.id = 1; t1.name = "A"; t1.environment = Dtev1Environment::QA;
    Dtev1DeployTarget t2; t2.id = 2; t2.name = "B"; t2.environment = Dtev1Environment::Production;
    dte.addTarget(t1); dte.addTarget(t2);
    REQUIRE(dte.countByEnvironment(Dtev1Environment::QA)         == 1);
    REQUIRE(dte.countByEnvironment(Dtev1Environment::Production) == 1);
}

TEST_CASE("dtev1TargetTypeName covers all values", "[Editor][S171]") {
    REQUIRE(std::string(dtev1TargetTypeName(Dtev1TargetType::LocalDevice)) == "LocalDevice");
    REQUIRE(std::string(dtev1TargetTypeName(Dtev1TargetType::QAFarm))      == "QAFarm");
}

TEST_CASE("dtev1EnvironmentName covers all values", "[Editor][S171]") {
    REQUIRE(std::string(dtev1EnvironmentName(Dtev1Environment::Development)) == "Development");
    REQUIRE(std::string(dtev1EnvironmentName(Dtev1Environment::Production))  == "Production");
}

TEST_CASE("DeploymentTargetEditorV1 onChange callback", "[Editor][S171]") {
    DeploymentTargetEditorV1 dte;
    uint64_t notified = 0;
    dte.setOnChange([&](uint64_t id) { notified = id; });
    Dtev1DeployTarget t; t.id = 6; t.name = "F";
    dte.addTarget(t);
    dte.setState(6, Dtev1DeployState::Connecting);
    REQUIRE(notified == 6);
}
