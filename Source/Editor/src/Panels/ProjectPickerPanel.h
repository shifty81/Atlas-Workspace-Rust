#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/project/ProjectManager.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>

namespace atlas::editor {

class ProjectPickerPanel : public EditorPanel {
public:
    const char* Name() const override { return "Project Picker"; }
    void Draw() override;

    bool HasProjectSelected() const { return m_projectSelected; }
    const std::string& SelectedProjectPath() const { return m_selectedPath; }

    void AddRecentProject(const std::string& path);
    const std::vector<std::string>& RecentProjects() const { return m_recentProjects; }

    bool OpenProject(const std::string& atlasFilePath);

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    bool m_projectSelected = false;
    std::string m_selectedPath;
    std::vector<std::string> m_recentProjects;
    project::ProjectManager m_projectManager;
    atlas::ui::UIDrawList m_drawList;
};

}
