#pragma once
// NF::Editor — Project path, launch, command registry, recent files
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Workspace/SelectionService.h"

namespace NF {

class ProjectPathService {
public:
    static constexpr const char* kProjectFileName = "atlas.project.json";

    void init(const std::string& executablePath) {
        m_executablePath = executablePath;

        // Derive project root from executable path
        auto exePath = std::filesystem::path(executablePath);
        // Walk up from bin/ or Builds/ to find project root
        constexpr int kMaxProjectRootSearchDepth = 5;
        auto dir = exePath.parent_path();
        for (int i = 0; i < kMaxProjectRootSearchDepth; ++i) {
            if (std::filesystem::exists(dir / "Config" / kProjectFileName)) {
                m_projectRoot = dir.string();
                break;
            }
            dir = dir.parent_path();
        }

        if (m_projectRoot.empty()) {
            m_projectRoot = std::filesystem::current_path().string();
        }

        m_contentPath = (std::filesystem::path(m_projectRoot) / "Content").string();
        m_dataPath = (std::filesystem::path(m_projectRoot) / "Data").string();
        m_configPath = (std::filesystem::path(m_projectRoot) / "Config").string();

        NF_LOG_INFO("Editor", "Project root: " + m_projectRoot);
        NF_LOG_INFO("Editor", "Content path: " + m_contentPath);
    }

    [[nodiscard]] const std::string& executablePath() const { return m_executablePath; }
    [[nodiscard]] const std::string& projectRoot() const { return m_projectRoot; }
    [[nodiscard]] const std::string& contentPath() const { return m_contentPath; }
    [[nodiscard]] const std::string& dataPath() const { return m_dataPath; }
    [[nodiscard]] const std::string& configPath() const { return m_configPath; }

    [[nodiscard]] std::string resolvePath(const std::string& relativePath) const {
        return (std::filesystem::path(m_projectRoot) / relativePath).string();
    }

private:
    std::string m_executablePath;
    std::string m_projectRoot;
    std::string m_contentPath;
    std::string m_dataPath;
    std::string m_configPath;
};

// ── Launch Service ───────────────────────────────────────────────

struct LaunchResult {
    bool success = false;
    std::string executablePath;
    std::string errorMessage;
};

class LaunchService {
public:
    void setGameExecutableName(const std::string& name) { m_gameExeName = name; }
    void setBuildDirectory(const std::string& dir) { m_buildDir = dir; }

    [[nodiscard]] std::string resolveGamePath() const {
        // Search order: explicit build dir, then common locations
        std::vector<std::string> searchPaths;

        if (!m_buildDir.empty()) {
            searchPaths.push_back(m_buildDir + "/bin/" + m_gameExeName);
            searchPaths.push_back(m_buildDir + "/" + m_gameExeName);
        }

        searchPaths.push_back("./Builds/debug/bin/" + m_gameExeName);
        searchPaths.push_back("./Builds/release/bin/" + m_gameExeName);
        searchPaths.push_back("./bin/" + m_gameExeName);
        searchPaths.push_back("./" + m_gameExeName);

        for (auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                return std::filesystem::canonical(path).string();
            }
        }

        return "";
    }

    [[nodiscard]] LaunchResult validateLaunch() const {
        LaunchResult result;
        result.executablePath = resolveGamePath();

        if (result.executablePath.empty()) {
            result.success = false;
            result.errorMessage = "Game executable '" + m_gameExeName + "' not found in search paths";
            return result;
        }

        if (!std::filesystem::exists(result.executablePath)) {
            result.success = false;
            result.errorMessage = "Game executable does not exist: " + result.executablePath;
            return result;
        }

        result.success = true;
        return result;
    }

private:
    std::string m_gameExeName = "AtlasGame";
    std::string m_buildDir;
};

// ── Editor Command State ─────────────────────────────────────────

struct CommandInfo {
    std::string name;
    std::string displayName;
    std::string hotkey;
    std::function<void()> handler;
    std::function<bool()> enabledCheck;  // returns true if command is available
    bool enabled = true;
};

class EditorCommandRegistry {
public:
    void registerCommand(const std::string& name, std::function<void()> handler,
                         const std::string& displayName = "",
                         const std::string& hotkey = "") {
        CommandInfo info;
        info.name = name;
        info.displayName = displayName.empty() ? name : displayName;
        info.hotkey = hotkey;
        info.handler = std::move(handler);
        info.enabled = true;
        m_commands[name] = std::move(info);
    }

    void setEnabledCheck(const std::string& name, std::function<bool()> check) {
        auto it = m_commands.find(name);
        if (it != m_commands.end()) {
            it->second.enabledCheck = std::move(check);
        }
    }

    bool executeCommand(const std::string& name) {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) {
            NF_LOG_WARN("Editor", "Unknown command: " + name);
            return false;
        }
        if (it->second.enabledCheck && !it->second.enabledCheck()) {
            NF_LOG_WARN("Editor", "Command disabled: " + name);
            return false;
        }
        it->second.handler();
        return true;
    }

    [[nodiscard]] bool isCommandEnabled(const std::string& name) const {
        auto it = m_commands.find(name);
        if (it == m_commands.end()) return false;
        if (it->second.enabledCheck) return it->second.enabledCheck();
        return it->second.enabled;
    }

    [[nodiscard]] const CommandInfo* findCommand(const std::string& name) const {
        auto it = m_commands.find(name);
        return (it != m_commands.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] std::vector<std::string> allCommandNames() const {
        std::vector<std::string> names;
        names.reserve(m_commands.size());
        for (auto& [k, _] : m_commands) names.push_back(k);
        std::sort(names.begin(), names.end());
        return names;
    }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

private:
    std::unordered_map<std::string, CommandInfo> m_commands;
};

// ── Recent Files ─────────────────────────────────────────────────

class RecentFilesList {
public:
    void addFile(const std::string& path) {
        // Remove if already present
        m_files.erase(std::remove(m_files.begin(), m_files.end(), path), m_files.end());
        m_files.insert(m_files.begin(), path);
        if (m_files.size() > m_maxEntries) {
            m_files.resize(m_maxEntries);
        }
    }

    void clear() { m_files.clear(); }

    [[nodiscard]] const std::vector<std::string>& files() const { return m_files; }
    [[nodiscard]] size_t count() const { return m_files.size(); }
    [[nodiscard]] bool empty() const { return m_files.empty(); }

    void setMaxEntries(size_t max) { m_maxEntries = max; }

private:
    std::vector<std::string> m_files;
    size_t m_maxEntries = 10;
};

// ── Editor Theme ─────────────────────────────────────────────────


} // namespace NF
