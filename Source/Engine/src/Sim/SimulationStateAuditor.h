#pragma once
// ============================================================
// Atlas Simulation State Auditor
// ============================================================
//
// Detects hidden state in simulation systems by requiring every
// system to declare all state it owns.  At audit time, the
// auditor verifies:
//
//   1. Every registered simulation system has declared at least
//      one state block.
//   2. Every declared state block is categorized as Simulated
//      (or explicitly marked as Derived/Presentation/Debug).
//   3. No simulation-critical state exists outside declared blocks.
//
// See: docs/ATLAS_CORE_CONTRACT.md  §2 (Hidden global state)
//      docs/ATLAS_LOCKDOWN_CHECKLIST.md (No hidden state)

#include "WorldState.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/// Describes a single audit finding.
struct AuditFinding {
    enum class Severity { Info, Warning, Error };

    Severity severity = Severity::Info;
    std::string systemName;
    std::string message;
};

/// Result of a full simulation state audit.
struct AuditResult {
    bool passed = false;
    std::vector<AuditFinding> findings;
    uint32_t systemsAudited = 0;
    uint32_t stateBlocksVerified = 0;
    uint32_t errorsFound = 0;
    uint32_t warningsFound = 0;
};

/// Audits simulation systems for hidden (undeclared) state.
class SimulationStateAuditor {
public:
    /// Register a simulation system that must declare its state.
    void RegisterSystem(const std::string& name);

    /// Declare that a system owns a named state block.
    void DeclareState(const std::string& systemName,
                      const std::string& blockName,
                      StateCategory category = StateCategory::Simulated);

    /// Returns the list of registered system names.
    const std::vector<std::string>& RegisteredSystems() const;

    /// Returns the number of state declarations for a given system.
    size_t DeclaredStateCount(const std::string& systemName) const;

    /// Returns the declared state block names for a system.
    std::vector<std::string> DeclaredBlocks(const std::string& systemName) const;

    /// Run a full audit.  Checks that every registered system has
    /// declared at least one state block.
    AuditResult Audit() const;

    /// Clear all registrations and declarations.
    void Clear();

private:
    struct StateDeclaration {
        std::string systemName;
        std::string blockName;
        StateCategory category;
    };

    std::vector<std::string> m_systems;
    std::vector<StateDeclaration> m_declarations;
};

}  // namespace atlas::sim
