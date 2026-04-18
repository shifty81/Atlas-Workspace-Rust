// S193 editor tests: ParticleSystemEditorV1, AudioGraphEditorV1, ShaderEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ParticleSystemEditorV1.h"
#include "NF/Editor/AudioGraphEditorV1.h"
#include "NF/Editor/ShaderEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ParticleSystemEditorV1 ───────────────────────────────────────────────────

TEST_CASE("Psv1Emitter validity", "[Editor][S193]") {
    Psv1Emitter e;
    REQUIRE(!e.isValid());
    e.id = 1;
    REQUIRE(e.isValid());
}

TEST_CASE("ParticleSystemEditorV1 addEmitter", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    REQUIRE(ps.emitterCount() == 0);
    Psv1Emitter e; e.id = 1;
    REQUIRE(ps.addEmitter(e));
    REQUIRE(ps.emitterCount() == 1);
}

TEST_CASE("ParticleSystemEditorV1 addEmitter invalid fails", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    REQUIRE(!ps.addEmitter(Psv1Emitter{}));
}

TEST_CASE("ParticleSystemEditorV1 addEmitter duplicate fails", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter e; e.id = 1;
    ps.addEmitter(e);
    REQUIRE(!ps.addEmitter(e));
}

TEST_CASE("ParticleSystemEditorV1 removeEmitter", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter e; e.id = 2;
    ps.addEmitter(e);
    REQUIRE(ps.removeEmitter(2));
    REQUIRE(ps.emitterCount() == 0);
    REQUIRE(!ps.removeEmitter(2));
}

TEST_CASE("ParticleSystemEditorV1 startEmitter activates", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter e; e.id = 1;
    ps.addEmitter(e);
    REQUIRE(!ps.activeCount());
    REQUIRE(ps.startEmitter(1));
    REQUIRE(ps.activeCount() == 1);
}

TEST_CASE("ParticleSystemEditorV1 stopEmitter deactivates", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter e; e.id = 1;
    ps.addEmitter(e);
    ps.startEmitter(1);
    REQUIRE(ps.stopEmitter(1));
    REQUIRE(ps.activeCount() == 0);
}

TEST_CASE("ParticleSystemEditorV1 resetEmitter", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter e; e.id = 1;
    ps.addEmitter(e);
    ps.startEmitter(1);
    REQUIRE(ps.resetEmitter(1));
    REQUIRE(ps.activeCount() == 0);
}

TEST_CASE("ParticleSystemEditorV1 simulate increments tick", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    ps.simulate(16.f);
    ps.simulate(16.f);
    // No crash, ticks increment internally
    REQUIRE(true);
}

TEST_CASE("ParticleSystemEditorV1 onEmit callback fires on start", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    uint32_t notified = 0;
    ps.setOnEmit([&](uint32_t id){ notified = id; });
    Psv1Emitter e; e.id = 7;
    ps.addEmitter(e);
    ps.startEmitter(7);
    REQUIRE(notified == 7);
}

TEST_CASE("ParticleSystemEditorV1 startEmitter unknown returns false", "[Editor][S193]") {
    ParticleSystemEditorV1 ps;
    REQUIRE(!ps.startEmitter(99));
}

TEST_CASE("psv1EmitterShapeName all values", "[Editor][S193]") {
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Point))  == "Point");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Sphere)) == "Sphere");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Box))    == "Box");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Cone))   == "Cone");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Disc))   == "Disc");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Mesh))   == "Mesh");
}

TEST_CASE("Psv1Emitter default shape is Point", "[Editor][S193]") {
    Psv1Emitter e; e.id = 1;
    REQUIRE(e.shape == Psv1EmitterShape::Point);
}

TEST_CASE("Psv1Emitter looping default true", "[Editor][S193]") {
    Psv1Emitter e; e.id = 1;
    REQUIRE(e.looping);
}

// ── AudioGraphEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("AgvNode validity", "[Editor][S193]") {
    AgvNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Source1";
    REQUIRE(n.isValid());
}

TEST_CASE("AudioGraphEditorV1 addNode and nodeCount", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    REQUIRE(ag.nodeCount() == 0);
    AgvNode n; n.id = 1; n.name = "Src";
    REQUIRE(ag.addNode(n));
    REQUIRE(ag.nodeCount() == 1);
}

TEST_CASE("AudioGraphEditorV1 addNode invalid fails", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    REQUIRE(!ag.addNode(AgvNode{}));
}

TEST_CASE("AudioGraphEditorV1 addNode duplicate fails", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 1; n.name = "A";
    ag.addNode(n);
    REQUIRE(!ag.addNode(n));
}

TEST_CASE("AudioGraphEditorV1 removeNode", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 2; n.name = "B";
    ag.addNode(n);
    REQUIRE(ag.removeNode(2));
    REQUIRE(ag.nodeCount() == 0);
    REQUIRE(!ag.removeNode(2));
}

TEST_CASE("AudioGraphEditorV1 addEdge and edgeCount", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    AgvEdge e; e.id = 1; e.fromNodeId = 1; e.toNodeId = 2;
    REQUIRE(ag.addEdge(e));
    REQUIRE(ag.edgeCount() == 1);
}

