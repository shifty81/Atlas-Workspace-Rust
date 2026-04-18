// S201 editor tests: TriggerVolumeEditorV1, InteractionEditorV1, HapticEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TriggerVolumeEditorV1.h"
#include "NF/Editor/InteractionEditorV1.h"
#include "NF/Editor/HapticEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── TriggerVolumeEditorV1 ───────────────────────────────────────────────────

TEST_CASE("Tvv1Trigger validity", "[Editor][S201]") {
    Tvv1Trigger t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Zone1";
    REQUIRE(t.isValid());
}

TEST_CASE("TriggerVolumeEditorV1 addTrigger and triggerCount", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    REQUIRE(te.triggerCount() == 0);
    Tvv1Trigger t; t.id = 1; t.name = "T1";
    REQUIRE(te.addTrigger(t));
    REQUIRE(te.triggerCount() == 1);
}

TEST_CASE("TriggerVolumeEditorV1 addTrigger invalid fails", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    REQUIRE(!te.addTrigger(Tvv1Trigger{}));
}

TEST_CASE("TriggerVolumeEditorV1 addTrigger duplicate fails", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t; t.id = 1; t.name = "A";
    te.addTrigger(t);
    REQUIRE(!te.addTrigger(t));
}

TEST_CASE("TriggerVolumeEditorV1 removeTrigger", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t; t.id = 2; t.name = "B";
    te.addTrigger(t);
    REQUIRE(te.removeTrigger(2));
    REQUIRE(te.triggerCount() == 0);
    REQUIRE(!te.removeTrigger(2));
}

TEST_CASE("TriggerVolumeEditorV1 findTrigger", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t; t.id = 3; t.name = "C";
    te.addTrigger(t);
    REQUIRE(te.findTrigger(3) != nullptr);
    REQUIRE(te.findTrigger(99) == nullptr);
}

TEST_CASE("TriggerVolumeEditorV1 setState", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t; t.id = 1; t.name = "T";
    te.addTrigger(t);
    REQUIRE(te.setState(1, Tvv1TriggerState::Fired));
    REQUIRE(te.findTrigger(1)->state == Tvv1TriggerState::Fired);
}

TEST_CASE("TriggerVolumeEditorV1 setSize clamped min 0.01", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t; t.id = 1; t.name = "T";
    te.addTrigger(t);
    te.setSize(1, 0.f, -1.f, 5.f);
    REQUIRE(te.findTrigger(1)->sizeX == Approx(0.01f));
    REQUIRE(te.findTrigger(1)->sizeY == Approx(0.01f));
    REQUIRE(te.findTrigger(1)->sizeZ == Approx(5.f));
}

TEST_CASE("TriggerVolumeEditorV1 setLayerMask", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t; t.id = 1; t.name = "T";
    te.addTrigger(t);
    REQUIRE(te.setLayerMask(1, 0x0000FFFFu));
    REQUIRE(te.findTrigger(1)->layerMask == 0x0000FFFFu);
}

TEST_CASE("TriggerVolumeEditorV1 activeCount", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t1; t1.id = 1; t1.name = "A";
    Tvv1Trigger t2; t2.id = 2; t2.name = "B";
    te.addTrigger(t1); te.addTrigger(t2);
    te.setState(2, Tvv1TriggerState::Disabled);
    REQUIRE(te.activeCount() == 1);
}

TEST_CASE("TriggerVolumeEditorV1 countByType", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    Tvv1Trigger t1; t1.id = 1; t1.name = "A"; t1.type = Tvv1TriggerType::Box;
    Tvv1Trigger t2; t2.id = 2; t2.name = "B"; t2.type = Tvv1TriggerType::Sphere;
    Tvv1Trigger t3; t3.id = 3; t3.name = "C"; t3.type = Tvv1TriggerType::Box;
    te.addTrigger(t1); te.addTrigger(t2); te.addTrigger(t3);
    REQUIRE(te.countByType(Tvv1TriggerType::Box) == 2);
    REQUIRE(te.countByType(Tvv1TriggerType::Sphere) == 1);
}

