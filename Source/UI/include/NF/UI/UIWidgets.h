#pragma once
// NF::UIContext — Immediate-mode widget toolkit built on UIRenderer.
// All widgets draw through UIRenderer primitives and use EditorTheme colours.
// Provides buttons, checkboxes, sliders, text input, tree nodes, scroll areas,
// layout helpers (horizontal/vertical stacking), and tooltip support.
#include "NF/Core/Core.h"
#include "NF/UI/UI.h"
#include "NF/Input/Input.h"

#include <functional>

namespace NF {

// Forward declare EditorTheme (defined in Editor.h) — we accept it by const-ref.
// To avoid circular dependency we define a minimal UITheme here that EditorTheme
// can be converted to.  EditorTheme IS-A UITheme by duck-typing (matching fields).

struct UITheme {
    // Panel
    uint32_t panelBackground     = 0x2B2B2BFF;
    uint32_t panelHeader         = 0x3C3C3CFF;
    uint32_t panelBorder         = 0x555555FF;
    uint32_t panelText           = 0xDCDCDCFF;

    // Selection
    uint32_t selectionHighlight  = 0x264F78FF;
    uint32_t selectionBorder     = 0x007ACCFF;
    uint32_t hoverHighlight      = 0x383838FF;

    // Input fields
    uint32_t inputBackground     = 0x1E1E1EFF;
    uint32_t inputBorder         = 0x474747FF;
    uint32_t inputText           = 0xD4D4D4FF;
    uint32_t inputFocusBorder    = 0x007ACCFF;

    // Buttons
    uint32_t buttonBackground    = 0x3C3C3CFF;
    uint32_t buttonHover         = 0x505050FF;
    uint32_t buttonPressed       = 0x007ACCFF;
    uint32_t buttonText          = 0xCCCCCCFF;
    uint32_t buttonDisabledText  = 0x666666FF;

    // Toolbar
    uint32_t toolbarBackground   = 0x333333FF;
    uint32_t toolbarSeparator    = 0x4A4A4AFF;

    // Status bar
    uint32_t statusBarBackground = 0x007ACCFF;
    uint32_t statusBarText       = 0xFFFFFFFF;

    // Viewport
    uint32_t viewportBackground  = 0x1A1A1AFF;
    uint32_t gridColor           = 0x333333FF;

    // Properties
    uint32_t propertyLabel       = 0xBBBBBBFF;
    uint32_t propertyValue       = 0xE0E0E0FF;
    uint32_t propertySeparator   = 0x404040FF;
    uint32_t dirtyIndicator      = 0xE8A435FF;

    // Sizes
    float fontSize               = 14.f;
    float headerFontSize         = 16.f;
    float smallFontSize          = 12.f;
    float panelPadding           = 8.f;
    float itemSpacing            = 4.f;
    float sectionSpacing         = 12.f;

    // Extras for widgets
    uint32_t scrollbarTrack      = 0x2A2A2AFF;
    uint32_t scrollbarThumb      = 0x555555FF;
    uint32_t scrollbarThumbHover = 0x777777FF;
    uint32_t treeNodeExpander     = 0x888888FF;
    uint32_t progressFill        = 0x007ACCFF;
    uint32_t progressBackground  = 0x333333FF;
    uint32_t tooltipBackground   = 0x1E1E1EFF;
    uint32_t tooltipBorder       = 0x555555FF;
    uint32_t tooltipText         = 0xE0E0E0FF;
    uint32_t errorColor          = 0xF44747FF;
    uint32_t warningColor        = 0xE8A435FF;
    uint32_t successColor        = 0x4EC9B0FF;

