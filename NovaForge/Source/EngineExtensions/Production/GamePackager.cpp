#include "GamePackager.h"
#include <filesystem>
#include <fstream>

namespace atlas::production {

PackageReport GamePackager::Package(const PackageConfig& config) {
    PackageReport report;
    report.outputDir = config.outputDir;

    // Stage 1: Validate
    if (m_progressCb) m_progressCb("Validate", 0.0f);
    std::string validationError;
    if (!Validate(config, validationError)) {
        report.result = PackageResult::ValidationFailed;
        report.errorMessage = validationError;
        m_lastReport = report;
        return report;
    }
    if (m_progressCb) m_progressCb("Validate", 1.0f);

    // Stage 2: Cook assets
    if (m_progressCb) m_progressCb("Cook", 0.0f);
    if (!CookAssets(config, report)) {
        report.result = PackageResult::CookFailed;
        m_lastReport = report;
        return report;
    }
    if (m_progressCb) m_progressCb("Cook", 1.0f);

    // Stage 3: Bundle output
    if (m_progressCb) m_progressCb("Bundle", 0.0f);
    if (!BundleOutput(config, report)) {
        report.result = PackageResult::BundleFailed;
        m_lastReport = report;
        return report;
    }
    if (m_progressCb) m_progressCb("Bundle", 1.0f);

    // Stage 4: Emit manifest
    if (m_progressCb) m_progressCb("Emit", 0.0f);
    if (!EmitManifest(config, report)) {
        report.result = PackageResult::OutputError;
        m_lastReport = report;
        return report;
    }
    if (m_progressCb) m_progressCb("Emit", 1.0f);

    report.result = PackageResult::Success;
    m_lastReport = report;
    return report;
}

void GamePackager::SetProgressCallback(ProgressCallback cb) {
    m_progressCb = std::move(cb);
}

bool GamePackager::Validate(const PackageConfig& config, std::string& outError) {
    if (config.sourceDir.empty()) {
        outError = "Source directory is empty";
        return false;
    }
    if (config.outputDir.empty()) {
        outError = "Output directory is empty";
        return false;
    }
    if (!std::filesystem::exists(config.sourceDir)) {
        outError = "Source directory does not exist: " + config.sourceDir;
        return false;
    }
    return true;
}

const PackageReport& GamePackager::LastReport() const {
    return m_lastReport;
}

bool GamePackager::CookAssets(const PackageConfig& config, PackageReport& report) {
    AssetCooker cooker;

    std::string cookDir = config.outputDir + "/cooked";
    cooker.SetOutputDir(cookDir);
    cooker.SetStripEditorData(config.stripEditorData);

    auto stats = cooker.CookAll(config.sourceDir);
    report.cookStats = stats;

    if (stats.failedAssets > 0) {
        report.errorMessage = "Failed to cook " + std::to_string(stats.failedAssets) + " assets";
        report.warnings.push_back(report.errorMessage);
        // Only fail if ALL assets failed
        if (stats.cookedAssets == 0 && stats.totalAssets > 0) {
            return false;
        }
    }

    // Collect cooked output files
    if (std::filesystem::exists(cookDir)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(cookDir)) {
            if (entry.is_regular_file()) {
                report.outputFiles.push_back(entry.path().string());
            }
        }
    }

    return true;
}

bool GamePackager::BundleOutput(const PackageConfig& config, PackageReport& report) {
    std::filesystem::create_directories(config.outputDir);

    // Determine executable name based on target and platform
    PlatformConfig platformCfg = PlatformTarget::DefaultFor(config.platform);
    std::string execName;
    switch (config.target) {
        case PackageTarget::Client:
            execName = "AtlasClient" + platformCfg.outputExtension;
            break;
        case PackageTarget::Server:
            execName = "AtlasServer" + platformCfg.outputExtension;
            break;
    }

    // Create a placeholder executable manifest entry
    std::string execPath = config.outputDir + "/" + execName;
    report.outputFiles.push_back(execPath);

    // Create data directory
    std::string dataDir = config.outputDir + "/data";
    std::filesystem::create_directories(dataDir);

    // Move cooked assets to data directory
    std::string cookDir = config.outputDir + "/cooked";
    if (std::filesystem::exists(cookDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(cookDir)) {
            if (entry.is_regular_file()) {
                std::filesystem::path dest = std::filesystem::path(dataDir) / entry.path().filename();
                std::filesystem::copy_file(entry.path(), dest,
                    std::filesystem::copy_options::overwrite_existing);
            }
        }
    }

    return true;
}

bool GamePackager::EmitManifest(const PackageConfig& config, PackageReport& report) {
    BuildManifest manifest;
    manifest.projectName = "AtlasPackage";
    manifest.buildType = BuildProfile::ProfileName(config.profile);
    manifest.platform = PlatformTarget::PlatformName(config.platform);
    manifest.engineVersion = "1.0.0";

    // Add artifact entries for output files
    for (const auto& file : report.outputFiles) {
        ArtifactEntry entry;
        entry.path = file;
        entry.type = "asset";
        manifest.AddArtifact(entry);
    }

    std::string manifestPath = config.outputDir + "/build.manifest";
    std::string json = manifest.ToJSON();

    std::ofstream out(manifestPath);
    if (!out.is_open()) {
        report.errorMessage = "Failed to write manifest: " + manifestPath;
        return false;
    }
    out << json;
    out.close();

    report.manifestPath = manifestPath;
    return true;
}

}
