#include "../engine/sim/ReplayVersioning.h"
#include <iostream>
#include <cassert>

using namespace atlas::sim;

void test_replay_version_defaults() {
    ReplayVersionRegistry reg;
    assert(reg.CurrentVersion() == 3);
    assert(reg.MinimumVersion() == 1);
    assert(reg.VersionCount() == 0);
    assert(reg.MigrationCount() == 0);

    std::cout << "[PASS] test_replay_version_defaults" << std::endl;
}

void test_replay_version_set_versions() {
    ReplayVersionRegistry reg;
    reg.SetCurrentVersion(5);
    assert(reg.CurrentVersion() == 5);

    reg.SetMinimumVersion(3);
    assert(reg.MinimumVersion() == 3);

    std::cout << "[PASS] test_replay_version_set_versions" << std::endl;
}

void test_replay_version_register() {
    ReplayVersionRegistry reg;
    reg.RegisterVersion({1, "Initial format", true});
    reg.RegisterVersion({2, "Added hashes", false});

    assert(reg.VersionCount() == 2);

    const auto* v1 = reg.GetVersionInfo(1);
    assert(v1 != nullptr);
    assert(v1->description == "Initial format");
    assert(v1->deprecated);

    const auto* v2 = reg.GetVersionInfo(2);
    assert(v2 != nullptr);
    assert(!v2->deprecated);

    assert(reg.GetVersionInfo(99) == nullptr);

    std::cout << "[PASS] test_replay_version_register" << std::endl;
}

void test_replay_version_replace() {
    ReplayVersionRegistry reg;
    reg.RegisterVersion({1, "Old", false});
    reg.RegisterVersion({1, "New", true});

    assert(reg.VersionCount() == 1);
    assert(reg.GetVersionInfo(1)->description == "New");
    assert(reg.GetVersionInfo(1)->deprecated);

    std::cout << "[PASS] test_replay_version_replace" << std::endl;
}

void test_replay_version_compatibility_current() {
    ReplayVersionRegistry reg;
    reg.SetCurrentVersion(3);
    assert(reg.CheckCompatibility(3) == ReplayCompatibility::Compatible);

    std::cout << "[PASS] test_replay_version_compatibility_current" << std::endl;
}

void test_replay_version_compatibility_too_new() {
    ReplayVersionRegistry reg;
    reg.SetCurrentVersion(3);
    assert(reg.CheckCompatibility(4) == ReplayCompatibility::TooNew);

    std::cout << "[PASS] test_replay_version_compatibility_too_new" << std::endl;
}

void test_replay_version_compatibility_too_old() {
    ReplayVersionRegistry reg;
    reg.SetCurrentVersion(3);
    reg.SetMinimumVersion(2);
    assert(reg.CheckCompatibility(1) == ReplayCompatibility::TooOld);

    std::cout << "[PASS] test_replay_version_compatibility_too_old" << std::endl;
}

void test_replay_version_compatibility_upgradeable() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();

    assert(reg.CheckCompatibility(1) == ReplayCompatibility::Upgradeable);
    assert(reg.CheckCompatibility(2) == ReplayCompatibility::Upgradeable);

    std::cout << "[PASS] test_replay_version_compatibility_upgradeable" << std::endl;
}

void test_replay_version_compatibility_unknown() {
    ReplayVersionRegistry reg;
    reg.SetCurrentVersion(5);
    reg.SetMinimumVersion(1);
    // Version 3 is between min and current but not registered
    assert(reg.CheckCompatibility(3) == ReplayCompatibility::Unknown);

    std::cout << "[PASS] test_replay_version_compatibility_unknown" << std::endl;
}

void test_replay_version_migration_path() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();

    auto path = reg.MigrationPath(1);
    assert(path.size() == 2);  // v1→v2, v2→v3
    assert(path[0].fromVersion == 1);
    assert(path[0].toVersion == 2);
    assert(path[1].fromVersion == 2);
    assert(path[1].toVersion == 3);

    auto path2 = reg.MigrationPath(2);
    assert(path2.size() == 1);

    auto path3 = reg.MigrationPath(3);
    assert(path3.empty());  // Already at current

    std::cout << "[PASS] test_replay_version_migration_path" << std::endl;
}

void test_replay_version_can_migrate() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();

    assert(reg.CanMigrate(1));
    assert(reg.CanMigrate(2));
    assert(reg.CanMigrate(3));

    std::cout << "[PASS] test_replay_version_can_migrate" << std::endl;
}

void test_replay_version_deprecated() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();

    auto deprecated = reg.DeprecatedVersions();
    assert(deprecated.size() == 2);  // v1 and v2

    std::cout << "[PASS] test_replay_version_deprecated" << std::endl;
}

void test_replay_version_all_versions() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();

    auto versions = reg.AllVersions();
    assert(versions.size() == 3);

    std::cout << "[PASS] test_replay_version_all_versions" << std::endl;
}

void test_replay_version_migration_execution() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();

    // Simulate migration: v1 frame data → v3
    std::vector<uint8_t> headerData;
    std::vector<std::vector<uint8_t>> frameData;
    frameData.push_back({0x01, 0x02, 0x03});  // One frame of raw data

    auto path = reg.MigrationPath(1);
    assert(path.size() == 2);

    // Apply migrations
    for (const auto& step : path) {
        bool ok = step.migrate(headerData, frameData);
        assert(ok);
    }

    // After v1→v2: each frame gets 8 bytes (hash field)
    // After v2→v3: each frame gets 1 byte (isSavePoint)
    assert(frameData[0].size() == 3 + 8 + 1);  // 12 bytes total

    std::cout << "[PASS] test_replay_version_migration_execution" << std::endl;
}

void test_replay_version_clear() {
    ReplayVersionRegistry reg;
    reg.RegisterDefaults();
    assert(reg.VersionCount() > 0);
    assert(reg.MigrationCount() > 0);

    reg.Clear();
    assert(reg.VersionCount() == 0);
    assert(reg.MigrationCount() == 0);

    std::cout << "[PASS] test_replay_version_clear" << std::endl;
}
