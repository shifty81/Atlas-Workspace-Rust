#pragma once
// NF::ToolViewRenderContext — per-frame render context passed to IHostedTool::renderToolView().
//
// WorkspaceRenderer::renderActiveToolView() builds one of these each frame and
// passes it to the active tool.  The tool uses it to draw its own panel layout
// (hierarchy, viewport, inspector, etc.) backed by the tool's live runtime state
// rather than static placeholder text.
//
// The shared color palette and drawPanel helper are provided here so that every
// tool produces a visually consistent chrome without duplicating the definitions.
//
// WorkspaceShell is passed by pointer (never null in practice, but nullable so
// that unit tests can exercise renderToolView without a full shell).
//
// uiCtx — nullable UIContext* provided by WorkspaceRenderer so that tools can
// register hit-regions for interactive click handling.  Always non-null in the
// live runtime path; null only in headless tests that call renderToolView directly.
// Tools must guard every uiCtx call with a null check, or use the hitRegion()
// / isHovered() convenience helpers that do so automatically.

#include "NF/UI/UI.h"
#include "NF/UI/UIWidgets.h"
#include <cstdio>
#include <cstring>

namespace NF {

class WorkspaceShell; // forward – avoids circular dep (WorkspaceShell → ToolRegistry → IHostedTool)

// ── ToolViewRenderContext ─────────────────────────────────────────

struct ToolViewRenderContext {
    UIRenderer&          ui;
    const UIMouseState&  mouse;
    float                x, y, w, h;   // bounding box of the tool content area
    WorkspaceShell*      shell  = nullptr;
    UIContext*           uiCtx  = nullptr; // nullable — null in headless tests

    // ── Shared color palette (RRGGBBAA, matches GDIBackend convention) ─
    static constexpr uint32_t kSurface      = 0x252525FF;
    static constexpr uint32_t kBorder       = 0x3C3C3CFF;
    static constexpr uint32_t kCardBg       = 0x2E2E2EFF;
    static constexpr uint32_t kButtonBg     = 0x3A3A3AFF;
    static constexpr uint32_t kAccentBlue   = 0x007ACCFF;
    static constexpr uint32_t kGreen        = 0x4EC94EFF;
    static constexpr uint32_t kRed          = 0xE05050FF;
    static constexpr uint32_t kTextPrimary  = 0xE0E0E0FF;
    static constexpr uint32_t kTextSecond   = 0x888888FF;
    static constexpr uint32_t kTextMuted    = 0x555555FF;

    // ── Click / hover helpers ─────────────────────────────────────
    // These delegate to uiCtx when non-null.  Safe to call from const
    // renderToolView() implementations.

    // Returns true when the user completes a left-click inside |r|.
    // Draws a subtle hover highlight when hovered (if uiCtx is set).
    // Always returns false when uiCtx is null (headless / test path).
    [[nodiscard]] bool hitRegion(const Rect& r, bool drawHover = true) const {
        if (!uiCtx) return false;
        return uiCtx->hitRegion(r, drawHover);
    }

    // Returns true when the mouse cursor is currently inside |r|.
    // Uses the mouse snapshot embedded in the context — no uiCtx needed.
    [[nodiscard]] bool isHovered(const Rect& r) const {
        return r.contains(mouse.x, mouse.y);
    }

    // Draws a button-style rect at (px,py,pw,ph) with |label|.
    // Returns true when clicked. Honours hover tint automatically.
    [[nodiscard]] bool drawButton(float px, float py, float pw, float ph,
                                   const char* label,
                                   uint32_t normalBg = kButtonBg,
                                   uint32_t hoverBg  = 0x4A4A4AFF) const {
        Rect r{px, py, pw, ph};
        bool hov = isHovered(r);
        ui.drawRect(r, hov ? hoverBg : normalBg);
        ui.drawRectOutline(r, kBorder, 1.f);
        float tw = static_cast<float>(std::strlen(label)) * 7.f;
        ui.drawText(px + (pw - tw) * 0.5f, py + (ph - 14.f) * 0.5f, label, kTextPrimary);
        return hitRegion(r, false);
    }

