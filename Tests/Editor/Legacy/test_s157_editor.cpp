// S157 editor tests: EventTimelineV1, StateGraphV1, TriggerEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TriggerEditorV1.h"
#include "NF/Editor/StateGraphV1.h"
#include "NF/Editor/EventTimelineV1.h"

using namespace NF;
using Catch::Approx;

// ── EventTimelineV1 ───────────────────────────────────────────────────────────

TEST_CASE("EtvEvent validity", "[Editor][S157]") {
    EtvEvent ev;
    REQUIRE(!ev.isValid());
    ev.id = 1; ev.label = "Jump";
    REQUIRE(ev.isValid());
}

TEST_CASE("EventTimelineV1 addEvent and eventCount", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(tl.eventCount() == 0);
    EtvEvent ev; ev.id = 1; ev.label = "Start";
    REQUIRE(tl.addEvent(ev));
    REQUIRE(tl.eventCount() == 1);
}

TEST_CASE("EventTimelineV1 addEvent invalid fails", "[Editor][S157]") {
    EventTimelineV1 tl;
    EtvEvent ev;
    REQUIRE(!tl.addEvent(ev));
}

TEST_CASE("EventTimelineV1 addEvent duplicate id fails", "[Editor][S157]") {
    EventTimelineV1 tl;
    EtvEvent ev; ev.id = 1; ev.label = "A";
    REQUIRE(tl.addEvent(ev));
    REQUIRE(!tl.addEvent(ev));
}

TEST_CASE("EventTimelineV1 removeEvent", "[Editor][S157]") {
    EventTimelineV1 tl;
    EtvEvent ev; ev.id = 1; ev.label = "X";
    tl.addEvent(ev);
    REQUIRE(tl.removeEvent(1));
    REQUIRE(tl.eventCount() == 0);
    REQUIRE(!tl.removeEvent(1));
}

TEST_CASE("EventTimelineV1 play and pause", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(tl.playState() == EtvPlayState::Stopped);
    REQUIRE(tl.play());
    REQUIRE(tl.playState() == EtvPlayState::Playing);
    REQUIRE(!tl.play());
    REQUIRE(tl.pause());
    REQUIRE(tl.playState() == EtvPlayState::Paused);
}

TEST_CASE("EventTimelineV1 stop resets cursor", "[Editor][S157]") {
    EventTimelineV1 tl;
    tl.play();
    tl.seekTo(500.f);
    REQUIRE(tl.stop());
    REQUIRE(tl.cursor() == Approx(0.f));
    REQUIRE(tl.playState() == EtvPlayState::Stopped);
}

TEST_CASE("EventTimelineV1 stop when already stopped fails", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(!tl.stop());
}

TEST_CASE("EventTimelineV1 seekTo", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(tl.seekTo(100.f));
    REQUIRE(tl.cursor() == Approx(100.f));
}

TEST_CASE("EventTimelineV1 seekTo negative fails", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(!tl.seekTo(-1.f));
}

TEST_CASE("EventTimelineV1 setDuration", "[Editor][S157]") {
    EventTimelineV1 tl;
    tl.setDuration(2000.f);
    REQUIRE(tl.duration() == Approx(2000.f));
}

TEST_CASE("EventTimelineV1 fireEvent triggers callback", "[Editor][S157]") {
    EventTimelineV1 tl;
    EtvEvent ev; ev.id = 10; ev.label = "Shoot";
    tl.addEvent(ev);
    uint64_t fired = 0;
    tl.setOnFire([&](const EtvEvent& e){ fired = e.id; });
    REQUIRE(tl.fireEvent(10));
    REQUIRE(fired == 10);
}

TEST_CASE("EventTimelineV1 fireEvent disabled event fails", "[Editor][S157]") {
    EventTimelineV1 tl;
    EtvEvent ev; ev.id = 5; ev.label = "Idle"; ev.enabled = false;
    tl.addEvent(ev);
    REQUIRE(!tl.fireEvent(5));
}

TEST_CASE("EventTimelineV1 fireEvent unknown id fails", "[Editor][S157]") {
    EventTimelineV1 tl;
    REQUIRE(!tl.fireEvent(999));
}

// ── StateGraphV1 ──────────────────────────────────────────────────────────────

TEST_CASE("Sgv1Node validity", "[Editor][S157]") {
    Sgv1Node n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Idle";
    REQUIRE(n.isValid());
}

TEST_CASE("Sgv1Transition validity", "[Editor][S157]") {
    Sgv1Transition t;
    REQUIRE(!t.isValid());
    t.id = 1; t.fromNode = 1; t.toNode = 2;
    REQUIRE(t.isValid());
}

TEST_CASE("StateGraphV1 addNode and nodeCount", "[Editor][S157]") {
    StateGraphV1 sg;
    REQUIRE(sg.nodeCount() == 0);
    Sgv1Node n; n.id = 1; n.name = "Idle";
    REQUIRE(sg.addNode(n));
    REQUIRE(sg.nodeCount() == 1);
}

TEST_CASE("StateGraphV1 addNode invalid fails", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Node n;
    REQUIRE(!sg.addNode(n));
}

TEST_CASE("StateGraphV1 addNode duplicate id fails", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Node n; n.id = 1; n.name = "Run";
    sg.addNode(n);
    REQUIRE(!sg.addNode(n));
}

