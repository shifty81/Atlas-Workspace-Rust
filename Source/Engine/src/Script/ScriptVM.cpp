#include "ScriptVM.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace atlas::script {

// ============================================================
// Helpers
// ============================================================

static uint64_t HashCombine(uint64_t seed, uint64_t value) {
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    return seed;
}

static uint64_t HashDouble(double v) {
    uint64_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    return bits;
}

static uint64_t HashString(const std::string& s) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (char c : s) {
        h ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        h *= 0x100000001b3ULL;
    }
    return h;
}

static uint64_t HashValue(const ScriptValue& v) {
    uint64_t h = static_cast<uint64_t>(v.type);
    switch (v.type) {
        case ScriptValue::Type::None:   break;
        case ScriptValue::Type::Int:    h = HashCombine(h, static_cast<uint64_t>(v.intVal)); break;
        case ScriptValue::Type::Float:  h = HashCombine(h, HashDouble(v.floatVal)); break;
        case ScriptValue::Type::Bool:   h = HashCombine(h, static_cast<uint64_t>(v.intVal)); break;
        case ScriptValue::Type::String: h = HashCombine(h, HashString(v.strVal)); break;
    }
    return h;
}

/// Convert a value to double for arithmetic.
static double AsFloat(const ScriptValue& v) {
    switch (v.type) {
        case ScriptValue::Type::Int:   return static_cast<double>(v.intVal);
        case ScriptValue::Type::Float: return v.floatVal;
        case ScriptValue::Type::Bool:  return static_cast<double>(v.intVal);
        default: return 0.0;
    }
}

/// Convert a value to int for arithmetic.
static int64_t AsInt(const ScriptValue& v) {
    switch (v.type) {
        case ScriptValue::Type::Int:   return v.intVal;
        case ScriptValue::Type::Float: return static_cast<int64_t>(v.floatVal);
        case ScriptValue::Type::Bool:  return v.intVal;
        default: return 0;
    }
}

/// True when either operand is floating point.
static bool EitherFloat(const ScriptValue& a, const ScriptValue& b) {
    return a.type == ScriptValue::Type::Float || b.type == ScriptValue::Type::Float;
}

// ============================================================
// ScriptVM
// ============================================================

ScriptVM::ScriptVM() = default;

