#pragma once
// NovaForge::DocumentPropertyGrid — schema-driven property grid for NovaForge documents.
//
// Provides a generic property editing surface backed by string key-value pairs that
// can be rendered by any UI layer. Supports field validation, dirty tracking, and
// type-annotated fields.
//
// Phase C.3 — Data Editor Tool (Real Content)

#include "NovaForge/EditorAdapter/IDocumentPanel.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace NovaForge {

// ── Property field types ─────────────────────────────────────────────────

enum class PropertyFieldType : uint8_t {
    String  = 0,
    Float   = 1,
    Int     = 2,
    Bool    = 3,
    Enum    = 4,   // string value constrained to enumOptions list
    Color   = 4,   // hex RRGGBB or RRGGBBAA
};

inline const char* propertyFieldTypeName(PropertyFieldType t) {
    switch (t) {
    case PropertyFieldType::String: return "String";
    case PropertyFieldType::Float:  return "Float";
    case PropertyFieldType::Int:    return "Int";
    case PropertyFieldType::Bool:   return "Bool";
    case PropertyFieldType::Enum:   return "Enum";
    }
    return "Unknown";
}

// ── PropertyField ────────────────────────────────────────────────────────

struct PropertyField {
    std::string           key;
    std::string           displayName;
    PropertyFieldType     type        = PropertyFieldType::String;
    std::string           value;                   // string representation
    std::string           defaultValue;
    bool                  readOnly    = false;
    std::string           tooltip;
    std::vector<std::string> enumOptions;           // valid values for Enum type
    std::function<bool(const std::string&)> validator; // returns true if valid
};

// ── PropertyCategory ─────────────────────────────────────────────────────

struct PropertyCategory {
    std::string              displayName;
    std::vector<PropertyField> fields;
};

// ── DocumentPropertyGrid ─────────────────────────────────────────────────

class DocumentPropertyGrid {
public:
    DocumentPropertyGrid() = default;

    // ── Schema definition ──────────────────────────────────────────────
    void addCategory(PropertyCategory cat) {
        m_categories.push_back(std::move(cat));
    }

    void addField(const std::string& category, PropertyField field) {
        auto it = std::find_if(m_categories.begin(), m_categories.end(),
                               [&](const auto& c) { return c.displayName == category; });
        if (it == m_categories.end()) {
            m_categories.push_back({category, {}});
            m_categories.back().fields.push_back(std::move(field));
        } else {
            it->fields.push_back(std::move(field));
        }
    }

    // ── Field access ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<PropertyCategory>& categories() const {
        return m_categories;
    }

    [[nodiscard]] const PropertyField* findField(const std::string& key) const {
        for (const auto& cat : m_categories) {
            for (const auto& f : cat.fields) {
                if (f.key == key) return &f;
            }
        }
        return nullptr;
    }

    [[nodiscard]] PropertyField* findField(const std::string& key) {
        for (auto& cat : m_categories) {
            for (auto& f : cat.fields) {
                if (f.key == key) return &f;
            }
        }
        return nullptr;
    }

    // ── Value mutation ────────────────────────────────────────────────
    // Returns true if the field was found and set. Returns false if the
    // field is read-only or validation fails.
    bool setValue(const std::string& key, const std::string& value) {
        auto* f = findField(key);
        if (!f || f->readOnly) return false;
        if (f->validator && !f->validator(value)) return false;
        f->value = value;
        m_dirty  = true;
        return true;
    }

    [[nodiscard]] std::string getValue(const std::string& key,
                                       const std::string& fallback = "") const {
        const auto* f = findField(key);
        return f ? f->value : fallback;
    }

    // ── Reset ─────────────────────────────────────────────────────────
    void resetToDefaults() {
        for (auto& cat : m_categories) {
            for (auto& f : cat.fields) {
                f.value = f.defaultValue;
            }
        }
        m_dirty = false;
    }

    // ── Dirty state ───────────────────────────────────────────────────
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty()                  { m_dirty = false; }

    // ── Validation ────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validateAll() const {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& cat : m_categories) {
            for (const auto& f : cat.fields) {
                if (f.readOnly) continue;
                if (f.validator && !f.validator(f.value)) {
                    msgs.push_back({f.key,
                                    "Invalid value for field '" + f.displayName + "'",
                                    DocumentPanelValidationSeverity::Error});
                }
                if (f.type == PropertyFieldType::Enum && !f.enumOptions.empty()) {
                    bool found = false;
                    for (const auto& opt : f.enumOptions) {
                        if (opt == f.value) { found = true; break; }
                    }
                    if (!found) {
                        msgs.push_back({f.key,
                                        "Value '" + f.value + "' not in allowed options",
                                        DocumentPanelValidationSeverity::Warning});
                    }
                }
            }
        }
        return msgs;
    }

    // ── Serialization (flat key=value pairs) ──────────────────────────
    // Returns all field key-value pairs in insertion order.
    [[nodiscard]] std::vector<std::pair<std::string,std::string>> toFlatMap() const {
        std::vector<std::pair<std::string,std::string>> out;
        for (const auto& cat : m_categories) {
            for (const auto& f : cat.fields) {
                out.emplace_back(f.key, f.value);
            }
        }
        return out;
    }

    // Total field count
    [[nodiscard]] size_t fieldCount() const {
        size_t n = 0;
        for (const auto& c : m_categories) n += c.fields.size();
        return n;
    }

private:
    std::vector<PropertyCategory> m_categories;
    bool                          m_dirty = false;
};

// ── DocumentPropertyGridBuilder ─────────────────────────────────────────
// Fluent builder for constructing a grid schema.

class DocumentPropertyGridBuilder {
public:
    explicit DocumentPropertyGridBuilder(DocumentPropertyGrid& grid) : m_grid(grid) {}

    DocumentPropertyGridBuilder& category(const std::string& name) {
        m_currentCategory = name;
        return *this;
    }

    DocumentPropertyGridBuilder& field(const std::string& key,
                                       const std::string& displayName,
                                       PropertyFieldType  type,
                                       const std::string& defaultVal = "") {
        PropertyField f;
        f.key          = key;
        f.displayName  = displayName;
        f.type         = type;
        f.value        = defaultVal;
        f.defaultValue = defaultVal;
        m_grid.addField(m_currentCategory, std::move(f));
        return *this;
    }

    DocumentPropertyGridBuilder& readOnlyField(const std::string& key,
                                               const std::string& displayName,
                                               const std::string& value) {
        PropertyField f;
        f.key         = key;
        f.displayName = displayName;
        f.type        = PropertyFieldType::String;
        f.value       = value;
        f.readOnly    = true;
        m_grid.addField(m_currentCategory, std::move(f));
        return *this;
    }

private:
    DocumentPropertyGrid& m_grid;
    std::string           m_currentCategory = "General";
};

} // namespace NovaForge
