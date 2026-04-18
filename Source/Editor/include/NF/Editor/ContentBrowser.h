#pragma once
// NF::Editor — Content browser
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


enum class ContentEntryType : uint8_t {
    Directory,
    Scene,
    Mesh,
    Material,
    Texture,
    Audio,
    Script,
    Data,
    Unknown
};

struct ContentEntry {
    std::string name;
    std::string path;
    ContentEntryType type = ContentEntryType::Unknown;
    size_t sizeBytes = 0;
    bool isDirectory = false;
};

class ContentBrowser {
public:
    void setRootPath(const std::string& root) {
        m_rootPath = root;
        m_currentPath = root;
    }

    bool navigateTo(const std::string& path) {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            NF_LOG_WARN("ContentBrowser", "Invalid path: " + path);
            return false;
        }
        m_currentPath = path;
        refresh();
        return true;
    }

    bool navigateUp() {
        if (m_currentPath == m_rootPath) return false;
        auto parent = std::filesystem::path(m_currentPath).parent_path().string();
        if (parent.size() < m_rootPath.size()) return false;
        return navigateTo(parent);
    }

    void refresh() {
        m_entries.clear();
        if (!std::filesystem::exists(m_currentPath)) return;

        for (auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
            ContentEntry ce;
            ce.name = entry.path().filename().string();
            ce.path = entry.path().string();
            ce.isDirectory = entry.is_directory();

            if (ce.isDirectory) {
                ce.type = ContentEntryType::Directory;
            } else {
                auto ext = entry.path().extension().string();
                ce.type = classifyExtension(ext);
                if (entry.is_regular_file()) {
                    ce.sizeBytes = static_cast<size_t>(entry.file_size());
                }
            }

            // Skip hidden files
            if (!ce.name.empty() && ce.name[0] != '.') {
                m_entries.push_back(std::move(ce));
            }
        }

        // Sort: directories first, then alphabetical
        std::sort(m_entries.begin(), m_entries.end(), [](const ContentEntry& a, const ContentEntry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory;
            return a.name < b.name;
        });
    }

    [[nodiscard]] const std::string& rootPath() const { return m_rootPath; }
    [[nodiscard]] const std::string& currentPath() const { return m_currentPath; }
    [[nodiscard]] const std::vector<ContentEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    [[nodiscard]] bool isAtRoot() const { return m_currentPath == m_rootPath; }

    static ContentEntryType classifyExtension(const std::string& ext) {
        if (ext == ".json") return ContentEntryType::Data;
        if (ext == ".scene" || ext == ".world") return ContentEntryType::Scene;
        if (ext == ".mesh" || ext == ".obj" || ext == ".fbx") return ContentEntryType::Mesh;
        if (ext == ".mat" || ext == ".material") return ContentEntryType::Material;
        if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") return ContentEntryType::Texture;
        if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") return ContentEntryType::Audio;
        if (ext == ".lua" || ext == ".graph") return ContentEntryType::Script;
        return ContentEntryType::Unknown;
    }

private:
    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<ContentEntry> m_entries;
};

// ── Project Path Service ─────────────────────────────────────────


} // namespace NF
