#pragma once
#include "DeterministicRNG.h"
#include <cstdint>
#include <string>
#include <array>

namespace atlas::procedural {

/// Enumeration of the 16 isolated PCG domains.  Each domain has its
/// own RNG stream derived from the universe seed so that changes in
/// one domain never affect another.
enum class PCGDomain : uint8_t {
    Universe   = 0,
    Galaxy     = 1,
    System     = 2,
    Sector     = 3,
    Planet     = 4,
    Asteroid   = 5,
    Ship       = 6,
    Station    = 7,
    NPC        = 8,
    Fleet      = 9,
    Loot       = 10,
    Mission    = 11,
    Anomaly    = 12,
    Economy    = 13,
    Weather    = 14,
    Terrain    = 15,
    COUNT      = 16
};

/// Human-readable name for a PCG domain.
inline const char* PCGDomainName(PCGDomain d) {
    static const char* names[] = {
        "Universe", "Galaxy", "System", "Sector",
        "Planet",   "Asteroid", "Ship", "Station",
        "NPC",      "Fleet", "Loot", "Mission",
        "Anomaly",  "Economy", "Weather", "Terrain"
    };
    auto idx = static_cast<uint8_t>(d);
    return (idx < 16) ? names[idx] : "Unknown";
}

/// Seed hierarchy level.  Seeds cascade downward:
/// Universe → Galaxy → System → Sector → Object.
enum class SeedLevel : uint8_t {
    Universe = 0,
    Galaxy   = 1,
    System   = 2,
    Sector   = 3,
    Object   = 4,
    COUNT    = 5
};

/// A scoped PCG context tied to a specific domain and hierarchy level.
/// Provides a deterministic RNG stream that is completely isolated from
/// all other domain/level combinations.
struct PCGContext {
    PCGDomain       domain = PCGDomain::Universe;
    SeedLevel       level  = SeedLevel::Universe;
    uint64_t        seed   = 0;
    DeterministicRNG rng;

    PCGContext() = default;
    PCGContext(PCGDomain d, SeedLevel l, uint64_t s)
        : domain(d), level(l), seed(s), rng(s) {}

    /// Create a child context at the next hierarchy level by mixing
    /// the current seed with a location-specific salt.
    PCGContext Child(uint64_t locationSalt) const {
        uint64_t childSeed = seed ^ (locationSalt * 6364136223846793005ULL + 1);
        SeedLevel childLevel = (static_cast<uint8_t>(level) + 1 <
                                static_cast<uint8_t>(SeedLevel::COUNT))
                               ? static_cast<SeedLevel>(static_cast<uint8_t>(level) + 1)
                               : level;
        return PCGContext(domain, childLevel, childSeed);
    }
};

} // namespace atlas::procedural
