// S123 editor tests: AssetBundleEditor, PackagingEditor, DeploymentEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/DeploymentEditor.h"
#include "NF/Editor/PackagingEditor.h"
#include "NF/Editor/AssetBundleEditor.h"

using namespace NF;

// ── AssetBundleEditor ─────────────────────────────────────────────────────────

TEST_CASE("BundleType names", "[Editor][S123]") {
    REQUIRE(std::string(bundleTypeName(BundleType::Core))      == "Core");
    REQUIRE(std::string(bundleTypeName(BundleType::Optional))  == "Optional");
    REQUIRE(std::string(bundleTypeName(BundleType::DLC))       == "DLC");
    REQUIRE(std::string(bundleTypeName(BundleType::Patch))     == "Patch");
    REQUIRE(std::string(bundleTypeName(BundleType::Streaming)) == "Streaming");
    REQUIRE(std::string(bundleTypeName(BundleType::Language))  == "Language");
}

TEST_CASE("BundleState names", "[Editor][S123]") {
    REQUIRE(std::string(bundleStateName(BundleState::Pending))  == "Pending");
    REQUIRE(std::string(bundleStateName(BundleState::Building)) == "Building");
    REQUIRE(std::string(bundleStateName(BundleState::Ready))    == "Ready");
    REQUIRE(std::string(bundleStateName(BundleState::Stale))    == "Stale");
    REQUIRE(std::string(bundleStateName(BundleState::Failed))   == "Failed");
}

TEST_CASE("AssetBundleEntry defaults", "[Editor][S123]") {
    AssetBundleEntry b(1, "core_assets", BundleType::Core, BundleState::Pending);
    REQUIRE(b.id()           == 1u);
    REQUIRE(b.name()         == "core_assets");
    REQUIRE(b.bundleType()   == BundleType::Core);
    REQUIRE(b.state()        == BundleState::Pending);
    REQUIRE(b.sizeKB()       == 0u);
    REQUIRE(b.isCompressed());
}

TEST_CASE("AssetBundleEntry mutation", "[Editor][S123]") {
    AssetBundleEntry b(2, "dlc_pack", BundleType::DLC, BundleState::Pending);
    b.setState(BundleState::Ready);
    b.setSizeKB(4096u);
    b.setIsCompressed(false);
    REQUIRE(b.state()        == BundleState::Ready);
    REQUIRE(b.sizeKB()       == 4096u);
    REQUIRE(!b.isCompressed());
}

TEST_CASE("AssetBundleEditor defaults", "[Editor][S123]") {
    AssetBundleEditor ed;
    REQUIRE(!ed.isShowStale());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.compressionLevel() == 6u);
    REQUIRE(ed.bundleCount()      == 0u);
}

TEST_CASE("AssetBundleEditor add/remove bundles", "[Editor][S123]") {
    AssetBundleEditor ed;
    REQUIRE(ed.addBundle(AssetBundleEntry(1, "core",     BundleType::Core,     BundleState::Ready)));
    REQUIRE(ed.addBundle(AssetBundleEntry(2, "optional", BundleType::Optional, BundleState::Pending)));
    REQUIRE(ed.addBundle(AssetBundleEntry(3, "dlc_a",   BundleType::DLC,      BundleState::Building)));
    REQUIRE(!ed.addBundle(AssetBundleEntry(1, "core",   BundleType::Core,     BundleState::Ready)));
    REQUIRE(ed.bundleCount() == 3u);
    REQUIRE(ed.removeBundle(2));
    REQUIRE(ed.bundleCount() == 2u);
    REQUIRE(!ed.removeBundle(99));
}

