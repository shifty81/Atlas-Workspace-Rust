#pragma once
// NF::Workspace — PanelRegistry: shared panel registration and context binding.
//
// Shared panels (inspector, outliner, console, content browser, etc.) are
// registered here by panel-id. Tools declare which panels they support via
// their HostedToolDescriptor::supportedPanels list. The registry tracks
// visibility and allows category-based queries.
//
// Panels can optionally have a factory (SharedPanelFactory) for lazy creation
// of ISharedPanel instances. The workspace shell creates panel instances on
// demand when they become visible or needed.
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the shared-panel policy.

#include "NF/Workspace/ISharedPanel.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Shared Panel Category ─────────────────────────────────────────

enum class SharedPanelCategory : uint8_t {
    Inspector,       // property inspection / details
    Navigation,      // outliner, scene tree, content browser
    Output,          // console, log, diagnostics, profiler
    AI,              // AtlasAI chat, code suggestions
    Preview,         // asset preview, viewport
    Editing,         // command palette, search, find/replace
    Status,          // notifications, progress bars
    Custom,          // project-specific (injected through adapter)
};

inline const char* sharedPanelCategoryName(SharedPanelCategory c) {
    switch (c) {
        case SharedPanelCategory::Inspector:  return "Inspector";
        case SharedPanelCategory::Navigation: return "Navigation";
        case SharedPanelCategory::Output:     return "Output";
        case SharedPanelCategory::AI:         return "AI";
        case SharedPanelCategory::Preview:    return "Preview";
        case SharedPanelCategory::Editing:    return "Editing";
        case SharedPanelCategory::Status:     return "Status";
        case SharedPanelCategory::Custom:     return "Custom";
    }
    return "Unknown";
}

// ── Shared Panel Descriptor ───────────────────────────────────────

struct SharedPanelDescriptor {
    std::string          panelId;      // e.g. "inspector", "console", "content_browser"
    std::string          displayName;  // e.g. "Inspector"
    SharedPanelCategory  category = SharedPanelCategory::Inspector;
    bool                 defaultVisible = true;

    [[nodiscard]] bool isValid() const { return !panelId.empty() && !displayName.empty(); }
};

// ── PanelRegistry ─────────────────────────────────────────────────

class PanelRegistry {
public:
    static constexpr size_t MAX_PANELS = 64;

    // ── Registration ──────────────────────────────────────────────

    bool registerPanel(SharedPanelDescriptor desc) {
        if (!desc.isValid()) return false;
        if (m_panels.size() >= MAX_PANELS) return false;
        for (const auto& p : m_panels)
            if (p.desc.panelId == desc.panelId) return false;
        Entry e;
        e.desc    = std::move(desc);
        e.visible = e.desc.defaultVisible;
        m_panels.push_back(std::move(e));
        return true;
    }

