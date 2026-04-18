#include "../engine/production/BuildManifest.h"
#include <iostream>
#include <cassert>

void test_manifest_defaults() {
    atlas::production::BuildManifest m;
    assert(m.schemaVersion == atlas::production::BuildManifest::SCHEMA_VERSION);
    assert(m.artifacts.empty());
    std::cout << "[PASS] test_manifest_defaults" << std::endl;
}

void test_manifest_add_artifact() {
    atlas::production::BuildManifest m;
    atlas::production::ArtifactEntry entry;
    entry.path = "bin/game.exe";
    entry.type = "executable";
    entry.hash = 12345;
    entry.size = 1024;
    entry.version = "1.0";
    m.AddArtifact(entry);
    assert(m.artifacts.size() == 1);
    std::cout << "[PASS] test_manifest_add_artifact" << std::endl;
}

void test_manifest_find_artifact() {
    atlas::production::BuildManifest m;
    atlas::production::ArtifactEntry entry;
    entry.path = "lib/engine.so";
    entry.type = "library";
    entry.hash = 99999;
    entry.size = 2048;
    m.AddArtifact(entry);
    auto* found = m.FindArtifact("lib/engine.so");
    assert(found != nullptr);
    assert(found->type == "library");
    assert(found->hash == 99999);
    std::cout << "[PASS] test_manifest_find_artifact" << std::endl;
}

void test_manifest_find_missing() {
    atlas::production::BuildManifest m;
    auto* found = m.FindArtifact("nonexistent");
    assert(found == nullptr);
    std::cout << "[PASS] test_manifest_find_missing" << std::endl;
}

void test_manifest_validate_valid() {
    atlas::production::BuildManifest m;
    m.projectName = "TestProject";
    m.buildType = "Debug";
    m.platform = "Linux";
    m.engineVersion = "1.0.0";
    m.buildTimestamp = "2025-01-01T00:00:00Z";
    assert(m.Validate());
    std::cout << "[PASS] test_manifest_validate_valid" << std::endl;
}

void test_manifest_validate_missing_fields() {
    atlas::production::BuildManifest m;
    // Leave required fields empty
    assert(!m.Validate());
    std::cout << "[PASS] test_manifest_validate_missing_fields" << std::endl;
}

void test_manifest_to_json() {
    atlas::production::BuildManifest m;
    m.projectName = "JsonTest";
    m.buildType = "Release";
    m.platform = "Windows";
    m.engineVersion = "2.0.0";
    m.buildTimestamp = "2025-06-01T12:00:00Z";
    std::string json = m.ToJSON();
    assert(!json.empty());
    assert(json.find("JsonTest") != std::string::npos);
    assert(json.find("Release") != std::string::npos);
    std::cout << "[PASS] test_manifest_to_json" << std::endl;
}

void test_manifest_roundtrip() {
    atlas::production::BuildManifest m;
    m.projectName = "Roundtrip";
    m.buildType = "Development";
    m.platform = "macOS";
    m.engineVersion = "3.0.0";
    m.buildTimestamp = "2025-06-15T08:30:00Z";
    atlas::production::ArtifactEntry entry;
    entry.path = "bin/app";
    entry.type = "executable";
    entry.hash = 42;
    entry.size = 512;
    entry.version = "1.0";
    m.AddArtifact(entry);

    std::string json = m.ToJSON();
    auto restored = atlas::production::BuildManifest::FromJSON(json);
    assert(restored.projectName == m.projectName);
    assert(restored.buildType == m.buildType);
    assert(restored.platform == m.platform);
    assert(restored.engineVersion == m.engineVersion);
    assert(restored.artifacts.size() == 1);
    assert(restored.artifacts[0].path == "bin/app");
    assert(restored.artifacts[0].hash == 42);
    std::cout << "[PASS] test_manifest_roundtrip" << std::endl;
}

void test_manifest_verify_hash() {
    atlas::production::BuildManifest m;
    atlas::production::ArtifactEntry entry;
    entry.path = "data/level.dat";
    entry.type = "asset";
    entry.hash = 777;
    entry.size = 4096;
    m.AddArtifact(entry);
    assert(m.VerifyArtifactHash("data/level.dat", 777));
    assert(!m.VerifyArtifactHash("data/level.dat", 999));
    std::cout << "[PASS] test_manifest_verify_hash" << std::endl;
}
