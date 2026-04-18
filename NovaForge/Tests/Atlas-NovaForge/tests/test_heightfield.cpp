#include "../engine/world/HeightfieldMesher.h"
#include <iostream>
#include <cassert>

void test_heightfield_at() {
    atlas::world::Heightfield hf;
    hf.size = 4;
    hf.scale = 1.0f;
    hf.data.resize(16, 0.0f);
    hf.data[0] = 1.0f;  // (0,0)
    hf.data[5] = 2.0f;  // (1,1)
    assert(hf.At(0, 0) == 1.0f);
    assert(hf.At(1, 1) == 2.0f);
    assert(hf.At(2, 2) == 0.0f);
    std::cout << "[PASS] test_heightfield_at" << std::endl;
}

void test_heightfield_mesh_generation() {
    atlas::world::Heightfield hf;
    hf.size = 4;
    hf.scale = 1.0f;
    hf.data.resize(16, 0.5f);

    auto mesh = atlas::world::HeightfieldMesher::BuildMesh(hf, 0);
    assert(!mesh.vertices.empty());
    assert(!mesh.indices.empty());
    assert(!mesh.normals.empty());
    // 4x4 grid = 16 vertices * 3 components = 48
    assert(mesh.vertices.size() == 48);
    std::cout << "[PASS] test_heightfield_mesh_generation" << std::endl;
}

void test_heightfield_mesh_lod() {
    atlas::world::Heightfield hf;
    hf.size = 8;
    hf.scale = 1.0f;
    hf.data.resize(64, 0.0f);

    auto mesh0 = atlas::world::HeightfieldMesher::BuildMesh(hf, 0);
    auto mesh1 = atlas::world::HeightfieldMesher::BuildMesh(hf, 1);
    // LOD 1 should have fewer vertices than LOD 0
    assert(mesh1.vertices.size() < mesh0.vertices.size());
    std::cout << "[PASS] test_heightfield_mesh_lod" << std::endl;
}
