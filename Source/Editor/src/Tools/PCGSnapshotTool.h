#pragma once
#include "ITool.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::tools {

/// A saved PCG snapshot (seed + version + optional state blob).
struct PCGSnapshot {
    uint32_t id = 0;
    std::string name;
    uint64_t seed = 0;
    uint32_t version = 0;
    std::vector<uint8_t> stateBlob;
};

/// In-client overlay tool for PCG snapshot and rollback.
///
/// Capture the current procedural generation state, name it, and later
/// restore any snapshot to quickly A/B test different generation results.
class PCGSnapshotTool : public ITool {
public:
    std::string Name() const override { return "PCG Snapshot"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    /// Take a snapshot of the current PCG state.
    uint32_t TakeSnapshot(const std::string& name, uint64_t seed, uint32_t version);

    /// Restore a previously saved snapshot by id.  Returns false if not found.
    bool Restore(uint32_t id);

    /// Get snapshot by id (nullptr if not found).
    const PCGSnapshot* GetSnapshot(uint32_t id) const;

    const std::vector<PCGSnapshot>& AllSnapshots() const { return m_snapshots; }
    size_t SnapshotCount() const { return m_snapshots.size(); }

    /// The id of the most recently restored snapshot (0 if none).
    uint32_t LastRestoredId() const { return m_lastRestoredId; }

    void ClearSnapshots();

private:
    bool m_active = false;
    std::vector<PCGSnapshot> m_snapshots;
    uint32_t m_nextId = 1;
    uint32_t m_lastRestoredId = 0;
};

} // namespace atlas::tools
