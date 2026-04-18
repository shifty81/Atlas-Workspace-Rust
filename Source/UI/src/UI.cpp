#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"

namespace NF {

void UIRenderer::flush() {
    if (m_backend && !m_vertices.empty()) {
        m_backend->flush(m_vertices.data(), m_vertices.size(),
                         m_indices.data(), m_indices.size());
        m_vertices.clear();
        m_indices.clear();
    }
}

void UIRenderer::drawText(float x, float y, std::string_view text, uint32_t color) {
    if (m_backend) {
        // Flush any pending geometry so backgrounds are drawn before text.
        flush();
        m_backend->drawTextNative(x, y, text, color);
        ++m_textDrawCount;
        return;
    }
    // Fallback (headless / no backend): draw a coloured rect per glyph.
    constexpr float kCharWidth  = 8.f;
    constexpr float kCharHeight = 14.f;
    float cx = x;
    float curY = y; // local Y cursor for multi-line support
    for (char ch : text) {
        if (ch == '\n') { cx = x; curY += kCharHeight + 2.f; continue; }
        if (ch == ' ')  { cx += kCharWidth; continue; }
        Rect charRect{cx, curY, kCharWidth, kCharHeight};
        drawRect(charRect, color);
        cx += kCharWidth;
    }
    ++m_textDrawCount;
}

void UIRenderer::endFrame() {
    // Flush any remaining batched geometry to the active backend.
    flush();
    m_lastFrameQuadCount = m_quadCount;
    m_lastFrameTextCount = m_textDrawCount;
}

Vec2 UIRenderer::measureText(std::string_view text, float fontSize) const {
    if (m_backend) return m_backend->measureText(text, fontSize);
    // Fallback: fixed-width 8×14 glyphs
    constexpr float kCharWidth  = 8.f;
    constexpr float kCharHeight = 14.f;
    float maxW = 0.f, cx = 0.f, lines = 1.f;
    for (char ch : text) {
        if (ch == '\n') { maxW = std::max(maxW, cx); cx = 0.f; lines += 1.f; continue; }
        cx += kCharWidth;
    }
    return {std::max(maxW, cx), lines * (kCharHeight + 2.f)};
}

} // namespace NF
