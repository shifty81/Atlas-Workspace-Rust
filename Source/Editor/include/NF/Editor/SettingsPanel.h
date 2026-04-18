#pragma once
// NF::Editor — Settings panel / control panel surface
#include "NF/Editor/EditorSettings.h"
#include "NF/Editor/EditorPanel.h"
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace NF {

// ── Setting Entry ────────────────────────────────────────────────
// A single setting with typed value and metadata.

using SettingValue = std::variant<bool, int, float, std::string>;

struct SettingEntry {
    std::string key;
    std::string label;
    std::string category;
    std::string description;
    SettingValue value;
    SettingValue defaultValue;
    bool readOnly = false;

    [[nodiscard]] bool isDefault() const { return value == defaultValue; }
    void resetToDefault() { value = defaultValue; }

    [[nodiscard]] bool isBool()   const { return std::holds_alternative<bool>(value); }
    [[nodiscard]] bool isInt()    const { return std::holds_alternative<int>(value); }
    [[nodiscard]] bool isFloat()  const { return std::holds_alternative<float>(value); }
    [[nodiscard]] bool isString() const { return std::holds_alternative<std::string>(value); }
};

// ── Settings Category ────────────────────────────────────────────

struct SettingsCategory {
    std::string name;
    std::string icon;
    std::vector<SettingEntry> entries;
    bool expanded = true;

    [[nodiscard]] size_t entryCount() const { return entries.size(); }
    [[nodiscard]] bool empty() const { return entries.empty(); }

    SettingEntry* findEntry(const std::string& key) {
        for (auto& e : entries) if (e.key == key) return &e;
        return nullptr;
    }

    const SettingEntry* findEntry(const std::string& key) const {
        for (const auto& e : entries) if (e.key == key) return &e;
        return nullptr;
    }
};

// ── Settings Registry ────────────────────────────────────────────
// Central store for all editor settings, organized by category.

class SettingsRegistry {
public:
    static constexpr size_t MAX_CATEGORIES = 32;

    SettingsCategory* addCategory(const std::string& name, const std::string& icon = "") {
        if (m_categories.size() >= MAX_CATEGORIES) return nullptr;
        for (auto& c : m_categories) if (c.name == name) return &c;
        m_categories.push_back({name, icon, {}, true});
        return &m_categories.back();
    }

    bool removeCategory(const std::string& name) {
        for (auto it = m_categories.begin(); it != m_categories.end(); ++it) {
            if (it->name == name) { m_categories.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SettingsCategory* findCategory(const std::string& name) {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    [[nodiscard]] const SettingsCategory* findCategory(const std::string& name) const {
        for (const auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    bool addSetting(const std::string& category, SettingEntry entry) {
        auto* cat = addCategory(category);
        if (!cat) return false;
        for (const auto& e : cat->entries) if (e.key == entry.key) return false;
        cat->entries.push_back(std::move(entry));
        return true;
    }

    [[nodiscard]] const SettingEntry* findSetting(const std::string& key) const {
        for (const auto& cat : m_categories) {
            for (const auto& e : cat.entries) {
                if (e.key == key) return &e;
            }
        }
        return nullptr;
    }

    SettingEntry* findSettingMut(const std::string& key) {
        for (auto& cat : m_categories) {
            for (auto& e : cat.entries) {
                if (e.key == key) return &e;
            }
        }
        return nullptr;
    }

    bool setSetting(const std::string& key, SettingValue value) {
        auto* entry = findSettingMut(key);
        if (!entry || entry->readOnly) return false;
        entry->value = std::move(value);
        ++m_changeCount;
        return true;
    }

    void resetAll() {
        for (auto& cat : m_categories) {
            for (auto& e : cat.entries) {
                e.resetToDefault();
            }
        }
        ++m_changeCount;
    }

    [[nodiscard]] size_t categoryCount() const { return m_categories.size(); }
    [[nodiscard]] size_t changeCount() const { return m_changeCount; }
    [[nodiscard]] const std::vector<SettingsCategory>& categories() const { return m_categories; }

    // Count total settings across all categories
    [[nodiscard]] size_t totalSettings() const {
        size_t total = 0;
        for (const auto& cat : m_categories) total += cat.entryCount();
        return total;
    }

    // Load default editor settings
    void loadDefaults() {
        addSetting("General", {"general.dark_mode", "Dark Mode", "General",
                               "Enable dark theme", true, true});
        addSetting("General", {"general.autosave", "Auto Save", "General",
                               "Auto-save periodically", true, true});
        addSetting("General", {"general.autosave_interval", "Auto Save Interval (s)", "General",
                               "Seconds between auto-saves", 300.f, 300.f});
        addSetting("General", {"general.undo_history", "Undo History Size", "General",
                               "Max undo steps", 100, 100});
        addSetting("Viewport", {"viewport.show_grid", "Show Grid", "Viewport",
                                "Display grid in viewport", true, true});
        addSetting("Viewport", {"viewport.show_gizmos", "Show Gizmos", "Viewport",
                                "Display gizmos in viewport", true, true});
        addSetting("Viewport", {"viewport.camera_speed", "Camera Speed", "Viewport",
                                "Fly camera speed", 10.f, 10.f});
        addSetting("Input", {"input.snap_enabled", "Snap Enabled", "Input",
                             "Enable grid snapping", false, false});
    }

private:
    std::vector<SettingsCategory> m_categories;
    size_t m_changeCount = 0;
};

// ── Settings Panel ───────────────────────────────────────────────
// The editor panel that presents the settings UI.

class EditorSettingsPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }

    void update(float /*dt*/) override {}

    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        ui.drawText(bounds.x + 8.f, bounds.y + 8.f, "Settings", theme.panelText);

        float y = bounds.y + 32.f;
        for (const auto& cat : m_registry.categories()) {
            ui.drawText(bounds.x + 8.f, y, cat.name.c_str(), theme.selectionBorder);
            y += 24.f;
            for (const auto& entry : cat.entries) {
                ui.drawText(bounds.x + 24.f, y, entry.label.c_str(), theme.panelText);
                y += 20.f;
            }
            y += 4.f;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        (void)ctx; (void)bounds;
    }

    [[nodiscard]] SettingsRegistry& registry() { return m_registry; }
    [[nodiscard]] const SettingsRegistry& registry() const { return m_registry; }

    void setSearchFilter(const std::string& filter) { m_searchFilter = filter; }
    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }

private:
    std::string m_name = "Settings";
    SettingsRegistry m_registry;
    std::string m_searchFilter;
};


} // namespace NF
