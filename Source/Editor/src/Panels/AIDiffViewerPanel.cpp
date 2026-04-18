#include "AIDiffViewerPanel.h"

namespace atlas::editor {

void AIDiffViewerPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 700, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 700, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "AI Diff Viewer", {220, 220, 220, 255});

    if (!m_hasDiff) {
        m_drawList.DrawText({4, 30, 400, 16}, "No diff loaded", {160, 160, 160, 255});
        return;
    }

    // Diff title and summary
    m_drawList.DrawText({4, 28, 690, 16}, m_currentDiff.title, {200, 200, 200, 255});
    std::string summary = std::to_string(AcceptedHunkCount()) + "/"
        + std::to_string(TotalHunkCount()) + " hunks accepted";
    m_drawList.DrawText({4, 46, 400, 14}, summary, {180, 220, 180, 255});

    // File list with hunks
    int32_t y = 66;
    for (const auto& file : m_currentDiff.files) {
        m_drawList.DrawText({4, y, 690, 16}, file.path, {100, 200, 255, 255});
        y += 18;
        for (const auto& hunk : file.hunks) {
            atlas::ui::UIColor hunkColor = {200, 200, 200, 255};
            if (hunk.accepted) hunkColor = {100, 255, 100, 255};
            else if (hunk.rejected) hunkColor = {255, 100, 100, 255};
            std::string hunkLabel = "L" + std::to_string(hunk.lineStart)
                + "+" + std::to_string(hunk.lineCount);
            m_drawList.DrawText({20, y, 300, 14}, hunkLabel, hunkColor);
            y += 16;
        }
    }
}

void AIDiffViewerPanel::LoadDiffSet(const DiffSet& diff) {
    m_currentDiff = diff;
    m_hasDiff = true;
}

const DiffSet* AIDiffViewerPanel::CurrentDiff() const {
    if (!m_hasDiff) return nullptr;
    return &m_currentDiff;
}

bool AIDiffViewerPanel::HasDiff() const {
    return m_hasDiff;
}

void AIDiffViewerPanel::AcceptHunk(size_t fileIndex, size_t hunkIndex) {
    if (fileIndex < m_currentDiff.files.size() &&
        hunkIndex < m_currentDiff.files[fileIndex].hunks.size()) {
        m_currentDiff.files[fileIndex].hunks[hunkIndex].accepted = true;
        m_currentDiff.files[fileIndex].hunks[hunkIndex].rejected = false;
    }
}

void AIDiffViewerPanel::RejectHunk(size_t fileIndex, size_t hunkIndex) {
    if (fileIndex < m_currentDiff.files.size() &&
        hunkIndex < m_currentDiff.files[fileIndex].hunks.size()) {
        m_currentDiff.files[fileIndex].hunks[hunkIndex].rejected = true;
        m_currentDiff.files[fileIndex].hunks[hunkIndex].accepted = false;
    }
}

bool AIDiffViewerPanel::IsHunkAccepted(size_t fileIndex, size_t hunkIndex) const {
    if (fileIndex < m_currentDiff.files.size() &&
        hunkIndex < m_currentDiff.files[fileIndex].hunks.size()) {
        return m_currentDiff.files[fileIndex].hunks[hunkIndex].accepted;
    }
    return false;
}

bool AIDiffViewerPanel::IsHunkRejected(size_t fileIndex, size_t hunkIndex) const {
    if (fileIndex < m_currentDiff.files.size() &&
        hunkIndex < m_currentDiff.files[fileIndex].hunks.size()) {
        return m_currentDiff.files[fileIndex].hunks[hunkIndex].rejected;
    }
    return false;
}

void AIDiffViewerPanel::AcceptAll() {
    for (auto& file : m_currentDiff.files) {
        for (auto& hunk : file.hunks) {
            hunk.accepted = true;
            hunk.rejected = false;
        }
    }
}

void AIDiffViewerPanel::RejectAll() {
    for (auto& file : m_currentDiff.files) {
        for (auto& hunk : file.hunks) {
            hunk.rejected = true;
            hunk.accepted = false;
        }
    }
}

bool AIDiffViewerPanel::ApplyAccepted() {
    if (!m_hasDiff) return false;
    m_currentDiff.applied = true;
    return true;
}

size_t AIDiffViewerPanel::AcceptedHunkCount() const {
    size_t count = 0;
    for (const auto& file : m_currentDiff.files) {
        for (const auto& hunk : file.hunks) {
            if (hunk.accepted) ++count;
        }
    }
    return count;
}

size_t AIDiffViewerPanel::TotalHunkCount() const {
    size_t count = 0;
    for (const auto& file : m_currentDiff.files) {
        count += file.hunks.size();
    }
    return count;
}

void AIDiffViewerPanel::AddToHistory(const DiffSet& diff) {
    m_history.push_back(diff);
}

const std::vector<DiffSet>& AIDiffViewerPanel::History() const {
    return m_history;
}

void AIDiffViewerPanel::Clear() {
    m_currentDiff = DiffSet{};
    m_hasDiff = false;
    m_history.clear();
}

} // namespace atlas::editor
