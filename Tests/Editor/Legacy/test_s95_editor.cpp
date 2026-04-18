#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AssetPackager.h"

using namespace NF;

// ── S95: AssetPackager + PlatformProfile + BuildReport ───────────

// ── AssetPackager ────────────────────────────────────────────────

TEST_CASE("PackageTargetPlatform names are correct", "[Editor][S95]") {
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::Windows)) == "Windows");
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::Linux))   == "Linux");
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::macOS))   == "macOS");
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::Android)) == "Android");
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::iOS))     == "iOS");
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::Console)) == "Console");
    REQUIRE(std::string(packageTargetPlatformName(PackageTargetPlatform::Web))     == "Web");
}

TEST_CASE("PackageCompressionMode names are correct", "[Editor][S95]") {
    REQUIRE(std::string(packageCompressionModeName(PackageCompressionMode::None))            == "None");
    REQUIRE(std::string(packageCompressionModeName(PackageCompressionMode::Fast))            == "Fast");
    REQUIRE(std::string(packageCompressionModeName(PackageCompressionMode::Default))         == "Default");
    REQUIRE(std::string(packageCompressionModeName(PackageCompressionMode::Best))            == "Best");
    REQUIRE(std::string(packageCompressionModeName(PackageCompressionMode::StreamingChunks)) == "StreamingChunks");
}

TEST_CASE("PackageJobStatus names are correct", "[Editor][S95]") {
    REQUIRE(std::string(packageJobStatusName(PackageJobStatus::Queued))    == "Queued");
    REQUIRE(std::string(packageJobStatusName(PackageJobStatus::Cooking))   == "Cooking");
    REQUIRE(std::string(packageJobStatusName(PackageJobStatus::Packaging)) == "Packaging");
    REQUIRE(std::string(packageJobStatusName(PackageJobStatus::Done))      == "Done");
    REQUIRE(std::string(packageJobStatusName(PackageJobStatus::Failed))    == "Failed");
    REQUIRE(std::string(packageJobStatusName(PackageJobStatus::Cancelled)) == "Cancelled");
}

TEST_CASE("PackageJob stores properties", "[Editor][S95]") {
    PackageJob job("WinShip", PackageTargetPlatform::Windows);
    job.setCompressionMode(PackageCompressionMode::Best);
    job.setStatus(PackageJobStatus::Done);
    job.setOutputPath("Dist/Win64");
    job.setProgress(100.0f);
    job.setIncludeDebugInfo(true);
    REQUIRE(job.name()         == "WinShip");
    REQUIRE(job.platform()     == PackageTargetPlatform::Windows);
    REQUIRE(job.compression()  == PackageCompressionMode::Best);
    REQUIRE(job.isDone());
    REQUIRE(job.outputPath()   == "Dist/Win64");
    REQUIRE(job.includeDebugInfo());
}

TEST_CASE("AssetPackager add findJob remove", "[Editor][S95]") {
    AssetPackager packager;
    PackageJob j1("Win", PackageTargetPlatform::Windows);
    PackageJob j2("Lin", PackageTargetPlatform::Linux);
    REQUIRE(packager.addJob(j1));
    REQUIRE(packager.addJob(j2));
    REQUIRE(packager.jobCount() == 2);
    REQUIRE(packager.findJob("Lin") != nullptr);
    packager.removeJob("Lin");
    REQUIRE(packager.jobCount() == 1);
}

TEST_CASE("AssetPackager rejects duplicate job name", "[Editor][S95]") {
    AssetPackager packager;
    PackageJob j("Android", PackageTargetPlatform::Android);
    packager.addJob(j);
    REQUIRE_FALSE(packager.addJob(j));
}

TEST_CASE("AssetPackager countByPlatform and doneCount", "[Editor][S95]") {
    AssetPackager packager;
    PackageJob j1("A", PackageTargetPlatform::Windows); j1.setStatus(PackageJobStatus::Done);
    PackageJob j2("B", PackageTargetPlatform::Windows); j2.setStatus(PackageJobStatus::Failed);
    PackageJob j3("C", PackageTargetPlatform::Linux);   j3.setStatus(PackageJobStatus::Done);
    packager.addJob(j1); packager.addJob(j2); packager.addJob(j3);
    REQUIRE(packager.countByPlatform(PackageTargetPlatform::Windows) == 2);
    REQUIRE(packager.doneCount()    == 2);
}

TEST_CASE("AssetPackager MAX_JOBS is 32", "[Editor][S95]") {
    REQUIRE(AssetPackager::MAX_JOBS == 32);
}

// ── PlatformProfile ──────────────────────────────────────────────

