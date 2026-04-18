// S183 editor tests: TrailEditorV1, VFXGraphEditorV1, TouchInputMapperV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TrailEditorV1.h"
#include "NF/Editor/VFXGraphEditorV1.h"
#include "NF/Editor/TouchInputMapperV1.h"

using namespace NF;
using Catch::Approx;

// ── TrailEditorV1 ────────────────────────────────────────────────────────────

TEST_CASE("Trev1Trail validity", "[Editor][S183]") {
    Trev1Trail t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "FireTrail";
    REQUIRE(t.isValid());
}

TEST_CASE("TrailEditorV1 addTrail and trailCount", "[Editor][S183]") {
    TrailEditorV1 tre;
    REQUIRE(tre.trailCount() == 0);
    Trev1Trail t; t.id = 1; t.name = "T1";
    REQUIRE(tre.addTrail(t));
    REQUIRE(tre.trailCount() == 1);
}

TEST_CASE("TrailEditorV1 addTrail invalid fails", "[Editor][S183]") {
    TrailEditorV1 tre;
    REQUIRE(!tre.addTrail(Trev1Trail{}));
}

TEST_CASE("TrailEditorV1 addTrail duplicate fails", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Trail t; t.id = 1; t.name = "A";
    tre.addTrail(t);
    REQUIRE(!tre.addTrail(t));
}

TEST_CASE("TrailEditorV1 removeTrail", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Trail t; t.id = 2; t.name = "B";
    tre.addTrail(t);
    REQUIRE(tre.removeTrail(2));
    REQUIRE(tre.trailCount() == 0);
    REQUIRE(!tre.removeTrail(2));
}

TEST_CASE("TrailEditorV1 addKeyframe and keyframeCount", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Keyframe kf; kf.id = 1; kf.trailId = 10; kf.time = 0.5f;
    REQUIRE(tre.addKeyframe(kf));
    REQUIRE(tre.keyframeCount() == 1);
}

TEST_CASE("TrailEditorV1 addKeyframe invalid fails", "[Editor][S183]") {
    TrailEditorV1 tre;
    REQUIRE(!tre.addKeyframe(Trev1Keyframe{}));
}

TEST_CASE("TrailEditorV1 removeKeyframe", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Keyframe kf; kf.id = 1; kf.trailId = 5; kf.time = 1.f;
    tre.addKeyframe(kf);
    REQUIRE(tre.removeKeyframe(1));
    REQUIRE(tre.keyframeCount() == 0);
    REQUIRE(!tre.removeKeyframe(1));
}

TEST_CASE("TrailEditorV1 activeCount and finishedCount", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Trail t1; t1.id = 1; t1.name = "A"; t1.state = Trev1TrailState::Active;
    Trev1Trail t2; t2.id = 2; t2.name = "B"; t2.state = Trev1TrailState::Finished;
    tre.addTrail(t1); tre.addTrail(t2);
    REQUIRE(tre.activeCount()   == 1);
    REQUIRE(tre.finishedCount() == 1);
}

TEST_CASE("TrailEditorV1 countByRendererType", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Trail t1; t1.id = 1; t1.name = "A"; t1.rendererType = Trev1RendererType::Ribbon;
    Trev1Trail t2; t2.id = 2; t2.name = "B"; t2.rendererType = Trev1RendererType::Tube;
    tre.addTrail(t1); tre.addTrail(t2);
    REQUIRE(tre.countByRendererType(Trev1RendererType::Ribbon) == 1);
    REQUIRE(tre.countByRendererType(Trev1RendererType::Tube)   == 1);
}

TEST_CASE("TrailEditorV1 findTrail returns ptr", "[Editor][S183]") {
    TrailEditorV1 tre;
    Trev1Trail t; t.id = 5; t.name = "Smoke";
    tre.addTrail(t);
    REQUIRE(tre.findTrail(5) != nullptr);
    REQUIRE(tre.findTrail(5)->name == "Smoke");
    REQUIRE(tre.findTrail(99) == nullptr);
}

