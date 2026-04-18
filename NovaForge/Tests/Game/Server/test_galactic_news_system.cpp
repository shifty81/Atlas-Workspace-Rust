// Tests for: GalacticNewsSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/galactic_news_system.h"

using namespace atlas;
using NC = components::GalacticNewsState::NewsCategory;

static void testGalacticNewsInit() {
    std::cout << "\n=== GalacticNews: Init ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getNewsCount("e1") == 0, "Zero news entries");
    assertTrue(sys.getActiveCount("e1") == 0, "Zero active entries");
    assertTrue(sys.getTotalPublished("e1") == 0, "Zero total_published");
    assertTrue(sys.getTotalExpired("e1") == 0, "Zero total_expired");
    assertTrue(sys.getSystemId("e1").empty(), "Empty system_id");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 1.0f), "Default decay rate 1.0");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testGalacticNewsPublish() {
    std::cout << "\n=== GalacticNews: Publish ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.publishNews("e1", "n1", "War breaks out", NC::Conflict, "SolSystem"), "Publish n1");
    assertTrue(sys.hasNews("e1", "n1"), "hasNews n1");
    assertTrue(sys.getNewsCount("e1") == 1, "1 news entry");
    assertTrue(sys.getHeadline("e1", "n1") == "War breaks out", "Headline matches");
    assertTrue(sys.getNewsCategory("e1", "n1") == NC::Conflict, "Category Conflict");
    assertTrue(sys.getSourceSystem("e1", "n1") == "SolSystem", "Source system matches");
    assertTrue(sys.getTotalPublished("e1") == 1, "total_published = 1");
    assertTrue(approxEqual(sys.getNewsAge("e1", "n1"), 0.0f), "Age starts at 0");
    assertTrue(!sys.isExpired("e1", "n1"), "Not expired");
}

static void testGalacticNewsPublishValidation() {
    std::cout << "\n=== GalacticNews: PublishValidation ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.publishNews("e1", "", "headline", NC::Economic, "src"), "Empty id rejected");
    assertTrue(!sys.publishNews("e1", "n1", "", NC::Economic, "src"), "Empty headline rejected");
    assertTrue(sys.publishNews("e1", "n1", "headline", NC::Economic, "src"), "Valid publish");
    assertTrue(!sys.publishNews("e1", "n1", "other", NC::Economic, "src"), "Duplicate id rejected");
    assertTrue(sys.getNewsCount("e1") == 1, "1 news entry");
    assertTrue(!sys.publishNews("missing", "n2", "h", NC::Economic, "s"), "Publish on missing fails");
}

static void testGalacticNewsCapacity() {
    std::cout << "\n=== GalacticNews: Capacity ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxEntries("e1", 3);

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.publishNews("e1", "n2", "H2", NC::Economic, "s");
    sys.publishNews("e1", "n3", "H3", NC::Political, "s");
    assertTrue(sys.getNewsCount("e1") == 3, "3 entries at capacity");

    // n4 should auto-purge oldest (n1) since none are expired
    sys.publishNews("e1", "n4", "H4", NC::Anomaly, "s");
    assertTrue(sys.getNewsCount("e1") == 3, "Still 3 after auto-purge");
    assertTrue(!sys.hasNews("e1", "n1"), "n1 purged");
    assertTrue(sys.hasNews("e1", "n4"), "n4 present");

    // Now expire n2 and add n5; expired should be purged first
    sys.setExpiryThreshold("e1", 1.0f);
    sys.update(5.0f); // n2, n3, n4 all expire
    sys.publishNews("e1", "n5", "H5", NC::Exploration, "s");
    assertTrue(sys.getNewsCount("e1") == 3, "Still 3 after second auto-purge");
}

static void testGalacticNewsRemove() {
    std::cout << "\n=== GalacticNews: Remove ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.publishNews("e1", "n2", "H2", NC::Economic, "s");
    assertTrue(sys.removeNews("e1", "n1"), "removeNews n1");
    assertTrue(!sys.hasNews("e1", "n1"), "n1 gone");
    assertTrue(sys.getNewsCount("e1") == 1, "1 news entry remains");
    assertTrue(!sys.removeNews("e1", "nonexistent"), "Remove nonexistent fails");
    assertTrue(!sys.removeNews("missing", "n2"), "Remove on missing entity fails");
}

