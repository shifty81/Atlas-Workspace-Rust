#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::editor {

/// Metadata for a single discovered project.
struct ProjectEntry {
    std::string name;
    std::string path;
    std::string engineVersion;
    std::string lastOpened;
    std::string description;
};

/// Launcher / project-browser screen displayed when the editor opens
/// without a project argument.  Mimics the Unreal Engine project
/// selection experience: lists available projects, allows creating
/// new ones, and launches the full editor for the chosen project.
///
/// Layout:
///   Root
///    ├── Header  (logo, engine version)
///    ├── ProjectList
///    │    ├── ProjectCard …
///    └── Footer  (Open / New / Browse / Quit)
class LauncherScreen {
public:
    /// Scan the given root directory for project descriptors (*.atlas).
    void ScanProjects(const std::string& projectsDir);

    /// Returns the list of discovered projects.
    const std::vector<ProjectEntry>& Projects() const;

    /// Select a project by index.
    void SelectProject(size_t index);

    /// Returns the currently selected index, or SIZE_MAX if none.
    size_t SelectedIndex() const;

    /// Returns the currently selected project, or nullptr.
    const ProjectEntry* SelectedProject() const;

    /// Returns true after the user has confirmed a project choice.
    bool IsProjectChosen() const;

    /// Mark the selected project as chosen (user clicked "Open").
    void ConfirmSelection();

    /// Returns true if the user requested to create a new project.
    bool IsNewProjectRequested() const;

    /// Request creation of a new project.
    void RequestNewProject();

    /// Returns true if the user requested to quit.
    bool IsQuitRequested() const;

    /// Request quit.
    void RequestQuit();

private:
    std::vector<ProjectEntry> m_projects;
    size_t m_selected = SIZE_MAX;
    bool m_projectChosen = false;
    bool m_newProjectRequested = false;
    bool m_quitRequested = false;
};

} // namespace atlas::editor