TEST_CASE("TriggerVolumeEditorV1 onChange callback", "[Editor][S201]") {
    TriggerVolumeEditorV1 te;
    uint64_t notified = 0;
    te.setOnChange([&](uint64_t id){ notified = id; });
    Tvv1Trigger t; t.id = 5; t.name = "X";
    te.addTrigger(t);
    REQUIRE(notified == 5);
}

TEST_CASE("tvv1TriggerTypeName all values", "[Editor][S201]") {
    REQUIRE(std::string(tvv1TriggerTypeName(Tvv1TriggerType::Box))      == "Box");
    REQUIRE(std::string(tvv1TriggerTypeName(Tvv1TriggerType::Sphere))   == "Sphere");
    REQUIRE(std::string(tvv1TriggerTypeName(Tvv1TriggerType::Cylinder)) == "Cylinder");
    REQUIRE(std::string(tvv1TriggerTypeName(Tvv1TriggerType::Capsule))  == "Capsule");
}

TEST_CASE("tvv1TriggerStateName all values", "[Editor][S201]") {
    REQUIRE(std::string(tvv1TriggerStateName(Tvv1TriggerState::Active))   == "Active");
    REQUIRE(std::string(tvv1TriggerStateName(Tvv1TriggerState::Disabled)) == "Disabled");
    REQUIRE(std::string(tvv1TriggerStateName(Tvv1TriggerState::Fired))    == "Fired");
}

// ── InteractionEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Iev1Interaction validity", "[Editor][S201]") {
    Iev1Interaction i;
    REQUIRE(!i.isValid());
    i.id = 1; i.name = "Door";
    REQUIRE(i.isValid());
}

TEST_CASE("InteractionEditorV1 addInteraction and interactionCount", "[Editor][S201]") {
    InteractionEditorV1 ie;
    REQUIRE(ie.interactionCount() == 0);
    Iev1Interaction i; i.id = 1; i.name = "I1";
    REQUIRE(ie.addInteraction(i));
    REQUIRE(ie.interactionCount() == 1);
}

TEST_CASE("InteractionEditorV1 addInteraction invalid fails", "[Editor][S201]") {
    InteractionEditorV1 ie;
    REQUIRE(!ie.addInteraction(Iev1Interaction{}));
}

TEST_CASE("InteractionEditorV1 addInteraction duplicate fails", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 1; i.name = "A";
    ie.addInteraction(i);
    REQUIRE(!ie.addInteraction(i));
}

TEST_CASE("InteractionEditorV1 removeInteraction", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 2; i.name = "B";
    ie.addInteraction(i);
    REQUIRE(ie.removeInteraction(2));
    REQUIRE(ie.interactionCount() == 0);
    REQUIRE(!ie.removeInteraction(2));
}

TEST_CASE("InteractionEditorV1 findInteraction", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 3; i.name = "C";
    ie.addInteraction(i);
    REQUIRE(ie.findInteraction(3) != nullptr);
    REQUIRE(ie.findInteraction(99) == nullptr);
}

TEST_CASE("InteractionEditorV1 setState", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 1; i.name = "I";
    ie.addInteraction(i);
    REQUIRE(ie.setState(1, Iev1InteractionState::Locked));
    REQUIRE(ie.findInteraction(1)->state == Iev1InteractionState::Locked);
}

TEST_CASE("InteractionEditorV1 setRange clamped min 0.1", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 1; i.name = "I";
    ie.addInteraction(i);
    ie.setRange(1, 0.f);
    REQUIRE(ie.findInteraction(1)->range == Approx(0.1f));
    ie.setRange(1, 5.f);
    REQUIRE(ie.findInteraction(1)->range == Approx(5.f));
}

TEST_CASE("InteractionEditorV1 setPriority", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 1; i.name = "I";
    ie.addInteraction(i);
    REQUIRE(ie.setPriority(1, Iev1Priority::High));
    REQUIRE(ie.findInteraction(1)->priority == Iev1Priority::High);
}

