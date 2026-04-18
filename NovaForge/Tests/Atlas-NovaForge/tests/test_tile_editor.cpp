#include "../editor/tools/TileEditorModule.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;

void test_tile_editor_name() {
    TileEditorModule mod;
    assert(mod.Name() == "Tile Editor");
    std::cout << "[PASS] test_tile_editor_name" << std::endl;
}

void test_tile_editor_register() {
    TileEditorModule mod;
    mod.OnRegister();
    // Should create a default layer
    assert(!mod.GetTileMap().layers.empty());
    assert(mod.GetTileMap().layers[0].name == "Default");
    std::cout << "[PASS] test_tile_editor_register" << std::endl;
}

void test_tile_editor_modes() {
    TileEditorModule mod;
    assert(mod.GetMode() == TileEditorMode::Paint);
    mod.SetMode(TileEditorMode::Erase);
    assert(mod.GetMode() == TileEditorMode::Erase);
    mod.SetMode(TileEditorMode::Select);
    assert(mod.GetMode() == TileEditorMode::Select);
    std::cout << "[PASS] test_tile_editor_modes" << std::endl;
}

void test_tile_editor_paint() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(42);
    assert(mod.GetSelectedTile() == 42);

    mod.PaintTile({3, 5});
    auto& layer = mod.GetTileMap().layers[0];
    auto it = layer.tiles.find({3, 5});
    assert(it != layer.tiles.end());
    assert(it->second.tileAssetId == 42);
    std::cout << "[PASS] test_tile_editor_paint" << std::endl;
}

void test_tile_editor_erase() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(1);
    mod.PaintTile({1, 1});
    assert(mod.GetTileMap().layers[0].tiles.count({1, 1}) == 1);

    mod.EraseTile({1, 1});
    assert(mod.GetTileMap().layers[0].tiles.count({1, 1}) == 0);
    std::cout << "[PASS] test_tile_editor_erase" << std::endl;
}

void test_tile_editor_paint_no_tile_selected() {
    TileEditorModule mod;
    mod.OnRegister();
    // selectedTile is 0 by default — painting should be a no-op
    mod.PaintTile({2, 2});
    assert(mod.GetTileMap().layers[0].tiles.empty());
    std::cout << "[PASS] test_tile_editor_paint_no_tile_selected" << std::endl;
}

void test_tile_editor_active_layer() {
    TileEditorModule mod;
    mod.OnRegister();
    assert(mod.GetActiveLayer() == 0);

    // Add a second layer
    TileLayer layer2;
    layer2.name = "Layer2";
    layer2.zIndex = 1;
    mod.GetTileMap().layers.push_back(std::move(layer2));

    mod.SetActiveLayer(1);
    assert(mod.GetActiveLayer() == 1);

    // Out-of-bounds should not change
    mod.SetActiveLayer(99);
    assert(mod.GetActiveLayer() == 1);
    std::cout << "[PASS] test_tile_editor_active_layer" << std::endl;
}

void test_tile_editor_unregister() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.OnUnregister();
    // Should not crash
    std::cout << "[PASS] test_tile_editor_unregister" << std::endl;
}

void test_tile_editor_grid_coord_hash() {
    // Verify that GridCoord hash works for positive and negative coords
    std::hash<GridCoord> h;
    GridCoord a{0, 0};
    GridCoord b{1, 0};
    GridCoord c{0, 1};
    GridCoord d{-1, -1};

    // Different coordinates should (very likely) produce different hashes
    assert(h(a) != h(b) || (a == b));
    assert(h(a) != h(c) || (a == c));
    assert(h(a) != h(d) || (a == d));
    std::cout << "[PASS] test_tile_editor_grid_coord_hash" << std::endl;
}

// --- New painting mode tests ---

void test_tile_editor_brush_paint_single() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(5);
    mod.SetBrushSize(0); // radius 0 = single cell
    mod.BrushPaint({2, 3});
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    assert(tiles.count({2, 3}) && tiles.at({2, 3}).tileAssetId == 5);
    assert(tiles.size() == 1);
    std::cout << "[PASS] test_tile_editor_brush_paint_single" << std::endl;
}

