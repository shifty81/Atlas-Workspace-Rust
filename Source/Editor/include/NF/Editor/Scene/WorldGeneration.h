#pragma once
// NF::Editor — Biome painter, structure seed bank, ore seam editor, PCG preview
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

enum class BiomeBrushType : uint8_t {
    Paint, Erase, Smooth, Raise, Lower, Flatten, Noise, Fill
};

inline const char* biomeBrushTypeName(BiomeBrushType t) {
    switch (t) {
        case BiomeBrushType::Paint:   return "Paint";
        case BiomeBrushType::Erase:   return "Erase";
        case BiomeBrushType::Smooth:  return "Smooth";
        case BiomeBrushType::Raise:   return "Raise";
        case BiomeBrushType::Lower:   return "Lower";
        case BiomeBrushType::Flatten: return "Flatten";
        case BiomeBrushType::Noise:   return "Noise";
        case BiomeBrushType::Fill:    return "Fill";
    }
    return "Unknown";
}

struct BiomePaintCell {
    int x = 0;
    int y = 0;
    int biomeIndex = 0;
    float intensity = 1.f;
};

class BiomePainter {
public:
    explicit BiomePainter(int gridSize = 64)
        : m_gridSize(gridSize > 256 ? 256 : (gridSize < 1 ? 1 : gridSize)) {}

    void paint(int x, int y) {
        if (x < 0 || x >= m_gridSize || y < 0 || y >= m_gridSize) return;
        auto* existing = cellAtMut(x, y);
        if (existing) {
            existing->biomeIndex = m_activeBiome;
            existing->intensity = m_brushIntensity;
            m_dirty = true;
            return;
        }
        if (m_cells.size() >= kMaxCells) return; // at capacity
        {
            BiomePaintCell c;
            c.x = x;
            c.y = y;
            c.biomeIndex = m_activeBiome;
            c.intensity = m_brushIntensity;
            m_cells.push_back(c);
        }
        m_dirty = true;
    }

    void erase(int x, int y) {
        if (x < 0 || x >= m_gridSize || y < 0 || y >= m_gridSize) return;
        auto* existing = cellAtMut(x, y);
        if (existing) {
            existing->biomeIndex = 0;
            m_dirty = true;
        }
    }

    void fill(int biomeIdx) {
        m_cells.clear();
        m_cells.reserve(static_cast<size_t>(m_gridSize) * static_cast<size_t>(m_gridSize));
        for (int cy = 0; cy < m_gridSize; ++cy) {
            for (int cx = 0; cx < m_gridSize; ++cx) {
                BiomePaintCell c;
                c.x = cx;
                c.y = cy;
                c.biomeIndex = biomeIdx;
                c.intensity = m_brushIntensity;
                m_cells.push_back(c);
            }
        }
        m_dirty = true;
    }

    [[nodiscard]] const BiomePaintCell* cellAt(int x, int y) const {
        for (auto& c : m_cells) {
            if (c.x == x && c.y == y) return &c;
        }
        return nullptr;
    }

    void clear() { m_cells.clear(); m_dirty = true; }

    [[nodiscard]] size_t cellCount() const { return m_cells.size(); }
    [[nodiscard]] int gridSize() const { return m_gridSize; }

    void setActiveBrush(BiomeBrushType b) { m_activeBrush = b; }
    [[nodiscard]] BiomeBrushType activeBrush() const { return m_activeBrush; }

    void setBrushRadius(float r) { m_brushRadius = (r < 0.f ? 0.f : (r > 16.f ? 16.f : r)); }
    [[nodiscard]] float brushRadius() const { return m_brushRadius; }

    void setBrushIntensity(float i) { m_brushIntensity = (i < 0.f ? 0.f : (i > 1.f ? 1.f : i)); }
    [[nodiscard]] float brushIntensity() const { return m_brushIntensity; }

    void setActiveBiome(int idx) { m_activeBiome = idx; }
    [[nodiscard]] int activeBiome() const { return m_activeBiome; }

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }
    void markDirty() { m_dirty = true; }

    static constexpr size_t kMaxCells = 256 * 256;

