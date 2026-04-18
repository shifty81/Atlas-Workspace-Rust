#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::production {

struct ArtifactEntry {
    std::string path;
    std::string type;       // "executable", "library", "asset", "config"
    uint64_t hash = 0;
    uint64_t size = 0;
    std::string version;
};

struct BuildManifest {
    static constexpr uint32_t SCHEMA_VERSION = 1;

    uint32_t schemaVersion = SCHEMA_VERSION;
    std::string projectName;
    std::string buildType;      // "Debug", "Development", "Release"
    std::string platform;       // "Windows", "Linux", "macOS"
    std::string engineVersion;
    std::string buildTimestamp;  // ISO 8601
    uint64_t layoutHash = 0;    // Hash of editor layout DSL
    std::string dslVersion;     // GUI DSL version

    std::vector<ArtifactEntry> artifacts;
    std::unordered_map<std::string, std::string> metadata;

    // JSON serialization
    std::string ToJSON() const;
    static BuildManifest FromJSON(const std::string& json);

    // Validation
    bool Validate() const;

    // Artifact management
    void AddArtifact(const ArtifactEntry& entry);
    const ArtifactEntry* FindArtifact(const std::string& path) const;

    // Hash verification
    bool VerifyArtifactHash(const std::string& path, uint64_t expectedHash) const;
};

} // namespace atlas::production