static void testGalacticNewsClear() {
    std::cout << "\n=== GalacticNews: Clear ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.publishNews("e1", "n2", "H2", NC::Economic, "s");
    int pub = sys.getTotalPublished("e1");

    assertTrue(sys.clearNews("e1"), "clearNews succeeds");
    assertTrue(sys.getNewsCount("e1") == 0, "Zero news after clear");
    assertTrue(sys.getTotalPublished("e1") == pub, "total_published preserved");

    assertTrue(!sys.clearNews("missing"), "clearNews on missing fails");
}

static void testGalacticNewsDecayTick() {
    std::cout << "\n=== GalacticNews: DecayTick ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setExpiryThreshold("e1", 10.0f);
    sys.setDecayRate("e1", 1.0f);

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.update(5.0f);
    assertTrue(sys.getNewsAge("e1", "n1") > 0.0f, "Age advanced after update");
    assertTrue(!sys.isExpired("e1", "n1"), "Not expired at age 5 (threshold 10)");

    sys.update(6.0f);
    assertTrue(sys.isExpired("e1", "n1"), "Expired after age >= 10");
    assertTrue(sys.getTotalExpired("e1") == 1, "total_expired = 1");
}

static void testGalacticNewsExpiredCount() {
    std::cout << "\n=== GalacticNews: ExpiredCount ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setExpiryThreshold("e1", 5.0f);

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.publishNews("e1", "n2", "H2", NC::Economic, "s");
    sys.publishNews("e1", "n3", "H3", NC::Political, "s");

    assertTrue(sys.getActiveCount("e1") == 3, "3 active initially");
    sys.update(10.0f);
    assertTrue(sys.getActiveCount("e1") == 0, "0 active after expiry");
    assertTrue(sys.getTotalExpired("e1") == 3, "total_expired = 3");
    assertTrue(sys.getNewsCount("e1") == 3, "Entries still present in list");
}

static void testGalacticNewsCategoryCount() {
    std::cout << "\n=== GalacticNews: CategoryCount ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.publishNews("e1", "n2", "H2", NC::Conflict, "s");
    sys.publishNews("e1", "n3", "H3", NC::Economic, "s");
    sys.publishNews("e1", "n4", "H4", NC::Political, "s");
    sys.publishNews("e1", "n5", "H5", NC::Anomaly, "s");
    sys.publishNews("e1", "n6", "H6", NC::Exploration, "s");
    sys.publishNews("e1", "n7", "H7", NC::Disaster, "s");

    assertTrue(sys.getCountByCategory("e1", NC::Conflict) == 2, "2 Conflict");
    assertTrue(sys.getCountByCategory("e1", NC::Economic) == 1, "1 Economic");
    assertTrue(sys.getCountByCategory("e1", NC::Political) == 1, "1 Political");
    assertTrue(sys.getCountByCategory("e1", NC::Anomaly) == 1, "1 Anomaly");
    assertTrue(sys.getCountByCategory("e1", NC::Exploration) == 1, "1 Exploration");
    assertTrue(sys.getCountByCategory("e1", NC::Disaster) == 1, "1 Disaster");
    assertTrue(sys.getCountByCategory("missing", NC::Conflict) == 0, "CountByCategory on missing = 0");
}