private:
    BiomePaintCell* cellAtMut(int x, int y) {
        for (auto& c : m_cells) {
            if (c.x == x && c.y == y) return &c;
        }
        return nullptr;
    }

    int m_gridSize = 64;
    BiomeBrushType m_activeBrush = BiomeBrushType::Paint;
    float m_brushRadius = 1.f;
    float m_brushIntensity = 1.f;
    int m_activeBiome = 0;
    bool m_dirty = false;
    std::vector<BiomePaintCell> m_cells;
};

// ── StructureSeedBank ────────────────────────────────────────────

struct StructureSeedOverride {
    std::string structureId;
    uint64_t    overrideSeed = 0;
    bool        locked = false;
    std::string notes;
};

class StructureSeedBank {
public:
    static constexpr size_t kMaxOverrides = 128;

    bool addOverride(StructureSeedOverride ov) {
        if (m_overrides.size() >= kMaxOverrides) return false;
        for (auto& o : m_overrides) {
            if (o.structureId == ov.structureId) return false;
        }
        m_overrides.push_back(std::move(ov));
        return true;
    }

    bool removeOverride(const std::string& structureId) {
        auto it = std::find_if(m_overrides.begin(), m_overrides.end(),
                               [&](const StructureSeedOverride& o) { return o.structureId == structureId; });
        if (it == m_overrides.end()) return false;
        m_overrides.erase(it);
        return true;
    }

    [[nodiscard]] const StructureSeedOverride* findOverride(const std::string& structureId) const {
        for (auto& o : m_overrides) {
            if (o.structureId == structureId) return &o;
        }
        return nullptr;
    }

    StructureSeedOverride* findOverride(const std::string& structureId) {
        for (auto& o : m_overrides) {
            if (o.structureId == structureId) return &o;
        }
        return nullptr;
    }

    bool lockOverride(const std::string& structureId) {
        auto* ov = findOverride(structureId);
        if (!ov) return false;
        ov->locked = true;
        return true;
    }

    bool unlockOverride(const std::string& structureId) {
        auto* ov = findOverride(structureId);
        if (!ov) return false;
        ov->locked = false;
        return true;
    }

    [[nodiscard]] size_t overrideCount() const { return m_overrides.size(); }
    [[nodiscard]] const std::vector<StructureSeedOverride>& overrides() const { return m_overrides; }
    void clear() { m_overrides.clear(); }

    [[nodiscard]] size_t lockedCount() const {
        size_t n = 0;
        for (auto& o : m_overrides) { if (o.locked) ++n; }
        return n;
    }

private:
    std::vector<StructureSeedOverride> m_overrides;
};

// ── OreSeamEditor ────────────────────────────────────────────────

enum class OreSeamType : uint8_t {
    Iron, Copper, Gold, Silver, Titanium, Uranium, Crystal, Exotic
};

inline const char* oreSeamTypeName(OreSeamType t) {
    switch (t) {
        case OreSeamType::Iron:     return "Iron";
        case OreSeamType::Copper:   return "Copper";
        case OreSeamType::Gold:     return "Gold";
        case OreSeamType::Silver:   return "Silver";
        case OreSeamType::Titanium: return "Titanium";
        case OreSeamType::Uranium:  return "Uranium";
        case OreSeamType::Crystal:  return "Crystal";
        case OreSeamType::Exotic:   return "Exotic";
    }
    return "Unknown";
}

struct OreSeamDef {
    std::string id;
    OreSeamType type = OreSeamType::Iron;
    Vec3 position;
    float radius = 5.f;
    float density = 0.5f;
    float depth = 10.f;
    uint64_t seed = 0;

    [[nodiscard]] float volume() const { return 4.f / 3.f * 3.14159265f * radius * radius * radius * density; }
};

class OreSeamEditor {
public:
    static constexpr size_t kMaxSeams = 64;

    bool addSeam(OreSeamDef seam) {
        if (m_seams.size() >= kMaxSeams) return false;
        for (auto& s : m_seams) {
            if (s.id == seam.id) return false;
        }
        m_seams.push_back(std::move(seam));
        return true;
    }

    bool removeSeam(const std::string& id) {
        auto it = std::find_if(m_seams.begin(), m_seams.end(),
                               [&](const OreSeamDef& s) { return s.id == id; });
        if (it == m_seams.end()) return false;
        m_seams.erase(it);
        return true;
    }

