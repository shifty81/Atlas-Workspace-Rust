#pragma once
// NF::Workspace — Phase 16: Workspace Scripting and Automation
//
// Workspace-level scripting and automation infrastructure:
//   ScriptParamType     — parameter type classification
//   ScriptParam         — typed parameter descriptor with default value
//   ScriptBinding       — typed script function binding with parameters
//   ScriptContext       — execution environment with variable scope
//   ScriptEngine        — register/execute script bindings with error handling
//   AutomationStepStatus — step execution result classification
//   AutomationStep      — named automation step with handler
//   AutomationTask      — named automation sequence with step execution

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// ScriptParamType — parameter type classification
// ═════════════════════════════════════════════════════════════════

enum class ScriptParamType : uint8_t {
    Void    = 0,   // No value
    Bool    = 1,   // Boolean
    Int     = 2,   // Integer
    Float   = 3,   // Floating point
    String  = 4,   // Text string
    Path    = 5,   // File system path
    Id      = 6,   // Identifier / reference
    Custom  = 7,   // User-defined
};

inline const char* scriptParamTypeName(ScriptParamType t) {
    switch (t) {
        case ScriptParamType::Void:   return "Void";
        case ScriptParamType::Bool:   return "Bool";
        case ScriptParamType::Int:    return "Int";
        case ScriptParamType::Float:  return "Float";
        case ScriptParamType::String: return "String";
        case ScriptParamType::Path:   return "Path";
        case ScriptParamType::Id:     return "Id";
        case ScriptParamType::Custom: return "Custom";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ScriptParam — typed parameter descriptor with default value
// ═════════════════════════════════════════════════════════════════

struct ScriptParam {
    std::string     name;
    ScriptParamType type         = ScriptParamType::Void;
    std::string     defaultValue;        // Optional default as string
    bool            required     = true; // If false, defaultValue is used when absent

    bool isValid() const { return !name.empty() && type != ScriptParamType::Void; }

    bool operator==(const ScriptParam& o) const { return name == o.name && type == o.type; }
    bool operator!=(const ScriptParam& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// ScriptBinding — typed script function binding
// ═════════════════════════════════════════════════════════════════

class ScriptBinding {
public:
    using Handler = std::function<bool(const std::vector<std::string>& args)>;

    ScriptBinding() = default;

    ScriptBinding(const std::string& name, const std::string& description)
        : name_(name), description_(description) {}

    // ── Identity ─────────────────────────────────────────────────
    const std::string& name()        const { return name_; }
    const std::string& description() const { return description_; }
    bool isValid() const { return !name_.empty() && handler_ != nullptr; }

    // ── Parameters ───────────────────────────────────────────────
    bool addParam(const ScriptParam& param) {
        if (!param.isValid()) return false;
        for (auto& p : params_) {
            if (p.name == param.name) return false;  // duplicate
        }
        params_.push_back(param);
        return true;
    }

    size_t paramCount() const { return params_.size(); }

    const ScriptParam* findParam(const std::string& name) const {
        for (auto& p : params_) {
            if (p.name == name) return &p;
        }
        return nullptr;
    }

    const std::vector<ScriptParam>& params() const { return params_; }

    size_t requiredParamCount() const {
        size_t n = 0;
        for (auto& p : params_) { if (p.required) ++n; }
        return n;
    }

    // ── Handler ──────────────────────────────────────────────────
    void setHandler(Handler h) { handler_ = std::move(h); }

    bool invoke(const std::vector<std::string>& args) const {
        if (!handler_) return false;
        return handler_(args);
    }

    // ── Return type ──────────────────────────────────────────────
    void setReturnType(ScriptParamType t) { returnType_ = t; }
    ScriptParamType returnType() const { return returnType_; }

private:
    std::string              name_;
    std::string              description_;
    std::vector<ScriptParam> params_;
    Handler                  handler_;
    ScriptParamType          returnType_ = ScriptParamType::Void;
};

// ═════════════════════════════════════════════════════════════════
// ScriptContext — execution environment with variable scope
// ═════════════════════════════════════════════════════════════════

class ScriptContext {
public:
    static constexpr size_t MAX_VARIABLES = 512;

    // ── Variable scope ───────────────────────────────────────────
    bool setVariable(const std::string& key, const std::string& value) {
        if (key.empty()) return false;
        for (auto& v : variables_) {
            if (v.first == key) { v.second = value; return true; }
        }
        if (variables_.size() >= MAX_VARIABLES) return false;
        variables_.push_back({key, value});
        return true;
    }

    std::string getVariable(const std::string& key) const {
        for (auto& v : variables_) {
            if (v.first == key) return v.second;
        }
        return {};
    }

    std::string getVariableOr(const std::string& key, const std::string& fallback) const {
        for (auto& v : variables_) {
            if (v.first == key) return v.second;
        }
        return fallback;
    }

    bool hasVariable(const std::string& key) const {
        for (auto& v : variables_) {
            if (v.first == key) return true;
        }
        return false;
    }

    bool removeVariable(const std::string& key) {
        for (auto it = variables_.begin(); it != variables_.end(); ++it) {
            if (it->first == key) { variables_.erase(it); return true; }
        }
        return false;
    }

    size_t variableCount() const { return variables_.size(); }

    void clearVariables() { variables_.clear(); }

    // ── Output capture ───────────────────────────────────────────
    void appendOutput(const std::string& text) { output_ += text; }
    const std::string& output() const { return output_; }
    void clearOutput() { output_.clear(); }

    // ── Error state ──────────────────────────────────────────────
    void setError(const std::string& err) { error_ = err; hasError_ = true; }
    const std::string& error() const { return error_; }
    bool hasError() const { return hasError_; }
    void clearError() { error_.clear(); hasError_ = false; }

    // ── Full reset ───────────────────────────────────────────────
    void reset() {
        clearVariables();
        clearOutput();
        clearError();
    }

private:
    std::vector<std::pair<std::string, std::string>> variables_;
    std::string output_;
    std::string error_;
    bool        hasError_ = false;
};

// ═════════════════════════════════════════════════════════════════
// ScriptExecResult — result of script execution
// ═════════════════════════════════════════════════════════════════

enum class ScriptExecStatus : uint8_t {
    Success          = 0,
    NotFound         = 1,
    InvalidArgs      = 2,
    HandlerFailed    = 3,
    BindingInvalid   = 4,
};

inline const char* scriptExecStatusName(ScriptExecStatus s) {
    switch (s) {
        case ScriptExecStatus::Success:        return "Success";
        case ScriptExecStatus::NotFound:       return "NotFound";
        case ScriptExecStatus::InvalidArgs:    return "InvalidArgs";
        case ScriptExecStatus::HandlerFailed:  return "HandlerFailed";
        case ScriptExecStatus::BindingInvalid: return "BindingInvalid";
        default:                               return "Unknown";
    }
}

struct ScriptExecResult {
    ScriptExecStatus status    = ScriptExecStatus::NotFound;
    std::string      bindingId;
    std::string      errorMessage;

    bool succeeded() const { return status == ScriptExecStatus::Success; }
    bool failed()    const { return !succeeded(); }

    static ScriptExecResult ok(const std::string& id) {
        return {ScriptExecStatus::Success, id, {}};
    }
    static ScriptExecResult fail(ScriptExecStatus s, const std::string& id,
                                  const std::string& msg = {}) {
        return {s, id, msg};
    }
};

// ═════════════════════════════════════════════════════════════════
// ScriptEngine — register/execute script bindings
// ═════════════════════════════════════════════════════════════════

class ScriptEngine {
public:
    static constexpr size_t MAX_BINDINGS = 1024;

    // ── Registration ─────────────────────────────────────────────
    bool registerBinding(const ScriptBinding& binding) {
        if (!binding.isValid()) return false;
        for (auto& b : bindings_) {
            if (b.name() == binding.name()) return false;
        }
        if (bindings_.size() >= MAX_BINDINGS) return false;
        bindings_.push_back(binding);
        return true;
    }

    bool unregisterBinding(const std::string& name) {
        for (auto it = bindings_.begin(); it != bindings_.end(); ++it) {
            if (it->name() == name) { bindings_.erase(it); return true; }
        }
        return false;
    }

    bool isRegistered(const std::string& name) const {
        for (auto& b : bindings_) {
            if (b.name() == name) return true;
        }
        return false;
    }

    const ScriptBinding* findBinding(const std::string& name) const {
        for (auto& b : bindings_) {
            if (b.name() == name) return &b;
        }
        return nullptr;
    }

    size_t bindingCount() const { return bindings_.size(); }

    const std::vector<ScriptBinding>& allBindings() const { return bindings_; }

    // ── Execution ────────────────────────────────────────────────
    ScriptExecResult execute(const std::string& name,
                              const std::vector<std::string>& args,
                              ScriptContext& ctx) {
        auto* binding = findBindingMut(name);
        if (!binding) {
            return ScriptExecResult::fail(ScriptExecStatus::NotFound, name, "Binding not found");
        }
        if (!binding->isValid()) {
            return ScriptExecResult::fail(ScriptExecStatus::BindingInvalid, name, "Binding has no handler");
        }

        // Validate required args
        size_t reqCount = binding->requiredParamCount();
        if (args.size() < reqCount) {
            return ScriptExecResult::fail(ScriptExecStatus::InvalidArgs, name,
                "Expected at least " + std::to_string(reqCount) + " args, got " + std::to_string(args.size()));
        }

        bool ok = binding->invoke(args);
        ++totalExecutions_;
        if (ok) {
            ++successfulExecutions_;
            return ScriptExecResult::ok(name);
        } else {
            ctx.setError("Handler returned false for '" + name + "'");
            return ScriptExecResult::fail(ScriptExecStatus::HandlerFailed, name, "Handler returned false");
        }
    }

    // ── Convenience: execute with no args ────────────────────────
    ScriptExecResult execute(const std::string& name, ScriptContext& ctx) {
        return execute(name, {}, ctx);
    }

    // ── Statistics ───────────────────────────────────────────────
    size_t totalExecutions()      const { return totalExecutions_; }
    size_t successfulExecutions() const { return successfulExecutions_; }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() {
        bindings_.clear();
        totalExecutions_      = 0;
        successfulExecutions_ = 0;
    }

private:
    ScriptBinding* findBindingMut(const std::string& name) {
        for (auto& b : bindings_) {
            if (b.name() == name) return &b;
        }
        return nullptr;
    }

    std::vector<ScriptBinding> bindings_;
    size_t totalExecutions_      = 0;
    size_t successfulExecutions_ = 0;
};

// ═════════════════════════════════════════════════════════════════
// AutomationStepStatus — step execution result
// ═════════════════════════════════════════════════════════════════

enum class AutomationStepStatus : uint8_t {
    Pending   = 0,   // Not yet executed
    Running   = 1,   // Currently executing
    Succeeded = 2,   // Completed successfully
    Failed    = 3,   // Completed with failure
    Skipped   = 4,   // Intentionally skipped
};

inline const char* automationStepStatusName(AutomationStepStatus s) {
    switch (s) {
        case AutomationStepStatus::Pending:   return "Pending";
        case AutomationStepStatus::Running:   return "Running";
        case AutomationStepStatus::Succeeded: return "Succeeded";
        case AutomationStepStatus::Failed:    return "Failed";
        case AutomationStepStatus::Skipped:   return "Skipped";
        default:                              return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// AutomationStep — named step with handler
// ═════════════════════════════════════════════════════════════════

struct AutomationStep {
    using StepHandler = std::function<bool(ScriptContext&)>;

    std::string           name;
    std::string           description;
    StepHandler           handler;
    AutomationStepStatus  status   = AutomationStepStatus::Pending;
    bool                  enabled  = true;
    std::string           errorMessage;

    bool isValid() const { return !name.empty() && handler != nullptr; }

    void reset() {
        status = AutomationStepStatus::Pending;
        errorMessage.clear();
    }
};

// ═════════════════════════════════════════════════════════════════
// AutomationTaskState — task execution state
// ═════════════════════════════════════════════════════════════════

enum class AutomationTaskState : uint8_t {
    Idle       = 0,
    Running    = 1,
    Completed  = 2,
    Failed     = 3,
    Aborted    = 4,
};

inline const char* automationTaskStateName(AutomationTaskState s) {
    switch (s) {
        case AutomationTaskState::Idle:      return "Idle";
        case AutomationTaskState::Running:   return "Running";
        case AutomationTaskState::Completed: return "Completed";
        case AutomationTaskState::Failed:    return "Failed";
        case AutomationTaskState::Aborted:   return "Aborted";
        default:                             return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// AutomationTask — named automation sequence with step execution
// ═════════════════════════════════════════════════════════════════

class AutomationTask {
public:
    static constexpr size_t MAX_STEPS = 256;

    AutomationTask() = default;
    explicit AutomationTask(const std::string& name, const std::string& description = {})
        : name_(name), description_(description) {}

    // ── Identity ─────────────────────────────────────────────────
    const std::string& name()        const { return name_; }
    const std::string& description() const { return description_; }
    bool isValid() const { return !name_.empty() && !steps_.empty(); }

    // ── Steps ────────────────────────────────────────────────────
    bool addStep(const AutomationStep& step) {
        if (!step.isValid()) return false;
        for (auto& s : steps_) {
            if (s.name == step.name) return false;
        }
        if (steps_.size() >= MAX_STEPS) return false;
        steps_.push_back(step);
        return true;
    }

    bool removeStep(const std::string& name) {
        for (auto it = steps_.begin(); it != steps_.end(); ++it) {
            if (it->name == name) { steps_.erase(it); return true; }
        }
        return false;
    }

    bool enableStep(const std::string& name, bool enabled) {
        for (auto& s : steps_) {
            if (s.name == name) { s.enabled = enabled; return true; }
        }
        return false;
    }

    size_t stepCount() const { return steps_.size(); }

    const AutomationStep* findStep(const std::string& name) const {
        for (auto& s : steps_) {
            if (s.name == name) return &s;
        }
        return nullptr;
    }

    const std::vector<AutomationStep>& steps() const { return steps_; }

    // ── Execution ────────────────────────────────────────────────
    AutomationTaskState run(ScriptContext& ctx) {
        if (steps_.empty()) return AutomationTaskState::Idle;

        state_ = AutomationTaskState::Running;
        stepsRun_       = 0;
        stepsSucceeded_ = 0;
        stepsFailed_    = 0;
        stepsSkipped_   = 0;

        for (auto& step : steps_) {
            if (!step.enabled) {
                step.status = AutomationStepStatus::Skipped;
                ++stepsSkipped_;
                continue;
            }

            step.status = AutomationStepStatus::Running;
            ++stepsRun_;

            bool ok = false;
            if (step.handler) {
                ok = step.handler(ctx);
            }

            if (ok) {
                step.status = AutomationStepStatus::Succeeded;
                ++stepsSucceeded_;
            } else {
                step.status = AutomationStepStatus::Failed;
                step.errorMessage = ctx.hasError() ? ctx.error() : "Step failed";
                ++stepsFailed_;

                if (abortOnFailure_) {
                    // Mark remaining as skipped
                    bool foundFailed = false;
                    for (auto& s : steps_) {
                        if (s.name == step.name) { foundFailed = true; continue; }
                        if (foundFailed && s.status == AutomationStepStatus::Pending) {
                            s.status = AutomationStepStatus::Skipped;
                            ++stepsSkipped_;
                        }
                    }
                    state_ = AutomationTaskState::Failed;
                    return state_;
                }
            }
        }

        state_ = (stepsFailed_ > 0) ? AutomationTaskState::Failed : AutomationTaskState::Completed;
        return state_;
    }

    void abort() {
        if (state_ == AutomationTaskState::Running) {
            state_ = AutomationTaskState::Aborted;
        }
    }

    void reset() {
        state_ = AutomationTaskState::Idle;
        stepsRun_       = 0;
        stepsSucceeded_ = 0;
        stepsFailed_    = 0;
        stepsSkipped_   = 0;
        for (auto& s : steps_) s.reset();
    }

    // ── Configuration ────────────────────────────────────────────
    void setAbortOnFailure(bool v) { abortOnFailure_ = v; }
    bool abortOnFailure() const { return abortOnFailure_; }

    // ── State ────────────────────────────────────────────────────
    AutomationTaskState state()          const { return state_; }
    size_t              stepsRun()       const { return stepsRun_; }
    size_t              stepsSucceeded() const { return stepsSucceeded_; }
    size_t              stepsFailed()    const { return stepsFailed_; }
    size_t              stepsSkipped()   const { return stepsSkipped_; }

private:
    std::string                  name_;
    std::string                  description_;
    std::vector<AutomationStep>  steps_;
    AutomationTaskState          state_           = AutomationTaskState::Idle;
    bool                         abortOnFailure_  = true;
    size_t                       stepsRun_        = 0;
    size_t                       stepsSucceeded_  = 0;
    size_t                       stepsFailed_     = 0;
    size_t                       stepsSkipped_    = 0;
};

} // namespace NF
