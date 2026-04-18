#pragma once
// NF::Editor — Iconography spec: icon sizes, grid, categories, and naming
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace NF {

// ── Icon Size Tier ────────────────────────────────────────────────

enum class IconSizeTier : uint8_t {
    XSmall  = 12,
    Small   = 16,
    Medium  = 20,
    Large   = 24,
    XLarge  = 32,
    Display = 48,
};

inline const char* iconSizeTierName(IconSizeTier t) {
    switch (t) {
        case IconSizeTier::XSmall:  return "XSmall";
        case IconSizeTier::Small:   return "Small";
        case IconSizeTier::Medium:  return "Medium";
        case IconSizeTier::Large:   return "Large";
        case IconSizeTier::XLarge:  return "XLarge";
        case IconSizeTier::Display: return "Display";
    }
    return "Unknown";
}

inline uint8_t iconSizePx(IconSizeTier t) { return static_cast<uint8_t>(t); }

// ── Icon Category ─────────────────────────────────────────────────

enum class IconCategory : uint8_t {
    Actions,     // verbs: add, delete, edit, run, stop
    Navigation,  // arrows, chevrons, home, back
    Status,      // success, warning, error, info
    FileTypes,   // document, folder, image, code, mesh
    UI,          // panel, dock, float, tab, close
    Tools,       // brush, transform, camera, light
    Media,       // play, pause, volume, record
    Editor,      // scene, inspector, console, profiler
};

inline const char* iconCategoryName(IconCategory c) {
    switch (c) {
        case IconCategory::Actions:    return "Actions";
        case IconCategory::Navigation: return "Navigation";
        case IconCategory::Status:     return "Status";
        case IconCategory::FileTypes:  return "FileTypes";
        case IconCategory::UI:         return "UI";
        case IconCategory::Tools:      return "Tools";
        case IconCategory::Media:      return "Media";
        case IconCategory::Editor:     return "Editor";
    }
    return "Unknown";
}

// ── Icon Descriptor ───────────────────────────────────────────────

struct IconDescriptor {
    std::string   id;          // canonical id e.g. "actions/add"
    std::string   label;       // human-readable label
    IconCategory  category = IconCategory::Actions;
    IconSizeTier  preferredSize = IconSizeTier::Small;
    std::string   assetPath;   // path to SVG/PNG asset
    bool          hasMultiSize = false;

    [[nodiscard]] bool isValid() const {
        return !id.empty() && !label.empty();
    }
};

// ── Icon Registry ─────────────────────────────────────────────────

class IconRegistry {
public:
    bool registerIcon(const IconDescriptor& desc) {
        if (!desc.isValid()) return false;
        if (m_icons.count(desc.id)) return false;
        m_icons[desc.id] = desc;
        return true;
    }

    bool unregisterIcon(const std::string& id) {
        return m_icons.erase(id) > 0;
    }

    [[nodiscard]] const IconDescriptor* find(const std::string& id) const {
        auto it = m_icons.find(id);
        return it != m_icons.end() ? &it->second : nullptr;
    }

    [[nodiscard]] size_t count() const { return m_icons.size(); }
    [[nodiscard]] bool has(const std::string& id) const { return m_icons.count(id) > 0; }

    [[nodiscard]] std::vector<IconDescriptor> byCategory(IconCategory cat) const {
        std::vector<IconDescriptor> result;
        for (const auto& [id, desc] : m_icons) {
            if (desc.category == cat) result.push_back(desc);
        }
        return result;
    }

    void loadAtlasCoreDefaults() {
        auto add = [&](const char* id, const char* label,
                       IconCategory cat, IconSizeTier size) {
            IconDescriptor d;
            d.id = id; d.label = label; d.category = cat;
            d.preferredSize = size;
            d.assetPath = std::string("Icons/") + id + ".svg";
            d.hasMultiSize = (size == IconSizeTier::Small);
            registerIcon(d);
        };
        // Actions
        add("actions/add",    "Add",    IconCategory::Actions,    IconSizeTier::Small);
        add("actions/delete", "Delete", IconCategory::Actions,    IconSizeTier::Small);
        add("actions/edit",   "Edit",   IconCategory::Actions,    IconSizeTier::Small);
        add("actions/run",    "Run",    IconCategory::Actions,    IconSizeTier::Small);
        add("actions/stop",   "Stop",   IconCategory::Actions,    IconSizeTier::Small);
        // Navigation
        add("nav/back",       "Back",      IconCategory::Navigation, IconSizeTier::Small);
        add("nav/forward",    "Forward",   IconCategory::Navigation, IconSizeTier::Small);
        add("nav/home",       "Home",      IconCategory::Navigation, IconSizeTier::Small);
        add("nav/chevron_up", "ChevronUp", IconCategory::Navigation, IconSizeTier::XSmall);
        // Status
        add("status/info",    "Info",    IconCategory::Status,     IconSizeTier::Small);
        add("status/warning", "Warning", IconCategory::Status,     IconSizeTier::Small);
        add("status/error",   "Error",   IconCategory::Status,     IconSizeTier::Small);
        add("status/success", "Success", IconCategory::Status,     IconSizeTier::Small);
        // UI chrome
        add("ui/close",  "Close",  IconCategory::UI, IconSizeTier::XSmall);
        add("ui/panel",  "Panel",  IconCategory::UI, IconSizeTier::Small);
        add("ui/dock",   "Dock",   IconCategory::UI, IconSizeTier::Small);
        add("ui/float",  "Float",  IconCategory::UI, IconSizeTier::Small);
    }

private:
    std::unordered_map<std::string, IconDescriptor> m_icons;
};

// ── Iconography Spec ──────────────────────────────────────────────
// Combines grid rules, preferred tiers per surface type, and registry access.

class IconographySpec {
public:
    static constexpr uint8_t GRID_BASE_PX  = 4;   // icons snap to 4px grid
    static constexpr uint8_t PIXEL_BUDGET  = 2;   // padding budget per side

    void initialize() {
        m_registry.loadAtlasCoreDefaults();
        m_initialized = true;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    [[nodiscard]] IconRegistry& registry() { return m_registry; }
    [[nodiscard]] const IconRegistry& registry() const { return m_registry; }

    // Preferred icon size for common surface types
    [[nodiscard]] IconSizeTier toolbarIconSize()    const { return IconSizeTier::Small;  }
    [[nodiscard]] IconSizeTier panelHeaderIconSize() const { return IconSizeTier::XSmall; }
    [[nodiscard]] IconSizeTier menuIconSize()        const { return IconSizeTier::Small;  }
    [[nodiscard]] IconSizeTier notificationIconSize() const { return IconSizeTier::Medium; }

    // Validate that an icon id follows the "category/name" convention
    [[nodiscard]] static bool isValidIconId(const std::string& id) {
        auto slash = id.find('/');
        return slash != std::string::npos
            && slash > 0
            && slash < id.size() - 1;
    }

    // Snap a pixel dimension to the 4px grid
    [[nodiscard]] static uint8_t snapToGrid(uint8_t px) {
        return (px / GRID_BASE_PX) * GRID_BASE_PX;
    }

private:
    IconRegistry m_registry;
    bool         m_initialized = false;
};

} // namespace NF