TEST_CASE("PlatformFeatureSet names are correct", "[Editor][S95]") {
    REQUIRE(std::string(platformFeatureSetName(PlatformFeatureSet::Desktop))  == "Desktop");
    REQUIRE(std::string(platformFeatureSetName(PlatformFeatureSet::Mobile))   == "Mobile");
    REQUIRE(std::string(platformFeatureSetName(PlatformFeatureSet::Console))  == "Console");
    REQUIRE(std::string(platformFeatureSetName(PlatformFeatureSet::XR))       == "XR");
    REQUIRE(std::string(platformFeatureSetName(PlatformFeatureSet::Server))   == "Server");
    REQUIRE(std::string(platformFeatureSetName(PlatformFeatureSet::Embedded)) == "Embedded");
}

TEST_CASE("PlatformAPILevel names are correct", "[Editor][S95]") {
    REQUIRE(std::string(platformAPILevelName(PlatformAPILevel::Minimum))     == "Minimum");
    REQUIRE(std::string(platformAPILevelName(PlatformAPILevel::Recommended)) == "Recommended");
    REQUIRE(std::string(platformAPILevelName(PlatformAPILevel::Latest))      == "Latest");
    REQUIRE(std::string(platformAPILevelName(PlatformAPILevel::Custom))      == "Custom");
}

TEST_CASE("PlatformOrientation names are correct", "[Editor][S95]") {
    REQUIRE(std::string(platformOrientationName(PlatformOrientation::Portrait))  == "Portrait");
    REQUIRE(std::string(platformOrientationName(PlatformOrientation::Landscape)) == "Landscape");
    REQUIRE(std::string(platformOrientationName(PlatformOrientation::Both))      == "Both");
    REQUIRE(std::string(platformOrientationName(PlatformOrientation::Auto))      == "Auto");
}

TEST_CASE("PlatformCapability stores properties", "[Editor][S95]") {
    PlatformCapability cap("Bluetooth", true);
    cap.setDescription("Requires Bluetooth 5.0");
    REQUIRE(cap.name()        == "Bluetooth");
    REQUIRE(cap.isRequired());
    REQUIRE(cap.isEnabled());
    REQUIRE(cap.description() == "Requires Bluetooth 5.0");
}

TEST_CASE("PlatformProfile stores properties and addCapability", "[Editor][S95]") {
    PlatformProfile profile("AndroidBase", PlatformFeatureSet::Mobile);
    profile.setAPILevel(PlatformAPILevel::Recommended);
    profile.setOrientation(PlatformOrientation::Both);
    profile.setActive(true);
    REQUIRE(profile.name()        == "AndroidBase");
    REQUIRE(profile.featureSet()  == PlatformFeatureSet::Mobile);
    REQUIRE(profile.apiLevel()    == PlatformAPILevel::Recommended);
    REQUIRE(profile.isActive());
}

TEST_CASE("PlatformProfile add remove findCapability", "[Editor][S95]") {
    PlatformProfile profile("PC", PlatformFeatureSet::Desktop);
    PlatformCapability c1("D3D12",   true);
    PlatformCapability c2("Vulkan",  false);
    REQUIRE(profile.addCapability(c1));
    REQUIRE(profile.addCapability(c2));
    REQUIRE(profile.capabilityCount() == 2);
    REQUIRE(profile.findCapability("Vulkan") != nullptr);
    profile.removeCapability("Vulkan");
    REQUIRE(profile.capabilityCount() == 1);
}

TEST_CASE("PlatformProfile rejects duplicate capability name", "[Editor][S95]") {
    PlatformProfile profile("XR", PlatformFeatureSet::XR);
    PlatformCapability c("Hand Tracking", true);
    profile.addCapability(c);
    REQUIRE_FALSE(profile.addCapability(c));
}

TEST_CASE("PlatformProfile requiredCapabilityCount and enabledCapabilityCount", "[Editor][S95]") {
    PlatformProfile profile("Server", PlatformFeatureSet::Server);
    PlatformCapability c1("TCP",  true); c1.setEnabled(true);
    PlatformCapability c2("UDP",  true); c2.setEnabled(false);
    PlatformCapability c3("HTTP", false);c3.setEnabled(true);
    profile.addCapability(c1); profile.addCapability(c2); profile.addCapability(c3);
    REQUIRE(profile.requiredCapabilityCount() == 2);
    REQUIRE(profile.enabledCapabilityCount()  == 2);
}

TEST_CASE("PlatformProfile MAX_CAPABILITIES is 64", "[Editor][S95]") {
    REQUIRE(PlatformProfile::MAX_CAPABILITIES == 64);
}

// ── BuildReport ──────────────────────────────────────────────────

