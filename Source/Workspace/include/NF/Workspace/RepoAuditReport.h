#pragma once
// NF::Workspace — RepoAuditReport: programmatic repository audit result.
//
// RepoAuditReport is a data model for the output of a repo audit. It complements
// the shell-script validator (Scripts/validate_project.sh) with a C++ type that
// can be produced by automated tooling, tested, and consumed by the workspace UI
// (e.g. a Diagnostics panel or CI gate).
//
// Each audit check is an AuditCheckEntry with an id, category, status, and
// optional message. The report also carries high-level summary counters.
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 6.

#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Audit check status ────────────────────────────────────────────────────

enum class AuditCheckStatus : uint8_t {
    Pass,    // check passed
    Warn,    // advisory — does not block
    Fail,    // check failed — blocks the audit
    Skip,    // check was intentionally skipped (e.g. platform-specific)
};

inline const char* auditCheckStatusName(AuditCheckStatus s) {
    switch (s) {
        case AuditCheckStatus::Pass: return "Pass";
        case AuditCheckStatus::Warn: return "Warn";
        case AuditCheckStatus::Fail: return "Fail";
        case AuditCheckStatus::Skip: return "Skip";
    }
    return "Unknown";
}

// ── Audit check category ──────────────────────────────────────────────────

enum class AuditCategory : uint8_t {
    Structure,    // required files and directories
    Naming,       // naming convention compliance
    Dependencies, // dependency policy
    Tests,        // test coverage and pass/fail
    Docs,         // documentation presence
    Build,        // build system health
    Security,     // secret scanning, deprecated API usage
};

inline const char* auditCategoryName(AuditCategory c) {
    switch (c) {
        case AuditCategory::Structure:    return "Structure";
        case AuditCategory::Naming:       return "Naming";
        case AuditCategory::Dependencies: return "Dependencies";
        case AuditCategory::Tests:        return "Tests";
        case AuditCategory::Docs:         return "Docs";
        case AuditCategory::Build:        return "Build";
        case AuditCategory::Security:     return "Security";
    }
    return "Unknown";
}

// ── AuditCheckEntry ───────────────────────────────────────────────────────

struct AuditCheckEntry {
    std::string       id;        // machine-readable key, e.g. "source_dir_exists"
    std::string       label;     // human-readable label, e.g. "Source/ directory exists"
    AuditCategory     category  = AuditCategory::Structure;
    AuditCheckStatus  status    = AuditCheckStatus::Fail;
    std::string       message;   // optional detail (populated on Warn/Fail)
};

// ── RepoAuditReport ───────────────────────────────────────────────────────

struct RepoAuditReport {
    std::string                  repoRoot;    // absolute path that was audited
    std::string                  auditedAt;   // ISO-8601 timestamp (informational)
    std::vector<AuditCheckEntry> checks;

    // ── Helpers ───────────────────────────────────────────────────

    void addCheck(std::string id, std::string label,
                  AuditCategory category, AuditCheckStatus status,
                  std::string message = {}) {
        checks.push_back({std::move(id), std::move(label),
                          category, status, std::move(message)});
    }

    void pass(std::string id, std::string label, AuditCategory cat) {
        addCheck(std::move(id), std::move(label), cat, AuditCheckStatus::Pass);
    }

    void warn(std::string id, std::string label, AuditCategory cat, std::string msg) {
        addCheck(std::move(id), std::move(label), cat, AuditCheckStatus::Warn, std::move(msg));
    }

    void fail(std::string id, std::string label, AuditCategory cat, std::string msg) {
        addCheck(std::move(id), std::move(label), cat, AuditCheckStatus::Fail, std::move(msg));
    }

    void skip(std::string id, std::string label, AuditCategory cat, std::string msg = {}) {
        addCheck(std::move(id), std::move(label), cat, AuditCheckStatus::Skip, std::move(msg));
    }

    // ── Summary counters ──────────────────────────────────────────

    [[nodiscard]] uint32_t totalChecks() const {
        return static_cast<uint32_t>(checks.size());
    }

    [[nodiscard]] uint32_t countByStatus(AuditCheckStatus s) const {
        uint32_t n = 0;
        for (const auto& c : checks)
            if (c.status == s) ++n;
        return n;
    }

    [[nodiscard]] uint32_t passCount()  const { return countByStatus(AuditCheckStatus::Pass); }
    [[nodiscard]] uint32_t warnCount()  const { return countByStatus(AuditCheckStatus::Warn); }
    [[nodiscard]] uint32_t failCount()  const { return countByStatus(AuditCheckStatus::Fail); }
    [[nodiscard]] uint32_t skipCount()  const { return countByStatus(AuditCheckStatus::Skip); }

    /// Audit passes if there are no Fail-status checks.
    [[nodiscard]] bool passed() const { return failCount() == 0; }

    [[nodiscard]] uint32_t countByCategory(AuditCategory cat) const {
        uint32_t n = 0;
        for (const auto& c : checks)
            if (c.category == cat) ++n;
        return n;
    }

    [[nodiscard]] std::vector<const AuditCheckEntry*> failures() const {
        std::vector<const AuditCheckEntry*> out;
        for (const auto& c : checks)
            if (c.status == AuditCheckStatus::Fail)
                out.push_back(&c);
        return out;
    }

    [[nodiscard]] std::vector<const AuditCheckEntry*> warnings() const {
        std::vector<const AuditCheckEntry*> out;
        for (const auto& c : checks)
            if (c.status == AuditCheckStatus::Warn)
                out.push_back(&c);
        return out;
    }
};

} // namespace NF
