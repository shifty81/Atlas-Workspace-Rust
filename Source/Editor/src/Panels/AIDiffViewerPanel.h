#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

struct DiffHunk {
    uint32_t lineStart = 0;
    uint32_t lineCount = 0;
    std::string oldContent;
    std::string newContent;
    bool accepted = false;
    bool rejected = false;
};

struct DiffFile {
    std::string path;
    std::vector<DiffHunk> hunks;
    bool isNew = false;
    bool isDeleted = false;
};

struct DiffSet {
    std::string title;
    std::string description;
    std::vector<DiffFile> files;
    uint64_t timestamp = 0;
    bool applied = false;
};

class AIDiffViewerPanel : public EditorPanel {
public:
    const char* Name() const override { return "AI Diff Viewer"; }
    void Draw() override;

    void LoadDiffSet(const DiffSet& diff);
    const DiffSet* CurrentDiff() const;
    bool HasDiff() const;

    void AcceptHunk(size_t fileIndex, size_t hunkIndex);
    void RejectHunk(size_t fileIndex, size_t hunkIndex);
    bool IsHunkAccepted(size_t fileIndex, size_t hunkIndex) const;
    bool IsHunkRejected(size_t fileIndex, size_t hunkIndex) const;

    void AcceptAll();
    void RejectAll();
    bool ApplyAccepted();
    size_t AcceptedHunkCount() const;
    size_t TotalHunkCount() const;

    void AddToHistory(const DiffSet& diff);
    const std::vector<DiffSet>& History() const;

    void Clear();

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    DiffSet m_currentDiff;
    bool m_hasDiff = false;
    std::vector<DiffSet> m_history;
    atlas::ui::UIDrawList m_drawList;
};

} // namespace atlas::editor
