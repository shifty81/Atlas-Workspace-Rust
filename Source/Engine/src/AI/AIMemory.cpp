#include "AIMemory.h"
#include <algorithm>

namespace atlas::ai {

void AIMemory::Store(const std::string& key, const std::string& value,
                     float strength, float decayRate, uint64_t tick) {
    MemoryEntry entry;
    entry.key = key;
    entry.value = value;
    entry.strength = strength;
    entry.decayRate = decayRate;
    entry.createdTick = tick;
    m_memories[key] = entry;
}

const MemoryEntry* AIMemory::Recall(const std::string& key) const {
    auto it = m_memories.find(key);
    if (it == m_memories.end()) return nullptr;
    return &it->second;
}

bool AIMemory::Has(const std::string& key) const {
    return m_memories.count(key) > 0;
}

void AIMemory::Forget(const std::string& key) {
    m_memories.erase(key);
}

void AIMemory::Clear() {
    m_memories.clear();
    m_lastTickProcessed = 0;
}

void AIMemory::Tick(uint64_t currentTick) {
    if (currentTick <= m_lastTickProcessed) return;

    uint64_t elapsed = currentTick - m_lastTickProcessed;
    m_lastTickProcessed = currentTick;

    auto it = m_memories.begin();
    while (it != m_memories.end()) {
        if (it->second.decayRate > 0.0f) {
            it->second.strength -= it->second.decayRate * static_cast<float>(elapsed);
            if (it->second.strength <= 0.0f) {
                it = m_memories.erase(it);
                continue;
            }
        }
        ++it;
    }
}

size_t AIMemory::Count() const {
    return m_memories.size();
}

}
