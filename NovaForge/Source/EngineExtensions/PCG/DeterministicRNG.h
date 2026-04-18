#pragma once
#include <cstdint>

namespace atlas::procedural {

/// Deterministic pseudo-random number generator based on xorshift64*.
/// This generator is designed for procedural content generation where
/// reproducibility is critical.  Given the same seed, the exact same
/// sequence of numbers is produced on every platform.
///
/// Properties:
///   - Period: 2^64 - 1
///   - No floating-point arithmetic in state transitions
///   - No standard library dependencies (no std::rand, no <random>)
class DeterministicRNG {
public:
    /// Construct with a specific seed.  Seed 0 is remapped to 1
    /// because xorshift requires a non-zero state.
    explicit DeterministicRNG(uint64_t seed = 1);

    /// Re-seed the generator.
    void Seed(uint64_t seed);

    /// Return the current seed / state.
    uint64_t GetSeed() const { return m_state; }

    /// Generate next 64-bit unsigned integer.
    uint64_t Next();

    /// Generate a 32-bit unsigned integer in [0, max).
    uint32_t NextU32(uint32_t max);

    /// Generate a float in [0, 1).
    float NextFloat();

    /// Generate a float in [min, max).
    float NextFloatRange(float min, float max);

    /// Generate an integer in [min, max] (inclusive).
    int32_t NextIntRange(int32_t min, int32_t max);

    /// Generate a boolean with the given probability of being true.
    bool NextBool(float probability = 0.5f);

    /// Create a child RNG by combining the current state with a
    /// domain-specific salt.  This lets different subsystems derive
    /// independent streams from the same parent seed.
    DeterministicRNG Fork(uint64_t salt) const;

private:
    uint64_t m_state;
};

} // namespace atlas::procedural
