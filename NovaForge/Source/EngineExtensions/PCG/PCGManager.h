#pragma once
#include "PCGDomain.h"
#include <array>
#include <cstdint>
#include <string>

namespace atlas::procedural {

/// Central PCG seed authority.  Owns the universe seed and provides
/// isolated, deterministic RNG contexts for each of the 16 PCG
/// domains.  This is the single source of truth for all procedural
/// generation — no subsystem should create its own RNG.
///
/// Usage:
///   PCGManager mgr(universeSeed);
///   auto ctx = mgr.CreateContext(PCGDomain::Ship, SeedLevel::Object, objectId);
///   float hull = ctx.rng.NextFloat();
///
class PCGManager {
public:
    /// Construct with a universe seed.
    explicit PCGManager(uint64_t universeSeed = 42);

    /// Re-seed the entire PCG system.
    void SetUniverseSeed(uint64_t seed);

    /// Get the universe seed.
    uint64_t GetUniverseSeed() const { return m_universeSeed; }

    /// Create a scoped PCG context for the given domain and hierarchy
    /// level.  The `locationSalt` further differentiates the context
    /// (e.g. a system index or entity ID).
    PCGContext CreateContext(PCGDomain domain,
                            SeedLevel level = SeedLevel::Universe,
                            uint64_t locationSalt = 0) const;

    /// Get the domain root seed (before any hierarchy scoping).
    uint64_t GetDomainSeed(PCGDomain domain) const;

    /// Get the version tag.  Bumping this invalidates all cached PCG
    /// output, ensuring clients regenerate content after rule changes.
    uint32_t GetVersion() const { return m_version; }
    void SetVersion(uint32_t v) { m_version = v; }

    /// Number of domains.
    static constexpr int DomainCount() {
        return static_cast<int>(PCGDomain::COUNT);
    }

private:
    uint64_t m_universeSeed = 42;
    uint32_t m_version = 1;

    /// Per-domain root seeds derived from the universe seed.
    std::array<uint64_t, static_cast<size_t>(PCGDomain::COUNT)> m_domainSeeds{};

    void DeriveAllDomainSeeds();
};

} // namespace atlas::procedural
