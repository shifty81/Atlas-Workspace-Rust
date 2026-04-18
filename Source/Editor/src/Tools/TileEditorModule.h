#pragma once
#include "IEditorToolModule.h"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>
#include <unordered_map>

namespace atlas::editor {

/// Tile definition metadata — each tile type is an asset.
struct TileAssetDef {
    uint32_t id = 0;
    std::string name;
    uint32_t textureId = 0;
    int32_t width = 1;   ///< Grid units
    int32_t height = 1;
    bool solid = false;
    bool blocksNav = false;
};

/// A single placed tile instance.
struct TileInstance {
    uint32_t tileAssetId = 0;
    uint8_t rotation = 0;    ///< 0, 90, 180, 270
    bool flippedX = false;
    bool flippedY = false;
};

/// Grid coordinate used as a map key.
struct GridCoord {
    int32_t x = 0;
    int32_t y = 0;

    bool operator==(const GridCoord& o) const { return x == o.x && y == o.y; }
};

} // namespace atlas::editor

// Hash specialisation for GridCoord so it can be used as a map key.
template<>
struct std::hash<atlas::editor::GridCoord> {
    size_t operator()(const atlas::editor::GridCoord& c) const noexcept {
        // Use XOR-shift hash combiner for safe handling of negative coordinates.
        size_t h1 = std::hash<int32_t>{}(c.x);
        size_t h2 = std::hash<int32_t>{}(c.y);
        return h1 ^ (h2 * 2654435761u + 0x9e3779b9u + (h1 << 6) + (h1 >> 2));
    }
};

namespace atlas::editor {

/// A single layer in a tile map.
struct TileLayer {
    std::string name;
    int32_t zIndex = 0;
    bool visible = true;
    bool locked = false;
    std::unordered_map<GridCoord, TileInstance> tiles;
};

/// The tile map being edited — the root data asset.
struct TileMap {
    std::string name;
    int32_t gridCellSize = 32;  ///< Pixels per grid cell
    std::vector<TileLayer> layers;
};

/// Editor modes for the tile editor.
enum class TileEditorMode : uint8_t {
    Paint,
    Erase,
    Select,
    LayerEdit,
    RuleEdit
};

/// Brush shape used when painting.
enum class BrushShape : uint8_t {
    Square,   ///< Square (or single-cell) brush
    Circle    ///< Circular brush
};

/// Event fired when a tile is placed or erased.
struct TilePaintEvent {
    GridCoord coord;
    uint32_t  tileAssetId;  ///< 0 on erase
    bool      isErase;
};

/// Gold-standard tile editor module.
/// Touches UI, data, blueprints, assets, undo, serialisation and
/// standalone builds — making it the ideal reference for all other
/// editor tool modules.
class TileEditorModule : public IEditorToolModule {
public:
    std::string Name() const override;

    void OnRegister() override;
    void OnUnregister() override;

    void RegisterPanels() override;
    void RegisterMenus() override;
    void RegisterModes() override;

    bool HandleInput(uint32_t keyCode, bool pressed) override;
    void Update(float deltaTime) override;
    void Render() override;

    // --- Tile-editor-specific API ---

    void SetMode(TileEditorMode mode);
    TileEditorMode GetMode() const;

    TileMap& GetTileMap();
    const TileMap& GetTileMap() const;

    void SetSelectedTile(uint32_t tileAssetId);
    uint32_t GetSelectedTile() const;

    void SetActiveLayer(size_t layerIndex);
    size_t GetActiveLayer() const;

    /// Single-cell paint / erase.
    void PaintTile(const GridCoord& coord);
    void EraseTile(const GridCoord& coord);

    /// Brush configuration (halfRadius 0 = single cell, 1 = 3×3, 2 = 5×5, etc.)
    void SetBrushSize(int32_t halfRadius);
    int32_t GetBrushSize() const;
    void SetBrushShape(BrushShape shape);
    BrushShape GetBrushShape() const;

    /// Paint / erase all cells covered by the current brush centred on coord.
    void BrushPaint(const GridCoord& coord);
    void BrushErase(const GridCoord& coord);

    /// Fill an axis-aligned rectangle (inclusive corners).
    void PaintRect(const GridCoord& min, const GridCoord& max);
    void EraseRect(const GridCoord& min, const GridCoord& max);

    /// Flood-fill starting from origin, replacing only tiles with the same
    /// tileAssetId as the origin cell (or empty cells when origin is empty).
    void FloodFill(const GridCoord& origin);

    /// Event callbacks — fired after each tile is placed or erased.
    void SetOnTilePainted(std::function<void(const TilePaintEvent&)> cb);
    void SetOnTileErased(std::function<void(const TilePaintEvent&)> cb);

private:
    void PlaceTile(const GridCoord& coord);
    void RemoveTile(const GridCoord& coord);

    TileMap m_tileMap;
    TileEditorMode m_mode = TileEditorMode::Paint;
    uint32_t m_selectedTile = 0;
    size_t m_activeLayer = 0;
    int32_t m_brushHalfRadius = 0;
    BrushShape m_brushShape = BrushShape::Square;

    std::function<void(const TilePaintEvent&)> m_onTilePainted;
    std::function<void(const TilePaintEvent&)> m_onTileErased;
};

} // namespace atlas::editor
