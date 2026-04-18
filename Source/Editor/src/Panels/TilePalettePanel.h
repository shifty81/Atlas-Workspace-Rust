#pragma once
#include "../ui/EditorPanel.h"
#include "../tools/TileEditorModule.h"
#include "../../engine/ui/UIDrawList.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::editor {

/// Palette entry displayed in the grid.
struct TilePaletteEntry {
    uint32_t    assetId    = 0;
    std::string name;
    uint32_t    textureId  = 0;
    bool        isFavorite = false;
};

/// Grid-based tile selection panel for the editor.
/// Displays tile thumbnails from the active TileSet, allows selection
/// by click, and drives the active brush in the TileEditorModule.
class TilePalettePanel : public EditorPanel {
public:
    TilePalettePanel();

    const char* Name() const override;
    void Draw() override;

    // --- Palette data ---

    void ClearEntries();
    void AddEntry(const TilePaletteEntry& entry);
    size_t EntryCount() const;
    const TilePaletteEntry* GetEntry(size_t index) const;

    // --- Selection ---

    void   Select(size_t index);
    size_t SelectedIndex() const;
    const TilePaletteEntry* SelectedEntry() const;

    // --- Filtering ---

    void SetFilter(const std::string& text);
    const std::string& GetFilter() const;

    // --- Favourites ---

    void ToggleFavorite(size_t index);
    bool ShowFavoritesOnly() const;
    void SetShowFavoritesOnly(bool on);

    // --- Layout ---

    void SetColumns(int32_t cols);
    int32_t GetColumns() const;

    // --- Draw commands (for rendering backend) ---

    const atlas::ui::UIDrawList& GetDrawList() const;

private:
    std::vector<TilePaletteEntry> m_entries;
    size_t      m_selectedIndex    = 0;
    std::string m_filter;
    bool        m_favoritesOnly    = false;
    int32_t     m_columns          = 4;
    atlas::ui::UIDrawList m_drawList;

    bool PassesFilter(const TilePaletteEntry& entry) const;
};

} // namespace atlas::editor
