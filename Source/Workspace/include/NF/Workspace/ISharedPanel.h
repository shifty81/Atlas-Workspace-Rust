#pragma once
// NF::Workspace — ISharedPanel: interface for workspace-owned shared panels.
//
// Shared panels (Inspector, Content Browser, Diagnostics, etc.) are owned by
// the workspace, not by individual tools. Each panel implements this interface
// so the workspace can manage its lifecycle, visibility, and rendering.
//
// Panels are registered via PanelRegistry::registerPanelFactory() and created
// on demand when made visible. Tool descriptors declare which shared panels
// they support via supportedPanels.
//
// See Docs/Canon/05_EDITOR_STRATEGY.md — "Panels are shared once, reused everywhere."

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace NF {

// ── ISharedPanel ──────────────────────────────────────────────────
// Interface for workspace-owned reusable panels.

class ISharedPanel {
public:
    virtual ~ISharedPanel() = default;

    /// Unique panel identifier matching PanelRegistry entry (e.g. "inspector").
    [[nodiscard]] virtual const std::string& panelId() const = 0;

    /// Human-readable display name (e.g. "Inspector").
    [[nodiscard]] virtual const std::string& displayName() const = 0;

    // ── Lifecycle ─────────────────────────────────────────────────

    /// Called once after creation. Return false to indicate init failure.
    virtual bool initialize() { return true; }

    /// Called when the panel is being destroyed.
    virtual void shutdown() {}

    /// Called every frame when the panel is visible.
    virtual void update(float dt) { (void)dt; }

    // ── Visibility ────────────────────────────────────────────────

    [[nodiscard]] virtual bool isVisible() const { return m_visible; }
    virtual void setVisible(bool v) { m_visible = v; }

    // ── Context binding ───────────────────────────────────────────
    // Panels can optionally bind to the active tool context.

    /// Called when the active tool changes. toolId is empty if no tool active.
    virtual void onToolActivated(const std::string& toolId) { (void)toolId; }

    /// Called when the current selection changes.
    virtual void onSelectionChanged() {}

private:
    bool m_visible = true;
};

// ── SharedPanelFactory ────────────────────────────────────────────
// Factory callable that creates a panel instance on demand.

using SharedPanelFactory = std::function<std::unique_ptr<ISharedPanel>()>;

} // namespace NF