TEST_CASE("trev1RendererTypeName covers all values", "[Editor][S183]") {
    REQUIRE(std::string(trev1RendererTypeName(Trev1RendererType::Line))      == "Line");
    REQUIRE(std::string(trev1RendererTypeName(Trev1RendererType::Billboard)) == "Billboard");
    REQUIRE(std::string(trev1RendererTypeName(Trev1RendererType::Mesh))      == "Mesh");
}

TEST_CASE("trev1TrailStateName covers all values", "[Editor][S183]") {
    REQUIRE(std::string(trev1TrailStateName(Trev1TrailState::Inactive)) == "Inactive");
    REQUIRE(std::string(trev1TrailStateName(Trev1TrailState::Paused))   == "Paused");
    REQUIRE(std::string(trev1TrailStateName(Trev1TrailState::Finished)) == "Finished");
}

TEST_CASE("Trev1Trail state helpers", "[Editor][S183]") {
    Trev1Trail t; t.id = 1; t.name = "X";
    t.state = Trev1TrailState::Active;
    REQUIRE(t.isActive());
    t.state = Trev1TrailState::Paused;
    REQUIRE(t.isPaused());
    t.state = Trev1TrailState::Finished;
    REQUIRE(t.isFinished());
}

TEST_CASE("TrailEditorV1 onChange callback", "[Editor][S183]") {
    TrailEditorV1 tre;
    uint64_t notified = 0;
    tre.setOnChange([&](uint64_t id) { notified = id; });
    Trev1Trail t; t.id = 7; t.name = "Spark";
    tre.addTrail(t);
    REQUIRE(notified == 7);
}

// ── VFXGraphEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Vfxv1Node validity", "[Editor][S183]") {
    Vfxv1Node n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Emitter01";
    REQUIRE(n.isValid());
}

TEST_CASE("VFXGraphEditorV1 addNode and nodeCount", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    REQUIRE(vfx.nodeCount() == 0);
    Vfxv1Node n; n.id = 1; n.name = "N1";
    REQUIRE(vfx.addNode(n));
    REQUIRE(vfx.nodeCount() == 1);
}

TEST_CASE("VFXGraphEditorV1 addNode invalid fails", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    REQUIRE(!vfx.addNode(Vfxv1Node{}));
}

TEST_CASE("VFXGraphEditorV1 addNode duplicate fails", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Node n; n.id = 1; n.name = "A";
    vfx.addNode(n);
    REQUIRE(!vfx.addNode(n));
}

TEST_CASE("VFXGraphEditorV1 removeNode", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Node n; n.id = 2; n.name = "B";
    vfx.addNode(n);
    REQUIRE(vfx.removeNode(2));
    REQUIRE(vfx.nodeCount() == 0);
    REQUIRE(!vfx.removeNode(2));
}

TEST_CASE("VFXGraphEditorV1 addLink and linkCount", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Link l; l.id = 1; l.fromNode = 1; l.toNode = 2;
    REQUIRE(vfx.addLink(l));
    REQUIRE(vfx.linkCount() == 1);
}

TEST_CASE("VFXGraphEditorV1 addLink invalid fails", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    REQUIRE(!vfx.addLink(Vfxv1Link{}));
}

TEST_CASE("VFXGraphEditorV1 removeLink", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Link l; l.id = 1; l.fromNode = 1; l.toNode = 2;
    vfx.addLink(l);
    REQUIRE(vfx.removeLink(1));
    REQUIRE(vfx.linkCount() == 0);
    REQUIRE(!vfx.removeLink(1));
}

TEST_CASE("VFXGraphEditorV1 enabledCount and lockedCount", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Node n1; n1.id = 1; n1.name = "A"; n1.state = Vfxv1NodeState::Enabled;
    Vfxv1Node n2; n2.id = 2; n2.name = "B"; n2.state = Vfxv1NodeState::Locked;
    vfx.addNode(n1); vfx.addNode(n2);
    REQUIRE(vfx.enabledCount() == 1);
    REQUIRE(vfx.lockedCount()  == 1);
}

