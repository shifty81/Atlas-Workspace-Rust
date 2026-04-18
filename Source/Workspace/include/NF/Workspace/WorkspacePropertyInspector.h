#pragma once
// NF::Workspace — Phase 59: Workspace Property Inspector
//
// Property grid/inspector for viewing and editing typed properties of
// selected items with categories, read-only support, and observer notifications.
//
//   PropertyType      — String / Int / Float / Bool / Color / Vec2 / Vec3 / Enum / Custom
//   PropertyEntry     — id + name + type + value + category + readOnly + description; isValid()
//   PropertyCategory  — named category of properties (MAX_PROPERTIES=128)
//   PropertyInspector — category registry (MAX_CATEGORIES=32); add/remove/find;
//                       set/get value shortcuts; observer callbacks; serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// PropertyType
// ═════════════════════════════════════════════════════════════════

enum class PropertyType : uint8_t {
    String = 0,
    Int    = 1,
    Float  = 2,
    Bool   = 3,
    Color  = 4,
    Vec2   = 5,
    Vec3   = 6,
    Enum   = 7,
    Custom = 8,
};

inline const char* propertyTypeName(PropertyType t) {
    switch (t) {
        case PropertyType::String: return "String";
        case PropertyType::Int:    return "Int";
        case PropertyType::Float:  return "Float";
        case PropertyType::Bool:   return "Bool";
        case PropertyType::Color:  return "Color";
        case PropertyType::Vec2:   return "Vec2";
        case PropertyType::Vec3:   return "Vec3";
        case PropertyType::Enum:   return "Enum";
        case PropertyType::Custom: return "Custom";
        default:                   return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// PropertyEntry
// ═════════════════════════════════════════════════════════════════

struct PropertyEntry {
    std::string  id;
    std::string  name;
    PropertyType type        = PropertyType::String;
    std::string  value;          // string representation of value
    std::string  category;
    bool         readOnly    = false;
    std::string  description;
    std::string  enumOptions;    // pipe-separated for Enum type

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    bool operator==(const PropertyEntry& o) const { return id == o.id; }
    bool operator!=(const PropertyEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// PropertyCategory
// ═════════════════════════════════════════════════════════════════

class PropertyCategory {
public:
    static constexpr int MAX_PROPERTIES = 128;

    std::string id;
    std::string name;
    bool        collapsed = false;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    bool addProperty(const PropertyEntry& prop) {
        if (!prop.isValid()) return false;
        if (findProperty(prop.id)) return false;
        if (static_cast<int>(m_properties.size()) >= MAX_PROPERTIES) return false;
        m_properties.push_back(prop);
        return true;
    }

    bool removeProperty(const std::string& propId) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
                               [&](const PropertyEntry& p) { return p.id == propId; });
        if (it == m_properties.end()) return false;
        m_properties.erase(it);
        return true;
    }

    const PropertyEntry* findProperty(const std::string& propId) const {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
                               [&](const PropertyEntry& p) { return p.id == propId; });
        return it != m_properties.end() ? &(*it) : nullptr;
    }

    PropertyEntry* findPropertyMut(const std::string& propId) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
                               [&](const PropertyEntry& p) { return p.id == propId; });
        return it != m_properties.end() ? &(*it) : nullptr;
    }

    bool containsProperty(const std::string& propId) const { return findProperty(propId) != nullptr; }

    [[nodiscard]] int                                count()      const { return static_cast<int>(m_properties.size()); }
    [[nodiscard]] bool                               empty()      const { return m_properties.empty(); }
    [[nodiscard]] const std::vector<PropertyEntry>&  properties() const { return m_properties; }

    void clear() { m_properties.clear(); }

private:
    std::vector<PropertyEntry> m_properties;
};

// ═════════════════════════════════════════════════════════════════
// PropertyInspector
// ═════════════════════════════════════════════════════════════════

class PropertyInspector {
public:
    using PropertyObserver = std::function<void(const std::string& propId, const std::string& oldVal, const std::string& newVal)>;

    static constexpr int MAX_CATEGORIES = 32;
    static constexpr int MAX_OBSERVERS  = 16;

    // Category management ──────────────────────────────────────

    bool addCategory(const PropertyCategory& cat) {
        if (!cat.isValid()) return false;
        if (findCategory(cat.id)) return false;
        if (static_cast<int>(m_categories.size()) >= MAX_CATEGORIES) return false;
        m_categories.push_back(cat);
        return true;
    }

    bool removeCategory(const std::string& catId) {
        auto it = findCategoryIt(catId);
        if (it == m_categories.end()) return false;
        m_categories.erase(it);
        return true;
    }

    PropertyCategory* findCategory(const std::string& catId) {
        auto it = findCategoryIt(catId);
        return it != m_categories.end() ? &(*it) : nullptr;
    }

    const PropertyCategory* findCategory(const std::string& catId) const {
        auto it = std::find_if(m_categories.begin(), m_categories.end(),
                               [&](const PropertyCategory& c) { return c.id == catId; });
        return it != m_categories.end() ? &(*it) : nullptr;
    }

