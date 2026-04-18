#pragma once
// NF::Workspace — Phase 62: Workspace Variable Store
//
// Typed key-value variable storage with scopes, observers,
// and serialization.
//
//   VariableType   — String / Int / Float / Bool / Custom
//   Variable       — key + type + value + scope + readOnly; isValid()
//   VariableScope  — named scope with variables (MAX_VARS=512)
//   VariableStore  — scope registry (MAX_SCOPES=64); add/remove/find scopes;
//                    set/get shortcuts; observer callbacks; serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// VariableType
// ═════════════════════════════════════════════════════════════════

enum class VariableType : uint8_t {
    String = 0,
    Int    = 1,
    Float  = 2,
    Bool   = 3,
    Custom = 4,
};

inline const char* variableTypeName(VariableType t) {
    switch (t) {
        case VariableType::String: return "String";
        case VariableType::Int:    return "Int";
        case VariableType::Float:  return "Float";
        case VariableType::Bool:   return "Bool";
        case VariableType::Custom: return "Custom";
        default:                   return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// Variable
// ═════════════════════════════════════════════════════════════════

struct Variable {
    std::string  key;
    VariableType type     = VariableType::String;
    std::string  value;
    std::string  scope;
    bool         readOnly = false;
    std::string  description;

    [[nodiscard]] bool isValid() const { return !key.empty(); }

    bool operator==(const Variable& o) const { return key == o.key && scope == o.scope; }
    bool operator!=(const Variable& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// VariableScope
// ═════════════════════════════════════════════════════════════════

class VariableScope {
public:
    static constexpr int MAX_VARS = 512;

    std::string id;
    std::string name;
    bool        persistent = true;  // whether this scope survives session restart

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    bool set(const Variable& var) {
        if (!var.isValid()) return false;
        auto* existing = findMut(var.key);
        if (existing) {
            if (existing->readOnly) return false;
            existing->value = var.value;
            return true;
        }
        if (static_cast<int>(m_vars.size()) >= MAX_VARS) return false;
        m_vars.push_back(var);
        return true;
    }

    bool setReadOnly(const std::string& key, bool readOnly) {
        auto* v = findMut(key);
        if (!v) return false;
        v->readOnly = readOnly;
        return true;
    }

    bool remove(const std::string& key) {
        auto it = std::find_if(m_vars.begin(), m_vars.end(),
                               [&](const Variable& v) { return v.key == key; });
        if (it == m_vars.end()) return false;
        if (it->readOnly) return false;
        m_vars.erase(it);
        return true;
    }

    const Variable* find(const std::string& key) const {
        auto it = std::find_if(m_vars.begin(), m_vars.end(),
                               [&](const Variable& v) { return v.key == key; });
        return it != m_vars.end() ? &(*it) : nullptr;
    }

    Variable* findMut(const std::string& key) {
        auto it = std::find_if(m_vars.begin(), m_vars.end(),
                               [&](const Variable& v) { return v.key == key; });
        return it != m_vars.end() ? &(*it) : nullptr;
    }

    bool contains(const std::string& key) const { return find(key) != nullptr; }

    [[nodiscard]] const std::string* get(const std::string& key) const {
        const auto* v = find(key);
        return v ? &v->value : nullptr;
    }

    [[nodiscard]] std::vector<const Variable*> filterByType(VariableType type) const {
        std::vector<const Variable*> result;
        for (const auto& v : m_vars) {
            if (v.type == type) result.push_back(&v);
        }
        return result;
    }

    [[nodiscard]] int                          count()     const { return static_cast<int>(m_vars.size()); }
    [[nodiscard]] bool                         empty()     const { return m_vars.empty(); }
    [[nodiscard]] const std::vector<Variable>& variables() const { return m_vars; }

    void clear() { m_vars.clear(); }

private:
    std::vector<Variable> m_vars;
};

// ═════════════════════════════════════════════════════════════════
// VariableStore
// ═════════════════════════════════════════════════════════════════

class VariableStore {
public:
    using VarObserver = std::function<void(const std::string& scopeId,
                                           const std::string& key,
                                           const std::string& oldVal,
                                           const std::string& newVal)>;

    static constexpr int MAX_SCOPES    = 64;
    static constexpr int MAX_OBSERVERS = 16;

    // Scope management ─────────────────────────────────────────

    bool addScope(const VariableScope& scope) {
        if (!scope.isValid()) return false;
        if (findScope(scope.id)) return false;
        if (static_cast<int>(m_scopes.size()) >= MAX_SCOPES) return false;
        m_scopes.push_back(scope);
        return true;
    }

    bool removeScope(const std::string& scopeId) {
        auto it = findScopeIt(scopeId);
        if (it == m_scopes.end()) return false;
        m_scopes.erase(it);
        return true;
    }

    VariableScope* findScope(const std::string& scopeId) {
        auto it = findScopeIt(scopeId);
        return it != m_scopes.end() ? &(*it) : nullptr;
    }

    const VariableScope* findScope(const std::string& scopeId) const {
        auto it = std::find_if(m_scopes.begin(), m_scopes.end(),
                               [&](const VariableScope& s) { return s.id == scopeId; });
        return it != m_scopes.end() ? &(*it) : nullptr;
    }

    bool hasScope(const std::string& scopeId) const { return findScope(scopeId) != nullptr; }

    [[nodiscard]] int scopeCount() const { return static_cast<int>(m_scopes.size()); }

    [[nodiscard]] const std::vector<VariableScope>& scopes() const { return m_scopes; }

    // Variable shortcuts ───────────────────────────────────────

    bool set(const std::string& scopeId, const Variable& var) {
        auto* scope = findScope(scopeId);
        if (!scope) return false;
        std::string oldVal;
        const auto* existing = scope->find(var.key);
        if (existing) {
            if (existing->readOnly) return false;
            oldVal = existing->value;
        }
        if (!scope->set(var)) return false;
        notifyObservers(scopeId, var.key, oldVal, var.value);
        return true;
    }

    const std::string* get(const std::string& scopeId, const std::string& key) const {
        const auto* scope = findScope(scopeId);
        if (!scope) return nullptr;
        return scope->get(key);
    }

    bool remove(const std::string& scopeId, const std::string& key) {
        auto* scope = findScope(scopeId);
        if (!scope) return false;
        return scope->remove(key);
    }

    bool contains(const std::string& scopeId, const std::string& key) const {
        const auto* scope = findScope(scopeId);
        if (!scope) return false;
        return scope->contains(key);
    }

    // Search ───────────────────────────────────────────────────

    [[nodiscard]] std::vector<const Variable*> searchByKey(const std::string& query) const {
        std::vector<const Variable*> results;
        if (query.empty()) return results;
        std::string lq = toLower(query);
        for (const auto& scope : m_scopes) {
            for (const auto& var : scope.variables()) {
                if (toLower(var.key).find(lq) != std::string::npos) {
                    results.push_back(&var);
                }
            }
        }
        return results;
    }

    [[nodiscard]] int totalVariables() const {
        int n = 0;
        for (const auto& s : m_scopes) n += s.count();
        return n;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(VarObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization ────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& scope : m_scopes) {
            out += "[" + esc(scope.id) + "|" + esc(scope.name) + "|"
                 + (scope.persistent ? "1" : "0") + "]\n";
            for (const auto& var : scope.variables()) {
                out += esc(var.key) + "|"
                     + std::to_string(static_cast<int>(var.type)) + "|"
                     + esc(var.value) + "|"
                     + (var.readOnly ? "1" : "0") + "|"
                     + esc(var.description) + "\n";
            }
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_scopes.clear();
        if (text.empty()) return true;

        VariableScope* current = nullptr;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                std::string inner = line.substr(1, line.size() - 2);
                auto fields = splitPipe(inner);
                if (fields.size() < 3) continue;
                VariableScope scope;
                scope.id         = unesc(fields[0]);
                scope.name       = unesc(fields[1]);
                scope.persistent = (fields[2] == "1");
                m_scopes.push_back(std::move(scope));
                current = &m_scopes.back();
            } else if (current) {
                auto fields = splitPipe(line);
                if (fields.size() < 5) continue;
                Variable var;
                var.key         = unesc(fields[0]);
                var.type        = static_cast<VariableType>(std::stoi(fields[1]));
                var.value       = unesc(fields[2]);
                var.readOnly    = (fields[3] == "1");
                var.description = unesc(fields[4]);
                current->set(var);
            }
        }
        return true;
    }

    void clear() {
        m_scopes.clear();
        m_observers.clear();
    }

private:
    std::vector<VariableScope> m_scopes;
    std::vector<VarObserver>   m_observers;

    std::vector<VariableScope>::iterator findScopeIt(const std::string& id) {
        return std::find_if(m_scopes.begin(), m_scopes.end(),
                            [&](const VariableScope& s) { return s.id == id; });
    }

    void notifyObservers(const std::string& scopeId, const std::string& key,
                         const std::string& oldVal, const std::string& newVal) {
        for (auto& cb : m_observers) if (cb) cb(scopeId, key, oldVal, newVal);
    }

    static std::string esc(const std::string& s) {
        std::string out; out.reserve(s.size());
        for (char c : s) {
            if (c == '|')       out += "\\P";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\N";
            else                out += c;
        }
        return out;
    }

    static std::string unesc(const std::string& s) {
        std::string out; out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if      (s[i+1] == 'P')  { out += '|';  ++i; }
                else if (s[i+1] == '\\') { out += '\\'; ++i; }
                else if (s[i+1] == 'N')  { out += '\n'; ++i; }
                else out += s[i];
            } else { out += s[i]; }
        }
        return out;
    }

    static size_t findPipe(const std::string& s, size_t start) {
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) { ++i; continue; }
            if (s[i] == '|') return i;
        }
        return std::string::npos;
    }

    static std::vector<std::string> splitPipe(const std::string& s) {
        std::vector<std::string> fields;
        size_t start = 0;
        while (start <= s.size()) {
            auto p = findPipe(s, start);
            if (p == std::string::npos) { fields.push_back(s.substr(start)); break; }
            fields.push_back(s.substr(start, p - start));
            start = p + 1;
        }
        return fields;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
};

} // namespace NF