TEST_CASE("VFXGraphEditorV1 countByNodeType", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Node n1; n1.id = 1; n1.name = "A"; n1.nodeType = Vfxv1NodeType::Force;
    Vfxv1Node n2; n2.id = 2; n2.name = "B"; n2.nodeType = Vfxv1NodeType::Output;
    vfx.addNode(n1); vfx.addNode(n2);
    REQUIRE(vfx.countByNodeType(Vfxv1NodeType::Force)  == 1);
    REQUIRE(vfx.countByNodeType(Vfxv1NodeType::Output) == 1);
}

TEST_CASE("VFXGraphEditorV1 findNode returns ptr", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    Vfxv1Node n; n.id = 6; n.name = "Collision";
    vfx.addNode(n);
    REQUIRE(vfx.findNode(6) != nullptr);
    REQUIRE(vfx.findNode(6)->name == "Collision");
    REQUIRE(vfx.findNode(99) == nullptr);
}

TEST_CASE("vfxv1NodeTypeName covers all values", "[Editor][S183]") {
    REQUIRE(std::string(vfxv1NodeTypeName(Vfxv1NodeType::Emitter))      == "Emitter");
    REQUIRE(std::string(vfxv1NodeTypeName(Vfxv1NodeType::Collision))    == "Collision");
    REQUIRE(std::string(vfxv1NodeTypeName(Vfxv1NodeType::ColorOverLife))== "ColorOverLife");
    REQUIRE(std::string(vfxv1NodeTypeName(Vfxv1NodeType::SizeOverLife)) == "SizeOverLife");
}

TEST_CASE("vfxv1NodeStateName covers all values", "[Editor][S183]") {
    REQUIRE(std::string(vfxv1NodeStateName(Vfxv1NodeState::Disabled)) == "Disabled");
    REQUIRE(std::string(vfxv1NodeStateName(Vfxv1NodeState::Preview))  == "Preview");
}

TEST_CASE("Vfxv1Node state helpers", "[Editor][S183]") {
    Vfxv1Node n; n.id = 1; n.name = "X";
    n.state = Vfxv1NodeState::Enabled;
    REQUIRE(n.isEnabled());
    n.state = Vfxv1NodeState::Locked;
    REQUIRE(n.isLocked());
}

TEST_CASE("VFXGraphEditorV1 onChange callback", "[Editor][S183]") {
    VFXGraphEditorV1 vfx;
    uint64_t notified = 0;
    vfx.setOnChange([&](uint64_t id) { notified = id; });
    Vfxv1Node n; n.id = 3; n.name = "C";
    vfx.addNode(n);
    REQUIRE(notified == 3);
}

// ── TouchInputMapperV1 ───────────────────────────────────────────────────────

TEST_CASE("Timv1Gesture validity", "[Editor][S183]") {
    Timv1Gesture g;
    REQUIRE(!g.isValid());
    g.id = 1; g.name = "TapAction";
    REQUIRE(g.isValid());
}

TEST_CASE("TouchInputMapperV1 addGesture and gestureCount", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    REQUIRE(tim.gestureCount() == 0);
    Timv1Gesture g; g.id = 1; g.name = "G1";
    REQUIRE(tim.addGesture(g));
    REQUIRE(tim.gestureCount() == 1);
}

TEST_CASE("TouchInputMapperV1 addGesture invalid fails", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    REQUIRE(!tim.addGesture(Timv1Gesture{}));
}

TEST_CASE("TouchInputMapperV1 addGesture duplicate fails", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Gesture g; g.id = 1; g.name = "A";
    tim.addGesture(g);
    REQUIRE(!tim.addGesture(g));
}

TEST_CASE("TouchInputMapperV1 removeGesture", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Gesture g; g.id = 2; g.name = "B";
    tim.addGesture(g);
    REQUIRE(tim.removeGesture(2));
    REQUIRE(tim.gestureCount() == 0);
    REQUIRE(!tim.removeGesture(2));
}

