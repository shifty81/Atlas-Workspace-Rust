#include "FPDriftDetector.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>

namespace atlas::sim {

// ---------------------------------------------------------------------------
// Platform identification
// ---------------------------------------------------------------------------

void FPDriftDetector::SetPlatformNames(const std::string& nameA,
                                       const std::string& nameB) {
    m_platformAName = nameA;
    m_platformBName = nameB;
}

// ---------------------------------------------------------------------------
// Core comparison
// ---------------------------------------------------------------------------

FPDriftReport FPDriftDetector::RunComparison(
        const StateHasher& platformA,
        const StateHasher& platformB) {

    const auto& histA = platformA.History();
    const auto& histB = platformB.History();

    FPDriftReport report;
    report.platformAName      = m_platformAName;
    report.platformBName      = m_platformBName;
    report.totalTicksCompared = static_cast<uint32_t>(
        std::min(histA.size(), histB.size()));

    for (uint32_t i = 0; i < report.totalTicksCompared; ++i) {
        if (histA[i].hash != histB[i].hash) {
            report.hasDrift            = true;
            report.firstDivergenceTick = static_cast<int64_t>(histA[i].tick);
            report.platformAHash       = histA[i].hash;
            report.platformBHash       = histB[i].hash;
            report.matchingTicks       = i;
            report.systemDrifts        = m_pendingSystemDrifts;
            return report;
        }
    }

    // No drift
    report.matchingTicks = report.totalTicksCompared;
    report.systemDrifts  = m_pendingSystemDrifts;
    return report;
}

// ---------------------------------------------------------------------------
// Per-system analysis
// ---------------------------------------------------------------------------

void FPDriftDetector::AddSystemHashes(
        const std::string& systemName,
        const std::vector<std::pair<std::string, uint64_t>>& perSystemA,
        const std::vector<std::pair<std::string, uint64_t>>& perSystemB) {

    auto drifts = CompareSystemHashes(perSystemA, perSystemB);
    // Tag each drift entry with the category if it has no name yet
    for (auto& d : drifts) {
        if (d.systemName.empty()) {
            d.systemName = systemName;
        }
    }
    m_pendingSystemDrifts.insert(m_pendingSystemDrifts.end(),
                                 drifts.begin(), drifts.end());
}

std::vector<FPSystemDrift> FPDriftDetector::CompareSystemHashes(
        const std::vector<std::pair<std::string, uint64_t>>& localSys,
        const std::vector<std::pair<std::string, uint64_t>>& remoteSys) {

    std::vector<FPSystemDrift> result;

    size_t count = std::min(localSys.size(), remoteSys.size());
    for (size_t i = 0; i < count; ++i) {
        FPSystemDrift drift;
        drift.systemName = localSys[i].first;
        drift.hashA      = localSys[i].second;
        drift.hashB      = remoteSys[i].second;
        drift.matches    = (drift.hashA == drift.hashB);
        result.push_back(std::move(drift));
    }

    // Handle remaining entries from the longer list
    for (size_t i = count; i < localSys.size(); ++i) {
        FPSystemDrift drift;
        drift.systemName = localSys[i].first;
        drift.hashA      = localSys[i].second;
        drift.hashB      = 0;
        drift.matches    = false;
        result.push_back(std::move(drift));
    }
    for (size_t i = count; i < remoteSys.size(); ++i) {
        FPSystemDrift drift;
        drift.systemName = remoteSys[i].first;
        drift.hashA      = 0;
        drift.hashB      = remoteSys[i].second;
        drift.matches    = false;
        result.push_back(std::move(drift));
    }

    return result;
}

// ---------------------------------------------------------------------------
// Report formatting
// ---------------------------------------------------------------------------

std::string FPDriftDetector::FormatReport(const FPDriftReport& report) {
    std::ostringstream os;
    os << "=== FP Drift Report ===" << "\n";
    os << "Platform A: " << report.platformAName << "\n";
    os << "Platform B: " << report.platformBName << "\n";

    if (!report.hasDrift) {
        os << "Status    : OK — no drift detected\n";
    } else {
        os << "Status    : DRIFT DETECTED\n";
        os << "Tick      : " << report.firstDivergenceTick << "\n";
        os << "Hash A    : 0x" << std::hex << std::setw(16)
           << std::setfill('0') << report.platformAHash << "\n";
        os << "Hash B    : 0x" << std::hex << std::setw(16)
           << std::setfill('0') << report.platformBHash << "\n";
        os << std::dec;
    }

    os << "Compared  : " << report.totalTicksCompared << " ticks\n";
    os << "Matched   : " << report.matchingTicks << " ticks\n";

    if (!report.systemDrifts.empty()) {
        os << "\n--- Per-System Drift ---\n";
        for (const auto& sd : report.systemDrifts) {
            os << "  " << sd.systemName << ": ";
            if (sd.matches) {
                os << "OK\n";
            } else {
                os << "DIVERGED  A=0x" << std::hex << std::setw(16)
                   << std::setfill('0') << sd.hashA
                   << "  B=0x" << std::setw(16)
                   << std::setfill('0') << sd.hashB
                   << std::dec << "\n";
            }
        }
    }

    return os.str();
}

// ---------------------------------------------------------------------------
// Export report to file
// ---------------------------------------------------------------------------

bool FPDriftDetector::ExportReport(const FPDriftReport& report,
                                   const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << FormatReport(report);
    return out.good();
}

}  // namespace atlas::sim