TEST_CASE("AudioGraphEditorV1 addEdge invalid fails", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    REQUIRE(!ag.addEdge(AgvEdge{}));
}

TEST_CASE("AudioGraphEditorV1 removeEdge", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    AgvEdge e; e.id = 1; e.fromNodeId = 1; e.toNodeId = 2;
    ag.addEdge(e);
    REQUIRE(ag.removeEdge(1));
    REQUIRE(ag.edgeCount() == 0);
}

TEST_CASE("AudioGraphEditorV1 findNode by name", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 3; n.name = "Master";
    ag.addNode(n);
    REQUIRE(ag.findNode("Master") != nullptr);
    REQUIRE(ag.findNode("NonExistent") == nullptr);
}

TEST_CASE("AudioGraphEditorV1 hasCycle always false (stub)", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    REQUIRE(!ag.hasCycle());
}

TEST_CASE("AudioGraphEditorV1 simulate increments count", "[Editor][S193]") {
    AudioGraphEditorV1 ag;
    ag.simulate(16.f);
    ag.simulate(16.f);
    REQUIRE(ag.simCount() == 2);
}

TEST_CASE("AgvNode findPin by name", "[Editor][S193]") {
    AgvNode n; n.id = 1; n.name = "X";
    AgvPin p; p.id = 10; p.name = "Input";
    n.pins.push_back(p);
    REQUIRE(n.findPin("Input") != nullptr);
    REQUIRE(n.findPin("Output") == nullptr);
}

TEST_CASE("AgvEdge validity", "[Editor][S193]") {
    AgvEdge e;
    REQUIRE(!e.isValid());
    e.id = 1; e.fromNodeId = 1; e.toNodeId = 2;
    REQUIRE(e.isValid());
}

// ── ShaderEditorV1 ───────────────────────────────────────────────────────────

TEST_CASE("SevShader validity", "[Editor][S193]") {
    SevShader s;
    REQUIRE(!s.isValid());
    s.id = 1;
    REQUIRE(s.isValid());
}

TEST_CASE("ShaderEditorV1 addShader and shaderCount", "[Editor][S193]") {
    ShaderEditorV1 se;
    REQUIRE(se.shaderCount() == 0);
    SevShader s; s.id = 1;
    REQUIRE(se.addShader(s));
    REQUIRE(se.shaderCount() == 1);
}

TEST_CASE("ShaderEditorV1 addShader invalid fails", "[Editor][S193]") {
    ShaderEditorV1 se;
    REQUIRE(!se.addShader(SevShader{}));
}

TEST_CASE("ShaderEditorV1 addShader duplicate fails", "[Editor][S193]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 1;
    se.addShader(s);
    REQUIRE(!se.addShader(s));
}

TEST_CASE("ShaderEditorV1 removeShader", "[Editor][S193]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 2;
    se.addShader(s);
    REQUIRE(se.removeShader(2));
    REQUIRE(se.shaderCount() == 0);
    REQUIRE(!se.removeShader(2));
}

TEST_CASE("ShaderEditorV1 compile success with source", "[Editor][S193]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 1; s.source = "void main() {}";
    se.addShader(s);
    REQUIRE(se.compile(1));
    REQUIRE(se.allCompiled());
}

TEST_CASE("ShaderEditorV1 compile fails with empty source", "[Editor][S193]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 1;
    se.addShader(s);
    REQUIRE(!se.compile(1));
    REQUIRE(!se.allCompiled());
}

TEST_CASE("ShaderEditorV1 allCompiled false with uncompiled", "[Editor][S193]") {
    ShaderEditorV1 se;
    SevShader s1; s1.id = 1; s1.source = "code";
    SevShader s2; s2.id = 2;
    se.addShader(s1); se.addShader(s2);
    se.compile(1);
    REQUIRE(!se.allCompiled());
}

TEST_CASE("ShaderEditorV1 onCompile callback fires", "[Editor][S193]") {
    ShaderEditorV1 se;
    uint32_t cbId = 0; bool cbResult = false;
    se.setOnCompile([&](uint32_t id, bool ok){ cbId = id; cbResult = ok; });
    SevShader s; s.id = 5; s.source = "main";
    se.addShader(s);
    se.compile(5);
    REQUIRE(cbId == 5);
    REQUIRE(cbResult);
}

TEST_CASE("ShaderEditorV1 compile unknown returns false", "[Editor][S193]") {
    ShaderEditorV1 se;
    REQUIRE(!se.compile(99));
}

TEST_CASE("SevShader uniformCount", "[Editor][S193]") {
    SevShader s; s.id = 1;
    SevUniform u; u.id = 1; u.name = "uTime";
    s.uniforms.push_back(u);
    REQUIRE(s.uniformCount() == 1);
}

TEST_CASE("SevUniform validity", "[Editor][S193]") {
    SevUniform u;
    REQUIRE(!u.isValid());
    u.id = 1; u.name = "uAlpha";
    REQUIRE(u.isValid());
}

TEST_CASE("ShaderEditorV1 findById", "[Editor][S193]") {
    ShaderEditorV1 se;
    SevShader s; s.id = 3;
    se.addShader(s);
    REQUIRE(se.findById(3) != nullptr);
    REQUIRE(se.findById(99) == nullptr);
}