    static UITheme dark()  { return {}; }
    static UITheme light() {
        UITheme t;
        t.panelBackground    = 0xF0F0F0FF;
        t.panelHeader        = 0xE0E0E0FF;
        t.panelBorder        = 0xCCCCCCFF;
        t.panelText          = 0x1E1E1EFF;
        t.selectionHighlight = 0xCCE8FFFF;
        t.selectionBorder    = 0x0078D4FF;
        t.hoverHighlight     = 0xE5E5E5FF;
        t.inputBackground    = 0xFFFFFFFF;
        t.inputBorder        = 0xCCCCCCFF;
        t.inputText          = 0x1E1E1EFF;
        t.buttonBackground   = 0xE0E0E0FF;
        t.buttonHover        = 0xD0D0D0FF;
        t.buttonText         = 0x1E1E1EFF;
        t.toolbarBackground  = 0xE8E8E8FF;
        t.statusBarBackground= 0x0078D4FF;
        t.viewportBackground = 0xD4D4D4FF;
        t.gridColor          = 0xBBBBBBFF;
        t.propertyLabel      = 0x444444FF;
        t.propertyValue      = 0x1E1E1EFF;
        t.scrollbarTrack     = 0xDDDDDDFF;
        t.scrollbarThumb     = 0xAAAAAAAA;
        t.tooltipBackground  = 0xFAFAFAFF;
        t.tooltipBorder      = 0xCCCCCCFF;
        t.tooltipText        = 0x1E1E1EFF;
        return t;
    }
};

// ── Widget ID ────────────────────────────────────────────────────
// Simple FNV-1a hash for widget identification.

inline uint32_t uiHash(std::string_view str) {
    uint32_t hash = 2166136261u;
    for (char c : str) {
        hash ^= static_cast<uint32_t>(c);
        hash *= 16777619u;
    }
    return hash;
}

// ── Mouse state snapshot ─────────────────────────────────────────

struct UIMouseState {
    float x = 0.f, y = 0.f;
    bool  leftDown    = false;
    bool  leftPressed = false;   // just pressed this frame
    bool  leftReleased = false;  // just released this frame
    float scrollDelta  = 0.f;
    std::string typedText;       // characters from WM_CHAR this frame; '\b'=Backspace, '\r'=Enter
};

// ── UIContext ────────────────────────────────────────────────────

class UIContext {
public:
    // ── Frame lifecycle ──────────────────────────────────────────

    void begin(UIRenderer& renderer, const UIMouseState& mouse, const UITheme& theme, float dt) {
        m_renderer = &renderer;
        m_mouse    = mouse;
        m_theme    = &theme;
        m_dt       = dt;
        m_cursorStack.clear();
        m_tooltipText.clear();
        m_clipStack.clear();
    }

    void end() {
        // Render tooltip if set
        if (!m_tooltipText.empty() && m_renderer) {
            float tw = static_cast<float>(m_tooltipText.size()) * 8.f + 12.f;
            float th = 20.f;
            float tx = m_mouse.x + 12.f;
            float ty = m_mouse.y + 16.f;
            m_renderer->drawRect({tx, ty, tw, th}, m_theme->tooltipBackground);
            m_renderer->drawRectOutline({tx, ty, tw, th}, m_theme->tooltipBorder, 1.f);
            m_renderer->drawText(tx + 6.f, ty + 3.f, m_tooltipText, m_theme->tooltipText);
        }
        m_renderer = nullptr;
    }

    // ── Layout helpers ───────────────────────────────────────────

    void beginPanel(std::string_view title, const Rect& bounds) {
        if (!m_renderer) return;

        // Push layout cursor
        LayoutState ls;
        ls.bounds = bounds;
        ls.cursorX = bounds.x + m_theme->panelPadding;
        ls.cursorY = bounds.y;
        ls.direction = LayoutDir::Vertical;
        m_cursorStack.push_back(ls);

        // Draw panel background
        m_renderer->drawRect(bounds, m_theme->panelBackground);

        // Draw header bar
        float headerH = 22.f;
        Rect headerR{bounds.x, bounds.y, bounds.w, headerH};
        m_renderer->drawRect(headerR, m_theme->panelHeader);
        m_renderer->drawText(bounds.x + m_theme->panelPadding, bounds.y + 4.f,
                             title, m_theme->panelText);

        // Draw border
        m_renderer->drawRectOutline(bounds, m_theme->panelBorder, 1.f);

        // Advance cursor past header
        currentLayout().cursorY += headerH + m_theme->itemSpacing;
    }

    void endPanel() {
        if (!m_cursorStack.empty()) m_cursorStack.pop_back();
    }

    void beginHorizontal() {
        if (m_cursorStack.empty()) return;
        auto& cur = currentLayout();
        LayoutState ls;
        ls.bounds = cur.bounds;
        ls.cursorX = cur.cursorX;
        ls.cursorY = cur.cursorY;
        ls.direction = LayoutDir::Horizontal;
        ls.maxItemHeight = 0.f;
        m_cursorStack.push_back(ls);
    }

