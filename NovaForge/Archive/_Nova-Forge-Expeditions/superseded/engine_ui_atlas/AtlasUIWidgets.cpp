#include "AtlasUIWidgets.h"
#include <algorithm>
#include <cmath>

namespace atlas::ui::atlas {

// ---- Helpers ----

static float DefaultWidth(const AtlasUIContext& ctx, float requested) {
    if (requested > 0) return requested;
    return ctx.ViewWidth() - 2.0f * static_cast<float>(ctx.GetTheme().paddingMedium);
}

static float LineH(const AtlasUIContext& ctx) {
    return static_cast<float>(ctx.GetTheme().lineHeight);
}

// ---- Label ----

void Label(AtlasUIContext& ctx, const std::string& text) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx);
    float w = DefaultWidth(ctx, 0);
    AtlasRect r{ctx.CursorX(), ctx.CursorY(), w, h};
    ctx.AddText(r, text, th.textPrimary);
    ctx.AdvanceCursor(h);
}

// ---- Button ----

bool Button(AtlasUIContext& ctx, const std::string& label, float width) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx) + static_cast<float>(th.paddingSmall) * 2;
    float w = DefaultWidth(ctx, width);

    AtlasRect r{ctx.CursorX(), ctx.CursorY(), w, h};
    uint32_t id = ctx.GetID(label);

    bool hovered = r.Contains(ctx.Input().mouseX, ctx.Input().mouseY);
    bool clicked = false;

    if (hovered) {
        ctx.SetHotWidget(id);
        if (ctx.Input().mouseClicked) {
            ctx.SetActiveWidget(id);
        }
        if (ctx.Input().mouseUp && ctx.ActiveWidget() == id) {
            clicked = true;
        }
    }

    AtlasColor bg = th.buttonNormal;
    if (ctx.ActiveWidget() == id && hovered) {
        bg = th.buttonPressed;
    } else if (hovered) {
        bg = th.buttonHover;
    }

    ctx.AddFilledRect(r, bg);
    ctx.AddBorderRect(r, th.border, th.borderThickness);

    AtlasRect textR{r.x + static_cast<float>(th.paddingSmall),
                    r.y + static_cast<float>(th.paddingSmall),
                    r.w - static_cast<float>(th.paddingSmall) * 2,
                    LineH(ctx)};
    ctx.AddText(textR, label, th.textPrimary);

    ctx.AdvanceCursor(h);
    return clicked;
}

// ---- Checkbox ----

bool Checkbox(AtlasUIContext& ctx, const std::string& label, bool& value) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx);
    float boxSize = h;
    float totalW = DefaultWidth(ctx, 0);

    AtlasRect boxR{ctx.CursorX(), ctx.CursorY(), boxSize, boxSize};
    uint32_t id = ctx.GetID(label);

    bool hovered = boxR.Contains(ctx.Input().mouseX, ctx.Input().mouseY);
    bool toggled = false;

    if (hovered) {
        ctx.SetHotWidget(id);
        if (ctx.Input().mouseClicked) {
            value = !value;
            toggled = true;
        }
    }

    ctx.AddFilledRect(boxR, hovered ? th.buttonHover : th.buttonNormal);
    ctx.AddBorderRect(boxR, th.border, th.borderThickness);
    if (value) {
        float inset = 3.0f;
        AtlasRect checkR{boxR.x + inset, boxR.y + inset,
                         boxR.w - inset * 2, boxR.h - inset * 2};
        ctx.AddFilledRect(checkR, th.accent);
    }

    AtlasRect labelR{ctx.CursorX() + boxSize + static_cast<float>(th.paddingSmall),
                     ctx.CursorY(), totalW - boxSize, h};
    ctx.AddText(labelR, label, th.textPrimary);

    ctx.AdvanceCursor(h);
    return toggled;
}

// ---- Slider ----

