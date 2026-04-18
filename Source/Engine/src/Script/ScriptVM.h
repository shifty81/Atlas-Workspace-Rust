#pragma once

// ============================================================
// Atlas Deterministic Script VM
// ============================================================
// A self-contained, sandboxed, stack-based bytecode
// VM.  No JIT, no native extensions, no wall-clock time, no OS
// randomness.  Every execution path is fully deterministic and
// replay-safe.
// ============================================================

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::script {

// ---- Opcodes ------------------------------------------------

enum class Opcode : uint8_t {
    PUSH_INT,    // Push constants[operand] (Int) onto stack
    PUSH_FLOAT,  // Push constants[operand] (Float) onto stack
    PUSH_STRING, // Push constants[operand] (String) onto stack

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,

    EQ,
    NEQ,
    LT,
    GT,
    LTE,
    GTE,

    AND,
    OR,
    NOT,

    LOAD_VAR,    // Push variable whose name is constants[operand]
    STORE_VAR,   // Pop value, store into variable named constants[operand]

    CALL,        // Call function named constants[operand]; next byte = arg count
    JMP,         // Unconditional jump to operand (absolute pc)
    JMP_IF,      // Pop top; jump to operand if truthy

    RET,
    HALT
};

// ---- Instruction --------------------------------------------

struct Instruction {
    Opcode   op;
    int32_t  operand = 0;  // index into constant pool, variable table, or jump target
};

// ---- Script value -------------------------------------------

/// A value manipulated by the VM.
struct ScriptValue {
    enum class Type : uint8_t { None, Int, Float, Bool, String };

    Type        type     = Type::None;
    int64_t     intVal   = 0;
    double      floatVal = 0.0;
    std::string strVal;

    ScriptValue() = default;
    explicit ScriptValue(int64_t v)            : type(Type::Int),    intVal(v) {}
    explicit ScriptValue(double v)             : type(Type::Float),  floatVal(v) {}
    explicit ScriptValue(bool v)               : type(Type::Bool),   intVal(v ? 1 : 0) {}
    explicit ScriptValue(const std::string& v) : type(Type::String), strVal(v) {}
    explicit ScriptValue(std::string&& v)      : type(Type::String), strVal(std::move(v)) {}

    bool operator==(const ScriptValue& o) const {
        if (type != o.type) return false;
        switch (type) {
            case Type::None:   return true;
            case Type::Int:    return intVal == o.intVal;
            case Type::Float:  return floatVal == o.floatVal;
            case Type::Bool:   return intVal == o.intVal;
            case Type::String: return strVal == o.strVal;
        }
        return false;
    }

    bool operator!=(const ScriptValue& o) const { return !(*this == o); }

    /// Interpret value as a boolean for conditional logic.
    bool IsTruthy() const {
        switch (type) {
            case Type::None:   return false;
            case Type::Int:    return intVal != 0;
            case Type::Float:  return floatVal != 0.0;
            case Type::Bool:   return intVal != 0;
            case Type::String: return !strVal.empty();
        }
        return false;
    }
};

// ---- Compiled script ----------------------------------------

/// A compiled script ready for deterministic execution.
struct CompiledScript {
    std::string              name;
    uint32_t                 version              = 1;
    std::vector<Instruction> code;
    std::vector<ScriptValue> constants;
    uint64_t                 sourceHash           = 0;
    bool                     deterministicDeclared = false;
    bool                     replaySafe           = false;
    bool                     migrationSafe        = false;
};

// ---- VM -----------------------------------------------------

/// Deterministic stack-based script VM.
class ScriptVM {
public:
    ScriptVM();

    /// Execute a compiled script and return the result value.
    ScriptValue Execute(const CompiledScript& script);

    /// Set a named variable accessible to scripts.
    void SetVariable(const std::string& name, ScriptValue value);

    /// Get a named variable from the VM.
    ScriptValue GetVariable(const std::string& name) const;

    /// Native function signature.
    using NativeFunction = std::function<ScriptValue(const std::vector<ScriptValue>&)>;

    /// Register a native function callable from scripts.
    void RegisterFunction(const std::string& name, NativeFunction fn);

    /// Reset all VM state (stack, variables, counters).
    void Reset();

    /// Get the number of instructions executed in the last Execute call.
    uint64_t StepCount() const;

    /// Set the maximum number of steps before execution is halted (0 = unlimited).
    void SetMaxSteps(uint64_t maxSteps);

    /// Whether the last execution was halted because the step budget was exceeded.
    bool WasBudgetExceeded() const;

    /// Compute a deterministic hash of the current VM state.
    uint64_t StateHash() const;

private:
    std::vector<ScriptValue>                          m_stack;
    std::unordered_map<std::string, ScriptValue>      m_variables;
    std::unordered_map<std::string, NativeFunction>   m_functions;
    uint64_t                                          m_stepCount      = 0;
    uint64_t                                          m_maxSteps       = 0;
    bool                                              m_budgetExceeded = false;
};

}  // namespace atlas::script
