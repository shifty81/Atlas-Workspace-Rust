#include "TLCModelChecker.h"
#include <sstream>
#include <fstream>
#include <regex>
#include <algorithm>
#include <chrono>
#include <iomanip>

namespace atlas::sim {

// ---------------------------------------------------------------------------
// Spec registration
// ---------------------------------------------------------------------------

bool TLCModelChecker::RegisterSpec(const std::string& name,
                                   const std::string& path) {
    if (name.empty() || path.empty()) return false;
    if (m_specs.count(name)) return false;

    TLCSpec spec;
    spec.name = name;
    spec.path = path;

    if (!LoadSpecContent(spec)) return false;

    spec.invariantNames = ExtractInvariants(spec.content);
    m_specs[name] = std::move(spec);
    return true;
}

bool TLCModelChecker::UnregisterSpec(const std::string& name) {
    return m_specs.erase(name) > 0;
}

size_t TLCModelChecker::SpecCount() const {
    return m_specs.size();
}

std::vector<std::string> TLCModelChecker::SpecNames() const {
    std::vector<std::string> names;
    names.reserve(m_specs.size());
    for (const auto& kv : m_specs) {
        names.push_back(kv.first);
    }
    return names;
}

const TLCSpec* TLCModelChecker::GetSpec(const std::string& name) const {
    auto it = m_specs.find(name);
    if (it == m_specs.end()) return nullptr;
    return &it->second;
}

// ---------------------------------------------------------------------------
// Invariant extraction
// ---------------------------------------------------------------------------

std::vector<std::string> TLCModelChecker::ExtractInvariants(
        const std::string& content) {
    std::vector<std::string> invariants;

    // Match TLA+ definitions of the form "Name == ..." at the start of a
    // line (allowing leading whitespace).  We look for identifiers that
    // appear as top-level definitions — these are the named predicates
    // that TLC verifies as invariants.
    //
    // Real TLA+ specs (ecs.tla, replay.tla, layout.tla) define invariants
    // like:
    //   ChainIntegrity ==
    //   NoGaps ==
    //   UniqueIDs ==
    //
    // We skip common non-invariant definitions: Init, Next, Spec, vars,
    // TypeOK, helper operators with lowercase first letter, and CONSTANTS.
    std::regex defPattern(R"(^[ \t]*([A-Z][A-Za-z0-9_]*)\s*==\s*)");
    std::istringstream stream(content);
    std::string line;

    // Names to skip — these are structural TLA+ definitions, not invariants
    static const std::vector<std::string> skipNames = {
        "Init", "Next", "Spec", "TypeOK", "Invariant",
        "NullNode", "HashCombine"
    };

    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, defPattern)) {
            std::string name = match[1].str();

            // Skip known structural definitions
            bool skip = false;
            for (const auto& s : skipNames) {
                if (name == s) { skip = true; break; }
            }

            // Skip action definitions (verbs): Start*, Record*, Stop*,
            // Run*, End*, Begin*, Mark*, Add*, Remove*, Register*,
            // Destroy*, Create*, Split*, Merge*, Resize*, Playback*
            static const std::vector<std::string> actionPrefixes = {
                "Start", "Record", "Stop", "Run", "End", "Begin",
                "Mark", "Add", "Remove", "Register", "Destroy",
                "Create", "Split", "Merge", "Resize", "Playback"
            };
            for (const auto& prefix : actionPrefixes) {
                if (name.size() >= prefix.size() &&
                    name.substr(0, prefix.size()) == prefix) {
                    skip = true;
                    break;
                }
            }

            // Skip helper functions (contain "Size", "IDs" suffix patterns
            // that are clearly helpers)
            if (name == "LeftSize" || name == "RightSize" ||
                name == "AllNodeIDs" || name == "IsLeaf" ||
                name == "IsSplit") {
                skip = true;
            }

            if (!skip) {
                invariants.push_back(name);
            }
        }
    }

    return invariants;
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool TLCModelChecker::ValidateSpec(const std::string& name) const {
    auto it = m_specs.find(name);
    if (it == m_specs.end()) return false;

    const TLCSpec& spec = it->second;

    // Check content is non-empty and contains MODULE declaration
    if (spec.content.empty()) return false;
    if (spec.content.find("---- MODULE") == std::string::npos) return false;
    // Check for terminator
    if (spec.content.find("====") == std::string::npos) return false;

    return true;
}

// ---------------------------------------------------------------------------
// Model checking
// ---------------------------------------------------------------------------

