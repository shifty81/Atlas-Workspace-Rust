#include "PCGSnapshotTool.h"

namespace atlas::tools {

void PCGSnapshotTool::Activate() {
    m_active = true;
    m_lastRestoredId = 0;
}

void PCGSnapshotTool::Deactivate() {
    m_active = false;
}

void PCGSnapshotTool::Update(float /*deltaTime*/) {
    // Snapshot tool is event-driven.
}

uint32_t PCGSnapshotTool::TakeSnapshot(const std::string& name, uint64_t seed, uint32_t version) {
    PCGSnapshot snap;
    snap.id = m_nextId++;
    snap.name = name;
    snap.seed = seed;
    snap.version = version;
    m_snapshots.push_back(std::move(snap));
    return m_snapshots.back().id;
}

bool PCGSnapshotTool::Restore(uint32_t id) {
    for (const auto& s : m_snapshots) {
        if (s.id == id) {
            m_lastRestoredId = id;
            return true;
        }
    }
    return false;
}

const PCGSnapshot* PCGSnapshotTool::GetSnapshot(uint32_t id) const {
    for (const auto& s : m_snapshots) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

void PCGSnapshotTool::ClearSnapshots() {
    m_snapshots.clear();
    m_nextId = 1;
    m_lastRestoredId = 0;
}

} // namespace atlas::tools
