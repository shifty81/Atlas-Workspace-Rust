// Tests for: CaptainSocialGraphSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/captain_social_graph_system.h"

using namespace atlas;
using RType = components::CaptainSocialGraphState::RelationshipType;
using EType = components::CaptainSocialGraphState::EventType;

static void testCaptainSocialGraphInit() {
    std::cout << "\n=== CaptainSocialGraphSystem: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getRelationshipCount("e1") == 0, "No relationships initially");
    assertTrue(sys.getEventCount("e1") == 0, "No events initially");
    assertTrue(sys.getOwnerCaptainId("e1").empty(), "Owner empty initially");
    assertTrue(sys.getTotalRelationshipsFormed("e1") == 0, "No total rels");
    assertTrue(sys.getTotalEventsRecorded("e1") == 0, "No total events");
    assertTrue(sys.getTotalGrudges("e1") == 0, "No grudges");
    assertTrue(sys.getTotalFriendships("e1") == 0, "No friendships");
    assertTrue(sys.getMaxRelationships("e1") == 50, "Default max rels");
    assertTrue(sys.getMaxEvents("e1") == 100, "Default max events");
    assertTrue(approxEqual(sys.getAverageTrust("e1"), 0.0f), "Avg trust 0 when empty");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCaptainSocialGraphAddRelationship() {
    std::cout << "\n=== CaptainSocialGraphSystem: Add Relationship ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addRelationship("e1", "cap_a", RType::Friendship, 0.8f, 0.5f),
               "Add friendship");
    assertTrue(sys.getRelationshipCount("e1") == 1, "One relationship");
    assertTrue(sys.hasRelationship("e1", "cap_a"), "Has cap_a");
    assertTrue(approxEqual(sys.getTrust("e1", "cap_a"), 0.8f), "Trust 0.8");
    assertTrue(approxEqual(sys.getAffinity("e1", "cap_a"), 0.5f), "Affinity 0.5");
    assertTrue(sys.getRelationshipType("e1", "cap_a") == RType::Friendship, "Type is Friendship");
    assertTrue(sys.getTotalFriendships("e1") == 1, "1 friendship total");
    assertTrue(sys.getTotalRelationshipsFormed("e1") == 1, "1 total formed");

    assertTrue(sys.addRelationship("e1", "cap_b", RType::Grudge, 0.1f, -0.9f),
               "Add grudge");
    assertTrue(sys.getRelationshipCount("e1") == 2, "Two relationships");
    assertTrue(sys.getTotalGrudges("e1") == 1, "1 grudge total");

    assertTrue(!sys.addRelationship("e1", "cap_a", RType::Neutral, 0.5f, 0.0f),
               "Duplicate rejected");
    assertTrue(sys.getRelationshipCount("e1") == 2, "Still two");
}

static void testCaptainSocialGraphAddRelationshipValidation() {
    std::cout << "\n=== CaptainSocialGraphSystem: Add Relationship Validation ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.addRelationship("e1", "", RType::Neutral, 0.5f, 0.0f),
               "Empty target rejected");
    assertTrue(!sys.addRelationship("e1", "cap_x", RType::Neutral, -0.1f, 0.0f),
               "Trust below 0 rejected");
    assertTrue(!sys.addRelationship("e1", "cap_x", RType::Neutral, 1.1f, 0.0f),
               "Trust above 1 rejected");
    assertTrue(!sys.addRelationship("e1", "cap_x", RType::Neutral, 0.5f, -1.1f),
               "Affinity below -1 rejected");
    assertTrue(!sys.addRelationship("e1", "cap_x", RType::Neutral, 0.5f, 1.1f),
               "Affinity above 1 rejected");
    assertTrue(!sys.addRelationship("missing", "cap_x", RType::Neutral, 0.5f, 0.0f),
               "Missing entity rejected");
    assertTrue(sys.getRelationshipCount("e1") == 0, "No relationships added");
}

static void testCaptainSocialGraphRemoveRelationship() {
    std::cout << "\n=== CaptainSocialGraphSystem: Remove Relationship ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addRelationship("e1", "cap_a", RType::Friendship, 0.5f, 0.0f);
    sys.addRelationship("e1", "cap_b", RType::Grudge, 0.5f, 0.0f);

    assertTrue(sys.removeRelationship("e1", "cap_a"), "Remove cap_a");
    assertTrue(sys.getRelationshipCount("e1") == 1, "One left");
    assertTrue(!sys.hasRelationship("e1", "cap_a"), "cap_a gone");
    assertTrue(!sys.removeRelationship("e1", "cap_a"), "Already removed");
    assertTrue(!sys.removeRelationship("missing", "cap_b"), "Missing entity");

    assertTrue(sys.clearRelationships("e1"), "Clear succeeds");
    assertTrue(sys.getRelationshipCount("e1") == 0, "All cleared");
    assertTrue(!sys.clearRelationships("missing"), "Clear missing entity");
}

