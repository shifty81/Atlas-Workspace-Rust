#pragma once
// NF::Workspace — ReleaseManifest: packaging and release path descriptor.
//
// A ReleaseManifest captures everything needed to describe a deliverable build:
// version, target platform, artifact list, build configuration, and release gates
// (conditions that must be satisfied before the release can be published).
//
// Design:
//   - ReleaseManifest is a plain data structure — no filesystem I/O in this header.
//   - A ReleaseGate is a named predicate: callers add gates; evaluate() runs them.
//   - ReleaseTarget maps a platform name to its artifact set.
//   - ReleaseManifestValidator checks consistency rules on the manifest.
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 6.

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Semantic version ──────────────────────────────────────────────────────

struct SemanticVersion {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
    std::string preRelease;  // e.g. "alpha.1", "rc.2"
    std::string buildMeta;   // e.g. git SHA

    [[nodiscard]] std::string toString() const {
        std::string s = std::to_string(major) + "." +
                        std::to_string(minor) + "." +
                        std::to_string(patch);
        if (!preRelease.empty()) s += "-" + preRelease;
        if (!buildMeta.empty())  s += "+" + buildMeta;
        return s;
    }

    [[nodiscard]] bool isStable() const {
        return preRelease.empty();
    }

    // Simple comparison ignoring pre-release/buildMeta.
    [[nodiscard]] bool operator==(const SemanticVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch &&
               preRelease == o.preRelease;
    }
    [[nodiscard]] bool operator!=(const SemanticVersion& o) const { return !(*this == o); }
};

// ── Build configuration ───────────────────────────────────────────────────

enum class BuildConfiguration : uint8_t {
    Debug,
    Release,
    RelWithDebInfo,
    MinSizeRel,
};

inline const char* buildConfigurationName(BuildConfiguration c) {
    switch (c) {
        case BuildConfiguration::Debug:          return "Debug";
        case BuildConfiguration::Release:        return "Release";
        case BuildConfiguration::RelWithDebInfo: return "RelWithDebInfo";
        case BuildConfiguration::MinSizeRel:     return "MinSizeRel";
    }
    return "Unknown";
}

// ── Release artifact ──────────────────────────────────────────────────────

struct ReleaseArtifact {
    std::string name;           // e.g. "AtlasWorkspace.exe"
    std::string relativePath;   // path within the release package
    bool        required = true;
    std::string checksum;       // optional expected SHA-256
};

// ── Release target (platform) ─────────────────────────────────────────────

struct ReleaseTarget {
    std::string                  platform;   // e.g. "windows-x64", "linux-x64"
    BuildConfiguration           config = BuildConfiguration::Release;
    std::vector<ReleaseArtifact> artifacts;

    [[nodiscard]] uint32_t requiredArtifactCount() const {
        uint32_t n = 0;
        for (const auto& a : artifacts)
            if (a.required) ++n;
        return n;
    }
};

// ── Release gate ──────────────────────────────────────────────────────────
// A predicate that must pass before the release may be published.

struct ReleaseGateResult {
    bool        passed = true;
    std::string gateId;
    std::string reason; // non-empty on failure
};

using ReleaseGateFn = std::function<ReleaseGateResult(const struct ReleaseManifest&)>;

// ── ReleaseManifest ───────────────────────────────────────────────────────

struct ReleaseManifest {
    // ── Identity ──────────────────────────────────────────────────
    std::string      projectName;
    SemanticVersion  version;
    std::string      releaseNotes;   // path or inline text
    std::string      releasedAt;     // ISO-8601 timestamp (informational)

    // ── Targets ───────────────────────────────────────────────────
    std::vector<ReleaseTarget> targets;

    // ── Helpers ───────────────────────────────────────────────────

    [[nodiscard]] const ReleaseTarget* findTarget(const std::string& platform) const {
        for (const auto& t : targets)
            if (t.platform == platform) return &t;
        return nullptr;
    }

    [[nodiscard]] bool hasTarget(const std::string& platform) const {
        return findTarget(platform) != nullptr;
    }

    [[nodiscard]] uint32_t targetCount() const {
        return static_cast<uint32_t>(targets.size());
    }

    [[nodiscard]] uint32_t totalArtifactCount() const {
        uint32_t n = 0;
        for (const auto& t : targets)
            n += static_cast<uint32_t>(t.artifacts.size());
        return n;
    }
};

// ── ReleaseManifestValidator ──────────────────────────────────────────────
// Runs registered gate functions against a manifest and collects results.

class ReleaseManifestValidator {
public:
    void addGate(std::string gateId, ReleaseGateFn fn) {
        if (!gateId.empty() && fn)
            m_gates.push_back({std::move(gateId), std::move(fn)});
    }

    void clearGates() { m_gates.clear(); }

    [[nodiscard]] uint32_t gateCount() const {
        return static_cast<uint32_t>(m_gates.size());
    }

    struct ValidationResult {
        bool                           passed = false;
        std::vector<ReleaseGateResult> gateResults;
        std::vector<std::string>       failures;

        [[nodiscard]] bool isPublishable() const { return passed; }
    };

    [[nodiscard]] ValidationResult validate(const ReleaseManifest& manifest) const {
        ValidationResult result;

        for (const auto& [gateId, fn] : m_gates) {
            auto gr = fn(manifest);
            gr.gateId = gateId;
            if (!gr.passed)
                result.failures.push_back("[Gate:" + gateId + "] " + gr.reason);
            result.gateResults.push_back(std::move(gr));
        }

        result.passed = result.failures.empty();
        return result;
    }

private:
    struct GateEntry {
        std::string    gateId;
        ReleaseGateFn  fn;
    };
    std::vector<GateEntry> m_gates;
};

} // namespace NF
