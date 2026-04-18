#include "../engine/procedural/ProceduralMaterialGraph.h"
#include "../engine/procedural/ProceduralMaterialNodes.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_material_solid_color() {
    auto mat = atlas::procedural::GenerateSolidColor(4, 4, 1.0f, 0.0f, 0.0f, 1.0f);
    assert(mat.IsValid());
    assert(mat.PixelCount() == 16);
    assert(mat.albedo.size() == 64);
    // First pixel should be red
    assert(std::fabs(mat.albedo[0] - 1.0f) < 0.001f);
    assert(std::fabs(mat.albedo[1] - 0.0f) < 0.001f);
    assert(std::fabs(mat.albedo[2] - 0.0f) < 0.001f);
    assert(std::fabs(mat.albedo[3] - 1.0f) < 0.001f);
    // Normals should be flat (0,0,1)
    assert(std::fabs(mat.normal[0] - 0.0f) < 0.001f);
    assert(std::fabs(mat.normal[1] - 0.0f) < 0.001f);
    assert(std::fabs(mat.normal[2] - 1.0f) < 0.001f);
    std::cout << "[PASS] test_material_solid_color" << std::endl;
}

void test_material_checkerboard() {
    auto mat = atlas::procedural::GenerateCheckerboard(8, 8, 4, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    assert(mat.IsValid());
    assert(mat.PixelCount() == 64);
    // Pixel (0,0) should be color1 (white)
    assert(std::fabs(mat.albedo[0] - 1.0f) < 0.001f);
    assert(std::fabs(mat.albedo[1] - 1.0f) < 0.001f);
    assert(std::fabs(mat.albedo[2] - 1.0f) < 0.001f);
    // Pixel (4,0) should be color2 (black)
    size_t idx = 4;
    assert(std::fabs(mat.albedo[idx * 4 + 0] - 0.0f) < 0.001f);
    assert(std::fabs(mat.albedo[idx * 4 + 1] - 0.0f) < 0.001f);
    assert(std::fabs(mat.albedo[idx * 4 + 2] - 0.0f) < 0.001f);
    std::cout << "[PASS] test_material_checkerboard" << std::endl;
}

void test_material_noise() {
    auto mat1 = atlas::procedural::GenerateNoiseTexture(8, 8, 42, 1.0f);
    auto mat2 = atlas::procedural::GenerateNoiseTexture(8, 8, 42, 1.0f);
    assert(mat1.IsValid());
    assert(mat2.IsValid());
    // Deterministic: same seed produces same result
    assert(mat1.albedo == mat2.albedo);
    assert(mat1.roughness == mat2.roughness);
    // Different seed produces different result
    auto mat3 = atlas::procedural::GenerateNoiseTexture(8, 8, 99, 1.0f);
    assert(mat1.albedo != mat3.albedo);
    std::cout << "[PASS] test_material_noise" << std::endl;
}

void test_material_blend() {
    auto red = atlas::procedural::GenerateSolidColor(4, 4, 1.0f, 0.0f, 0.0f);
    auto blue = atlas::procedural::GenerateSolidColor(4, 4, 0.0f, 0.0f, 1.0f);
    auto blended = atlas::procedural::BlendMaterials(red, blue, 0.5f);
    assert(blended.IsValid());
    // 50/50 blend of red and blue should give (0.5, 0, 0.5)
    assert(std::fabs(blended.albedo[0] - 0.5f) < 0.001f);
    assert(std::fabs(blended.albedo[1] - 0.0f) < 0.001f);
    assert(std::fabs(blended.albedo[2] - 0.5f) < 0.001f);
    std::cout << "[PASS] test_material_blend" << std::endl;
}

void test_material_normal_map() {
    auto noise = atlas::procedural::GenerateNoiseTexture(8, 8, 42, 1.0f);
    auto normalMapped = atlas::procedural::ComputeNormalMap(noise, 1.0f);
    assert(normalMapped.IsValid());
    assert(normalMapped.width == 8);
    assert(normalMapped.height == 8);
    // Normals should be unit length
    for (size_t i = 0; i < normalMapped.PixelCount(); ++i) {
        float nx = normalMapped.normal[i * 3 + 0];
        float ny = normalMapped.normal[i * 3 + 1];
        float nz = normalMapped.normal[i * 3 + 2];
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        assert(std::fabs(len - 1.0f) < 0.01f);
    }
    std::cout << "[PASS] test_material_normal_map" << std::endl;
}

void test_material_graph_compile() {
    atlas::procedural::ProceduralMaterialGraph graph;
    auto solidId = graph.AddNode(atlas::procedural::MaterialNodeType::SolidColor);
    auto outId = graph.AddNode(atlas::procedural::MaterialNodeType::Output);
    graph.AddEdge({solidId, 0, outId, 0});

    assert(!graph.IsCompiled());
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_material_graph_compile" << std::endl;
}

void test_material_graph_execute_solid() {
    atlas::procedural::ProceduralMaterialGraph graph;
    auto solidId = graph.AddNode(atlas::procedural::MaterialNodeType::SolidColor);
    graph.SetNodeProperty(solidId, "width", "8");
    graph.SetNodeProperty(solidId, "height", "8");
    graph.SetNodeProperty(solidId, "r", "0.0");
    graph.SetNodeProperty(solidId, "g", "1.0");
    graph.SetNodeProperty(solidId, "b", "0.0");

    auto outId = graph.AddNode(atlas::procedural::MaterialNodeType::Output);
    graph.AddEdge({solidId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->width == 8);
    assert(output->height == 8);
    // Should be green
    assert(std::fabs(output->albedo[0] - 0.0f) < 0.001f);
    assert(std::fabs(output->albedo[1] - 1.0f) < 0.001f);
    assert(std::fabs(output->albedo[2] - 0.0f) < 0.001f);
    std::cout << "[PASS] test_material_graph_execute_solid" << std::endl;
}

void test_material_graph_blend_pipeline() {
    atlas::procedural::ProceduralMaterialGraph graph;

    auto solid1 = graph.AddNode(atlas::procedural::MaterialNodeType::SolidColor);
    graph.SetNodeProperty(solid1, "width", "4");
    graph.SetNodeProperty(solid1, "height", "4");
    graph.SetNodeProperty(solid1, "r", "1.0");
    graph.SetNodeProperty(solid1, "g", "0.0");
    graph.SetNodeProperty(solid1, "b", "0.0");

    auto solid2 = graph.AddNode(atlas::procedural::MaterialNodeType::SolidColor);
    graph.SetNodeProperty(solid2, "width", "4");
    graph.SetNodeProperty(solid2, "height", "4");
    graph.SetNodeProperty(solid2, "r", "0.0");
    graph.SetNodeProperty(solid2, "g", "0.0");
    graph.SetNodeProperty(solid2, "b", "1.0");

    auto blendId = graph.AddNode(atlas::procedural::MaterialNodeType::Blend);
    graph.SetNodeProperty(blendId, "factor", "0.5");

    auto outId = graph.AddNode(atlas::procedural::MaterialNodeType::Output);

    graph.AddEdge({solid1, 0, blendId, 0});
    graph.AddEdge({solid2, 0, blendId, 1});
    graph.AddEdge({blendId, 0, outId, 0});

    assert(graph.Compile());
    assert(graph.Execute());

    auto* output = graph.GetOutput();
    assert(output != nullptr);
    assert(output->IsValid());
    assert(output->PixelCount() == 16);
    // 50/50 red+blue => (0.5, 0, 0.5)
    assert(std::fabs(output->albedo[0] - 0.5f) < 0.001f);
    assert(std::fabs(output->albedo[1] - 0.0f) < 0.001f);
    assert(std::fabs(output->albedo[2] - 0.5f) < 0.001f);
    std::cout << "[PASS] test_material_graph_blend_pipeline" << std::endl;
}

void test_material_graph_node_count() {
    atlas::procedural::ProceduralMaterialGraph graph;
    assert(graph.NodeCount() == 0);
    graph.AddNode(atlas::procedural::MaterialNodeType::SolidColor);
    assert(graph.NodeCount() == 1);
    graph.AddNode(atlas::procedural::MaterialNodeType::Noise);
    assert(graph.NodeCount() == 2);
    graph.AddNode(atlas::procedural::MaterialNodeType::Output);
    assert(graph.NodeCount() == 3);
    std::cout << "[PASS] test_material_graph_node_count" << std::endl;
}

void test_material_graph_remove_node() {
    atlas::procedural::ProceduralMaterialGraph graph;
    auto id1 = graph.AddNode(atlas::procedural::MaterialNodeType::SolidColor);
    auto id2 = graph.AddNode(atlas::procedural::MaterialNodeType::Output);
    graph.AddEdge({id1, 0, id2, 0});
    assert(graph.NodeCount() == 2);

    graph.RemoveNode(id1);
    assert(graph.NodeCount() == 1);

    // Graph should still compile with remaining node
    assert(graph.Compile());
    std::cout << "[PASS] test_material_graph_remove_node" << std::endl;
}

void test_procedural_texture_generation() {
    auto mat = atlas::procedural::GenerateProceduralTexture(16, 16, 42, 0.05f, 4, 0.3f);
    assert(mat.IsValid());
    assert(mat.PixelCount() == 256);
    assert(mat.albedo.size() == 256 * 4);
    assert(mat.roughness.size() == 256);

    // All color values should be in [0, 1]
    for (size_t i = 0; i < mat.albedo.size(); ++i) {
        assert(mat.albedo[i] >= 0.0f && mat.albedo[i] <= 1.0f);
    }
    std::cout << "[PASS] test_procedural_texture_generation" << std::endl;
}

void test_procedural_texture_deterministic() {
    auto mat1 = atlas::procedural::GenerateProceduralTexture(8, 8, 42, 0.05f, 4, 0.3f);
    auto mat2 = atlas::procedural::GenerateProceduralTexture(8, 8, 42, 0.05f, 4, 0.3f);
    assert(mat1.IsValid());
    assert(mat2.IsValid());
    // Same seed should produce same results
    assert(mat1.albedo == mat2.albedo);
    assert(mat1.roughness == mat2.roughness);

    // Different seed should produce different results
    auto mat3 = atlas::procedural::GenerateProceduralTexture(8, 8, 99, 0.05f, 4, 0.3f);
    assert(mat1.albedo != mat3.albedo);
    std::cout << "[PASS] test_procedural_texture_deterministic" << std::endl;
}

void test_procedural_texture_non_repeating() {
    auto mat1 = atlas::procedural::GenerateProceduralTexture(16, 16, 100, 0.05f, 4, 0.5f);
    auto mat2 = atlas::procedural::GenerateProceduralTexture(16, 16, 200, 0.05f, 4, 0.5f);
    assert(mat1.IsValid());
    assert(mat2.IsValid());
    // Different seeds should produce visually different textures
    assert(mat1.albedo != mat2.albedo);
    std::cout << "[PASS] test_procedural_texture_non_repeating" << std::endl;
}
