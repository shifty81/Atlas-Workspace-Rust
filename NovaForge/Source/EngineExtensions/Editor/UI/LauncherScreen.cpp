#include "LauncherScreen.h"
#include "../../engine/core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace atlas::editor {

// --- Minimal JSON field extraction (no external dependency) ---

static std::string ExtractJSONString(const std::string& json,
                                     const std::string& key) {
    std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return {};

    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) return {};

    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return {};

    ++pos; // skip opening quote
    auto end = json.find('"', pos);
    if (end == std::string::npos) return {};

    return json.substr(pos, end - pos);
}

static bool ParseProjectDescriptor(const std::string& filePath,
                                   ProjectEntry& entry) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;

    std::ostringstream ss;
    ss << in.rdbuf();
    std::string json = ss.str();

    std::string name = ExtractJSONString(json, "name");
    if (!name.empty()) entry.name = name;

    std::string version = ExtractJSONString(json, "version");
    if (!version.empty()) entry.engineVersion = version;

    std::string lastOpened = ExtractJSONString(json, "lastOpened");
    if (!lastOpened.empty()) entry.lastOpened = lastOpened;

    std::string description = ExtractJSONString(json, "description");
    if (!description.empty()) entry.description = description;

    return true;
}

void LauncherScreen::ScanProjects(const std::string& projectsDir) {
    m_projects.clear();
    m_selected = SIZE_MAX;
    m_projectChosen = false;

    if (!std::filesystem::is_directory(projectsDir)) {
        Logger::Warn("Projects directory not found: " + projectsDir);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(projectsDir)) {
        if (!entry.is_directory()) continue;

        std::string descriptorPath;
        for (const auto& file : std::filesystem::directory_iterator(entry.path())) {
            if (file.is_regular_file() && file.path().extension() == ".atlas") {
                descriptorPath = file.path().string();
                break;
            }
        }
        if (descriptorPath.empty()) continue;

        ProjectEntry pe;
        pe.name = entry.path().filename().string();
        pe.path = entry.path().string();
        pe.engineVersion = "0.1.0";
        pe.lastOpened = "";

        ParseProjectDescriptor(descriptorPath, pe);

        m_projects.push_back(std::move(pe));
    }

    Logger::Info("Launcher: found " + std::to_string(m_projects.size()) +
                 " project(s) in " + projectsDir);
}

const std::vector<ProjectEntry>& LauncherScreen::Projects() const {
    return m_projects;
}

void LauncherScreen::SelectProject(size_t index) {
    if (index < m_projects.size()) {
        m_selected = index;
    }
}

size_t LauncherScreen::SelectedIndex() const {
    return m_selected;
}

const ProjectEntry* LauncherScreen::SelectedProject() const {
    if (m_selected < m_projects.size()) {
        return &m_projects[m_selected];
    }
    return nullptr;
}

bool LauncherScreen::IsProjectChosen() const {
    return m_projectChosen;
}

void LauncherScreen::ConfirmSelection() {
    if (m_selected < m_projects.size()) {
        m_projectChosen = true;
    }
}

bool LauncherScreen::IsNewProjectRequested() const {
    return m_newProjectRequested;
}

void LauncherScreen::RequestNewProject() {
    m_newProjectRequested = true;
}

bool LauncherScreen::IsQuitRequested() const {
    return m_quitRequested;
}

void LauncherScreen::RequestQuit() {
    m_quitRequested = true;
}

} // namespace atlas::editor
