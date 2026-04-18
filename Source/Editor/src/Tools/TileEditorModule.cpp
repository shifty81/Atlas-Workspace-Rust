#include "TileEditorModule.h"
#include "../../engine/core/Logger.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace atlas::editor {

std::string TileEditorModule::Name() const {
    return "Tile Editor";
}

void TileEditorModule::OnRegister() {
    // Create a default layer so the editor is immediately usable.
    if (m_tileMap.layers.empty()) {
        TileLayer layer;
        layer.name = "Default";
        layer.zIndex = 0;
        m_tileMap.layers.push_back(std::move(layer));
    }
    Logger::Info("TileEditorModule registered");
}

void TileEditorModule::OnUnregister() {
    Logger::Info("TileEditorModule unregistered");
}

void TileEditorModule::RegisterPanels() {
    // Panels: TilePalette, LayerStack, Properties, BrushSettings, Viewport
    Logger::Info("TileEditorModule: panels registered");
}

void TileEditorModule::RegisterMenus() {
    Logger::Info("TileEditorModule: menus registered");
}

void TileEditorModule::RegisterModes() {
    // Paint, Erase, Select, LayerEdit, RuleEdit
    Logger::Info("TileEditorModule: modes registered");
}

bool TileEditorModule::HandleInput(uint32_t keyCode, bool pressed) {
    if (!pressed) return false;

    switch (keyCode) {
        case 'P': case 'p':
            SetMode(TileEditorMode::Paint);
            return true;
        case 'E': case 'e':
            SetMode(TileEditorMode::Erase);
            return true;
        case 'S': case 's':
            SetMode(TileEditorMode::Select);
            return true;
        case 'L': case 'l':
            SetMode(TileEditorMode::LayerEdit);
            return true;
        case 'R': case 'r':
            SetMode(TileEditorMode::RuleEdit);
            return true;
        default:
            return false;
    }
}

void TileEditorModule::Update(float /*deltaTime*/) {
    // Tick logic (e.g. auto-tiling rule evaluation).
}

void TileEditorModule::Render() {
    // Tile rendering delegated to ITileRenderer (to be implemented).
}

// --- Tile-editor-specific API ---

void TileEditorModule::SetMode(TileEditorMode mode) {
    m_mode = mode;
}

TileEditorMode TileEditorModule::GetMode() const {
    return m_mode;
}

TileMap& TileEditorModule::GetTileMap() {
    return m_tileMap;
}

const TileMap& TileEditorModule::GetTileMap() const {
    return m_tileMap;
}

void TileEditorModule::SetSelectedTile(uint32_t tileAssetId) {
    m_selectedTile = tileAssetId;
}

uint32_t TileEditorModule::GetSelectedTile() const {
    return m_selectedTile;
}

void TileEditorModule::SetActiveLayer(size_t layerIndex) {
    if (layerIndex < m_tileMap.layers.size()) {
        m_activeLayer = layerIndex;
    }
}

size_t TileEditorModule::GetActiveLayer() const {
    return m_activeLayer;
}

// --- Internal helpers ---

void TileEditorModule::PlaceTile(const GridCoord& coord) {
    if (m_activeLayer >= m_tileMap.layers.size()) return;
    if (m_selectedTile == 0) return;

    TileInstance inst;
    inst.tileAssetId = m_selectedTile;
    m_tileMap.layers[m_activeLayer].tiles[coord] = inst;

    if (m_onTilePainted) {
        TilePaintEvent ev{coord, m_selectedTile, false};
        m_onTilePainted(ev);
    }
}

void TileEditorModule::RemoveTile(const GridCoord& coord) {
    if (m_activeLayer >= m_tileMap.layers.size()) return;
    m_tileMap.layers[m_activeLayer].tiles.erase(coord);

    if (m_onTileErased) {
        TilePaintEvent ev{coord, 0u, true};
        m_onTileErased(ev);
    }
}

// --- Single-cell API ---

void TileEditorModule::PaintTile(const GridCoord& coord) {
    PlaceTile(coord);
}

void TileEditorModule::EraseTile(const GridCoord& coord) {
    RemoveTile(coord);
}

// --- Brush configuration ---

void TileEditorModule::SetBrushSize(int32_t halfRadius) {
    m_brushHalfRadius = std::max(0, halfRadius);
}

int32_t TileEditorModule::GetBrushSize() const {
    return m_brushHalfRadius;
}

