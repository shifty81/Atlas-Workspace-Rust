#include "LODBakingNodes.h"
#include <cmath>
#include <unordered_map>

namespace atlas::procedural {

MeshData DecimateMesh(const MeshData& input, float keepFactor) {
    MeshData result;
    if (!input.IsValid() || input.TriangleCount() == 0) return input;

    if (keepFactor <= 0.0f) {
        // Keep vertices and normals but remove all triangles
        result.vertices = input.vertices;
        result.normals = input.normals;
        return result;
    }
    if (keepFactor >= 1.0f) return input;

    size_t totalTris = input.TriangleCount();
    size_t keepCount = static_cast<size_t>(std::floor(keepFactor * static_cast<float>(totalTris)));
    if (keepCount == 0) keepCount = 1;

    result.vertices = input.vertices;
    result.normals = input.normals;
    // Keep the first keepCount triangles (remove from the end)
    result.indices.assign(input.indices.begin(),
                          input.indices.begin() + static_cast<ptrdiff_t>(keepCount * 3));

    return result;
}

MeshData MergeNearbyVertices(const MeshData& input, float threshold) {
    MeshData result;
    if (!input.IsValid() || input.VertexCount() == 0) return input;

    float thresh2 = threshold * threshold;
    size_t vertCount = input.VertexCount();

    // Map from old vertex index to new vertex index
    std::vector<uint32_t> remap(vertCount);
    std::vector<float> newVerts;
    std::vector<float> newNormals;
    uint32_t newCount = 0;

    for (size_t i = 0; i < vertCount; ++i) {
        float vx = input.vertices[i * 3];
        float vy = input.vertices[i * 3 + 1];
        float vz = input.vertices[i * 3 + 2];

        // Check if this vertex is within threshold of an earlier vertex
        bool merged = false;
        for (size_t j = 0; j < i; ++j) {
            float dx = vx - input.vertices[j * 3];
            float dy = vy - input.vertices[j * 3 + 1];
            float dz = vz - input.vertices[j * 3 + 2];
            if (dx * dx + dy * dy + dz * dz <= thresh2) {
                remap[i] = remap[j];
                merged = true;
                break;
            }
        }

        if (!merged) {
            remap[i] = newCount;
            newVerts.push_back(vx);
            newVerts.push_back(vy);
            newVerts.push_back(vz);
            newNormals.push_back(input.normals[i * 3]);
            newNormals.push_back(input.normals[i * 3 + 1]);
            newNormals.push_back(input.normals[i * 3 + 2]);
            newCount++;
        }
    }

    result.vertices = std::move(newVerts);
    result.normals = std::move(newNormals);

    // Remap indices, skip degenerate triangles
    for (size_t i = 0; i + 2 < input.indices.size(); i += 3) {
        uint32_t a = remap[input.indices[i]];
        uint32_t b = remap[input.indices[i + 1]];
        uint32_t c = remap[input.indices[i + 2]];
        if (a != b && b != c && a != c) {
            result.indices.push_back(a);
            result.indices.push_back(b);
            result.indices.push_back(c);
        }
    }

    return result;
}

MeshData RecomputeNormals(const MeshData& input) {
    MeshData result;
    if (!input.IsValid()) return input;

    result.vertices = input.vertices;
    result.indices = input.indices;
    result.normals.resize(input.vertices.size(), 0.0f);

    // Accumulate face normals at each vertex
    for (size_t i = 0; i + 2 < input.indices.size(); i += 3) {
        uint32_t i0 = input.indices[i];
        uint32_t i1 = input.indices[i + 1];
        uint32_t i2 = input.indices[i + 2];

        float ax = input.vertices[i0 * 3], ay = input.vertices[i0 * 3 + 1], az = input.vertices[i0 * 3 + 2];
        float bx = input.vertices[i1 * 3], by = input.vertices[i1 * 3 + 1], bz = input.vertices[i1 * 3 + 2];
        float cx = input.vertices[i2 * 3], cy = input.vertices[i2 * 3 + 1], cz = input.vertices[i2 * 3 + 2];

        float ux = bx - ax, uy = by - ay, uz = bz - az;
        float vx = cx - ax, vy = cy - ay, vz = cz - az;
        float nx = uy * vz - uz * vy;
        float ny = uz * vx - ux * vz;
        float nz = ux * vy - uy * vx;

        result.normals[i0 * 3]     += nx;
        result.normals[i0 * 3 + 1] += ny;
        result.normals[i0 * 3 + 2] += nz;
        result.normals[i1 * 3]     += nx;
        result.normals[i1 * 3 + 1] += ny;
        result.normals[i1 * 3 + 2] += nz;
        result.normals[i2 * 3]     += nx;
        result.normals[i2 * 3 + 1] += ny;
        result.normals[i2 * 3 + 2] += nz;
    }

    // Normalize
    for (size_t i = 0; i + 2 < result.normals.size(); i += 3) {
        float nx = result.normals[i], ny = result.normals[i + 1], nz = result.normals[i + 2];
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.0f) {
            result.normals[i]     = nx / len;
            result.normals[i + 1] = ny / len;
            result.normals[i + 2] = nz / len;
        }
    }

    return result;
}

LODChain GenerateLODChain(const MeshData& baseMesh, uint32_t levelCount) {
    LODChain chain;
    if (levelCount == 0) return chain;

    for (uint32_t i = 0; i < levelCount; ++i) {
        LODLevel level;
        level.level = i;
        // Each level halves the factor: 1.0, 0.5, 0.25, 0.125, ...
        float factor = 1.0f / static_cast<float>(1u << i);
        level.reductionFactor = factor;
        level.mesh = DecimateMesh(baseMesh, factor);
        chain.levels.push_back(std::move(level));
    }

    return chain;
}

}
