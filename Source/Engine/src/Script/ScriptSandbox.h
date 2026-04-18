#pragma once

// ============================================================
// Atlas Script Sandbox
// ============================================================
// Provides deterministic-safe built-in functions for the VM.
// No IO, no system calls, no OS randomness.
// ============================================================

#include "ScriptVM.h"

#include <string>
#include <vector>

namespace atlas::script {

/// Registers deterministic-safe built-in functions into a ScriptVM.
class ScriptSandbox {
public:
    /// Register all safe built-in functions into the given VM.
    static void RegisterBuiltins(ScriptVM& vm);

    /// Return the names of all registered builtins.
    static std::vector<std::string> BuiltinNames();

    /// Absolute value (int or float).
    static ScriptValue Fn_atlas_abs(const std::vector<ScriptValue>& args);

    /// Minimum of two values.
    static ScriptValue Fn_atlas_min(const std::vector<ScriptValue>& args);

    /// Maximum of two values.
    static ScriptValue Fn_atlas_max(const std::vector<ScriptValue>& args);

    /// Clamp a value between min and max.
    static ScriptValue Fn_atlas_clamp(const std::vector<ScriptValue>& args);

    /// Floor of a float value (returns Int).
    static ScriptValue Fn_atlas_floor(const std::vector<ScriptValue>& args);

    /// Ceiling of a float value (returns Int).
    static ScriptValue Fn_atlas_ceil(const std::vector<ScriptValue>& args);

    /// Square root (returns Float).
    static ScriptValue Fn_atlas_sqrt(const std::vector<ScriptValue>& args);

    /// String length (returns Int).
    static ScriptValue Fn_atlas_strlen(const std::vector<ScriptValue>& args);
};

}  // namespace atlas::script