void TileEditorModule::SetBrushShape(BrushShape shape) {
    m_brushShape = shape;
}

BrushShape TileEditorModule::GetBrushShape() const {
    return m_brushShape;
}

// --- Brush paint / erase ---

void TileEditorModule::BrushPaint(const GridCoord& center) {
    int32_t r = m_brushHalfRadius;
    for (int32_t dy = -r; dy <= r; ++dy) {
        for (int32_t dx = -r; dx <= r; ++dx) {
            if (m_brushShape == BrushShape::Circle) {
                // Use squared distance to avoid sqrt.
                if (dx * dx + dy * dy > r * r) continue;
            }
            PlaceTile({center.x + dx, center.y + dy});
        }
    }
}

void TileEditorModule::BrushErase(const GridCoord& center) {
    int32_t r = m_brushHalfRadius;
    for (int32_t dy = -r; dy <= r; ++dy) {
        for (int32_t dx = -r; dx <= r; ++dx) {
            if (m_brushShape == BrushShape::Circle) {
                if (dx * dx + dy * dy > r * r) continue;
            }
            RemoveTile({center.x + dx, center.y + dy});
        }
    }
}

// --- Rectangle fill ---

void TileEditorModule::PaintRect(const GridCoord& minCoord, const GridCoord& maxCoord) {
    int32_t x0 = std::min(minCoord.x, maxCoord.x);
    int32_t x1 = std::max(minCoord.x, maxCoord.x);
    int32_t y0 = std::min(minCoord.y, maxCoord.y);
    int32_t y1 = std::max(minCoord.y, maxCoord.y);

    for (int32_t y = y0; y <= y1; ++y) {
        for (int32_t x = x0; x <= x1; ++x) {
            PlaceTile({x, y});
        }
    }
}

void TileEditorModule::EraseRect(const GridCoord& minCoord, const GridCoord& maxCoord) {
    int32_t x0 = std::min(minCoord.x, maxCoord.x);
    int32_t x1 = std::max(minCoord.x, maxCoord.x);
    int32_t y0 = std::min(minCoord.y, maxCoord.y);
    int32_t y1 = std::max(minCoord.y, maxCoord.y);

    for (int32_t y = y0; y <= y1; ++y) {
        for (int32_t x = x0; x <= x1; ++x) {
            RemoveTile({x, y});
        }
    }
}

// --- Flood fill ---

void TileEditorModule::FloodFill(const GridCoord& origin) {
    if (m_activeLayer >= m_tileMap.layers.size()) return;
    if (m_selectedTile == 0) return;

    auto& tileMap = m_tileMap.layers[m_activeLayer].tiles;

    // Determine the target tileAssetId at the origin (0 = empty cell).
    uint32_t targetId = 0;
    auto it = tileMap.find(origin);
    if (it != tileMap.end()) {
        targetId = it->second.tileAssetId;
    }

    // Nothing to do if origin already has the fill tile.
    if (targetId == m_selectedTile) return;

    // BFS flood fill (4-connected).
    std::queue<GridCoord> frontier;
    std::unordered_set<GridCoord> visited;

    frontier.push(origin);
    visited.insert(origin);

    // Safety cap: avoid runaway fills on unbounded maps.
    constexpr size_t kMaxCells = 1'000'000u;

    while (!frontier.empty() && visited.size() <= kMaxCells) {
        GridCoord current = frontier.front();
        frontier.pop();

        // Paint the current cell.
        PlaceTile(current);

        // Expand to 4-connected neighbours.
        const GridCoord neighbours[4] = {
            {current.x + 1, current.y},
            {current.x - 1, current.y},
            {current.x,     current.y + 1},
            {current.x,     current.y - 1},
        };

        for (const GridCoord& nb : neighbours) {
            if (visited.count(nb)) continue;

            // Check if the neighbour has the same original tile id.
            uint32_t nbId = 0;
            auto nbIt = tileMap.find(nb);
            if (nbIt != tileMap.end()) {
                nbId = nbIt->second.tileAssetId;
            }

            if (nbId == targetId) {
                visited.insert(nb);
                frontier.push(nb);
            }
        }
    }
}

// --- Event callbacks ---

void TileEditorModule::SetOnTilePainted(std::function<void(const TilePaintEvent&)> cb) {
    m_onTilePainted = std::move(cb);
}

void TileEditorModule::SetOnTileErased(std::function<void(const TilePaintEvent&)> cb) {
    m_onTileErased = std::move(cb);
}

} // namespace atlas::editor
