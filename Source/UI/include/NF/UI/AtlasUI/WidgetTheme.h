#pragma once

#include "NF/UI/AtlasUI/DrawPrimitives.h"

namespace NF::UI::AtlasUI::Theme {

struct Spacing {
    static constexpr float Tiny = 4.f;
    static constexpr float Small = 8.f;
    static constexpr float Medium = 12.f;
    static constexpr float Large = 16.f;
};

struct Radius {
    static constexpr float Default = 6.f;
};

struct ColorToken {
    static constexpr Color Background = 0xFF151515;
    static constexpr Color Surface = 0xFF202020;
    static constexpr Color SurfaceAlt = 0xFF2A2A2A;
    static constexpr Color Border = 0xFF4A4A4A;
    static constexpr Color Text = 0xFFF2F2F2;
    static constexpr Color TextMuted = 0xFFB0B0B0;
    static constexpr Color Accent = 0xFF3A7AFE;
    static constexpr Color AccentHover = 0xFF5A94FF;
    static constexpr Color Pressed = 0xFF244DA3;
    static constexpr Color Selection = 0x553A7AFE;
};

} // namespace NF::UI::AtlasUI::Theme