static void testCaptainSocialGraphSetRelationshipType() {
    std::cout << "\n=== CaptainSocialGraphSystem: Set Relationship Type ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addRelationship("e1", "cap_a", RType::Neutral, 0.5f, 0.0f);

    assertTrue(sys.setRelationshipType("e1", "cap_a", RType::Friendship), "Set to Friendship");
    assertTrue(sys.getRelationshipType("e1", "cap_a") == RType::Friendship, "Now Friendship");
    assertTrue(sys.getTotalFriendships("e1") == 1, "Friendship counter incremented");

    assertTrue(sys.setRelationshipType("e1", "cap_a", RType::Grudge), "Set to Grudge");
    assertTrue(sys.getRelationshipType("e1", "cap_a") == RType::Grudge, "Now Grudge");
    assertTrue(sys.getTotalGrudges("e1") == 1, "Grudge counter incremented");
    assertTrue(sys.getTotalFriendships("e1") == 1, "Friendship counter kept (lifetime)");

    assertTrue(!sys.setRelationshipType("e1", "nonexistent", RType::Rivalry), "Missing target");
    assertTrue(!sys.setRelationshipType("missing", "cap_a", RType::Rivalry), "Missing entity");
}

static void testCaptainSocialGraphAdjustTrust() {
    std::cout << "\n=== CaptainSocialGraphSystem: Adjust Trust ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addRelationship("e1", "cap_a", RType::Neutral, 0.5f, 0.0f);

    assertTrue(sys.adjustTrust("e1", "cap_a", 0.2f), "Adjust trust up");
    assertTrue(approxEqual(sys.getTrust("e1", "cap_a"), 0.7f), "Trust now 0.7");
    assertTrue(sys.getInteractionCount("e1", "cap_a") == 1, "1 interaction");

    assertTrue(sys.adjustTrust("e1", "cap_a", -0.3f), "Adjust trust down");
    assertTrue(approxEqual(sys.getTrust("e1", "cap_a"), 0.4f), "Trust now 0.4");
    assertTrue(sys.getInteractionCount("e1", "cap_a") == 2, "2 interactions");

    assertTrue(sys.adjustTrust("e1", "cap_a", 5.0f), "Adjust trust clamp up");
    assertTrue(approxEqual(sys.getTrust("e1", "cap_a"), 1.0f), "Trust clamped to 1.0");

    assertTrue(sys.adjustTrust("e1", "cap_a", -5.0f), "Adjust trust clamp down");
    assertTrue(approxEqual(sys.getTrust("e1", "cap_a"), 0.0f), "Trust clamped to 0.0");

    assertTrue(!sys.adjustTrust("e1", "nonexistent", 0.1f), "Missing target");
    assertTrue(!sys.adjustTrust("missing", "cap_a", 0.1f), "Missing entity");
}

static void testCaptainSocialGraphAdjustAffinity() {
    std::cout << "\n=== CaptainSocialGraphSystem: Adjust Affinity ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addRelationship("e1", "cap_a", RType::Neutral, 0.5f, 0.0f);

    assertTrue(sys.adjustAffinity("e1", "cap_a", 0.3f), "Adjust affinity up");
    assertTrue(approxEqual(sys.getAffinity("e1", "cap_a"), 0.3f), "Affinity 0.3");
    assertTrue(sys.getInteractionCount("e1", "cap_a") == 1, "1 interaction");

    assertTrue(sys.adjustAffinity("e1", "cap_a", -0.5f), "Adjust affinity down");
    assertTrue(approxEqual(sys.getAffinity("e1", "cap_a"), -0.2f), "Affinity -0.2");
    assertTrue(sys.getInteractionCount("e1", "cap_a") == 2, "2 interactions");

    assertTrue(sys.adjustAffinity("e1", "cap_a", 5.0f), "Clamp up");
    assertTrue(approxEqual(sys.getAffinity("e1", "cap_a"), 1.0f), "Affinity clamped to 1.0");

    assertTrue(sys.adjustAffinity("e1", "cap_a", -5.0f), "Clamp down");
    assertTrue(approxEqual(sys.getAffinity("e1", "cap_a"), -1.0f), "Affinity clamped to -1.0");

    assertTrue(!sys.adjustAffinity("e1", "nonexistent", 0.1f), "Missing target");
    assertTrue(!sys.adjustAffinity("missing", "cap_a", 0.1f), "Missing entity");
}

