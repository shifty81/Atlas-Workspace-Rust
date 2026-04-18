#pragma once
// NF::Editor — Script context, console
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class ScriptLanguage : uint8_t {
    Lua        = 0,
    Python     = 1,
    JavaScript = 2,
    TypeScript = 3,
    Bash       = 4,
    Ruby       = 5,
    CSharp     = 6,
    DSL        = 7
};

inline const char* scriptLanguageName(ScriptLanguage lang) {
    switch (lang) {
        case ScriptLanguage::Lua:        return "Lua";
        case ScriptLanguage::Python:     return "Python";
        case ScriptLanguage::JavaScript: return "JavaScript";
        case ScriptLanguage::TypeScript: return "TypeScript";
        case ScriptLanguage::Bash:       return "Bash";
        case ScriptLanguage::Ruby:       return "Ruby";
        case ScriptLanguage::CSharp:     return "CSharp";
        case ScriptLanguage::DSL:        return "DSL";
        default:                         return "Unknown";
    }
}

struct ScriptVariable {
    std::string name;
    std::string value;
    std::string typeName;
    bool        readOnly = false;

    [[nodiscard]] bool isValid()    const { return !name.empty(); }
    [[nodiscard]] bool isReadOnly() const { return readOnly; }

    bool set(const std::string& newValue) {
        if (readOnly) return false;
        value = newValue;
        return true;
    }
};

struct ScriptResult {
    std::string output;
    std::string errorMessage;
    int         exitCode    = 0;
    float       durationMs  = 0.f;

    [[nodiscard]] bool isSuccess()  const { return exitCode == 0 && errorMessage.empty(); }
    [[nodiscard]] bool hasOutput()  const { return !output.empty(); }
    [[nodiscard]] bool hasError()   const { return !errorMessage.empty(); }
};

class ScriptContext {
public:
    static constexpr size_t kMaxVariables = 128;

    explicit ScriptContext(const std::string& name, ScriptLanguage lang = ScriptLanguage::Lua)
        : m_name(name), m_language(lang) {}

    [[nodiscard]] const std::string& name()     const { return m_name; }
    [[nodiscard]] ScriptLanguage     language()  const { return m_language; }

    bool setVariable(const ScriptVariable& var) {
        for (auto& v : m_variables) {
            if (v.name == var.name) {
                if (v.readOnly) return false;
                v = var;
                return true;
            }
        }
        if (m_variables.size() >= kMaxVariables) return false;
        m_variables.push_back(var);
        return true;
    }

    [[nodiscard]] ScriptVariable* getVariable(const std::string& varName) {
        for (auto& v : m_variables) { if (v.name == varName) return &v; }
        return nullptr;
    }

    bool removeVariable(const std::string& varName) {
        for (auto it = m_variables.begin(); it != m_variables.end(); ++it) {
            if (it->name == varName) { m_variables.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] bool hasVariable(const std::string& varName) const {
        for (const auto& v : m_variables) { if (v.name == varName) return true; }
        return false;
    }

    void clear() { m_variables.clear(); }

    [[nodiscard]] size_t variableCount() const { return m_variables.size(); }

private:
    std::string                m_name;
    ScriptLanguage             m_language;
    std::vector<ScriptVariable> m_variables;
};

class ScriptConsole {
public:
    static constexpr size_t kMaxContexts = 16;

    void init() { m_initialized = true; }

    void shutdown() {
        m_contexts.clear();
        m_executionCount = 0;
        m_errorCount     = 0;
        m_tickCount      = 0;
        m_initialized    = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    ScriptContext* createContext(const std::string& name, ScriptLanguage lang = ScriptLanguage::Lua) {
        if (!m_initialized) return nullptr;
        if (m_contexts.size() >= kMaxContexts) return nullptr;
        for (const auto& c : m_contexts) { if (c.name() == name) return nullptr; }
        m_contexts.emplace_back(name, lang);
        return &m_contexts.back();
    }

    [[nodiscard]] ScriptContext* contextByName(const std::string& name) {
        for (auto& c : m_contexts) { if (c.name() == name) return &c; }
        return nullptr;
    }

    ScriptResult execute(const std::string& code, ScriptContext* /*context*/ = nullptr) {
        ScriptResult result;
        if (!m_initialized) {
            result.exitCode    = -1;
            result.errorMessage = "Console not initialized";
            m_errorCount++;
            return result;
        }
        if (code.empty()) {
            result.exitCode    = -1;
            result.errorMessage = "Empty script";
            m_errorCount++;
            return result;
        }
        // Simulate successful execution
        result.output     = "ok";
        result.exitCode   = 0;
        result.durationMs = 1.f;
        m_executionCount++;
        return result;
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t totalContexts()   const { return m_contexts.size(); }
    [[nodiscard]] size_t executionCount()  const { return m_executionCount; }
    [[nodiscard]] size_t errorCount()      const { return m_errorCount; }
    [[nodiscard]] size_t tickCount()       const { return m_tickCount; }

private:
    std::vector<ScriptContext> m_contexts;
    bool   m_initialized  = false;
    size_t m_executionCount = 0;
    size_t m_errorCount     = 0;
    size_t m_tickCount      = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// S16 — Hot-Reload System
// ─────────────────────────────────────────────────────────────────────────────


} // namespace NF