    void endHorizontal() {
        if (m_cursorStack.size() < 2) return;
        float maxH = currentLayout().maxItemHeight;
        m_cursorStack.pop_back();
        if (!m_cursorStack.empty()) {
            currentLayout().cursorY += maxH + m_theme->itemSpacing;
        }
    }

    void beginVertical() {
        if (m_cursorStack.empty()) return;
        auto& cur = currentLayout();
        LayoutState ls;
        ls.bounds = cur.bounds;
        ls.cursorX = cur.cursorX;
        ls.cursorY = cur.cursorY;
        ls.direction = LayoutDir::Vertical;
        m_cursorStack.push_back(ls);
    }

    void endVertical() {
        if (m_cursorStack.size() < 2) return;
        m_cursorStack.pop_back();
    }

    void indent(float px) {
        if (!m_cursorStack.empty()) currentLayout().cursorX += px;
    }

    void unindent(float px) {
        if (!m_cursorStack.empty()) currentLayout().cursorX -= px;
    }

    void separator() {
        if (m_cursorStack.empty() || !m_renderer) return;
        auto& ls = currentLayout();
        float w = ls.bounds.x + ls.bounds.w - ls.cursorX - m_theme->panelPadding;
        m_renderer->drawRect({ls.cursorX, ls.cursorY, w, 1.f}, m_theme->propertySeparator);
        ls.cursorY += 1.f + m_theme->sectionSpacing;
    }

    void spacing(float px) {
        if (!m_cursorStack.empty()) currentLayout().cursorY += px;
    }

    // ── Widgets ──────────────────────────────────────────────────

    bool button(std::string_view label, float width = 0.f) {
        if (m_cursorStack.empty() || !m_renderer) return false;
        auto& ls = currentLayout();

        float btnW = width > 0.f ? width : (static_cast<float>(label.size()) * 8.f + 16.f);
        float btnH = 22.f;
        Rect r{ls.cursorX, ls.cursorY, btnW, btnH};

        uint32_t id = uiHash(label);
        bool hovered = hitTest(r);
        bool pressed = hovered && m_mouse.leftPressed;
        bool clicked = hovered && m_mouse.leftReleased;

        uint32_t bg = m_theme->buttonBackground;
        if (pressed)       bg = m_theme->buttonPressed;
        else if (hovered)  bg = m_theme->buttonHover;

        m_renderer->drawRect(r, bg);
        m_renderer->drawRectOutline(r, m_theme->panelBorder, 1.f);
        m_renderer->drawText(r.x + 8.f, r.y + 4.f, label, m_theme->buttonText);

        advanceCursor(btnW, btnH);
        (void)id;
        return clicked;
    }

    bool checkbox(std::string_view label, bool& value) {
        if (m_cursorStack.empty() || !m_renderer) return false;
        auto& ls = currentLayout();

        float boxSize = 14.f;
        float totalW = boxSize + 6.f + static_cast<float>(label.size()) * 8.f;
        float h = 18.f;
        Rect boxR{ls.cursorX, ls.cursorY + 2.f, boxSize, boxSize};

        bool hovered = hitTest(boxR);
        bool toggled = false;
        if (hovered && m_mouse.leftReleased) {
            value = !value;
            toggled = true;
        }

        m_renderer->drawRect(boxR, m_theme->inputBackground);
        m_renderer->drawRectOutline(boxR, hovered ? m_theme->inputFocusBorder : m_theme->inputBorder, 1.f);

        if (value) {
            // Draw checkmark as a smaller filled rect
            Rect check{boxR.x + 3.f, boxR.y + 3.f, boxSize - 6.f, boxSize - 6.f};
            m_renderer->drawRect(check, m_theme->selectionBorder);
        }

        m_renderer->drawText(ls.cursorX + boxSize + 6.f, ls.cursorY + 2.f,
                             label, m_theme->panelText);
        advanceCursor(totalW, h);
        return toggled;
    }

    void label(std::string_view text, uint32_t color = 0) {
        if (m_cursorStack.empty() || !m_renderer) return;
        auto& ls = currentLayout();
        uint32_t c = (color != 0) ? color : m_theme->panelText;
        m_renderer->drawText(ls.cursorX, ls.cursorY, text, c);
        advanceCursor(static_cast<float>(text.size()) * 8.f, 16.f);
    }

