#pragma once
#include "PCGDomain.h"
#include <cstdint>
#include <vector>

namespace atlas::procedural {

/// Record of a single PCG verification check.  The server generates
/// content with the same seed and compares the resulting hash against
/// the client's reported hash.
struct PCGVerifyRecord {
    PCGDomain domain    = PCGDomain::Universe;
    uint64_t  seed      = 0;
    uint64_t  expected  = 0;  // hash expected by the authority
    uint64_t  actual    = 0;  // hash reported by the client
    bool      passed    = false;
};

/// Result of a batch verification run.
struct PCGVerifyResult {
    bool allPassed = true;
    std::vector<PCGVerifyRecord> records;

    size_t PassedCount() const {
        size_t n = 0;
        for (auto& r : records) if (r.passed) ++n;
        return n;
    }
    size_t FailedCount() const { return records.size() - PassedCount(); }
};

/// Server-authoritative PCG verifier.  Compares client-generated PCG
/// hashes against server-generated reference hashes to detect
/// desynchronisation or tampering.
///
/// Workflow:
///   1. Server generates content, stores hashes via AddExpected().
///   2. Client submits its hashes via Submit().
///   3. Call Verify() to compare all submitted pairs.
///
class PCGVerifier {
public:
    PCGVerifier() = default;

    /// Register an authoritative (server-side) hash for a domain/seed.
    void AddExpected(PCGDomain domain, uint64_t seed, uint64_t hash);

    /// Submit a client-reported hash for verification.
    void Submit(PCGDomain domain, uint64_t seed, uint64_t hash);

    /// Run verification.  Returns a result with per-record pass/fail.
    PCGVerifyResult Verify() const;

    /// Clear all expected and submitted records.
    void Clear();

    /// Number of expected records registered.
    size_t ExpectedCount() const { return m_expected.size(); }

    /// Number of submitted records registered.
    size_t SubmittedCount() const { return m_submitted.size(); }

    /// Compute a deterministic hash from a data buffer.  Utility
    /// function that subsystems can use to hash their PCG output.
    static uint64_t HashData(const void* data, size_t bytes);

private:
    struct Entry {
        PCGDomain domain;
        uint64_t  seed;
        uint64_t  hash;
    };
    std::vector<Entry> m_expected;
    std::vector<Entry> m_submitted;
};

} // namespace atlas::procedural
