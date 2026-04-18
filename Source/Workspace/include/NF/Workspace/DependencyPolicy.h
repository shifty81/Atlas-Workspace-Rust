#pragma once
// NF::Workspace — DependencyPolicy: dependency classification and policy enforcement.
//
// DependencyPolicy describes the approved set of external dependencies for the
// Atlas Workspace build. It supports:
//   - Classifying dependencies as Required / Optional / Forbidden
//   - Tagging dependencies with how they are acquired (Vendored / FetchContent / vcpkg / SystemProvided)
//   - Enforcing that online fetches are explicitly opt-in (ATLAS_ENABLE_ONLINE_DEPS)
//   - Producing a policy report for audit and CI gating
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 6.

#include <string>
#include <vector>
#include <unordered_map>

namespace NF {

// ── Dependency tier ───────────────────────────────────────────────────────

enum class DependencyTier : uint8_t {
    Required,   // must be present; build fails without it
    Optional,   // may be absent; feature degrades gracefully
    Forbidden,  // must NOT appear in the build graph
};

inline const char* dependencyTierName(DependencyTier t) {
    switch (t) {
        case DependencyTier::Required:  return "Required";
        case DependencyTier::Optional:  return "Optional";
        case DependencyTier::Forbidden: return "Forbidden";
    }
    return "Unknown";
}

// ── Dependency acquisition method ─────────────────────────────────────────

enum class DependencySource : uint8_t {
    Vendored,       // committed to repo (zero network needed)
    FetchContent,   // CMake FetchContent (needs ATLAS_ENABLE_ONLINE_DEPS)
    Vcpkg,          // vcpkg-managed (needs vcpkg toolchain)
    SystemProvided, // expected from OS / sysroot (e.g. OpenGL)
};

inline const char* dependencySourceName(DependencySource s) {
    switch (s) {
        case DependencySource::Vendored:       return "Vendored";
        case DependencySource::FetchContent:   return "FetchContent";
        case DependencySource::Vcpkg:          return "Vcpkg";
        case DependencySource::SystemProvided: return "SystemProvided";
    }
    return "Unknown";
}

// ── Dependency descriptor ─────────────────────────────────────────────────

struct DependencyDescriptor {
    std::string       name;          // e.g. "Catch2", "nlohmann-json"
    std::string       minVersion;    // e.g. "3.4.0" (informational)
    DependencyTier    tier   = DependencyTier::Required;
    DependencySource  source = DependencySource::Vendored;
    bool              onlineRequired = false; // true if ATLAS_ENABLE_ONLINE_DEPS needed
    std::string       notes;
};

// ── Policy violation ──────────────────────────────────────────────────────

struct PolicyViolation {
    std::string name;     // dependency name
    std::string reason;   // human-readable description
};

// ── DependencyPolicyReport ────────────────────────────────────────────────

struct DependencyPolicyReport {
    std::vector<DependencyDescriptor> checked;
    std::vector<PolicyViolation>      violations;
    bool onlineEnabled = false;  // mirrors ATLAS_ENABLE_ONLINE_DEPS at evaluation time

    [[nodiscard]] bool passed() const { return violations.empty(); }
    [[nodiscard]] uint32_t checkedCount() const {
        return static_cast<uint32_t>(checked.size());
    }
    [[nodiscard]] uint32_t violationCount() const {
        return static_cast<uint32_t>(violations.size());
    }
};

// ── DependencyPolicy ──────────────────────────────────────────────────────

class DependencyPolicy {
public:
    DependencyPolicy() = default;

    // ── Registration ──────────────────────────────────────────────

    void add(DependencyDescriptor dep) {
        m_deps.push_back(std::move(dep));
    }

    [[nodiscard]] uint32_t count() const {
        return static_cast<uint32_t>(m_deps.size());
    }

    [[nodiscard]] const std::vector<DependencyDescriptor>& all() const {
        return m_deps;
    }

