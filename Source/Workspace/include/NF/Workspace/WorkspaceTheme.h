#pragma once
// NF::Workspace — Phase 31: Workspace Theme System
//
// Workspace-level theme management with color tokens, theme descriptors,
// a theme registry, and an enforcer:
//   ThemeSlot      — semantic color slot enum (14 slots)
//   ThemeColorMap  — slot→RRGGBBAA color map with get/set/reset
//   ThemeDescriptor— id + displayName + author + colorMap; isValid()
//   ThemeRegistry  — named theme store; register/unregister/find/apply/active
//   ThemeEnforcer  — validates descriptors for missing or degenerate slots

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// ThemeSlot — semantic color role in the workspace UI
// ═════════════════════════════════════════════════════════════════

enum class ThemeSlot : uint8_t {
    Background,
    Surface,
    Border,
    Accent,
    AccentHover,
    AccentActive,
    TextPrimary,
    TextSecondary,
    TextDisabled,
    IconPrimary,
    IconSecondary,
    SelectionHighlight,
    ErrorColor,
    WarningColor,
};

inline const char* themeSlotName(ThemeSlot s) {
    switch (s) {
        case ThemeSlot::Background:         return "Background";
        case ThemeSlot::Surface:            return "Surface";
        case ThemeSlot::Border:             return "Border";
        case ThemeSlot::Accent:             return "Accent";
        case ThemeSlot::AccentHover:        return "AccentHover";
        case ThemeSlot::AccentActive:       return "AccentActive";
        case ThemeSlot::TextPrimary:        return "TextPrimary";
        case ThemeSlot::TextSecondary:      return "TextSecondary";
        case ThemeSlot::TextDisabled:       return "TextDisabled";
        case ThemeSlot::IconPrimary:        return "IconPrimary";
        case ThemeSlot::IconSecondary:      return "IconSecondary";
        case ThemeSlot::SelectionHighlight: return "SelectionHighlight";
        case ThemeSlot::ErrorColor:         return "ErrorColor";
        case ThemeSlot::WarningColor:       return "WarningColor";
        default:                            return "Unknown";
    }
}

static constexpr int kThemeSlotCount = 14;

// ═════════════════════════════════════════════════════════════════
// ThemeColorMap — slot-indexed RRGGBBAA color table
// ═════════════════════════════════════════════════════════════════

struct ThemeColorMap {
    static constexpr uint32_t kDefaultColor = 0xFF000000u; // opaque black

    uint32_t colors[kThemeSlotCount] = {};
    bool     defined[kThemeSlotCount] = {};

    ThemeColorMap() {
        for (int i = 0; i < kThemeSlotCount; ++i) {
            colors[i]  = kDefaultColor;
            defined[i] = false;
        }
    }

    void set(ThemeSlot s, uint32_t rrggbbaa) {
        int idx = static_cast<int>(s);
        if (idx < 0 || idx >= kThemeSlotCount) return;
        colors[idx]  = rrggbbaa;
        defined[idx] = true;
    }

    uint32_t get(ThemeSlot s) const {
        int idx = static_cast<int>(s);
        if (idx < 0 || idx >= kThemeSlotCount) return kDefaultColor;
        return colors[idx];
    }

    bool isDefined(ThemeSlot s) const {
        int idx = static_cast<int>(s);
        if (idx < 0 || idx >= kThemeSlotCount) return false;
        return defined[idx];
    }

    void reset(ThemeSlot s) {
        int idx = static_cast<int>(s);
        if (idx < 0 || idx >= kThemeSlotCount) return;
        colors[idx]  = kDefaultColor;
        defined[idx] = false;
    }

    void resetAll() {
        for (int i = 0; i < kThemeSlotCount; ++i) {
            colors[i]  = kDefaultColor;
            defined[i] = false;
        }
    }

    int definedCount() const {
        int n = 0;
        for (int i = 0; i < kThemeSlotCount; ++i)
            if (defined[i]) ++n;
        return n;
    }

    bool allDefined() const { return definedCount() == kThemeSlotCount; }
};

// ═════════════════════════════════════════════════════════════════
// ThemeDescriptor — metadata + color map for one named theme
// ═════════════════════════════════════════════════════════════════

