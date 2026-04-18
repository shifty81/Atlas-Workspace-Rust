#include "../engine/procedural/LODBakingGraph.h"
#include "../engine/procedural/LODBakingNodes.h"
#include "../engine/procedural/ProceduralMeshNodes.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_lod_decimate_mesh() {
    auto cube = atlas::procedural::GenerateCube(2.0f);
    assert(cube.TriangleCount() == 12);

    auto decimated = atlas::procedural::DecimateMesh(cube, 0.5f);
    assert(decimated.IsValid());
    assert(decimated.TriangleCount() == 6);
    assert(decimated.VertexCount() == cube.VertexCount());

    // keepFactor 1.0 should keep all
    auto full = atlas::procedural::DecimateMesh(cube, 1.0f);
    assert(full.TriangleCount() == 12);

    // keepFactor 0.25 should keep 3 triangles
    auto quarter = atlas::procedural::DecimateMesh(cube, 0.25f);
    assert(quarter.TriangleCount() == 3);

    std::cout << "[PASS] test_lod_decimate_mesh" << std::endl;
}

void test_lod_merge_vertices() {
    atlas::procedural::MeshData mesh;
    // Two vertices very close together, one far away
    mesh.vertices = {0.0f, 0.0f, 0.0f,  0.001f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f};
    mesh.normals  = {0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f};
    mesh.indices  = {0, 1, 2};
    assert(mesh.IsValid());

    auto merged = atlas::procedural::MergeNearbyVertices(mesh, 0.01f);
    assert(merged.IsValid());
    assert(merged.VertexCount() == 2);
    // Triangle becomes degenerate after merging vertex 1 into vertex 0
    assert(merged.TriangleCount() == 0);

    // With a very small threshold, nothing should merge
    auto noMerge = atlas::procedural::MergeNearbyVertices(mesh, 0.0001f);
    assert(noMerge.VertexCount() == 3);
    assert(noMerge.TriangleCount() == 1);

    std::cout << "[PASS] test_lod_merge_vertices" << std::endl;
}

void test_lod_recompute_normals() {
    // Create a simple triangle with wrong normals
    atlas::procedural::MeshData mesh;
    mesh.vertices = {0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f};
    mesh.normals  = {1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f}; // wrong
    mesh.indices  = {0, 1, 2};

    auto fixed = atlas::procedural::RecomputeNormals(mesh);
    assert(fixed.IsValid());
    assert(fixed.VertexCount() == 3);
    assert(fixed.TriangleCount() == 1);

    // The face normal for this triangle should point in +Z direction
    for (size_t i = 0; i < fixed.normals.size(); i += 3) {
        assert(std::fabs(fixed.normals[i])     < 0.01f);  // nx ~ 0
        assert(std::fabs(fixed.normals[i + 1]) < 0.01f);  // ny ~ 0
        assert(std::fabs(fixed.normals[i + 2] - 1.0f) < 0.01f);  // nz ~ 1
    }

    std::cout << "[PASS] test_lod_recompute_normals" << std::endl;
}

void test_lod_chain_generation() {
    auto cube = atlas::procedural::GenerateCube(2.0f);
    assert(cube.TriangleCount() == 12);

    auto chain = atlas::procedural::GenerateLODChain(cube, 3);
    assert(chain.IsValid());
    assert(chain.LevelCount() == 3);

    // Level 0: full detail (12 triangles)
    assert(chain.levels[0].level == 0);
    assert(chain.levels[0].reductionFactor == 1.0f);
    assert(chain.levels[0].mesh.TriangleCount() == 12);

    // Level 1: half detail (6 triangles)
    assert(chain.levels[1].level == 1);
    assert(std::fabs(chain.levels[1].reductionFactor - 0.5f) < 0.01f);
    assert(chain.levels[1].mesh.TriangleCount() == 6);

    // Level 2: quarter detail (3 triangles)
    assert(chain.levels[2].level == 2);
    assert(std::fabs(chain.levels[2].reductionFactor - 0.25f) < 0.01f);
    assert(chain.levels[2].mesh.TriangleCount() == 3);

    // Each level should have fewer or equal triangles
    for (size_t i = 1; i < chain.levels.size(); ++i) {
        assert(chain.levels[i].mesh.TriangleCount() <= chain.levels[i - 1].mesh.TriangleCount());
    }

    std::cout << "[PASS] test_lod_chain_generation" << std::endl;
}

