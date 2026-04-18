#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"
#include <unordered_set>

namespace NF::UI::AtlasUI {

class BasicLayoutContext final : public ILayoutContext {
public:
    BasicLayoutContext(float dpiScaleValue, NF::Vec2 availableSizeValue)
        : m_dpiScale(dpiScaleValue), m_availableSize(availableSizeValue) {}

    [[nodiscard]] float dpiScale() const override { return m_dpiScale; }
    [[nodiscard]] NF::Vec2 availableSize() const override { return m_availableSize; }
    [[nodiscard]] NF::Vec2 measureText(std::string_view text, float) const override {
        constexpr float kCharWidth = 8.f;
        constexpr float kCharHeight = 16.f;
        float width = 0.f;
        float maxWidth = 0.f;
        float lines = 1.f;
        for (char ch : text) {
            if (ch == '\n') {
                maxWidth = std::max(maxWidth, width);
                width = 0.f;
                lines += 1.f;
                continue;
            }
            width += kCharWidth;
        }
        maxWidth = std::max(maxWidth, width);
        return {maxWidth, lines * kCharHeight};
    }
    void invalidateLayout() override { m_layoutInvalidated = true; }
    [[nodiscard]] bool layoutInvalidated() const { return m_layoutInvalidated; }

private:
    float m_dpiScale = 1.f;
    NF::Vec2 m_availableSize{};
    bool m_layoutInvalidated = false;
};

class BasicPaintContext final : public IPaintContext {
public:
    void drawRect(const NF::Rect& rect, Color color) override { m_drawList.push(DrawRectCmd{rect, color}); }
    void fillRect(const NF::Rect& rect, Color color) override { m_drawList.push(FillRectCmd{rect, color}); }
    void drawText(const NF::Rect& rect, std::string_view text, FontId font, Color color) override {
        m_drawList.push(DrawTextCmd{rect, std::string(text), font, color});
    }
    void drawImage(const NF::Rect& rect, uint32_t textureId, Color tint) override {
        m_drawList.push(DrawImageCmd{rect, textureId, tint});
    }
    void pushClip(const NF::Rect& rect) override { m_drawList.push(PushClipCmd{rect}); }
    void popClip() override { m_drawList.push(PopClipCmd{}); }
    [[nodiscard]] DrawList& drawList() override { return m_drawList; }
    [[nodiscard]] const DrawList& drawList() const { return m_drawList; }

private:
    DrawList m_drawList;
};

class BasicInputContext final : public IInputContext {
public:
    void setMousePosition(NF::Vec2 position) { m_mousePosition = position; }
    void setPrimaryDown(bool down) { m_primaryDown = down; }
    void setPrimaryPressed(bool pressed) { m_primaryPressed = pressed; }
    void setSecondaryDown(bool down) { m_secondaryDown = down; }
    void setScrollDelta(float delta) { m_scrollDelta = delta; }
    void setKeyDown(int keyCode, bool down) {
        if (down) {
            m_keysDown.insert(keyCode);
        } else {
            m_keysDown.erase(keyCode);
        }
    }

    void setTypedText(std::string_view text) { m_typedText = text; }

    [[nodiscard]] NF::Vec2 mousePosition() const override { return m_mousePosition; }
    [[nodiscard]] bool primaryDown() const override { return m_primaryDown; }
    [[nodiscard]] bool primaryPressed() const override { return m_primaryPressed; }
    [[nodiscard]] bool secondaryDown() const override { return m_secondaryDown; }
    [[nodiscard]] float scrollDelta() const override { return m_scrollDelta; }
    [[nodiscard]] bool keyDown(int keyCode) const override { return m_keysDown.contains(keyCode); }
    [[nodiscard]] std::string_view typedText() const override { return m_typedText; }
    void requestFocus(IWidget* widget) override { m_focusWidget = widget; }
    void capturePointer(IWidget* widget) override { m_captureWidget = widget; }
    void releasePointer(IWidget* widget) override {
        if (m_captureWidget == widget) {
            m_captureWidget = nullptr;
        }
    }

    [[nodiscard]] IWidget* focusWidget() const { return m_focusWidget; }
    [[nodiscard]] IWidget* captureWidget() const { return m_captureWidget; }

private:
    NF::Vec2 m_mousePosition{};
    bool m_primaryDown    = false;
    bool m_primaryPressed = false;
    bool m_secondaryDown  = false;
    float m_scrollDelta   = 0.f;
    std::string m_typedText;
    std::unordered_set<int> m_keysDown;
    IWidget* m_focusWidget  = nullptr;
    IWidget* m_captureWidget = nullptr;
};

} // namespace NF::UI::AtlasUI
