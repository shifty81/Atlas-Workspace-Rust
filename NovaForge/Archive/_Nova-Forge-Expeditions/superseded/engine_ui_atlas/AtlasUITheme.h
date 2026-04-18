#pragma once
#include <cstdint>
#include <string>

namespace atlas::ui::atlas {

/// RGBA colour used throughout the Atlas UI framework.
struct AtlasColor {
    uint8_t r = 255, g = 255, b = 255, a = 255;

    bool operator==(const AtlasColor& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    bool operator!=(const AtlasColor& o) const { return !(*this == o); }
};

/// Complete theme definition controlling the visual appearance of all
/// Atlas UI widgets.  Three built-in presets are provided: Teal (sci-fi
/// default), Amber (warm alternative), and a colorblind-safe variant.
struct AtlasTheme {
    std::string name;

    // ---- Surface / background ----
    AtlasColor background       {18,  22,  28,  245};
    AtlasColor panelBackground  {24,  30,  38,  240};
    AtlasColor headerBackground {30,  38,  48,  255};

    // ---- Text ----
    AtlasColor textPrimary      {210, 220, 230, 255};
    AtlasColor textSecondary    {140, 155, 170, 255};
    AtlasColor textDisabled     {80,  90,  100, 255};

    // ---- Accent / interactive ----
    AtlasColor accent           {0,   200, 200, 255};
    AtlasColor accentHover      {40,  220, 220, 255};
    AtlasColor accentPressed    {0,   160, 160, 255};

    // ---- Button ----
    AtlasColor buttonNormal     {40,  50,  60,  255};
    AtlasColor buttonHover      {55,  68,  80,  255};
    AtlasColor buttonPressed    {30,  38,  46,  255};
    AtlasColor buttonDisabled   {35,  40,  48,  180};

    // ---- Status ----
    AtlasColor statusOk         {78,  201, 176, 255};
    AtlasColor statusWarning    {230, 180, 34,  255};
    AtlasColor statusError      {244, 71,  71,  255};

    // ---- Sci-fi decorations ----
    AtlasColor arcColor         {0,   200, 200, 120};
    AtlasColor capacitorFull    {0,   255, 200, 200};
    AtlasColor capacitorEmpty   {60,  70,  80,  120};

    // ---- Border / selection ----
    AtlasColor border           {50,  62,  75,  255};
    AtlasColor borderFocused    {0,   200, 200, 255};
    AtlasColor selectionBg      {0,   200, 200, 50};
    AtlasColor hoverBg          {255, 255, 255, 15};

    // ---- Spacing (pixels) ----
    int32_t paddingSmall   = 4;
    int32_t paddingMedium  = 8;
    int32_t paddingLarge   = 16;
    int32_t itemSpacing    = 4;
    int32_t sectionGap     = 12;
    int32_t panelTitleBar  = 28;

    // ---- Typography ----
    float   fontSizeSmall   = 11.0f;
    float   fontSizeNormal  = 13.0f;
    float   fontSizeLarge   = 17.0f;
    float   fontSizeHeading = 21.0f;
    int32_t lineHeight      = 18;

    // ---- Border / rounding ----
    int32_t borderThickness = 1;
    int32_t cornerRadius    = 3;

    // ---- Built-in presets ----

    /// Teal sci-fi theme (default).
    static AtlasTheme Teal() {
        AtlasTheme t;
        t.name = "Teal";
        // Defaults are already teal-tuned.
        return t;
    }

    /// Warm amber theme.
    static AtlasTheme Amber() {
        AtlasTheme t;
        t.name = "Amber";
        t.accent          = {220, 170, 30,  255};
        t.accentHover     = {240, 190, 50,  255};
        t.accentPressed   = {180, 140, 20,  255};
        t.arcColor        = {220, 170, 30,  120};
        t.capacitorFull   = {255, 200, 50,  200};
        t.borderFocused   = {220, 170, 30,  255};
        t.selectionBg     = {220, 170, 30,  50};
        return t;
    }

    /// Colorblind-safe theme using blue/orange distinguishable palette.
    static AtlasTheme ColorblindSafe() {
        AtlasTheme t;
        t.name = "ColorblindSafe";
        t.accent          = {86,  180, 233, 255};
        t.accentHover     = {110, 200, 245, 255};
        t.accentPressed   = {60,  150, 200, 255};
        t.statusOk        = {86,  180, 233, 255};
        t.statusWarning   = {230, 159, 0,   255};
        t.statusError     = {213, 94,  0,   255};
        t.arcColor        = {86,  180, 233, 120};
        t.capacitorFull   = {86,  180, 233, 200};
        t.borderFocused   = {86,  180, 233, 255};
        t.selectionBg     = {86,  180, 233, 50};
        return t;
    }
};

} // namespace atlas::ui::atlas
