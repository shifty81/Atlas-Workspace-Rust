#pragma once
#include <cstdint>

namespace atlas::ui {

/// Maximum widget ID to check when iterating.  This is a reasonable upper
/// bound for editor UI.  All managers share this constant to stay in sync.
static constexpr uint32_t kMaxWidgetId = 1000;

/// Bitmap font glyph dimensions and scale factor.
/// GLRenderer uses a 5×7 bitmap font rendered at this scale.
/// Other UI code (cursor positioning, line spacing) must use these
/// values to stay in sync with the renderer.
static constexpr int kFontGlyphWidth  = 5;
static constexpr int kFontGlyphHeight = 7;
static constexpr int kFontScale       = 2;
static constexpr int kFontCharAdvance = (kFontGlyphWidth + 1) * kFontScale;  // 12px
static constexpr int kFontLineHeight  = (kFontGlyphHeight + 2) * kFontScale; // 18px

/// Spacing between log/text entries in scroll views.
static constexpr float kLogLineSpacing = 20.0f;

/// Height of a single log/text entry in scroll views.
static constexpr float kLogEntryHeight = 18.0f;

} // namespace atlas::ui
