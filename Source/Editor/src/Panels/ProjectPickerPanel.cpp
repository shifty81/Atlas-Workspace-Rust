#include "ProjectPickerPanel.h"
#include <filesystem>

namespace atlas::editor {

void ProjectPickerPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 500, 300}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 500, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Project Picker", {220, 220, 220, 255});

    // Recent projects
    m_drawList.DrawText({4, 30, 200, 16}, "Recent Projects:", {180, 180, 180, 255});
    int32_t y = 50;
    for (const auto& path : m_recentProjects) {
        m_drawList.DrawRect({4, y, 492, 22}, {40, 40, 40, 255});
        m_drawList.DrawText({8, y + 3, 484, 16}, path, {200, 200, 200, 255});
        y += 26;
    }

    // Status
    if (m_projectSelected) {
        m_drawList.DrawText({4, y + 10, 490, 16},
            "Loaded: " + m_selectedPath, {100, 255, 100, 255});
    }
}

void ProjectPickerPanel::AddRecentProject(const std::string& path) {
    for (const auto& p : m_recentProjects) {
        if (p == path) return;
    }
    m_recentProjects.push_back(path);
}

bool ProjectPickerPanel::OpenProject(const std::string& atlasFilePath) {
    if (!std::filesystem::exists(atlasFilePath)) {
        return false;
    }
    if (!m_projectManager.Load(atlasFilePath)) {
        return false;
    }
    m_selectedPath = atlasFilePath;
    m_projectSelected = true;
    AddRecentProject(atlasFilePath);
    return true;
}

}