    void headerLabel(std::string_view text) {
        if (m_cursorStack.empty() || !m_renderer) return;
        auto& ls = currentLayout();
        m_renderer->drawText(ls.cursorX, ls.cursorY, text, m_theme->panelText);
        advanceCursor(static_cast<float>(text.size()) * 8.f, 18.f);
    }

    bool textInput(std::string_view label, std::string& value) {
        if (m_cursorStack.empty() || !m_renderer) return false;
        auto& ls = currentLayout();

        float labelW = static_cast<float>(label.size()) * 8.f + 8.f;
        float inputW = ls.bounds.x + ls.bounds.w - ls.cursorX - m_theme->panelPadding - labelW;
        if (inputW < 40.f) inputW = 40.f;
        float h = 20.f;

        // Label
        m_renderer->drawText(ls.cursorX, ls.cursorY + 3.f, label, m_theme->propertyLabel);

        // Input box
        Rect inputR{ls.cursorX + labelW, ls.cursorY, inputW, h};
        bool hovered = hitTest(inputR);

        m_renderer->drawRect(inputR, m_theme->inputBackground);
        m_renderer->drawRectOutline(inputR, hovered ? m_theme->inputFocusBorder : m_theme->inputBorder, 1.f);
        m_renderer->drawText(inputR.x + 4.f, inputR.y + 3.f, value, m_theme->inputText);

        advanceCursor(labelW + inputW, h);
        return false; // text editing requires focus tracking (future)
    }

    bool slider(std::string_view label, float& value, float minVal, float maxVal) {
        if (m_cursorStack.empty() || !m_renderer) return false;
        auto& ls = currentLayout();

        float labelW = static_cast<float>(label.size()) * 8.f + 8.f;
        float sliderW = ls.bounds.x + ls.bounds.w - ls.cursorX - m_theme->panelPadding - labelW - 48.f;
        if (sliderW < 40.f) sliderW = 40.f;
        float h = 18.f;

        // Label
        m_renderer->drawText(ls.cursorX, ls.cursorY + 2.f, label, m_theme->propertyLabel);

        // Slider track
        float trackX = ls.cursorX + labelW;
        float trackY = ls.cursorY + 6.f;
        float trackH = 6.f;
        Rect trackR{trackX, trackY, sliderW, trackH};

        m_renderer->drawRect(trackR, m_theme->inputBackground);

        // Fill portion
        float frac = (maxVal > minVal) ? (value - minVal) / (maxVal - minVal) : 0.f;
        frac = std::clamp(frac, 0.f, 1.f);
        float fillW = sliderW * frac;
        m_renderer->drawRect({trackX, trackY, fillW, trackH}, m_theme->selectionBorder);

        // Handle
        float handleX = trackX + fillW - 4.f;
        Rect handleR{handleX, ls.cursorY + 2.f, 8.f, 14.f};
        bool hovered = hitTest(trackR);
        m_renderer->drawRect(handleR, hovered ? m_theme->buttonHover : m_theme->buttonBackground);

        // Value text
        char valBuf[16];
        std::snprintf(valBuf, sizeof(valBuf), "%.2f", value);
        m_renderer->drawText(trackX + sliderW + 4.f, ls.cursorY + 2.f,
                             valBuf, m_theme->propertyValue);

        // Drag interaction
        bool changed = false;
        if (hovered && m_mouse.leftDown) {
            float relX = m_mouse.x - trackX;
            float newFrac = std::clamp(relX / sliderW, 0.f, 1.f);
            float newVal = minVal + newFrac * (maxVal - minVal);
            if (newVal != value) { value = newVal; changed = true; }
        }

        advanceCursor(labelW + sliderW + 48.f, h);
        return changed;
    }

    bool dropdown(std::string_view label, int& selected,
                  const std::vector<std::string>& options) {
        if (m_cursorStack.empty() || !m_renderer || options.empty()) return false;
        auto& ls = currentLayout();

        float labelW = static_cast<float>(label.size()) * 8.f + 8.f;
        float dropW = 120.f;
        float h = 20.f;

        m_renderer->drawText(ls.cursorX, ls.cursorY + 3.f, label, m_theme->propertyLabel);

        Rect dropR{ls.cursorX + labelW, ls.cursorY, dropW, h};
        bool hovered = hitTest(dropR);

        m_renderer->drawRect(dropR, m_theme->inputBackground);
        m_renderer->drawRectOutline(dropR, hovered ? m_theme->inputFocusBorder : m_theme->inputBorder, 1.f);

        int idx = std::clamp(selected, 0, static_cast<int>(options.size()) - 1);
        m_renderer->drawText(dropR.x + 4.f, dropR.y + 3.f, options[static_cast<size_t>(idx)], m_theme->inputText);
        // Down-arrow indicator
        m_renderer->drawText(dropR.x + dropW - 14.f, dropR.y + 3.f, "v", m_theme->inputText);

        // Simple cycle-on-click for now
        bool changed = false;
        if (hovered && m_mouse.leftReleased) {
            selected = (selected + 1) % static_cast<int>(options.size());
            changed = true;
        }

        advanceCursor(labelW + dropW, h);
        return changed;
    }