static void testCaptainSocialGraphRecordEvent() {
    std::cout << "\n=== CaptainSocialGraphSystem: Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordEvent("e1", "evt1", "capA", "capB", EType::SharedVictory, 0.8f),
               "Record event");
    assertTrue(sys.getEventCount("e1") == 1, "One event");
    assertTrue(sys.hasEvent("e1", "evt1"), "Has evt1");
    assertTrue(sys.getTotalEventsRecorded("e1") == 1, "Total 1");

    assertTrue(!sys.recordEvent("e1", "evt1", "capA", "capB", EType::Betrayal, -0.5f),
               "Duplicate rejected");
    assertTrue(!sys.recordEvent("e1", "", "capA", "capB", EType::Betrayal, -0.5f),
               "Empty event_id rejected");
    assertTrue(!sys.recordEvent("e1", "evt2", "", "capB", EType::Betrayal, -0.5f),
               "Empty captain_a rejected");
    assertTrue(!sys.recordEvent("e1", "evt2", "capA", "", EType::Betrayal, -0.5f),
               "Empty captain_b rejected");
    assertTrue(!sys.recordEvent("e1", "evt2", "capA", "capB", EType::Betrayal, -1.5f),
               "Impact below -1 rejected");
    assertTrue(!sys.recordEvent("e1", "evt2", "capA", "capB", EType::Betrayal, 1.5f),
               "Impact above 1 rejected");

    // Auto-purge test
    sys.setMaxEvents("e1", 3);
    sys.recordEvent("e1", "evt2", "capA", "capB", EType::Rescue, 0.5f);
    sys.recordEvent("e1", "evt3", "capA", "capB", EType::Argument, -0.3f);
    assertTrue(sys.getEventCount("e1") == 3, "Three events at cap");
    assertTrue(sys.recordEvent("e1", "evt4", "capA", "capB", EType::GiftGiven, 0.2f),
               "Fourth event triggers purge");
    assertTrue(sys.getEventCount("e1") == 3, "Still 3 after purge");
    assertTrue(!sys.hasEvent("e1", "evt1"), "Oldest purged");
    assertTrue(sys.hasEvent("e1", "evt4"), "Newest kept");
}

static void testCaptainSocialGraphConfiguration() {
    std::cout << "\n=== CaptainSocialGraphSystem: Configuration ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setOwnerCaptainId("e1", "captain_007"), "Set owner");
    assertTrue(sys.getOwnerCaptainId("e1") == "captain_007", "Owner matches");

    assertTrue(sys.setMaxRelationships("e1", 10), "Set max rels");
    assertTrue(sys.getMaxRelationships("e1") == 10, "Max rels 10");

    assertTrue(sys.setMaxEvents("e1", 20), "Set max events");
    assertTrue(sys.getMaxEvents("e1") == 20, "Max events 20");

    assertTrue(!sys.setMaxRelationships("e1", -1), "Negative max rels rejected");
    assertTrue(!sys.setMaxEvents("e1", -1), "Negative max events rejected");

    assertTrue(!sys.setOwnerCaptainId("missing", "x"), "Missing entity owner");
    assertTrue(!sys.setMaxRelationships("missing", 5), "Missing entity max rels");
    assertTrue(!sys.setMaxEvents("missing", 5), "Missing entity max events");
}

static void testCaptainSocialGraphAggregateQueries() {
    std::cout << "\n=== CaptainSocialGraphSystem: Aggregate Queries ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addRelationship("e1", "cap_a", RType::Friendship, 0.8f, 0.5f);
    sys.addRelationship("e1", "cap_b", RType::Friendship, 0.6f, 0.3f);
    sys.addRelationship("e1", "cap_c", RType::Grudge, 0.2f, -0.8f);
    sys.addRelationship("e1", "cap_d", RType::Rivalry, 0.4f, -0.2f);

    assertTrue(approxEqual(sys.getAverageTrust("e1"), 0.5f), "Avg trust 0.5");
    assertTrue(sys.getCountByRelationshipType("e1", RType::Friendship) == 2, "2 friendships");
    assertTrue(sys.getCountByRelationshipType("e1", RType::Grudge) == 1, "1 grudge");
    assertTrue(sys.getCountByRelationshipType("e1", RType::Rivalry) == 1, "1 rivalry");
    assertTrue(sys.getCountByRelationshipType("e1", RType::Neutral) == 0, "0 neutral");

    sys.recordEvent("e1", "ev1", "a", "b", EType::SharedVictory, 0.5f);
    sys.recordEvent("e1", "ev2", "a", "b", EType::SharedVictory, 0.3f);
    sys.recordEvent("e1", "ev3", "a", "b", EType::Betrayal, -0.9f);
    assertTrue(sys.getCountByEventType("e1", EType::SharedVictory) == 2, "2 shared victories");
    assertTrue(sys.getCountByEventType("e1", EType::Betrayal) == 1, "1 betrayal");
    assertTrue(sys.getCountByEventType("e1", EType::Rescue) == 0, "0 rescues");
}

