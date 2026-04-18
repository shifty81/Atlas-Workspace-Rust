#pragma once
// NovaForge::PCGDeterministicSeedContext — reproducible seed management.
//
// Manages a hierarchy of deterministic seeds for PCG generation.
// The context binds a universe seed with named domain seeds derived from it.
// Child seeds are derived by XOR-folding the parent state with a domain hash
// so each domain always produces the same sequence from the same universe seed.
//
// Phase E.1 — Shared NovaForge PCG Core

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace NovaForge {

// ── PCGDeterministicSeedContext ───────────────────────────────────────────────

class PCGDeterministicSeedContext {
public:
    static constexpr uint64_t kDefaultUniverseSeed = 42424242ULL;

    explicit PCGDeterministicSeedContext(uint64_t universeSeed = kDefaultUniverseSeed)
        : m_universeSeed(universeSeed) {}

    // ── Universe seed ─────────────────────────────────────────────────────

    [[nodiscard]] uint64_t universeSeed() const { return m_universeSeed; }

    /// Replace universe seed; clears all cached domain seeds.
    void setUniverseSeed(uint64_t seed) {
        m_universeSeed = (seed == 0) ? 1 : seed;
        m_domainSeeds.clear();
    }

    // ── Domain seed derivation ────────────────────────────────────────────
    // Domain seeds are deterministically derived from the universe seed + domain name.
    // The same universe seed always produces the same domain seed for a given name.

    [[nodiscard]] uint64_t seedForDomain(const std::string& domain) const {
        auto it = m_domainSeeds.find(domain);
        if (it != m_domainSeeds.end()) return it->second;
        uint64_t s = deriveSeed(m_universeSeed, domain);
        m_domainSeeds[domain] = s;
        return s;
    }

    // ── Scoped child context ──────────────────────────────────────────────
    // Derive a child context for a named sub-domain (e.g., object instance).

    [[nodiscard]] PCGDeterministicSeedContext childContext(const std::string& childName) const {
        return PCGDeterministicSeedContext(deriveSeed(m_universeSeed, childName));
    }

    // ── Named pinned seeds ────────────────────────────────────────────────
    // Allow callers to pin specific domain seeds for debugging / reproduction.

    void pinDomainSeed(const std::string& domain, uint64_t seed) {
        m_domainSeeds[domain] = seed;
    }

    bool hasPinnedSeed(const std::string& domain) const {
        return m_domainSeeds.find(domain) != m_domainSeeds.end();
    }

    void clearPinnedSeeds() { m_domainSeeds.clear(); }

    // ── Registered domains ────────────────────────────────────────────────
    // Callers can register domains up-front so the context can enumerate them.

    void registerDomain(const std::string& domain) {
        if (!hasDomain(domain)) m_registeredDomains.push_back(domain);
    }

    [[nodiscard]] bool hasDomain(const std::string& domain) const {
        for (const auto& d : m_registeredDomains)
            if (d == domain) return true;
        return false;
    }

    [[nodiscard]] const std::vector<std::string>& registeredDomains() const {
        return m_registeredDomains;
    }

private:
    uint64_t                              m_universeSeed;
    mutable std::unordered_map<std::string, uint64_t> m_domainSeeds;
    std::vector<std::string>              m_registeredDomains;

    /// Deterministic hash of universeSeed + domain name string.
    /// Uses xorshift64* style mixing without any stdlib dependency.
    static uint64_t deriveSeed(uint64_t base, const std::string& name) {
        // FNV-1a hash of name bytes into a 64-bit salt
        uint64_t salt = 14695981039346656037ULL;
        for (unsigned char c : name) {
            salt ^= static_cast<uint64_t>(c);
            salt *= 1099511628211ULL;
        }
        // Mix base and salt with xorshift
        uint64_t s = base ^ salt;
        if (s == 0) s = 1;
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        s *= 2685821657736338717ULL;
        return s != 0 ? s : 1;
    }
};

} // namespace NovaForge
