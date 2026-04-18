#pragma once
// ============================================================
// Atlas Desync Reproducer — One-Command Bug Reproduction
// ============================================================
//
// Captures state snapshot + replay data on desync detection,
// then provides a one-command reproduction path.
//
// Usage:
//   DesyncReproducer reproducer;
//   reproducer.SetOutputDirectory("/tmp/atlas_repro");
//   reproducer.CaptureDesync(tick, stateData, replayData);
//   // Later:
//   reproducer.GenerateReproCommand(...);
//
// See: docs/ATLAS_LOCKDOWN_CHECKLIST.md (One-command reproduction)

#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/// Bundle of data captured at the point of a desync.
struct DesyncCapture {
    uint64_t tick = 0;
    uint64_t localHash = 0;
    uint64_t remoteHash = 0;
    std::string savePath;
    std::string replayPath;
    std::string reportPath;
    bool valid = false;
};

/// Captures desync state and generates reproduction artifacts.
class DesyncReproducer {
public:
    /// Set the directory where reproduction artifacts are written.
    void SetOutputDirectory(const std::string& dir);

    /// Returns the configured output directory.
    const std::string& OutputDirectory() const;

    /// Capture desync state: saves world state, replay data, and
    /// a human-readable report into the output directory.
    /// Returns a DesyncCapture describing the files produced.
    DesyncCapture CaptureDesync(uint64_t tick,
                                uint64_t localHash,
                                uint64_t remoteHash,
                                const std::vector<uint8_t>& ecsData,
                                uint32_t tickRate,
                                uint64_t seed,
                                const std::vector<uint8_t>& replayData);

    /// Generate a shell command that reproduces the desync.
    static std::string GenerateReproCommand(const DesyncCapture& capture);

    /// Returns the number of captures produced so far.
    size_t CaptureCount() const;

    /// Returns all captures produced.
    const std::vector<DesyncCapture>& Captures() const;

    // --- Crash Report Bundle ---

    /// A portable bundle containing everything needed to reproduce a desync.
    struct CrashReportBundle {
        DesyncCapture capture;
        std::string engineVersion;
        std::string platformId;
        uint32_t tickRate = 0;
        uint64_t seed = 0;
        std::string bundlePath;   ///< Path to the written bundle file
        bool valid = false;
    };

    /// Set the engine version string included in crash bundles.
    void SetEngineVersion(const std::string& version);
    const std::string& EngineVersion() const;

    /// Set the platform identifier included in crash bundles.
    void SetPlatformId(const std::string& platformId);
    const std::string& PlatformId() const;

    /// Build a CrashReportBundle from the most recent capture.
    /// Writes a single `.atlascrash` manifest alongside the existing
    /// save/replay/report files.
    CrashReportBundle BuildCrashBundle(uint32_t tickRate, uint64_t seed);

    /// Build a CrashReportBundle from a specific capture index.
    CrashReportBundle BuildCrashBundleAt(size_t index, uint32_t tickRate, uint64_t seed);

    /// Returns all bundles produced.
    const std::vector<CrashReportBundle>& Bundles() const;

private:
    std::string m_outputDir = "/tmp/atlas_repro";
    std::vector<DesyncCapture> m_captures;
    std::string m_engineVersion = "dev";
    std::string m_platformId = "unknown";
    std::vector<CrashReportBundle> m_bundles;
};

}  // namespace atlas::sim