TEST_CASE("AssetBundleEditor counts and find", "[Editor][S123]") {
    AssetBundleEditor ed;
    AssetBundleEntry b1(1, "b1", BundleType::Core,     BundleState::Ready);
    AssetBundleEntry b2(2, "b2", BundleType::Core,     BundleState::Stale);   b2.setIsCompressed(false);
    AssetBundleEntry b3(3, "b3", BundleType::Optional, BundleState::Pending);
    AssetBundleEntry b4(4, "b4", BundleType::DLC,      BundleState::Ready);   b4.setIsCompressed(false);
    ed.addBundle(b1); ed.addBundle(b2); ed.addBundle(b3); ed.addBundle(b4);
    REQUIRE(ed.countByType(BundleType::Core)       == 2u);
    REQUIRE(ed.countByType(BundleType::Optional)   == 1u);
    REQUIRE(ed.countByType(BundleType::Patch)      == 0u);
    REQUIRE(ed.countByState(BundleState::Ready)    == 2u);
    REQUIRE(ed.countByState(BundleState::Stale)    == 1u);
    REQUIRE(ed.countByState(BundleState::Failed)   == 0u);
    REQUIRE(ed.countCompressed()                   == 2u);
    auto* found = ed.findBundle(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->bundleType() == BundleType::Optional);
    REQUIRE(ed.findBundle(99) == nullptr);
}

TEST_CASE("AssetBundleEditor settings mutation", "[Editor][S123]") {
    AssetBundleEditor ed;
    ed.setIsShowStale(true);
    ed.setIsGroupByType(true);
    ed.setCompressionLevel(9u);
    REQUIRE(ed.isShowStale());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.compressionLevel() == 9u);
}

// ── PackagingEditor ───────────────────────────────────────────────────────────

TEST_CASE("PackageFormat names", "[Editor][S123]") {
    REQUIRE(std::string(packageFormatName(PackageFormat::Zip))     == "Zip");
    REQUIRE(std::string(packageFormatName(PackageFormat::Tar))     == "Tar");
    REQUIRE(std::string(packageFormatName(PackageFormat::Pak))     == "Pak");
    REQUIRE(std::string(packageFormatName(PackageFormat::Iso))     == "Iso");
    REQUIRE(std::string(packageFormatName(PackageFormat::Flatpak)) == "Flatpak");
    REQUIRE(std::string(packageFormatName(PackageFormat::Custom))  == "Custom");
}

TEST_CASE("PackagePlatform names", "[Editor][S123]") {
    REQUIRE(std::string(packagePlatformName(PackagePlatform::Windows)) == "Windows");
    REQUIRE(std::string(packagePlatformName(PackagePlatform::MacOS))   == "MacOS");
    REQUIRE(std::string(packagePlatformName(PackagePlatform::Linux))   == "Linux");
    REQUIRE(std::string(packagePlatformName(PackagePlatform::Android)) == "Android");
    REQUIRE(std::string(packagePlatformName(PackagePlatform::iOS))     == "iOS");
    REQUIRE(std::string(packagePlatformName(PackagePlatform::Console)) == "Console");
    REQUIRE(std::string(packagePlatformName(PackagePlatform::Web))     == "Web");
}

TEST_CASE("PackageEntry defaults", "[Editor][S123]") {
    PackageEntry p(1, "win_release", PackageFormat::Zip, PackagePlatform::Windows);
    REQUIRE(p.id()            == 1u);
    REQUIRE(p.name()          == "win_release");
    REQUIRE(p.format()        == PackageFormat::Zip);
    REQUIRE(p.platform()      == PackagePlatform::Windows);
    REQUIRE(p.versionString() == "1.0.0");
    REQUIRE(!p.isRelease());
}

TEST_CASE("PackageEntry mutation", "[Editor][S123]") {
    PackageEntry p(2, "linux_pkg", PackageFormat::Flatpak, PackagePlatform::Linux);
    p.setVersionString("2.3.1");
    p.setIsRelease(true);
    REQUIRE(p.versionString() == "2.3.1");
    REQUIRE(p.isRelease());
}

TEST_CASE("PackagingEditor defaults", "[Editor][S123]") {
    PackagingEditor ed;
    REQUIRE(!ed.isShowDraft());
    REQUIRE(!ed.isGroupByPlatform());
    REQUIRE(ed.outputDir()     == "output");
    REQUIRE(ed.packageCount()  == 0u);
}

