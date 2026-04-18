#pragma once
// NF::Workspace — Phase 13: Workspace Preferences and Configuration
//
// Layered on top of SettingsStore with:
//   PreferenceCategory  — typed grouping
//   PreferenceType      — value type enum
//   PreferenceEntry     — typed preference with default, range, description
//   PreferenceRegistry  — register/lookup/validate preferences
//   PreferenceSerializer — serialize/deserialize to WorkspaceProjectFile

#include "NF/Workspace/SettingsStore.h"
#include "NF/Workspace/WorkspaceEventBus.h"
#include "NF/Workspace/WorkspaceProjectFile.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// PreferenceCategory — grouping for preferences
// ═════════════════════════════════════════════════════════════════

enum class PreferenceCategory : uint8_t {
    General     = 0,
    Appearance  = 1,
    Keybindings = 2,
    Editor      = 3,
    Build       = 4,
    AI          = 5,
    Plugin      = 6,
    Custom      = 7,
};

inline const char* preferenceCategoryName(PreferenceCategory c) {
    switch (c) {
        case PreferenceCategory::General:     return "General";
        case PreferenceCategory::Appearance:  return "Appearance";
        case PreferenceCategory::Keybindings: return "Keybindings";
        case PreferenceCategory::Editor:      return "Editor";
        case PreferenceCategory::Build:       return "Build";
        case PreferenceCategory::AI:          return "AI";
        case PreferenceCategory::Plugin:      return "Plugin";
        case PreferenceCategory::Custom:      return "Custom";
        default:                              return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// PreferenceType — value type classification
// ═════════════════════════════════════════════════════════════════

enum class PreferenceType : uint8_t {
    String  = 0,
    Bool    = 1,
    Int     = 2,
    Float   = 3,
};

inline const char* preferenceTypeName(PreferenceType t) {
    switch (t) {
        case PreferenceType::String: return "String";
        case PreferenceType::Bool:   return "Bool";
        case PreferenceType::Int:    return "Int";
        case PreferenceType::Float:  return "Float";
        default:                     return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// PreferenceEntry — individual preference descriptor
// ═════════════════════════════════════════════════════════════════

struct PreferenceEntry {
    std::string         key;
    std::string         displayName;
    std::string         description;
    std::string         defaultValue;
    PreferenceCategory  category    = PreferenceCategory::General;
    PreferenceType      type        = PreferenceType::String;
    // Range constraints (for Int and Float types)
    float               minValue    = 0.f;
    float               maxValue    = 0.f;
    bool                hasRange    = false;
    bool                readOnly    = false;

    [[nodiscard]] bool isValid() const {
        return !key.empty() && !displayName.empty();
    }

    // Validate a value against the entry's constraints.
    [[nodiscard]] bool validate(const std::string& value) const {
        if (value.empty()) return true; // empty = use default
        switch (type) {
            case PreferenceType::Bool:
                return value == "true" || value == "false" ||
                       value == "1"    || value == "0";
            case PreferenceType::Int: {
                try {
                    int v = std::stoi(value);
                    if (hasRange) return v >= static_cast<int>(minValue)
                                      && v <= static_cast<int>(maxValue);
                    return true;
                } catch (...) { return false; }
            }
            case PreferenceType::Float: {
                try {
                    float v = std::stof(value);
                    if (hasRange) return v >= minValue && v <= maxValue;
                    return true;
                } catch (...) { return false; }
            }
            case PreferenceType::String:
                return true;
        }
        return true;
    }

    static PreferenceEntry makeString(const std::string& key, const std::string& display,
                                       const std::string& defaultVal,
                                       PreferenceCategory cat = PreferenceCategory::General,
                                       const std::string& desc = {}) {
        PreferenceEntry e;
        e.key = key; e.displayName = display; e.defaultValue = defaultVal;
        e.category = cat; e.description = desc; e.type = PreferenceType::String;
        return e;
    }

    static PreferenceEntry makeBool(const std::string& key, const std::string& display,
                                     bool defaultVal,
                                     PreferenceCategory cat = PreferenceCategory::General,
                                     const std::string& desc = {}) {
        PreferenceEntry e;
        e.key = key; e.displayName = display; e.defaultValue = defaultVal ? "true" : "false";
        e.category = cat; e.description = desc; e.type = PreferenceType::Bool;
        return e;
    }

    static PreferenceEntry makeInt(const std::string& key, const std::string& display,
                                    int defaultVal, int minV, int maxV,
                                    PreferenceCategory cat = PreferenceCategory::General,
                                    const std::string& desc = {}) {
        PreferenceEntry e;
        e.key = key; e.displayName = display; e.defaultValue = std::to_string(defaultVal);
        e.category = cat; e.description = desc; e.type = PreferenceType::Int;
        e.minValue = static_cast<float>(minV); e.maxValue = static_cast<float>(maxV);
        e.hasRange = true;
        return e;
    }

    static PreferenceEntry makeFloat(const std::string& key, const std::string& display,
                                      float defaultVal, float minV, float maxV,
                                      PreferenceCategory cat = PreferenceCategory::General,
                                      const std::string& desc = {}) {
        PreferenceEntry e;
        e.key = key; e.displayName = display; e.defaultValue = std::to_string(defaultVal);
        e.category = cat; e.description = desc; e.type = PreferenceType::Float;
        e.minValue = minV; e.maxValue = maxV; e.hasRange = true;
        return e;
    }
};

// ═════════════════════════════════════════════════════════════════
// PreferenceRegistry — preference registration and management
// ═════════════════════════════════════════════════════════════════

class PreferenceRegistry {
public:
    static constexpr size_t MAX_ENTRIES = 512;

    // Register a preference entry. Returns false if key duplicate or at capacity.
    bool registerEntry(const PreferenceEntry& entry) {
        if (!entry.isValid()) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        for (const auto& e : m_entries) {
            if (e.key == entry.key) return false;
        }
        m_entries.push_back(entry);
        return true;
    }

    // Unregister by key.
    bool unregisterEntry(const std::string& key) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->key == key) { m_entries.erase(it); return true; }
        }
        return false;
    }

    // Find entry by key.
    [[nodiscard]] const PreferenceEntry* find(const std::string& key) const {
        for (const auto& e : m_entries) if (e.key == key) return &e;
        return nullptr;
    }

    // Check if registered.
    [[nodiscard]] bool isRegistered(const std::string& key) const {
        return find(key) != nullptr;
    }

    // Get all entries in a category.
    [[nodiscard]] std::vector<const PreferenceEntry*> findByCategory(PreferenceCategory cat) const {
        std::vector<const PreferenceEntry*> result;
        for (const auto& e : m_entries) {
            if (e.category == cat) result.push_back(&e);
        }
        return result;
    }

    // Count entries in a category.
    [[nodiscard]] size_t countByCategory(PreferenceCategory cat) const {
        size_t c = 0;
        for (const auto& e : m_entries) if (e.category == cat) ++c;
        return c;
    }

    // Validate a value for a given key.
    [[nodiscard]] bool validate(const std::string& key, const std::string& value) const {
        auto* entry = find(key);
        if (!entry) return false;
        return entry->validate(value);
    }

    [[nodiscard]] size_t count() const { return m_entries.size(); }
    [[nodiscard]] bool   empty() const { return m_entries.empty(); }
    [[nodiscard]] const std::vector<PreferenceEntry>& all() const { return m_entries; }

    void clear() { m_entries.clear(); }

    // Populate SettingsStore defaults from all registered entries.
    void populateDefaults(SettingsStore& store) const {
        for (const auto& e : m_entries) {
            if (!e.defaultValue.empty()) {
                store.setDefault(e.key, e.defaultValue);
            }
        }
    }

    // Load workspace defaults — common preferences everyone needs.
    void loadWorkspaceDefaults() {
        registerEntry(PreferenceEntry::makeBool(
            "general.auto_save", "Auto Save", true,
            PreferenceCategory::General, "Automatically save project on interval"));
        registerEntry(PreferenceEntry::makeInt(
            "general.auto_save_interval", "Auto Save Interval (sec)", 300, 30, 3600,
            PreferenceCategory::General, "Seconds between auto-saves"));
        registerEntry(PreferenceEntry::makeBool(
            "general.show_welcome", "Show Welcome Screen", true,
            PreferenceCategory::General, "Show welcome screen on startup"));

        registerEntry(PreferenceEntry::makeString(
            "appearance.theme", "Theme", "dark",
            PreferenceCategory::Appearance, "UI theme (dark/light)"));
        registerEntry(PreferenceEntry::makeInt(
            "appearance.font_size", "Font Size", 14, 8, 32,
            PreferenceCategory::Appearance, "Base UI font size in points"));
        registerEntry(PreferenceEntry::makeFloat(
            "appearance.ui_scale", "UI Scale", 1.0f, 0.5f, 3.0f,
            PreferenceCategory::Appearance, "Global UI scale factor"));

        registerEntry(PreferenceEntry::makeBool(
            "editor.vim_mode", "Vim Mode", false,
            PreferenceCategory::Editor, "Enable Vim keybindings in text editors"));
        registerEntry(PreferenceEntry::makeInt(
            "editor.tab_size", "Tab Size", 4, 1, 8,
            PreferenceCategory::Editor, "Number of spaces per tab"));
        registerEntry(PreferenceEntry::makeBool(
            "editor.show_line_numbers", "Show Line Numbers", true,
            PreferenceCategory::Editor, "Display line numbers in code editors"));

        registerEntry(PreferenceEntry::makeBool(
            "build.parallel", "Parallel Build", true,
            PreferenceCategory::Build, "Enable parallel compilation"));
        registerEntry(PreferenceEntry::makeInt(
            "build.max_jobs", "Max Build Jobs", 8, 1, 64,
            PreferenceCategory::Build, "Maximum number of parallel build jobs"));

        registerEntry(PreferenceEntry::makeBool(
            "ai.enabled", "Enable AtlasAI", true,
            PreferenceCategory::AI, "Enable AtlasAI suggestions and analysis"));
        registerEntry(PreferenceEntry::makeBool(
            "ai.auto_suggest", "Auto Suggest", true,
            PreferenceCategory::AI, "Show automatic AI suggestions"));
    }

private:
    std::vector<PreferenceEntry> m_entries;
};

// ═════════════════════════════════════════════════════════════════
// PreferenceController — coordinated preference access
//   Binds PreferenceRegistry + SettingsStore + EventBus
// ═════════════════════════════════════════════════════════════════

class PreferenceController {
public:
    PreferenceController(PreferenceRegistry& registry, SettingsStore& store,
                          WorkspaceEventBus* bus = nullptr)
        : m_registry(registry), m_store(store), m_bus(bus) {}

    // Set a preference value (validates, writes to store, fires event).
    bool set(const std::string& key, const std::string& value,
             SettingsLayer layer = SettingsLayer::User) {
        auto* entry = m_registry.find(key);
        if (!entry) return false;
        if (entry->readOnly) return false;
        if (!entry->validate(value)) return false;

        m_store.set(key, value, layer);

        // Fire preference changed event on bus
        if (m_bus) {
            m_bus->publish(WorkspaceEvent::make(
                WorkspaceEventType::System,
                "preferences",
                key + "=" + value
            ));
        }
        return true;
    }

    // Get a preference value (reads from store with fallback to default).
    [[nodiscard]] std::string get(const std::string& key) const {
        return m_store.get(key);
    }

    // Get with explicit fallback if not set anywhere.
    [[nodiscard]] std::string getOr(const std::string& key, const std::string& fallback) const {
        return m_store.getOr(key, fallback);
    }

    // Typed getters
    [[nodiscard]] bool getBool(const std::string& key) const {
        return m_store.getBool(key);
    }

    [[nodiscard]] int32_t getInt(const std::string& key) const {
        return m_store.getInt32(key);
    }

    [[nodiscard]] float getFloat(const std::string& key) const {
        return m_store.getFloat(key);
    }

    // Reset a preference to its registered default.
    bool resetToDefault(const std::string& key) {
        auto* entry = m_registry.find(key);
        if (!entry) return false;
        // Remove from User and Project layers, leaving only Default
        m_store.remove(key, SettingsLayer::User);
        m_store.remove(key, SettingsLayer::Project);

        if (m_bus) {
            m_bus->publish(WorkspaceEvent::make(
                WorkspaceEventType::System,
                "preferences",
                key + "=reset"
            ));
        }
        return true;
    }

    // Reset all preferences to defaults.
    void resetAll() {
        m_store.clearLayer(SettingsLayer::User);
        m_store.clearLayer(SettingsLayer::Project);
        if (m_bus) {
            m_bus->publish(WorkspaceEvent::make(
                WorkspaceEventType::System,
                "preferences",
                "reset_all"
            ));
        }
    }

    // Initialize: populate SettingsStore defaults from registry.
    void initialize() {
        m_registry.populateDefaults(m_store);
    }

    [[nodiscard]] const PreferenceRegistry& registry() const { return m_registry; }
    [[nodiscard]] SettingsStore& store() { return m_store; }

private:
    PreferenceRegistry& m_registry;
    SettingsStore&      m_store;
    WorkspaceEventBus*  m_bus;
};

// ═════════════════════════════════════════════════════════════════
// PreferenceSerializer — persist preferences via WorkspaceProjectFile
// ═════════════════════════════════════════════════════════════════

struct PreferenceSerializeResult {
    bool        succeeded   = false;
    size_t      entryCount  = 0;
    std::string errorMessage;

    static PreferenceSerializeResult ok(size_t count) {
        return {true, count, {}};
    }
    static PreferenceSerializeResult fail(const std::string& msg) {
        return {false, 0, msg};
    }
};

class PreferenceSerializer {
public:
    static constexpr const char* SECTION_REGISTRY = "Preferences.Registry";

    // Serialize preference registry entries into a WorkspaceProjectFile section.
    static PreferenceSerializeResult serializeRegistry(
            const PreferenceRegistry& registry, WorkspaceProjectFile& file) {
        auto& section = file.section(SECTION_REGISTRY);
        section.clear();
        size_t idx = 0;
        for (const auto& entry : registry.all()) {
            std::string prefix = "pref." + std::to_string(idx);
            section.set(prefix + ".key",          entry.key);
            section.set(prefix + ".display",      entry.displayName);
            section.set(prefix + ".desc",         entry.description);
            section.set(prefix + ".default",      entry.defaultValue);
            section.set(prefix + ".category",     preferenceCategoryName(entry.category));
            section.set(prefix + ".type",         preferenceTypeName(entry.type));
            section.set(prefix + ".readOnly",     entry.readOnly ? "true" : "false");
            if (entry.hasRange) {
                section.set(prefix + ".min", std::to_string(entry.minValue));
                section.set(prefix + ".max", std::to_string(entry.maxValue));
            }
            ++idx;
        }
        section.set("count", std::to_string(idx));
        return PreferenceSerializeResult::ok(idx);
    }

    // Deserialize preference registry entries from a WorkspaceProjectFile section.
    static PreferenceSerializeResult deserializeRegistry(
            PreferenceRegistry& registry, const WorkspaceProjectFile& file) {
        if (!file.hasSection(SECTION_REGISTRY)) {
            return PreferenceSerializeResult::fail("missing section");
        }
        const auto* section = file.findSection(SECTION_REGISTRY);
        std::string countStr = section->getOr("count", "");
        if (countStr.empty()) {
            return PreferenceSerializeResult::fail("missing count");
        }
        size_t count = static_cast<size_t>(std::stoi(countStr));
        size_t loaded = 0;
        for (size_t i = 0; i < count; ++i) {
            std::string prefix = "pref." + std::to_string(i);
            PreferenceEntry entry;
            entry.key          = section->getOr(prefix + ".key", "");
            entry.displayName  = section->getOr(prefix + ".display", "");
            entry.description  = section->getOr(prefix + ".desc", "");
            entry.defaultValue = section->getOr(prefix + ".default", "");
            entry.readOnly     = section->getOr(prefix + ".readOnly", "") == "true";

            // Parse category
            std::string catStr = section->getOr(prefix + ".category", "");
            if (catStr == "General")      entry.category = PreferenceCategory::General;
            else if (catStr == "Appearance")  entry.category = PreferenceCategory::Appearance;
            else if (catStr == "Keybindings") entry.category = PreferenceCategory::Keybindings;
            else if (catStr == "Editor")      entry.category = PreferenceCategory::Editor;
            else if (catStr == "Build")       entry.category = PreferenceCategory::Build;
            else if (catStr == "AI")          entry.category = PreferenceCategory::AI;
            else if (catStr == "Plugin")      entry.category = PreferenceCategory::Plugin;
            else entry.category = PreferenceCategory::Custom;

            // Parse type
            std::string typeStr = section->getOr(prefix + ".type", "");
            if (typeStr == "String")    entry.type = PreferenceType::String;
            else if (typeStr == "Bool") entry.type = PreferenceType::Bool;
            else if (typeStr == "Int")  entry.type = PreferenceType::Int;
            else if (typeStr == "Float") entry.type = PreferenceType::Float;

            // Parse range
            std::string minStr = section->getOr(prefix + ".min", "");
            std::string maxStr = section->getOr(prefix + ".max", "");
            if (!minStr.empty() && !maxStr.empty()) {
                entry.hasRange = true;
                entry.minValue = std::stof(minStr);
                entry.maxValue = std::stof(maxStr);
            }

            if (entry.isValid()) {
                registry.registerEntry(entry);
                ++loaded;
            }
        }
        return PreferenceSerializeResult::ok(loaded);
    }

    // Round-trip helper for testing.
    static PreferenceSerializeResult roundTrip(
            const PreferenceRegistry& source, PreferenceRegistry& dest) {
        WorkspaceProjectFile file;
        file.setProjectId("roundtrip");
        file.setProjectName("RoundTrip");
        auto sres = serializeRegistry(source, file);
        if (!sres.succeeded) return sres;
        std::string text = file.serialize();
        WorkspaceProjectFile parsed;
        if (!WorkspaceProjectFile::parse(text, parsed)) {
            return PreferenceSerializeResult::fail("parse failed");
        }
        return deserializeRegistry(dest, parsed);
    }
};

} // namespace NF
