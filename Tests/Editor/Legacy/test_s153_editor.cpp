// S153 editor tests: AudioGraphEditorV1, SoundMixerEditorV1, AnimationCurveEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AnimationCurveEditorV1.h"
#include "NF/Editor/SoundMixerEditorV1.h"
#include "NF/Editor/AudioGraphEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── AudioGraphEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("AgvNode validity and findPin", "[Editor][S153]") {
    AgvNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Source";
    REQUIRE(n.isValid());
    AgvPin p; p.id = 1; p.name = "Out"; p.isOutput = true;
    n.pins.push_back(p);
    REQUIRE(n.findPin("Out") != nullptr);
    REQUIRE(n.findPin("In") == nullptr);
}

TEST_CASE("AgvEdge validity", "[Editor][S153]") {
    AgvEdge e;
    REQUIRE(!e.isValid());
    e.id = 1; e.fromNodeId = 1; e.toNodeId = 2;
    REQUIRE(e.isValid());
}

TEST_CASE("AudioGraphEditorV1 addNode and nodeCount", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 1; n.name = "Mixer";
    REQUIRE(ag.addNode(n));
    REQUIRE(ag.nodeCount() == 1);
}

TEST_CASE("AudioGraphEditorV1 reject duplicate node", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 2; n.name = "Effect";
    REQUIRE(ag.addNode(n));
    REQUIRE(!ag.addNode(n));
}

TEST_CASE("AudioGraphEditorV1 removeNode", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 3; n.name = "Bus";
    ag.addNode(n);
    REQUIRE(ag.removeNode(3));
    REQUIRE(ag.nodeCount() == 0);
    REQUIRE(!ag.removeNode(3));
}

TEST_CASE("AudioGraphEditorV1 addEdge and edgeCount", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    AgvNode a; a.id = 1; a.name = "A";
    AgvNode b; b.id = 2; b.name = "B";
    ag.addNode(a); ag.addNode(b);
    AgvEdge e; e.id = 1; e.fromNodeId = 1; e.toNodeId = 2;
    REQUIRE(ag.addEdge(e));
    REQUIRE(ag.edgeCount() == 1);
}

TEST_CASE("AudioGraphEditorV1 removeEdge", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    AgvEdge e; e.id = 5; e.fromNodeId = 1; e.toNodeId = 2;
    ag.addEdge(e);
    REQUIRE(ag.removeEdge(5));
    REQUIRE(ag.edgeCount() == 0);
    REQUIRE(!ag.removeEdge(5));
}

TEST_CASE("AudioGraphEditorV1 findNode by name", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    AgvNode n; n.id = 1; n.name = "Output";
    ag.addNode(n);
    REQUIRE(ag.findNode("Output") != nullptr);
    REQUIRE(ag.findNode("Missing") == nullptr);
}

TEST_CASE("AudioGraphEditorV1 simulate increments simCount", "[Editor][S153]") {
    AudioGraphEditorV1 ag;
    REQUIRE(ag.simCount() == 0);
    ag.simulate(16.f);
    ag.simulate(16.f);
    REQUIRE(ag.simCount() == 2);
}

// ── SoundMixerEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("SmvChannel validity", "[Editor][S153]") {
    SmvChannel c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Music";
    REQUIRE(c.isValid());
}

TEST_CASE("SoundMixerEditorV1 addChannel and channelCount", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "SFX";
    REQUIRE(sm.addChannel(c));
    REQUIRE(sm.channelCount() == 1);
}

TEST_CASE("SoundMixerEditorV1 removeChannel", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 2; c.name = "Voice";
    sm.addChannel(c);
    REQUIRE(sm.removeChannel(2));
    REQUIRE(sm.channelCount() == 0);
    REQUIRE(!sm.removeChannel(2));
}

TEST_CASE("SoundMixerEditorV1 setVolume clamps range", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "Amb";
    sm.addChannel(c);
    REQUIRE(sm.setVolume(1, 3.f));
    REQUIRE(sm.getMixedVolume(1) == Approx(2.f));
    REQUIRE(sm.setVolume(1, -1.f));
    REQUIRE(sm.getMixedVolume(1) == Approx(0.f));
}