TEST_CASE("PackagingEditor add/remove packages", "[Editor][S123]") {
    PackagingEditor ed;
    REQUIRE(ed.addPackage(PackageEntry(1, "pkg_a", PackageFormat::Zip,     PackagePlatform::Windows)));
    REQUIRE(ed.addPackage(PackageEntry(2, "pkg_b", PackageFormat::Tar,     PackagePlatform::Linux)));
    REQUIRE(ed.addPackage(PackageEntry(3, "pkg_c", PackageFormat::Flatpak, PackagePlatform::Linux)));
    REQUIRE(!ed.addPackage(PackageEntry(1, "pkg_a", PackageFormat::Zip,    PackagePlatform::Windows)));
    REQUIRE(ed.packageCount() == 3u);
    REQUIRE(ed.removePackage(2));
    REQUIRE(ed.packageCount() == 2u);
    REQUIRE(!ed.removePackage(99));
}

TEST_CASE("PackagingEditor counts and find", "[Editor][S123]") {
    PackagingEditor ed;
    PackageEntry p1(1, "p1", PackageFormat::Zip,     PackagePlatform::Windows);
    PackageEntry p2(2, "p2", PackageFormat::Zip,     PackagePlatform::Linux);   p2.setIsRelease(true);
    PackageEntry p3(3, "p3", PackageFormat::Tar,     PackagePlatform::Linux);
    PackageEntry p4(4, "p4", PackageFormat::Flatpak, PackagePlatform::Android); p4.setIsRelease(true);
    ed.addPackage(p1); ed.addPackage(p2); ed.addPackage(p3); ed.addPackage(p4);
    REQUIRE(ed.countByFormat(PackageFormat::Zip)          == 2u);
    REQUIRE(ed.countByFormat(PackageFormat::Tar)          == 1u);
    REQUIRE(ed.countByFormat(PackageFormat::Pak)          == 0u);
    REQUIRE(ed.countByPlatform(PackagePlatform::Linux)    == 2u);
    REQUIRE(ed.countByPlatform(PackagePlatform::Windows)  == 1u);
    REQUIRE(ed.countByPlatform(PackagePlatform::MacOS)    == 0u);
    REQUIRE(ed.countRelease()                             == 2u);
    auto* found = ed.findPackage(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->format() == PackageFormat::Tar);
    REQUIRE(ed.findPackage(99) == nullptr);
}

TEST_CASE("PackagingEditor settings mutation", "[Editor][S123]") {
    PackagingEditor ed;
    ed.setIsShowDraft(true);
    ed.setIsGroupByPlatform(true);
    ed.setOutputDir("dist/release");
    REQUIRE(ed.isShowDraft());
    REQUIRE(ed.isGroupByPlatform());
    REQUIRE(ed.outputDir() == "dist/release");
}

// ── DeploymentEditor ──────────────────────────────────────────────────────────

TEST_CASE("DeploymentEnv names", "[Editor][S123]") {
    REQUIRE(std::string(deploymentEnvName(DeploymentEnv::Dev))        == "Dev");
    REQUIRE(std::string(deploymentEnvName(DeploymentEnv::Staging))    == "Staging");
    REQUIRE(std::string(deploymentEnvName(DeploymentEnv::QA))         == "QA");
    REQUIRE(std::string(deploymentEnvName(DeploymentEnv::Production)) == "Production");
    REQUIRE(std::string(deploymentEnvName(DeploymentEnv::Canary))     == "Canary");
}

TEST_CASE("DeploymentStatus names", "[Editor][S123]") {
    REQUIRE(std::string(deploymentStatusName(DeploymentStatus::NotDeployed)) == "NotDeployed");
    REQUIRE(std::string(deploymentStatusName(DeploymentStatus::Queued))      == "Queued");
    REQUIRE(std::string(deploymentStatusName(DeploymentStatus::Deploying))   == "Deploying");
    REQUIRE(std::string(deploymentStatusName(DeploymentStatus::Live))        == "Live");
    REQUIRE(std::string(deploymentStatusName(DeploymentStatus::Rollback))    == "Rollback");
    REQUIRE(std::string(deploymentStatusName(DeploymentStatus::Failed))      == "Failed");
}

