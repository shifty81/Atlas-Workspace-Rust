#include "SaveFileBrowserPanel.h"
#include <filesystem>

namespace atlas::editor {

void SaveFileBrowserPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 500, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 500, 24}, {50, 50, 50, 255});
    std::string title = "Save Files: " + m_directory;
    m_drawList.DrawText({4, 4, 490, 20}, title, {220, 220, 220, 255});

    // File list
    int32_t y = 28;
    for (size_t i = 0; i < m_files.size(); ++i) {
        const auto& info = m_files[i];
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        atlas::ui::UIColor bgColor = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 500, 20}, bgColor);

        std::string line = info.filename + "  tick:" + std::to_string(info.saveTick);
        atlas::ui::UIColor textColor = info.valid
            ? atlas::ui::UIColor{200, 200, 200, 255}
            : atlas::ui::UIColor{255, 100, 100, 255};
        m_drawList.DrawText({4, y + 2, 490, 16}, line, textColor);
        y += 22;
    }
}

void SaveFileBrowserPanel::ScanDirectory(const std::string& directory) {
    m_directory = directory;
    m_files.clear();
    m_selectedIndex = -1;

    namespace fs = std::filesystem;
    if (!fs::exists(directory) || !fs::is_directory(directory)) return;

    atlas::sim::SaveSystem sys;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        if (ext != ".asav") continue;

        SaveFileInfo info;
        info.path = entry.path().string();
        info.filename = entry.path().filename().string();

        auto result = sys.Validate(info.path);
        if (result == atlas::sim::SaveResult::Success) {
            // Load to get header details
            auto loadResult = sys.Load(info.path);
            if (loadResult == atlas::sim::SaveResult::Success) {
                const auto& hdr = sys.Header();
                info.saveTick = hdr.saveTick;
                info.tickRate = hdr.tickRate;
                info.seed = hdr.seed;
                info.stateHash = hdr.stateHash;
                info.metadata = sys.Metadata();
                info.valid = true;
            }
        }

        m_files.push_back(std::move(info));
    }
}

const std::vector<SaveFileInfo>& SaveFileBrowserPanel::Files() const {
    return m_files;
}

int SaveFileBrowserPanel::SelectedIndex() const {
    return m_selectedIndex;
}

void SaveFileBrowserPanel::SelectFile(int index) {
    if (index >= 0 && index < static_cast<int>(m_files.size())) {
        m_selectedIndex = index;
    } else {
        m_selectedIndex = -1;
    }
}

const SaveFileInfo* SaveFileBrowserPanel::SelectedFile() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_files.size())) {
        return &m_files[m_selectedIndex];
    }
    return nullptr;
}

const std::string& SaveFileBrowserPanel::Directory() const {
    return m_directory;
}

}