bool Slider(AtlasUIContext& ctx, const std::string& label,
            float& value, float min, float max, float width) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx);
    float w = DefaultWidth(ctx, width);

    // Label row
    AtlasRect labelR{ctx.CursorX(), ctx.CursorY(), w, h};
    ctx.AddText(labelR, label, th.textSecondary);
    ctx.AdvanceCursor(h);

    // Track
    float trackH = 6.0f;
    float trackY = ctx.CursorY() + (h - trackH) * 0.5f;
    AtlasRect trackR{ctx.CursorX(), trackY, w, trackH};

    uint32_t id = ctx.GetID(label);
    bool dragging = false;

    // Hit test the full slider row
    AtlasRect hitR{ctx.CursorX(), ctx.CursorY(), w, h};
    bool hovered = hitR.Contains(ctx.Input().mouseX, ctx.Input().mouseY);

    if (hovered && ctx.Input().mouseClicked) {
        ctx.SetActiveWidget(id);
    }

    if (ctx.ActiveWidget() == id && ctx.Input().mouseDown) {
        float t = (trackR.w > 0) ? (ctx.Input().mouseX - trackR.x) / trackR.w : 0.0f;
        t = std::max(0.0f, std::min(1.0f, t));
        value = min + t * (max - min);
        dragging = true;
    }

    // Clamp value
    value = std::max(min, std::min(max, value));

    // Draw track background
    ctx.AddFilledRect(trackR, th.buttonNormal);
    ctx.AddBorderRect(trackR, th.border, th.borderThickness);

    // Draw filled portion
    float frac = (max > min) ? (value - min) / (max - min) : 0.0f;
    AtlasRect filledR{trackR.x, trackR.y, trackR.w * frac, trackR.h};
    ctx.AddFilledRect(filledR, th.accent);

    // Draw handle
    float handleW = 10.0f;
    float handleX = trackR.x + frac * trackR.w - handleW * 0.5f;
    AtlasRect handleR{handleX, ctx.CursorY(), handleW, h};
    ctx.AddFilledRect(handleR, hovered ? th.accentHover : th.accent);

    ctx.AdvanceCursor(h);
    return dragging;
}

// ---- TextInput ----

bool TextInput(AtlasUIContext& ctx, const std::string& label,
               std::string& text, float width) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx) + static_cast<float>(th.paddingSmall) * 2;
    float w = DefaultWidth(ctx, width);

    // Label
    AtlasRect labelR{ctx.CursorX(), ctx.CursorY(), w, LineH(ctx)};
    ctx.AddText(labelR, label, th.textSecondary);
    ctx.AdvanceCursor(LineH(ctx));

    // Input box
    AtlasRect boxR{ctx.CursorX(), ctx.CursorY(), w, h};
    uint32_t id = ctx.GetID(label);

    bool hovered = boxR.Contains(ctx.Input().mouseX, ctx.Input().mouseY);
    if (hovered && ctx.Input().mouseClicked) {
        ctx.SetActiveWidget(id);
    }

    bool focused = (ctx.ActiveWidget() == id);
    bool submitted = false;

    if (focused) {
        // Append typed text
        if (!ctx.Input().textInput.empty()) {
            text += ctx.Input().textInput;
        }
        if (ctx.Input().backspace && !text.empty()) {
            text.pop_back();
        }
        if (ctx.Input().enterKey) {
            submitted = true;
        }
    }

    // Draw
    ctx.AddFilledRect(boxR, th.panelBackground);
    ctx.AddBorderRect(boxR, focused ? th.borderFocused : th.border, th.borderThickness);

    AtlasRect textR{boxR.x + static_cast<float>(th.paddingSmall),
                    boxR.y + static_cast<float>(th.paddingSmall),
                    boxR.w - static_cast<float>(th.paddingSmall) * 2,
                    LineH(ctx)};
    ctx.AddText(textR, text, th.textPrimary);

    ctx.AdvanceCursor(h);
    return submitted;
}

// ---- ComboBox ----

