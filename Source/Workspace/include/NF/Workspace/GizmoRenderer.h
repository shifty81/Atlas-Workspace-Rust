#pragma once
// NF::GizmoRenderer — Gizmo overlay pass (Item 6 of 10).
//
// Draws transformation handles (Translate/Rotate/Scale) as a second overlay
// pass on top of the scene FBO before blitting to the UI panel.
//
// GizmoDrawCommands are accumulated each frame, then renderToSurface() is
// called by the frame loop after the main scene render pass to composite the
// gizmo geometry.  In the current (architecturally complete) implementation the
// gizmo pass is a counted stub — real geometry is emitted by concrete backends.

#include "NF/Workspace/IViewportSurface.h"
#include "NF/Core/Core.h"
#include "NF/Workspace/GizmoAxis.h"
#include <vector>

namespace NF {

// ── GizmoType ─────────────────────────────────────────────────────────────────

enum class GizmoType : uint8_t { Translate, Rotate, Scale };

inline const char* gizmoTypeName(GizmoType type) {
    switch (type) {
    case GizmoType::Translate: return "Translate";
    case GizmoType::Rotate:    return "Rotate";
    case GizmoType::Scale:     return "Scale";
    }
    return "Unknown";
}

// ── GizmoDrawCommand ──────────────────────────────────────────────────────────

struct GizmoDrawCommand {
    GizmoAxis axis        = GizmoAxis::XYZ;
    GizmoType type        = GizmoType::Translate;
    Vec3      position    = {0.f, 0.f, 0.f};
    float     size        = 1.f;
    bool      highlighted = false; ///< axis is being hovered or dragged
};

// ── GizmoRenderer ─────────────────────────────────────────────────────────────
// Accumulates GizmoDrawCommands per frame and composites them onto the scene
// surface as a second overlay pass.

class GizmoRenderer {
public:
    static constexpr size_t kMaxGizmos = 64;

    /// Add a gizmo draw command for this frame.
    void addGizmo(const GizmoDrawCommand& cmd) {
        if (m_commands.size() < kMaxGizmos)
            m_commands.push_back(cmd);
    }

    /// Remove all gizmo commands (call at start of each frame).
    void clear() { m_commands.clear(); }

    /// Composite all gizmo commands onto the given surface.
    /// Returns the number of gizmos that were submitted (or would be rendered).
    /// The surface must be valid; returns 0 for invalid surfaces.
    /// Real backends emit line/triangle geometry here; this is an architecturally
    /// complete stub that counts submissions without issuing GPU calls.
    uint32_t renderToSurface(IViewportSurface& surface, ViewportHandle handle) {
        if (!surface.isValid()) return 0;
        (void)handle;
        return static_cast<uint32_t>(m_commands.size());
    }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }
    [[nodiscard]] const std::vector<GizmoDrawCommand>& commands() const { return m_commands; }

    /// Returns true if any gizmo command has the highlighted flag set.
    [[nodiscard]] bool hasHighlightedGizmo() const {
        for (const auto& cmd : m_commands)
            if (cmd.highlighted) return true;
        return false;
    }

private:
    std::vector<GizmoDrawCommand> m_commands;
};

} // namespace NF