static void testCaptainSocialGraphCapacityCap() {
    std::cout << "\n=== CaptainSocialGraphSystem: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxRelationships("e1", 3);

    assertTrue(sys.addRelationship("e1", "cap_1", RType::Neutral, 0.5f, 0.0f), "Add 1");
    assertTrue(sys.addRelationship("e1", "cap_2", RType::Neutral, 0.5f, 0.0f), "Add 2");
    assertTrue(sys.addRelationship("e1", "cap_3", RType::Neutral, 0.5f, 0.0f), "Add 3");
    assertTrue(!sys.addRelationship("e1", "cap_4", RType::Neutral, 0.5f, 0.0f),
               "4th rejected at cap");
    assertTrue(sys.getRelationshipCount("e1") == 3, "Still 3");
}

static void testCaptainSocialGraphMissingEntity() {
    std::cout << "\n=== CaptainSocialGraphSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainSocialGraphSystem sys(&world);

    assertTrue(sys.getRelationshipCount("missing") == 0, "Missing: relCount 0");
    assertTrue(sys.getEventCount("missing") == 0, "Missing: eventCount 0");
    assertTrue(!sys.hasRelationship("missing", "x"), "Missing: hasRel false");
    assertTrue(!sys.hasEvent("missing", "x"), "Missing: hasEvent false");
    assertTrue(approxEqual(sys.getTrust("missing", "x"), 0.0f), "Missing: trust 0");
    assertTrue(approxEqual(sys.getAffinity("missing", "x"), 0.0f), "Missing: affinity 0");
    assertTrue(sys.getInteractionCount("missing", "x") == 0, "Missing: interactions 0");
    assertTrue(sys.getRelationshipType("missing", "x") == RType::Neutral, "Missing: type Neutral");
    assertTrue(sys.getCountByRelationshipType("missing", RType::Friendship) == 0, "Missing: countByRelType 0");
    assertTrue(sys.getCountByEventType("missing", EType::Betrayal) == 0, "Missing: countByEvtType 0");
    assertTrue(sys.getOwnerCaptainId("missing").empty(), "Missing: owner empty");
    assertTrue(sys.getTotalRelationshipsFormed("missing") == 0, "Missing: totalRels 0");
    assertTrue(sys.getTotalEventsRecorded("missing") == 0, "Missing: totalEvents 0");
    assertTrue(sys.getTotalGrudges("missing") == 0, "Missing: grudges 0");
    assertTrue(sys.getTotalFriendships("missing") == 0, "Missing: friendships 0");
    assertTrue(sys.getMaxRelationships("missing") == 0, "Missing: maxRels 0");
    assertTrue(sys.getMaxEvents("missing") == 0, "Missing: maxEvents 0");
    assertTrue(approxEqual(sys.getAverageTrust("missing"), 0.0f), "Missing: avgTrust 0");
    assertTrue(!sys.addRelationship("missing", "x", RType::Neutral, 0.5f, 0.0f), "Missing: addRel false");
    assertTrue(!sys.removeRelationship("missing", "x"), "Missing: removeRel false");
    assertTrue(!sys.clearRelationships("missing"), "Missing: clearRels false");
    assertTrue(!sys.setRelationshipType("missing", "x", RType::Grudge), "Missing: setRelType false");
    assertTrue(!sys.adjustTrust("missing", "x", 0.1f), "Missing: adjustTrust false");
    assertTrue(!sys.adjustAffinity("missing", "x", 0.1f), "Missing: adjustAffinity false");
    assertTrue(!sys.recordEvent("missing", "e", "a", "b", EType::Betrayal, 0.1f), "Missing: recordEvent false");
    assertTrue(!sys.removeEvent("missing", "e"), "Missing: removeEvent false");
    assertTrue(!sys.clearEvents("missing"), "Missing: clearEvents false");
}

void run_captain_social_graph_system_tests() {
    testCaptainSocialGraphInit();
    testCaptainSocialGraphAddRelationship();
    testCaptainSocialGraphAddRelationshipValidation();
    testCaptainSocialGraphRemoveRelationship();
    testCaptainSocialGraphSetRelationshipType();
    testCaptainSocialGraphAdjustTrust();
    testCaptainSocialGraphAdjustAffinity();
    testCaptainSocialGraphRecordEvent();
    testCaptainSocialGraphConfiguration();
    testCaptainSocialGraphAggregateQueries();
    testCaptainSocialGraphCapacityCap();
    testCaptainSocialGraphMissingEntity();
}
