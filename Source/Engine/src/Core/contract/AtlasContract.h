#pragma once
// ============================================================
// ATLAS CORE CONTRACT — Compile-Time Enforcement
// ============================================================
//
// Include this header in every simulation translation unit.
// It enforces determinism constraints at compile time.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ATLAS_DETERMINISM_ENFORCEMENT.md

#include <cstdint>
#include <cstdlib>

// ---- Build Modes ----

#ifndef ATLAS_DETERMINISM_STRICT
#define ATLAS_DETERMINISM_STRICT 0
#endif

// ---- Platform Detection ----

#if defined(_WIN32)
  #define ATLAS_PLATFORM_WINDOWS 1
#elif defined(__linux__)
  #define ATLAS_PLATFORM_LINUX 1
#elif defined(__APPLE__)
  #define ATLAS_PLATFORM_MACOS 1
#else
  #error "Unsupported platform for deterministic simulation"
#endif

// ---- Deterministic Tick ----

namespace atlas {

using AtlasTick = uint64_t;
constexpr AtlasTick ATLAS_DEFAULT_TICK_RATE = 60;

}  // namespace atlas

// ---- Static Assertions ----

static_assert(sizeof(float) == 4, "Float size mismatch — determinism at risk");
static_assert(sizeof(double) == 8, "Double size mismatch — determinism at risk");

// ---- Contract Attribute Markers ----
//
// Use these to annotate functions by domain:
//   ATLAS_SIMULATION_ONLY  — simulation code, determinism-critical
//   ATLAS_PRESENTATION_ONLY — rendering/audio, non-authoritative
//   ATLAS_EDITOR_ONLY — editor tooling, never in runtime

#define ATLAS_SIMULATION_ONLY
#define ATLAS_PRESENTATION_ONLY
#define ATLAS_EDITOR_ONLY

// ---- Cross-platform trap ----
#if defined(_MSC_VER)
  #define ATLAS_CONTRACT_TRAP() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
  #define ATLAS_CONTRACT_TRAP() __builtin_trap()
#else
  #define ATLAS_CONTRACT_TRAP() std::abort()
#endif

// ---- Simulation Boundary Guard ----
//
// ATLAS_FORBID_IN_SIM(msg) — Place at the top of any function that must
// never be called from deterministic simulation code. In strict mode
// (ATLAS_SIMULATION_BUILD defined), including any translation unit that
// uses a marked function will produce a linker or runtime error.

#if ATLAS_DETERMINISM_STRICT && defined(ATLAS_SIMULATION_BUILD)
  #define ATLAS_FORBID_IN_SIM(msg) \
      do { \
          /* Presentation/IO function called in simulation build: */ \
          volatile int atlas_contract_violation = 0; \
          (void)atlas_contract_violation; \
          ATLAS_CONTRACT_TRAP(); \
      } while (0)
#else
  #define ATLAS_FORBID_IN_SIM(msg) ((void)0)
#endif

// ---- Forbidden API Guards (Strict Mode) ----
//
// In strict mode, common non-deterministic APIs are poisoned so
// that any accidental use produces a clear compile error.

#if ATLAS_DETERMINISM_STRICT && defined(ATLAS_SIMULATION_BUILD)
  #ifdef _GLIBCXX_CHRONO
    #error "std::chrono included in simulation code — violates Atlas Core Contract"
  #endif
  #ifdef _GLIBCXX_RANDOM
    #error "<random> included in simulation code — violates Atlas Core Contract"
  #endif
#endif

// ---- Banned Third-Party UI Libraries ----
//
// Atlas uses a fully custom UI stack. Third-party UI libraries are
// permanently banned from all code (engine, editor, client, server).
// See: docs/ATLAS_CORE_CONTRACT.md §6
//
// This guard fires in ALL translation units, not just simulation code.

#if defined(IMGUI_VERSION) || defined(IMGUI_API)
  #error "ImGui detected — ImGui is permanently banned. Use Atlas custom UI (UISceneGraph, WidgetDSL, UILayoutSolver). See docs/ATLAS_CORE_CONTRACT.md"
#endif

#if defined(NK_NUKLEAR_H_)
  #error "Nuklear detected — third-party UI libraries are permanently banned. Use Atlas custom UI. See docs/ATLAS_CORE_CONTRACT.md"
#endif
