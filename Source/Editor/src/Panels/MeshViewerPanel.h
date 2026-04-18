#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/procedural/ProceduralMeshGraph.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <sstream>

namespace atlas::editor {

enum class MeshViewMode : uint8_t {
    Solid,
    Wireframe,
    SolidWireframe,
    Normals
};

struct MeshViewerStats {
    size_t vertexCount = 0;
    size_t triangleCount = 0;
    float boundsMinX = 0, boundsMinY = 0, boundsMinZ = 0;
    float boundsMaxX = 0, boundsMaxY = 0, boundsMaxZ = 0;
};

class MeshViewerPanel : public EditorPanel {
public:
    const char* Name() const override { return "Mesh Viewer"; }
    void Draw() override;

    void LoadMesh(const procedural::MeshData& mesh) {
        m_mesh = mesh;
        m_loaded = true;
        ComputeStats();
    }

    void Clear() {
        m_mesh.Clear();
        m_loaded = false;
        m_stats = {};
        m_selectedVertex = -1;
    }

    bool HasMesh() const { return m_loaded && m_mesh.IsValid(); }
    const procedural::MeshData& GetMesh() const { return m_mesh; }
    const MeshViewerStats& Stats() const { return m_stats; }

    void SetViewMode(MeshViewMode mode) { m_viewMode = mode; }
    MeshViewMode ViewMode() const { return m_viewMode; }

    void SetShowNormals(bool show) { m_showNormals = show; }
    bool ShowNormals() const { return m_showNormals; }

    void SetShowGrid(bool show) { m_showGrid = show; }
    bool ShowGrid() const { return m_showGrid; }

    void SelectVertex(int index) {
        if (index >= 0 && static_cast<size_t>(index) < m_mesh.VertexCount()) {
            m_selectedVertex = index;
        } else {
            m_selectedVertex = -1;
        }
    }
    int SelectedVertex() const { return m_selectedVertex; }

    std::string Summary() const {
        if (!m_loaded) return "No mesh loaded";
        std::ostringstream oss;
        oss << "Vertices: " << m_stats.vertexCount
            << ", Triangles: " << m_stats.triangleCount;
        return oss.str();
    }

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    procedural::MeshData m_mesh;
    bool m_loaded = false;
    MeshViewerStats m_stats;
    MeshViewMode m_viewMode = MeshViewMode::Solid;
    bool m_showNormals = false;
    bool m_showGrid = true;
    int m_selectedVertex = -1;
    atlas::ui::UIDrawList m_drawList;

    void ComputeStats() {
        m_stats.vertexCount = m_mesh.VertexCount();
        m_stats.triangleCount = m_mesh.TriangleCount();

        if (m_stats.vertexCount == 0) return;

        m_stats.boundsMinX = m_mesh.vertices[0];
        m_stats.boundsMinY = m_mesh.vertices[1];
        m_stats.boundsMinZ = m_mesh.vertices[2];
        m_stats.boundsMaxX = m_mesh.vertices[0];
        m_stats.boundsMaxY = m_mesh.vertices[1];
        m_stats.boundsMaxZ = m_mesh.vertices[2];

        for (size_t i = 0; i < m_mesh.vertices.size(); i += 3) {
            float x = m_mesh.vertices[i];
            float y = m_mesh.vertices[i + 1];
            float z = m_mesh.vertices[i + 2];
            if (x < m_stats.boundsMinX) m_stats.boundsMinX = x;
            if (y < m_stats.boundsMinY) m_stats.boundsMinY = y;
            if (z < m_stats.boundsMinZ) m_stats.boundsMinZ = z;
            if (x > m_stats.boundsMaxX) m_stats.boundsMaxX = x;
            if (y > m_stats.boundsMaxY) m_stats.boundsMaxY = y;
            if (z > m_stats.boundsMaxZ) m_stats.boundsMaxZ = z;
        }
    }
};

} // namespace atlas::editor