    bool treeNode(std::string_view label, bool& expanded) {
        if (m_cursorStack.empty() || !m_renderer) return false;
        auto& ls = currentLayout();

        float h = 18.f;
        float arrowW = 14.f;
        float totalW = arrowW + static_cast<float>(label.size()) * 8.f + 4.f;

        Rect hitR{ls.cursorX, ls.cursorY, totalW, h};
        bool hovered = hitTest(hitR);
        bool toggled = false;

        if (hovered && m_mouse.leftReleased) {
            expanded = !expanded;
            toggled = true;
        }

        // Hover highlight
        if (hovered) {
            m_renderer->drawRect(hitR, m_theme->hoverHighlight);
        }

        // Arrow
        std::string_view arrow = expanded ? "v" : ">";
        m_renderer->drawText(ls.cursorX, ls.cursorY + 2.f, arrow, m_theme->treeNodeExpander);

        // Label
        m_renderer->drawText(ls.cursorX + arrowW, ls.cursorY + 2.f, label, m_theme->panelText);

        advanceCursor(totalW, h);
        return expanded;
    }

    void progressBar(float fraction, float height = 4.f) {
        if (m_cursorStack.empty() || !m_renderer) return;
        auto& ls = currentLayout();

        float w = ls.bounds.x + ls.bounds.w - ls.cursorX - m_theme->panelPadding;
        if (w < 10.f) return;

        Rect bgR{ls.cursorX, ls.cursorY, w, height};
        m_renderer->drawRect(bgR, m_theme->progressBackground);

        float fillW = w * std::clamp(fraction, 0.f, 1.f);
        Rect fillR{ls.cursorX, ls.cursorY, fillW, height};
        m_renderer->drawRect(fillR, m_theme->progressFill);

        advanceCursor(w, height);
    }

    void colorSwatch(uint32_t color, float size = 16.f) {
        if (m_cursorStack.empty() || !m_renderer) return;
        auto& ls = currentLayout();

        Rect r{ls.cursorX, ls.cursorY, size, size};
        m_renderer->drawRect(r, color);
        m_renderer->drawRectOutline(r, m_theme->panelBorder, 1.f);

        advanceCursor(size + 4.f, size);
    }

    bool menuItem(std::string_view label, std::string_view hotkey = "") {
        if (m_cursorStack.empty() || !m_renderer) return false;
        auto& ls = currentLayout();

        float w = ls.bounds.x + ls.bounds.w - ls.cursorX - m_theme->panelPadding;
        float h = 20.f;
        Rect r{ls.cursorX, ls.cursorY, w, h};

        bool hovered = hitTest(r);
        bool clicked = hovered && m_mouse.leftReleased;

        if (hovered) {
            m_renderer->drawRect(r, m_theme->selectionHighlight);
        }

        m_renderer->drawText(r.x + 8.f, r.y + 3.f, label, m_theme->panelText);
        if (!hotkey.empty()) {
            float hkX = r.x + r.w - static_cast<float>(hotkey.size()) * 8.f - 8.f;
            m_renderer->drawText(hkX, r.y + 3.f, hotkey, m_theme->propertyLabel);
        }

        advanceCursor(w, h);
        return clicked;
    }

    // ── Scroll area ──────────────────────────────────────────────

