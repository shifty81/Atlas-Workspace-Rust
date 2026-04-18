// S194 editor tests: MaterialNodeEditorV1, TextureViewerV1, RenderPassEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/MaterialNodeEditorV1.h"
#include "NF/Editor/TextureViewerV1.h"
#include "NF/Editor/RenderPassEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── MaterialNodeEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("MnvNode validity", "[Editor][S194]") {
    MnvNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Color";
    REQUIRE(n.isValid());
}

TEST_CASE("MaterialNodeEditorV1 addNode and nodeCount", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    REQUIRE(mne.nodeCount() == 0);
    MnvNode n; n.id = 1; n.name = "N1";
    REQUIRE(mne.addNode(n));
    REQUIRE(mne.nodeCount() == 1);
}

TEST_CASE("MaterialNodeEditorV1 addNode invalid fails", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    REQUIRE(!mne.addNode(MnvNode{}));
}

TEST_CASE("MaterialNodeEditorV1 addNode duplicate fails", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 1; n.name = "A";
    mne.addNode(n);
    REQUIRE(!mne.addNode(n));
}

TEST_CASE("MaterialNodeEditorV1 removeNode", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 2; n.name = "B";
    mne.addNode(n);
    REQUIRE(mne.removeNode(2));
    REQUIRE(mne.nodeCount() == 0);
    REQUIRE(!mne.removeNode(2));
}

TEST_CASE("MaterialNodeEditorV1 addEdge and edgeCount", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    MnvEdge e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    REQUIRE(mne.addEdge(e));
    REQUIRE(mne.edgeCount() == 1);
}

TEST_CASE("MaterialNodeEditorV1 addEdge invalid fails", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    REQUIRE(!mne.addEdge(MnvEdge{}));
}

TEST_CASE("MaterialNodeEditorV1 removeEdge", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    MnvEdge e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    mne.addEdge(e);
    REQUIRE(mne.removeEdge(1));
    REQUIRE(mne.edgeCount() == 0);
}

TEST_CASE("MaterialNodeEditorV1 validate false without output node", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 1; n.name = "Color"; n.type = MnvNodeType::Color;
    mne.addNode(n);
    REQUIRE(!mne.validate());
}

TEST_CASE("MaterialNodeEditorV1 validate true with output node", "[Editor][S194]") {
    MaterialNodeEditorV1 mne;
    MnvNode n; n.id = 1; n.name = "Out"; n.type = MnvNodeType::Output;
    mne.addNode(n);
    REQUIRE(mne.validate());
}

TEST_CASE("MnvNode findPort by name", "[Editor][S194]") {
    MnvNode n; n.id = 1; n.name = "X";
    MnvPort p; p.id = 10; p.name = "BaseColor";
    n.ports.push_back(p);
    REQUIRE(n.findPort("BaseColor") != nullptr);
    REQUIRE(n.findPort("Roughness") == nullptr);
}

TEST_CASE("MnvEdge validity", "[Editor][S194]") {
    MnvEdge e;
    REQUIRE(!e.isValid());
    e.id = 1; e.fromNode = 1; e.toNode = 2;
    REQUIRE(e.isValid());
}

TEST_CASE("MnvNode default type is Color", "[Editor][S194]") {
    MnvNode n;
    REQUIRE(n.type == MnvNodeType::Color);
}

// ── TextureViewerV1 ──────────────────────────────────────────────────────────

TEST_CASE("TvvTextureInfo validity", "[Editor][S194]") {
    TvvTextureInfo t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Albedo"; t.width = 512; t.height = 512;
    REQUIRE(t.isValid());
}

TEST_CASE("TextureViewerV1 loadTexture and textureCount", "[Editor][S194]") {
    TextureViewerV1 tv;
    REQUIRE(tv.textureCount() == 0);
    TvvTextureInfo t; t.id = 1; t.name = "T1"; t.width = 256; t.height = 256;
    REQUIRE(tv.loadTexture(t));
    REQUIRE(tv.textureCount() == 1);
}

TEST_CASE("TextureViewerV1 loadTexture invalid fails", "[Editor][S194]") {
    TextureViewerV1 tv;
    REQUIRE(!tv.loadTexture(TvvTextureInfo{}));
}

TEST_CASE("TextureViewerV1 loadTexture duplicate fails", "[Editor][S194]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 1; t.name = "A"; t.width = 64; t.height = 64;
    tv.loadTexture(t);
    REQUIRE(!tv.loadTexture(t));
}

TEST_CASE("TextureViewerV1 unloadTexture", "[Editor][S194]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 2; t.name = "B"; t.width = 128; t.height = 128;
    tv.loadTexture(t);
    REQUIRE(tv.unloadTexture(2));
    REQUIRE(tv.textureCount() == 0);
    REQUIRE(!tv.unloadTexture(2));
}