TEST_CASE("BuildReportSeverity names are correct", "[Editor][S95]") {
    REQUIRE(std::string(buildReportSeverityName(BuildReportSeverity::Info))    == "Info");
    REQUIRE(std::string(buildReportSeverityName(BuildReportSeverity::Warning)) == "Warning");
    REQUIRE(std::string(buildReportSeverityName(BuildReportSeverity::Error))   == "Error");
    REQUIRE(std::string(buildReportSeverityName(BuildReportSeverity::Fatal))   == "Fatal");
}

TEST_CASE("BuildReportCategory names are correct", "[Editor][S95]") {
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Shader))      == "Shader");
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Asset))       == "Asset");
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Code))        == "Code");
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Config))      == "Config");
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Dependency))  == "Dependency");
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Size))        == "Size");
    REQUIRE(std::string(buildReportCategoryName(BuildReportCategory::Performance)) == "Performance");
}

TEST_CASE("BuildReportStatus names are correct", "[Editor][S95]") {
    REQUIRE(std::string(buildReportStatusName(BuildReportStatus::Empty))               == "Empty");
    REQUIRE(std::string(buildReportStatusName(BuildReportStatus::Building))            == "Building");
    REQUIRE(std::string(buildReportStatusName(BuildReportStatus::Success))             == "Success");
    REQUIRE(std::string(buildReportStatusName(BuildReportStatus::SuccessWithWarnings)) == "SuccessWithWarnings");
    REQUIRE(std::string(buildReportStatusName(BuildReportStatus::Failed))              == "Failed");
}

TEST_CASE("BuildReportEntry stores properties", "[Editor][S95]") {
    BuildReportEntry entry("Shader compilation failed", BuildReportSeverity::Error, BuildReportCategory::Shader);
    entry.setSource("Materials/Water.shader");
    entry.setLine(42);
    REQUIRE(entry.message()   == "Shader compilation failed");
    REQUIRE(entry.severity()  == BuildReportSeverity::Error);
    REQUIRE(entry.category()  == BuildReportCategory::Shader);
    REQUIRE(entry.source()    == "Materials/Water.shader");
    REQUIRE(entry.line()      == 42);
    REQUIRE(entry.isError());
    REQUIRE_FALSE(entry.isWarning());
    REQUIRE_FALSE(entry.isResolved());
}

TEST_CASE("BuildReport addEntry clearEntries and counters", "[Editor][S95]") {
    BuildReport report;
    report.setStatus(BuildReportStatus::SuccessWithWarnings);
    report.setBuildTime(45.5f);
    report.setOutputSizeBytes(1024 * 1024 * 200);
    BuildReportEntry e1("Missing texture", BuildReportSeverity::Warning, BuildReportCategory::Asset);
    BuildReportEntry e2("Shader error",    BuildReportSeverity::Error,   BuildReportCategory::Shader);
    BuildReportEntry e3("Config note",     BuildReportSeverity::Info,    BuildReportCategory::Config);
    REQUIRE(report.addEntry(e1));
    REQUIRE(report.addEntry(e2));
    REQUIRE(report.addEntry(e3));
    REQUIRE(report.entryCount()  == 3);
    REQUIRE(report.warningCount()== 1);
    REQUIRE(report.errorCount()  == 1);
    REQUIRE(report.isSuccess());
}

TEST_CASE("BuildReport countBySeverity and countByCategory", "[Editor][S95]") {
    BuildReport report;
    BuildReportEntry e1("A", BuildReportSeverity::Warning, BuildReportCategory::Asset);
    BuildReportEntry e2("B", BuildReportSeverity::Warning, BuildReportCategory::Shader);
    BuildReportEntry e3("C", BuildReportSeverity::Error,   BuildReportCategory::Asset);
    report.addEntry(e1); report.addEntry(e2); report.addEntry(e3);
    REQUIRE(report.countBySeverity(BuildReportSeverity::Warning)    == 2);
    REQUIRE(report.countByCategory(BuildReportCategory::Asset)      == 2);
}

TEST_CASE("BuildReport clearEntries resets entries", "[Editor][S95]") {
    BuildReport report;
    BuildReportEntry entry("Info msg", BuildReportSeverity::Info, BuildReportCategory::Code);
    report.addEntry(entry);
    report.clearEntries();
    REQUIRE(report.entryCount() == 0);
}

TEST_CASE("BuildReport resolvedCount", "[Editor][S95]") {
    BuildReport report;
    BuildReportEntry e1("A", BuildReportSeverity::Warning, BuildReportCategory::Asset); e1.setResolved(true);
    BuildReportEntry e2("B", BuildReportSeverity::Error,   BuildReportCategory::Code);
    report.addEntry(e1); report.addEntry(e2);
    REQUIRE(report.resolvedCount() == 1);
}

TEST_CASE("BuildReport MAX_ENTRIES is 4096", "[Editor][S95]") {
    REQUIRE(BuildReport::MAX_ENTRIES == 4096);
}
