#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/sim/SaveSystem.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>

namespace atlas::editor {

struct SaveFileInfo {
    std::string path;
    std::string filename;
    uint64_t saveTick = 0;
    uint32_t tickRate = 0;
    uint32_t seed = 0;
    uint64_t stateHash = 0;
    std::string metadata;
    bool valid = false;
};

class SaveFileBrowserPanel : public EditorPanel {
public:
    const char* Name() const override { return "Save File Browser"; }
    void Draw() override;

    /// Scan a directory for .asav files and populate the file list.
    void ScanDirectory(const std::string& directory);

    /// Get the list of discovered save files.
    const std::vector<SaveFileInfo>& Files() const;

    /// Get the currently selected file index (-1 if none).
    int SelectedIndex() const;

    /// Select a file by index.
    void SelectFile(int index);

    /// Get info about the selected file (nullptr if none selected).
    const SaveFileInfo* SelectedFile() const;

    /// Get the directory being browsed.
    const std::string& Directory() const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::string m_directory;
    std::vector<SaveFileInfo> m_files;
    int m_selectedIndex = -1;
    atlas::ui::UIDrawList m_drawList;
};

}
