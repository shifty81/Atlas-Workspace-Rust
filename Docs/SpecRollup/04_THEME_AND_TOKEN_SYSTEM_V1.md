# Theme + Token System v1

## Purpose
Single theming and token source for all AtlasUI development-facing surfaces.

## Required token families
- colors
- spacing
- radii
- typography
- motion
- sizes
- borders
- layers

## Core rule
No development-facing panel or widget should hardcode colors, radii, spacing, animation timings, or control heights when a token should exist.

## Canonical baseline theme
Start with one canonical theme:
- Atlas Dark

Prepare for later variants:
- Atlas Dark High Contrast
- Atlas Light

## Required semantic token examples

### Colors
- WindowBg
- PanelBg
- Surface
- HeaderBg
- Border
- TextPrimary
- TextSecondary
- ButtonBg
- ButtonHover
- ButtonPressed
- TabBg
- TabHover
- TabActive
- TooltipBg
- Accent
- Success
- Warning
- Error
- Info

### Spacing
- XXS
- XS
- SM
- MD
- LG
- XL
- XXL
- PanelPadding
- ToolbarGap
- TooltipPadding
- RowGap

### Radii
- Panel
- Button
- Tab
- Tooltip
- Popup
- Input
- Toast

### Typography
- Caption
- Body
- BodyStrong
- Subtitle
- Title
- Mono
- Tooltip
- Tab
- Menu

### Motion
- HoverFadeMs
- PressMs
- TabActivateMs
- TooltipFadeMs
- ToastFadeMs

## Performance rule
Theme resolution should be cheap.
Do not build new brushes/fonts every paint.
Do not recompute token tables every frame.
