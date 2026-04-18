// S152 editor tests: ShaderEditorV1, RenderPassEditorV1, PipelineStateEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PipelineStateEditorV1.h"
#include "NF/Editor/RenderPassEditorV1.h"
#include "NF/Editor/ShaderEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ShaderEditorV1 ────────────────────────────────────────────────────────────

TEST_CASE("SevUniform validity", "[Editor][S152]") {
    SevUniform u;
    REQUIRE(!u.isValid());
    u.id = 1; u.name = "uColor";
    REQUIRE(u.isValid());
}

TEST_CASE("SevShader validity and uniformCount", "[Editor][S152]") {
    SevShader s;
    REQUIRE(!s.isValid());
    s.id = 1;
    REQUIRE(s.isValid());
    REQUIRE(s.uniformCount() == 0);
    SevUniform u; u.id = 1; u.name = "uMVP";
    s.uniforms.push_back(u);
    REQUIRE(s.uniformCount() == 1);
}

TEST_CASE("ShaderEditorV1 addShader and shaderCount", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 1; s.stage = SevShaderStage::Vertex;
    REQUIRE(se.addShader(s));
    REQUIRE(se.shaderCount() == 1);
}

TEST_CASE("ShaderEditorV1 reject duplicate shader", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 2; s.stage = SevShaderStage::Fragment;
    REQUIRE(se.addShader(s));
    REQUIRE(!se.addShader(s));
}

TEST_CASE("ShaderEditorV1 removeShader", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 3; s.stage = SevShaderStage::Compute;
    se.addShader(s);
    REQUIRE(se.removeShader(3));
    REQUIRE(se.shaderCount() == 0);
    REQUIRE(!se.removeShader(3));
}

TEST_CASE("ShaderEditorV1 compile with source succeeds", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 4; s.source = "void main() {}";
    se.addShader(s);
    REQUIRE(se.compile(4));
    REQUIRE(se.findById(4)->compiled);
}

TEST_CASE("ShaderEditorV1 compile empty source fails", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 5;
    se.addShader(s);
    REQUIRE(!se.compile(5));
}

TEST_CASE("ShaderEditorV1 allCompiled", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s1; s1.id = 1; s1.source = "vs";
    SevShader s2; s2.id = 2; s2.source = "fs";
    se.addShader(s1); se.addShader(s2);
    REQUIRE(!se.allCompiled());
    se.compile(1); se.compile(2);
    REQUIRE(se.allCompiled());
}

TEST_CASE("ShaderEditorV1 compile callback", "[Editor][S152]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 6; s.source = "main";
    se.addShader(s);
    uint32_t cbId = 0; bool cbResult = false;
    se.setOnCompile([&](uint32_t id, bool ok){ cbId = id; cbResult = ok; });
    se.compile(6);
    REQUIRE(cbId == 6);
    REQUIRE(cbResult);
}

// ── RenderPassEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("RpvAttachment validity", "[Editor][S152]") {
    RpvAttachment a;
    REQUIRE(!a.isValid());
    a.id = 1;
    REQUIRE(a.isValid());
}

TEST_CASE("RenderPassEditorV1 addAttachment and attachmentCount", "[Editor][S152]") {
    RenderPassEditorV1 rpe;
    RpvAttachment att; att.id = 1; att.type = RpvAttachmentType::Color;
    REQUIRE(rpe.addAttachment(att));
    REQUIRE(rpe.attachmentCount() == 1);
}

TEST_CASE("RenderPassEditorV1 removeAttachment", "[Editor][S152]") {
    RenderPassEditorV1 rpe;
    RpvAttachment att; att.id = 2; att.type = RpvAttachmentType::Depth;
    rpe.addAttachment(att);
    REQUIRE(rpe.removeAttachment(2));
    REQUIRE(rpe.attachmentCount() == 0);
}

TEST_CASE("RenderPassEditorV1 addPass and passCount", "[Editor][S152]") {
    RenderPassEditorV1 rpe;
    RpvPass pass; pass.id = 1; pass.name = "ShadowPass";
    REQUIRE(rpe.addPass(pass));
    REQUIRE(rpe.passCount() == 1);
}

TEST_CASE("RenderPassEditorV1 removePass", "[Editor][S152]") {
    RenderPassEditorV1 rpe;
    RpvPass pass; pass.id = 2; pass.name = "GBuffer";
    rpe.addPass(pass);
    REQUIRE(rpe.removePass(2));
    REQUIRE(rpe.passCount() == 0);
    REQUIRE(!rpe.removePass(2));
}

TEST_CASE("RenderPassEditorV1 validate passes with valid attachments", "[Editor][S152]") {
    RenderPassEditorV1 rpe;
    RpvAttachment att; att.id = 1; att.type = RpvAttachmentType::Color;
    rpe.addAttachment(att);
    RpvPass pass; pass.id = 1; pass.name = "Main"; pass.attachmentIds = {1};
    rpe.addPass(pass);
    REQUIRE(rpe.validate());
}

TEST_CASE("RenderPassEditorV1 validate fails with missing attachment", "[Editor][S152]") {
    RenderPassEditorV1 rpe;
    RpvPass pass; pass.id = 1; pass.name = "Main"; pass.attachmentIds = {99};
    rpe.addPass(pass);
    REQUIRE(!rpe.validate());
}

// ── PipelineStateEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("PsvPipelineState validity", "[Editor][S152]") {
    PsvPipelineState ps;
    REQUIRE(!ps.isValid());
    ps.id = 1; ps.name = "Default";
    REQUIRE(ps.isValid());
}

TEST_CASE("PipelineStateEditorV1 addState and stateCount", "[Editor][S152]") {
    PipelineStateEditorV1 pse;
    PsvPipelineState ps; ps.id = 1; ps.name = "Opaque";
    REQUIRE(pse.addState(ps));
    REQUIRE(pse.stateCount() == 1);
}

TEST_CASE("PipelineStateEditorV1 removeState", "[Editor][S152]") {
    PipelineStateEditorV1 pse;
    PsvPipelineState ps; ps.id = 2; ps.name = "Wire";
    pse.addState(ps);
    REQUIRE(pse.removeState(2));
    REQUIRE(pse.stateCount() == 0);
}

TEST_CASE("PipelineStateEditorV1 cloneState", "[Editor][S152]") {
    PipelineStateEditorV1 pse;
    PsvPipelineState ps; ps.id = 1; ps.name = "Base"; ps.blend = PsvBlendMode::Translucent;
    pse.addState(ps);
    REQUIRE(pse.cloneState(1, 2));
    REQUIRE(pse.stateCount() == 2);
    auto* cloned = pse.findState("Base_copy");
    REQUIRE(cloned != nullptr);
    REQUIRE(cloned->blend == PsvBlendMode::Translucent);
}

TEST_CASE("PipelineStateEditorV1 setDefault and getDefault", "[Editor][S152]") {
    PipelineStateEditorV1 pse;
    PsvPipelineState ps; ps.id = 1; ps.name = "DefaultState";
    pse.addState(ps);
    REQUIRE(pse.setDefault(1));
    REQUIRE(pse.getDefault() != nullptr);
    REQUIRE(pse.getDefault()->id == 1);
}

TEST_CASE("PipelineStateEditorV1 removeState clears default", "[Editor][S152]") {
    PipelineStateEditorV1 pse;
    PsvPipelineState ps; ps.id = 1; ps.name = "D";
    pse.addState(ps);
    pse.setDefault(1);
    pse.removeState(1);
    REQUIRE(pse.getDefault() == nullptr);
}