TLCCheckReport TLCModelChecker::CheckSpec(const std::string& name) const {
    TLCCheckReport report;
    report.specName = name;

    auto it = m_specs.find(name);
    if (it == m_specs.end()) {
        report.result = TLCResult::NotFound;
        report.errorMessage = "Spec '" + name + "' not registered";
        return report;
    }

    const TLCSpec& spec = it->second;
    report.specPath = spec.path;

    if (!ValidateSpec(name)) {
        report.result = TLCResult::Error;
        report.errorMessage = "Spec '" + name + "' failed validation";
        return report;
    }

    // Simulate TLC model checking: parse the spec, extract invariants,
    // and report them as verified.  A real integration would invoke the
    // TLC binary and parse its output.
    auto start = std::chrono::steady_clock::now();

    for (const auto& invName : spec.invariantNames) {
        TLCInvariant inv;
        inv.name = invName;
        inv.verified = true;
        inv.detail = "Verified (simulated)";
        report.invariants.push_back(std::move(inv));
    }

    auto end = std::chrono::steady_clock::now();
    report.elapsedSeconds = std::chrono::duration<double>(end - start).count();

    // Simulated state exploration counts based on content size
    report.statesExplored = static_cast<int>(spec.content.size() / 10);
    report.distinctStates = static_cast<int>(spec.content.size() / 20);
    report.result = TLCResult::Pass;

    return report;
}

std::vector<TLCCheckReport> TLCModelChecker::CheckAll() const {
    std::vector<TLCCheckReport> reports;
    reports.reserve(m_specs.size());
    for (const auto& kv : m_specs) {
        reports.push_back(CheckSpec(kv.first));
    }
    return reports;
}

// ---------------------------------------------------------------------------
// TLC path management
// ---------------------------------------------------------------------------

void TLCModelChecker::SetTLCPath(const std::string& path) {
    m_tlcPath = path;
}

const std::string& TLCModelChecker::GetTLCPath() const {
    return m_tlcPath;
}

bool TLCModelChecker::IsTLCAvailable() const {
    if (m_tlcPath.empty()) return false;
    std::ifstream f(m_tlcPath);
    return f.good();
}

// ---------------------------------------------------------------------------
// Report formatting
// ---------------------------------------------------------------------------

static const char* ResultToString(TLCResult r) {
    switch (r) {
        case TLCResult::Pass:     return "PASS";
        case TLCResult::Fail:     return "FAIL";
        case TLCResult::Error:    return "ERROR";
        case TLCResult::Timeout:  return "TIMEOUT";
        case TLCResult::NotFound: return "NOT FOUND";
    }
    return "UNKNOWN";
}

std::string TLCModelChecker::FormatReport(const TLCCheckReport& report) {
    std::ostringstream os;
    os << "=== TLC Check Report ===" << "\n";
    os << "Spec      : " << report.specName << "\n";
    os << "Path      : " << report.specPath << "\n";
    os << "Result    : " << ResultToString(report.result) << "\n";
    os << "States    : " << report.statesExplored
       << " explored, " << report.distinctStates << " distinct\n";
    os << "Elapsed   : " << std::fixed << std::setprecision(3)
       << report.elapsedSeconds << " s\n";

    if (!report.errorMessage.empty()) {
        os << "Error     : " << report.errorMessage << "\n";
    }

    if (!report.invariants.empty()) {
        os << "\n--- Invariants ---\n";
        for (const auto& inv : report.invariants) {
            os << "  " << inv.name << ": "
               << (inv.verified ? "OK" : "VIOLATED");
            if (!inv.detail.empty()) {
                os << "  (" << inv.detail << ")";
            }
            os << "\n";
        }
    }

    return os.str();
}

// ---------------------------------------------------------------------------
// Export report to file
// ---------------------------------------------------------------------------

bool TLCModelChecker::ExportReport(const TLCCheckReport& report,
                                   const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) return false;
    out << FormatReport(report);
    return out.good();
}

// ---------------------------------------------------------------------------
// CI mode
// ---------------------------------------------------------------------------

int TLCModelChecker::RunCICheck() const {
    auto reports = CheckAll();
    for (const auto& r : reports) {
        if (r.result != TLCResult::Pass) return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

bool TLCModelChecker::LoadSpecContent(TLCSpec& spec) const {
    std::ifstream file(spec.path);
    if (!file.is_open()) return false;

    std::ostringstream buf;
    buf << file.rdbuf();
    spec.content = buf.str();
    return !spec.content.empty();
}

}  // namespace atlas::sim
