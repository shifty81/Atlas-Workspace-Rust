#pragma once
// NF::ViewportRenderMode — Canonical viewport rendering mode enumeration.
//
// Previously defined independently in Workspace (ViewportHostContract.h) and
// Editor (AdvancedViewports.h) with overlapping but incompatible value sets.
// This header merges them into a single definition that satisfies all consumers.

#include <cstdint>

namespace NF {

enum class ViewportRenderMode : uint8_t {
    Lit,       // PBR / standard lighting
    Unlit,     // ignore lights, full albedo
    Wireframe, // geometry outline
    Solid,     // solid fill without lighting
    Textured,  // textured without full PBR lighting
    Normals,   // visualize vertex normals
    Overdraw,  // visualize overdraw cost
};

inline const char* viewportRenderModeName(ViewportRenderMode m) {
    switch (m) {
    case ViewportRenderMode::Lit:       return "Lit";
    case ViewportRenderMode::Unlit:     return "Unlit";
    case ViewportRenderMode::Wireframe: return "Wireframe";
    case ViewportRenderMode::Solid:     return "Solid";
    case ViewportRenderMode::Textured:  return "Textured";
    case ViewportRenderMode::Normals:   return "Normals";
    case ViewportRenderMode::Overdraw:  return "Overdraw";
    }
    return "Unknown";
}

} // namespace NF
