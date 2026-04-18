#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::sim {

struct ProofTraceEntry {
    uint64_t tick = 0;
    uint64_t stateHash = 0;
    uint64_t prevHash = 0;
    uint64_t inputHash = 0;
    bool isSavePoint = false;
};

struct ProofTrace {
    std::string specName;
    std::vector<ProofTraceEntry> entries;
    uint64_t initialHash = 0;
    uint64_t finalHash = 0;
    int tickCount = 0;
    int savePointCount = 0;
};

enum class ProofFormat {
    TLATrace,
    JSONTrace,
    CSVTrace
};

struct ProofExportResult {
    bool success = false;
    std::string outputPath;
    ProofFormat format = ProofFormat::TLATrace;
    int entriesExported = 0;
    std::string errorMessage;
};

class ReplayProofExporter {
public:
    void AddEntry(uint64_t tick, uint64_t stateHash, uint64_t prevHash,
                  uint64_t inputHash, bool isSavePoint = false);

    void SetSpecName(const std::string& name);
    const std::string& GetSpecName() const;

    const ProofTrace& GetTrace() const;
    size_t EntryCount() const;
    void Clear();

    /// Validate that chained hashes are consistent: each entry's prevHash
    /// matches the previous entry's stateHash.
    bool ValidateChain() const;

    ProofExportResult ExportTLA(const std::string& outputPath) const;
    ProofExportResult ExportJSON(const std::string& outputPath) const;
    ProofExportResult ExportCSV(const std::string& outputPath) const;
    ProofExportResult Export(const std::string& outputPath, ProofFormat format) const;

    static std::string FormatEntry(const ProofTraceEntry& entry);

    std::vector<uint64_t> SavePointTicks() const;

private:
    ProofTrace m_trace;
};

} // namespace atlas::sim
