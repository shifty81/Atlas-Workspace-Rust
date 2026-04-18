#pragma once
// ============================================================
// Atlas Include Firewall — Compile-Time Namespace Enforcement
// ============================================================
//
// Prevents forbidden cross-layer includes at compile time.
// Include this header in simulation code to enforce that
// presentation, editor, and IO headers are never pulled in.
//
// Layer rules:
//   core/       → No dependencies (C++ stdlib only)
//   simulation/ → core/ only
//   editor/     → Never includes sim internals directly
//
// Usage:
//   #define ATLAS_LAYER_SIMULATION
//   #include "engine/core/contract/IncludeFirewall.h"
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ARCHITECTURE.md (Dependency Rules)
//      docs/NEXT_IMPLEMENTATION_TASKS.md §4

// ---- Simulation Layer Guards ----
//
// When ATLAS_LAYER_SIMULATION is defined, the following headers
// are forbidden. Any include will trigger a compile error.

#if defined(ATLAS_LAYER_SIMULATION)

  // Prevent OpenGL/Vulkan includes in simulation code
  #if defined(GL_VERSION_1_0) || defined(__gl_h_) || defined(__GL_H__)
    #error "OpenGL headers included in simulation code — violates Atlas layer rules"
  #endif

  #if defined(VULKAN_CORE_H_) || defined(VK_VERSION_1_0)
    #error "Vulkan headers included in simulation code — violates Atlas layer rules"
  #endif

  // Prevent platform window includes
  #if defined(_WINDOWS_) || defined(_INC_WINDOWS)
    #error "Windows.h included in simulation code — violates Atlas layer rules"
  #endif

  // Prevent editor-only includes
  #if defined(ATLAS_EDITOR_PANEL_H)
    #error "Editor panel headers included in simulation code — violates Atlas layer rules"
  #endif

  // Prevent direct renderer includes
  #if defined(ATLAS_GL_RENDERER_H) || defined(ATLAS_VULKAN_RENDERER_H)
    #error "Renderer headers included in simulation code — violates Atlas layer rules"
  #endif

#endif  // ATLAS_LAYER_SIMULATION

// ---- Core Layer Guards ----
//
// When ATLAS_LAYER_CORE is defined, even stricter rules apply.
// Core code cannot depend on anything outside of core/.

#if defined(ATLAS_LAYER_CORE)

  // Everything forbidden in simulation is also forbidden in core
  #if defined(GL_VERSION_1_0) || defined(__gl_h_) || defined(__GL_H__)
    #error "OpenGL headers included in core code — violates Atlas layer rules"
  #endif

  #if defined(VULKAN_CORE_H_) || defined(VK_VERSION_1_0)
    #error "Vulkan headers included in core code — violates Atlas layer rules"
  #endif

  #if defined(_WINDOWS_) || defined(_INC_WINDOWS)
    #error "Windows.h included in core code — violates Atlas layer rules"
  #endif

  // Core cannot include simulation internals
  #if defined(ATLAS_TICK_SCHEDULER_H) || defined(ATLAS_REPLAY_RECORDER_H)
    #error "Simulation headers included in core code — violates Atlas layer rules"
  #endif

#endif  // ATLAS_LAYER_CORE

// ---- Firewall Validation Marker ----
//
// Set this so tests can verify the firewall was included.
#define ATLAS_INCLUDE_FIREWALL_ACTIVE 1
