// Tests for: AmbientEventSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/ambient_event_system.h"

using namespace atlas;
using ET = components::AmbientEventState::AmbientEventType;

static void testAmbientEventInit() {
    std::cout << "\n=== AmbientEvent: Init ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getEventCount("e1") == 0, "Zero events initially");
    assertTrue(sys.getActiveEventCount("e1") == 0, "Zero active events");
    assertTrue(sys.getTotalEventsFired("e1") == 0, "Zero total fired");
    assertTrue(sys.getTotalEventsResolved("e1") == 0, "Zero total resolved");
    assertTrue(sys.getSystemId("e1").empty(), "Empty system_id");
    assertTrue(sys.getMaxEvents("e1") == 20, "Default max_events 20");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testAmbientEventFireEvent() {
    std::cout << "\n=== AmbientEvent: FireEvent ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.fireEvent("e1", "ev1", ET::NavBeaconMalfunction, 0.8f, 120.0f),
               "Fire nav beacon event");
    assertTrue(sys.getEventCount("e1") == 1, "1 event");
    assertTrue(sys.hasEvent("e1", "ev1"), "hasEvent ev1");
    assertTrue(sys.isEventActive("e1", "ev1"), "ev1 active");
    assertTrue(approxEqual(sys.getEventIntensity("e1", "ev1"), 0.8f), "Intensity 0.8");
    assertTrue(sys.getTimeRemaining("e1", "ev1") > 0.0f, "Time remaining > 0");
    assertTrue(sys.getEventType("e1", "ev1") == ET::NavBeaconMalfunction, "Type NavBeaconMalfunction");
    assertTrue(sys.getTotalEventsFired("e1") == 1, "Total fired = 1");

    // Duplicate rejected
    assertTrue(!sys.fireEvent("e1", "ev1", ET::StationLockdown, 0.5f, 60.0f),
               "Duplicate event_id rejected");
    assertTrue(sys.getEventCount("e1") == 1, "Still 1 event");

    // Second event
    assertTrue(sys.fireEvent("e1", "ev2", ET::RadiationStorm, 0.6f, 300.0f), "Fire radiation storm");
    assertTrue(sys.getEventCount("e1") == 2, "2 events");
    assertTrue(sys.getTotalEventsFired("e1") == 2, "Total fired = 2");
}

static void testAmbientEventFireValidation() {
    std::cout << "\n=== AmbientEvent: FireEvent Validation ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.fireEvent("e1", "", ET::DistressBeacon, 0.5f, 60.0f), "Empty id rejected");
    assertTrue(!sys.fireEvent("e1", "ev1", ET::DistressBeacon, -0.1f, 60.0f), "Negative intensity rejected");
    assertTrue(!sys.fireEvent("e1", "ev1", ET::DistressBeacon, 1.1f, 60.0f), "Intensity > 1 rejected");
    assertTrue(!sys.fireEvent("e1", "ev1", ET::DistressBeacon, 0.5f, 0.0f), "Zero duration rejected");
    assertTrue(!sys.fireEvent("e1", "ev1", ET::DistressBeacon, 0.5f, -1.0f), "Negative duration rejected");
    assertTrue(!sys.fireEvent("missing", "ev1", ET::DistressBeacon, 0.5f, 60.0f), "Missing entity rejected");
    assertTrue(sys.getEventCount("e1") == 0, "Still zero events");
}

static void testAmbientEventResolve() {
    std::cout << "\n=== AmbientEvent: ResolveEvent ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", ET::StationLockdown, 1.0f, 60.0f);
    assertTrue(sys.getActiveEventCount("e1") == 1, "1 active event");

    assertTrue(sys.resolveEvent("e1", "ev1"), "resolveEvent succeeds");
    assertTrue(sys.getEventCount("e1") == 0, "Event removed after resolve");
    assertTrue(!sys.hasEvent("e1", "ev1"), "hasEvent = false after resolve");
    assertTrue(sys.getTotalEventsResolved("e1") == 1, "Total resolved = 1");

    assertTrue(!sys.resolveEvent("e1", "ev1"), "Re-resolve returns false");
    assertTrue(!sys.resolveEvent("e1", "nonexistent"), "Unknown event returns false");
    assertTrue(!sys.resolveEvent("missing", "ev1"), "Missing entity returns false");
}

static void testAmbientEventTimeoutAutoResolve() {
    std::cout << "\n=== AmbientEvent: Timeout Auto-Resolve ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", ET::TrafficJam, 0.5f, 2.0f);
    assertTrue(sys.getActiveEventCount("e1") == 1, "1 active event");
    assertTrue(sys.getTotalEventsResolved("e1") == 0, "0 resolved before tick");

    sys.update(1.0f);  // partial tick
    assertTrue(sys.getActiveEventCount("e1") == 1, "Still 1 active after 1s");

    sys.update(2.0f);  // exceeds duration
    assertTrue(sys.getEventCount("e1") == 0, "Event removed after expiry");
    assertTrue(sys.getTotalEventsResolved("e1") == 1, "Total resolved = 1");
}

