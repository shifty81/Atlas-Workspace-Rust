// Tests for: FleetAdvertisementSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/fleet_advertisement_system.h"

using namespace atlas;

// ==================== FleetAdvertisementSystem Tests ====================

static void testFleetAdInit() {
    std::cout << "\n=== FleetAdvertisement: Init ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getApplicationCount("e1") == 0, "Zero applications");
    assertTrue(!sys.isListed("e1"), "Not listed initially");
    assertTrue(sys.getTitle("e1") == "", "No title initially");
    assertTrue(sys.getBossName("e1") == "", "No boss initially");
    assertTrue(sys.getFleetId("e1") == "", "No fleet ID initially");
    assertTrue(sys.getCurrentMembers("e1") == 0, "Zero members");
    assertTrue(sys.getMaxMembers("e1") == 50, "Default max 50");
    assertTrue(sys.getTotalAdsPosted("e1") == 0, "Zero ads posted");
    assertTrue(sys.getTotalApplicationsReceived("e1") == 0, "Zero received");
    assertTrue(sys.getTotalAccepted("e1") == 0, "Zero accepted");
    assertTrue(sys.getTotalRejected("e1") == 0, "Zero rejected");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetAdPostDelist() {
    std::cout << "\n=== FleetAdvertisement: Post/Delist ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FleetAdvertisementState::FleetType;

    assertTrue(sys.postAd("e1", "Mining Fleet", "Ore ops", FT::Mining), "Post ad");
    assertTrue(sys.isListed("e1"), "Is listed");
    assertTrue(sys.getTitle("e1") == "Mining Fleet", "Title correct");
    assertTrue(sys.getFleetType("e1") == FT::Mining, "Type is Mining");
    assertTrue(sys.getTotalAdsPosted("e1") == 1, "1 ad posted");
    assertTrue(sys.getTimeRemaining("e1") > 0.0f, "Has time remaining");

    // Cannot double-list
    assertTrue(!sys.postAd("e1", "Another", "Desc", FT::PvE), "Double-list rejected");

    assertTrue(sys.delistAd("e1"), "Delist ad");
    assertTrue(!sys.isListed("e1"), "Not listed after delist");
    assertTrue(approxEqual(sys.getTimeRemaining("e1"), 0.0f), "Zero time after delist");
    assertTrue(!sys.delistAd("e1"), "Cannot delist when not listed");

    // Post again after delist
    assertTrue(sys.postAd("e1", "New Fleet", "Desc", FT::Exploration), "Re-post");
    assertTrue(sys.getTotalAdsPosted("e1") == 2, "2 ads posted");

    // Empty title rejected
    assertTrue(!sys.postAd("missing", "Title", "Desc", FT::PvE), "Post on missing fails");
    assertTrue(!sys.delistAd("missing"), "Delist on missing fails");
}

static void testFleetAdEmptyTitle() {
    std::cout << "\n=== FleetAdvertisement: EmptyTitle ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FleetAdvertisementState::FleetType;
    assertTrue(!sys.postAd("e1", "", "Desc", FT::PvE), "Empty title rejected");
    assertTrue(!sys.isListed("e1"), "Not listed");
}

static void testFleetAdConfiguration() {
    std::cout << "\n=== FleetAdvertisement: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setFleetId("e1", "fleet123"), "Set fleet ID");
    assertTrue(sys.getFleetId("e1") == "fleet123", "Fleet ID correct");

    assertTrue(sys.setBossName("e1", "Commander"), "Set boss name");
    assertTrue(sys.getBossName("e1") == "Commander", "Boss name correct");

    assertTrue(sys.setTtl("e1", 7200.0f), "Set TTL 7200");
    assertTrue(!sys.setTtl("e1", 0.0f), "Zero TTL rejected");
    assertTrue(!sys.setTtl("e1", -1.0f), "Negative TTL rejected");

    assertTrue(sys.setMinMembers("e1", 5), "Set min members 5");
    assertTrue(!sys.setMinMembers("e1", 0), "Zero min rejected");

    assertTrue(sys.setMaxMembers("e1", 100), "Set max members 100");
    assertTrue(!sys.setMaxMembers("e1", 0), "Zero max rejected");

    assertTrue(sys.setMaxApplications("e1", 30), "Set max apps 30");
    assertTrue(!sys.setMaxApplications("e1", 0), "Zero max apps rejected");

    assertTrue(sys.setCurrentMembers("e1", 10), "Set current members 10");
    assertTrue(sys.getCurrentMembers("e1") == 10, "Current members 10");
    assertTrue(!sys.setCurrentMembers("e1", -1), "Negative members rejected");

    assertTrue(!sys.setFleetId("missing", "x"), "FleetId on missing fails");
    assertTrue(!sys.setBossName("missing", "x"), "Boss on missing fails");
    assertTrue(!sys.setTtl("missing", 100.0f), "TTL on missing fails");
    assertTrue(!sys.setMinMembers("missing", 5), "Min on missing fails");
    assertTrue(!sys.setMaxMembers("missing", 50), "Max on missing fails");
    assertTrue(!sys.setMaxApplications("missing", 10), "MaxApps on missing fails");
    assertTrue(!sys.setCurrentMembers("missing", 5), "Current on missing fails");
}

static void testFleetAdApply() {
    std::cout << "\n=== FleetAdvertisement: Apply ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FleetAdvertisementState::FleetType;
    sys.postAd("e1", "Mining Fleet", "Ore ops", FT::Mining);

    assertTrue(sys.applyToFleet("e1", "app1", "Pilot1", "Venture"),
               "Apply succeeds");
    assertTrue(sys.getApplicationCount("e1") == 1, "1 application");
    assertTrue(sys.getPendingCount("e1") == 1, "1 pending");
    assertTrue(sys.hasApplication("e1", "app1"), "Has app1");
    assertTrue(sys.getTotalApplicationsReceived("e1") == 1, "1 received");

    // Duplicate rejected
    assertTrue(!sys.applyToFleet("e1", "app1", "Pilot2", "Barge"),
               "Duplicate app rejected");

    // Empty ID rejected
    assertTrue(!sys.applyToFleet("e1", "", "Pilot2", "Barge"),
               "Empty app ID rejected");

    // Empty pilot name rejected
    assertTrue(!sys.applyToFleet("e1", "app2", "", "Barge"),
               "Empty pilot name rejected");

    assertTrue(!sys.applyToFleet("missing", "app", "P", "S"),
               "Apply on missing fails");
}

static void testFleetAdApplyNotListed() {
    std::cout << "\n=== FleetAdvertisement: ApplyNotListed ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    // Not listed — should reject
    assertTrue(!sys.applyToFleet("e1", "app1", "Pilot1", "Venture"),
               "Apply to unlisted fleet rejected");
}

static void testFleetAdAcceptReject() {
    std::cout << "\n=== FleetAdvertisement: Accept/Reject ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FleetAdvertisementState::FleetType;
    sys.postAd("e1", "PvE Fleet", "Running sites", FT::PvE);
    sys.applyToFleet("e1", "app1", "Pilot1", "Drake");
    sys.applyToFleet("e1", "app2", "Pilot2", "Raven");
    sys.applyToFleet("e1", "app3", "Pilot3", "Tengu");

    using AS = components::FleetAdvertisementState::AppStatus;

    assertTrue(sys.acceptApplication("e1", "app1"), "Accept app1");
    assertTrue(sys.getApplicationStatus("e1", "app1") == AS::Accepted,
               "app1 accepted");
    assertTrue(sys.getTotalAccepted("e1") == 1, "1 accepted");
    assertTrue(sys.getCurrentMembers("e1") == 1, "1 member (auto-increment)");
    assertTrue(!sys.acceptApplication("e1", "app1"), "Double-accept rejected");

    assertTrue(sys.rejectApplication("e1", "app2"), "Reject app2");
    assertTrue(sys.getApplicationStatus("e1", "app2") == AS::Rejected,
               "app2 rejected");
    assertTrue(sys.getTotalRejected("e1") == 1, "1 rejected");
    assertTrue(!sys.rejectApplication("e1", "app2"), "Double-reject rejected");

    // Cannot accept already-rejected
    assertTrue(!sys.acceptApplication("e1", "app2"),
               "Cannot accept rejected app");

    // Non-existent app
    assertTrue(!sys.acceptApplication("e1", "nonexistent"), "Accept non-existent fails");
    assertTrue(!sys.rejectApplication("e1", "nonexistent"), "Reject non-existent fails");

    assertTrue(sys.getPendingCount("e1") == 1, "1 pending (app3)");

    assertTrue(!sys.acceptApplication("missing", "app3"), "Accept on missing fails");
    assertTrue(!sys.rejectApplication("missing", "app3"), "Reject on missing fails");
}

static void testFleetAdRemoveClear() {
    std::cout << "\n=== FleetAdvertisement: Remove/Clear ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FleetAdvertisementState::FleetType;
    sys.postAd("e1", "Fleet", "Desc", FT::PvE);
    sys.applyToFleet("e1", "app1", "P1", "S1");
    sys.applyToFleet("e1", "app2", "P2", "S2");

    assertTrue(sys.removeApplication("e1", "app1"), "Remove app1");
    assertTrue(sys.getApplicationCount("e1") == 1, "1 app remaining");
    assertTrue(!sys.removeApplication("e1", "app1"), "Remove non-existent fails");

    assertTrue(sys.clearApplications("e1"), "Clear all");
    assertTrue(sys.getApplicationCount("e1") == 0, "0 apps");

    assertTrue(!sys.removeApplication("missing", "app"), "Remove on missing fails");
    assertTrue(!sys.clearApplications("missing"), "Clear on missing fails");
}

static void testFleetAdCapacity() {
    std::cout << "\n=== FleetAdvertisement: Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxApplications("e1", 2);

    using FT = components::FleetAdvertisementState::FleetType;
    sys.postAd("e1", "Fleet", "Desc", FT::PvE);

    assertTrue(sys.applyToFleet("e1", "a1", "P1", "S1"), "Apply a1");
    assertTrue(sys.applyToFleet("e1", "a2", "P2", "S2"), "Apply a2");
    assertTrue(!sys.applyToFleet("e1", "a3", "P3", "S3"), "At capacity rejected");
    assertTrue(sys.getApplicationCount("e1") == 2, "Still 2 apps");
}

static void testFleetAdTtlExpiry() {
    std::cout << "\n=== FleetAdvertisement: TTL Expiry ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setTtl("e1", 100.0f);

    using FT = components::FleetAdvertisementState::FleetType;
    sys.postAd("e1", "Fleet", "Desc", FT::Hauling);
    assertTrue(sys.isListed("e1"), "Listed after post");

    sys.update(50.0f);
    assertTrue(sys.isListed("e1"), "Still listed at 50s");
    assertTrue(approxEqual(sys.getTimeRemaining("e1"), 50.0f), "50s remaining");

    sys.update(60.0f);
    assertTrue(!sys.isListed("e1"), "Delisted after expiry");
    assertTrue(approxEqual(sys.getTimeRemaining("e1"), 0.0f), "0s remaining");
}

static void testFleetAdMissing() {
    std::cout << "\n=== FleetAdvertisement: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetAdvertisementSystem sys(&world);

    using FT = components::FleetAdvertisementState::FleetType;
    using AS = components::FleetAdvertisementState::AppStatus;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.postAd("m", "T", "D", FT::PvE), "PostAd fails");
    assertTrue(!sys.delistAd("m"), "Delist fails");
    assertTrue(!sys.setFleetId("m", "f"), "SetFleetId fails");
    assertTrue(!sys.setBossName("m", "b"), "SetBossName fails");
    assertTrue(!sys.setTtl("m", 100.0f), "SetTtl fails");
    assertTrue(!sys.setMinMembers("m", 5), "SetMinMembers fails");
    assertTrue(!sys.setMaxMembers("m", 50), "SetMaxMembers fails");
    assertTrue(!sys.setMaxApplications("m", 10), "SetMaxApplications fails");
    assertTrue(!sys.setCurrentMembers("m", 5), "SetCurrentMembers fails");
    assertTrue(!sys.applyToFleet("m", "a", "p", "s"), "ApplyToFleet fails");
    assertTrue(!sys.acceptApplication("m", "a"), "AcceptApp fails");
    assertTrue(!sys.rejectApplication("m", "a"), "RejectApp fails");
    assertTrue(!sys.removeApplication("m", "a"), "RemoveApp fails");
    assertTrue(!sys.clearApplications("m"), "ClearApps fails");
    assertTrue(sys.getApplicationCount("m") == 0, "getAppCount returns 0");
    assertTrue(sys.getPendingCount("m") == 0, "getPendingCount returns 0");
    assertTrue(!sys.hasApplication("m", "a"), "hasApp returns false");
    assertTrue(!sys.isListed("m"), "isListed returns false");
    assertTrue(approxEqual(sys.getTimeRemaining("m"), 0.0f), "getTimeRemaining returns 0");
    assertTrue(sys.getTitle("m") == "", "getTitle returns empty");
    assertTrue(sys.getBossName("m") == "", "getBossName returns empty");
    assertTrue(sys.getFleetId("m") == "", "getFleetId returns empty");
    assertTrue(sys.getCurrentMembers("m") == 0, "getCurrentMembers returns 0");
    assertTrue(sys.getMaxMembers("m") == 0, "getMaxMembers returns 0");
    assertTrue(sys.getTotalAdsPosted("m") == 0, "getTotalAdsPosted returns 0");
    assertTrue(sys.getTotalApplicationsReceived("m") == 0, "getTotalReceived returns 0");
    assertTrue(sys.getTotalAccepted("m") == 0, "getTotalAccepted returns 0");
    assertTrue(sys.getTotalRejected("m") == 0, "getTotalRejected returns 0");
    assertTrue(sys.getFleetType("m") == FT::PvE, "getFleetType returns PvE");
    assertTrue(sys.getApplicationStatus("m", "a") == AS::Pending, "getAppStatus returns Pending");
}

void run_fleet_advertisement_system_tests() {
    testFleetAdInit();
    testFleetAdPostDelist();
    testFleetAdEmptyTitle();
    testFleetAdConfiguration();
    testFleetAdApply();
    testFleetAdApplyNotListed();
    testFleetAdAcceptReject();
    testFleetAdRemoveClear();
    testFleetAdCapacity();
    testFleetAdTtlExpiry();
    testFleetAdMissing();
}
