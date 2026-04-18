#pragma once
// NF::ViewportCompositor — Multi-viewport layout compositor (Item 9 of 10).
//
// ViewportHostRegistry supports up to 16 slots but provides no layout logic.
// ViewportCompositor fills that gap: given a full viewport region and an
// ordered list of active handles it computes the pixel bounds for each slot
// according to the chosen ViewportLayoutMode.
//
// Supported layouts:
//   Single      — one viewport fills the entire region
//   SideBySide  — two viewports, horizontal split (50 / 50)
//   TwoByTwo    — four viewports, quad split
//   ThreeLeft   — one large left (⅔) + two stacked right (⅓)
//   ThreeRight  — two stacked left (⅓) + one large right (⅔)
//
// Usage:
//   ViewportCompositor comp;
//   comp.setLayoutMode(ViewportLayoutMode::TwoByTwo);
//   auto slotBounds = comp.computeSlotBounds(fullBounds, {h1, h2, h3, h4});
//   // → 4 CompositorSlotBounds entries with their sub-regions

#include "NF/Workspace/ViewportHostContract.h"
#include <vector>

namespace NF {

// ── ViewportLayoutMode ────────────────────────────────────────────────────────

enum class ViewportLayoutMode : uint8_t {
    Single,      ///< One viewport fills the full region
    SideBySide,  ///< Two viewports, horizontal 50/50 split
    TwoByTwo,    ///< Four viewports, quad split
    ThreeLeft,   ///< One large left (⅔) + two stacked right (⅓ each)
    ThreeRight,  ///< Two stacked left (⅓ each) + one large right (⅔)
};

inline const char* viewportLayoutModeName(ViewportLayoutMode m) {
    switch (m) {
    case ViewportLayoutMode::Single:      return "Single";
    case ViewportLayoutMode::SideBySide:  return "SideBySide";
    case ViewportLayoutMode::TwoByTwo:    return "TwoByTwo";
    case ViewportLayoutMode::ThreeLeft:   return "ThreeLeft";
    case ViewportLayoutMode::ThreeRight:  return "ThreeRight";
    }
    return "Unknown";
}

// ── CompositorSlotBounds ──────────────────────────────────────────────────────

struct CompositorSlotBounds {
    ViewportHandle handle = kInvalidViewportHandle;
    ViewportBounds bounds;
};

// ── ViewportCompositor ────────────────────────────────────────────────────────

class ViewportCompositor {
public:
    void setLayoutMode(ViewportLayoutMode mode) { m_mode = mode; }
    [[nodiscard]] ViewportLayoutMode layoutMode() const { return m_mode; }
    [[nodiscard]] const char* layoutName() const { return viewportLayoutModeName(m_mode); }

    /// Number of sub-regions this layout produces.
    [[nodiscard]] uint32_t regionCount() const {
        switch (m_mode) {
        case ViewportLayoutMode::Single:      return 1;
        case ViewportLayoutMode::SideBySide:  return 2;
        case ViewportLayoutMode::TwoByTwo:    return 4;
        case ViewportLayoutMode::ThreeLeft:   return 3;
        case ViewportLayoutMode::ThreeRight:  return 3;
        }
        return 1;
    }

    /// Compute per-slot bounds given the full viewport region and ordered handles.
    /// Handles are assigned to sub-regions in index order; excess handles are ignored;
    /// missing handles leave regions empty (no CompositorSlotBound emitted).
    [[nodiscard]] std::vector<CompositorSlotBounds> computeSlotBounds(
        const ViewportBounds& fullBounds,
        const std::vector<ViewportHandle>& handles) const
    {
        auto regions = computeRegions(fullBounds);
        std::vector<CompositorSlotBounds> result;
        result.reserve(std::min(regions.size(), handles.size()));
        for (size_t i = 0; i < regions.size() && i < handles.size(); ++i)
            result.push_back({handles[i], regions[i]});
        return result;
    }

private:
    ViewportLayoutMode m_mode = ViewportLayoutMode::Single;

    [[nodiscard]] std::vector<ViewportBounds> computeRegions(const ViewportBounds& full) const {
        float x = full.x, y = full.y;
        float w = full.width, h = full.height;
        float hw = w * 0.5f, hh = h * 0.5f;

        switch (m_mode) {
        case ViewportLayoutMode::Single:
            return {{x, y, w, h}};

        case ViewportLayoutMode::SideBySide:
            return {{x, y, hw, h}, {x + hw, y, hw, h}};

        case ViewportLayoutMode::TwoByTwo:
            return {
                {x,      y,      hw, hh},
                {x + hw, y,      hw, hh},
                {x,      y + hh, hw, hh},
                {x + hw, y + hh, hw, hh},
            };

        case ViewportLayoutMode::ThreeLeft: {
            float lw = w * (2.f / 3.f);
            float rw = w - lw;
            return {
                {x,      y,      lw, h},
                {x + lw, y,      rw, hh},
                {x + lw, y + hh, rw, hh},
            };
        }

        case ViewportLayoutMode::ThreeRight: {
            float lw = w / 3.f;
            float rw = w - lw;
            return {
                {x,      y,      lw, hh},
                {x,      y + hh, lw, hh},
                {x + lw, y,      rw, h},
            };
        }
        }
        return {{x, y, w, h}};
    }
};

} // namespace NF
