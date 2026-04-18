#pragma once
// NF::Workspace — Phase 53: Workspace Project Template
//
// Project template catalogue and instantiation contract.
// UI-agnostic: holds metadata, categories, file stubs, and variable
// substitution rules. The actual file-system write is performed by the
// caller after calling instantiate().
//
//   TemplateCategory     — id + label + description; isValid()
//   TemplateFileStub     — relative path + content template (may contain
//                          {{VAR}} placeholders); isValid()
//   TemplateVariable     — key + defaultValue + description; required flag
//   TemplateDefinition   — id + name + categoryId + description + version +
//                          stubs (MAX_STUBS=64) + variables (MAX_VARS=32);
//                          isValid()
//   TemplateInstance     — resolved variable map + resolvedFiles (path→content);
//                          isComplete() (all required vars present)
//   TemplateRegistry     — category + template store; addCategory/removeCategory;
//                          addTemplate/removeTemplate/findTemplate/findByCategory;
//                          instantiate(templateId, vars) → TemplateInstance;
//                          observer callbacks on mutation (MAX_OBSERVERS=16)

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// TemplateCategory
// ═════════════════════════════════════════════════════════════════════════

struct TemplateCategory {
    std::string id;
    std::string label;
    std::string description;

    [[nodiscard]] bool isValid() const { return !id.empty() && !label.empty(); }
};

// ═════════════════════════════════════════════════════════════════════════
// TemplateFileStub
// ═════════════════════════════════════════════════════════════════════════

struct TemplateFileStub {
    std::string relativePath;   // e.g. "src/main.cpp"
    std::string contentTemplate; // text, may contain {{VARIABLE_NAME}}

    [[nodiscard]] bool isValid() const { return !relativePath.empty(); }
};

// ═════════════════════════════════════════════════════════════════════════
// TemplateVariable
// ═════════════════════════════════════════════════════════════════════════

struct TemplateVariable {
    std::string key;
    std::string defaultValue;
    std::string description;
    bool        required = false;

    [[nodiscard]] bool isValid() const { return !key.empty(); }
};

// ═════════════════════════════════════════════════════════════════════════
// TemplateDefinition
// ═════════════════════════════════════════════════════════════════════════

class TemplateDefinition {
public:
    static constexpr size_t MAX_STUBS = 64;
    static constexpr size_t MAX_VARS  = 32;

    std::string id;
    std::string name;
    std::string categoryId;
    std::string description;
    std::string version;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    // ── Stubs ─────────────────────────────────────────────────────

    bool addStub(const TemplateFileStub& stub) {
        if (!stub.isValid()) return false;
        if (m_stubs.size() >= MAX_STUBS) return false;
        for (const auto& s : m_stubs)
            if (s.relativePath == stub.relativePath) return false;
        m_stubs.push_back(stub);
        return true;
    }

    bool removeStub(const std::string& path) {
        auto it = std::find_if(m_stubs.begin(), m_stubs.end(),
                               [&](const TemplateFileStub& s){
                                   return s.relativePath == path;
                               });
        if (it == m_stubs.end()) return false;
        m_stubs.erase(it);
        return true;
    }

    [[nodiscard]] const TemplateFileStub* findStub(const std::string& path) const {
        for (const auto& s : m_stubs)
            if (s.relativePath == path) return &s;
        return nullptr;
    }

    [[nodiscard]] const std::vector<TemplateFileStub>& stubs() const { return m_stubs; }
    [[nodiscard]] size_t stubCount() const { return m_stubs.size(); }

    // ── Variables ─────────────────────────────────────────────────

    bool addVariable(const TemplateVariable& var) {
        if (!var.isValid()) return false;
        if (m_vars.size() >= MAX_VARS) return false;
        for (const auto& v : m_vars)
            if (v.key == var.key) return false;
        m_vars.push_back(var);
        return true;
    }

    bool removeVariable(const std::string& key) {
        auto it = std::find_if(m_vars.begin(), m_vars.end(),
                               [&](const TemplateVariable& v){ return v.key == key; });
        if (it == m_vars.end()) return false;
        m_vars.erase(it);
        return true;
    }

    [[nodiscard]] const TemplateVariable* findVariable(const std::string& key) const {
        for (const auto& v : m_vars)
            if (v.key == key) return &v;
        return nullptr;
    }

    [[nodiscard]] const std::vector<TemplateVariable>& variables() const { return m_vars; }
    [[nodiscard]] size_t variableCount() const { return m_vars.size(); }

    // ── Variable substitution ─────────────────────────────────────

    // Replace all {{KEY}} occurrences in text with the matching value from vars.
    // Falls back to variable defaultValue if key not in vars.
    [[nodiscard]] std::string substitute(
            const std::string& text,
            const std::map<std::string, std::string>& vars) const {
        std::string out = text;
        for (const auto& v : m_vars) {
            std::string token = "{{" + v.key + "}}";
            auto it = vars.find(v.key);
            std::string value = (it != vars.end()) ? it->second : v.defaultValue;
            size_t pos = 0;
            while ((pos = out.find(token, pos)) != std::string::npos) {
                out.replace(pos, token.size(), value);
                pos += value.size();
            }
        }
        return out;
    }

private:
    std::vector<TemplateFileStub>  m_stubs;
    std::vector<TemplateVariable>  m_vars;
};