TEST_CASE("DeployRecord defaults", "[Editor][S123]") {
    DeployRecord r(1, "api_service", DeploymentEnv::Dev, DeploymentStatus::NotDeployed);
    REQUIRE(r.id()             == 1u);
    REQUIRE(r.name()           == "api_service");
    REQUIRE(r.env()            == DeploymentEnv::Dev);
    REQUIRE(r.status()         == DeploymentStatus::NotDeployed);
    REQUIRE(r.buildVersion()   == "0.0.0");
    REQUIRE(!r.isAutoRollback());
}

TEST_CASE("DeployRecord mutation", "[Editor][S123]") {
    DeployRecord r(2, "frontend", DeploymentEnv::Staging, DeploymentStatus::NotDeployed);
    r.setStatus(DeploymentStatus::Live);
    r.setBuildVersion("1.5.2");
    r.setIsAutoRollback(true);
    REQUIRE(r.status()         == DeploymentStatus::Live);
    REQUIRE(r.buildVersion()   == "1.5.2");
    REQUIRE(r.isAutoRollback());
}

TEST_CASE("DeploymentEditor defaults", "[Editor][S123]") {
    DeploymentEditor ed;
    REQUIRE(!ed.isShowFailed());
    REQUIRE(!ed.isGroupByEnv());
    REQUIRE(ed.retryLimit()  == 3u);
    REQUIRE(ed.recordCount() == 0u);
}

TEST_CASE("DeploymentEditor add/remove records", "[Editor][S123]") {
    DeploymentEditor ed;
    REQUIRE(ed.addRecord(DeployRecord(1, "svc_a", DeploymentEnv::Dev,     DeploymentStatus::Live)));
    REQUIRE(ed.addRecord(DeployRecord(2, "svc_b", DeploymentEnv::Staging, DeploymentStatus::Queued)));
    REQUIRE(ed.addRecord(DeployRecord(3, "svc_c", DeploymentEnv::QA,      DeploymentStatus::NotDeployed)));
    REQUIRE(!ed.addRecord(DeployRecord(1, "svc_a", DeploymentEnv::Dev,    DeploymentStatus::Live)));
    REQUIRE(ed.recordCount() == 3u);
    REQUIRE(ed.removeRecord(2));
    REQUIRE(ed.recordCount() == 2u);
    REQUIRE(!ed.removeRecord(99));
}

TEST_CASE("DeploymentEditor counts and find", "[Editor][S123]") {
    DeploymentEditor ed;
    DeployRecord r1(1, "r1", DeploymentEnv::Dev,        DeploymentStatus::Live);
    DeployRecord r2(2, "r2", DeploymentEnv::Dev,        DeploymentStatus::Failed);  r2.setIsAutoRollback(true);
    DeployRecord r3(3, "r3", DeploymentEnv::Staging,    DeploymentStatus::Live);
    DeployRecord r4(4, "r4", DeploymentEnv::Production, DeploymentStatus::Rollback); r4.setIsAutoRollback(true);
    ed.addRecord(r1); ed.addRecord(r2); ed.addRecord(r3); ed.addRecord(r4);
    REQUIRE(ed.countByEnv(DeploymentEnv::Dev)               == 2u);
    REQUIRE(ed.countByEnv(DeploymentEnv::Staging)           == 1u);
    REQUIRE(ed.countByEnv(DeploymentEnv::Canary)            == 0u);
    REQUIRE(ed.countByStatus(DeploymentStatus::Live)        == 2u);
    REQUIRE(ed.countByStatus(DeploymentStatus::Failed)      == 1u);
    REQUIRE(ed.countByStatus(DeploymentStatus::Queued)      == 0u);
    REQUIRE(ed.countAutoRollback()                          == 2u);
    auto* found = ed.findRecord(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->env() == DeploymentEnv::Staging);
    REQUIRE(ed.findRecord(99) == nullptr);
}

TEST_CASE("DeploymentEditor settings mutation", "[Editor][S123]") {
    DeploymentEditor ed;
    ed.setIsShowFailed(true);
    ed.setIsGroupByEnv(true);
    ed.setRetryLimit(5u);
    REQUIRE(ed.isShowFailed());
    REQUIRE(ed.isGroupByEnv());
    REQUIRE(ed.retryLimit() == 5u);
}
