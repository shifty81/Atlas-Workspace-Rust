#include "PCGVerify.h"
#include <algorithm>

namespace atlas::procedural {

void PCGVerifier::AddExpected(PCGDomain domain, uint64_t seed, uint64_t hash) {
    m_expected.push_back({domain, seed, hash});
}

void PCGVerifier::Submit(PCGDomain domain, uint64_t seed, uint64_t hash) {
    m_submitted.push_back({domain, seed, hash});
}

PCGVerifyResult PCGVerifier::Verify() const {
    PCGVerifyResult result;
    result.allPassed = true;

    for (auto& exp : m_expected) {
        PCGVerifyRecord rec;
        rec.domain   = exp.domain;
        rec.seed     = exp.seed;
        rec.expected = exp.hash;
        rec.actual   = 0;
        rec.passed   = false;

        // Find matching submission.
        for (auto& sub : m_submitted) {
            if (sub.domain == exp.domain && sub.seed == exp.seed) {
                rec.actual = sub.hash;
                rec.passed = (sub.hash == exp.hash);
                break;
            }
        }

        if (!rec.passed) result.allPassed = false;
        result.records.push_back(rec);
    }

    return result;
}

void PCGVerifier::Clear() {
    m_expected.clear();
    m_submitted.clear();
}

uint64_t PCGVerifier::HashData(const void* data, size_t bytes) {
    // FNV-1a 64-bit hash — simple, deterministic, no platform variance.
    uint64_t hash = 14695981039346656037ULL;
    auto ptr = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < bytes; ++i) {
        hash ^= static_cast<uint64_t>(ptr[i]);
        hash *= 1099511628211ULL;
    }
    return hash;
}

} // namespace atlas::procedural