TEST_CASE("StateGraphV1 removeNode", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Node n; n.id = 2; n.name = "Jump";
    sg.addNode(n);
    REQUIRE(sg.removeNode(2));
    REQUIRE(sg.nodeCount() == 0);
    REQUIRE(!sg.removeNode(2));
}

TEST_CASE("StateGraphV1 addTransition and transitionCount", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Transition t; t.id = 1; t.fromNode = 1; t.toNode = 2;
    REQUIRE(sg.addTransition(t));
    REQUIRE(sg.transitionCount() == 1);
}

TEST_CASE("StateGraphV1 addTransition invalid fails", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Transition t;
    REQUIRE(!sg.addTransition(t));
}

TEST_CASE("StateGraphV1 removeTransition", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Transition t; t.id = 1; t.fromNode = 1; t.toNode = 2;
    sg.addTransition(t);
    REQUIRE(sg.removeTransition(1));
    REQUIRE(sg.transitionCount() == 0);
    REQUIRE(!sg.removeTransition(1));
}

TEST_CASE("StateGraphV1 setActiveNode and activeNode", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Node n; n.id = 3; n.name = "Walk";
    sg.addNode(n);
    REQUIRE(sg.setActiveNode(3));
    REQUIRE(sg.activeNode() == 3);
}

TEST_CASE("StateGraphV1 setActiveNode unknown id fails", "[Editor][S157]") {
    StateGraphV1 sg;
    REQUIRE(!sg.setActiveNode(999));
}

TEST_CASE("StateGraphV1 setActiveNode fires onEnter callback", "[Editor][S157]") {
    StateGraphV1 sg;
    Sgv1Node n; n.id = 7; n.name = "Attack";
    sg.addNode(n);
    uint64_t entered = 0;
    sg.setOnEnter([&](uint64_t id){ entered = id; });
    sg.setActiveNode(7);
    REQUIRE(entered == 7);
}

// ── TriggerEditorV1 ───────────────────────────────────────────────────────────

TEST_CASE("Tev1Action validity", "[Editor][S157]") {
    Tev1Action a;
    REQUIRE(!a.isValid());
    a.id = 1; a.name = "Spawn";
    REQUIRE(a.isValid());
}

TEST_CASE("Tev1Trigger validity", "[Editor][S157]") {
    Tev1Trigger t;
    REQUIRE(!t.isValid());
    t.id = 1; t.label = "EnterZone";
    REQUIRE(t.isValid());
}

TEST_CASE("TriggerEditorV1 addTrigger and triggerCount", "[Editor][S157]") {
    TriggerEditorV1 te;
    REQUIRE(te.triggerCount() == 0);
    Tev1Trigger t; t.id = 1; t.label = "Zone";
    REQUIRE(te.addTrigger(t));
    REQUIRE(te.triggerCount() == 1);
}

TEST_CASE("TriggerEditorV1 addTrigger invalid fails", "[Editor][S157]") {
    TriggerEditorV1 te;
    Tev1Trigger t;
    REQUIRE(!te.addTrigger(t));
}

TEST_CASE("TriggerEditorV1 addTrigger duplicate id fails", "[Editor][S157]") {
    TriggerEditorV1 te;
    Tev1Trigger t; t.id = 1; t.label = "A";
    te.addTrigger(t);
    REQUIRE(!te.addTrigger(t));
}

TEST_CASE("TriggerEditorV1 removeTrigger", "[Editor][S157]") {
    TriggerEditorV1 te;
    Tev1Trigger t; t.id = 2; t.label = "Exit";
    te.addTrigger(t);
    REQUIRE(te.removeTrigger(2));
    REQUIRE(te.triggerCount() == 0);
    REQUIRE(!te.removeTrigger(2));
}

TEST_CASE("TriggerEditorV1 enableTrigger", "[Editor][S157]") {
    TriggerEditorV1 te;
    Tev1Trigger t; t.id = 3; t.label = "Door";
    te.addTrigger(t);
    REQUIRE(te.enableTrigger(3, false));
    REQUIRE(!te.fireTrigger(3));
    REQUIRE(te.enableTrigger(3, true));
    REQUIRE(te.fireTrigger(3));
}

TEST_CASE("TriggerEditorV1 enableTrigger unknown id fails", "[Editor][S157]") {
    TriggerEditorV1 te;
    REQUIRE(!te.enableTrigger(99, false));
}

TEST_CASE("TriggerEditorV1 fireTrigger fires callback", "[Editor][S157]") {
    TriggerEditorV1 te;
    Tev1Trigger t; t.id = 4; t.label = "Bomb";
    te.addTrigger(t);
    uint64_t fired = 0;
    te.setOnFire([&](const Tev1Trigger& tr){ fired = tr.id; });
    REQUIRE(te.fireTrigger(4));
    REQUIRE(fired == 4);
}

TEST_CASE("TriggerEditorV1 fireTrigger disabled fails", "[Editor][S157]") {
    TriggerEditorV1 te;
    Tev1Trigger t; t.id = 5; t.label = "Mine"; t.enabled = false;
    te.addTrigger(t);
    REQUIRE(!te.fireTrigger(5));
}

TEST_CASE("TriggerEditorV1 fireTrigger unknown id fails", "[Editor][S157]") {
    TriggerEditorV1 te;
    REQUIRE(!te.fireTrigger(999));
}
