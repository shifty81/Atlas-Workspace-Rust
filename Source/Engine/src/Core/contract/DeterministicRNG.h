#pragma once
// ============================================================
// Atlas Deterministic RNG
// ============================================================
//
// A fast, portable, deterministic pseudo-random number generator
// for use in simulation code. Uses xorshift64* algorithm.
//
// All simulation randomness MUST use this RNG (or equivalent
// deterministic source). OS-level rand()/random/<random> are
// forbidden in simulation code.
//
// See: docs/ATLAS_CORE_CONTRACT.md

#include <cstdint>

namespace atlas {

struct DeterministicRNG {
    uint64_t state;

    explicit DeterministicRNG(uint64_t seed) : state(seed ? seed : 1) {}

    uint32_t Next() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return static_cast<uint32_t>(state * 2685821657736338717ULL);
    }

    /// Returns a value in [0, max) deterministically.
    uint32_t NextInRange(uint32_t max) {
        if (max == 0) return 0;
        return Next() % max;
    }
};

}  // namespace atlas
