#include "DeterminismVersioning.h"
#include <sstream>
#include <algorithm>

namespace atlas::sim {

void DeterminismVersionRegistry::SetCurrentVersion(const DeterminismVersion& version) {
    m_currentVersion = version;
}

DeterminismVersion DeterminismVersionRegistry::GetCurrentVersion() const {
    return m_currentVersion;
}

void DeterminismVersionRegistry::RegisterFork(const ForkInfo& info) {
    m_forks.push_back(info);
}

void DeterminismVersionRegistry::UnregisterFork(const std::string& name) {
    m_forks.erase(
        std::remove_if(m_forks.begin(), m_forks.end(),
                       [&name](const ForkInfo& f) { return f.name == name; }),
        m_forks.end());
}

const ForkInfo* DeterminismVersionRegistry::GetFork(const std::string& name) const {
    for (auto& fork : m_forks) {
        if (fork.name == name) return &fork;
    }
    return nullptr;
}

std::vector<ForkInfo> DeterminismVersionRegistry::ListForks() const {
    return m_forks;
}

size_t DeterminismVersionRegistry::ForkCount() const {
    return m_forks.size();
}

bool DeterminismVersionRegistry::CheckCompatibility(const std::string& forkName) const {
    const ForkInfo* fork = GetFork(forkName);
    if (!fork) return false;
    return fork->isCompatible &&
           fork->baseVersion.major == m_currentVersion.major &&
           fork->baseVersion.minor == m_currentVersion.minor;
}

std::vector<std::string> DeterminismVersionRegistry::CheckAllCompatibility() const {
    std::vector<std::string> incompatible;
    for (auto& fork : m_forks) {
        if (!fork.isCompatible ||
            fork.baseVersion.major != m_currentVersion.major ||
            fork.baseVersion.minor != m_currentVersion.minor) {
            incompatible.push_back(fork.name);
        }
    }
    return incompatible;
}

std::string DeterminismVersionRegistry::GenerateReport() const {
    std::ostringstream ss;
    ss << "Determinism Version Report\n";
    ss << "Current: " << m_currentVersion.major << "."
       << m_currentVersion.minor << "."
       << m_currentVersion.patch << "\n";
    ss << "Forks: " << m_forks.size() << "\n";
    for (auto& fork : m_forks) {
        ss << "  " << fork.name << " - "
           << (CheckCompatibility(fork.name) ? "compatible" : "incompatible")
           << "\n";
    }
    return ss.str();
}

void DeterminismVersionRegistry::Clear() {
    m_currentVersion = DeterminismVersion{};
    m_forks.clear();
}

} // namespace atlas::sim
