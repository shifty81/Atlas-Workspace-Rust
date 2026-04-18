#pragma once
// NF::Workspace — ProjectLoadContract: formal project loading result and manifest.
//
// A ProjectLoadContract is produced by WorkspaceShell::loadProject() and captures
// the full metadata and validation result for a hosted project. It is the canonical
// record of what loaded, how it went, and whether the project is build-ready.
//
// Roles:
//   - Project identity (id, display name, version)
//   - Load lifecycle state (Unloaded → Loading → Ready | Failed)
//   - Validation error accumulation
//   - Content root and command inventory
//   - Panel count (for diagnostics)
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 5.

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Project load state ────────────────────────────────────────────────────

enum class ProjectLoadState : uint8_t {
    Unloaded,    // default; no project bound
    Loading,     // adapter::initialize() in progress
    Ready,       // fully loaded and validated
    Failed,      // adapter::initialize() returned false or validation failed
};

inline const char* projectLoadStateName(ProjectLoadState s) {
    switch (s) {
        case ProjectLoadState::Unloaded: return "Unloaded";
        case ProjectLoadState::Loading:  return "Loading";
        case ProjectLoadState::Ready:    return "Ready";
        case ProjectLoadState::Failed:   return "Failed";
    }
    return "Unknown";
}

// ── Validation error severity ─────────────────────────────────────────────

enum class ProjectValidationSeverity : uint8_t {
    Info,     // informational — does not block anything
    Warning,  // advisory — does NOT block loads or builds
    Error,    // blocks build
    Fatal,    // blocks load
};

inline const char* validationSeverityName(ProjectValidationSeverity s) {
    switch (s) {
        case ProjectValidationSeverity::Info:    return "Info";
        case ProjectValidationSeverity::Warning: return "Warning";
        case ProjectValidationSeverity::Error:   return "Error";
        case ProjectValidationSeverity::Fatal:   return "Fatal";
    }
    return "Unknown";
}

// ── Validation entry ──────────────────────────────────────────────────────
//
// Warning — advisory only; does NOT block loads or builds.
// Error   — blocks builds (but not loads).
// Fatal   — blocks loads.

struct ProjectValidationEntry {
    ProjectValidationSeverity severity = ProjectValidationSeverity::Info;
    std::string               code;    // short machine-readable key, e.g. "missing_content_root"
    std::string               message; // human-readable description
};

// ── ProjectLoadContract ───────────────────────────────────────────────────
//
// Populated by WorkspaceShell after calling IGameProjectAdapter::initialize().
// Consumers read this to understand what loaded and whether it is build-ready.

struct ProjectLoadContract {
    // ── Identity ──────────────────────────────────────────────────
    std::string projectId;          // from IGameProjectAdapter::projectId()
    std::string projectDisplayName; // from IGameProjectAdapter::projectDisplayName()
    std::string projectVersion;     // optional, supplied by adapter
    std::string projectRoot;        // from IGameProjectAdapter::projectRoot()

    // ── State ─────────────────────────────────────────────────────
    ProjectLoadState state = ProjectLoadState::Unloaded;

    // ── Timing ────────────────────────────────────────────────────
    int64_t loadTimestampMs = 0;    // epoch ms when loading completed (or failed)

    // ── Inventory ─────────────────────────────────────────────────
    std::vector<std::string> contentRoots;    // from adapter
    std::vector<std::string> customCommands;  // from adapter
    uint32_t                 panelCount = 0;  // number of panels registered

    // ── Validation ────────────────────────────────────────────────
    std::vector<ProjectValidationEntry> validationEntries;

    // ── Helpers ───────────────────────────────────────────────────

    [[nodiscard]] bool isLoaded() const { return state == ProjectLoadState::Ready; }
    [[nodiscard]] bool hasFailed() const { return state == ProjectLoadState::Failed; }

    /// Returns true if there are no Fatal or Error-severity entries.
    [[nodiscard]] bool isValid() const {
        for (const auto& e : validationEntries) {
            if (e.severity == ProjectValidationSeverity::Fatal ||
                e.severity == ProjectValidationSeverity::Error)
                return false;
        }
        return true;
    }

    /// Returns true if the project is build-ready (loaded + valid, no blocking errors).
    [[nodiscard]] bool isBuildReady() const {
        return state == ProjectLoadState::Ready && isValid();
    }

    /// Count entries by severity.
    [[nodiscard]] uint32_t countBySeverity(ProjectValidationSeverity sev) const {
        uint32_t n = 0;
        for (const auto& e : validationEntries)
            if (e.severity == sev) ++n;
        return n;
    }

    void addEntry(ProjectValidationSeverity sev,
                  const std::string& code,
                  const std::string& message) {
        validationEntries.push_back({sev, code, message});
    }

    void addInfo(const std::string& code, const std::string& message) {
        addEntry(ProjectValidationSeverity::Info, code, message);
    }

    void addWarning(const std::string& code, const std::string& message) {
        addEntry(ProjectValidationSeverity::Warning, code, message);
    }

    void addError(const std::string& code, const std::string& message) {
        addEntry(ProjectValidationSeverity::Error, code, message);
    }

    void addFatal(const std::string& code, const std::string& message) {
        addEntry(ProjectValidationSeverity::Fatal, code, message);
    }
};

} // namespace NF
