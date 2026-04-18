#pragma once
// NF::Editor — ProjectSurfaceV1: lightweight project browser surface.
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │  SCOPE BOUNDARY — ProjectSurfaceV1 is a light shell/browser helper.    │
// │                                                                         │
// │  This class IS responsible for:                                         │
// │    • Recent project list                                                │
// │    • File tree shell model (project browser nodes)                      │
// │    • Lightweight project open/close metadata surface                    │
// │    • Project state transitions for browser UI                           │
// │                                                                         │
// │  This class is NOT responsible for:                                     │
// │    • Authoritative authored project data  (→ NovaForgeDocument)         │
// │    • Dirty/save truth across documents    (→ WorkspaceProjectState)     │
// │    • Panel edit binding source            (→ WorkspaceProjectState)     │
// │    • Viewport project data binding                                      │
// │                                                                         │
// │  Planned rename: ProjectBrowserSurface or ProjectMetaSurface            │
// │  Do NOT add project-data ownership or panel binding to this class.      │
// └─────────────────────────────────────────────────────────────────────────┘
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Project State ─────────────────────────────────────────────────

enum class ProjectState : uint8_t {
    Closed,
    Opening,
    Open,
    Saving,
    Error,
};

inline const char* projectStateName(ProjectState s) {
    switch (s) {
        case ProjectState::Closed:  return "Closed";
        case ProjectState::Opening: return "Opening";
        case ProjectState::Open:    return "Open";
        case ProjectState::Saving:  return "Saving";
        case ProjectState::Error:   return "Error";
    }
    return "Unknown";
}

// ── Project Metadata ──────────────────────────────────────────────

struct ProjectMetadata {
    std::string name;
    std::string version;
    std::string description;
    std::string engineVersion;
    std::string author;
    std::string rootPath;          // absolute path to project root
    std::string entryPoint;        // main scene or startup file
    std::vector<std::string> tags;

    [[nodiscard]] bool isValid() const { return !name.empty() && !rootPath.empty(); }
};

// ── Recent Project Entry ──────────────────────────────────────────

struct RecentProjectEntry {
    uint32_t    id          = 0;
    std::string name;
    std::string rootPath;
    uint64_t    lastOpenedMs = 0;  // epoch ms
    bool        exists       = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !rootPath.empty(); }
};

// ── File Tree Node ────────────────────────────────────────────────

struct ProjectFileNode {
    uint32_t                   id        = 0;
    std::string                name;
    std::string                path;      // relative to project root
    bool                       isDir     = false;
    std::vector<uint32_t>      childIds;
    uint32_t                   parentId  = 0;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRoot()   const { return parentId == 0;             }
};

// ── Project Surface V1 ────────────────────────────────────────────

using ProjectStateCallback = std::function<void(ProjectState)>;

class ProjectSurfaceV1 {
public:
    static constexpr size_t MAX_RECENT    = 20;
    static constexpr size_t MAX_FILE_NODES = 65536;

    bool openProject(const ProjectMetadata& meta) {
        if (!meta.isValid()) return false;
        m_state    = ProjectState::Opening;
        m_metadata = meta;
        m_state    = ProjectState::Open;
        ++m_openCount;
        addToRecent(meta);
        notifyState();
        return true;
    }

    void closeProject() {
        if (m_state == ProjectState::Closed) return;
        m_state    = ProjectState::Closed;
        m_metadata = {};
        m_fileNodes.clear();
        notifyState();
    }

    void saveProject() {
        if (m_state != ProjectState::Open) return;
        m_state = ProjectState::Saving;
        ++m_saveCount;
        m_state = ProjectState::Open;
        notifyState();
    }

    // File tree management
    bool addFileNode(const ProjectFileNode& node) {
        if (!node.isValid()) return false;
        if (m_fileNodes.size() >= MAX_FILE_NODES) return false;
        for (const auto& n : m_fileNodes) if (n.id == node.id) return false;
        m_fileNodes.push_back(node);
        if (node.parentId != 0) {
            for (auto& n : m_fileNodes) {
                if (n.id == node.parentId) { n.childIds.push_back(node.id); break; }
            }
        }
        return true;
    }

    bool removeFileNode(uint32_t id) {
        for (auto it = m_fileNodes.begin(); it != m_fileNodes.end(); ++it) {
            if (it->id == id) {
                m_fileNodes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] const ProjectFileNode* findFileNode(uint32_t id) const {
        for (const auto& n : m_fileNodes) if (n.id == id) return &n;
        return nullptr;
    }
    [[nodiscard]] const ProjectFileNode* findFileNodeByPath(const std::string& path) const {
        for (const auto& n : m_fileNodes) if (n.path == path) return &n;
        return nullptr;
    }

    [[nodiscard]] std::vector<uint32_t> rootFileNodes() const {
        std::vector<uint32_t> ids;
        for (const auto& n : m_fileNodes) if (n.isRoot()) ids.push_back(n.id);
        return ids;
    }

    // Recent projects
    [[nodiscard]] const std::vector<RecentProjectEntry>& recentProjects() const { return m_recent; }

    bool removeRecent(uint32_t id) {
        for (auto it = m_recent.begin(); it != m_recent.end(); ++it) {
            if (it->id == id) { m_recent.erase(it); return true; }
        }
        return false;
    }

    void clearRecent() { m_recent.clear(); }

    // Update metadata (while project is open)
    bool updateMetadata(const ProjectMetadata& meta) {
        if (m_state != ProjectState::Open) return false;
        if (!meta.isValid()) return false;
        m_metadata = meta;
        return true;
    }

    void setOnStateChange(ProjectStateCallback cb) { m_onStateChange = std::move(cb); }

    [[nodiscard]] ProjectState          state()         const { return m_state;         }
    [[nodiscard]] const ProjectMetadata& metadata()     const { return m_metadata;      }
    [[nodiscard]] bool isOpen()                         const { return m_state == ProjectState::Open; }
    [[nodiscard]] size_t openCount()                    const { return m_openCount;      }
    [[nodiscard]] size_t saveCount()                    const { return m_saveCount;      }
    [[nodiscard]] size_t fileNodeCount()                const { return m_fileNodes.size(); }
    [[nodiscard]] size_t recentCount()                  const { return m_recent.size(); }

private:
    void addToRecent(const ProjectMetadata& meta) {
        // Update existing or add new
        for (auto& r : m_recent) {
            if (r.rootPath == meta.rootPath) {
                r.lastOpenedMs = m_openCount;  // simplified timestamp
                r.name = meta.name;
                return;
            }
        }
        RecentProjectEntry e;
        e.id           = static_cast<uint32_t>(m_recent.size() + 1);
        e.name         = meta.name;
        e.rootPath     = meta.rootPath;
        e.lastOpenedMs = m_openCount;
        m_recent.push_back(e);
        if (m_recent.size() > MAX_RECENT) m_recent.erase(m_recent.begin());
    }

    void notifyState() { if (m_onStateChange) m_onStateChange(m_state); }

    ProjectMetadata               m_metadata;
    std::vector<RecentProjectEntry> m_recent;
    std::vector<ProjectFileNode>  m_fileNodes;
    ProjectStateCallback          m_onStateChange;
    ProjectState                  m_state     = ProjectState::Closed;
    size_t                        m_openCount = 0;
    size_t                        m_saveCount = 0;
};

} // namespace NF