ScriptValue ScriptVM::Execute(const CompiledScript& script) {
    m_stepCount      = 0;
    m_budgetExceeded = false;
    m_stack.clear();

    const auto& code      = script.code;
    const auto& constants = script.constants;
    const size_t codeSize = code.size();
    size_t pc = 0;

    while (pc < codeSize) {
        // Budget check
        ++m_stepCount;
        if (m_maxSteps > 0 && m_stepCount > m_maxSteps) {
            m_budgetExceeded = true;
            break;
        }

        const Instruction& inst = code[pc];

        switch (inst.op) {

        // ---- Push constants ---------------------------------
        case Opcode::PUSH_INT:
        case Opcode::PUSH_FLOAT:
        case Opcode::PUSH_STRING: {
            if (inst.operand >= 0 && static_cast<size_t>(inst.operand) < constants.size()) {
                m_stack.push_back(constants[static_cast<size_t>(inst.operand)]);
            } else {
                m_stack.emplace_back();
            }
            ++pc;
            break;
        }

        // ---- Arithmetic -------------------------------------
        case Opcode::ADD: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (a.type == ScriptValue::Type::String && b.type == ScriptValue::Type::String) {
                m_stack.emplace_back(a.strVal + b.strVal);
            } else if (EitherFloat(a, b)) {
                m_stack.emplace_back(AsFloat(a) + AsFloat(b));
            } else {
                m_stack.emplace_back(AsInt(a) + AsInt(b));
            }
            ++pc;
            break;
        }
        case Opcode::SUB: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b))
                m_stack.emplace_back(AsFloat(a) - AsFloat(b));
            else
                m_stack.emplace_back(AsInt(a) - AsInt(b));
            ++pc;
            break;
        }
        case Opcode::MUL: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b))
                m_stack.emplace_back(AsFloat(a) * AsFloat(b));
            else
                m_stack.emplace_back(AsInt(a) * AsInt(b));
            ++pc;
            break;
        }
        case Opcode::DIV: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b)) {
                double denom = AsFloat(b);
                m_stack.emplace_back(denom != 0.0 ? AsFloat(a) / denom : 0.0);
            } else {
                int64_t denom = AsInt(b);
                m_stack.emplace_back(denom != 0 ? AsInt(a) / denom : int64_t{0});
            }
            ++pc;
            break;
        }
        case Opcode::MOD: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b)) {
                double denom = AsFloat(b);
                m_stack.emplace_back(denom != 0.0 ? std::fmod(AsFloat(a), denom) : 0.0);
            } else {
                int64_t denom = AsInt(b);
                m_stack.emplace_back(denom != 0 ? AsInt(a) % denom : int64_t{0});
            }
            ++pc;
            break;
        }
        case Opcode::NEG: {
            if (m_stack.empty()) { ++pc; break; }
            ScriptValue& top = m_stack.back();
            if (top.type == ScriptValue::Type::Float)
                top.floatVal = -top.floatVal;
            else
                top.intVal = -top.intVal;
            ++pc;
            break;
        }

        // ---- Comparison -------------------------------------
        case Opcode::EQ: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            m_stack.emplace_back(a == b);
            ++pc;
            break;
        }
        case Opcode::NEQ: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            m_stack.emplace_back(a != b);
            ++pc;
            break;
        }
        case Opcode::LT: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b))
                m_stack.emplace_back(AsFloat(a) < AsFloat(b));
            else
                m_stack.emplace_back(AsInt(a) < AsInt(b));
            ++pc;
            break;
        }
        case Opcode::GT: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b))
                m_stack.emplace_back(AsFloat(a) > AsFloat(b));
            else
                m_stack.emplace_back(AsInt(a) > AsInt(b));
            ++pc;
            break;
        }
        case Opcode::LTE: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b))
                m_stack.emplace_back(AsFloat(a) <= AsFloat(b));
            else
                m_stack.emplace_back(AsInt(a) <= AsInt(b));
            ++pc;
            break;
        }
        case Opcode::GTE: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            if (EitherFloat(a, b))
                m_stack.emplace_back(AsFloat(a) >= AsFloat(b));
            else
                m_stack.emplace_back(AsInt(a) >= AsInt(b));
            ++pc;
            break;
        }

        // ---- Logical ----------------------------------------
        case Opcode::AND: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            m_stack.emplace_back(a.IsTruthy() && b.IsTruthy());
            ++pc;
            break;
        }
        case Opcode::OR: {
            if (m_stack.size() < 2) { ++pc; break; }
            ScriptValue b = m_stack.back(); m_stack.pop_back();
            ScriptValue a = m_stack.back(); m_stack.pop_back();
            m_stack.emplace_back(a.IsTruthy() || b.IsTruthy());
            ++pc;
            break;
        }
        case Opcode::NOT: {
            if (m_stack.empty()) { ++pc; break; }
            ScriptValue top = m_stack.back(); m_stack.pop_back();
            m_stack.emplace_back(!top.IsTruthy());
            ++pc;
            break;
        }

        // ---- Variables --------------------------------------
        case Opcode::LOAD_VAR: {
            if (inst.operand >= 0 && static_cast<size_t>(inst.operand) < constants.size()) {
                const std::string& varName = constants[static_cast<size_t>(inst.operand)].strVal;
                auto it = m_variables.find(varName);
                if (it != m_variables.end())
                    m_stack.push_back(it->second);
                else
                    m_stack.emplace_back();
            } else {
                m_stack.emplace_back();
            }
            ++pc;
            break;
        }
        case Opcode::STORE_VAR: {
            if (inst.operand >= 0 && static_cast<size_t>(inst.operand) < constants.size() && !m_stack.empty()) {
                const std::string& varName = constants[static_cast<size_t>(inst.operand)].strVal;
                m_variables[varName] = m_stack.back();
                m_stack.pop_back();
            }
            ++pc;
            break;
        }

        // ---- Function call ----------------------------------
        case Opcode::CALL: {
            // The function name is constants[operand].  The argument count
            // is encoded in the operand of the *next* instruction, which
            // acts as a pseudo-instruction (skipped by pc += 2).  This
            // keeps Instruction a fixed-size POD while supporting
            // variable-arity calls.
            std::string fnName;
            if (inst.operand >= 0 && static_cast<size_t>(inst.operand) < constants.size())
                fnName = constants[static_cast<size_t>(inst.operand)].strVal;

            int32_t argCount = 0;
            if (pc + 1 < codeSize)
                argCount = code[pc + 1].operand;

            std::vector<ScriptValue> args;
            for (int32_t i = 0; i < argCount && !m_stack.empty(); ++i) {
                args.push_back(m_stack.back());
                m_stack.pop_back();
            }
            // Reverse so first-pushed arg is args[0]
            std::reverse(args.begin(), args.end());

            auto it = m_functions.find(fnName);
            if (it != m_functions.end())
                m_stack.push_back(it->second(args));
            else
                m_stack.emplace_back();

            pc += 2; // skip CALL + arg-count pseudo-instruction
            break;
        }

        // ---- Control flow -----------------------------------
        case Opcode::JMP: {
            pc = static_cast<size_t>(inst.operand);
            break;
        }
        case Opcode::JMP_IF: {
            if (!m_stack.empty()) {
                ScriptValue cond = m_stack.back();
                m_stack.pop_back();
                if (cond.IsTruthy()) {
                    pc = static_cast<size_t>(inst.operand);
                    break;
                }
            }
            ++pc;
            break;
        }

        // ---- Return / Halt ----------------------------------
        case Opcode::RET:
        case Opcode::HALT: {
            if (!m_stack.empty())
                return m_stack.back();
            return ScriptValue{};
        }

        } // switch
    }

    // Fell off the end of the code
    if (!m_stack.empty())
        return m_stack.back();
    return ScriptValue{};
}

