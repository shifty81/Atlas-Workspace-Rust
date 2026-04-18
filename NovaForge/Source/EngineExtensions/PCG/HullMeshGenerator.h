#pragma once
#include <cstdint>
#include <vector>

namespace atlas::procedural {

/// A single vertex in a hull mesh.
struct HullVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/// A triangle defined by three vertex indices.
struct HullTriangle {
    uint32_t v0 = 0;
    uint32_t v1 = 0;
    uint32_t v2 = 0;
};

/// Triangle mesh representation for hull geometry.
struct HullMesh {
    std::vector<HullVertex> vertices;
    std::vector<HullTriangle> triangles;

    size_t VertexCount() const { return vertices.size(); }
    size_t TriangleCount() const { return triangles.size(); }
    void Clear() { vertices.clear(); triangles.clear(); }
    bool IsValid() const { return !vertices.empty() && !triangles.empty(); }

    /// Compute the center of mass of all vertices.
    HullVertex Centroid() const;
};

/// Configuration for the hull mesh generator pipeline.
struct HullGeneratorConfig {
    float gapFillThreshold = 0.5f;
    float stretchWrapFactor = 0.3f;
    uint32_t smoothIterations = 2;
    uint32_t subdivisionLevel = 1;

    static HullGeneratorConfig Default() { return {}; }
};

/// Generates hull meshes from point clouds using convex hull computation,
/// gap filling, stretch-wrap deformation, subdivision, and smoothing.
class HullMeshGenerator {
public:
    void SetConfig(const HullGeneratorConfig& cfg) { m_config = cfg; }
    const HullGeneratorConfig& GetConfig() const { return m_config; }

    /// Compute the convex hull of a set of points (incremental algorithm).
    HullMesh GenerateConvexHull(const std::vector<HullVertex>& points);

    /// Fill gaps by splitting edges longer than the gap-fill threshold.
    void FillGaps(HullMesh& mesh);

    /// Deform the hull toward interior points by the stretch-wrap factor.
    void ApplyStretchWrap(HullMesh& mesh, const std::vector<HullVertex>& interiorPoints);

    /// Loop subdivision — split each triangle into 4.
    void Subdivide(HullMesh& mesh);

    /// Laplacian smoothing pass.
    void Smooth(HullMesh& mesh);

    /// Full pipeline: convex hull → gap fill → stretch wrap → subdivide → smooth.
    HullMesh GenerateFromInterior(const std::vector<HullVertex>& modulePositions);

    /// Retrieve the last mesh produced by GenerateFromInterior.
    const HullMesh& GetLastMesh() const { return m_lastMesh; }

private:
    HullGeneratorConfig m_config;
    HullMesh m_lastMesh;
};

} // namespace atlas::procedural
