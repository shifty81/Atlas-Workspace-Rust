#include "ReplayProofExporter.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace atlas::sim {

void ReplayProofExporter::AddEntry(uint64_t tick, uint64_t stateHash,
                                   uint64_t prevHash, uint64_t inputHash,
                                   bool isSavePoint) {
    ProofTraceEntry entry;
    entry.tick = tick;
    entry.stateHash = stateHash;
    entry.prevHash = prevHash;
    entry.inputHash = inputHash;
    entry.isSavePoint = isSavePoint;

    if (m_trace.entries.empty()) {
        m_trace.initialHash = stateHash;
    }
    m_trace.finalHash = stateHash;
    m_trace.entries.push_back(entry);
    m_trace.tickCount = static_cast<int>(m_trace.entries.size());
    if (isSavePoint) {
        ++m_trace.savePointCount;
    }
}

void ReplayProofExporter::SetSpecName(const std::string& name) {
    m_trace.specName = name;
}

const std::string& ReplayProofExporter::GetSpecName() const {
    return m_trace.specName;
}

const ProofTrace& ReplayProofExporter::GetTrace() const {
    return m_trace;
}

size_t ReplayProofExporter::EntryCount() const {
    return m_trace.entries.size();
}

void ReplayProofExporter::Clear() {
    m_trace = ProofTrace{};
}

bool ReplayProofExporter::ValidateChain() const {
    if (m_trace.entries.size() < 2) return true;
    for (size_t i = 1; i < m_trace.entries.size(); ++i) {
        if (m_trace.entries[i].prevHash != m_trace.entries[i - 1].stateHash) {
            return false;
        }
    }
    return true;
}

ProofExportResult ReplayProofExporter::ExportTLA(const std::string& outputPath) const {
    ProofExportResult result;
    result.format = ProofFormat::TLATrace;
    result.outputPath = outputPath;

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        result.errorMessage = "Failed to open file: " + outputPath;
        return result;
    }

    const auto& specName = m_trace.specName.empty() ? "replay" : m_trace.specName;

    out << "---- MODULE ReplayTrace ----\n";
    out << "EXTENDS Integers, Sequences\n";
    out << "\n";
    out << "\\* Trace exported from Atlas replay\n";
    out << "\\* Spec: " << specName << "\n";
    out << "\\* Ticks: " << m_trace.tickCount << "\n";
    out << "\\* Save points: " << m_trace.savePointCount << "\n";
    out << "\n";
    out << "TraceLength == " << m_trace.tickCount << "\n";
    out << "InitialHash == " << m_trace.initialHash << "\n";
    out << "FinalHash == " << m_trace.finalHash << "\n";
    out << "\n";

    out << "Trace == <<\n";
    for (size_t i = 0; i < m_trace.entries.size(); ++i) {
        const auto& e = m_trace.entries[i];
        out << "    [tick |-> " << e.tick
            << ", stateHash |-> " << e.stateHash
            << ", prevHash |-> " << e.prevHash
            << ", inputHash |-> " << e.inputHash
            << ", savePoint |-> " << (e.isSavePoint ? "TRUE" : "FALSE")
            << "]";
        if (i + 1 < m_trace.entries.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << ">>\n";
    out << "\n";
    out << "\\* Chain integrity check: each entry's prevHash matches previous stateHash\n";
    out << "ChainValid == \\A i \\in 2..Len(Trace) :\n";
    out << "    Trace[i].prevHash = Trace[i-1].stateHash\n";
    out << "\n";
    out << "====\n";

    result.success = true;
    result.entriesExported = static_cast<int>(m_trace.entries.size());
    return result;
}

ProofExportResult ReplayProofExporter::ExportJSON(const std::string& outputPath) const {
    ProofExportResult result;
    result.format = ProofFormat::JSONTrace;
    result.outputPath = outputPath;

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        result.errorMessage = "Failed to open file: " + outputPath;
        return result;
    }

    out << "[\n";
    for (size_t i = 0; i < m_trace.entries.size(); ++i) {
        const auto& e = m_trace.entries[i];
        out << "  {\n";
        out << "    \"tick\": " << e.tick << ",\n";
        out << "    \"stateHash\": " << e.stateHash << ",\n";
        out << "    \"prevHash\": " << e.prevHash << ",\n";
        out << "    \"inputHash\": " << e.inputHash << ",\n";
        out << "    \"isSavePoint\": " << (e.isSavePoint ? "true" : "false") << "\n";
        out << "  }";
        if (i + 1 < m_trace.entries.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";

    result.success = true;
    result.entriesExported = static_cast<int>(m_trace.entries.size());
    return result;
}

ProofExportResult ReplayProofExporter::ExportCSV(const std::string& outputPath) const {
    ProofExportResult result;
    result.format = ProofFormat::CSVTrace;
    result.outputPath = outputPath;

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        result.errorMessage = "Failed to open file: " + outputPath;
        return result;
    }

    out << "tick,stateHash,prevHash,inputHash,isSavePoint\n";
    for (const auto& e : m_trace.entries) {
        out << e.tick << ","
            << e.stateHash << ","
            << e.prevHash << ","
            << e.inputHash << ","
            << (e.isSavePoint ? "true" : "false") << "\n";
    }

    result.success = true;
    result.entriesExported = static_cast<int>(m_trace.entries.size());
    return result;
}

ProofExportResult ReplayProofExporter::Export(const std::string& outputPath,
                                              ProofFormat format) const {
    switch (format) {
        case ProofFormat::TLATrace:  return ExportTLA(outputPath);
        case ProofFormat::JSONTrace: return ExportJSON(outputPath);
        case ProofFormat::CSVTrace:  return ExportCSV(outputPath);
        default: break;
    }
    ProofExportResult result;
    result.errorMessage = "Unknown export format";
    return result;
}

std::string ReplayProofExporter::FormatEntry(const ProofTraceEntry& entry) {
    std::ostringstream ss;
    ss << "tick=" << entry.tick
       << " stateHash=" << entry.stateHash
       << " prevHash=" << entry.prevHash
       << " inputHash=" << entry.inputHash
       << " savePoint=" << (entry.isSavePoint ? "true" : "false");
    return ss.str();
}

std::vector<uint64_t> ReplayProofExporter::SavePointTicks() const {
    std::vector<uint64_t> ticks;
    for (const auto& e : m_trace.entries) {
        if (e.isSavePoint) {
            ticks.push_back(e.tick);
        }
    }
    return ticks;
}

} // namespace atlas::sim
