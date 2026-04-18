#pragma once

// ============================================================
// Atlas Script System
// ============================================================
// Manages script registration, contract validation, and
// per-tick execution within the deterministic simulation loop.
// ============================================================

#include "ScriptVM.h"

#include <cstdint>
#include <string>
#include <vector>

namespace atlas::script {

// ---- Contract -----------------------------------------------

/// Contract declaration for a script (mirrors ATLAS_SYSTEM_TRAITS).
struct ScriptContract {
    std::string scriptName;
    bool deterministic  = false;
    bool replaySafe     = false;
    bool migrationSafe  = false;
};

// ---- ScriptSystem -------------------------------------------

/// Manages script execution within the simulation tick.
class ScriptSystem {
public:
    /// Register a compiled script for execution.
    void RegisterScript(const CompiledScript& script);

    /// Unregister a script by name.
    void UnregisterScript(const std::string& name);

    /// Execute all registered scripts for the current tick.
    /// Scripts are executed in registration order (deterministic).
    void ExecuteTick(uint64_t tick, uint64_t seed);

    /// Get the underlying VM (for variable / function setup).
    ScriptVM&       GetVM();
    const ScriptVM& GetVM() const;

    /// Validate that all registered scripts meet contract requirements.
    bool ValidateContracts() const;

    /// Get validation error messages from the last ValidateContracts call.
    const std::vector<std::string>& ValidationErrors() const;

    /// Get the names of all registered scripts in execution order.
    std::vector<std::string> RegisteredScripts() const;

    /// Get the number of registered scripts.
    size_t ScriptCount() const;

    /// Total instruction steps executed across all scripts in the last tick.
    uint64_t TotalStepsThisTick() const;

    /// Compute a combined deterministic hash of all script states.
    uint64_t CombinedHash() const;

private:
    ScriptVM                   m_vm;
    std::vector<CompiledScript> m_scripts;
    mutable std::vector<std::string> m_validationErrors;
    uint64_t                   m_totalSteps = 0;
};

}  // namespace atlas::script
