#pragma once
#include <cstdint>
#include <string>

namespace atlas::ui {

struct UIColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};

struct UIRect {
    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
};

class UIRenderer {
public:
    virtual ~UIRenderer() = default;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void DrawRect(const UIRect& rect, const UIColor& color) = 0;
    virtual void DrawText(const UIRect& rect, const std::string& text, const UIColor& color) = 0;
    virtual void DrawIcon(const UIRect& rect, uint32_t iconId, const UIColor& tint) = 0;
    virtual void DrawBorder(const UIRect& rect, int32_t thickness, const UIColor& color) = 0;
    virtual void DrawImage(const UIRect& rect, uint32_t textureId, const UIColor& tint) = 0;
};

class NullUIRenderer : public UIRenderer {
public:
    void BeginFrame() override {}
    void EndFrame() override {}

    void DrawRect(const UIRect& /*rect*/, const UIColor& /*color*/) override {}
    void DrawText(const UIRect& /*rect*/, const std::string& /*text*/, const UIColor& /*color*/) override {}
    void DrawIcon(const UIRect& /*rect*/, uint32_t /*iconId*/, const UIColor& /*tint*/) override {}
    void DrawBorder(const UIRect& /*rect*/, int32_t /*thickness*/, const UIColor& /*color*/) override {}
    void DrawImage(const UIRect& /*rect*/, uint32_t /*textureId*/, const UIColor& /*tint*/) override {}
};

} // namespace atlas::ui