TEST_CASE("TouchInputMapperV1 addMapping and mappingCount", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Mapping m; m.id = 1; m.gestureId = 10; m.action = "jump";
    REQUIRE(tim.addMapping(m));
    REQUIRE(tim.mappingCount() == 1);
}

TEST_CASE("TouchInputMapperV1 addMapping invalid fails", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    REQUIRE(!tim.addMapping(Timv1Mapping{}));
}

TEST_CASE("TouchInputMapperV1 removeMapping", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Mapping m; m.id = 1; m.gestureId = 5; m.action = "dash";
    tim.addMapping(m);
    REQUIRE(tim.removeMapping(1));
    REQUIRE(tim.mappingCount() == 0);
    REQUIRE(!tim.removeMapping(1));
}

TEST_CASE("TouchInputMapperV1 boundCount and conflictingCount", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Gesture g1; g1.id = 1; g1.name = "A"; g1.state = Timv1BindingState::Bound;
    Timv1Gesture g2; g2.id = 2; g2.name = "B"; g2.state = Timv1BindingState::Conflicting;
    tim.addGesture(g1); tim.addGesture(g2);
    REQUIRE(tim.boundCount()       == 1);
    REQUIRE(tim.conflictingCount() == 1);
}

TEST_CASE("TouchInputMapperV1 countByGestureType", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Gesture g1; g1.id = 1; g1.name = "A"; g1.gestureType = Timv1GestureType::Swipe;
    Timv1Gesture g2; g2.id = 2; g2.name = "B"; g2.gestureType = Timv1GestureType::Pinch;
    tim.addGesture(g1); tim.addGesture(g2);
    REQUIRE(tim.countByGestureType(Timv1GestureType::Swipe) == 1);
    REQUIRE(tim.countByGestureType(Timv1GestureType::Pinch) == 1);
}

TEST_CASE("TouchInputMapperV1 findGesture returns ptr", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    Timv1Gesture g; g.id = 4; g.name = "ZoomIn";
    tim.addGesture(g);
    REQUIRE(tim.findGesture(4) != nullptr);
    REQUIRE(tim.findGesture(4)->name == "ZoomIn");
    REQUIRE(tim.findGesture(99) == nullptr);
}

TEST_CASE("timv1GestureTypeName covers all values", "[Editor][S183]") {
    REQUIRE(std::string(timv1GestureTypeName(Timv1GestureType::Tap))       == "Tap");
    REQUIRE(std::string(timv1GestureTypeName(Timv1GestureType::DoubleTap)) == "DoubleTap");
    REQUIRE(std::string(timv1GestureTypeName(Timv1GestureType::LongPress)) == "LongPress");
    REQUIRE(std::string(timv1GestureTypeName(Timv1GestureType::Rotate))    == "Rotate");
    REQUIRE(std::string(timv1GestureTypeName(Timv1GestureType::Pan))       == "Pan");
}

TEST_CASE("timv1BindingStateName covers all values", "[Editor][S183]") {
    REQUIRE(std::string(timv1BindingStateName(Timv1BindingState::Unbound))    == "Unbound");
    REQUIRE(std::string(timv1BindingStateName(Timv1BindingState::Disabled))   == "Disabled");
    REQUIRE(std::string(timv1BindingStateName(Timv1BindingState::Conflicting))== "Conflicting");
}

TEST_CASE("Timv1Gesture state helpers", "[Editor][S183]") {
    Timv1Gesture g; g.id = 1; g.name = "X";
    g.state = Timv1BindingState::Bound;
    REQUIRE(g.isBound());
    g.state = Timv1BindingState::Disabled;
    REQUIRE(g.isDisabled());
    g.state = Timv1BindingState::Conflicting;
    REQUIRE(g.isConflicting());
}

TEST_CASE("TouchInputMapperV1 onChange callback", "[Editor][S183]") {
    TouchInputMapperV1 tim;
    uint64_t notified = 0;
    tim.setOnChange([&](uint64_t id) { notified = id; });
    Timv1Gesture g; g.id = 6; g.name = "F";
    tim.addGesture(g);
    REQUIRE(notified == 6);
}
