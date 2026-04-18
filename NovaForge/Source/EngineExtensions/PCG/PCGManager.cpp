#include "PCGManager.h"

namespace atlas::procedural {

PCGManager::PCGManager(uint64_t universeSeed)
    : m_universeSeed(universeSeed) {
    DeriveAllDomainSeeds();
}

void PCGManager::SetUniverseSeed(uint64_t seed) {
    m_universeSeed = seed;
    DeriveAllDomainSeeds();
}

void PCGManager::DeriveAllDomainSeeds() {
    // Each domain gets a unique seed derived from the universe seed by
    // mixing with the domain index.  This ensures complete isolation:
    // changes to one domain's generator never affect another.
    DeterministicRNG root(m_universeSeed);
    for (size_t i = 0; i < static_cast<size_t>(PCGDomain::COUNT); ++i) {
        DeterministicRNG domainRng = root.Fork(i + 1);
        m_domainSeeds[i] = domainRng.Next();
    }
}

uint64_t PCGManager::GetDomainSeed(PCGDomain domain) const {
    auto idx = static_cast<size_t>(domain);
    if (idx >= m_domainSeeds.size()) return 0;
    return m_domainSeeds[idx];
}

PCGContext PCGManager::CreateContext(PCGDomain domain,
                                    SeedLevel level,
                                    uint64_t locationSalt) const {
    uint64_t baseSeed = GetDomainSeed(domain);

    // Walk down the seed hierarchy from Universe to the requested level.
    PCGContext ctx(domain, SeedLevel::Universe, baseSeed);
    for (uint8_t l = 0; l < static_cast<uint8_t>(level); ++l) {
        ctx = ctx.Child(locationSalt + l);
    }
    return ctx;
}

} // namespace atlas::procedural