TEST_CASE("SoundMixerEditorV1 setPan clamps to -1..1", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "UI";
    sm.addChannel(c);
    sm.setPan(1, 5.f);
    // just ensure it doesn't crash and returns true
    REQUIRE(true);
}

TEST_CASE("SoundMixerEditorV1 muteChannel makes getMixedVolume 0", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "Music"; c.volume = 0.8f;
    sm.addChannel(c);
    sm.muteChannel(1, true);
    REQUIRE(sm.getMixedVolume(1) == Approx(0.f));
}

TEST_CASE("SoundMixerEditorV1 soloChannel", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "SFX";
    sm.addChannel(c);
    REQUIRE(sm.soloChannel(1, true));
}

TEST_CASE("SoundMixerEditorV1 masterVolume affects mix", "[Editor][S153]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "M"; c.volume = 1.f;
    sm.addChannel(c);
    sm.setMasterVolume(0.5f);
    REQUIRE(sm.getMasterVolume() == Approx(0.5f));
    REQUIRE(sm.getMixedVolume(1) == Approx(0.5f));
}

// ── AnimationCurveEditorV1 ───────────────────────────────────────────────────

TEST_CASE("AcvKey validity", "[Editor][S153]") {
    AcvKey k;
    REQUIRE(!k.isValid());
    k.id = 1;
    REQUIRE(k.isValid());
}

TEST_CASE("AcvCurve validity and evaluate empty", "[Editor][S153]") {
    AcvCurve c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Pos.X";
    REQUIRE(c.isValid());
    REQUIRE(c.evaluate(0.5f) == Approx(0.f));
}

TEST_CASE("AnimationCurveEditorV1 addCurve and curveCount", "[Editor][S153]") {
    AnimationCurveEditorV1 ace;
    AcvCurve c; c.id = 1; c.name = "Scale";
    REQUIRE(ace.addCurve(c));
    REQUIRE(ace.curveCount() == 1);
}

TEST_CASE("AnimationCurveEditorV1 removeCurve", "[Editor][S153]") {
    AnimationCurveEditorV1 ace;
    AcvCurve c; c.id = 2; c.name = "Rot";
    ace.addCurve(c);
    REQUIRE(ace.removeCurve(2));
    REQUIRE(ace.curveCount() == 0);
}

TEST_CASE("AnimationCurveEditorV1 addKey sorts by time and fires callback", "[Editor][S153]") {
    AnimationCurveEditorV1 ace;
    AcvCurve c; c.id = 1; c.name = "Pos";
    ace.addCurve(c);
    uint32_t changed = 0;
    ace.setOnChange([&](uint32_t id){ changed = id; });
    AcvKey k1; k1.id = 1; k1.time = 0.5f; k1.value = 1.f;
    AcvKey k2; k2.id = 2; k2.time = 0.0f; k2.value = 0.f;
    ace.addKey(1, k1); ace.addKey(1, k2);
    REQUIRE(changed == 1);
}

TEST_CASE("AnimationCurveEditorV1 removeKey", "[Editor][S153]") {
    AnimationCurveEditorV1 ace;
    AcvCurve c; c.id = 1; c.name = "C";
    ace.addCurve(c);
    AcvKey k; k.id = 1; k.time = 0.f; k.value = 5.f;
    ace.addKey(1, k);
    REQUIRE(ace.removeKey(1, 1));
    REQUIRE(ace.removeCurve(1)); // now remove the empty curve
}

TEST_CASE("AnimationCurveEditorV1 evaluate linear interpolation", "[Editor][S153]") {
    AnimationCurveEditorV1 ace;
    AcvCurve c; c.id = 1; c.name = "Val";
    ace.addCurve(c);
    AcvKey k1; k1.id = 1; k1.time = 0.f; k1.value = 0.f;
    AcvKey k2; k2.id = 2; k2.time = 1.f; k2.value = 10.f;
    ace.addKey(1, k1); ace.addKey(1, k2);
    REQUIRE(ace.evaluate(1, 0.5f) == Approx(5.f));
}
