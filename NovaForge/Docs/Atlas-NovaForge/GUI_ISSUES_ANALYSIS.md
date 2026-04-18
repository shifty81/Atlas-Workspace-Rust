# GUI Issues Analysis

## Summary

The file `gui_issues.txt` is a ChatGPT conversation log committed in
`1265ce7`.  **It does not cause GUI issues** — it is a text file with no
executable content.  However, it accurately **diagnoses** real rendering
architecture gaps in the AtlasForge editor.

## Root Cause Identified in gui_issues.txt

The conversation pinpoints a single root cause for the editor viewport
appearing blank while the game scene renders behind the UI:

> **No scene framebuffer → viewport binding.**
>
> The scene and editor UI were both rendering directly to the swapchain
> backbuffer in the same pass, so the game world appeared fullscreen
> *behind* the editor panels instead of *inside* the viewport panel.

This matched the actual code in `Engine::RunEditor()`, which had a single
render pass where `UIManager::Render()` drew the UI directly to the
backbuffer with no prior scene-to-framebuffer step.

## Changes Made

| File | Change |
|------|--------|
| `engine/render/EditorViewportFramebuffer.h` | New — abstract framebuffer interface + `NullViewportFramebuffer` for headless builds |
| `engine/core/Engine.h` | Added `m_viewportFB` member and `GetViewportFramebuffer()` accessor |
| `engine/core/Engine.cpp` | `InitEditor()` creates the viewport framebuffer; `RunEditor()` splits rendering into two passes (scene → framebuffer, UI → swapchain); `ProcessWindowEvents()` resizes the framebuffer on window resize |
| `tests/test_viewport_framebuffer.cpp` | New — tests for `NullViewportFramebuffer` and engine integration |
| `tests/CMakeLists.txt` | Added `test_viewport_framebuffer.cpp` |
| `tests/main.cpp` | Registered new test functions |

## Remaining Work

The `NullViewportFramebuffer` is a no-op placeholder.  A real GPU-backed
implementation (OpenGL FBO / Vulkan `VkFramebuffer`) is needed to actually
display the scene inside the viewport panel.  The architectural split is
now in place so that adding a GPU backend is a localised change to the
`EditorViewportFramebuffer` subclass — no further modifications to the
engine render loop are required.
