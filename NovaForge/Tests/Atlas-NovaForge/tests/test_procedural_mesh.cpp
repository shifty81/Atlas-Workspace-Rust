#include "../engine/procedural/ProceduralMeshGraph.h"
#include "../engine/procedural/ProceduralMeshNodes.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_procedural_add_nodes() {
    atlas::procedural::ProceduralMeshGraph graph;
    auto id1 = graph.AddNode(atlas::procedural::ProceduralNodeType::Primitive);
    auto id2 = graph.AddNode(atlas::procedural::ProceduralNodeType::Output);
    assert(id1 == 1);
    assert(id2 == 2);
    assert(graph.NodeCount() == 2);
    std::cout << "[PASS] test_procedural_add_nodes" << std::endl;
}

void test_procedural_remove_node() {
    atlas::procedural::ProceduralMeshGraph graph;
    auto id = graph.AddNode(atlas::procedural::ProceduralNodeType::Primitive);
    assert(graph.NodeCount() == 1);
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_procedural_remove_node" << std::endl;
}

void test_procedural_compile_empty() {
    atlas::procedural::ProceduralMeshGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_procedural_compile_empty" << std::endl;
}

void test_procedural_cube() {
    auto mesh = atlas::procedural::GenerateCube(2.0f);
    assert(mesh.IsValid());
    assert(mesh.VertexCount() == 8);
    assert(mesh.TriangleCount() == 12);
    assert(mesh.indices.size() == 36);
    std::cout << "[PASS] test_procedural_cube" << std::endl;
}

void test_procedural_plane() {
    auto mesh = atlas::procedural::GeneratePlane(4.0f);
    assert(mesh.IsValid());
    assert(mesh.VertexCount() == 4);
    assert(mesh.TriangleCount() == 2);
    assert(mesh.indices.size() == 6);
    std::cout << "[PASS] test_procedural_plane" << std::endl;
}

void test_procedural_sphere() {
    auto mesh = atlas::procedural::GenerateSphere(2.0f, 8);
    assert(mesh.IsValid());
    assert(mesh.VertexCount() > 0);
    assert(mesh.TriangleCount() > 0);
    // UV sphere with 8 segments/rings: (8+1)*(8+1)=81 verts, 8*8*2=128 tris
    assert(mesh.VertexCount() == 81);
    assert(mesh.TriangleCount() == 128);
    std::cout << "[PASS] test_procedural_sphere" << std::endl;
}

void test_procedural_transform() {
    atlas::procedural::ProceduralMeshGraph graph;
    auto primId = graph.AddNode(atlas::procedural::ProceduralNodeType::Primitive);
    graph.SetNodeProperty(primId, "shape", "cube");
    graph.SetNodeProperty(primId, "size", "2.0");

    auto xformId = graph.AddNode(atlas::procedural::ProceduralNodeType::Transform);
    graph.SetNodeProperty(xformId, "translateX", "10.0");

    auto outId = graph.AddNode(atlas::procedural::ProceduralNodeType::Output);

    graph.AddEdge({primId, 0, xformId, 0});
    graph.AddEdge({xformId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->VertexCount() == 8);

    // All X coords should be shifted by +10
    for (size_t i = 0; i < output->vertices.size(); i += 3) {
        assert(output->vertices[i] >= 9.0f);
    }
    std::cout << "[PASS] test_procedural_transform" << std::endl;
}

void test_procedural_merge() {
    atlas::procedural::ProceduralMeshGraph graph;
    auto prim1 = graph.AddNode(atlas::procedural::ProceduralNodeType::Primitive);
    graph.SetNodeProperty(prim1, "shape", "cube");
    graph.SetNodeProperty(prim1, "size", "1.0");

    auto prim2 = graph.AddNode(atlas::procedural::ProceduralNodeType::Primitive);
    graph.SetNodeProperty(prim2, "shape", "cube");
    graph.SetNodeProperty(prim2, "size", "1.0");

    auto mergeId = graph.AddNode(atlas::procedural::ProceduralNodeType::Merge);
    auto outId = graph.AddNode(atlas::procedural::ProceduralNodeType::Output);

    graph.AddEdge({prim1, 0, mergeId, 0});
    graph.AddEdge({prim2, 0, mergeId, 1});
    graph.AddEdge({mergeId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->VertexCount() == 16);  // 8 + 8
    assert(output->TriangleCount() == 24);  // 12 + 12
    std::cout << "[PASS] test_procedural_merge" << std::endl;
}

void test_procedural_subdivide() {
    auto cube = atlas::procedural::GenerateCube(2.0f);
    size_t origTris = cube.TriangleCount();

    auto subdivided = atlas::procedural::SubdivideMesh(cube);
    assert(subdivided.IsValid());
    assert(subdivided.TriangleCount() == origTris * 4);
    assert(subdivided.VertexCount() > cube.VertexCount());
    std::cout << "[PASS] test_procedural_subdivide" << std::endl;
}

void test_procedural_noise_determinism() {
    auto cube = atlas::procedural::GenerateCube(2.0f);

    auto a = atlas::procedural::ApplyNoiseDisplacement(cube, 0.5f, 42);
    auto b = atlas::procedural::ApplyNoiseDisplacement(cube, 0.5f, 42);
    assert(a.vertices == b.vertices);

    auto c = atlas::procedural::ApplyNoiseDisplacement(cube, 0.5f, 99);
    assert(a.vertices != c.vertices);
    std::cout << "[PASS] test_procedural_noise_determinism" << std::endl;
}

void test_procedural_full_pipeline() {
    atlas::procedural::ProceduralMeshGraph graph;

    auto primId = graph.AddNode(atlas::procedural::ProceduralNodeType::Primitive);
    graph.SetNodeProperty(primId, "shape", "plane");
    graph.SetNodeProperty(primId, "size", "4.0");

    auto xformId = graph.AddNode(atlas::procedural::ProceduralNodeType::Transform);
    graph.SetNodeProperty(xformId, "translateY", "5.0");
    graph.SetNodeProperty(xformId, "scaleX", "2.0");

    auto outId = graph.AddNode(atlas::procedural::ProceduralNodeType::Output);

    graph.AddEdge({primId, 0, xformId, 0});
    graph.AddEdge({xformId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->VertexCount() == 4);
    assert(output->TriangleCount() == 2);

    // Y should be shifted by 5, X should be scaled by 2
    for (size_t i = 0; i < output->vertices.size(); i += 3) {
        assert(std::fabs(output->vertices[i + 1] - 5.0f) < 0.01f);  // Y = 0*1 + 5
        assert(std::fabs(output->vertices[i]) <= 4.0f + 0.01f);  // X scaled by 2
    }
    std::cout << "[PASS] test_procedural_full_pipeline" << std::endl;
}
