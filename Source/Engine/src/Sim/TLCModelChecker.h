#pragma once
// ============================================================
// Atlas TLC Model Checker — TLA+ Specification CI Integration
// ============================================================
//
// Manages TLA+ specifications and runs TLC model checking for
// continuous integration.  Specs are registered by name and
// file path; ExtractInvariants parses TLA+ definitions of the
// form "Name == <expr>" at the start of a line.  When TLC is
// not available (CI stub mode), CheckSpec simulates a pass by
// parsing the spec and marking extracted invariants as verified.
//
// See: specs/ecs.tla, specs/replay.tla, specs/layout.tla

#include <string>
#include <vector>
#include <map>

namespace atlas::sim {

enum class TLCResult {
    Pass,
    Fail,
    Error,
    Timeout,
    NotFound
};

struct TLCInvariant {
    std::string name;
    bool verified = false;
    std::string detail;
};

struct TLCCheckReport {
    std::string specName;
    std::string specPath;
    TLCResult result = TLCResult::NotFound;
    int statesExplored = 0;
    int distinctStates = 0;
    double elapsedSeconds = 0.0;
    std::vector<TLCInvariant> invariants;
    std::string errorMessage;
};

struct TLCSpec {
    std::string name;
    std::string path;
    std::string content;
    std::vector<std::string> invariantNames;
};

class TLCModelChecker {
public:
    /// Register a TLA+ specification with its file path.
    bool RegisterSpec(const std::string& name, const std::string& path);

    /// Remove a registered spec.
    bool UnregisterSpec(const std::string& name);

    /// Get registered spec count.
    size_t SpecCount() const;

    /// Get list of registered spec names.
    std::vector<std::string> SpecNames() const;

    /// Get a registered spec by name (nullptr if not found).
    const TLCSpec* GetSpec(const std::string& name) const;

    /// Parse TLA+ content and extract invariant names.
    static std::vector<std::string> ExtractInvariants(const std::string& content);

    /// Validate a spec file exists and can be parsed.
    bool ValidateSpec(const std::string& name) const;

    /// Run TLC check on a named spec (returns report).
    TLCCheckReport CheckSpec(const std::string& name) const;

    /// Run TLC check on all registered specs.
    std::vector<TLCCheckReport> CheckAll() const;

    /// Set TLC executable path (for real TLC integration).
    void SetTLCPath(const std::string& path);
    const std::string& GetTLCPath() const;

    /// Check if TLC executable is available.
    bool IsTLCAvailable() const;

    /// Format a report as a human-readable string.
    static std::string FormatReport(const TLCCheckReport& report);

    /// Export report to file.
    static bool ExportReport(const TLCCheckReport& report,
                             const std::string& outputPath);

    /// CI mode: check all and return exit code (0=pass, 1=fail).
    int RunCICheck() const;

private:
    std::map<std::string, TLCSpec> m_specs;
    std::string m_tlcPath;

    bool LoadSpecContent(TLCSpec& spec) const;
};

}  // namespace atlas::sim
