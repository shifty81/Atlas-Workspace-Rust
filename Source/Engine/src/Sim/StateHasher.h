#pragma once
// ============================================================
// Atlas State Hasher — Hash Ladder Verification
// ============================================================
//
// Implements the hash ladder:
//   H[n] = Hash(H[n-1] || State[n] || Inputs[n])
//
// Each tick produces a chained hash that depends on ALL prior
// history. Any divergence in state or inputs at any tick will
// cause all subsequent hashes to differ, making desync detection
// immediate and precise.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ATLAS_DETERMINISM_ENFORCEMENT.md

#include <cstdint>
#include <vector>
#include <string>

namespace atlas::sim {

/// A single entry in the hash ladder.
struct HashEntry {
    uint64_t tick = 0;
    uint64_t hash = 0;
};

/// Deterministic state hasher implementing a hash ladder.
///
/// Usage:
///   StateHasher hasher;
///   hasher.Reset(seed);
///   // Each tick:
///   hasher.AdvanceTick(tick, stateBytes, inputBytes);
///   uint64_t h = hasher.CurrentHash();
class StateHasher {
public:
    /// Reset the hash ladder with an initial seed.
    void Reset(uint64_t seed = 0);

    /// Advance the hash ladder by one tick.
    /// Computes H[n] = Hash(H[n-1] || state || inputs).
    void AdvanceTick(uint64_t tick,
                     const uint8_t* stateData, size_t stateSize,
                     const uint8_t* inputData, size_t inputSize);

    /// Convenience overload accepting vectors.
    void AdvanceTick(uint64_t tick,
                     const std::vector<uint8_t>& state,
                     const std::vector<uint8_t>& inputs);

    /// Current hash value.
    uint64_t CurrentHash() const;

    /// Current tick.
    uint64_t CurrentTick() const;

    /// Full history of hash entries (for replay comparison).
    const std::vector<HashEntry>& History() const;

    /// Verify this hasher's history against another.
    /// Returns the first tick where hashes diverge, or -1 if identical
    /// over the shared range.
    int64_t FindDivergence(const StateHasher& other) const;

    /// Deterministic hash combining function (FNV-1a based).
    static uint64_t HashCombine(uint64_t prev,
                                const uint8_t* data, size_t size);

private:
    uint64_t m_currentHash = 0;
    uint64_t m_currentTick = 0;
    std::vector<HashEntry> m_history;
};

}  // namespace atlas::sim