static void testAmbientEventMultipleTypes() {
    std::cout << "\n=== AmbientEvent: Multiple Event Types ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "nav1", ET::NavBeaconMalfunction, 0.5f, 60.0f);
    sys.fireEvent("e1", "lock1", ET::StationLockdown, 0.7f, 60.0f);
    sys.fireEvent("e1", "rad1", ET::RadiationStorm, 0.9f, 60.0f);
    sys.fireEvent("e1", "dist1", ET::DistressBeacon, 0.4f, 60.0f);
    sys.fireEvent("e1", "sal1", ET::SalvageFieldAppearance, 0.6f, 60.0f);
    sys.fireEvent("e1", "jam1", ET::TrafficJam, 0.3f, 60.0f);

    assertTrue(sys.getEventCount("e1") == 6, "6 events total");
    assertTrue(sys.getCountByType("e1", ET::NavBeaconMalfunction) == 1, "1 NavBeacon");
    assertTrue(sys.getCountByType("e1", ET::StationLockdown) == 1, "1 Lockdown");
    assertTrue(sys.getCountByType("e1", ET::RadiationStorm) == 1, "1 RadiationStorm");
    assertTrue(sys.getCountByType("e1", ET::DistressBeacon) == 1, "1 DistressBeacon");
    assertTrue(sys.getCountByType("e1", ET::SalvageFieldAppearance) == 1, "1 SalvageField");
    assertTrue(sys.getCountByType("e1", ET::TrafficJam) == 1, "1 TrafficJam");
}

static void testAmbientEventCapacity() {
    std::cout << "\n=== AmbientEvent: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxEvents("e1", 3);

    assertTrue(sys.fireEvent("e1", "ev1", ET::NavBeaconMalfunction, 0.5f, 60.0f), "Fire 1");
    assertTrue(sys.fireEvent("e1", "ev2", ET::StationLockdown, 0.5f, 60.0f), "Fire 2");
    assertTrue(sys.fireEvent("e1", "ev3", ET::RadiationStorm, 0.5f, 60.0f), "Fire 3");
    assertTrue(!sys.fireEvent("e1", "ev4", ET::DistressBeacon, 0.5f, 60.0f), "4th rejected at cap 3");
    assertTrue(sys.getEventCount("e1") == 3, "3 events at cap");
}

static void testAmbientEventClearRemove() {
    std::cout << "\n=== AmbientEvent: ClearEvents and RemoveEvent ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.fireEvent("e1", "ev1", ET::NavBeaconMalfunction, 0.5f, 60.0f);
    sys.fireEvent("e1", "ev2", ET::StationLockdown, 0.5f, 60.0f);
    assertTrue(sys.getEventCount("e1") == 2, "2 events before clear");

    assertTrue(sys.removeEvent("e1", "ev1"), "removeEvent ev1");
    assertTrue(sys.getEventCount("e1") == 1, "1 event after remove");
    assertTrue(!sys.removeEvent("e1", "ev1"), "Remove non-existent = false");

    assertTrue(sys.clearEvents("e1"), "clearEvents succeeds");
    assertTrue(sys.getEventCount("e1") == 0, "0 events after clear");

    assertTrue(!sys.clearEvents("missing"), "clearEvents on missing = false");
    assertTrue(!sys.removeEvent("missing", "ev1"), "removeEvent on missing = false");
}

static void testAmbientEventConfiguration() {
    std::cout << "\n=== AmbientEvent: Configuration ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSystemId("e1", "sol_prime"), "setSystemId succeeds");
    assertTrue(sys.getSystemId("e1") == "sol_prime", "SystemId = sol_prime");
    assertTrue(!sys.setSystemId("e1", ""), "Empty systemId rejected");

    assertTrue(sys.setMaxEvents("e1", 5), "setMaxEvents(5) succeeds");
    assertTrue(sys.getMaxEvents("e1") == 5, "MaxEvents = 5");
    assertTrue(!sys.setMaxEvents("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxEvents("e1", -1), "Negative max rejected");

    assertTrue(!sys.setSystemId("missing", "x"), "setSystemId on missing fails");
    assertTrue(!sys.setMaxEvents("missing", 5), "setMaxEvents on missing fails");
}

static void testAmbientEventMissingEntity() {
    std::cout << "\n=== AmbientEvent: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AmbientEventSystem sys(&world);

    assertTrue(sys.getEventCount("missing") == 0, "getEventCount = 0");
    assertTrue(sys.getActiveEventCount("missing") == 0, "getActiveEventCount = 0");
    assertTrue(!sys.hasEvent("missing", "ev1"), "hasEvent = false");
    assertTrue(!sys.isEventActive("missing", "ev1"), "isEventActive = false");
    assertTrue(approxEqual(sys.getEventIntensity("missing", "ev1"), 0.0f), "getEventIntensity = 0");
    assertTrue(approxEqual(sys.getTimeRemaining("missing", "ev1"), 0.0f), "getTimeRemaining = 0");
    assertTrue(sys.getEventType("missing", "ev1") == ET::NavBeaconMalfunction, "getEventType default");
    assertTrue(sys.getCountByType("missing", ET::RadiationStorm) == 0, "getCountByType = 0");
    assertTrue(sys.getTotalEventsFired("missing") == 0, "getTotalFired = 0");
    assertTrue(sys.getTotalEventsResolved("missing") == 0, "getTotalResolved = 0");
    assertTrue(sys.getSystemId("missing").empty(), "getSystemId = ''");
    assertTrue(sys.getMaxEvents("missing") == 0, "getMaxEvents = 0");
    assertTrue(!sys.fireEvent("missing", "ev1", ET::DistressBeacon, 0.5f, 60.0f), "fire = false");
    assertTrue(!sys.resolveEvent("missing", "ev1"), "resolve = false");
    assertTrue(!sys.clearEvents("missing"), "clear = false");
    assertTrue(!sys.removeEvent("missing", "ev1"), "remove = false");
}

void run_ambient_event_system_tests() {
    testAmbientEventInit();
    testAmbientEventFireEvent();
    testAmbientEventFireValidation();
    testAmbientEventResolve();
    testAmbientEventTimeoutAutoResolve();
    testAmbientEventMultipleTypes();
    testAmbientEventCapacity();
    testAmbientEventClearRemove();
    testAmbientEventConfiguration();
    testAmbientEventMissingEntity();
}
