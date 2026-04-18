#include "StateHasher.h"

namespace atlas::sim {

// FNV-1a constants for 64-bit
static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
static constexpr uint64_t FNV_PRIME  = 1099511628211ULL;

uint64_t StateHasher::HashCombine(uint64_t prev,
                                  const uint8_t* data, size_t size) {
    uint64_t h = prev;
    for (size_t i = 0; i < size; ++i) {
        h ^= static_cast<uint64_t>(data[i]);
        h *= FNV_PRIME;
    }
    return h;
}

void StateHasher::Reset(uint64_t seed) {
    m_currentHash = FNV_OFFSET ^ seed;
    m_currentTick = 0;
    m_history.clear();
}

void StateHasher::AdvanceTick(uint64_t tick,
                              const uint8_t* stateData, size_t stateSize,
                              const uint8_t* inputData, size_t inputSize) {
    // H[n] = Hash(H[n-1] || state || inputs)
    uint64_t h = m_currentHash;

    // Mix in the tick number itself for extra safety
    auto tickBytes = reinterpret_cast<const uint8_t*>(&tick);
    h = HashCombine(h, tickBytes, sizeof(tick));

    // Mix in state
    if (stateData && stateSize > 0) {
        h = HashCombine(h, stateData, stateSize);
    }

    // Mix in inputs
    if (inputData && inputSize > 0) {
        h = HashCombine(h, inputData, inputSize);
    }

    m_currentHash = h;
    m_currentTick = tick;

    HashEntry entry;
    entry.tick = tick;
    entry.hash = h;
    m_history.push_back(entry);
}

void StateHasher::AdvanceTick(uint64_t tick,
                              const std::vector<uint8_t>& state,
                              const std::vector<uint8_t>& inputs) {
    AdvanceTick(tick,
                state.data(), state.size(),
                inputs.data(), inputs.size());
}

uint64_t StateHasher::CurrentHash() const {
    return m_currentHash;
}

uint64_t StateHasher::CurrentTick() const {
    return m_currentTick;
}

const std::vector<HashEntry>& StateHasher::History() const {
    return m_history;
}

int64_t StateHasher::FindDivergence(const StateHasher& other) const {
    size_t minLen = m_history.size();
    if (other.m_history.size() < minLen) {
        minLen = other.m_history.size();
    }

    for (size_t i = 0; i < minLen; ++i) {
        if (m_history[i].hash != other.m_history[i].hash) {
            return static_cast<int64_t>(m_history[i].tick);
        }
    }

    return -1;  // No divergence in shared range
}

}  // namespace atlas::sim
