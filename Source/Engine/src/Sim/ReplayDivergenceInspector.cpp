#include "ReplayDivergenceInspector.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>

namespace atlas::sim {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

static DivergenceSeverity ClassifySeverity(const DivergenceReport& r) {
    if (r.divergeTick < 0)
        return DivergenceSeverity::None;
    if (r.totalTicksCompared == 0 || r.firstMatchingTicks == 0)
        return DivergenceSeverity::Critical;
    double ratio = static_cast<double>(r.firstMatchingTicks)
                   / static_cast<double>(r.totalTicksCompared);
    return (ratio < 0.5) ? DivergenceSeverity::Critical
                         : DivergenceSeverity::Warning;
}

// ---------------------------------------------------------------------------
// Compare two StateHasher ladders
// ---------------------------------------------------------------------------

DivergenceReport ReplayDivergenceInspector::Compare(
        const StateHasher& local,
        const StateHasher& remote) {

    const auto& lh = local.History();
    const auto& rh = remote.History();

    DivergenceReport report;
    report.totalTicksCompared = static_cast<uint32_t>(
        std::min(lh.size(), rh.size()));

    for (uint32_t i = 0; i < report.totalTicksCompared; ++i) {
        if (lh[i].hash != rh[i].hash) {
            report.divergeTick      = static_cast<int64_t>(lh[i].tick);
            report.localHash        = lh[i].hash;
            report.remoteHash       = rh[i].hash;
            report.firstMatchingTicks = i;
            report.severity         = ClassifySeverity(report);
            return report;
        }
    }

    // No divergence
    report.firstMatchingTicks = report.totalTicksCompared;
    report.severity           = DivergenceSeverity::None;
    return report;
}

// ---------------------------------------------------------------------------
// Compare two frame streams
// ---------------------------------------------------------------------------

DivergenceReport ReplayDivergenceInspector::CompareFrames(
        const std::vector<ReplayFrame>& a,
        const std::vector<ReplayFrame>& b) {

    DivergenceReport report;
    report.totalTicksCompared = static_cast<uint32_t>(
        std::min(a.size(), b.size()));

    for (uint32_t i = 0; i < report.totalTicksCompared; ++i) {
        if (a[i].stateHash != b[i].stateHash) {
            report.divergeTick      = static_cast<int64_t>(a[i].tick);
            report.localHash        = a[i].stateHash;
            report.remoteHash       = b[i].stateHash;
            report.firstMatchingTicks = i;
            report.severity         = ClassifySeverity(report);
            return report;
        }
    }

    report.firstMatchingTicks = report.totalTicksCompared;
    report.severity           = DivergenceSeverity::None;
    return report;
}

// ---------------------------------------------------------------------------
// Single-frame check
// ---------------------------------------------------------------------------

bool ReplayDivergenceInspector::CheckFrame(
        const ReplayFrame& expected,
        const ReplayFrame& actual) {
    return expected.tick == actual.tick
        && expected.stateHash == actual.stateHash;
}

// ---------------------------------------------------------------------------
// Human-readable analysis
// ---------------------------------------------------------------------------

std::string ReplayDivergenceInspector::AnalyzePattern(
        const DivergenceReport& report) {
    if (report.divergeTick < 0)
        return "No divergence detected. Simulations are in sync.";

    std::ostringstream os;
    os << "Divergence at tick " << report.divergeTick << ". ";
    os << report.firstMatchingTicks << " of "
       << report.totalTicksCompared << " ticks matched before mismatch. ";

    switch (report.severity) {
        case DivergenceSeverity::Critical:
            os << "CRITICAL: early divergence suggests a fundamental desync.";
            break;
        case DivergenceSeverity::Warning:
            os << "WARNING: late divergence may indicate an intermittent issue.";
            break;
        default:
            break;
    }
    return os.str();
}

// ---------------------------------------------------------------------------
// Report formatting
// ---------------------------------------------------------------------------

std::string ReplayDivergenceInspector::FormatReport(
        const DivergenceReport& report) {
    std::ostringstream os;
    os << "=== Divergence Report ===" << "\n";

    if (report.divergeTick < 0) {
        os << "Status    : OK — no divergence\n";
    } else {
        os << "Status    : DIVERGED\n";
        os << "Tick      : " << report.divergeTick << "\n";
        os << "Local     : 0x" << std::hex << std::setw(16)
           << std::setfill('0') << report.localHash << "\n";
        os << "Remote    : 0x" << std::hex << std::setw(16)
           << std::setfill('0') << report.remoteHash << "\n";
        os << std::dec;
    }

    os << "Compared  : " << report.totalTicksCompared << " ticks\n";
    os << "Matched   : " << report.firstMatchingTicks << " ticks\n";

    const char* sevStr = "None";
    if (report.severity == DivergenceSeverity::Warning)  sevStr = "Warning";
    if (report.severity == DivergenceSeverity::Critical) sevStr = "Critical";
    os << "Severity  : " << sevStr << "\n";

    return os.str();
}

// ---------------------------------------------------------------------------
// Export report to file
// ---------------------------------------------------------------------------

bool ReplayDivergenceInspector::ExportReport(
        const DivergenceReport& report,
        const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << FormatReport(report);
    return out.good();
}

// ---------------------------------------------------------------------------
// Severity query
// ---------------------------------------------------------------------------

bool ReplayDivergenceInspector::IsCritical(const DivergenceReport& report) {
    return report.severity == DivergenceSeverity::Critical;
}

// ---------------------------------------------------------------------------
// Instance history
// ---------------------------------------------------------------------------

void ReplayDivergenceInspector::RecordReport(const DivergenceReport& report) {
    m_lastReport = report;
    m_reportsHistory.push_back(report);
    if (m_reportsHistory.size() > m_maxReports) {
        m_reportsHistory.erase(m_reportsHistory.begin());
    }
}

const DivergenceReport& ReplayDivergenceInspector::LastReport() const {
    return m_lastReport;
}

const std::vector<DivergenceReport>&
ReplayDivergenceInspector::ReportHistory() const {
    return m_reportsHistory;
}

void ReplayDivergenceInspector::ClearHistory() {
    m_reportsHistory.clear();
    m_lastReport = DivergenceReport{};
}

// ---------------------------------------------------------------------------
// Replay Minimizer — binary search for smallest divergent window
// ---------------------------------------------------------------------------

ReplayDivergenceInspector::MinimizedResult
ReplayDivergenceInspector::MinimizeDivergence(
        const std::vector<ReplayFrame>& a,
        const std::vector<ReplayFrame>& b) {
    MinimizedResult result;

    size_t count = std::min(a.size(), b.size());
    if (count == 0) {
        return result;
    }

    // Quick full check: if everything matches, nothing to minimize
    bool anyMismatch = false;
    for (size_t i = 0; i < count; ++i) {
        if (a[i].stateHash != b[i].stateHash) {
            anyMismatch = true;
            break;
        }
    }
    if (!anyMismatch) {
        return result;
    }

    // Binary search for the first divergent tick
    size_t lo = 0;
    size_t hi = count;
    uint32_t iterations = 0;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        ++iterations;

        // Check if any divergence exists in [0, mid]
        bool divergedBefore = false;
        for (size_t i = 0; i <= mid && i < count; ++i) {
            if (a[i].stateHash != b[i].stateHash) {
                divergedBefore = true;
                break;
            }
        }

        if (divergedBefore) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    // lo is now the index of the first divergent tick
    if (lo >= count) {
        return result; // Should not happen given anyMismatch == true
    }
    result.firstDivergentTick = static_cast<int64_t>(a[lo].tick);
    result.lastMatchingTick = (lo > 0) ? static_cast<int64_t>(a[lo - 1].tick) : -1;
    result.windowSize = 1;
    result.iterationsUsed = iterations;
    result.minimized = true;
    return result;
}

ReplayDivergenceInspector::MinimizedResult
ReplayDivergenceInspector::MinimizeDivergenceFromHashers(
        const StateHasher& local,
        const StateHasher& remote) {
    MinimizedResult result;

    const auto& lh = local.History();
    const auto& rh = remote.History();
    size_t count = std::min(lh.size(), rh.size());
    if (count == 0) {
        return result;
    }

    // Binary search for the first divergent entry
    size_t lo = 0;
    size_t hi = count;
    uint32_t iterations = 0;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        ++iterations;

        bool divergedBefore = false;
        for (size_t i = 0; i <= mid && i < count; ++i) {
            if (lh[i].hash != rh[i].hash) {
                divergedBefore = true;
                break;
            }
        }

        if (divergedBefore) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    if (lo >= count) {
        return result; // Should not happen given pre-check
    }
    result.firstDivergentTick = static_cast<int64_t>(lh[lo].tick);
    result.lastMatchingTick = (lo > 0) ? static_cast<int64_t>(lh[lo - 1].tick) : -1;
    result.windowSize = 1;
    result.iterationsUsed = iterations;
    result.minimized = true;
    return result;
}

DetailedDivergenceReport ReplayDivergenceInspector::CompareDetailed(
        const StateHasher& local,
        const StateHasher& remote,
        const std::vector<std::pair<std::string, uint64_t>>& localSystemHashes,
        const std::vector<std::pair<std::string, uint64_t>>& remoteSystemHashes) {
    DetailedDivergenceReport result;
    result.baseReport = Compare(local, remote);

    // Build per-system diffs by pairing entries with the same name
    size_t count = std::min(localSystemHashes.size(), remoteSystemHashes.size());
    for (size_t i = 0; i < count; ++i) {
        SystemStateDiff diff;
        diff.systemName = localSystemHashes[i].first;
        diff.localHash = localSystemHashes[i].second;
        diff.remoteHash = remoteSystemHashes[i].second;
        diff.matches = (diff.localHash == diff.remoteHash);
        result.systemDiffs.push_back(std::move(diff));
    }

    // Handle any remaining entries from the longer list
    for (size_t i = count; i < localSystemHashes.size(); ++i) {
        SystemStateDiff diff;
        diff.systemName = localSystemHashes[i].first;
        diff.localHash = localSystemHashes[i].second;
        diff.remoteHash = 0;
        diff.matches = false;
        result.systemDiffs.push_back(std::move(diff));
    }
    for (size_t i = count; i < remoteSystemHashes.size(); ++i) {
        SystemStateDiff diff;
        diff.systemName = remoteSystemHashes[i].first;
        diff.localHash = 0;
        diff.remoteHash = remoteSystemHashes[i].second;
        diff.matches = false;
        result.systemDiffs.push_back(std::move(diff));
    }

    return result;
}

}  // namespace atlas::sim
