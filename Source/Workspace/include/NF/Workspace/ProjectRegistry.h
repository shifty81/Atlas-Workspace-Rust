#pragma once
// NF::Workspace — ProjectRegistry: multi-project plugin/factory registry.
//
// ProjectRegistry is the plugin model for hosted projects in Atlas Workspace.
// Projects register a factory (by project ID) and the registry manages
// discovery, instantiation, and lifecycle of adapters.
//
// Design principles:
//   - Mirrors ToolRegistry's factory pattern — one factory per project ID.
//   - At most one project may be loaded (active) at a time.
//   - Registration may happen before or after initialize(); loading always
//     requires the registry to be in the Running state.
//   - Produces a ProjectLoadContract for each load attempt.
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 5.

#include "NF/Workspace/IGameProjectAdapter.h"
#include "NF/Workspace/ProjectLoadContract.h"
#include "NF/Workspace/AtlasProjectFileLoader.h"
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Project factory type ──────────────────────────────────────────────────
// Callable that constructs and returns a new IGameProjectAdapter.

using ProjectFactory = std::function<std::unique_ptr<IGameProjectAdapter>()>;

// ── Registry lifecycle state ──────────────────────────────────────────────

enum class ProjectRegistryState : uint8_t {
    Idle,      // not initialized
    Running,   // ready to load projects
    Stopped,   // shut down
};

// ── ProjectRegistry ───────────────────────────────────────────────────────

class ProjectRegistry {
public:
    // ── Lifecycle ─────────────────────────────────────────────────

    void initialize() {
        if (m_state == ProjectRegistryState::Idle)
            m_state = ProjectRegistryState::Running;
    }

    void shutdown() {
        unloadActive();
        m_state = ProjectRegistryState::Stopped;
    }

    [[nodiscard]] ProjectRegistryState state() const { return m_state; }
    [[nodiscard]] bool isRunning() const { return m_state == ProjectRegistryState::Running; }

    // ── Factory registration ──────────────────────────────────────
    // Factories may be registered in any lifecycle state.

    bool registerProject(const std::string& projectId, ProjectFactory factory) {
        if (projectId.empty() || !factory) return false;
        if (m_factories.count(projectId)) return false; // already registered
        m_factories[projectId] = std::move(factory);
        return true;
    }

    [[nodiscard]] bool isRegistered(const std::string& projectId) const {
        return m_factories.count(projectId) > 0;
    }

    [[nodiscard]] std::vector<std::string> registeredProjectIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_factories.size());
        for (const auto& kv : m_factories)
            ids.push_back(kv.first);
        return ids;
    }

    [[nodiscard]] uint32_t registeredCount() const {
        return static_cast<uint32_t>(m_factories.size());
    }

    // ── Loading ───────────────────────────────────────────────────

    /// Load a project by ID. Returns the resulting contract.
    /// Only one project may be active at a time — loading a new one
    /// unloads the previous one first.
    ProjectLoadContract loadProject(const std::string& projectId) {
        ProjectLoadContract contract;
        contract.projectId = projectId;

        if (m_state != ProjectRegistryState::Running) {
            contract.state = ProjectLoadState::Failed;
            contract.addFatal("registry_not_running",
                "ProjectRegistry must be initialized before loading a project.");
            return contract;
        }

        auto it = m_factories.find(projectId);
        if (it == m_factories.end()) {
            contract.state = ProjectLoadState::Failed;
            contract.addFatal("unknown_project_id",
                "No factory registered for project '" + projectId + "'.");
            return contract;
        }

        // Unload any currently active project first
        unloadActive();

        contract.state = ProjectLoadState::Loading;

        auto adapter = it->second();
        if (!adapter) {
            contract.state = ProjectLoadState::Failed;
            contract.addFatal("factory_returned_null",
                "Factory for project '" + projectId + "' returned a null adapter.");
            return contract;
        }

        contract.projectDisplayName = adapter->projectDisplayName();
        contract.contentRoots       = adapter->contentRoots();
        contract.customCommands     = adapter->customCommands();
        contract.panelCount         = static_cast<uint32_t>(adapter->panelDescriptors().size());
        contract.loadTimestampMs    = nowMs();

        if (!adapter->initialize()) {
            contract.state = ProjectLoadState::Failed;
            contract.addFatal("adapter_init_failed",
                "IGameProjectAdapter::initialize() returned false for '" + projectId + "'.");
            return contract;
        }

        // Validate content roots are non-empty
        if (contract.contentRoots.empty()) {
            contract.addWarning("no_content_roots",
                "Project '" + projectId + "' declared no content roots.");
        }

        contract.state   = ProjectLoadState::Ready;
        m_activeAdapter  = std::move(adapter);
        m_activeContract = contract;
        return contract;
    }

    /// Load a project by reading an .atlas project file.
    /// Parses the file, extracts the 'adapter' field, and delegates to loadProject().
    /// Returns a failed contract if the file cannot be parsed or has no 'adapter' field.
    ProjectLoadContract loadProjectFromAtlasFile(const std::string& atlasFilePath) {
        ProjectLoadContract contract;

        AtlasProjectFileLoader loader;
        if (!loader.loadFromFile(atlasFilePath)) {
            contract.state = ProjectLoadState::Failed;
            contract.addFatal("atlas_file_parse_error",
                "Failed to parse .atlas file '" + atlasFilePath + "': " + loader.error());
            return contract;
        }

        const AtlasProjectManifest& manifest = loader.manifest();
        contract.projectId          = manifest.adapterId;
        contract.projectDisplayName = manifest.name;
        contract.projectVersion     = manifest.version;
        contract.contentRoots       = { manifest.contentRoot };

        if (!manifest.hasAdapter()) {
            contract.state = ProjectLoadState::Failed;
            contract.addFatal("missing_adapter_field",
                "The .atlas file '" + atlasFilePath
                + "' does not specify an 'adapter' field. "
                  "Add '\"adapter\": \"<project-id>\"' to the file.");
            return contract;
        }

        return loadProject(manifest.adapterId);
    }

    /// Unload the currently active project (no-op if none active).
    void unloadActive() {
        if (m_activeAdapter) {
            m_activeAdapter->shutdown();
            m_activeAdapter.reset();
        }
        m_activeContract = std::nullopt;
    }

    // ── Active project accessors ──────────────────────────────────

    [[nodiscard]] bool hasActiveProject() const {
        return m_activeAdapter != nullptr;
    }

    [[nodiscard]] const IGameProjectAdapter* activeAdapter() const {
        return m_activeAdapter.get();
    }

    [[nodiscard]] const std::optional<ProjectLoadContract>& activeContract() const {
        return m_activeContract;
    }

    [[nodiscard]] std::string activeProjectId() const {
        return m_activeAdapter ? m_activeAdapter->projectId() : std::string{};
    }

private:
    static int64_t nowMs() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count();
    }

    ProjectRegistryState      m_state = ProjectRegistryState::Idle;
    std::unordered_map<std::string, ProjectFactory> m_factories;
    std::unique_ptr<IGameProjectAdapter>            m_activeAdapter;
    std::optional<ProjectLoadContract>              m_activeContract;
};

} // namespace NF
