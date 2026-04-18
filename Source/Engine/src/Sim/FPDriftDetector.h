#pragma once
// ============================================================
// Atlas FP Drift Detector — Cross-Platform Determinism Checker
// ============================================================
//
// Compares hash ladders produced by identical simulations
// running on two different platforms. When floating-point
// drift causes a divergence, generates an FPDriftReport
// identifying the first tick and per-system breakdown of
// which systems first diverge.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ATLAS_DETERMINISM_ENFORCEMENT.md

#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#include "StateHasher.h"     // for HashEntry

namespace atlas::sim {

/// Per-system drift information at the divergence point.
struct FPSystemDrift {
    std::string systemName;
    uint64_t hashA   = 0;
    uint64_t hashB   = 0;
    bool     matches = true;
};

/// Report describing cross-platform floating-point drift.
struct FPDriftReport {
    bool        hasDrift            = false;  ///< Whether drift was detected
    int64_t     firstDivergenceTick = -1;     ///< Tick where drift first occurred (-1 if none)
    uint64_t    platformAHash       = 0;      ///< Hash on platform A at divergence
    uint64_t    platformBHash       = 0;      ///< Hash on platform B at divergence
    std::string platformAName;                ///< Name of platform A
    std::string platformBName;                ///< Name of platform B
    uint32_t    totalTicksCompared  = 0;      ///< Total ticks that were compared
    uint32_t    matchingTicks       = 0;      ///< Ticks that matched before divergence
    std::vector<FPSystemDrift> systemDrifts;  ///< Per-system drift info
};

/// Detects floating-point drift between two platform runs.
class FPDriftDetector {
public:
    // --- Platform identification ---

    /// Set human-readable names for the two platforms being compared.
    void SetPlatformNames(const std::string& nameA, const std::string& nameB);

    // --- Core comparison ---

    /// Compare two StateHasher hash ladders from different platforms.
    FPDriftReport RunComparison(const StateHasher& platformA,
                                const StateHasher& platformB);

    // --- Per-system analysis ---

    /// Add per-system hashes for detailed drift analysis.
    void AddSystemHashes(
        const std::string& systemName,
        const std::vector<std::pair<std::string, uint64_t>>& perSystemA,
        const std::vector<std::pair<std::string, uint64_t>>& perSystemB);

    /// Compare per-system hash lists and return drift entries.
    static std::vector<FPSystemDrift> CompareSystemHashes(
        const std::vector<std::pair<std::string, uint64_t>>& localSys,
        const std::vector<std::pair<std::string, uint64_t>>& remoteSys);

    // --- Report formatting ---

    /// Format a report as a multi-line string for logging / display.
    static std::string FormatReport(const FPDriftReport& report);

    /// Export a report to a file.
    static bool ExportReport(const FPDriftReport& report,
                             const std::string& path);

private:
    std::string m_platformAName = "PlatformA";
    std::string m_platformBName = "PlatformB";
    std::vector<FPSystemDrift> m_pendingSystemDrifts;
};

}  // namespace atlas::sim
