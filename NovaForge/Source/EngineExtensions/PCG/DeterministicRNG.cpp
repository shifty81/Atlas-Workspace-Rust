#include "DeterministicRNG.h"

namespace atlas::procedural {

DeterministicRNG::DeterministicRNG(uint64_t seed)
    : m_state(seed == 0 ? 1 : seed) {}

void DeterministicRNG::Seed(uint64_t seed) {
    m_state = (seed == 0) ? 1 : seed;
}

uint64_t DeterministicRNG::Next() {
    // xorshift64* — fast, high-quality, deterministic.
    m_state ^= m_state >> 12;
    m_state ^= m_state << 25;
    m_state ^= m_state >> 27;
    return m_state * 2685821657736338717ULL;
}

uint32_t DeterministicRNG::NextU32(uint32_t max) {
    if (max == 0) return 0;
    return static_cast<uint32_t>(Next() % max);
}

float DeterministicRNG::NextFloat() {
    // Use upper 24 bits for uniform [0, 1) float.
    return static_cast<float>(Next() >> 40) / 16777216.0f;
}

float DeterministicRNG::NextFloatRange(float min, float max) {
    return min + NextFloat() * (max - min);
}

int32_t DeterministicRNG::NextIntRange(int32_t min, int32_t max) {
    if (min >= max) return min;
    uint32_t range = static_cast<uint32_t>(max - min + 1);
    return min + static_cast<int32_t>(NextU32(range));
}

bool DeterministicRNG::NextBool(float probability) {
    return NextFloat() < probability;
}

DeterministicRNG DeterministicRNG::Fork(uint64_t salt) const {
    // Mix the current state with salt to produce an independent stream.
    uint64_t mixed = m_state ^ (salt * 6364136223846793005ULL + 1442695040888963407ULL);
    return DeterministicRNG(mixed);
}

} // namespace atlas::procedural
