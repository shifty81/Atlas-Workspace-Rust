# Deferred to Workspace

Systems that are conceptually defined but not yet executing at runtime.

## Deferred Systems

| System | Status | Notes |
|--------|--------|-------|
| D3D11 Backend | Stub complete | `D3D11Backend.h` — HLSL shaders, COM structure defined |
| DirectWrite Text | Stub complete | `DirectWriteTextBackend.h` — IDWriteFactory hierarchy defined |
| Tile Authoring Tool | Spec only | Archived as future tool concept |
| Full Asset Pipeline | Partial | AssetCatalog exists; runtime population on project load pending |
| Workspace Docking | Deferred | LayoutManagerV1 is placeholder; full docking system planned |
| Settings Persistence | Partial | SettingsStore defined; save/restore on exit/startup not wired |
| Layout Persistence | Partial | LayoutPersistence defined; auto-save not wired |

## Policy

Do not expand deferred systems without stabilizing the active runtime path first.
See `Docs/Canon/01_LOCKED_DIRECTION.md` for locked decisions.