    void beginScrollArea(std::string_view id, const Rect& bounds, float contentHeight) {
        if (!m_renderer) return;

        uint32_t scrollId = uiHash(id);

        // Find or create scroll state
        float& scrollY = m_scrollStates[scrollId];

        // Clamp scroll
        float maxScroll = std::max(0.f, contentHeight - bounds.h);
        if (hitTest(bounds) && m_mouse.scrollDelta != 0.f) {
            scrollY -= m_mouse.scrollDelta * 20.f;
        }
        scrollY = std::clamp(scrollY, 0.f, maxScroll);

        // Push clipped layout
        LayoutState ls;
        ls.bounds = bounds;
        ls.cursorX = bounds.x + m_theme->panelPadding;
        ls.cursorY = bounds.y - scrollY;
        ls.direction = LayoutDir::Vertical;
        m_cursorStack.push_back(ls);
        m_clipStack.push_back(bounds);

        // Draw scrollbar if content overflows
        if (contentHeight > bounds.h) {
            float trackH = bounds.h;
            float thumbH = std::max(20.f, trackH * (bounds.h / contentHeight));
            float thumbY = bounds.y + (scrollY / maxScroll) * (trackH - thumbH);
            float scrollbarX = bounds.x + bounds.w - 8.f;

            m_renderer->drawRect({scrollbarX, bounds.y, 8.f, trackH}, m_theme->scrollbarTrack);
            bool sbHovered = hitTest({scrollbarX, thumbY, 8.f, thumbH});
            m_renderer->drawRect({scrollbarX, thumbY, 8.f, thumbH},
                                 sbHovered ? m_theme->scrollbarThumbHover : m_theme->scrollbarThumb);
        }
    }

    void endScrollArea() {
        if (!m_cursorStack.empty()) m_cursorStack.pop_back();
        if (!m_clipStack.empty()) m_clipStack.pop_back();
    }

    // ── Absolute-position hit regions ────────────────────────────
    // hitRegion tests |r| (in client pixels) against the current mouse state.
    // Does NOT require a layout cursor — call any time between begin()/end().
    // When hovered and |drawHover| is true, a subtle highlight is drawn over |r|.
    // Returns true on left-click-release (i.e. when the user clicks inside |r|).

    bool hitRegion(const Rect& r, bool drawHover = true) {
        if (!m_renderer) return false;
        bool hovered = hitTest(r);
        bool clicked = hovered && m_mouse.leftReleased;
        if (hovered && drawHover)
            m_renderer->drawRect(r, m_theme->hoverHighlight);
        return clicked;
    }

    // ── Tooltip ──────────────────────────────────────────────────

    void tooltip(std::string_view text) {
        m_tooltipText = std::string(text);
    }

    // ── State queries ────────────────────────────────────────────

    [[nodiscard]] const UIMouseState& mouseState() const { return m_mouse; }
    [[nodiscard]] float deltaTime() const { return m_dt; }

private:
    // ── Hit testing ──────────────────────────────────────────────

    [[nodiscard]] bool hitTest(const Rect& r) const {
        // Check clip stack
        if (!m_clipStack.empty()) {
            const Rect& clip = m_clipStack.back();
            if (m_mouse.x < clip.x || m_mouse.x > clip.x + clip.w ||
                m_mouse.y < clip.y || m_mouse.y > clip.y + clip.h) {
                return false;
            }
        }
        return m_mouse.x >= r.x && m_mouse.x <= r.x + r.w &&
               m_mouse.y >= r.y && m_mouse.y <= r.y + r.h;
    }

    // ── Layout cursor management ─────────────────────────────────

    enum class LayoutDir { Vertical, Horizontal };

    struct LayoutState {
        Rect bounds;
        float cursorX = 0.f;
        float cursorY = 0.f;
        LayoutDir direction = LayoutDir::Vertical;
        float maxItemHeight = 0.f;
    };

    LayoutState& currentLayout() { return m_cursorStack.back(); }

    void advanceCursor(float itemW, float itemH) {
        if (m_cursorStack.empty()) return;
        auto& ls = currentLayout();
        if (ls.direction == LayoutDir::Vertical) {
            ls.cursorY += itemH + m_theme->itemSpacing;
        } else {
            ls.cursorX += itemW + m_theme->itemSpacing;
            ls.maxItemHeight = std::max(ls.maxItemHeight, itemH);
        }
    }

    // ── State ────────────────────────────────────────────────────

    UIRenderer*  m_renderer = nullptr;
    UIMouseState m_mouse;
    const UITheme* m_theme = nullptr;
    float m_dt = 0.f;

    std::vector<LayoutState> m_cursorStack;
    std::vector<Rect> m_clipStack;
    std::string m_tooltipText;

    // Scroll state per scroll-area (persists across frames)
    std::unordered_map<uint32_t, float> m_scrollStates;
};

} // namespace NF
