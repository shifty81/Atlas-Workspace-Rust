#include "WorldStateSerializer.h"
#include "StateHasher.h"
#include <algorithm>

namespace atlas::sim {

void WorldStateSerializer::SetCurrentVersion(SchemaVersion version) {
    m_currentVersion = version;
}

SchemaVersion WorldStateSerializer::CurrentVersion() const {
    return m_currentVersion;
}

void WorldStateSerializer::SetMinimumVersion(SchemaVersion version) {
    m_minimumVersion = version;
}

SchemaVersion WorldStateSerializer::MinimumVersion() const {
    return m_minimumVersion;
}

void WorldStateSerializer::RegisterMigration(MigrationStep step) {
    m_migrations.push_back(std::move(step));
}

size_t WorldStateSerializer::MigrationCount() const {
    return m_migrations.size();
}

SerializedState WorldStateSerializer::Serialize(const std::vector<uint8_t>& data) const {
    SerializedState state;
    state.version = m_currentVersion;
    state.data = data;
    if (!data.empty()) {
        state.hash = StateHasher::HashCombine(0, data.data(), data.size());
    }
    return state;
}

SerializerResult WorldStateSerializer::Deserialize(SerializedState& state) const {
    // Validate the envelope first.
    SerializerResult validation = Validate(state);
    if (validation != SerializerResult::Success) {
        return validation;
    }

    // Already at current version — nothing to migrate.
    if (state.version == m_currentVersion) {
        return SerializerResult::Success;
    }

    // Build migration path.
    auto path = MigrationPath(state.version);
    if (path.empty()) {
        return SerializerResult::MigrationFailed;
    }

    // Apply each migration step in sequence.
    for (const auto& step : path) {
        if (!step.migrate(state.data)) {
            return SerializerResult::MigrationFailed;
        }
        state.version = step.to;
    }

    // Recompute hash after migration.
    if (!state.data.empty()) {
        state.hash = StateHasher::HashCombine(0, state.data.data(), state.data.size());
    } else {
        state.hash = 0;
    }

    return SerializerResult::Success;
}

bool WorldStateSerializer::CanMigrate(SchemaVersion from) const {
    if (from == m_currentVersion) return true;
    return !MigrationPath(from).empty();
}

std::vector<MigrationStep> WorldStateSerializer::MigrationPath(SchemaVersion from) const {
    std::vector<MigrationStep> path;
    SchemaVersion current = from;

    // Guard against cycles: path can never exceed migration count.
    const size_t maxSteps = m_migrations.size();

    while (current != m_currentVersion) {
        if (path.size() >= maxSteps) {
            return {};  // Cycle detected or path too long.
        }
        bool found = false;
        for (const auto& m : m_migrations) {
            if (m.from == current) {
                path.push_back(m);
                current = m.to;
                found = true;
                break;
            }
        }
        if (!found) {
            return {};  // No path found.
        }
    }

    return path;
}

SerializerResult WorldStateSerializer::Validate(const SerializedState& state) const {
    if (state.version < m_minimumVersion) {
        return SerializerResult::VersionTooOld;
    }

    if (m_currentVersion < state.version) {
        return SerializerResult::VersionTooNew;
    }

    // Verify hash integrity.
    uint64_t expectedHash = 0;
    if (!state.data.empty()) {
        expectedHash = StateHasher::HashCombine(0, state.data.data(), state.data.size());
    }

    if (expectedHash != state.hash) {
        return SerializerResult::HashMismatch;
    }

    return SerializerResult::Success;
}

}  // namespace atlas::sim
