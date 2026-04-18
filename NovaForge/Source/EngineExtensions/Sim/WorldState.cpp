#include "WorldState.h"
#include "StateHasher.h"
#include "../core/contract/SimulationGuard.h"
#include <algorithm>

namespace atlas::sim {

void WorldState::RegisterBlock(const std::string& name, StateCategory category,
                               uint32_t typeTag, size_t estimatedSize) {
    // Avoid duplicate registrations.
    for (const auto& b : m_blocks) {
        if (b.name == name) return;
    }
    StateBlockInfo info;
    info.name = name;
    info.category = category;
    info.typeTag = typeTag;
    info.estimatedSize = estimatedSize;
    m_blocks.push_back(std::move(info));
}

const std::vector<StateBlockInfo>& WorldState::RegisteredBlocks() const {
    return m_blocks;
}

const StateBlockInfo* WorldState::FindBlock(const std::string& name) const {
    for (const auto& b : m_blocks) {
        if (b.name == name) return &b;
    }
    return nullptr;
}

WorldSnapshot WorldState::TakeSnapshot(uint64_t tick,
                                       const std::vector<uint8_t>& ecsData,
                                       const std::vector<uint8_t>& auxiliaryData) const {
    WorldSnapshot snap;
    snap.tick = tick;
    snap.ecsData = ecsData;
    snap.auxiliaryData = auxiliaryData;

    // Compute hash over all simulated data.
    uint64_t hash = StateHasher::HashCombine(0, ecsData.data(), ecsData.size());
    if (!auxiliaryData.empty()) {
        hash = StateHasher::HashCombine(hash, auxiliaryData.data(), auxiliaryData.size());
    }
    snap.stateHash = hash;

    return snap;
}

void WorldState::PushSnapshot(WorldSnapshot snapshot) {
    // Snapshots represent simulation state and should only be taken during ticks
    ATLAS_SIM_MUTATION_GUARD();
    
    m_snapshots.push_back(std::move(snapshot));

    // Enforce max snapshot limit.
    while (m_snapshots.size() > m_maxSnapshots && !m_snapshots.empty()) {
        m_snapshots.erase(m_snapshots.begin());
    }
}

const WorldSnapshot* WorldState::LatestSnapshot() const {
    if (m_snapshots.empty()) return nullptr;
    return &m_snapshots.back();
}

const WorldSnapshot* WorldState::SnapshotAtTick(uint64_t tick) const {
    for (const auto& s : m_snapshots) {
        if (s.tick == tick) return &s;
    }
    return nullptr;
}

size_t WorldState::SnapshotCount() const {
    return m_snapshots.size();
}

void WorldState::SetMaxSnapshots(size_t max) {
    m_maxSnapshots = max > 0 ? max : 1;
}

size_t WorldState::MaxSnapshots() const {
    return m_maxSnapshots;
}

void WorldState::ClearSnapshots() {
    m_snapshots.clear();
}

void WorldState::PruneSnapshotsBefore(uint64_t tick) {
    m_snapshots.erase(
        std::remove_if(m_snapshots.begin(), m_snapshots.end(),
                        [tick](const WorldSnapshot& s) { return s.tick < tick; }),
        m_snapshots.end());
}

void WorldState::SetDerivedRebuildCallback(std::function<void(const WorldSnapshot&)> cb) {
    m_derivedRebuildCb = std::move(cb);
}

void WorldState::RebuildDerived() {
    if (m_derivedRebuildCb && !m_snapshots.empty()) {
        m_derivedRebuildCb(m_snapshots.back());
    }
}

// ---------------------------------------------------------------------------
// System mutation ownership
// ---------------------------------------------------------------------------

void WorldState::RegisterOwnership(const std::string& systemName,
                                   const std::string& componentName) {
    // Only register if no other system already owns it.
    auto it = m_componentOwners.find(componentName);
    if (it == m_componentOwners.end()) {
        m_componentOwners[componentName] = systemName;
    }
}

bool WorldState::OwnsComponent(const std::string& systemName,
                               const std::string& componentName) const {
    auto it = m_componentOwners.find(componentName);
    return it != m_componentOwners.end() && it->second == systemName;
}

std::vector<std::string> WorldState::OwnedComponents(
        const std::string& systemName) const {
    std::vector<std::string> result;
    for (const auto& [comp, owner] : m_componentOwners) {
        if (owner == systemName) {
            result.push_back(comp);
        }
    }
    return result;
}

std::string WorldState::OwnerOf(const std::string& componentName) const {
    auto it = m_componentOwners.find(componentName);
    if (it != m_componentOwners.end()) return it->second;
    return {};
}

bool WorldState::CanMutate(const std::string& systemName,
                           const std::string& componentName) const {
    auto it = m_componentOwners.find(componentName);
    if (it == m_componentOwners.end()) return true;  // unowned → anyone can mutate
    return it->second == systemName;
}

}  // namespace atlas::sim
