#pragma once
// ============================================================
// Atlas Replay Versioning — Format Compatibility & Migration
// ============================================================
//
// Manages replay format version compatibility to ensure that
// replays survive engine updates. Each format version defines
// the binary layout of ReplayHeader and ReplayFrame data.
//
// Compatibility rules:
//   - Replays at the current version load directly.
//   - Replays at older supported versions are migrated forward.
//   - Replays below the minimum version are rejected.
//   - Replays from newer versions than current are rejected.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/14_REPLAY_AND_PROOFS.md

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace atlas::sim {

/// Describes a supported replay format version.
struct ReplayVersionInfo {
    uint32_t version = 0;
    std::string description;
    bool deprecated = false;     ///< True if this version should warn on load.
};

/// Result of a replay compatibility check.
enum class ReplayCompatibility {
    Compatible,         ///< Same version, loads directly.
    Upgradeable,        ///< Older version, can be migrated.
    TooOld,             ///< Below minimum supported version.
    TooNew,             ///< Newer than current version.
    Unknown,            ///< Version not recognized at all.
};

/// A migration function that upgrades replay data from one version to the next.
/// Takes the raw frame data vector and modifies it in place.
/// Returns true on success.
using ReplayMigrationFn = std::function<bool(std::vector<uint8_t>& headerData,
                                              std::vector<std::vector<uint8_t>>& frameData)>;

/// Describes a single replay format migration step.
struct ReplayMigrationStep {
    uint32_t fromVersion = 0;
    uint32_t toVersion = 0;
    std::string description;
    ReplayMigrationFn migrate;
};

/// Manages replay format versions, compatibility checking, and migration.
class ReplayVersionRegistry {
public:
    /// Set the current (latest) replay format version.
    void SetCurrentVersion(uint32_t version);
    uint32_t CurrentVersion() const;

    /// Set the minimum replay format version that can be loaded.
    void SetMinimumVersion(uint32_t version);
    uint32_t MinimumVersion() const;

    /// Register a known replay format version.
    void RegisterVersion(ReplayVersionInfo info);

    /// Register a migration step between two versions.
    void RegisterMigration(ReplayMigrationStep step);

    /// Number of registered versions.
    size_t VersionCount() const;

    /// Number of registered migration steps.
    size_t MigrationCount() const;

    /// Check compatibility of a replay at the given version.
    ReplayCompatibility CheckCompatibility(uint32_t version) const;

    /// Get information about a specific version. Returns nullptr if not registered.
    const ReplayVersionInfo* GetVersionInfo(uint32_t version) const;

    /// Get the migration path from a version to current. Empty if no path exists.
    std::vector<ReplayMigrationStep> MigrationPath(uint32_t fromVersion) const;

    /// Check whether migration is possible from the given version.
    bool CanMigrate(uint32_t fromVersion) const;

    /// Get all registered versions.
    std::vector<ReplayVersionInfo> AllVersions() const;

    /// Get all deprecated versions.
    std::vector<uint32_t> DeprecatedVersions() const;

    /// Initialize with the default Atlas replay version history.
    void RegisterDefaults();

    /// Remove all registrations.
    void Clear();

private:
    uint32_t m_currentVersion = 3;
    uint32_t m_minimumVersion = 1;
    std::vector<ReplayVersionInfo> m_versions;
    std::vector<ReplayMigrationStep> m_migrations;
};

}  // namespace atlas::sim
