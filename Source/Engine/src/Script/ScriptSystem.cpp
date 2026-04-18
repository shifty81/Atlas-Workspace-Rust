#include "ScriptSystem.h"

#include <algorithm>

namespace atlas::script {

// ============================================================
// Registration
// ============================================================

void ScriptSystem::RegisterScript(const CompiledScript& script) {
    m_scripts.push_back(script);
}

void ScriptSystem::UnregisterScript(const std::string& name) {
    m_scripts.erase(
        std::remove_if(m_scripts.begin(), m_scripts.end(),
                       [&](const CompiledScript& s) { return s.name == name; }),
        m_scripts.end());
}

// ============================================================
// Execution
// ============================================================

void ScriptSystem::ExecuteTick(uint64_t tick, uint64_t seed) {
    m_totalSteps = 0;

    m_vm.SetVariable("atlas_tick", ScriptValue(static_cast<int64_t>(tick)));
    m_vm.SetVariable("atlas_seed", ScriptValue(static_cast<int64_t>(seed)));

    for (const auto& script : m_scripts) {
        m_vm.Execute(script);
        m_totalSteps += m_vm.StepCount();
    }
}

// ============================================================
// Accessors
// ============================================================

ScriptVM& ScriptSystem::GetVM() {
    return m_vm;
}

const ScriptVM& ScriptSystem::GetVM() const {
    return m_vm;
}

// ============================================================
// Validation
// ============================================================

bool ScriptSystem::ValidateContracts() const {
    m_validationErrors.clear();

    for (const auto& script : m_scripts) {
        if (!script.deterministicDeclared) {
            m_validationErrors.push_back(
                "Script '" + script.name + "' is not declared deterministic");
        }
        if (!script.replaySafe) {
            m_validationErrors.push_back(
                "Script '" + script.name + "' is not declared replay-safe");
        }
    }

    return m_validationErrors.empty();
}

const std::vector<std::string>& ScriptSystem::ValidationErrors() const {
    return m_validationErrors;
}

// ============================================================
// Queries
// ============================================================

std::vector<std::string> ScriptSystem::RegisteredScripts() const {
    std::vector<std::string> names;
    names.reserve(m_scripts.size());
    for (const auto& s : m_scripts)
        names.push_back(s.name);
    return names;
}

size_t ScriptSystem::ScriptCount() const {
    return m_scripts.size();
}

uint64_t ScriptSystem::TotalStepsThisTick() const {
    return m_totalSteps;
}

uint64_t ScriptSystem::CombinedHash() const {
    uint64_t h = 0;
    // Hash contribution from VM state
    h ^= m_vm.StateHash() + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    // Hash contribution from script metadata
    for (const auto& s : m_scripts) {
        uint64_t sh = s.sourceHash;
        h ^= sh + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    }
    return h;
}

}  // namespace atlas::script
