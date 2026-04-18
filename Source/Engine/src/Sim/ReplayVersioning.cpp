#include "ReplayVersioning.h"
#include <algorithm>

namespace atlas::sim {

void ReplayVersionRegistry::SetCurrentVersion(uint32_t version) {
    m_currentVersion = version;
}

uint32_t ReplayVersionRegistry::CurrentVersion() const {
    return m_currentVersion;
}

void ReplayVersionRegistry::SetMinimumVersion(uint32_t version) {
    m_minimumVersion = version;
}

uint32_t ReplayVersionRegistry::MinimumVersion() const {
    return m_minimumVersion;
}

void ReplayVersionRegistry::RegisterVersion(ReplayVersionInfo info) {
    // Replace existing entry for the same version.
    for (auto& v : m_versions) {
        if (v.version == info.version) {
            v = std::move(info);
            return;
        }
    }
    m_versions.push_back(std::move(info));
}

void ReplayVersionRegistry::RegisterMigration(ReplayMigrationStep step) {
    m_migrations.push_back(std::move(step));
}

size_t ReplayVersionRegistry::VersionCount() const {
    return m_versions.size();
}

size_t ReplayVersionRegistry::MigrationCount() const {
    return m_migrations.size();
}

ReplayCompatibility ReplayVersionRegistry::CheckCompatibility(uint32_t version) const {
    if (version == m_currentVersion) {
        return ReplayCompatibility::Compatible;
    }

    if (version > m_currentVersion) {
        return ReplayCompatibility::TooNew;
    }

    if (version < m_minimumVersion) {
        return ReplayCompatibility::TooOld;
    }

    // Check if we have a known version entry.
    if (GetVersionInfo(version) == nullptr) {
        return ReplayCompatibility::Unknown;
    }

    // Check if migration path exists.
    if (CanMigrate(version)) {
        return ReplayCompatibility::Upgradeable;
    }

    return ReplayCompatibility::Unknown;
}

const ReplayVersionInfo* ReplayVersionRegistry::GetVersionInfo(uint32_t version) const {
    for (const auto& v : m_versions) {
        if (v.version == version) return &v;
    }
    return nullptr;
}

std::vector<ReplayMigrationStep> ReplayVersionRegistry::MigrationPath(uint32_t fromVersion) const {
    std::vector<ReplayMigrationStep> path;
    uint32_t current = fromVersion;
    // Guard against cycles by limiting iterations.
    size_t maxSteps = m_migrations.size();
    size_t steps = 0;

    while (current != m_currentVersion && steps < maxSteps) {
        bool found = false;
        for (const auto& m : m_migrations) {
            if (m.fromVersion == current) {
                path.push_back(m);
                current = m.toVersion;
                found = true;
                break;
            }
        }
        if (!found) {
            return {};  // No path found.
        }
        ++steps;
    }

    if (current != m_currentVersion) {
        return {};  // Cycle detected or path too long.
    }

    return path;
}

bool ReplayVersionRegistry::CanMigrate(uint32_t fromVersion) const {
    if (fromVersion == m_currentVersion) return true;
    return !MigrationPath(fromVersion).empty();
}

std::vector<ReplayVersionInfo> ReplayVersionRegistry::AllVersions() const {
    return m_versions;
}

std::vector<uint32_t> ReplayVersionRegistry::DeprecatedVersions() const {
    std::vector<uint32_t> deprecated;
    for (const auto& v : m_versions) {
        if (v.deprecated) deprecated.push_back(v.version);
    }
    return deprecated;
}

void ReplayVersionRegistry::RegisterDefaults() {
    RegisterVersion({1, "Initial replay format (input frames only)", true});
    RegisterVersion({2, "Added state hash per frame", true});
    RegisterVersion({3, "Added save-point markers per frame", false});

    // v1 → v2: Add zero hash field to each frame.
    RegisterMigration({1, 2, "Add stateHash field (zero-initialized)",
        [](std::vector<uint8_t>&, std::vector<std::vector<uint8_t>>& frames) -> bool {
            // In the actual binary format, each frame would get an additional
            // uint64_t hash field. For this migration, we append 8 zero bytes
            // to each frame's raw data to represent the new hash field.
            for (auto& frame : frames) {
                frame.insert(frame.end(), 8, 0);
            }
            return true;
        }});

    // v2 → v3: Add save-point boolean to each frame.
    RegisterMigration({2, 3, "Add isSavePoint field (false-initialized)",
        [](std::vector<uint8_t>&, std::vector<std::vector<uint8_t>>& frames) -> bool {
            // Append a single zero byte (false) for isSavePoint to each frame.
            for (auto& frame : frames) {
                frame.push_back(0);
            }
            return true;
        }});
}

void ReplayVersionRegistry::Clear() {
    m_versions.clear();
    m_migrations.clear();
}

}  // namespace atlas::sim