void test_lod_graph_add_nodes() {
    atlas::procedural::LODBakingGraph graph;
    auto id1 = graph.AddNode(atlas::procedural::LODNodeType::MeshInput);
    auto id2 = graph.AddNode(atlas::procedural::LODNodeType::Output);
    assert(id1 == 1);
    assert(id2 == 2);
    assert(graph.NodeCount() == 2);
    std::cout << "[PASS] test_lod_graph_add_nodes" << std::endl;
}

void test_lod_graph_remove_node() {
    atlas::procedural::LODBakingGraph graph;
    auto id = graph.AddNode(atlas::procedural::LODNodeType::MeshInput);
    assert(graph.NodeCount() == 1);
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_lod_graph_remove_node" << std::endl;
}

void test_lod_graph_compile_empty() {
    atlas::procedural::LODBakingGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_lod_graph_compile_empty" << std::endl;
}

void test_lod_graph_execute_basic() {
    atlas::procedural::LODBakingGraph graph;
    auto inputId = graph.AddNode(atlas::procedural::LODNodeType::MeshInput);
    graph.SetNodeProperty(inputId, "shape", "cube");
    graph.SetNodeProperty(inputId, "size", "2.0");

    auto outId = graph.AddNode(atlas::procedural::LODNodeType::Output);
    graph.SetNodeProperty(outId, "levels", "3");

    graph.AddEdge({inputId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->LevelCount() == 3);
    assert(output->levels[0].mesh.TriangleCount() == 12);

    std::cout << "[PASS] test_lod_graph_execute_basic" << std::endl;
}

void test_lod_graph_decimate_pipeline() {
    atlas::procedural::LODBakingGraph graph;

    auto inputId = graph.AddNode(atlas::procedural::LODNodeType::MeshInput);
    graph.SetNodeProperty(inputId, "shape", "cube");
    graph.SetNodeProperty(inputId, "size", "2.0");

    auto decimateId = graph.AddNode(atlas::procedural::LODNodeType::Decimate);
    graph.SetNodeProperty(decimateId, "factor", "0.5");

    auto normalsId = graph.AddNode(atlas::procedural::LODNodeType::BakeNormals);

    auto outId = graph.AddNode(atlas::procedural::LODNodeType::Output);
    graph.SetNodeProperty(outId, "levels", "2");

    graph.AddEdge({inputId, 0, decimateId, 0});
    graph.AddEdge({decimateId, 0, normalsId, 0});
    graph.AddEdge({normalsId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->LevelCount() == 2);
    // After decimating cube (12 tris) by 0.5, we get 6 tris as base
    assert(output->levels[0].mesh.TriangleCount() == 6);
    // Level 1 at half of that = 3
    assert(output->levels[1].mesh.TriangleCount() == 3);

    std::cout << "[PASS] test_lod_graph_decimate_pipeline" << std::endl;
}

void test_lod_graph_lod_chain_output() {
    atlas::procedural::LODBakingGraph graph;

    auto inputId = graph.AddNode(atlas::procedural::LODNodeType::MeshInput);
    graph.SetNodeProperty(inputId, "shape", "sphere");
    graph.SetNodeProperty(inputId, "size", "2.0");
    graph.SetNodeProperty(inputId, "segments", "8");

    auto outId = graph.AddNode(atlas::procedural::LODNodeType::Output);
    graph.SetNodeProperty(outId, "levels", "4");

    graph.AddEdge({inputId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->LevelCount() == 4);

    // Sphere with 8 segments = 128 triangles
    assert(output->levels[0].mesh.TriangleCount() == 128);

    // Each level should have decreasing triangle count
    for (size_t i = 1; i < output->levels.size(); ++i) {
        assert(output->levels[i].mesh.TriangleCount() < output->levels[i - 1].mesh.TriangleCount());
    }

    std::cout << "[PASS] test_lod_graph_lod_chain_output" << std::endl;
}