    bool unregisterPanel(const std::string& panelId) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const Entry& e) { return e.desc.panelId == panelId; });
        if (it == m_panels.end()) return false;
        m_panels.erase(it);
        return true;
    }

    // ── Lookup ────────────────────────────────────────────────────

    [[nodiscard]] const SharedPanelDescriptor* find(const std::string& panelId) const {
        for (const auto& e : m_panels)
            if (e.desc.panelId == panelId) return &e.desc;
        return nullptr;
    }

    [[nodiscard]] bool isRegistered(const std::string& panelId) const {
        return find(panelId) != nullptr;
    }

    [[nodiscard]] size_t count() const { return m_panels.size(); }
    [[nodiscard]] bool   empty() const { return m_panels.empty(); }

    // ── Filtered queries ──────────────────────────────────────────

    [[nodiscard]] std::vector<const SharedPanelDescriptor*> byCategory(
        SharedPanelCategory cat) const
    {
        std::vector<const SharedPanelDescriptor*> out;
        for (const auto& e : m_panels)
            if (e.desc.category == cat) out.push_back(&e.desc);
        return out;
    }

    [[nodiscard]] std::vector<const SharedPanelDescriptor*> visiblePanels() const {
        std::vector<const SharedPanelDescriptor*> out;
        for (const auto& e : m_panels)
            if (e.visible) out.push_back(&e.desc);
        return out;
    }

    [[nodiscard]] std::vector<const SharedPanelDescriptor*> allDescriptors() const {
        std::vector<const SharedPanelDescriptor*> out;
        for (const auto& e : m_panels) out.push_back(&e.desc);
        return out;
    }

    // ── Visibility ────────────────────────────────────────────────

    bool setPanelVisible(const std::string& panelId, bool visible) {
        for (auto& e : m_panels) {
            if (e.desc.panelId == panelId) { e.visible = visible; return true; }
        }
        return false;
    }

    // Convenience aliases for setPanelVisible.
    bool setVisible(const std::string& panelId, bool visible) {
        return setPanelVisible(panelId, visible);
    }

    bool toggleVisible(const std::string& panelId) {
        for (auto& e : m_panels) {
            if (e.desc.panelId == panelId) { e.visible = !e.visible; return true; }
        }
        return false;
    }

    [[nodiscard]] bool isPanelVisible(const std::string& panelId) const {
        for (const auto& e : m_panels)
            if (e.desc.panelId == panelId) return e.visible;
        return false;
    }

    // ── Context binding ───────────────────────────────────────────
    // Returns panels that are supported by a given tool (by matching tool
    // supportedPanels list against registered panels).

    [[nodiscard]] std::vector<const SharedPanelDescriptor*> panelsForTool(
        const std::vector<std::string>& supportedPanelIds) const
    {
        std::vector<const SharedPanelDescriptor*> out;
        for (const auto& id : supportedPanelIds) {
            if (const auto* d = find(id)) out.push_back(d);
        }
        return out;
    }

    // ── Factory registration ─────────────────────────────────────
    // Register a panel with its factory and descriptor in one call.

    bool registerPanelWithFactory(SharedPanelDescriptor desc, SharedPanelFactory factory) {
        if (!desc.isValid() || !factory) return false;
        if (m_panels.size() >= MAX_PANELS) return false;
        for (const auto& p : m_panels)
            if (p.desc.panelId == desc.panelId) return false;
        Entry e;
        e.desc    = std::move(desc);
        e.visible = e.desc.defaultVisible;
        e.factory = std::move(factory);
        m_panels.push_back(std::move(e));
        return true;
    }

    // ── Panel instance access ────────────────────────────────────
    // Create or return the existing panel instance for the given panelId.
    // Returns nullptr if the panel has no factory or panelId is not registered.

    ISharedPanel* getOrCreatePanel(const std::string& panelId) {
        for (auto& e : m_panels) {
            if (e.desc.panelId == panelId) {
                if (!e.instance && e.factory) {
                    e.instance = e.factory();
                    if (e.instance) e.instance->initialize();
                }
                return e.instance.get();
            }
        }
        return nullptr;
    }

    [[nodiscard]] ISharedPanel* findPanel(const std::string& panelId) {
        for (auto& e : m_panels)
            if (e.desc.panelId == panelId) return e.instance.get();
        return nullptr;
    }

    [[nodiscard]] const ISharedPanel* findPanel(const std::string& panelId) const {
        for (const auto& e : m_panels)
            if (e.desc.panelId == panelId) return e.instance.get();
        return nullptr;
    }

    [[nodiscard]] bool hasFactory(const std::string& panelId) const {
        for (const auto& e : m_panels)
            if (e.desc.panelId == panelId) return e.factory != nullptr;
        return false;
    }

    [[nodiscard]] bool hasInstance(const std::string& panelId) const {
        for (const auto& e : m_panels)
            if (e.desc.panelId == panelId) return e.instance != nullptr;
        return false;
    }

    /// Notify all created panel instances that the active tool changed.
    void notifyToolActivated(const std::string& toolId) {
        for (auto& e : m_panels)
            if (e.instance) e.instance->onToolActivated(toolId);
    }

    /// Notify all created panel instances that the selection changed.
    void notifySelectionChanged() {
        for (auto& e : m_panels)
            if (e.instance) e.instance->onSelectionChanged();
    }

    /// Shut down and destroy all panel instances. Called during workspace shutdown.
    void shutdownAll() {
        for (auto& e : m_panels) {
            if (e.instance) {
                e.instance->shutdown();
                e.instance.reset();
            }
        }
    }

private:
    struct Entry {
        SharedPanelDescriptor desc;
        bool visible = true;
        SharedPanelFactory factory;
        std::unique_ptr<ISharedPanel> instance;
    };

    std::vector<Entry> m_panels;
};

} // namespace NF