struct ThemeDescriptor {
    std::string    id;
    std::string    displayName;
    std::string    author;
    ThemeColorMap  colorMap;
    bool           isBuiltIn = false;

    bool isValid() const { return !id.empty() && !displayName.empty(); }
};

// ═════════════════════════════════════════════════════════════════
// ThemeViolation — a single enforcement finding
// ═════════════════════════════════════════════════════════════════

struct ThemeViolation {
    ThemeSlot   slot    = ThemeSlot::Background;
    std::string message;
};

// ═════════════════════════════════════════════════════════════════
// ThemeEnforcementReport — result of validating a ThemeDescriptor
// ═════════════════════════════════════════════════════════════════

struct ThemeEnforcementReport {
    bool                       passed     = true;
    std::vector<ThemeViolation> violations;

    void addViolation(ThemeSlot s, const std::string& msg) {
        violations.push_back({s, msg});
        passed = false;
    }

    int violationCount() const { return static_cast<int>(violations.size()); }
};

// ═════════════════════════════════════════════════════════════════
// ThemeEnforcer — validates a ThemeDescriptor
// ═════════════════════════════════════════════════════════════════

struct ThemeEnforcer {
    // Check that every slot is explicitly defined
    ThemeEnforcementReport enforce(const ThemeDescriptor& desc) const {
        ThemeEnforcementReport report;
        if (!desc.isValid()) {
            report.addViolation(ThemeSlot::Background, "ThemeDescriptor is invalid (empty id or displayName)");
            return report;
        }
        for (int i = 0; i < kThemeSlotCount; ++i) {
            auto slot = static_cast<ThemeSlot>(i);
            if (!desc.colorMap.isDefined(slot)) {
                std::string msg = std::string("Slot '") + themeSlotName(slot) + "' is not defined";
                report.addViolation(slot, msg);
            }
        }
        return report;
    }
};

// ═════════════════════════════════════════════════════════════════
// ThemeRegistry — stores named themes; tracks active theme
// ═════════════════════════════════════════════════════════════════

struct ThemeRegistry {
    static constexpr int kMaxThemes = 64;

    using ObserverFn = std::function<void(const std::string& /*themeId*/)>;

    bool registerTheme(const ThemeDescriptor& desc) {
        if (!desc.isValid()) return false;
        if (static_cast<int>(themes_.size()) >= kMaxThemes) return false;
        for (auto& t : themes_)
            if (t.id == desc.id) return false; // duplicate
        themes_.push_back(desc);
        return true;
    }

    bool unregisterTheme(const std::string& id) {
        if (id == activeId_) return false; // cannot remove active
        auto it = std::find_if(themes_.begin(), themes_.end(),
            [&](const ThemeDescriptor& t){ return t.id == id; });
        if (it == themes_.end()) return false;
        themes_.erase(it);
        return true;
    }

    const ThemeDescriptor* find(const std::string& id) const {
        for (auto& t : themes_)
            if (t.id == id) return &t;
        return nullptr;
    }

    bool contains(const std::string& id) const { return find(id) != nullptr; }

    bool applyTheme(const std::string& id) {
        if (!contains(id)) return false;
        std::string prev = activeId_;
        activeId_ = id;
        for (auto& obs : observers_)
            if (obs) obs(activeId_);
        return true;
    }

    const std::string& activeThemeId() const { return activeId_; }

    const ThemeDescriptor* activeTheme() const { return find(activeId_); }

    int count() const { return static_cast<int>(themes_.size()); }

    bool empty() const { return themes_.empty(); }

    std::vector<std::string> allIds() const {
        std::vector<std::string> ids;
        ids.reserve(themes_.size());
        for (auto& t : themes_) ids.push_back(t.id);
        return ids;
    }

    void clear() {
        themes_.clear();
        activeId_.clear();
    }

    bool addObserver(ObserverFn fn) {
        if (static_cast<int>(observers_.size()) >= kMaxObservers) return false;
        observers_.push_back(std::move(fn));
        return true;
    }

    void clearObservers() { observers_.clear(); }

private:
    static constexpr int kMaxObservers = 16;
    std::vector<ThemeDescriptor> themes_;
    std::string                  activeId_;
    std::vector<ObserverFn>      observers_;
};

} // namespace NF
