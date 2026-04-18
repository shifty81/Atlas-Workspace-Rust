#include "../engine/render/AtlasShaderIR.h"
#include <iostream>
#include <cassert>

void test_shader_module_defaults() {
    atlas::render::ShaderModule mod;
    assert(atlas::render::ShaderModule::MAGIC == 0x53484452);
    assert(atlas::render::ShaderModule::VERSION == 1);
    assert(mod.stage == atlas::render::ShaderStage::Vertex);
    assert(mod.name.empty());
    assert(mod.instructions.empty());
    std::cout << "[PASS] test_shader_module_defaults" << std::endl;
}

void test_shader_passthrough_vertex() {
    auto mod = atlas::render::ShaderIRCompiler::CreatePassthroughVertex();
    assert(mod.stage == atlas::render::ShaderStage::Vertex);
    assert(!mod.instructions.empty());
    assert(!mod.inputs.empty());
    assert(!mod.outputs.empty());
    std::cout << "[PASS] test_shader_passthrough_vertex" << std::endl;
}

void test_shader_solid_color_fragment() {
    auto mod = atlas::render::ShaderIRCompiler::CreateSolidColorFragment(1.0f, 0.0f, 0.0f, 1.0f);
    assert(mod.stage == atlas::render::ShaderStage::Fragment);
    assert(!mod.instructions.empty());
    std::cout << "[PASS] test_shader_solid_color_fragment" << std::endl;
}

void test_shader_validate_valid() {
    auto mod = atlas::render::ShaderIRCompiler::CreatePassthroughVertex();
    assert(mod.Validate());
    std::cout << "[PASS] test_shader_validate_valid" << std::endl;
}

void test_shader_validate_empty() {
    atlas::render::ShaderModule mod;
    // An empty module may or may not be valid; just ensure it doesn't crash
    mod.Validate();
    std::cout << "[PASS] test_shader_validate_empty" << std::endl;
}

void test_shader_hash_deterministic() {
    auto a = atlas::render::ShaderIRCompiler::CreatePassthroughVertex();
    auto b = atlas::render::ShaderIRCompiler::CreatePassthroughVertex();
    assert(a.Hash() == b.Hash());
    std::cout << "[PASS] test_shader_hash_deterministic" << std::endl;
}

void test_shader_hash_different() {
    auto vert = atlas::render::ShaderIRCompiler::CreatePassthroughVertex();
    auto frag = atlas::render::ShaderIRCompiler::CreateSolidColorFragment(1.0f, 0.0f, 0.0f, 1.0f);
    assert(vert.Hash() != frag.Hash());
    std::cout << "[PASS] test_shader_hash_different" << std::endl;
}

void test_shader_serialize_roundtrip() {
    auto original = atlas::render::ShaderIRCompiler::CreatePassthroughVertex();
    auto data = original.Serialize();
    assert(!data.empty());
    auto restored = atlas::render::ShaderModule::Deserialize(data);
    assert(restored.stage == original.stage);
    assert(restored.name == original.name);
    assert(restored.inputs.size() == original.inputs.size());
    assert(restored.outputs.size() == original.outputs.size());
    assert(restored.instructions.size() == original.instructions.size());
    assert(restored.Hash() == original.Hash());
    std::cout << "[PASS] test_shader_serialize_roundtrip" << std::endl;
}
