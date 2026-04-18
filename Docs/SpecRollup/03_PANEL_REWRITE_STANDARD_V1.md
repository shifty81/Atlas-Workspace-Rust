# Panel Rewrite Standard v1

## Required rewrite target
Rewrite every development-facing panel against a custom retained-mode AtlasUI panel/widget framework on Win32, rendered through an abstract backend with GPU as the main path and GDI as fallback only.

## Required panel traits
- explicit layout
- backend-agnostic paint
- shared input routing
- shell-managed docking/floating
- stable panel identity
- serializable state
- theme-token-driven visuals

## Required interfaces
Recommended contract families:
- `IWidget`
- `IPanel`
- `ILayoutContext`
- `IPaintContext`
- `IInputContext`
- `IWindowHost`
- `IRenderBackend`

## Prohibited panel behavior
Do not allow canonical panels to:
- draw directly with raw GDI
- draw directly with raw OpenGL
- depend on ImGui
- own native windows directly
- embed backend-specific logic in panel code
- hardcode visual constants extensively

## Lifecycle
Panels should follow:
1. construction
2. initialize
3. activate
4. layout pass
5. paint pass
6. input handling
7. deactivate
8. destroy

## Panel categories
- DockPanel
- DocumentPanel
- ToolPanel
- PopupPanel
- OverlayPanel

## Rewrite acceptance
A panel rewrite is only complete when:
- no legacy UI framework dependency remains
- paint goes through abstract paint context
- docking/floating capability is host-managed
- panel state is persistable
- theme tokens are used
- no raw backend drawing calls exist in panel logic