    // ── Panel chrome helper ───────────────────────────────────────
    // Draws a titled panel zone with an optional centered hint string.
    // px/py/pw/ph are absolute screen coordinates inside the tool area.
    void drawPanel(float px, float py, float pw, float ph,
                   const char* title, const char* hint = nullptr) const {
        ui.drawRect({px, py, pw, ph}, kCardBg);
        ui.drawRectOutline({px, py, pw, ph}, kBorder, 1.f);
        // header strip
        ui.drawRect({px, py, pw, 22.f}, kSurface);
        ui.drawRect({px, py + 22.f, pw, 1.f}, kBorder);
        ui.drawText(px + 8.f, py + 4.f, title, kTextSecond);
        if (hint) {
            float hx = px + (pw - static_cast<float>(std::strlen(hint)) * 8.f) * 0.5f;
            float hy = py + 22.f + (ph - 22.f - 14.f) * 0.5f;
            ui.drawText(hx, hy, hint, kTextMuted);
        }
    }

    // ── Status pill helper ────────────────────────────────────────
    // Draws a small pill (px, py) with label text.
    void drawStatusPill(float px, float py, const char* label, uint32_t pillColor) const {
        float labelW = static_cast<float>(std::strlen(label)) * 7.f + 12.f;
        ui.drawRect({px, py, labelW, 18.f}, pillColor);
        ui.drawText(px + 6.f, py + 2.f, label, kTextPrimary);
    }

    // ── Stat row helper ───────────────────────────────────────────
    // Draws "key: value" on one row at (px, py).
    void drawStatRow(float px, float py, const char* key, const char* value) const {
        ui.drawText(px,       py, key,   kTextSecond);
        ui.drawText(px + 110.f, py, value, kTextPrimary);
    }

    // ── Interactive slider helper ──────────────────────────────────
    // Draws a labeled slider at (px, py) with the given label and a mutable
    // float value clamped to [minVal, maxVal].  Returns true when the value
    // was changed by the user.  The slider is 14px tall and uses the full
    // row width (rowW).  Label occupies 48px; value display occupies 48px;
    // the track fills the remainder.
    //
    // This is a self-contained immediate-mode slider — it does NOT require
    // UIContext layout or begin()/end().  It reads mouse state directly from
    // the context's mouse snapshot.
    bool drawSlider(float px, float py, float rowW, const char* label,
                    float& value, float minVal, float maxVal) const {
        // Layout
        constexpr float kLabelW  = 48.f;
        constexpr float kValueW  = 48.f;
        constexpr float kTrackH  = 6.f;
        constexpr float kHandleW = 8.f;
        constexpr float kHandleH = 14.f;
        constexpr float kRowH    = 16.f;

        const float trackX = px + kLabelW;
        const float trackW = rowW - kLabelW - kValueW;
        const float trackY = py + (kRowH - kTrackH) * 0.5f;

        // Label
        ui.drawText(px, py, label, kTextSecond);

        // Track background
        ui.drawRect({trackX, trackY, trackW, kTrackH}, 0x404040FF);
        ui.drawRectOutline({trackX, trackY, trackW, kTrackH}, kBorder, 1.f);

        // Fill portion
        constexpr float kMinRange = 0.001f; // minimum valid range to avoid division by zero
        float range = maxVal - minVal;
        float frac  = (range > kMinRange) ? (value - minVal) / range : 0.f;
        if (frac < 0.f) frac = 0.f;
        if (frac > 1.f) frac = 1.f;
        ui.drawRect({trackX, trackY, trackW * frac, kTrackH}, kAccentBlue);

        // Handle
        float handleX = trackX + (trackW - kHandleW) * frac;
        float handleY = py + (kRowH - kHandleH) * 0.5f;
        bool  handleHov = Rect{handleX, handleY, kHandleW, kHandleH}.contains(mouse.x, mouse.y);
        ui.drawRect({handleX, handleY, kHandleW, kHandleH},
                    handleHov ? 0xCCCCCCFF : kTextPrimary);

        // Value text
        char valBuf[16];
        std::snprintf(valBuf, sizeof(valBuf), "%.1f", value);
        ui.drawText(px + kLabelW + trackW + 4.f, py, valBuf, kTextPrimary);

        // Interaction — drag on track or handle changes value
        bool changed = false;
        Rect trackHitR{trackX, py - 2.f, trackW, kRowH + 4.f};
        if (trackHitR.contains(mouse.x, mouse.y) && mouse.leftDown) {
            float newFrac = (mouse.x - trackX) / trackW;
            if (newFrac < 0.f) newFrac = 0.f;
            if (newFrac > 1.f) newFrac = 1.f;
            float newVal = minVal + newFrac * range;
            if (newVal != value) {
                value   = newVal;
                changed = true;
            }
        }
        return changed;
    }
};

} // namespace NF
