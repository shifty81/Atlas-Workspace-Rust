#pragma once
// ============================================================
// Atlas AI — Memory System
// ============================================================
//
// DETERMINISM CONTRACT:
//   AIMemory stores key-value facts for AI decision making.
//   All entries are deterministic and serializable. Memory
//   contents participate in state hashing and must be
//   identical across client/server for the same simulation
//   tick. See BehaviorGraph.h for the full AI determinism
//   contract.
// ============================================================
#include <string>
#include <unordered_map>
#include <cstdint>

namespace atlas::ai {

struct MemoryEntry {
    std::string key;
    std::string value;
    float strength = 1.0f;
    float decayRate = 0.0f;  // Strength lost per tick (0 = permanent)
    uint64_t createdTick = 0;
};

class AIMemory {
public:
    void Store(const std::string& key, const std::string& value,
               float strength, float decayRate, uint64_t tick);

    const MemoryEntry* Recall(const std::string& key) const;

    bool Has(const std::string& key) const;
    void Forget(const std::string& key);
    void Clear();

    // Decay all memories based on elapsed ticks, remove those at zero strength
    void Tick(uint64_t currentTick);

    size_t Count() const;

private:
    std::unordered_map<std::string, MemoryEntry> m_memories;
    uint64_t m_lastTickProcessed = 0;
};

}
