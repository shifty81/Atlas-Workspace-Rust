// Tests for: SystemEventSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/system_event_system.h"

using namespace atlas;
using SET = components::SystemEventState::SystemEventType;

static void testSystemEventInit() {
    std::cout << "\n=== SystemEvent: Init ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getEventCount("e1") == 0, "Zero events initially");
    assertTrue(sys.getTotalEventsFired("e1") == 0, "Zero total fired");
    assertTrue(sys.getTotalEventsResolved("e1") == 0, "Zero total resolved");
    assertTrue(sys.getActiveEventCount("e1") == 0, "Zero active events");
    assertTrue(sys.getSystemId("e1").empty(), "Empty system_id");
    assertTrue(approxEqual(sys.getThreatLevel("e1"), 0.3f), "Default threat_level 0.3");
    assertTrue(approxEqual(sys.getEconomyHealth("e1"), 0.7f), "Default economy_health 0.7");
    assertTrue(approxEqual(sys.getSecurityLevel("e1"), 0.6f), "Default security_level 0.6");
    assertTrue(approxEqual(sys.getTradeVolume("e1"), 0.5f), "Default trade_volume 0.5");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSystemEventFireEvent() {
    std::cout << "\n=== SystemEvent: FireEvent ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.7f, 300.0f),
        "Fire PirateSurge event");
    assertTrue(sys.getEventCount("e1") == 1, "1 event");
    assertTrue(sys.hasEvent("e1", "ev1"), "hasEvent ev1");
    assertTrue(approxEqual(sys.getEventSeverity("e1", "ev1"), 0.7f), "Severity matches");
    assertTrue(approxEqual(sys.getTimeRemaining("e1", "ev1"), 300.0f), "Duration matches");
    assertTrue(sys.isEventActive("e1", "ev1"), "Event is active");
    assertTrue(sys.getTotalEventsFired("e1") == 1, "total_fired = 1");
    assertTrue(sys.getActiveEventCount("e1") == 1, "1 active event");

    // Second event of different type
    assertTrue(
        sys.fireEvent("e1", "ev2", SET::TradeShortage, 0.5f, 200.0f),
        "Fire TradeShortage");
    assertTrue(sys.getEventCount("e1") == 2, "2 events");
    assertTrue(sys.getTotalEventsFired("e1") == 2, "total_fired = 2");

    // Duplicate rejected
    assertTrue(
        !sys.fireEvent("e1", "ev1", SET::FactionWarning, 0.3f, 100.0f),
        "Duplicate event_id rejected");
    assertTrue(sys.getEventCount("e1") == 2, "Still 2 events");
}

static void testSystemEventFireEventValidation() {
    std::cout << "\n=== SystemEvent: FireEvent Validation ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        !sys.fireEvent("e1", "", SET::PirateSurge, 0.5f, 100.0f),
        "Empty event_id rejected");
    assertTrue(
        !sys.fireEvent("e1", "ev1", SET::PirateSurge, -0.1f, 100.0f),
        "Negative severity rejected");
    assertTrue(
        !sys.fireEvent("e1", "ev1", SET::PirateSurge, 1.1f, 100.0f),
        "Severity > 1 rejected");
    assertTrue(
        !sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.5f, 0.0f),
        "Zero duration rejected");
    assertTrue(
        !sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.5f, -10.0f),
        "Negative duration rejected");
    assertTrue(
        !sys.fireEvent("missing", "ev1", SET::PirateSurge, 0.5f, 100.0f),
        "Fire on missing entity fails");
    assertTrue(sys.getEventCount("e1") == 0, "Still zero events");
}

static void testSystemEventResolveEvent() {
    std::cout << "\n=== SystemEvent: ResolveEvent ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", SET::MigrationWave, 0.4f, 300.0f);
    sys.fireEvent("e1", "ev2", SET::RadiationStorm, 0.6f, 200.0f);
    assertTrue(sys.getEventCount("e1") == 2, "2 events before resolve");

    assertTrue(sys.resolveEvent("e1", "ev1"), "Resolve ev1");
    assertTrue(sys.getEventCount("e1") == 1, "1 event after resolve");
    assertTrue(!sys.hasEvent("e1", "ev1"), "ev1 gone");
    assertTrue(sys.hasEvent("e1", "ev2"), "ev2 remains");
    assertTrue(sys.getTotalEventsResolved("e1") == 1, "total_resolved = 1");

    assertTrue(!sys.resolveEvent("e1", "ev1"), "Resolve missing fails");
    assertTrue(!sys.resolveEvent("missing", "ev2"), "Resolve on missing entity fails");

    assertTrue(sys.clearEvents("e1"), "Clear events");
    assertTrue(sys.getEventCount("e1") == 0, "Zero after clear");
    assertTrue(!sys.clearEvents("missing"), "Clear on missing fails");
}

