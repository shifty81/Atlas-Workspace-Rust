#include "../engine/world/TerrainMeshGenerator.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::world;

void test_terrain_flat_mesh() {
    ChunkCoord chunk = {0, 0, 0, 0};
    TerrainMesh mesh = TerrainMeshGenerator::Generate(chunk, 4, 16.0f);

    // 4x4 quads = 5x5 vertices = 25
    assert(mesh.vertices.size() == 25);
    // 4x4 quads * 2 triangles * 3 indices = 96
    assert(mesh.indices.size() == 96);

    // All heights should be 0 for flat mesh
    for (const auto& v : mesh.vertices) {
        assert(v.y == 0.0f);
    }

    std::cout << "[PASS] test_terrain_flat_mesh" << std::endl;
}

void test_terrain_heightmap_mesh() {
    ChunkCoord chunk = {0, 0, 0, 0};
    auto heightFn = [](float x, float z) -> float {
        return x + z;
    };

    TerrainMesh mesh = TerrainMeshGenerator::Generate(chunk, 2, 10.0f, heightFn);

    // 2x2 quads = 3x3 vertices = 9
    assert(mesh.vertices.size() == 9);
    // 2x2 quads * 2 * 3 = 24
    assert(mesh.indices.size() == 24);

    // Verify heights match the height function
    for (const auto& v : mesh.vertices) {
        float expected = v.x + v.z;
        assert(std::abs(v.y - expected) < 0.001f);
    }

    std::cout << "[PASS] test_terrain_heightmap_mesh" << std::endl;
}

void test_terrain_normals_computed() {
    ChunkCoord chunk = {0, 0, 0, 0};
    auto heightFn = [](float x, float z) -> float {
        return x * 0.5f; // slope in x direction
    };

    TerrainMesh mesh = TerrainMeshGenerator::Generate(chunk, 2, 10.0f, heightFn);

    // Normals should be computed and non-zero
    bool hasNonZeroNormal = false;
    for (const auto& v : mesh.vertices) {
        float len = std::sqrt(v.nx * v.nx + v.ny * v.ny + v.nz * v.nz);
        if (len > 0.5f) {
            hasNonZeroNormal = true;
        }
    }
    assert(hasNonZeroNormal);

    std::cout << "[PASS] test_terrain_normals_computed" << std::endl;
}

void test_terrain_resolution_one() {
    ChunkCoord chunk = {1, 0, 1, 0};
    TerrainMesh mesh = TerrainMeshGenerator::Generate(chunk, 1, 8.0f);

    // 1x1 quad = 2x2 vertices = 4
    assert(mesh.vertices.size() == 4);
    // 1 quad * 2 triangles * 3 = 6
    assert(mesh.indices.size() == 6);

    std::cout << "[PASS] test_terrain_resolution_one" << std::endl;
}
