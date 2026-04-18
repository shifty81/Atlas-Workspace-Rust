#pragma once
#include <cstdint>
#include <string>

namespace NF::UI::AtlasUI {

struct Rect {
    float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
};

struct Vec2 {
    float x = 0.f, y = 0.f;
};

enum class WidgetState : uint32_t {
    None      = 0,
    Hovered   = 1 << 0,
    Pressed   = 1 << 1,
    Focused   = 1 << 2,
    Disabled  = 1 << 3,
    Selected  = 1 << 4,
    Active    = 1 << 5,
    Expanded  = 1 << 6,
    Error     = 1 << 7
};

inline WidgetState operator|(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline WidgetState operator&(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline bool hasState(WidgetState s, WidgetState flag) {
    return (static_cast<uint32_t>(s) & static_cast<uint32_t>(flag)) != 0;
}

enum class SizeClass { Small, Medium, Large };

} // namespace NF::UI::AtlasUI