static void testSystemEventAutoThresholdFiring() {
    std::cout << "\n=== SystemEvent: Auto Threshold Firing ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // PirateSurge: threat_level >= pirate_surge_threshold (0.7)
    sys.setThreatLevel("e1", 0.8f);
    sys.update(1.0f);
    assertTrue(sys.hasEvent("e1", "auto_pirate_surge"), "auto_pirate_surge fired");
    assertTrue(approxEqual(sys.getEventSeverity("e1", "auto_pirate_surge"), 0.8f), "Severity = threat_level");
    assertTrue(sys.getTotalEventsFired("e1") == 1, "total_fired = 1");

    // Should not fire duplicate
    sys.update(1.0f);
    assertTrue(sys.getCountByType("e1", SET::PirateSurge) == 1, "Only 1 PirateSurge");

    // TradeShortage: economy_health <= shortage_threshold (0.3)
    sys.clearEvents("e1");
    sys.setThreatLevel("e1", 0.3f);  // below pirate threshold
    sys.setEconomyHealth("e1", 0.2f);
    sys.update(1.0f);
    assertTrue(sys.hasEvent("e1", "auto_trade_shortage"), "auto_trade_shortage fired");

    // SecurityLockdown: security_level <= lockdown_threshold (0.2)
    sys.clearEvents("e1");
    sys.setEconomyHealth("e1", 0.7f);  // reset
    sys.setSecurityLevel("e1", 0.1f);
    sys.update(1.0f);
    assertTrue(sys.hasEvent("e1", "auto_security_lockdown"), "auto_security_lockdown fired");

    // ResourceBoom: economy_health >= boom_threshold (0.8)
    sys.clearEvents("e1");
    sys.setSecurityLevel("e1", 0.6f);  // reset
    sys.setEconomyHealth("e1", 0.9f);
    sys.update(1.0f);
    assertTrue(sys.hasEvent("e1", "auto_resource_boom"), "auto_resource_boom fired");
}

static void testSystemEventExpiry() {
    std::cout << "\n=== SystemEvent: Expiry ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.5f, 2.0f);
    assertTrue(sys.getEventCount("e1") == 1, "1 event before expiry");

    sys.update(1.0f);
    assertTrue(sys.getEventCount("e1") == 1, "1 event after 1s");

    sys.update(2.0f);
    assertTrue(sys.getEventCount("e1") == 0, "0 events after expiry");
    assertTrue(!sys.hasEvent("e1", "ev1"), "ev1 expired and removed");
    assertTrue(sys.getTotalEventsResolved("e1") == 1, "total_resolved = 1");
}

static void testSystemEventConfiguration() {
    std::cout << "\n=== SystemEvent: Configuration ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSystemId("e1", "sys-alpha"), "Set system_id");
    assertTrue(sys.getSystemId("e1") == "sys-alpha", "system_id matches");
    assertTrue(!sys.setSystemId("e1", ""), "Empty system_id rejected");

    assertTrue(sys.setThreatLevel("e1", 0.5f), "Set threat_level 0.5");
    assertTrue(approxEqual(sys.getThreatLevel("e1"), 0.5f), "Threat matches");
    assertTrue(!sys.setThreatLevel("e1", -0.1f), "Negative threat rejected");
    assertTrue(!sys.setThreatLevel("e1", 1.1f), "Threat > 1 rejected");

    assertTrue(sys.setEconomyHealth("e1", 0.4f), "Set economy_health");
    assertTrue(approxEqual(sys.getEconomyHealth("e1"), 0.4f), "Economy matches");

    assertTrue(sys.setSecurityLevel("e1", 0.8f), "Set security_level");
    assertTrue(approxEqual(sys.getSecurityLevel("e1"), 0.8f), "Security matches");

    assertTrue(sys.setTradeVolume("e1", 0.6f), "Set trade_volume");
    assertTrue(approxEqual(sys.getTradeVolume("e1"), 0.6f), "Trade matches");

    assertTrue(sys.setMaxEvents("e1", 5), "Set max_events 5");
    assertTrue(!sys.setMaxEvents("e1", 0), "Zero max rejected");

    assertTrue(sys.setPirateSurgeThreshold("e1", 0.8f), "Set pirate threshold");
    assertTrue(sys.setShortageThreshold("e1", 0.25f), "Set shortage threshold");
    assertTrue(sys.setLockdownThreshold("e1", 0.15f), "Set lockdown threshold");
    assertTrue(sys.setBoomThreshold("e1", 0.85f), "Set boom threshold");

    assertTrue(!sys.setPirateSurgeThreshold("e1", -0.1f), "Negative threshold rejected");
    assertTrue(!sys.setShortageThreshold("e1", 1.5f), "Out-of-range threshold rejected");

    assertTrue(!sys.setSystemId("missing", "s"), "Set on missing fails");
    assertTrue(!sys.setThreatLevel("missing", 0.5f), "Threat on missing fails");
}