    // ── Lookup ────────────────────────────────────────────────────

    [[nodiscard]] const DependencyDescriptor* find(const std::string& name) const {
        for (const auto& d : m_deps)
            if (d.name == name) return &d;
        return nullptr;
    }

    [[nodiscard]] bool isDeclared(const std::string& name) const {
        return find(name) != nullptr;
    }

    // ── Policy evaluation ─────────────────────────────────────────
    //
    // Evaluates the registered dependencies against the current environment.
    //   onlineEnabled — reflects whether ATLAS_ENABLE_ONLINE_DEPS=ON was set
    //   presentNames  — the set of dependency names actually found in the build
    //                   (Forbidden entries in this list are violations;
    //                    Required entries absent from this list are violations)

    [[nodiscard]] DependencyPolicyReport evaluate(
        bool onlineEnabled,
        const std::vector<std::string>& presentNames) const
    {
        DependencyPolicyReport report;
        report.checked = m_deps;
        report.onlineEnabled = onlineEnabled;

        // Build a quick lookup set
        std::unordered_map<std::string, bool> present;
        for (const auto& n : presentNames)
            present[n] = true;

        for (const auto& dep : m_deps) {
            bool isPresent = present.count(dep.name) > 0;

            // Forbidden dep is present → violation
            if (dep.tier == DependencyTier::Forbidden && isPresent) {
                report.violations.push_back({dep.name,
                    "Dependency '" + dep.name + "' is Forbidden but was found in build."});
                continue;
            }

            // Required dep is absent → violation
            if (dep.tier == DependencyTier::Required && !isPresent) {
                report.violations.push_back({dep.name,
                    "Required dependency '" + dep.name + "' is absent from the build."});
                continue;
            }

            // Online dep present but online deps disabled → violation
            if (dep.onlineRequired && !onlineEnabled && isPresent) {
                report.violations.push_back({dep.name,
                    "Dependency '" + dep.name + "' requires ATLAS_ENABLE_ONLINE_DEPS=ON "
                    "but online deps are disabled."});
            }
        }

        return report;
    }

private:
    std::vector<DependencyDescriptor> m_deps;
};

// ── Canonical workspace dependency policy ─────────────────────────────────
// Returns the standard policy used by AtlasWorkspace builds.

inline DependencyPolicy makeCanonicalDependencyPolicy() {
    DependencyPolicy policy;

    // Test framework — FetchContent, gated by ATLAS_ENABLE_ONLINE_DEPS
    policy.add({"Catch2",        "3.4.0",  DependencyTier::Optional,  DependencySource::FetchContent, true,  "Test framework"});
    // JSON — vcpkg
    policy.add({"nlohmann-json", "3.11.0", DependencyTier::Optional,  DependencySource::Vcpkg,        false, "JSON support"});
    // Logging — vcpkg
    policy.add({"spdlog",        "1.12.0", DependencyTier::Optional,  DependencySource::Vcpkg,        false, "Structured logging"});
    // Math — vcpkg
    policy.add({"glm",           "0.9.9",  DependencyTier::Optional,  DependencySource::Vcpkg,        false, "Math library"});
    // Format — vcpkg
    policy.add({"fmt",           "10.0.0", DependencyTier::Optional,  DependencySource::Vcpkg,        false, "String formatting"});
    // GLFW — vcpkg (platform-conditional)
    policy.add({"glfw3",         "3.3.0",  DependencyTier::Optional,  DependencySource::Vcpkg,        false, "Window system (Windows/macOS)"});
    // OpenGL — system
    policy.add({"opengl",        "",       DependencyTier::Optional,  DependencySource::SystemProvided, false, "OpenGL (system, Windows)"});
    // Vulkan headers — vcpkg
    policy.add({"vulkan-headers", "1.3.0", DependencyTier::Optional,  DependencySource::Vcpkg,        false, "Vulkan header-only"});

    return policy;
}

} // namespace NF