void test_tile_editor_brush_paint_square_3x3() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(7);
    mod.SetBrushSize(1); // radius 1 = 3×3 square
    mod.SetBrushShape(BrushShape::Square);
    mod.BrushPaint({0, 0});
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    assert(tiles.size() == 9);
    // Check corners
    assert(tiles.count({-1, -1}));
    assert(tiles.count({ 1,  1}));
    std::cout << "[PASS] test_tile_editor_brush_paint_square_3x3" << std::endl;
}

void test_tile_editor_brush_paint_circle() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(3);
    mod.SetBrushSize(2);
    mod.SetBrushShape(BrushShape::Circle);
    mod.BrushPaint({0, 0});
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    // Circle of radius 2 should contain (0,0), (2,0), (-2,0), (0,2), (0,-2)
    assert(tiles.count({ 0,  0}));
    assert(tiles.count({ 2,  0}));
    assert(tiles.count({-2,  0}));
    // Corners (2,2) are distance sqrt(8) > 2, so excluded
    assert(!tiles.count({ 2,  2}));
    std::cout << "[PASS] test_tile_editor_brush_paint_circle" << std::endl;
}

void test_tile_editor_brush_erase() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(4);
    mod.SetBrushSize(1);
    mod.SetBrushShape(BrushShape::Square);
    mod.BrushPaint({0, 0});
    assert(mod.GetTileMap().layers[0].tiles.size() == 9);
    mod.BrushErase({0, 0});
    assert(mod.GetTileMap().layers[0].tiles.empty());
    std::cout << "[PASS] test_tile_editor_brush_erase" << std::endl;
}

void test_tile_editor_paint_rect() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(2);
    mod.PaintRect({0, 0}, {2, 2});
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    assert(tiles.size() == 9); // 3×3
    assert(tiles.count({0, 0}));
    assert(tiles.count({2, 2}));
    std::cout << "[PASS] test_tile_editor_paint_rect" << std::endl;
}

void test_tile_editor_paint_rect_reversed_corners() {
    // PaintRect must work regardless of which corner is min/max.
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(1);
    mod.PaintRect({3, 3}, {1, 1}); // reversed
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    assert(tiles.size() == 9);
    std::cout << "[PASS] test_tile_editor_paint_rect_reversed_corners" << std::endl;
}

void test_tile_editor_erase_rect() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(8);
    mod.PaintRect({0, 0}, {4, 4}); // 5×5 = 25 tiles
    assert(mod.GetTileMap().layers[0].tiles.size() == 25);
    mod.EraseRect({1, 1}, {3, 3}); // erase 3×3 = 9 tiles
    assert(mod.GetTileMap().layers[0].tiles.size() == 16);
    std::cout << "[PASS] test_tile_editor_erase_rect" << std::endl;
}

void test_tile_editor_flood_fill_empty_region() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(9);
    // Flood fill from an empty cell on an otherwise empty map — on an
    // unbounded map this would be infinite, so the cap kicks in.
    // We test that flood fill on a small enclosed region works instead.
    // Place a ring of tile id=2 to bound the fill.
    mod.SetSelectedTile(2);
    for (int x = 0; x <= 4; ++x) { mod.PaintTile({x, 0}); mod.PaintTile({x, 4}); }
    for (int y = 1; y <= 3; ++y) { mod.PaintTile({0, y}); mod.PaintTile({4, y}); }
    // Now flood-fill the interior with tile 9.
    mod.SetSelectedTile(9);
    mod.FloodFill({2, 2});
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    // Interior 3×3 = 9 cells should now be tile 9
    assert(tiles.at({2, 2}).tileAssetId == 9);
    assert(tiles.at({1, 1}).tileAssetId == 9);
    assert(tiles.at({3, 3}).tileAssetId == 9);
    // Border tiles remain tile 2
    assert(tiles.at({0, 0}).tileAssetId == 2);
    std::cout << "[PASS] test_tile_editor_flood_fill_empty_region" << std::endl;
}

