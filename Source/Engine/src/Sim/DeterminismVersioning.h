#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

struct DeterminismVersion {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
    uint64_t hash = 0;
    std::string description;
};

struct ForkInfo {
    std::string name;
    DeterminismVersion baseVersion;
    DeterminismVersion currentVersion;
    bool isCompatible = true;
};

class DeterminismVersionRegistry {
public:
    void SetCurrentVersion(const DeterminismVersion& version);
    DeterminismVersion GetCurrentVersion() const;

    void RegisterFork(const ForkInfo& info);
    void UnregisterFork(const std::string& name);
    const ForkInfo* GetFork(const std::string& name) const;
    std::vector<ForkInfo> ListForks() const;
    size_t ForkCount() const;

    bool CheckCompatibility(const std::string& forkName) const;
    std::vector<std::string> CheckAllCompatibility() const;

    std::string GenerateReport() const;
    void Clear();

private:
    DeterminismVersion m_currentVersion;
    std::vector<ForkInfo> m_forks;
};

} // namespace atlas::sim
