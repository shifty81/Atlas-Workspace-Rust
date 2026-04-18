#pragma once
#include "../ui/UIRenderer.h"

#ifdef DrawText
#undef DrawText
#endif

namespace atlas::render {

class GLRenderer : public ui::UIRenderer {
public:
    void BeginFrame() override;
    void EndFrame() override;

    void DrawRect(const ui::UIRect& rect, const ui::UIColor& color) override;
    void DrawText(const ui::UIRect& rect, const std::string& text, const ui::UIColor& color) override;
    void DrawIcon(const ui::UIRect& rect, uint32_t iconId, const ui::UIColor& tint) override;
    void DrawBorder(const ui::UIRect& rect, int32_t thickness, const ui::UIColor& color) override;
    void DrawImage(const ui::UIRect& rect, uint32_t textureId, const ui::UIColor& tint) override;

    void SetViewport(int32_t width, int32_t height) override;

private:
    int32_t m_viewportWidth = 1280;
    int32_t m_viewportHeight = 720;
};

} // namespace atlas::render
