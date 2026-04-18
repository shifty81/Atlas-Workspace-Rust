#pragma once
// ============================================================
// Atlas World State — Formal State Model
// ============================================================
//
// Defines the canonical state categories for the engine:
//
//   SimulatedState  — Authoritative game state. Fully deterministic,
//                     serializable, and replicable. This is the ONLY
//                     state that participates in hashing, save/load,
//                     and replay verification.
//
//   DerivedState    — Computed from SimulatedState each tick. Never
//                     serialized or hashed. Rebuilt on load/rollback.
//                     Examples: spatial queries, cached path results.
//
//   PresentationState — Rendering-only state (interpolation, VFX,
//                       camera smoothing). Never enters simulation.
//
//   DebugState      — Editor/debug overlays. Stripped in release.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ATLAS_SIMULATION_PHILOSOPHY.md

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace atlas::sim {

/// Category tag for state classification.
enum class StateCategory {
    Simulated,      ///< Authoritative, hashed, serialized.
    Derived,        ///< Recomputed from simulated each tick.
    Presentation,   ///< Rendering only, never in simulation.
    Debug           ///< Editor/debug, stripped in release builds.
};

/// Metadata for a registered state block.
struct StateBlockInfo {
    std::string name;
    StateCategory category = StateCategory::Simulated;
    uint32_t typeTag = 0;
    size_t estimatedSize = 0;
};

/// Snapshot of all simulated state at a given tick.
struct WorldSnapshot {
    uint64_t tick = 0;
    uint64_t stateHash = 0;
    std::vector<uint8_t> ecsData;       ///< Serialized ECS world.
    std::vector<uint8_t> auxiliaryData;  ///< Additional simulated state.
};

/// Manages world state classification, snapshotting, and rollback.
class WorldState {
public:
    /// Register a named state block with its category.
    void RegisterBlock(const std::string& name, StateCategory category,
                       uint32_t typeTag = 0, size_t estimatedSize = 0);

    /// Query registered state blocks.
    const std::vector<StateBlockInfo>& RegisteredBlocks() const;

    /// Find a registered block by name. Returns nullptr if not found.
    const StateBlockInfo* FindBlock(const std::string& name) const;

    /// Take a snapshot of the current simulated state.
    /// The caller provides serialized ECS data and any auxiliary data.
    WorldSnapshot TakeSnapshot(uint64_t tick,
                               const std::vector<uint8_t>& ecsData,
                               const std::vector<uint8_t>& auxiliaryData = {}) const;

    /// Store a snapshot for potential rollback.
    void PushSnapshot(WorldSnapshot snapshot);

    /// Retrieve the most recent snapshot, or nullptr if none.
    const WorldSnapshot* LatestSnapshot() const;

    /// Retrieve a snapshot at a specific tick, or nullptr if not found.
    const WorldSnapshot* SnapshotAtTick(uint64_t tick) const;

    /// Number of stored snapshots.
    size_t SnapshotCount() const;

    /// Set maximum number of snapshots to retain (for memory control).
    void SetMaxSnapshots(size_t max);
    size_t MaxSnapshots() const;

    /// Discard all stored snapshots.
    void ClearSnapshots();

    /// Discard snapshots older than the given tick.
    void PruneSnapshotsBefore(uint64_t tick);

    // --- System mutation ownership ---

    /// Register that a system owns (may mutate) a component type.
    void RegisterOwnership(const std::string& systemName,
                           const std::string& componentName);

    /// Check whether a system owns a component type.
    bool OwnsComponent(const std::string& systemName,
                       const std::string& componentName) const;

    /// Get all components owned by a system.
    std::vector<std::string> OwnedComponents(const std::string& systemName) const;

    /// Get the owning system for a component, or empty string if unowned.
    std::string OwnerOf(const std::string& componentName) const;

    /// Check whether a system is allowed to mutate a component.
    /// Returns false if another system already owns the component.
    bool CanMutate(const std::string& systemName,
                   const std::string& componentName) const;

    /// Register a callback invoked when derived state should be rebuilt.
    void SetDerivedRebuildCallback(std::function<void(const WorldSnapshot&)> cb);

    /// Trigger derived state rebuild from the latest snapshot.
    void RebuildDerived();

private:
    std::vector<StateBlockInfo> m_blocks;
    std::vector<WorldSnapshot> m_snapshots;
    size_t m_maxSnapshots = 60;  ///< Default: ~2 seconds at 30 Hz.
    std::function<void(const WorldSnapshot&)> m_derivedRebuildCb;

    /// Maps component name → owning system name.
    std::unordered_map<std::string, std::string> m_componentOwners;
};

}  // namespace atlas::sim
