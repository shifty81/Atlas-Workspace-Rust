// S154 editor tests: ParticleSystemEditorV1, MaterialNodeEditorV1, TextureViewerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TextureViewerV1.h"
#include "NF/Editor/MaterialNodeEditorV1.h"
#include "NF/Editor/ParticleSystemEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ParticleSystemEditorV1 ────────────────────────────────────────────────────

TEST_CASE("Psv1EmitterShape names", "[Editor][S154]") {
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Point))  == "Point");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Sphere)) == "Sphere");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Box))    == "Box");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Cone))   == "Cone");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Disc))   == "Disc");
    REQUIRE(std::string(psv1EmitterShapeName(Psv1EmitterShape::Mesh))   == "Mesh");
}

TEST_CASE("Psv1Emitter validity", "[Editor][S154]") {
    Psv1Emitter e;
    REQUIRE(!e.isValid());
    e.id = 1;
    REQUIRE(e.isValid());
}

TEST_CASE("ParticleSystemEditorV1 addEmitter and emitterCount", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 1;
    REQUIRE(ps.addEmitter(em));
    REQUIRE(ps.emitterCount() == 1);
}

TEST_CASE("ParticleSystemEditorV1 reject duplicate emitter", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 2;
    REQUIRE(ps.addEmitter(em));
    REQUIRE(!ps.addEmitter(em));
}

TEST_CASE("ParticleSystemEditorV1 removeEmitter", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 3;
    ps.addEmitter(em);
    REQUIRE(ps.removeEmitter(3));
    REQUIRE(ps.emitterCount() == 0);
    REQUIRE(!ps.removeEmitter(3));
}

TEST_CASE("ParticleSystemEditorV1 startEmitter sets active", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 4;
    ps.addEmitter(em);
    REQUIRE(!ps.activeCount());
    REQUIRE(ps.startEmitter(4));
    REQUIRE(ps.activeCount() == 1);
}

TEST_CASE("ParticleSystemEditorV1 stopEmitter clears active", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 5;
    ps.addEmitter(em);
    ps.startEmitter(5);
    REQUIRE(ps.stopEmitter(5));
    REQUIRE(ps.activeCount() == 0);
}

TEST_CASE("ParticleSystemEditorV1 resetEmitter", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 6;
    ps.addEmitter(em);
    ps.startEmitter(6);
    REQUIRE(ps.resetEmitter(6));
    REQUIRE(ps.activeCount() == 0);
}

TEST_CASE("ParticleSystemEditorV1 simulate and onEmit callback", "[Editor][S154]") {
    ParticleSystemEditorV1 ps;
    Psv1Emitter em; em.id = 7;
    ps.addEmitter(em);
    uint32_t fired = 0;
    ps.setOnEmit([&](uint32_t id){ fired = id; });
    ps.startEmitter(7);
    REQUIRE(fired == 7);
    ps.simulate(16.f);
    ps.simulate(16.f);
}

// ── MaterialNodeEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("MnvNode validity and findPort", "[Editor][S154]") {
    MnvNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Tex";
    REQUIRE(n.isValid());
    MnvPort p; p.id = 1; p.name = "Color";
    n.ports.push_back(p);
    REQUIRE(n.findPort("Color") != nullptr);
    REQUIRE(n.findPort("Alpha") == nullptr);
}

TEST_CASE("MnvEdge validity", "[Editor][S154]") {
    MnvEdge e;
    REQUIRE(!e.isValid());
    e.id = 1; e.fromNode = 1; e.toNode = 2;
    REQUIRE(e.isValid());
}

TEST_CASE("MaterialNodeEditorV1 addNode and nodeCount", "[Editor][S154]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 1; n.name = "Color";
    REQUIRE(mne.addNode(n));
    REQUIRE(mne.nodeCount() == 1);
}

