#pragma once
// NF::Editor — Phase H.3: Layout Persistence Controller
//
// Manages named workspace layout presets including save, load, rename, delete,
// built-in presets, and automatic persistence on close / restore on open.
//
//   BuiltInLayoutPreset   — enum for the three built-in presets
//   LayoutPersistenceController —
//       savePreset(name, layout)        — save current layout as named preset
//       loadPreset(name, outLayout)     — restore layout from named preset
//       renamePreset(old, new)          — rename an existing preset
//       deletePreset(name)              — remove a preset (cannot delete built-ins)
//       hasPreset(name)                 — check existence
//       presetCount() / presetNames()   — enumerate saved presets
//       saveBuiltIns()                  — populate the three built-in presets
//       loadBuiltIn(preset, outLayout)  — restore a built-in preset
//       setAutoSaveEnabled(bool)        — toggle auto-save on close
//       onBeforeClose(layout)           — call to save current layout (if auto-save on)
//       onAfterOpen(outLayout)          — call to restore last-used layout

#include "NF/Workspace/LayoutPersistence.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// BuiltInLayoutPreset
// ═════════════════════════════════════════════════════════════════

enum class BuiltInLayoutPreset : uint8_t {
    Default = 0,
    Compact = 1,
    Wide    = 2,
};