    bool hasCategory(const std::string& catId) const { return findCategory(catId) != nullptr; }

    [[nodiscard]] int categoryCount() const { return static_cast<int>(m_categories.size()); }

    [[nodiscard]] const std::vector<PropertyCategory>& categories() const { return m_categories; }

    // Property shortcuts ───────────────────────────────────────

    bool addProperty(const std::string& catId, const PropertyEntry& prop) {
        auto* cat = findCategory(catId);
        if (!cat) return false;
        return cat->addProperty(prop);
    }

    bool removeProperty(const std::string& catId, const std::string& propId) {
        auto* cat = findCategory(catId);
        if (!cat) return false;
        return cat->removeProperty(propId);
    }

    bool setValue(const std::string& catId, const std::string& propId, const std::string& newValue) {
        auto* cat = findCategory(catId);
        if (!cat) return false;
        auto* prop = cat->findPropertyMut(propId);
        if (!prop) return false;
        if (prop->readOnly) return false;
        std::string oldValue = prop->value;
        if (oldValue == newValue) return true;
        prop->value = newValue;
        notifyObservers(propId, oldValue, newValue);
        return true;
    }

    const std::string* getValue(const std::string& catId, const std::string& propId) const {
        const auto* cat = findCategory(catId);
        if (!cat) return nullptr;
        const auto* prop = cat->findProperty(propId);
        if (!prop) return nullptr;
        return &prop->value;
    }

    // Search ───────────────────────────────────────────────────

    [[nodiscard]] std::vector<const PropertyEntry*> searchByName(const std::string& query) const {
        std::vector<const PropertyEntry*> results;
        if (query.empty()) return results;
        std::string lq = toLower(query);
        for (const auto& cat : m_categories) {
            for (const auto& prop : cat.properties()) {
                if (toLower(prop.name).find(lq) != std::string::npos) {
                    results.push_back(&prop);
                }
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const PropertyEntry*> filterByType(PropertyType type) const {
        std::vector<const PropertyEntry*> results;
        for (const auto& cat : m_categories) {
            for (const auto& prop : cat.properties()) {
                if (prop.type == type) {
                    results.push_back(&prop);
                }
            }
        }
        return results;
    }

    [[nodiscard]] int totalProperties() const {
        int total = 0;
        for (const auto& cat : m_categories) total += cat.count();
        return total;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(PropertyObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization ────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& cat : m_categories) {
            out += "[" + esc(cat.id) + "|" + esc(cat.name) + "|"
                 + (cat.collapsed ? "1" : "0") + "]\n";
            for (const auto& prop : cat.properties()) {
                out += esc(prop.id) + "|"
                     + esc(prop.name) + "|"
                     + std::to_string(static_cast<int>(prop.type)) + "|"
                     + esc(prop.value) + "|"
                     + (prop.readOnly ? "1" : "0") + "|"
                     + esc(prop.description) + "|"
                     + esc(prop.enumOptions) + "\n";
            }
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_categories.clear();
        if (text.empty()) return true;

        PropertyCategory* current = nullptr;
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
                PropertyCategory cat;
                cat.id        = unesc(fields[0]);
                cat.name      = unesc(fields[1]);
                cat.collapsed = (fields[2] == "1");
                m_categories.push_back(std::move(cat));
                current = &m_categories.back();
            } else if (current) {
                auto fields = splitPipe(line);
                if (fields.size() < 7) continue;
                PropertyEntry prop;
                prop.id          = unesc(fields[0]);
                prop.name        = unesc(fields[1]);
                prop.type        = static_cast<PropertyType>(std::stoi(fields[2]));
                prop.value       = unesc(fields[3]);
                prop.readOnly    = (fields[4] == "1");
                prop.description = unesc(fields[5]);
                prop.enumOptions = unesc(fields[6]);
                current->addProperty(prop);
            }
        }
        return true;
    }

    void clear() {
        m_categories.clear();
        m_observers.clear();
    }

private:
    std::vector<PropertyCategory>  m_categories;
    std::vector<PropertyObserver>  m_observers;

    std::vector<PropertyCategory>::iterator findCategoryIt(const std::string& id) {
        return std::find_if(m_categories.begin(), m_categories.end(),
                            [&](const PropertyCategory& c) { return c.id == id; });
    }

    void notifyObservers(const std::string& propId, const std::string& oldVal, const std::string& newVal) {
        for (auto& cb : m_observers) {
            if (cb) cb(propId, oldVal, newVal);
        }
    }

    static std::string esc(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '|')       out += "\\P";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\N";
            else                out += c;
        }
        return out;
    }

    static std::string unesc(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if (s[i + 1] == 'P')  { out += '|'; ++i; }
                else if (s[i + 1] == '\\') { out += '\\'; ++i; }
                else if (s[i + 1] == 'N')  { out += '\n'; ++i; }
                else out += s[i];
            } else {
                out += s[i];
            }
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
            if (p == std::string::npos) {
                fields.push_back(s.substr(start));
                break;
            }
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