TEST_CASE("MaterialNodeEditorV1 removeNode", "[Editor][S154]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 2; n.name = "Blend";
    mne.addNode(n);
    REQUIRE(mne.removeNode(2));
    REQUIRE(mne.nodeCount() == 0);
}

TEST_CASE("MaterialNodeEditorV1 addEdge and edgeCount", "[Editor][S154]") {
    MaterialNodeEditorV1 mne;
    MnvEdge e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    REQUIRE(mne.addEdge(e));
    REQUIRE(mne.edgeCount() == 1);
}

TEST_CASE("MaterialNodeEditorV1 removeEdge", "[Editor][S154]") {
    MaterialNodeEditorV1 mne;
    MnvEdge e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    mne.addEdge(e);
    REQUIRE(mne.removeEdge(1));
    REQUIRE(mne.edgeCount() == 0);
}

TEST_CASE("MaterialNodeEditorV1 validate no output returns false", "[Editor][S154]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 1; n.name = "Color"; n.type = MnvNodeType::Color;
    mne.addNode(n);
    REQUIRE(!mne.validate());
}

TEST_CASE("MaterialNodeEditorV1 validate with output node returns true", "[Editor][S154]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 1; n.name = "Out"; n.type = MnvNodeType::Output;
    mne.addNode(n);
    REQUIRE(mne.validate());
}

// ── TextureViewerV1 ───────────────────────────────────────────────────────────

TEST_CASE("TvvTextureInfo validity and estimatedBytes", "[Editor][S154]") {
    TvvTextureInfo t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Diffuse"; t.width = 512; t.height = 512;
    REQUIRE(t.isValid());
    REQUIRE(t.estimatedBytes() == 512 * 512 * 4);
}

TEST_CASE("TextureViewerV1 loadTexture and textureCount", "[Editor][S154]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 1; t.name = "Albedo"; t.width = 256; t.height = 256;
    REQUIRE(tv.loadTexture(t));
    REQUIRE(tv.textureCount() == 1);
}

TEST_CASE("TextureViewerV1 reject invalid texture", "[Editor][S154]") {
    TextureViewerV1 tv;
    TvvTextureInfo bad;
    REQUIRE(!tv.loadTexture(bad));
}

TEST_CASE("TextureViewerV1 unloadTexture", "[Editor][S154]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 1; t.name = "N"; t.width = 64; t.height = 64;
    tv.loadTexture(t);
    REQUIRE(tv.unloadTexture(1));
    REQUIRE(tv.textureCount() == 0);
    REQUIRE(!tv.unloadTexture(1));
}

TEST_CASE("TextureViewerV1 currentTexture set on first load", "[Editor][S154]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 1; t.name = "T"; t.width = 128; t.height = 128;
    tv.loadTexture(t);
    REQUIRE(tv.currentTexture() != nullptr);
    REQUIRE(tv.currentTexture()->id == 1);
}

TEST_CASE("TextureViewerV1 findTexture by name", "[Editor][S154]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 2; t.name = "Normal"; t.width = 512; t.height = 512;
    tv.loadTexture(t);
    REQUIRE(tv.findTexture("Normal") != nullptr);
    REQUIRE(tv.findTexture("Unknown") == nullptr);
}

TEST_CASE("TextureViewerV1 setZoom clamps to minimum", "[Editor][S154]") {
    TextureViewerV1 tv;
    tv.setZoom(0.f);
    REQUIRE(tv.getZoom() == Approx(0.01f));
    tv.setZoom(4.f);
    REQUIRE(tv.getZoom() == Approx(4.f));
}

TEST_CASE("TextureViewerV1 setMipLevel and setMipFilter", "[Editor][S154]") {
    TextureViewerV1 tv;
    tv.setMipLevel(3);
    REQUIRE(tv.getMipLevel() == 3);
    tv.setMipFilter(TvvMipFilter::Trilinear);
    REQUIRE(tv.getMipFilter() == TvvMipFilter::Trilinear);
}