inline const char* builtInLayoutPresetName(BuiltInLayoutPreset p) {
    switch (p) {
        case BuiltInLayoutPreset::Default: return "Default";
        case BuiltInLayoutPreset::Compact: return "Compact";
        case BuiltInLayoutPreset::Wide:    return "Wide";
        default:                           return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// LayoutPersistenceController
// ═════════════════════════════════════════════════════════════════

class LayoutPersistenceController {
public:
    static constexpr size_t MAX_USER_PRESETS   = 32;
    static constexpr size_t BUILTIN_COUNT      = 3;

    LayoutPersistenceController() {
        // Seed the three built-in presets immediately.
        seedBuiltIns();
    }

    // ── Save / load user presets ──────────────────────────────────

    bool savePreset(const std::string& name, const WorkspaceLayout& layout) {
        if (name.empty()) return false;
        if (isBuiltInName(name)) return false;  // cannot overwrite built-ins
        if (userPresetCount() >= MAX_USER_PRESETS && !hasPreset(name)) return false;
        return m_manager.savePreset(name, layout);
    }

    bool loadPreset(const std::string& name, WorkspaceLayout& outLayout) const {
        const LayoutPreset* preset = m_manager.findPreset(name);
        if (!preset) {
            // Try built-ins
            for (const auto& bi : m_builtIns) {
                if (bi.name == name) {
                    return LayoutSerializer::deserialize(bi.serializedData, outLayout);
                }
            }
            return false;
        }
        return LayoutSerializer::deserialize(preset->serializedData, outLayout);
    }

    bool renamePreset(const std::string& oldName, const std::string& newName) {
        if (oldName.empty() || newName.empty()) return false;
        if (isBuiltInName(oldName)) return false;
        if (hasPreset(newName)) return false;
        return m_manager.renamePreset(oldName, newName);
    }

    bool deletePreset(const std::string& name) {
        if (isBuiltInName(name)) return false;
        return m_manager.removePreset(name);
    }

    [[nodiscard]] bool hasPreset(const std::string& name) const {
        if (m_manager.findPreset(name)) return true;
        for (const auto& bi : m_builtIns)
            if (bi.name == name) return true;
        return false;
    }

    // ── Preset enumeration ────────────────────────────────────────

    [[nodiscard]] size_t userPresetCount() const { return m_manager.userCount(); }
    [[nodiscard]] size_t totalPresetCount() const { return userPresetCount() + BUILTIN_COUNT; }

    [[nodiscard]] std::vector<std::string> userPresetNames() const {
        std::vector<std::string> names;
        for (const auto& p : m_manager.presets())
            if (!p.isBuiltIn) names.push_back(p.name);
        return names;
    }

    [[nodiscard]] std::vector<std::string> allPresetNames() const {
        std::vector<std::string> names;
        for (const auto& bi : m_builtIns) names.push_back(bi.name);
        for (const auto& p : m_manager.presets())
            if (!p.isBuiltIn) names.push_back(p.name);
        return names;
    }

    // ── Built-in presets ──────────────────────────────────────────

    [[nodiscard]] bool loadBuiltIn(BuiltInLayoutPreset preset, WorkspaceLayout& outLayout) const {
        const char* name = builtInLayoutPresetName(preset);
        for (const auto& bi : m_builtIns) {
            if (bi.name == name) {
                return LayoutSerializer::deserialize(bi.serializedData, outLayout);
            }
        }
        return false;
    }

    // ── Auto-save on close / restore on open ─────────────────────

    void setAutoSaveEnabled(bool enabled) { m_autoSaveEnabled = enabled; }
    [[nodiscard]] bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }

    bool onBeforeClose(const WorkspaceLayout& layout) {
        if (!m_autoSaveEnabled) return false;
        return m_manager.savePreset(kAutoSavePresetName, layout);
    }

    bool onAfterOpen(WorkspaceLayout& outLayout) const {
        return loadPreset(kAutoSavePresetName, outLayout);
    }

    // ── Callbacks ─────────────────────────────────────────────────

    void setOnPresetSaved(std::function<void(const std::string&)> cb) {
        m_onPresetSaved = std::move(cb);
    }
    void setOnPresetLoaded(std::function<void(const std::string&)> cb) {
        m_onPresetLoaded = std::move(cb);
    }

private:
    static constexpr const char* kAutoSavePresetName = "__autosave__";

    LayoutPersistenceManager          m_manager;
    std::vector<LayoutPreset>         m_builtIns;
    bool                              m_autoSaveEnabled = true;
    std::function<void(const std::string&)> m_onPresetSaved;
    std::function<void(const std::string&)> m_onPresetLoaded;

    [[nodiscard]] static bool isBuiltInName(const std::string& name) {
        return name == builtInLayoutPresetName(BuiltInLayoutPreset::Default)
            || name == builtInLayoutPresetName(BuiltInLayoutPreset::Compact)
            || name == builtInLayoutPresetName(BuiltInLayoutPreset::Wide);
    }

    void seedBuiltIns() {
        // Default — standard 4-panel layout
        {
            WorkspaceLayout def("Default");
            LayoutPanel vp; vp.id = "viewport"; vp.title = "Viewport";
            vp.type = LayoutPanelType::Viewport; vp.width = 800.f; vp.height = 600.f;
            def.addPanel(vp);
            LayoutPanel ins; ins.id = "inspector"; ins.title = "Inspector";
            ins.type = LayoutPanelType::Inspector; ins.dockZone = LayoutDockZone::Right;
            ins.width = 320.f; ins.height = 600.f;
            def.addPanel(ins);
            LayoutPreset p;
            p.name = "Default";
            p.serializedData = LayoutSerializer::serialize(def);
            p.isBuiltIn = true;
            m_builtIns.push_back(p);
        }
        // Compact — minimal layout
        {
            WorkspaceLayout compact("Compact");
            LayoutPanel vp; vp.id = "viewport"; vp.title = "Viewport";
            vp.type = LayoutPanelType::Viewport; vp.width = 1024.f; vp.height = 768.f;
            compact.addPanel(vp);
            LayoutPreset p;
            p.name = "Compact";
            p.serializedData = LayoutSerializer::serialize(compact);
            p.isBuiltIn = true;
            m_builtIns.push_back(p);
        }
        // Wide — multi-panel widescreen layout
        {
            WorkspaceLayout wide("Wide");
            LayoutPanel vp; vp.id = "viewport"; vp.title = "Viewport";
            vp.type = LayoutPanelType::Viewport; vp.width = 1200.f; vp.height = 800.f;
            wide.addPanel(vp);
            LayoutPanel cb; cb.id = "content"; cb.title = "Content Browser";
            cb.type = LayoutPanelType::ContentBrowser; cb.dockZone = LayoutDockZone::Bottom;
            cb.width = 1200.f; cb.height = 200.f;
            wide.addPanel(cb);
            LayoutPanel hier; hier.id = "hierarchy"; hier.title = "Hierarchy";
            hier.type = LayoutPanelType::Hierarchy; hier.dockZone = LayoutDockZone::Left;
            hier.width = 240.f; hier.height = 800.f;
            wide.addPanel(hier);
            LayoutPreset p;
            p.name = "Wide";
            p.serializedData = LayoutSerializer::serialize(wide);
            p.isBuiltIn = true;
            m_builtIns.push_back(p);
        }
    }
};

} // namespace NF