TEST_CASE("TextureViewerV1 first loaded becomes current", "[Editor][S194]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 1; t.name = "X"; t.width = 32; t.height = 32;
    tv.loadTexture(t);
    REQUIRE(tv.currentTexture() != nullptr);
    REQUIRE(tv.currentTexture()->id == 1);
}

TEST_CASE("TextureViewerV1 findTexture by name", "[Editor][S194]") {
    TextureViewerV1 tv;
    TvvTextureInfo t; t.id = 1; t.name = "Normal"; t.width = 512; t.height = 512;
    tv.loadTexture(t);
    REQUIRE(tv.findTexture("Normal") != nullptr);
    REQUIRE(tv.findTexture("Missing") == nullptr);
}

TEST_CASE("TextureViewerV1 setZoom clamps to positive", "[Editor][S194]") {
    TextureViewerV1 tv;
    tv.setZoom(2.f); REQUIRE(tv.getZoom() == Approx(2.f));
    tv.setZoom(-1.f); REQUIRE(tv.getZoom() > 0.f);
}

TEST_CASE("TextureViewerV1 setMipLevel", "[Editor][S194]") {
    TextureViewerV1 tv;
    tv.setMipLevel(3);
    REQUIRE(tv.getMipLevel() == 3);
}

TEST_CASE("TextureViewerV1 setMipFilter", "[Editor][S194]") {
    TextureViewerV1 tv;
    tv.setMipFilter(TvvMipFilter::Trilinear);
    REQUIRE(tv.getMipFilter() == TvvMipFilter::Trilinear);
}

TEST_CASE("TvvTextureInfo estimatedBytes RGBA8", "[Editor][S194]") {
    TvvTextureInfo t; t.id = 1; t.name = "X"; t.width = 8; t.height = 8; t.format = TvvFormat::RGBA8;
    REQUIRE(t.estimatedBytes() == 256);
}

TEST_CASE("TvvTextureInfo estimatedBytes R8", "[Editor][S194]") {
    TvvTextureInfo t; t.id = 1; t.name = "X"; t.width = 4; t.height = 4; t.format = TvvFormat::R8;
    REQUIRE(t.estimatedBytes() == 16);
}

// ── RenderPassEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("RpvAttachment validity", "[Editor][S194]") {
    RpvAttachment a;
    REQUIRE(!a.isValid());
    a.id = 1;
    REQUIRE(a.isValid());
}

TEST_CASE("RenderPassEditorV1 addAttachment and attachmentCount", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    REQUIRE(rpe.attachmentCount() == 0);
    RpvAttachment a; a.id = 1;
    REQUIRE(rpe.addAttachment(a));
    REQUIRE(rpe.attachmentCount() == 1);
}

TEST_CASE("RenderPassEditorV1 addAttachment invalid fails", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    REQUIRE(!rpe.addAttachment(RpvAttachment{}));
}

TEST_CASE("RenderPassEditorV1 addAttachment duplicate fails", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvAttachment a; a.id = 1;
    rpe.addAttachment(a);
    REQUIRE(!rpe.addAttachment(a));
}

TEST_CASE("RenderPassEditorV1 removeAttachment", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvAttachment a; a.id = 2;
    rpe.addAttachment(a);
    REQUIRE(rpe.removeAttachment(2));
    REQUIRE(rpe.attachmentCount() == 0);
}

TEST_CASE("RpvPass validity", "[Editor][S194]") {
    RpvPass p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "GBuffer";
    REQUIRE(p.isValid());
}

TEST_CASE("RenderPassEditorV1 addPass and passCount", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvPass p; p.id = 1; p.name = "Main";
    REQUIRE(rpe.addPass(p));
    REQUIRE(rpe.passCount() == 1);
}

TEST_CASE("RenderPassEditorV1 addPass invalid fails", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    REQUIRE(!rpe.addPass(RpvPass{}));
}

TEST_CASE("RenderPassEditorV1 removePass", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvPass p; p.id = 1; p.name = "Shadow";
    rpe.addPass(p);
    REQUIRE(rpe.removePass(1));
    REQUIRE(rpe.passCount() == 0);
}

TEST_CASE("RenderPassEditorV1 validate pass with valid attachment", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvAttachment a; a.id = 10;
    rpe.addAttachment(a);
    RpvPass p; p.id = 1; p.name = "GBuf";
    p.attachmentIds.push_back(10);
    rpe.addPass(p);
    REQUIRE(rpe.validate());
}

TEST_CASE("RenderPassEditorV1 validate fails with missing attachment", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvPass p; p.id = 1; p.name = "GBuf";
    p.attachmentIds.push_back(99);
    rpe.addPass(p);
    REQUIRE(!rpe.validate());
}

TEST_CASE("RenderPassEditorV1 validate passes with no attachment refs", "[Editor][S194]") {
    RenderPassEditorV1 rpe;
    RpvPass p; p.id = 1; p.name = "Empty";
    rpe.addPass(p);
    REQUIRE(rpe.validate());
}