void test_tile_editor_flood_fill_replace_tile() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(1);
    // Paint a 3×3 block with tile 1
    mod.PaintRect({0, 0}, {2, 2});
    // Place a different tile in the centre
    mod.SetSelectedTile(2);
    mod.PaintTile({1, 1});
    // Flood fill the surrounding tile-1 region with tile 3
    mod.SetSelectedTile(3);
    mod.FloodFill({0, 0});
    auto& tiles = mod.GetTileMap().layers[0].tiles;
    // All tile-1 cells should now be tile-3
    assert(tiles.at({0, 0}).tileAssetId == 3);
    assert(tiles.at({2, 2}).tileAssetId == 3);
    // Centre cell (tile 2) should be untouched
    assert(tiles.at({1, 1}).tileAssetId == 2);
    std::cout << "[PASS] test_tile_editor_flood_fill_replace_tile" << std::endl;
}

void test_tile_editor_flood_fill_same_tile_noop() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(5);
    mod.PaintTile({0, 0});
    // Flood-fill with the same tile — should be a no-op
    mod.FloodFill({0, 0});
    assert(mod.GetTileMap().layers[0].tiles.size() == 1);
    std::cout << "[PASS] test_tile_editor_flood_fill_same_tile_noop" << std::endl;
}

void test_tile_editor_on_tile_painted_callback() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(42);

    int callCount = 0;
    TilePaintEvent lastEv{};
    mod.SetOnTilePainted([&](const TilePaintEvent& ev) {
        ++callCount;
        lastEv = ev;
    });

    mod.PaintTile({3, 4});
    assert(callCount == 1);
    assert(lastEv.coord.x == 3 && lastEv.coord.y == 4);
    assert(lastEv.tileAssetId == 42);
    assert(!lastEv.isErase);
    std::cout << "[PASS] test_tile_editor_on_tile_painted_callback" << std::endl;
}

void test_tile_editor_on_tile_erased_callback() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(7);
    mod.PaintTile({1, 2});

    int callCount = 0;
    TilePaintEvent lastEv{};
    mod.SetOnTileErased([&](const TilePaintEvent& ev) {
        ++callCount;
        lastEv = ev;
    });

    mod.EraseTile({1, 2});
    assert(callCount == 1);
    assert(lastEv.coord.x == 1 && lastEv.coord.y == 2);
    assert(lastEv.tileAssetId == 0);
    assert(lastEv.isErase);
    std::cout << "[PASS] test_tile_editor_on_tile_erased_callback" << std::endl;
}

void test_tile_editor_brush_paint_fires_callback() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(3);
    mod.SetBrushSize(1);
    mod.SetBrushShape(BrushShape::Square);

    int callCount = 0;
    mod.SetOnTilePainted([&](const TilePaintEvent&) { ++callCount; });

    mod.BrushPaint({0, 0}); // 3×3 = 9 cells
    assert(callCount == 9);
    std::cout << "[PASS] test_tile_editor_brush_paint_fires_callback" << std::endl;
}

void test_tile_editor_rect_paint_fires_callback() {
    TileEditorModule mod;
    mod.OnRegister();
    mod.SetSelectedTile(6);

    int callCount = 0;
    mod.SetOnTilePainted([&](const TilePaintEvent&) { ++callCount; });

    mod.PaintRect({0, 0}, {1, 1}); // 2×2 = 4
    assert(callCount == 4);
    std::cout << "[PASS] test_tile_editor_rect_paint_fires_callback" << std::endl;
}

void test_tile_editor_get_brush_shape() {
    TileEditorModule mod;
    mod.OnRegister();
    assert(mod.GetBrushShape() == BrushShape::Square);
    mod.SetBrushShape(BrushShape::Circle);
    assert(mod.GetBrushShape() == BrushShape::Circle);
    std::cout << "[PASS] test_tile_editor_get_brush_shape" << std::endl;
}

void test_tile_editor_get_brush_size() {
    TileEditorModule mod;
    mod.OnRegister();
    assert(mod.GetBrushSize() == 0);
    mod.SetBrushSize(3);
    assert(mod.GetBrushSize() == 3);
    // Negative size clamped to 0.
    mod.SetBrushSize(-1);
    assert(mod.GetBrushSize() == 0);
    std::cout << "[PASS] test_tile_editor_get_brush_size" << std::endl;
}
