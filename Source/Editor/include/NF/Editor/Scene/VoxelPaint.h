#pragma once
// NF::Editor — Voxel paint tool
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class VoxelBrushShape : uint8_t { Sphere, Cube, Cylinder };

struct VoxelBrushSettings {
    VoxelBrushShape shape = VoxelBrushShape::Sphere;
    int      radius     = 1;
    uint32_t materialId = 0;
    float    strength   = 1.0f;
};

struct PaintStroke {
    std::vector<Vec3i> positions;
    uint32_t materialId = 0;
    VoxelBrushSettings brush;
};

class VoxelPaintTool {
public:
    void setBrush(const VoxelBrushSettings& b) { m_brush = b; }
    [[nodiscard]] const VoxelBrushSettings& brush() const { return m_brush; }

    void beginStroke() {
        m_currentStroke = PaintStroke{};
        m_currentStroke.materialId = m_brush.materialId;
        m_currentStroke.brush = m_brush;
        m_stroking = true;
    }

    void addToStroke(const Vec3i& pos) {
        if (m_stroking) m_currentStroke.positions.push_back(pos);
    }

    void endStroke() {
        if (m_stroking) {
            m_strokes.push_back(std::move(m_currentStroke));
            m_currentStroke = PaintStroke{};
            m_stroking = false;
        }
    }

    void addStroke(const PaintStroke& stroke) { m_strokes.push_back(stroke); }

    bool removeLastStroke() {
        if (m_strokes.empty()) return false;
        m_strokes.pop_back();
        return true;
    }

    [[nodiscard]] bool isStroking() const { return m_stroking; }
    [[nodiscard]] const std::vector<PaintStroke>& strokes() const { return m_strokes; }
    [[nodiscard]] size_t strokeCount() const { return m_strokes.size(); }

    void clear() {
        m_strokes.clear();
        m_stroking = false;
        m_currentStroke = PaintStroke{};
    }

    static constexpr size_t kMaxPaletteSize = 32;

    void setPaletteSlot(int slot, uint32_t materialId) {
        if (slot < 0 || static_cast<size_t>(slot) >= kMaxPaletteSize) return;
        if (static_cast<size_t>(slot) >= m_palette.size())
            m_palette.resize(static_cast<size_t>(slot) + 1, 0);
        m_palette[static_cast<size_t>(slot)] = materialId;
    }

    [[nodiscard]] uint32_t getPaletteSlot(int slot) const {
        if (slot < 0 || static_cast<size_t>(slot) >= m_palette.size()) return 0;
        return m_palette[static_cast<size_t>(slot)];
    }

    [[nodiscard]] size_t paletteSize() const { return m_palette.size(); }

    void setActivePaletteSlot(int slot) {
        m_activePaletteSlot = slot;
        if (slot >= 0 && static_cast<size_t>(slot) < m_palette.size())
            m_brush.materialId = m_palette[static_cast<size_t>(slot)];
    }
    [[nodiscard]] int activePaletteSlot() const { return m_activePaletteSlot; }

private:
    VoxelBrushSettings m_brush;
    PaintStroke m_currentStroke;
    bool m_stroking = false;
    std::vector<PaintStroke> m_strokes;
    std::vector<uint32_t> m_palette;
    int m_activePaletteSlot = -1;
};

// ── Editor Undo System ──────────────────────────────────────────


} // namespace NF
