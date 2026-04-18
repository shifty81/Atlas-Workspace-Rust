#pragma once
// NF::Editor — Workspace app registry: authoritative catalog of child app targets
// All executables launched from AtlasWorkspace must be registered here.
// No panel or subsystem may spawn child processes outside this registry.
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── WorkspaceAppId ───────────────────────────────────────────────
// Canonical identifiers for every app that Workspace can launch.

enum class WorkspaceAppId : uint16_t {
    Unknown          = 0,
    // Project-scoped child apps
    NovaForgeEditor  = 1,
    NovaForgeGame    = 2,
    NovaForgeServer  = 3,
    // Workspace-owned standalone tools
    TileEditor       = 10,
    // Reserved range for hosted project extensions
    ProjectExtension = 100,
};

inline const char* workspaceAppName(WorkspaceAppId id) {
    switch (id) {
        case WorkspaceAppId::NovaForgeEditor:  return "NovaForgeEditor";
        case WorkspaceAppId::NovaForgeGame:    return "NovaForgeGame";
        case WorkspaceAppId::NovaForgeServer:  return "NovaForgeServer";
        case WorkspaceAppId::TileEditor:       return "TileEditor";
        case WorkspaceAppId::ProjectExtension: return "ProjectExtension";
        default:                               return "Unknown";
    }
}

// ── WorkspaceAppDescriptor ───────────────────────────────────────
// Describes a launchable child app, including its path and behaviour rules.

struct WorkspaceAppDescriptor {
    WorkspaceAppId          id               = WorkspaceAppId::Unknown;
    std::string             name;
    std::string             executablePath;
    std::vector<std::string> defaultArgs;
    bool                    isProjectScoped  = true;
    bool                    allowDirectLaunch = false;

    [[nodiscard]] bool isValid() const {
        return id != WorkspaceAppId::Unknown
            && !name.empty()
            && !executablePath.empty();
    }

    // Returns a display string including name and path
    [[nodiscard]] std::string displayLabel() const {
        return name + " (" + executablePath + ")";
    }
};

// ── WorkspaceAppRegistry ─────────────────────────────────────────
// Authoritative registry of all apps Workspace may launch.
// Workspace code must call LaunchService through this registry only.

class WorkspaceAppRegistry {
public:
    static constexpr size_t MAX_APPS = 64;

    // Register a new app descriptor.
    // Returns false if already registered or capacity reached.
    bool registerApp(WorkspaceAppDescriptor desc) {
        if (!desc.isValid()) return false;
        if (m_apps.size() >= MAX_APPS) return false;
        for (const auto& a : m_apps)
            if (a.id == desc.id) return false;
        m_apps.push_back(std::move(desc));
        return true;
    }

    // Unregister by app id. Returns false if not found.
    bool unregisterApp(WorkspaceAppId id) {
        for (auto it = m_apps.begin(); it != m_apps.end(); ++it) {
            if (it->id == id) { m_apps.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const WorkspaceAppDescriptor* find(WorkspaceAppId id) const {
        for (const auto& a : m_apps)
            if (a.id == id) return &a;
        return nullptr;
    }

    [[nodiscard]] const WorkspaceAppDescriptor* findByName(const std::string& name) const {
        for (const auto& a : m_apps)
            if (a.name == name) return &a;
        return nullptr;
    }

    [[nodiscard]] size_t count() const { return m_apps.size(); }
    [[nodiscard]] bool empty() const { return m_apps.empty(); }

    [[nodiscard]] const std::vector<WorkspaceAppDescriptor>& apps() const { return m_apps; }

    // Returns descriptors filtered to project-scoped apps only
    [[nodiscard]] std::vector<const WorkspaceAppDescriptor*> projectScopedApps() const {
        std::vector<const WorkspaceAppDescriptor*> out;
        for (const auto& a : m_apps)
            if (a.isProjectScoped) out.push_back(&a);
        return out;
    }

    // Returns true if any app with the given id is registered
    [[nodiscard]] bool isRegistered(WorkspaceAppId id) const {
        return find(id) != nullptr;
    }

private:
    std::vector<WorkspaceAppDescriptor> m_apps;
};


} // namespace NF
