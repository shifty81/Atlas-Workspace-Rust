> **HISTORICAL** — This document describes a completed foundation phase (Phases 0–71).
> The canonical source of truth for current and future phases is [00_MASTER_ROADMAP.md](00_MASTER_ROADMAP.md).

# AtlasUI and Render Backends

**Phase 2 of the Master Roadmap — Status: Done**

## Purpose

Correct the backend strategy so GDI is fallback-only and D3D11/DirectWrite become the primary target path.

## Tasks

### Backend Selector
- [x] Create `UIBackendSelector.h` — `UIBackendType` enum, `selectUIBackend()`, `queryCapabilities()`
- [x] Define priority chain: D3D11 → GDI → Null (with fallback logic)
- [x] `BackendCapabilities` struct for feature queries

### Backend Interface Split
- [x] `IUIBackendInterfaces.h` — formal split into 4 sub-interfaces:
  - `IFrameBackend` (lifecycle + frame boundaries)
  - `IGeometryBackend` (batched quad submission + scissor clip)
  - `ITextRenderBackend` (font load + draw + measure)
  - `ITextureBackend` (upload + bind + destroy)
- [x] `NullTextRenderBackend` — headless text backend for testing
- [x] `NullTextureBackend` — headless texture backend for testing

### D3D11 Backend
- [x] `D3D11Backend.h` implements `IFrameBackend + IGeometryBackend + ITextureBackend`
- [x] Embedded HLSL shaders (`kVS_Source`, `kPS_Source`) in `D3D11Shaders` namespace
- [x] Full COM resource handle structure (device, context, swap chain, RTV, VB, IB, CB, VS, PS, layout, blend, sampler, rasteriser)
- [x] Text delegation via `setTextBackend(ITextRenderBackend*)`
- [x] Diagnostics: `lastVertexCount()`, `lastIndexCount()`, `scissorActive()`
- [x] Architecturally complete — real COM calls stubbed with detailed comments

### DirectWrite Text Backend
- [x] `DirectWriteTextBackend.h` implements `ITextRenderBackend`
- [x] `IDWriteFactory` hierarchy documented (factory → font collection → text format → text layout)
- [x] Glyph atlas strategy documented (ASCII pre-rasterisation → `ITextureBackend::uploadTexture`)
- [x] `FontKey` cache (`{family, size}` → `IDWriteTextFormat*`)
- [x] `loadFont()`, `drawText()`, `measureTextWidth()`, `lineHeight()` with fallback approximations
- [x] Architecturally complete — real COM calls stubbed with detailed comments

### GDI Isolation
- [x] Marked fallback-only in headers and docs
- [x] Not implied as "primary" or "default"
- [x] Kept functional for bootstrap/recovery scenarios

### OpenGL / GLFW Isolation
- [x] `Source/UI/include/NF/UI/Compat/` subdirectory created
- [x] `Compat/CompatBackends.h` bundles OpenGL + GLFW headers with ⚠️ warning
- [x] `OpenGLBackend.h` marked COMPAT ONLY, now also implements `IFrameBackend + IGeometryBackend`
- [x] `GLFWWindowProvider.h` marked COMPAT ONLY
- [x] `GLFWInputAdapter.h` marked COMPAT ONLY

### Tests
- [x] `Tests/UI/test_ui_backends.cpp` — 30+ contract tests:
  - `IFrameBackend` contract via `NullBackend`
  - `IGeometryBackend` contract
  - `ITextRenderBackend` contract via `NullTextRenderBackend`
  - `ITextureBackend` contract via `NullTextureBackend`
  - `UIBackendSelector` priority chain + `queryCapabilities`
  - `D3D11Backend` smoke tests (Windows only, guarded with `#ifdef _WIN32`)
  - `DirectWriteTextBackend` smoke tests (Windows only)