// ═════════════════════════════════════════════════════════════════════════
// TemplateInstance — result of instantiate()
// ═════════════════════════════════════════════════════════════════════════

struct TemplateInstance {
    std::string                        templateId;
    std::map<std::string, std::string> variables;      // resolved variable values
    std::map<std::string, std::string> resolvedFiles;  // path → content

    // Returns false if any required variable was not supplied (empty value).
    [[nodiscard]] bool isComplete() const { return m_missingRequired.empty(); }
    [[nodiscard]] const std::vector<std::string>& missingRequired() const {
        return m_missingRequired;
    }

    std::vector<std::string> m_missingRequired;
};

// ═════════════════════════════════════════════════════════════════════════
// TemplateRegistry
// ═════════════════════════════════════════════════════════════════════════

using TemplateObserver = std::function<void()>;

class TemplateRegistry {
public:
    static constexpr size_t MAX_CATEGORIES = 32;
    static constexpr size_t MAX_TEMPLATES  = 256;
    static constexpr size_t MAX_OBSERVERS  = 16;

    // ── Category management ───────────────────────────────────────

    bool addCategory(const TemplateCategory& cat) {
        if (!cat.isValid()) return false;
        if (m_categories.size() >= MAX_CATEGORIES) return false;
        for (const auto& c : m_categories)
            if (c.id == cat.id) return false;
        m_categories.push_back(cat);
        notify();
        return true;
    }

    bool removeCategory(const std::string& id) {
        auto it = std::find_if(m_categories.begin(), m_categories.end(),
                               [&](const TemplateCategory& c){ return c.id == id; });
        if (it == m_categories.end()) return false;
        m_categories.erase(it);
        notify();
        return true;
    }

    [[nodiscard]] const TemplateCategory* findCategory(const std::string& id) const {
        for (const auto& c : m_categories)
            if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool   hasCategory(const std::string& id) const {
        return findCategory(id) != nullptr;
    }
    [[nodiscard]] size_t categoryCount()  const { return m_categories.size(); }
    [[nodiscard]] const std::vector<TemplateCategory>& categories() const {
        return m_categories;
    }

    // ── Template management ───────────────────────────────────────

    bool addTemplate(const TemplateDefinition& tmpl) {
        if (!tmpl.isValid()) return false;
        if (m_templates.size() >= MAX_TEMPLATES) return false;
        for (const auto& t : m_templates)
            if (t.id == tmpl.id) return false;
        m_templates.push_back(tmpl);
        notify();
        return true;
    }

    bool removeTemplate(const std::string& id) {
        auto it = std::find_if(m_templates.begin(), m_templates.end(),
                               [&](const TemplateDefinition& t){ return t.id == id; });
        if (it == m_templates.end()) return false;
        m_templates.erase(it);
        notify();
        return true;
    }

    [[nodiscard]] const TemplateDefinition* findTemplate(const std::string& id) const {
        for (const auto& t : m_templates)
            if (t.id == id) return &t;
        return nullptr;
    }

    [[nodiscard]] bool hasTemplate(const std::string& id) const {
        return findTemplate(id) != nullptr;
    }

    // All templates in a given category.
    [[nodiscard]] std::vector<const TemplateDefinition*>
    findByCategory(const std::string& categoryId) const {
        std::vector<const TemplateDefinition*> out;
        for (const auto& t : m_templates)
            if (t.categoryId == categoryId) out.push_back(&t);
        return out;
    }

    [[nodiscard]] size_t templateCount() const { return m_templates.size(); }

    // ── Instantiation ─────────────────────────────────────────────

    // Resolve template variables using supplied vars, substitute in stubs.
    // On success, result.isComplete()==true. Missing required vars are listed.
    [[nodiscard]] TemplateInstance instantiate(
            const std::string& templateId,
            const std::map<std::string, std::string>& vars = {}) const {
        TemplateInstance inst;
        inst.templateId = templateId;

        const auto* tmpl = findTemplate(templateId);
        if (!tmpl) return inst;

        // Resolve effective variable values (supplied → default)
        std::map<std::string, std::string> resolved = vars;
        for (const auto& v : tmpl->variables()) {
            if (resolved.find(v.key) == resolved.end())
                resolved[v.key] = v.defaultValue;
            // Check required
            if (v.required && resolved[v.key].empty())
                inst.m_missingRequired.push_back(v.key);
        }
        inst.variables = resolved;

        // Resolve each file stub
        for (const auto& stub : tmpl->stubs()) {
            std::string content = tmpl->substitute(stub.contentTemplate, resolved);
            inst.resolvedFiles[stub.relativePath] = content;
        }

        return inst;
    }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(TemplateObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    void clear() {
        m_categories.clear();
        m_templates.clear();
        notify();
    }

private:
    void notify() {
        for (auto& obs : m_observers) obs();
    }

    std::vector<TemplateCategory>   m_categories;
    std::vector<TemplateDefinition> m_templates;
    std::vector<TemplateObserver>   m_observers;
};

} // namespace NF
