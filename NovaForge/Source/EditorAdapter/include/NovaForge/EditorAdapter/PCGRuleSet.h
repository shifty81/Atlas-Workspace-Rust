#pragma once
// NovaForge::PCGRuleSet — typed rule container for procedural content generation.
//
// A PCGRuleSet groups related generation rules under a named ruleset identifier.
// Rules are stored as key→value pairs (string-encoded) alongside type metadata.
// The ruleset can be loaded from project data files, edited in the PCG rule panel,
// and saved back to disk.
//
// Phase E.1 — Shared NovaForge PCG Core

#include <cstdint>
#include <string>
#include <vector>

namespace NovaForge {

// ── PCGRuleValueType ──────────────────────────────────────────────────────────

enum class PCGRuleValueType : uint8_t {
    Float,
    Int,
    Bool,
    String,
    Vec2,   ///< "x,y"
    Vec3,   ///< "x,y,z"
    Range,  ///< "min,max"
    Tag,    ///< a placement/generation tag reference
};

inline const char* pcgRuleValueTypeName(PCGRuleValueType t) {
    switch (t) {
        case PCGRuleValueType::Float:  return "Float";
        case PCGRuleValueType::Int:    return "Int";
        case PCGRuleValueType::Bool:   return "Bool";
        case PCGRuleValueType::String: return "String";
        case PCGRuleValueType::Vec2:   return "Vec2";
        case PCGRuleValueType::Vec3:   return "Vec3";
        case PCGRuleValueType::Range:  return "Range";
        case PCGRuleValueType::Tag:    return "Tag";
    }
    return "Unknown";
}

// ── PCGRule ───────────────────────────────────────────────────────────────────

struct PCGRule {
    std::string        key;
    PCGRuleValueType   type         = PCGRuleValueType::Float;
    std::string        value;
    std::string        defaultValue;
    std::string        category;    ///< logical group (e.g. "placement", "density", "appearance")
    std::string        description;
    bool               readOnly     = false;
};

// ── PCGRuleSet ────────────────────────────────────────────────────────────────

class PCGRuleSet {
public:
    static constexpr uint32_t kMaxRules = 512;

    PCGRuleSet() = default;

    explicit PCGRuleSet(const std::string& id, const std::string& domain = "")
        : m_id(id), m_domain(domain) {}

    // ── Identity ──────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& id()     const { return m_id; }
    [[nodiscard]] const std::string& domain() const { return m_domain; }
    [[nodiscard]] const std::string& name()   const { return m_name; }
    [[nodiscard]] const std::string& version() const { return m_version; }

    void setId(const std::string& id)         { m_id = id; }
    void setDomain(const std::string& domain) { m_domain = domain; }
    void setName(const std::string& name)     { m_name = name; }
    void setVersion(const std::string& ver)   { m_version = ver; }

    // ── Rule management ───────────────────────────────────────────────────

    /// Add a new rule. Returns false if key already exists or capacity reached.
    bool addRule(PCGRule rule) {
        if (rule.key.empty() || m_rules.size() >= kMaxRules) return false;
        for (const auto& r : m_rules)
            if (r.key == rule.key) return false;
        m_rules.push_back(std::move(rule));
        m_dirty = true;
        return true;
    }

    /// Update rule value. Returns false if key not found or rule is readOnly.
    bool setValue(const std::string& key, const std::string& value) {
        for (auto& r : m_rules) {
            if (r.key == key) {
                if (r.readOnly) return false;
                r.value = value;
                m_dirty = true;
                return true;
            }
        }
        return false;
    }

    /// Remove a rule by key. Returns false if not found.
    bool removeRule(const std::string& key) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->key == key) {
                m_rules.erase(it);
                m_dirty = true;
                return true;
            }
        }
        return false;
    }

    /// Find a rule by key. Returns nullptr if not found.
    [[nodiscard]] const PCGRule* findRule(const std::string& key) const {
        for (const auto& r : m_rules)
            if (r.key == key) return &r;
        return nullptr;
    }

    [[nodiscard]] PCGRule* findRule(const std::string& key) {
        for (auto& r : m_rules)
            if (r.key == key) return &r;
        return nullptr;
    }

    /// Get value with fallback.
    [[nodiscard]] std::string getValue(const std::string& key,
                                       const std::string& fallback = "") const {
        if (const auto* r = findRule(key)) return r->value;
        return fallback;
    }

    [[nodiscard]] bool hasRule(const std::string& key) const {
        return findRule(key) != nullptr;
    }

    /// Reset all rules to default values.
    void resetToDefaults() {
        for (auto& r : m_rules) r.value = r.defaultValue;
        m_dirty = true;
    }

    /// Reset a single rule to its default.
    bool resetRule(const std::string& key) {
        if (auto* r = findRule(key)) { r->value = r->defaultValue; m_dirty = true; return true; }
        return false;
    }

    [[nodiscard]] const std::vector<PCGRule>& rules() const { return m_rules; }
    [[nodiscard]] uint32_t ruleCount() const { return static_cast<uint32_t>(m_rules.size()); }

    /// Rules filtered by category.
    [[nodiscard]] std::vector<const PCGRule*> rulesInCategory(const std::string& cat) const {
        std::vector<const PCGRule*> out;
        for (const auto& r : m_rules)
            if (r.category == cat) out.push_back(&r);
        return out;
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty()                  { m_dirty = false; }

private:
    std::string           m_id;
    std::string           m_domain;
    std::string           m_name;
    std::string           m_version = "1.0";
    std::vector<PCGRule>  m_rules;
    bool                  m_dirty   = false;
};

} // namespace NovaForge
