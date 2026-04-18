# UI Backend Strategy

## Architecture Layers

| Layer | Responsibility |
|-------|---------------|
| **AtlasUI** | Layout, widgets, input, theming |
| **Backend** | Rendering primitives, text rendering, texture/icon rendering |
| **Backend Selector** | Runtime selection of rendering backend |

## Backend Tiers

### Primary Target

| Backend | Role |
|---------|------|
| **D3D11** | Primary UI rendering backend |
| **DirectWrite** | Primary text rendering backend |

### Fallback

| Backend | Role |
|---------|------|
| **GDI** | Fallback/bootstrap renderer (Windows only) |

### Compatibility

| Backend | Role |
|---------|------|
| **OpenGL** | Optional compatibility backend, not canonical |
| **GLFW** | Optional window provider, not canonical |

## Backend Interface Split

The backend contract should separate:

1. **Primitive rendering** — quads, lines, rectangles
2. **Text rendering** — glyph layout, font atlas, string measurement
3. **Texture/icon rendering** — image loading, atlas management
4. **Layout/input** — handled by AtlasUI, not backend

## Rules

- Backend must be selectable at startup (not hardcoded in main.cpp)
- Text rendering is separated from draw backend
- GDI must not be presented as "primary" or "default" in any header or doc
- D3D11 + DirectWrite are the intended long-term primary pair
- OpenGL/GLFW paths are retained only for cross-platform compatibility
- No new backend-specific code should assume GDI is the active path

## Current State

- GDI backend: **implemented**, used in active workspace path
- OpenGL backend: **implemented**, isolated in `Compat/` subdirectory
- D3D11 backend: **architecturally complete stub** — HLSL shaders, COM handle structure, IFrameBackend+IGeometryBackend+ITextureBackend (see `D3D11Backend.h`)
- DirectWrite backend: **architecturally complete stub** — IDWriteFactory hierarchy, glyph atlas strategy, ITextRenderBackend with FontKey cache (see `DirectWriteTextBackend.h`)
- Backend selector: **implemented** — UIBackendSelector.h with priority chain (D3D11→GDI→Null) and BackendCapabilities query
- Backend interface split: **done** — IUIBackendInterfaces.h formalizes IFrameBackend, IGeometryBackend, ITextRenderBackend, ITextureBackend