bool ComboBox(AtlasUIContext& ctx, const std::string& label,
              const std::vector<std::string>& items,
              int& selectedIndex, float width) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx) + static_cast<float>(th.paddingSmall) * 2;
    float w = DefaultWidth(ctx, width);

    // Label
    AtlasRect labelR{ctx.CursorX(), ctx.CursorY(), w, LineH(ctx)};
    ctx.AddText(labelR, label, th.textSecondary);
    ctx.AdvanceCursor(LineH(ctx));

    // Combo header
    AtlasRect boxR{ctx.CursorX(), ctx.CursorY(), w, h};
    uint32_t id = ctx.GetID(label);

    bool hovered = boxR.Contains(ctx.Input().mouseX, ctx.Input().mouseY);
    bool changed = false;

    if (hovered && ctx.Input().mouseClicked) {
        // Simple cycling behaviour for immediate-mode: each click
        // advances to the next item.
        if (!items.empty()) {
            selectedIndex = (selectedIndex + 1) % static_cast<int>(items.size());
            changed = true;
        }
    }

    ctx.AddFilledRect(boxR, hovered ? th.buttonHover : th.buttonNormal);
    ctx.AddBorderRect(boxR, th.border, th.borderThickness);

    std::string display = (selectedIndex >= 0 &&
                           selectedIndex < static_cast<int>(items.size()))
                          ? items[static_cast<size_t>(selectedIndex)]
                          : "";
    AtlasRect textR{boxR.x + static_cast<float>(th.paddingSmall),
                    boxR.y + static_cast<float>(th.paddingSmall),
                    boxR.w - static_cast<float>(th.paddingSmall) * 2,
                    LineH(ctx)};
    ctx.AddText(textR, display, th.textPrimary);

    ctx.AdvanceCursor(h);
    return changed;
}

// ---- Separator ----

void Separator(AtlasUIContext& ctx, float width) {
    const auto& th = ctx.GetTheme();
    float w = DefaultWidth(ctx, width);
    AtlasRect r{ctx.CursorX(), ctx.CursorY(), w, 1.0f};
    ctx.AddFilledRect(r, th.border);
    ctx.AdvanceCursor(static_cast<float>(th.itemSpacing));
}

// ---- Spacing ----

void Spacing(AtlasUIContext& ctx, float height) {
    if (height <= 0) height = static_cast<float>(ctx.GetTheme().sectionGap);
    ctx.AdvanceCursor(height);
}

// ---- StatusArc ----

void StatusArc(AtlasUIContext& ctx, float fraction,
               float radius, float /*startAngle*/, float /*endAngle*/) {
    const auto& th = ctx.GetTheme();
    fraction = std::max(0.0f, std::min(1.0f, fraction));

    float size = radius * 2.0f;
    AtlasRect outer{ctx.CursorX(), ctx.CursorY(), size, size};

    // Background ring
    ctx.AddFilledRect(outer, th.capacitorEmpty);

    // Filled portion (approximate as a smaller rect scaled by fraction)
    AtlasRect filled{outer.x, outer.y, outer.w * fraction, outer.h};
    ctx.AddFilledRect(filled, th.arcColor);

    ctx.AdvanceCursor(size);
}

// ---- CapacitorRing ----

void CapacitorRing(AtlasUIContext& ctx, float charge,
                   int segments, float radius) {
    const auto& th = ctx.GetTheme();
    charge = std::max(0.0f, std::min(1.0f, charge));
    segments = std::max(1, segments);

    float size = radius * 2.0f;
    float segW = size / static_cast<float>(segments);
    int filledSegs = static_cast<int>(charge * static_cast<float>(segments) + 0.5f);

    for (int i = 0; i < segments; ++i) {
        AtlasRect seg{ctx.CursorX() + static_cast<float>(i) * segW,
                      ctx.CursorY(),
                      segW - 2.0f, size * 0.3f};
        ctx.AddFilledRect(seg, (i < filledSegs) ? th.capacitorFull : th.capacitorEmpty);
    }

    ctx.AdvanceCursor(size * 0.3f);
}

// ---- StatusIndicator ----

void StatusIndicator(AtlasUIContext& ctx, const std::string& label,
                     const AtlasColor& color) {
    const auto& th = ctx.GetTheme();
    float h = LineH(ctx);
    float dotSize = h * 0.5f;

    AtlasRect dotR{ctx.CursorX(), ctx.CursorY() + (h - dotSize) * 0.5f,
                   dotSize, dotSize};
    ctx.AddFilledRect(dotR, color);

    AtlasRect textR{ctx.CursorX() + dotSize + static_cast<float>(th.paddingSmall),
                    ctx.CursorY(), 200.0f, h};
    ctx.AddText(textR, label, th.textPrimary);

    ctx.AdvanceCursor(h);
}

} // namespace atlas::ui::atlas