static void testSystemEventCountByType() {
    std::cout << "\n=== SystemEvent: CountByType ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.5f, 100.0f);
    sys.fireEvent("e1", "ev2", SET::PirateSurge, 0.4f, 100.0f);
    sys.fireEvent("e1", "ev3", SET::TradeShortage, 0.6f, 100.0f);
    sys.fireEvent("e1", "ev4", SET::ResourceBoom, 0.8f, 100.0f);

    assertTrue(sys.getCountByType("e1", SET::PirateSurge) == 2, "2 PirateSurge");
    assertTrue(sys.getCountByType("e1", SET::TradeShortage) == 1, "1 TradeShortage");
    assertTrue(sys.getCountByType("e1", SET::ResourceBoom) == 1, "1 ResourceBoom");
    assertTrue(sys.getCountByType("e1", SET::SecurityLockdown) == 0, "0 SecurityLockdown");
    assertTrue(sys.getCountByType("missing", SET::PirateSurge) == 0, "CountByType on missing = 0");
}

static void testSystemEventCapacityCap() {
    std::cout << "\n=== SystemEvent: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxEvents("e1", 3);

    assertTrue(sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.5f, 100.0f), "Fire ev1");
    assertTrue(sys.fireEvent("e1", "ev2", SET::TradeShortage, 0.4f, 100.0f), "Fire ev2");
    assertTrue(sys.fireEvent("e1", "ev3", SET::MigrationWave, 0.3f, 100.0f), "Fire ev3");
    assertTrue(!sys.fireEvent("e1", "ev4", SET::RadiationStorm, 0.2f, 100.0f), "ev4 rejected at capacity");
    assertTrue(sys.getEventCount("e1") == 3, "Still 3 events");
}

static void testSystemEventClearEvents() {
    std::cout << "\n=== SystemEvent: ClearEvents ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", SET::PirateSurge, 0.5f, 100.0f);
    sys.fireEvent("e1", "ev2", SET::TradeShortage, 0.4f, 100.0f);
    assertTrue(sys.getEventCount("e1") == 2, "2 events before clear");
    assertTrue(sys.clearEvents("e1"), "Clear succeeds");
    assertTrue(sys.getEventCount("e1") == 0, "Zero after clear");
    assertTrue(sys.getTotalEventsFired("e1") == 2, "total_fired unchanged after clear");
}

static void testSystemEventMissingEntity() {
    std::cout << "\n=== SystemEvent: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SystemEventSystem sys(&world);

    assertTrue(sys.getEventCount("missing") == 0, "getEventCount = 0");
    assertTrue(!sys.hasEvent("missing", "ev1"), "hasEvent = false");
    assertTrue(approxEqual(sys.getEventSeverity("missing", "ev1"), 0.0f), "getEventSeverity = 0");
    assertTrue(approxEqual(sys.getTimeRemaining("missing", "ev1"), 0.0f), "getTimeRemaining = 0");
    assertTrue(!sys.isEventActive("missing", "ev1"), "isEventActive = false");
    assertTrue(sys.getTotalEventsFired("missing") == 0, "getTotalEventsFired = 0");
    assertTrue(sys.getTotalEventsResolved("missing") == 0, "getTotalEventsResolved = 0");
    assertTrue(sys.getActiveEventCount("missing") == 0, "getActiveEventCount = 0");
    assertTrue(approxEqual(sys.getThreatLevel("missing"), 0.0f), "getThreatLevel = 0");
    assertTrue(approxEqual(sys.getEconomyHealth("missing"), 0.0f), "getEconomyHealth = 0");
    assertTrue(approxEqual(sys.getSecurityLevel("missing"), 0.0f), "getSecurityLevel = 0");
    assertTrue(approxEqual(sys.getTradeVolume("missing"), 0.0f), "getTradeVolume = 0");
    assertTrue(sys.getSystemId("missing").empty(), "getSystemId = ''");
    assertTrue(sys.getCountByType("missing", SET::PirateSurge) == 0, "getCountByType = 0");
}

void run_system_event_system_tests() {
    testSystemEventInit();
    testSystemEventFireEvent();
    testSystemEventFireEventValidation();
    testSystemEventResolveEvent();
    testSystemEventAutoThresholdFiring();
    testSystemEventExpiry();
    testSystemEventConfiguration();
    testSystemEventCountByType();
    testSystemEventCapacityCap();
    testSystemEventClearEvents();
    testSystemEventMissingEntity();
}
