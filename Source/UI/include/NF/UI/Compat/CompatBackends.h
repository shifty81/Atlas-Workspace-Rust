#pragma once
// NF::UI::Compat — Compatibility / legacy backend bundle.
//
// ⚠️  WARNING: These backends are NOT the canonical render path.
//
//   OpenGLBackend      — Compatibility backend (cross-platform, non-primary)
//   GLFWWindowProvider — OpenGL window / context provider (compat only)
//   GLFWInputAdapter   — GLFW input bridging to InputSystem (compat only)
//
// Primary backend path: D3D11 + DirectWrite (Windows, GPU-first).
// Fallback path:        GDI (Windows recovery / bootstrap only).
//
// These headers are intentionally placed under Compat/ to communicate that:
//   - They are NOT included in default builds.
//   - They are NOT supported as production render targets.
//   - They exist for compatibility testing and cross-platform bring-up only.
//
// To enable compat backends in CMake:
//   -DATLAS_INCLUDE_COMPAT_BACKENDS=ON   (default: OFF)

#include "NF/UI/OpenGLBackend.h"
#include "NF/UI/GLFWWindowProvider.h"
#include "NF/UI/GLFWInputAdapter.h"
