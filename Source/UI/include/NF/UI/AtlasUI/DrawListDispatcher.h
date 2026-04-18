#pragma once
// NF::UI::AtlasUI::DrawListDispatcher
// Walks an AtlasUI DrawList and dispatches each command to the UIRenderer
// (and, when available, the platform UIBackend for native text rendering).
//
// Color format notes:
//   AtlasUI DrawPrimitives use AARRGGBB (e.g. 0xFF3A7AFE = opaque blue).
//   UIRenderer / GDIBackend expect RRGGBBAA (e.g. 0x3A7AFEFF = same blue).
//   The dispatcher converts automatically via atlasToRendererColor().

#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/UI.h"

namespace NF::UI::AtlasUI {

class DrawListDispatcher {
public:
    /// Bind the UIRenderer used for filled/outlined rect commands.
    void setRenderer(NF::UIRenderer* renderer) { m_renderer = renderer; }
    [[nodiscard]] NF::UIRenderer* renderer() const { return m_renderer; }

    /// Walk every command in drawList and dispatch to the renderer/backend.
    /// FillRectCmd  → UIRenderer::drawRect
    /// DrawRectCmd  → UIRenderer::drawRectOutline (1px border)
    /// DrawTextCmd  → UIBackend::drawTextNative if backend present, else UIRenderer::drawText
    /// PushClipCmd / PopClipCmd — intentionally ignored: the GDI batched-quad
    ///   pipeline has no clip-rect support; silently no-ops so panel paint never crashes.
    void dispatch(const DrawList& drawList) const;

private:
    // Convert AARRGGBB (AtlasUI) → RRGGBBAA (UIRenderer/GDIBackend).
    static constexpr uint32_t atlasToRendererColor(uint32_t c) {
        return (c << 8u) | (c >> 24u);
    }

    NF::UIRenderer* m_renderer = nullptr;
};

} // namespace NF::UI::AtlasUI