    [[nodiscard]] const OreSeamDef* findSeam(const std::string& id) const {
        for (auto& s : m_seams) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    OreSeamDef* findSeam(const std::string& id) {
        for (auto& s : m_seams) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    [[nodiscard]] std::vector<const OreSeamDef*> seamsOfType(OreSeamType type) const {
        std::vector<const OreSeamDef*> result;
        for (auto& s : m_seams) {
            if (s.type == type) result.push_back(&s);
        }
        return result;
    }

    [[nodiscard]] float totalVolume() const {
        float v = 0.f;
        for (auto& s : m_seams) v += s.volume();
        return v;
    }

    [[nodiscard]] size_t seamCount() const { return m_seams.size(); }
    [[nodiscard]] const std::vector<OreSeamDef>& seams() const { return m_seams; }
    void clear() { m_seams.clear(); }

private:
    std::vector<OreSeamDef> m_seams;
};

// ── PCGPreviewRenderer ──────────────────────────────────────────

enum class PCGPreviewMode : uint8_t {
    Heightmap, Biome, Moisture, OreDeposits, Structures, Combined, Wireframe, Heatmap
};

inline const char* pcgPreviewModeName(PCGPreviewMode m) {
    switch (m) {
        case PCGPreviewMode::Heightmap:   return "Heightmap";
        case PCGPreviewMode::Biome:       return "Biome";
        case PCGPreviewMode::Moisture:    return "Moisture";
        case PCGPreviewMode::OreDeposits: return "OreDeposits";
        case PCGPreviewMode::Structures:  return "Structures";
        case PCGPreviewMode::Combined:    return "Combined";
        case PCGPreviewMode::Wireframe:   return "Wireframe";
        case PCGPreviewMode::Heatmap:     return "Heatmap";
    }
    return "Unknown";
}

struct PCGPreviewSettings {
    PCGPreviewMode mode = PCGPreviewMode::Combined;
    int resolution = 128;
    float zoom = 1.f;
    bool autoRefresh = true;
    bool showGrid = true;
    bool showLabels = false;
    uint64_t seed = 0;
};

class PCGPreviewRenderer {
public:
    void setSettings(const PCGPreviewSettings& s) {
        m_settings = s;
        clampSettings();
    }
    [[nodiscard]] const PCGPreviewSettings& settings() const { return m_settings; }

    void setResolution(int r) {
        m_settings.resolution = (r < 32 ? 32 : (r > 512 ? 512 : r));
    }

    void setZoom(float z) {
        m_settings.zoom = (z < 0.1f ? 0.1f : (z > 10.f ? 10.f : z));
    }

    void setMode(PCGPreviewMode mode) { m_settings.mode = mode; }

    void refresh() { m_stale = true; ++m_refreshCount; }
    [[nodiscard]] size_t refreshCount() const { return m_refreshCount; }
    [[nodiscard]] bool isStale() const { return m_stale; }
    void markFresh() { m_stale = false; }

    void setPreviewData(std::vector<float> data) { m_previewData = std::move(data); }
    [[nodiscard]] const std::vector<float>& previewData() const { return m_previewData; }
    [[nodiscard]] size_t previewPixelCount() const {
        return static_cast<size_t>(m_settings.resolution) * static_cast<size_t>(m_settings.resolution);
    }

    void clear() {
        m_settings = PCGPreviewSettings{};
        m_previewData.clear();
        m_stale = false;
        m_refreshCount = 0;
    }

private:
    void clampSettings() {
        if (m_settings.resolution < 32) m_settings.resolution = 32;
        if (m_settings.resolution > 512) m_settings.resolution = 512;
        if (m_settings.zoom < 0.1f) m_settings.zoom = 0.1f;
        if (m_settings.zoom > 10.f) m_settings.zoom = 10.f;
    }

    PCGPreviewSettings m_settings;
    std::vector<float> m_previewData;
    bool m_stale = false;
    size_t m_refreshCount = 0;
};

// ---------------------------------------------------------------------------
// S7 — Logic Wiring UI
// ---------------------------------------------------------------------------


} // namespace NF