TEST_CASE("InteractionEditorV1 setPromptText", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i; i.id = 1; i.name = "I";
    ie.addInteraction(i);
    REQUIRE(ie.setPromptText(1, "Press E to use"));
    REQUIRE(ie.findInteraction(1)->promptText == "Press E to use");
}

TEST_CASE("InteractionEditorV1 enabledCount", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i1; i1.id = 1; i1.name = "A";
    Iev1Interaction i2; i2.id = 2; i2.name = "B";
    ie.addInteraction(i1); ie.addInteraction(i2);
    ie.setState(2, Iev1InteractionState::Disabled);
    REQUIRE(ie.enabledCount() == 1);
}

TEST_CASE("InteractionEditorV1 countByType", "[Editor][S201]") {
    InteractionEditorV1 ie;
    Iev1Interaction i1; i1.id = 1; i1.name = "A"; i1.type = Iev1InteractionType::Grab;
    Iev1Interaction i2; i2.id = 2; i2.name = "B"; i2.type = Iev1InteractionType::Talk;
    Iev1Interaction i3; i3.id = 3; i3.name = "C"; i3.type = Iev1InteractionType::Grab;
    ie.addInteraction(i1); ie.addInteraction(i2); ie.addInteraction(i3);
    REQUIRE(ie.countByType(Iev1InteractionType::Grab) == 2);
    REQUIRE(ie.countByType(Iev1InteractionType::Talk) == 1);
}

TEST_CASE("InteractionEditorV1 onChange callback", "[Editor][S201]") {
    InteractionEditorV1 ie;
    uint64_t notified = 0;
    ie.setOnChange([&](uint64_t id){ notified = id; });
    Iev1Interaction i; i.id = 6; i.name = "X";
    ie.addInteraction(i);
    REQUIRE(notified == 6);
}

TEST_CASE("iev1InteractionTypeName all values", "[Editor][S201]") {
    REQUIRE(std::string(iev1InteractionTypeName(Iev1InteractionType::Grab))    == "Grab");
    REQUIRE(std::string(iev1InteractionTypeName(Iev1InteractionType::Use))     == "Use");
    REQUIRE(std::string(iev1InteractionTypeName(Iev1InteractionType::Examine)) == "Examine");
    REQUIRE(std::string(iev1InteractionTypeName(Iev1InteractionType::Talk))    == "Talk");
    REQUIRE(std::string(iev1InteractionTypeName(Iev1InteractionType::Push))    == "Push");
}

TEST_CASE("iev1PriorityName all values", "[Editor][S201]") {
    REQUIRE(std::string(iev1PriorityName(Iev1Priority::High))   == "High");
    REQUIRE(std::string(iev1PriorityName(Iev1Priority::Normal)) == "Normal");
    REQUIRE(std::string(iev1PriorityName(Iev1Priority::Low))    == "Low");
}

// ── HapticEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Hev1Pattern validity", "[Editor][S201]") {
    Hev1Pattern p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "Click1";
    REQUIRE(p.isValid());
}

TEST_CASE("HapticEditorV1 addPattern and patternCount", "[Editor][S201]") {
    HapticEditorV1 he;
    REQUIRE(he.patternCount() == 0);
    Hev1Pattern p; p.id = 1; p.name = "P1";
    REQUIRE(he.addPattern(p));
    REQUIRE(he.patternCount() == 1);
}

TEST_CASE("HapticEditorV1 addPattern invalid fails", "[Editor][S201]") {
    HapticEditorV1 he;
    REQUIRE(!he.addPattern(Hev1Pattern{}));
}

TEST_CASE("HapticEditorV1 addPattern duplicate fails", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 1; p.name = "A";
    he.addPattern(p);
    REQUIRE(!he.addPattern(p));
}

