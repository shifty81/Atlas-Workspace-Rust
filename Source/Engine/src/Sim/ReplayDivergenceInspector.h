#pragma once
// ============================================================
// Atlas Replay Divergence Inspector — Auto-Diff on Hash Mismatch
// ============================================================
//
// Compares hash ladders and replay frame streams to detect
// determinism divergences. When a mismatch is found, generates
// a DivergenceReport with tick, hashes, and severity info
// suitable for auto-opening a diff view in the editor.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ATLAS_DETERMINISM_ENFORCEMENT.md

#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#include "ReplayRecorder.h"  // for ReplayFrame
#include "StateHasher.h"     // for HashEntry

namespace atlas::sim {

/// Severity of a divergence.
enum class DivergenceSeverity {
    None,      ///< No divergence detected
    Warning,   ///< Divergence occurred late in the stream
    Critical   ///< Divergence occurred early or immediately
};

/// Report describing where and how two simulations diverged.
struct DivergenceReport {
    int64_t  divergeTick       = -1;   ///< Tick of first mismatch (-1 = none)
    uint64_t localHash         = 0;    ///< Local hash at divergeTick
    uint64_t remoteHash        = 0;    ///< Remote hash at divergeTick
    uint32_t totalTicksCompared = 0;   ///< Total ticks that were compared
    uint32_t firstMatchingTicks = 0;   ///< Ticks that matched before divergence
    DivergenceSeverity severity = DivergenceSeverity::None;
};

/// Per-system state difference at a specific tick.
struct SystemStateDiff {
    std::string systemName;
    uint64_t localHash = 0;
    uint64_t remoteHash = 0;
    bool matches = true;
};

/// Extended report with per-system breakdown.
struct DetailedDivergenceReport {
    DivergenceReport baseReport;
    std::vector<SystemStateDiff> systemDiffs;
};

/// Inspects replay and hash-ladder data for divergences.
class ReplayDivergenceInspector {
public:
    // --- Static comparison helpers ---

    /// Compare two StateHasher hash ladders.
    static DivergenceReport Compare(const StateHasher& local,
                                    const StateHasher& remote);

    /// Compare two replay frame streams using their stateHash fields.
    static DivergenceReport CompareFrames(const std::vector<ReplayFrame>& a,
                                          const std::vector<ReplayFrame>& b);

    /// Quick single-frame equality check.
    static bool CheckFrame(const ReplayFrame& expected,
                           const ReplayFrame& actual);

    /// Return a human-readable analysis string for a report.
    static std::string AnalyzePattern(const DivergenceReport& report);

    // --- Report formatting ---

    /// Format a report as a multi-line string for logging / display.
    static std::string FormatReport(const DivergenceReport& report);

    /// Export a report to a file.
    static bool ExportReport(const DivergenceReport& report, const std::string& path);

    /// Whether the report indicates a critical divergence.
    static bool IsCritical(const DivergenceReport& report);

    /// Build a detailed report with per-system state diff at the divergence tick.
    static DetailedDivergenceReport CompareDetailed(
        const StateHasher& local,
        const StateHasher& remote,
        const std::vector<std::pair<std::string, uint64_t>>& localSystemHashes,
        const std::vector<std::pair<std::string, uint64_t>>& remoteSystemHashes);

    // --- Instance history tracking ---

    /// Store a report in the history ring buffer.
    void RecordReport(const DivergenceReport& report);

    /// Last recorded report.
    const DivergenceReport& LastReport() const;

    /// Full report history.
    const std::vector<DivergenceReport>& ReportHistory() const;

    /// Clear all stored reports.
    void ClearHistory();

    // --- Replay Minimizer ---

    /// Result of a replay minimization.
    struct MinimizedResult {
        int64_t  firstDivergentTick = -1;   ///< First tick that causes divergence
        int64_t  lastMatchingTick   = -1;   ///< Last tick that was still matching
        uint32_t windowSize         = 0;    ///< Number of ticks in the divergent window
        uint32_t iterationsUsed     = 0;    ///< Binary-search iterations performed
        bool     minimized          = false; ///< True if minimization succeeded
    };

    /// Binary-search the replay frame stream to find the smallest window
    /// around the first divergence point.  Compares frames from `a` and `b`
    /// and returns a MinimizedResult identifying the exact divergent tick.
    static MinimizedResult MinimizeDivergence(
        const std::vector<ReplayFrame>& a,
        const std::vector<ReplayFrame>& b);

    /// Minimize using pre-computed StateHasher ladders.
    static MinimizedResult MinimizeDivergenceFromHashers(
        const StateHasher& local,
        const StateHasher& remote);

private:
    DivergenceReport              m_lastReport{};
    std::vector<DivergenceReport> m_reportsHistory;
    size_t                        m_maxReports = 100;
};

}  // namespace atlas::sim
