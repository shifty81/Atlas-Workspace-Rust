#include "../editor/panels/TilePalettePanel.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace atlas::editor;

void test_tile_palette_name() {
    TilePalettePanel panel;
    assert(std::strcmp(panel.Name(), "Tile Palette") == 0);
    std::cout << "[PASS] test_tile_palette_name" << std::endl;
}

void test_tile_palette_add_entries() {
    TilePalettePanel panel;
    assert(panel.EntryCount() == 0);

    panel.AddEntry({1, "Grass", 10, false});
    panel.AddEntry({2, "Stone", 11, false});
    assert(panel.EntryCount() == 2);

    auto* entry = panel.GetEntry(0);
    assert(entry != nullptr);
    assert(entry->name == "Grass");
    assert(entry->assetId == 1);
    std::cout << "[PASS] test_tile_palette_add_entries" << std::endl;
}

void test_tile_palette_selection() {
    TilePalettePanel panel;
    panel.AddEntry({1, "Grass", 10, false});
    panel.AddEntry({2, "Stone", 11, false});
    panel.AddEntry({3, "Water", 12, false});

    assert(panel.SelectedIndex() == 0);
    panel.Select(2);
    assert(panel.SelectedIndex() == 2);

    auto* sel = panel.SelectedEntry();
    assert(sel != nullptr);
    assert(sel->name == "Water");
    std::cout << "[PASS] test_tile_palette_selection" << std::endl;
}

void test_tile_palette_selection_oob() {
    TilePalettePanel panel;
    panel.AddEntry({1, "Grass", 10, false});

    panel.Select(99);  // out of bounds
    assert(panel.SelectedIndex() == 0);  // unchanged
    std::cout << "[PASS] test_tile_palette_selection_oob" << std::endl;
}

void test_tile_palette_filter() {
    TilePalettePanel panel;
    assert(panel.GetFilter().empty());
    panel.SetFilter("stone");
    assert(panel.GetFilter() == "stone");
    std::cout << "[PASS] test_tile_palette_filter" << std::endl;
}

void test_tile_palette_favorites() {
    TilePalettePanel panel;
    panel.AddEntry({1, "Grass", 10, false});
    panel.AddEntry({2, "Stone", 11, false});

    assert(!panel.GetEntry(0)->isFavorite);
    panel.ToggleFavorite(0);
    assert(panel.GetEntry(0)->isFavorite);
    panel.ToggleFavorite(0);
    assert(!panel.GetEntry(0)->isFavorite);
    std::cout << "[PASS] test_tile_palette_favorites" << std::endl;
}

void test_tile_palette_favorites_only() {
    TilePalettePanel panel;
    assert(!panel.ShowFavoritesOnly());
    panel.SetShowFavoritesOnly(true);
    assert(panel.ShowFavoritesOnly());
    std::cout << "[PASS] test_tile_palette_favorites_only" << std::endl;
}

void test_tile_palette_columns() {
    TilePalettePanel panel;
    assert(panel.GetColumns() == 4);
    panel.SetColumns(6);
    assert(panel.GetColumns() == 6);
    panel.SetColumns(0);  // invalid, should not change
    assert(panel.GetColumns() == 6);
    std::cout << "[PASS] test_tile_palette_columns" << std::endl;
}

void test_tile_palette_clear() {
    TilePalettePanel panel;
    panel.AddEntry({1, "Grass", 10, false});
    panel.AddEntry({2, "Stone", 11, false});
    panel.Select(1);

    panel.ClearEntries();
    assert(panel.EntryCount() == 0);
    assert(panel.SelectedIndex() == 0);
    std::cout << "[PASS] test_tile_palette_clear" << std::endl;
}

void test_tile_palette_draw() {
    TilePalettePanel panel;
    panel.AddEntry({1, "Grass", 10, false});
    panel.AddEntry({2, "Stone", 11, false});

    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_tile_palette_draw" << std::endl;
}

void test_tile_palette_draw_with_filter() {
    TilePalettePanel panel;
    panel.AddEntry({1, "Grass", 10, false});
    panel.AddEntry({2, "Stone", 11, false});
    panel.SetFilter("grass");

    panel.Draw();
    // Should still produce draw commands
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_tile_palette_draw_with_filter" << std::endl;
}
