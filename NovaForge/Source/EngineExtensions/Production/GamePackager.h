#pragma once
#include "AssetCooker.h"
#include "BuildProfile.h"
#include "PlatformTarget.h"
#include "BuildManifest.h"
#include <string>
#include <vector>
#include <functional>

namespace atlas::production {

enum class PackageResult {
    Success,
    ValidationFailed,
    CookFailed,
    BundleFailed,
    OutputError
};

enum class PackageTarget {
    Client,
    Server
};

struct PackageConfig {
    PackageTarget target = PackageTarget::Client;
    ProfileType profile = ProfileType::Release;
    Platform platform = Platform::Linux;
    std::string sourceDir = "assets";
    std::string outputDir = "./build/package";
    bool includeMods = false;
    bool stripEditorData = true;
    bool stripTools = true;
    bool singleExecutable = false;
};

struct PackageReport {
    PackageResult result = PackageResult::Success;
    CookStats cookStats;
    std::string manifestPath;
    std::string outputDir;
    std::string errorMessage;
    std::vector<std::string> warnings;
    std::vector<std::string> outputFiles;
};

class GamePackager {
public:
    using ProgressCallback = std::function<void(const std::string& stage, float progress)>;

    /// Run the full packaging pipeline: Validate → Cook → Bundle → Emit.
    PackageReport Package(const PackageConfig& config);

    /// Set a progress callback for the packaging stages.
    void SetProgressCallback(ProgressCallback cb);

    /// Validate the source directory and configuration.
    bool Validate(const PackageConfig& config, std::string& outError);

    /// Get the last report.
    const PackageReport& LastReport() const;

private:
    bool CookAssets(const PackageConfig& config, PackageReport& report);
    bool BundleOutput(const PackageConfig& config, PackageReport& report);
    bool EmitManifest(const PackageConfig& config, PackageReport& report);

    ProgressCallback m_progressCb;
    PackageReport m_lastReport;
};

}
