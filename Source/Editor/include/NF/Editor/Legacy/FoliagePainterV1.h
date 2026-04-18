#pragma once
// NF::Editor — Foliage painter v1: scatter-based foliage placement authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Fpv1PlacementMode: uint8_t { Scatter, Aligned, Surface, Manual };
enum class Fpv1CullMode     : uint8_t { None, Frustum, Distance, Combined };

struct Fpv1FoliageType {
    uint64_t    id        = 0;
    std::string meshName;
    float       density   = 1.f;
    float       minScale  = 0.8f;
    float       maxScale  = 1.2f;
    bool        castShadow= true;
    [[nodiscard]] bool isValid() const { return id != 0 && !meshName.empty(); }
};

struct Fpv1PaintStroke {
    uint64_t         id         = 0;
    float            x          = 0.f;
    float            y          = 0.f;
    float            radius     = 5.f;
    uint64_t         typeId     = 0;
    uint32_t         instanceCount = 0;
    [[nodiscard]] bool isValid() const { return id != 0 && typeId != 0; }
};

using Fpv1PaintCallback = std::function<void(const Fpv1PaintStroke&)>;

class FoliagePainterV1 {
public:
    bool addFoliageType(const Fpv1FoliageType& t) {
        if (!t.isValid()) return false;
        for (const auto& ft : m_types) if (ft.id == t.id) return false;
        m_types.push_back(t);
        return true;
    }

    bool removeFoliageType(uint64_t id) {
        for (auto it = m_types.begin(); it != m_types.end(); ++it) {
            if (it->id == id) { m_types.erase(it); return true; }
        }
        return false;
    }

    bool paint(const Fpv1PaintStroke& s) {
        if (!s.isValid()) return false;
        m_strokes.push_back(s);
        if (m_onPaint) m_onPaint(s);
        return true;
    }

    bool undoStroke() {
        if (m_strokes.empty()) return false;
        m_strokes.pop_back();
        return true;
    }

    bool clearStrokes() {
        if (m_strokes.empty()) return false;
        m_strokes.clear();
        return true;
    }

    void setPlacementMode(Fpv1PlacementMode m) { m_placement = m; }
    void setCullMode(Fpv1CullMode m)            { m_cull = m;      }

    [[nodiscard]] Fpv1PlacementMode placementMode() const { return m_placement;    }
    [[nodiscard]] Fpv1CullMode      cullMode()       const { return m_cull;         }
    [[nodiscard]] size_t            typeCount()      const { return m_types.size(); }
    [[nodiscard]] size_t            strokeCount()    const { return m_strokes.size(); }

    void setOnPaint(Fpv1PaintCallback cb) { m_onPaint = std::move(cb); }

private:
    std::vector<Fpv1FoliageType>  m_types;
    std::vector<Fpv1PaintStroke>  m_strokes;
    Fpv1PlacementMode             m_placement = Fpv1PlacementMode::Scatter;
    Fpv1CullMode                  m_cull      = Fpv1CullMode::Combined;
    Fpv1PaintCallback             m_onPaint;
};

} // namespace NF
