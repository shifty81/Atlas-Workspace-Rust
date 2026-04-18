#pragma once
#include "../../engine/ui/UIRenderer.h"
#include <string>
#include <cstdint>

namespace atlas::editor {

/// Editor theme configuration providing dark-themed, professional aesthetics.
/// All values are integers to ensure deterministic layout across platforms.
struct EditorTheme {
    // --- Base Colors ---
    atlas::ui::UIColor background        = {30,  31,  34,  255};  // Main background
    atlas::ui::UIColor panelBackground   = {43,  43,  43,  255};  // Panel body
    atlas::ui::UIColor panelHeader       = {60,  63,  65,  255};  // Panel header bar
    atlas::ui::UIColor panelBorder       = {70,  73,  75,  255};  // Panel border
    atlas::ui::UIColor separator         = {50,  50,  50,  255};  // Dividers

    // --- Text Colors ---
    atlas::ui::UIColor textPrimary       = {220, 220, 220, 255};  // Primary text
    atlas::ui::UIColor textSecondary     = {160, 160, 160, 255};  // Secondary / dimmed text
    atlas::ui::UIColor textDisabled      = {100, 100, 100, 255};  // Disabled text
    atlas::ui::UIColor textHeader        = {230, 230, 230, 255};  // Panel header text

    // --- Accent Colors ---
    atlas::ui::UIColor accentPrimary     = {66,  135, 245, 255};  // Selection, focus
    atlas::ui::UIColor accentHover       = {82,  150, 255, 255};  // Hover state
    atlas::ui::UIColor accentActive      = {50,  120, 220, 255};  // Active / pressed

    // --- Status Colors ---
    atlas::ui::UIColor statusError       = {230, 80,  80,  255};  // Errors
    atlas::ui::UIColor statusWarning     = {230, 180, 60,  255};  // Warnings
    atlas::ui::UIColor statusSuccess     = {80,  200, 120, 255};  // Success indicators
    atlas::ui::UIColor statusInfo        = {80,  160, 230, 255};  // Informational

    // --- Button Colors ---
    atlas::ui::UIColor buttonNormal      = {55,  58,  62,  255};
    atlas::ui::UIColor buttonHover       = {70,  73,  78,  255};
    atlas::ui::UIColor buttonPressed     = {45,  48,  52,  255};
    atlas::ui::UIColor buttonBorder      = {80,  83,  88,  255};

    // --- Input Field Colors ---
    atlas::ui::UIColor inputBackground   = {35,  37,  40,  255};
    atlas::ui::UIColor inputBorder       = {70,  100, 150, 255};
    atlas::ui::UIColor inputBorderFocus  = {66,  135, 245, 255};

    // --- Typography (integer sizes) ---
    int32_t fontSizeHeader    = 16;
    int32_t fontSizeBody      = 13;
    int32_t fontSizeSmall     = 11;
    int32_t fontSizeMonospace  = 12;

    // --- Spacing (base unit = 4px) ---
    int32_t spacingUnit       = 4;   // Base spacing unit
    int32_t paddingSmall      = 4;   // 1x unit
    int32_t paddingMedium     = 8;   // 2x unit
    int32_t paddingLarge      = 16;  // 4x unit
    int32_t panelHeaderHeight = 28;  // Panel header bar height
    int32_t borderWidth       = 1;   // Default border thickness
    int32_t scrollbarWidth    = 12;  // Scrollbar track width

    // --- Icon Settings ---
    int32_t iconSizeSmall     = 16;
    int32_t iconSizeMedium    = 24;
    int32_t iconSizeLarge     = 32;

    /// Return the default dark theme.
    static EditorTheme Dark() { return EditorTheme{}; }
};

} // namespace atlas::editor
