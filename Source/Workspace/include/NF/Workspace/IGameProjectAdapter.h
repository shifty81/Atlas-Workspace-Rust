#pragma once
// NF::IGameProjectAdapter — Contract for hosted projects.
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │  OWNERSHIP BOUNDARY — IGameProjectAdapter is an integration bridge.     │
// │                                                                         │
// │  Adapter owns:                                                           │
// │    • Project identity (projectId, displayName)                          │
// │    • Project-specific panel descriptors                                 │
// │    • Project-specific content roots and custom commands                 │
// │    • Adapter lifecycle (initialize / shutdown)                          │
// │                                                                         │
// │  Adapter does NOT own:                                                   │
// │    • Full mutable project session state (→ WorkspaceProjectState)       │
// │    • Global save orchestration                                           │
// │    • Document persistence internals (→ NovaForgeDocument)               │
// │                                                                         │
// │  Keep adapters thin. They are integration bridges, not state owners.    │
// └─────────────────────────────────────────────────────────────────────────┘
//
// This interface defines how a hosted project (e.g., NovaForge) integrates
// with the Atlas Workspace. Projects implement this interface to register
// their extensions, panels, schemas, and commands.
//
// See Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md for boundary rules.

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace NF {

// ── IEditorPanel ────────────────────────────────────────────────
// Base interface for all project-specific editor panels.
// Panels are created by factory functions stored in
// GameplaySystemPanelDescriptor::createPanel and cached by ProjectSystemsTool.

class IEditorPanel {
public:
    virtual ~IEditorPanel() = default;

    // Panel identity
    [[nodiscard]] virtual const std::string& panelId()    const = 0;
    [[nodiscard]] virtual const std::string& panelTitle() const = 0;

    // Lifecycle hooks — called when the host project is loaded/unloaded.
    virtual void onProjectLoaded(const std::string& /*projectRoot*/) {}
    virtual void onProjectUnloaded() {}

    // Called once per frame when the panel is visible and active.
    virtual void update(float /*dt*/) {}

    // Status — override to expose panel-specific health or state.
    [[nodiscard]] virtual bool isReady() const { return true; }

    // Returns a list of (label, value) rows that the workspace renderer uses
    // to display real panel data in the project-systems dashboard.
    // Override in concrete panels to surface meaningful authoring state.
    // Each row is rendered as a property-grid line: "label   value".
    [[nodiscard]] virtual std::vector<std::pair<std::string, std::string>>
        summaryRows() const { return {}; }
};

// ── Panel descriptor ────────────────────────────────────────────
// Describes a project-specific panel that should be hosted inside
// a workspace tool. The workspace uses these descriptors to insert
// project panels into the correct host tool by category.

struct GameplaySystemPanelDescriptor {
    std::string id;           // unique panel identifier
    std::string displayName;  // user-facing panel title
    std::string hostToolId;   // which workspace tool hosts this panel
    std::string category;     // grouping within the host tool
    std::string projectId;    // owning project identifier
    bool defaultVisible = false;
    bool enabled = true;
    std::function<std::unique_ptr<IEditorPanel>()> createPanel;
};

// ── Project adapter interface ───────────────────────────────────
// Workspace calls this to discover and load project-specific extensions.

class IGameProjectAdapter {
public:
    virtual ~IGameProjectAdapter() = default;

    // Project identity
    virtual std::string projectId() const = 0;
    virtual std::string projectDisplayName() const = 0;

    // Absolute path to the project's root directory (the folder that contains
    // the .atlas file and all sub-trees).  Returns an empty string by default;
    // adapters that know their on-disk location should override this.
    virtual std::string projectRoot() const { return {}; }

    // Extension registration
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Panel provider — returns descriptors for all project-specific panels
    virtual std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const = 0;

    // Schema/validation hooks (optional)
    virtual std::vector<std::string> contentRoots() const { return {}; }
    virtual std::vector<std::string> customCommands() const { return {}; }
};

// ── Panel factory interface ─────────────────────────────────────
// Creates project-specific panels on demand.

class IProjectPanelFactory {
public:
    virtual ~IProjectPanelFactory() = default;
    virtual std::unique_ptr<IEditorPanel> createPanel(const std::string& panelId) = 0;
    virtual std::vector<std::string> availablePanelIds() const = 0;
};

// ── Schema provider interface ───────────────────────────────────
// Supplies project-specific schemas for data validation and editing.

class IGameplaySystemSchemaProvider {
public:
    virtual ~IGameplaySystemSchemaProvider() = default;
    virtual std::vector<std::string> schemaIds() const = 0;
    virtual std::string schemaDefinition(const std::string& id) const = 0;
};

} // namespace NF
