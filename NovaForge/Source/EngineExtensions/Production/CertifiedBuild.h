#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace atlas::production {

enum class CertificationLevel : uint8_t {
    None,
    Basic,
    Standard,
    Full,
    AuditGrade
};

struct VerificationArtifact {
    std::string name;
    std::string type;
    uint64_t hash = 0;
    uint64_t timestamp = 0;
    bool passed = false;
    std::string details;
};

struct CertifiedBuildResult {
    CertificationLevel level = CertificationLevel::None;
    bool passed = false;
    std::vector<VerificationArtifact> artifacts;
    std::string summary;
    uint64_t timestamp = 0;
    std::string buildID;
};

/// A named verification step that produces a VerificationArtifact.
struct VerificationStep {
    std::string name;
    std::string type;
    std::function<VerificationArtifact()> execute;
};

class CertifiedBuildSystem {
public:
    void SetLevel(CertificationLevel level);
    CertificationLevel GetLevel() const;

    void AddArtifact(const VerificationArtifact& artifact);
    const VerificationArtifact* GetArtifact(const std::string& name) const;
    std::vector<VerificationArtifact> ListArtifacts() const;
    size_t ArtifactCount() const;

    CertifiedBuildResult RunVerification() const;
    std::string GenerateReport() const;
    std::string ExportJSON() const;

    void Clear();

    // --- Verification step registration ---

    /// Register a verification step that will be executed during RunAllSteps.
    void RegisterStep(const VerificationStep& step);

    /// Number of registered steps.
    size_t StepCount() const;

    /// Execute all registered steps, adding their artifacts.
    /// Returns the overall build result.
    CertifiedBuildResult RunAllSteps();

    // --- Built-in verification utilities ---

    /// Compute a hash of the given file contents and return an artifact.
    static VerificationArtifact VerifyFileHash(const std::string& filePath,
                                               uint64_t expectedHash);

    /// Create a test-result artifact from pass/fail counts.
    static VerificationArtifact VerifyTestResults(const std::string& name,
                                                  int passed, int failed);

    /// Create a contract-scan artifact from the scan result.
    static VerificationArtifact VerifyContractScan(bool scanPassed,
                                                   int violationCount);

private:
    CertificationLevel m_level = CertificationLevel::None;
    std::vector<VerificationArtifact> m_artifacts;
    std::vector<VerificationStep> m_steps;
};

} // namespace atlas::production
