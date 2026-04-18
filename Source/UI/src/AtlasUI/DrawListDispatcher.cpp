#include "NF/UI/AtlasUI/DrawListDispatcher.h"
#include "NF/UI/UIBackend.h"
#include <variant>

namespace NF::UI::AtlasUI {

void DrawListDispatcher::dispatch(const DrawList& drawList) const {
    if (!m_renderer) return;

    NF::UIBackend* backend = m_renderer->backend();

    // Pass 1: emit all geometry (filled and outlined rects) into the UIRenderer
    // vertex buffer.  Text and clip commands are skipped here — text must be
    // drawn AFTER backgrounds are flushed so it appears on top of them, and
    // clip commands are unsupported in the GDI batch path (silently ignored).
    for (const DrawCommand& cmd : drawList.commands()) {
        std::visit([this](const auto& c) {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, FillRectCmd>) {
                m_renderer->drawRect(c.rect, atlasToRendererColor(c.color));
            }
            else if constexpr (std::is_same_v<T, DrawRectCmd>) {
                m_renderer->drawRectOutline(c.rect, atlasToRendererColor(c.color), 1.f);
            }
            else if constexpr (std::is_same_v<T, DrawImageCmd>) {
                // DrawImage: render the scene texture into the viewport rect.
                // The UIRenderer draws a textured quad; tint color is passed through.
                m_renderer->drawImage(c.rect, c.textureId, atlasToRendererColor(c.tint));
            }
            // DrawTextCmd: deferred to pass 2.
            // PushClipCmd / PopClipCmd: GDI batch path has no clip support; silently ignored.
        }, cmd);
    }

    // Flush all accumulated geometry (this panel's backgrounds + any earlier
    // batched draws) to the GDI memDC so that text drawn next appears on top.
    // Per-panel flushing is required for correct text-over-background ordering:
    // the GDI rasterisation path has no depth/stencil — draw order is all we have.
    m_renderer->flush();

    // Pass 2: draw text commands on top of the flushed geometry.
    for (const DrawCommand& cmd : drawList.commands()) {
        std::visit([this, backend](const auto& c) {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, DrawTextCmd>) {
                uint32_t col = atlasToRendererColor(c.color);
                if (backend) {
                    // Prefer native platform text (GDI TextOutA on Win32).
                    backend->drawTextNative(c.rect.x, c.rect.y, c.text, col);
                } else {
                    // Fallback: pixel-block characters via UIRenderer.
                    m_renderer->drawText(c.rect.x, c.rect.y, c.text, col);
                }
            }
            // PushClipCmd / PopClipCmd: see pass 1 comment — silently ignored.
        }, cmd);
    }
}

} // namespace NF::UI::AtlasUI
