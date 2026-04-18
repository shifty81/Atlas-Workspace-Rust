#pragma once

namespace NF::UI::AtlasUI {

struct ColorRGBA {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr ColorRGBA() = default;
    constexpr ColorRGBA(float inR, float inG, float inB, float inA = 1.0f)
        : r(inR), g(inG), b(inB), a(inA) {}
};

struct EdgeInsets {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct FontRole {
    const char* family = "Segoe UI";
    float size = 13.0f;
    int weight = 400;
    float lineHeight = 16.0f;
};

} // namespace NF::UI::AtlasUI
