#include "CertifiedBuild.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cstdint>

namespace atlas::production {

void CertifiedBuildSystem::SetLevel(CertificationLevel level) {
    m_level = level;
}

CertificationLevel CertifiedBuildSystem::GetLevel() const {
    return m_level;
}

void CertifiedBuildSystem::AddArtifact(const VerificationArtifact& artifact) {
    m_artifacts.push_back(artifact);
}

const VerificationArtifact* CertifiedBuildSystem::GetArtifact(const std::string& name) const {
    for (auto& a : m_artifacts) {
        if (a.name == name) return &a;
    }
    return nullptr;
}

std::vector<VerificationArtifact> CertifiedBuildSystem::ListArtifacts() const {
    return m_artifacts;
}

size_t CertifiedBuildSystem::ArtifactCount() const {
    return m_artifacts.size();
}

CertifiedBuildResult CertifiedBuildSystem::RunVerification() const {
    CertifiedBuildResult result;
    result.level = m_level;
    result.artifacts = m_artifacts;
    result.passed = true;
    for (auto& a : m_artifacts) {
        if (!a.passed) {
            result.passed = false;
            break;
        }
    }
    result.summary = result.passed ? "All verifications passed" : "Some verifications failed";
    return result;
}

std::string CertifiedBuildSystem::GenerateReport() const {
    std::ostringstream ss;
    ss << "Certified Build Report\n";
    ss << "Level: " << static_cast<int>(m_level) << "\n";
    ss << "Artifacts: " << m_artifacts.size() << "\n";
    for (auto& a : m_artifacts) {
        ss << "  " << a.name << " [" << a.type << "] - "
           << (a.passed ? "PASS" : "FAIL") << "\n";
    }
    return ss.str();
}

std::string CertifiedBuildSystem::ExportJSON() const {
    std::ostringstream ss;
    ss << "{\"level\":" << static_cast<int>(m_level)
       << ",\"artifacts\":[";
    bool first = true;
    for (auto& a : m_artifacts) {
        if (!first) ss << ",";
        first = false;
        ss << "{\"name\":\"" << a.name << "\""
           << ",\"type\":\"" << a.type << "\""
           << ",\"hash\":" << a.hash
           << ",\"passed\":" << (a.passed ? "true" : "false")
           << "}";
    }
    ss << "]}";
    return ss.str();
}

void CertifiedBuildSystem::Clear() {
    m_level = CertificationLevel::None;
    m_artifacts.clear();
    m_steps.clear();
}

// --- Verification step registration ---

void CertifiedBuildSystem::RegisterStep(const VerificationStep& step) {
    m_steps.push_back(step);
}

size_t CertifiedBuildSystem::StepCount() const {
    return m_steps.size();
}

CertifiedBuildResult CertifiedBuildSystem::RunAllSteps() {
    for (auto& step : m_steps) {
        VerificationArtifact artifact = step.execute();
        if (artifact.name.empty()) artifact.name = step.name;
        if (artifact.type.empty()) artifact.type = step.type;
        m_artifacts.push_back(artifact);
    }
    return RunVerification();
}

// --- Built-in verification utilities ---

static uint64_t ComputeFileHash(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) return 0;

    uint64_t hash = 14695981039346656037ULL; // FNV-1a offset basis
    char buf[4096];
    while (in.read(buf, sizeof(buf)) || in.gcount() > 0) {
        auto count = in.gcount();
        for (std::streamsize i = 0; i < count; ++i) {
            hash ^= static_cast<uint64_t>(static_cast<uint8_t>(buf[i]));
            hash *= 1099511628211ULL; // FNV-1a prime
        }
    }
    return hash;
}

VerificationArtifact CertifiedBuildSystem::VerifyFileHash(
    const std::string& filePath, uint64_t expectedHash) {
    VerificationArtifact artifact;
    artifact.name = filePath;
    artifact.type = "file_hash";
    artifact.hash = ComputeFileHash(filePath);
    artifact.passed = (artifact.hash == expectedHash);
    artifact.details = artifact.passed
        ? "Hash matches expected value"
        : "Hash mismatch: expected " + std::to_string(expectedHash)
          + " got " + std::to_string(artifact.hash);
    return artifact;
}

VerificationArtifact CertifiedBuildSystem::VerifyTestResults(
    const std::string& name, int passed, int failed) {
    VerificationArtifact artifact;
    artifact.name = name;
    artifact.type = "test_results";
    artifact.passed = (failed == 0);
    artifact.details = std::to_string(passed) + " passed, "
                     + std::to_string(failed) + " failed";
    return artifact;
}

VerificationArtifact CertifiedBuildSystem::VerifyContractScan(
    bool scanPassed, int violationCount) {
    VerificationArtifact artifact;
    artifact.name = "contract_scan";
    artifact.type = "contract";
    artifact.passed = scanPassed;
    artifact.details = scanPassed
        ? "No contract violations"
        : std::to_string(violationCount) + " violation(s) detected";
    return artifact;
}

} // namespace atlas::production