// ---- Public API ---------------------------------------------

void ScriptVM::SetVariable(const std::string& name, ScriptValue value) {
    m_variables[name] = std::move(value);
}

ScriptValue ScriptVM::GetVariable(const std::string& name) const {
    auto it = m_variables.find(name);
    if (it != m_variables.end())
        return it->second;
    return ScriptValue{};
}

void ScriptVM::RegisterFunction(const std::string& name, NativeFunction fn) {
    m_functions[name] = std::move(fn);
}

void ScriptVM::Reset() {
    m_stack.clear();
    m_variables.clear();
    m_stepCount      = 0;
    m_budgetExceeded = false;
    // Registered functions are intentionally preserved across resets.
}

uint64_t ScriptVM::StepCount() const {
    return m_stepCount;
}

void ScriptVM::SetMaxSteps(uint64_t maxSteps) {
    m_maxSteps = maxSteps;
}

bool ScriptVM::WasBudgetExceeded() const {
    return m_budgetExceeded;
}

uint64_t ScriptVM::StateHash() const {
    uint64_t h = 0;

    // Hash all variables in a deterministic (sorted) order.
    // unordered_map iteration order is non-deterministic, so we
    // collect keys, sort, and hash in order.
    std::vector<std::string> keys;
    keys.reserve(m_variables.size());
    for (const auto& [k, _] : m_variables)
        keys.push_back(k);
    std::sort(keys.begin(), keys.end());

    for (const auto& k : keys) {
        h = HashCombine(h, HashString(k));
        h = HashCombine(h, HashValue(m_variables.at(k)));
    }

    // Include stack contents
    for (const auto& v : m_stack)
        h = HashCombine(h, HashValue(v));

    return h;
}

}  // namespace atlas::script
