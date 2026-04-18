#pragma once
// NF::GizmoAxis — Axis selection for gizmo operations.
//
// Extracted into a standalone minimal header so that EditorCamera.h
// can share this type without pulling in the full GizmoRenderer chain.

#include <cstdint>

namespace NF {

// ── GizmoAxis ─────────────────────────────────────────────────────────────────

enum class GizmoAxis : uint8_t { X, Y, Z, XY, XZ, YZ, XYZ, None };

inline const char* gizmoAxisName(GizmoAxis axis) {
    switch (axis) {
    case GizmoAxis::X:    return "X";
    case GizmoAxis::Y:    return "Y";
    case GizmoAxis::Z:    return "Z";
    case GizmoAxis::XY:   return "XY";
    case GizmoAxis::XZ:   return "XZ";
    case GizmoAxis::YZ:   return "YZ";
    case GizmoAxis::XYZ:  return "XYZ";
    case GizmoAxis::None: return "None";
    }
    return "Unknown";
}

} // namespace NF
