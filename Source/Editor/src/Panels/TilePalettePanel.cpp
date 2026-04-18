#include "TilePalettePanel.h"
#include <algorithm>
#include <cstdio>

namespace atlas::editor {

TilePalettePanel::TilePalettePanel() {
    SetVisible(true);
}

const char* TilePalettePanel::Name() const {
    return "Tile Palette";
}

void TilePalettePanel::Draw() {
    m_drawList.Clear();

    // Panel background
    atlas::ui::UIRect bg{0, 0, m_columns * 52 + 16, 0};
    m_drawList.DrawRect(bg, {40, 40, 40, 255});

    // Header
    m_drawList.DrawText({4, 4, 200, 20}, "Tile Palette", {220, 220, 220, 255});

    // Filter bar
    if (!m_filter.empty()) {
        char filterLabel[128];
        std::snprintf(filterLabel, sizeof(filterLabel), "Filter: %s", m_filter.c_str());
        m_drawList.DrawText({4, 28, 200, 16}, filterLabel, {160, 160, 160, 255});
    }

    int32_t startY = 50;
    int32_t col = 0;
    int32_t row = 0;
    const int32_t cellSize = 48;
    const int32_t spacing  = 4;

    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        if (!PassesFilter(entry)) continue;

        int32_t x = 8 + col * (cellSize + spacing);
        int32_t y = startY + row * (cellSize + spacing);

        // Tile thumbnail placeholder
        atlas::ui::UIColor tileColor = {80, 80, 80, 255};
        if (i == m_selectedIndex) {
            tileColor = {0, 122, 204, 255};
        }
        m_drawList.DrawRect({x, y, cellSize, cellSize}, tileColor);

        // Tile image
        if (entry.textureId != 0) {
            m_drawList.DrawImage({x + 2, y + 2, cellSize - 4, cellSize - 4},
                                entry.textureId, {255, 255, 255, 255});
        }

        // Selection border
        if (i == m_selectedIndex) {
            m_drawList.DrawBorder({x, y, cellSize, cellSize}, 2, {0, 150, 255, 255});
        }

        // Favorite indicator
        if (entry.isFavorite) {
            m_drawList.DrawText({x + cellSize - 12, y + 2, 12, 12}, "*", {255, 200, 0, 255});
        }

        ++col;
        if (col >= m_columns) {
            col = 0;
            ++row;
        }
    }
}

// --- Palette data ---

void TilePalettePanel::ClearEntries() {
    m_entries.clear();
    m_selectedIndex = 0;
}

void TilePalettePanel::AddEntry(const TilePaletteEntry& entry) {
    m_entries.push_back(entry);
}

size_t TilePalettePanel::EntryCount() const {
    return m_entries.size();
}

const TilePaletteEntry* TilePalettePanel::GetEntry(size_t index) const {
    if (index >= m_entries.size()) return nullptr;
    return &m_entries[index];
}

// --- Selection ---

void TilePalettePanel::Select(size_t index) {
    if (index < m_entries.size()) {
        m_selectedIndex = index;
    }
}

size_t TilePalettePanel::SelectedIndex() const {
    return m_selectedIndex;
}

const TilePaletteEntry* TilePalettePanel::SelectedEntry() const {
    if (m_selectedIndex >= m_entries.size()) return nullptr;
    return &m_entries[m_selectedIndex];
}

// --- Filtering ---

void TilePalettePanel::SetFilter(const std::string& text) {
    m_filter = text;
}

const std::string& TilePalettePanel::GetFilter() const {
    return m_filter;
}

// --- Favourites ---

void TilePalettePanel::ToggleFavorite(size_t index) {
    if (index < m_entries.size()) {
        m_entries[index].isFavorite = !m_entries[index].isFavorite;
    }
}

bool TilePalettePanel::ShowFavoritesOnly() const {
    return m_favoritesOnly;
}

void TilePalettePanel::SetShowFavoritesOnly(bool on) {
    m_favoritesOnly = on;
}

// --- Layout ---

void TilePalettePanel::SetColumns(int32_t cols) {
    if (cols > 0) m_columns = cols;
}

int32_t TilePalettePanel::GetColumns() const {
    return m_columns;
}

// --- Draw commands ---

const atlas::ui::UIDrawList& TilePalettePanel::GetDrawList() const {
    return m_drawList;
}

// --- Internal ---

bool TilePalettePanel::PassesFilter(const TilePaletteEntry& entry) const {
    if (m_favoritesOnly && !entry.isFavorite) return false;
    if (m_filter.empty()) return true;

    // Case-insensitive substring match
    std::string lowerName = entry.name;
    std::string lowerFilter = m_filter;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
    return lowerName.find(lowerFilter) != std::string::npos;
}

} // namespace atlas::editor
