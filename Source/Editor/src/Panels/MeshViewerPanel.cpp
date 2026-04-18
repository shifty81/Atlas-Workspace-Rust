#include "MeshViewerPanel.h"
#include <cstdio>

namespace atlas::editor {

void MeshViewerPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    const char* modeLabel = "Solid";
    switch (m_viewMode) {
        case MeshViewMode::Wireframe:     modeLabel = "Wireframe"; break;
        case MeshViewMode::SolidWireframe: modeLabel = "Solid+Wire"; break;
        case MeshViewMode::Normals:       modeLabel = "Normals"; break;
        default: break;
    }
    std::string title = std::string("Mesh Viewer [") + modeLabel + "]";
    m_drawList.DrawText({4, 4, 300, 20}, title, {220, 220, 220, 255});

    if (!m_loaded) {
        m_drawList.DrawText({4, 30, 590, 16}, "No mesh loaded", {160, 160, 160, 255});
        return;
    }

    // Stats section
    int32_t y = 28;
    char buf[128];

    std::snprintf(buf, sizeof(buf), "Vertices: %zu  Triangles: %zu",
                  m_stats.vertexCount, m_stats.triangleCount);
    m_drawList.DrawText({4, y, 590, 16}, buf, {200, 200, 200, 255});
    y += 20;

    // Bounding box
    std::snprintf(buf, sizeof(buf), "Bounds: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f)",
                  m_stats.boundsMinX, m_stats.boundsMinY, m_stats.boundsMinZ,
                  m_stats.boundsMaxX, m_stats.boundsMaxY, m_stats.boundsMaxZ);
    m_drawList.DrawText({4, y, 590, 16}, buf, {180, 180, 180, 255});
    y += 20;

    // Viewport area
    m_drawList.DrawRect({0, y, 600, 300}, {25, 25, 25, 255});
    m_drawList.DrawBorder({0, y, 600, 300}, 1, {60, 60, 60, 255});

    // Grid overlay
    if (m_showGrid) {
        int32_t gridY = y + 150;
        m_drawList.DrawRect({0, gridY, 600, 1}, {50, 50, 50, 255});
        m_drawList.DrawRect({300, y, 1, 300}, {50, 50, 50, 255});
    }

    // Normals overlay indicator
    if (m_showNormals) {
        m_drawList.DrawText({4, y + 4, 200, 16}, "[Normals ON]", {100, 200, 100, 255});
    }

    // Selected vertex indicator
    if (m_selectedVertex >= 0) {
        std::snprintf(buf, sizeof(buf), "Selected vertex: %d", m_selectedVertex);
        m_drawList.DrawText({4, y + 280, 300, 16}, buf, {255, 200, 100, 255});
    }
}

} // namespace atlas::editor
