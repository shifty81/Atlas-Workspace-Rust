#pragma once
// ============================================================
// Atlas World State Serializer — Versioned State Persistence
// ============================================================
//
// Provides schema-versioned serialization for world state,
// enabling forward/backward compatibility across engine versions.
//
// Each state schema version has a migration path so that saves
// from older engine versions can be loaded and upgraded to the
// current format without data loss.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/STATE_MODEL.md

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace atlas::sim {

/// Schema version identifier for serialized state.
struct SchemaVersion {
    uint32_t major = 1;
    uint32_t minor = 0;

    bool operator==(const SchemaVersion& o) const {
        return major == o.major && minor == o.minor;
    }
    bool operator!=(const SchemaVersion& o) const { return !(*this == o); }
    bool operator<(const SchemaVersion& o) const {
        return major < o.major || (major == o.major && minor < o.minor);
    }
    bool operator<=(const SchemaVersion& o) const {
        return *this == o || *this < o;
    }
};

/// Result of a serialization or migration operation.
enum class SerializerResult {
    Success,
    VersionTooOld,       ///< Source version below minimum supported.
    VersionTooNew,       ///< Source version newer than current.
    MigrationFailed,     ///< A migration step returned an error.
    HashMismatch,        ///< Post-migration hash verification failed.
    InvalidData,         ///< Input data is malformed.
};

/// A migration function that transforms state data from one version to the next.
/// Returns true on success. The data vector is modified in place.
using MigrationFn = std::function<bool(std::vector<uint8_t>& data)>;

/// Describes a single migration step.
struct MigrationStep {
    SchemaVersion from;
    SchemaVersion to;
    std::string description;
    MigrationFn migrate;
};

/// Serialized state envelope with version metadata.
struct SerializedState {
    SchemaVersion version;
    uint64_t hash = 0;         ///< Hash of the payload data.
    std::vector<uint8_t> data; ///< The serialized payload.
};

/// Manages schema-versioned serialization and migration of world state.
class WorldStateSerializer {
public:
    /// Set the current (latest) schema version.
    void SetCurrentVersion(SchemaVersion version);
    SchemaVersion CurrentVersion() const;

    /// Set the minimum schema version that can be migrated from.
    void SetMinimumVersion(SchemaVersion version);
    SchemaVersion MinimumVersion() const;

    /// Register a migration step from one version to the next.
    void RegisterMigration(MigrationStep step);

    /// Number of registered migration steps.
    size_t MigrationCount() const;

    /// Serialize data at the current schema version.
    /// Computes a hash over the data for integrity verification.
    SerializedState Serialize(const std::vector<uint8_t>& data) const;

    /// Deserialize and migrate data to the current schema version.
    /// If the source version differs from current, registered migrations
    /// are applied in sequence. Returns an error if no migration path exists.
    SerializerResult Deserialize(SerializedState& state) const;

    /// Check whether a migration path exists from the given version to current.
    bool CanMigrate(SchemaVersion from) const;

    /// Get the list of migration steps that would be applied.
    std::vector<MigrationStep> MigrationPath(SchemaVersion from) const;

    /// Validate a serialized state envelope (check hash, version bounds).
    SerializerResult Validate(const SerializedState& state) const;

private:
    SchemaVersion m_currentVersion{1, 0};
    SchemaVersion m_minimumVersion{1, 0};
    std::vector<MigrationStep> m_migrations;
};

}  // namespace atlas::sim