TEST_CASE("HapticEditorV1 removePattern", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 2; p.name = "B";
    he.addPattern(p);
    REQUIRE(he.removePattern(2));
    REQUIRE(he.patternCount() == 0);
    REQUIRE(!he.removePattern(2));
}

TEST_CASE("HapticEditorV1 findPattern", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 3; p.name = "C";
    he.addPattern(p);
    REQUIRE(he.findPattern(3) != nullptr);
    REQUIRE(he.findPattern(99) == nullptr);
}

TEST_CASE("HapticEditorV1 setIntensity clamped", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 1; p.name = "P";
    he.addPattern(p);
    he.setIntensity(1, 2.f);
    REQUIRE(he.findPattern(1)->intensity == Approx(1.f));
    he.setIntensity(1, -0.5f);
    REQUIRE(he.findPattern(1)->intensity == Approx(0.f));
}

TEST_CASE("HapticEditorV1 setDuration clamped", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 1; p.name = "P";
    he.addPattern(p);
    he.setDuration(1, 20.f);
    REQUIRE(he.findPattern(1)->duration == Approx(10.f));
    he.setDuration(1, -1.f);
    REQUIRE(he.findPattern(1)->duration == Approx(0.f));
}

TEST_CASE("HapticEditorV1 setFrequency clamped", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 1; p.name = "P";
    he.addPattern(p);
    he.setFrequency(1, 2000.f);
    REQUIRE(he.findPattern(1)->frequency == Approx(1000.f));
    he.setFrequency(1, -5.f);
    REQUIRE(he.findPattern(1)->frequency == Approx(0.f));
}

TEST_CASE("HapticEditorV1 toggleLooping", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p; p.id = 1; p.name = "P";
    he.addPattern(p);
    REQUIRE(!he.findPattern(1)->looping);
    REQUIRE(he.toggleLooping(1));
    REQUIRE(he.findPattern(1)->looping);
    he.toggleLooping(1);
    REQUIRE(!he.findPattern(1)->looping);
}

TEST_CASE("HapticEditorV1 loopingCount", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p1; p1.id = 1; p1.name = "A";
    Hev1Pattern p2; p2.id = 2; p2.name = "B";
    he.addPattern(p1); he.addPattern(p2);
    he.toggleLooping(1);
    REQUIRE(he.loopingCount() == 1);
}

TEST_CASE("HapticEditorV1 countByType", "[Editor][S201]") {
    HapticEditorV1 he;
    Hev1Pattern p1; p1.id = 1; p1.name = "A"; p1.type = Hev1HapticType::Rumble;
    Hev1Pattern p2; p2.id = 2; p2.name = "B"; p2.type = Hev1HapticType::Click;
    Hev1Pattern p3; p3.id = 3; p3.name = "C"; p3.type = Hev1HapticType::Rumble;
    he.addPattern(p1); he.addPattern(p2); he.addPattern(p3);
    REQUIRE(he.countByType(Hev1HapticType::Rumble) == 2);
    REQUIRE(he.countByType(Hev1HapticType::Click) == 1);
}

TEST_CASE("HapticEditorV1 onChange callback", "[Editor][S201]") {
    HapticEditorV1 he;
    uint64_t notified = 0;
    he.setOnChange([&](uint64_t id){ notified = id; });
    Hev1Pattern p; p.id = 8; p.name = "X";
    he.addPattern(p);
    REQUIRE(notified == 8);
}

TEST_CASE("hev1HapticTypeName all values", "[Editor][S201]") {
    REQUIRE(std::string(hev1HapticTypeName(Hev1HapticType::Click))   == "Click");
    REQUIRE(std::string(hev1HapticTypeName(Hev1HapticType::Pulse))   == "Pulse");
    REQUIRE(std::string(hev1HapticTypeName(Hev1HapticType::Rumble))  == "Rumble");
    REQUIRE(std::string(hev1HapticTypeName(Hev1HapticType::Vibrate)) == "Vibrate");
    REQUIRE(std::string(hev1HapticTypeName(Hev1HapticType::Impact))  == "Impact");
}
