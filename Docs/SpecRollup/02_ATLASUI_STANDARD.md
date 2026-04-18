# AtlasUI Standard

## Canonical rule
AtlasUI is the mandatory UI standard for Atlas Workspace, NovaForge tooling, and all development-facing interfaces.

All tooling surfaces must use the shared AtlasUI:
- widget kit
- docking model
- floating host model
- chrome renderer
- tooltip system
- theming tokens
- command system
- interaction rules

## Rendering model
- retained-mode UI
- explicit layout pass
- explicit paint pass
- explicit input handling
- backend abstraction for rendering
- panels must not draw directly with raw backend APIs

## Backend standard
- primary: GPU-backed tooling renderer
- fallback: GDI
- same widget/panel tree through shared interfaces

## Windowing standard
- Win32 native host layer
- shell owns windows, docking, floating, focus, and message routing
- panels do not own native windows directly

## Widget standard
All panels should be built from shared widgets instead of bespoke controls.
Minimum expected widget kit includes:
- label
- button
- icon
- text field
- checkbox
- dropdown
- splitter
- tabs
- scroll container
- list/tree/table
- property grid
- graph canvas host
- viewport host
- menu controls
- notification items

## Visual standard
Use uniform development-facing chrome across workspace and tools:
- slight rounded panel corners
- slight rounded button corners
- hover highlight
- pressed/depress action behavior
- subtle transitions
- shared border and spacing scale
- consistent tab chrome
- tooltip styling aligned with the rest of AtlasUI

## Performance rule
Rounded corners, hover states, and pressed states are acceptable and should not be treated as a blocker.
Implement them once at framework level and keep them lightweight.

## Tooltip rule
All interactive AtlasUI controls should support standardized hover tooltips via a shared tooltip service.
Tooltips should be concise and theme-consistent.

## Surface types
Recommended surface roles:
- shell
- document panel
- dock panel
- tool panel
- popup
- overlay
- modal
- tray/notification
- AI/chat surface
- viewport
- graph canvas
