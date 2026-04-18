#pragma once
#include "AtlasUI.h"
#include <string>

namespace atlas::ui::atlas {

// ============================================================
// Standard widget set for the Atlas UI framework.
// All functions follow the immediate-mode pattern: call each
// frame, return value indicates interaction (e.g. button click).
// ============================================================

/// Simple text label.
void Label(AtlasUIContext& ctx, const std::string& text);

/// Clickable button.  Returns true on the frame the button is clicked.
bool Button(AtlasUIContext& ctx, const std::string& label, float width = 0);

/// Checkbox toggle.  Modifies `value` in-place.  Returns true when toggled.
bool Checkbox(AtlasUIContext& ctx, const std::string& label, bool& value);

/// Horizontal slider.  Modifies `value` (clamped to [min,max]).
/// Returns true while the slider is being dragged.
bool Slider(AtlasUIContext& ctx, const std::string& label,
            float& value, float min = 0.0f, float max = 1.0f,
            float width = 200.0f);

/// Single-line text input field.  Modifies `text` in-place.
/// Returns true when Enter is pressed.
bool TextInput(AtlasUIContext& ctx, const std::string& label,
               std::string& text, float width = 200.0f);

/// Combo box (dropdown selector).  `selectedIndex` is modified when
/// the user picks an item.  Returns true on selection change.
bool ComboBox(AtlasUIContext& ctx, const std::string& label,
              const std::vector<std::string>& items,
              int& selectedIndex, float width = 200.0f);

/// Horizontal separator line.
void Separator(AtlasUIContext& ctx, float width = 0);

/// Spacing (advances layout cursor without drawing).
void Spacing(AtlasUIContext& ctx, float height = 0);

// ---- Sci-fi widgets ----

/// Status arc indicator (partial circle from startAngle to endAngle,
/// fraction 0..1 controls how much is filled).
void StatusArc(AtlasUIContext& ctx, float fraction,
               float radius = 30.0f, float startAngle = 0.0f,
               float endAngle = 270.0f);

/// Capacitor ring display (segmented ring showing charge level).
void CapacitorRing(AtlasUIContext& ctx, float charge,
                   int segments = 8, float radius = 24.0f);

/// Compact status indicator (coloured dot with optional label).
void StatusIndicator(AtlasUIContext& ctx, const std::string& label,
                     const AtlasColor& color);

} // namespace atlas::ui::atlas