static void testGalacticNewsConfiguration() {
    std::cout << "\n=== GalacticNews: Configuration ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSystemId("e1", "Jita"), "Set system_id");
    assertTrue(sys.getSystemId("e1") == "Jita", "system_id matches");
    assertTrue(!sys.setSystemId("e1", ""), "Empty system_id rejected");
    assertTrue(!sys.setSystemId("missing", "x"), "setSystemId on missing fails");

    assertTrue(sys.setDecayRate("e1", 2.5f), "Set decay rate 2.5");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 2.5f), "Decay rate matches");
    assertTrue(sys.setDecayRate("e1", 0.0f), "Zero decay rate allowed");
    assertTrue(!sys.setDecayRate("e1", -0.1f), "Negative decay rate rejected");
    assertTrue(!sys.setDecayRate("missing", 1.0f), "setDecayRate on missing fails");

    assertTrue(sys.setMaxEntries("e1", 50), "Set max_entries 50");
    assertTrue(!sys.setMaxEntries("e1", 0), "Zero max_entries rejected");
    assertTrue(!sys.setMaxEntries("e1", -1), "Negative max_entries rejected");
    assertTrue(!sys.setMaxEntries("missing", 10), "setMaxEntries on missing fails");

    assertTrue(sys.setExpiryThreshold("e1", 3600.0f), "Set expiry threshold");
    assertTrue(!sys.setExpiryThreshold("e1", 0.0f), "Zero expiry threshold rejected");
    assertTrue(!sys.setExpiryThreshold("e1", -1.0f), "Negative expiry threshold rejected");
    assertTrue(!sys.setExpiryThreshold("missing", 100.0f), "setExpiryThreshold on missing fails");
}

static void testGalacticNewsTotals() {
    std::cout << "\n=== GalacticNews: Totals ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setExpiryThreshold("e1", 1.0f);

    sys.publishNews("e1", "n1", "H1", NC::Conflict, "s");
    sys.publishNews("e1", "n2", "H2", NC::Economic, "s");
    assertTrue(sys.getTotalPublished("e1") == 2, "total_published = 2");

    sys.update(5.0f);
    assertTrue(sys.getTotalExpired("e1") == 2, "total_expired = 2");

    sys.publishNews("e1", "n3", "H3", NC::Political, "s");
    assertTrue(sys.getTotalPublished("e1") == 3, "total_published = 3 after more");
    assertTrue(sys.getTotalExpired("e1") == 2, "total_expired still 2");
}

static void testGalacticNewsMissingEntity() {
    std::cout << "\n=== GalacticNews: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::GalacticNewsSystem sys(&world);

    assertTrue(sys.getNewsCount("missing") == 0, "getNewsCount = 0");
    assertTrue(!sys.hasNews("missing", "n1"), "hasNews = false");
    assertTrue(sys.getHeadline("missing", "n1").empty(), "getHeadline = ''");
    assertTrue(approxEqual(sys.getNewsAge("missing", "n1"), 0.0f), "getNewsAge = 0");
    assertTrue(!sys.isExpired("missing", "n1"), "isExpired = false");
    assertTrue(sys.getActiveCount("missing") == 0, "getActiveCount = 0");
    assertTrue(sys.getCountByCategory("missing", NC::Conflict) == 0, "getCountByCategory = 0");
    assertTrue(sys.getTotalPublished("missing") == 0, "getTotalPublished = 0");
    assertTrue(sys.getTotalExpired("missing") == 0, "getTotalExpired = 0");
    assertTrue(sys.getSystemId("missing").empty(), "getSystemId = ''");
    assertTrue(approxEqual(sys.getDecayRate("missing"), 0.0f), "getDecayRate = 0");
    assertTrue(sys.getSourceSystem("missing", "n1").empty(), "getSourceSystem = ''");
    assertTrue(sys.getNewsCategory("missing", "n1") == NC::Conflict, "getNewsCategory = Conflict default");
    assertTrue(!sys.publishNews("missing", "n1", "h", NC::Conflict, "s"), "publishNews on missing fails");
    assertTrue(!sys.removeNews("missing", "n1"), "removeNews on missing fails");
    assertTrue(!sys.clearNews("missing"), "clearNews on missing fails");
    assertTrue(!sys.setSystemId("missing", "x"), "setSystemId on missing fails");
}

void run_galactic_news_system_tests() {
    testGalacticNewsInit();
    testGalacticNewsPublish();
    testGalacticNewsPublishValidation();
    testGalacticNewsCapacity();
    testGalacticNewsRemove();
    testGalacticNewsClear();
    testGalacticNewsDecayTick();
    testGalacticNewsExpiredCount();
    testGalacticNewsCategoryCount();
    testGalacticNewsConfiguration();
    testGalacticNewsTotals();
    testGalacticNewsMissingEntity();
}
