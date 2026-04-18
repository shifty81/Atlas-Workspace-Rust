// Tests for: WalletJournalSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/wallet_journal_system.h"

using namespace atlas;

// ==================== WalletJournalSystem Tests ====================

static void testWalletJournalInit() {
    std::cout << "\n=== WalletJournal: Init ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getEntryCount("e1") == 0, "Zero entries");
    assertTrue(approxEqual(sys.getBalance("e1"), 0.0f), "Zero balance");
    assertTrue(sys.getOwner("e1") == "", "No owner initially");
    assertTrue(sys.getTotalEntriesEver("e1") == 0, "Zero total entries");
    assertTrue(approxEqual(sys.getTotalCredits("e1"), 0.0f), "Zero credits");
    assertTrue(approxEqual(sys.getTotalDebits("e1"), 0.0f), "Zero debits");
    assertTrue(approxEqual(sys.getNetFlow("e1"), 0.0f), "Zero net flow");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testWalletJournalAddEntry() {
    std::cout << "\n=== WalletJournal: AddEntry ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using TT = components::WalletJournalState::TransactionType;

    // Add a credit (bounty payment)
    assertTrue(sys.addEntry("e1", "e1", TT::Bounty, 10000.0f, "Rat bounty", "NPC"),
               "Add bounty credit");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry");
    assertTrue(sys.hasEntry("e1", "e1"), "Has entry e1");
    assertTrue(approxEqual(sys.getBalance("e1"), 10000.0f), "Balance 10000");
    assertTrue(approxEqual(sys.getTotalCredits("e1"), 10000.0f), "Credits 10000");
    assertTrue(approxEqual(sys.getTotalDebits("e1"), 0.0f), "Debits still 0");
    assertTrue(sys.getTotalEntriesEver("e1") == 1, "1 total ever");

    // Add a debit (market purchase)
    assertTrue(sys.addEntry("e1", "e2", TT::MarketBuy, -3000.0f, "Buy ammo", "Market"),
               "Add market debit");
    assertTrue(sys.getEntryCount("e1") == 2, "2 entries");
    assertTrue(approxEqual(sys.getBalance("e1"), 7000.0f), "Balance 7000");
    assertTrue(approxEqual(sys.getTotalCredits("e1"), 10000.0f), "Credits still 10000");
    assertTrue(approxEqual(sys.getTotalDebits("e1"), 3000.0f), "Debits 3000");
    assertTrue(approxEqual(sys.getNetFlow("e1"), 7000.0f), "Net flow 7000");

    // Duplicate rejected
    assertTrue(!sys.addEntry("e1", "e1", TT::Bounty, 5000.0f, "Dupe", "X"),
               "Duplicate entry rejected");

    // Empty ID rejected
    assertTrue(!sys.addEntry("e1", "", TT::Bounty, 5000.0f, "X", "X"),
               "Empty ID rejected");

    // Zero amount rejected
    assertTrue(!sys.addEntry("e1", "e3", TT::Other, 0.0f, "Zero", "X"),
               "Zero amount rejected");

    assertTrue(!sys.addEntry("missing", "e9", TT::Bounty, 100.0f, "X", "X"),
               "Add on missing fails");
}

static void testWalletJournalRemoveEntry() {
    std::cout << "\n=== WalletJournal: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using TT = components::WalletJournalState::TransactionType;
    sys.addEntry("e1", "e1", TT::Bounty, 5000.0f, "Bounty", "NPC");
    sys.addEntry("e1", "e2", TT::MarketSell, 3000.0f, "Sell ore", "Market");

    assertTrue(sys.removeEntry("e1", "e1"), "Remove e1");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry remaining");
    assertTrue(!sys.hasEntry("e1", "e1"), "e1 gone");
    assertTrue(sys.hasEntry("e1", "e2"), "e2 still there");

    assertTrue(!sys.removeEntry("e1", "e1"), "Remove non-existent fails");
    assertTrue(!sys.removeEntry("missing", "e2"), "Remove on missing fails");
}

static void testWalletJournalClear() {
    std::cout << "\n=== WalletJournal: Clear ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using TT = components::WalletJournalState::TransactionType;
    sys.addEntry("e1", "e1", TT::Bounty, 5000.0f, "X", "Y");
    sys.addEntry("e1", "e2", TT::MissionReward, 2000.0f, "X", "Y");

    assertTrue(sys.clearJournal("e1"), "Clear journal");
    assertTrue(sys.getEntryCount("e1") == 0, "Zero entries");
    // Balance and totals preserved
    assertTrue(approxEqual(sys.getBalance("e1"), 7000.0f), "Balance preserved");
    assertTrue(!sys.clearJournal("missing"), "Clear on missing fails");
}

static void testWalletJournalConfiguration() {
    std::cout << "\n=== WalletJournal: Configuration ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setOwner("e1", "player1"), "Set owner");
    assertTrue(sys.getOwner("e1") == "player1", "Owner correct");

    assertTrue(sys.setMaxEntries("e1", 50), "Set max entries 50");
    assertTrue(!sys.setMaxEntries("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxEntries("e1", -1), "Negative max rejected");

    assertTrue(sys.setBalance("e1", 100000.0f), "Set balance");
    assertTrue(approxEqual(sys.getBalance("e1"), 100000.0f), "Balance 100000");

    assertTrue(!sys.setOwner("missing", "x"), "Owner on missing fails");
    assertTrue(!sys.setMaxEntries("missing", 50), "MaxEntries on missing fails");
    assertTrue(!sys.setBalance("missing", 100.0f), "Balance on missing fails");
}

static void testWalletJournalAutoPurge() {
    std::cout << "\n=== WalletJournal: AutoPurge ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxEntries("e1", 3);

    using TT = components::WalletJournalState::TransactionType;
    sys.addEntry("e1", "e1", TT::Bounty, 1000.0f, "First", "");
    sys.addEntry("e1", "e2", TT::Bounty, 2000.0f, "Second", "");
    sys.addEntry("e1", "e3", TT::Bounty, 3000.0f, "Third", "");
    assertTrue(sys.getEntryCount("e1") == 3, "3 entries at cap");

    // Adding a 4th should purge the oldest
    assertTrue(sys.addEntry("e1", "e4", TT::Bounty, 4000.0f, "Fourth", ""),
               "Add 4th entry (auto-purge)");
    assertTrue(sys.getEntryCount("e1") == 3, "Still 3 entries");
    assertTrue(!sys.hasEntry("e1", "e1"), "First entry purged");
    assertTrue(sys.hasEntry("e1", "e4"), "Fourth entry present");
    assertTrue(sys.getTotalEntriesEver("e1") == 4, "4 total ever");
}

static void testWalletJournalCreditDebitTracking() {
    std::cout << "\n=== WalletJournal: Credit/Debit Tracking ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using TT = components::WalletJournalState::TransactionType;

    sys.addEntry("e1", "e1", TT::Bounty, 10000.0f, "Bounty", "NPC");
    sys.addEntry("e1", "e2", TT::MissionReward, 5000.0f, "Mission", "Agent");
    sys.addEntry("e1", "e3", TT::MarketBuy, -3000.0f, "Buy ship", "Market");
    sys.addEntry("e1", "e4", TT::RepairCost, -1000.0f, "Repair", "Station");
    sys.addEntry("e1", "e5", TT::InsurancePayout, 8000.0f, "Insurance", "Corp");

    assertTrue(approxEqual(sys.getBalance("e1"), 19000.0f), "Balance 19000");
    assertTrue(approxEqual(sys.getTotalCredits("e1"), 23000.0f), "Total credits 23000");
    assertTrue(approxEqual(sys.getTotalDebits("e1"), 4000.0f), "Total debits 4000");
    assertTrue(approxEqual(sys.getNetFlow("e1"), 19000.0f), "Net flow 19000");
}

static void testWalletJournalCountByType() {
    std::cout << "\n=== WalletJournal: CountByType ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using TT = components::WalletJournalState::TransactionType;

    sys.addEntry("e1", "e1", TT::Bounty, 1000.0f, "B1", "");
    sys.addEntry("e1", "e2", TT::Bounty, 2000.0f, "B2", "");
    sys.addEntry("e1", "e3", TT::MarketSell, 5000.0f, "Sell", "");
    sys.addEntry("e1", "e4", TT::MarketBuy, -3000.0f, "Buy", "");
    sys.addEntry("e1", "e5", TT::RepairCost, -500.0f, "Repair", "");

    assertTrue(sys.getCountByType("e1", TT::Bounty) == 2, "2 bounty entries");
    assertTrue(sys.getCountByType("e1", TT::MarketSell) == 1, "1 market sell");
    assertTrue(sys.getCountByType("e1", TT::MarketBuy) == 1, "1 market buy");
    assertTrue(sys.getCountByType("e1", TT::RepairCost) == 1, "1 repair");
    assertTrue(sys.getCountByType("e1", TT::MissionReward) == 0, "0 mission");

    assertTrue(approxEqual(sys.getCreditsByType("e1", TT::Bounty), 3000.0f),
               "Bounty credits 3000");
    assertTrue(approxEqual(sys.getCreditsByType("e1", TT::MarketSell), 5000.0f),
               "MarketSell credits 5000");
    assertTrue(approxEqual(sys.getDebitsByType("e1", TT::MarketBuy), 3000.0f),
               "MarketBuy debits 3000");
    assertTrue(approxEqual(sys.getDebitsByType("e1", TT::RepairCost), 500.0f),
               "Repair debits 500");

    assertTrue(sys.getCountByType("missing", TT::Bounty) == 0, "CountByType on missing");
    assertTrue(approxEqual(sys.getCreditsByType("missing", TT::Bounty), 0.0f),
               "CreditsByType on missing");
    assertTrue(approxEqual(sys.getDebitsByType("missing", TT::Bounty), 0.0f),
               "DebitsByType on missing");
}

static void testWalletJournalUpdate() {
    std::cout << "\n=== WalletJournal: Update ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Update just advances elapsed, no special behavior
    sys.update(5.0f);
    sys.update(10.0f);
    // No crash, just verify it still works
    assertTrue(sys.getEntryCount("e1") == 0, "Still zero entries after update");
}

static void testWalletJournalMissing() {
    std::cout << "\n=== WalletJournal: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::WalletJournalSystem sys(&world);

    using TT = components::WalletJournalState::TransactionType;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.addEntry("m", "e", TT::Bounty, 100.0f, "X", "Y"), "AddEntry fails");
    assertTrue(!sys.removeEntry("m", "e"), "RemoveEntry fails");
    assertTrue(!sys.clearJournal("m"), "ClearJournal fails");
    assertTrue(!sys.setOwner("m", "x"), "SetOwner fails");
    assertTrue(!sys.setMaxEntries("m", 50), "SetMaxEntries fails");
    assertTrue(!sys.setBalance("m", 100.0f), "SetBalance fails");
    assertTrue(sys.getEntryCount("m") == 0, "getEntryCount returns 0");
    assertTrue(!sys.hasEntry("m", "e"), "hasEntry returns false");
    assertTrue(approxEqual(sys.getBalance("m"), 0.0f), "getBalance returns 0");
    assertTrue(sys.getOwner("m") == "", "getOwner returns empty");
    assertTrue(sys.getTotalEntriesEver("m") == 0, "getTotalEntriesEver returns 0");
    assertTrue(approxEqual(sys.getTotalCredits("m"), 0.0f), "getTotalCredits returns 0");
    assertTrue(approxEqual(sys.getTotalDebits("m"), 0.0f), "getTotalDebits returns 0");
    assertTrue(approxEqual(sys.getNetFlow("m"), 0.0f), "getNetFlow returns 0");
    assertTrue(approxEqual(sys.getCreditsByType("m", TT::Bounty), 0.0f), "getCreditsByType returns 0");
    assertTrue(approxEqual(sys.getDebitsByType("m", TT::Bounty), 0.0f), "getDebitsByType returns 0");
    assertTrue(sys.getCountByType("m", TT::Bounty) == 0, "getCountByType returns 0");
}

void run_wallet_journal_system_tests() {
    testWalletJournalInit();
    testWalletJournalAddEntry();
    testWalletJournalRemoveEntry();
    testWalletJournalClear();
    testWalletJournalConfiguration();
    testWalletJournalAutoPurge();
    testWalletJournalCreditDebitTracking();
    testWalletJournalCountByType();
    testWalletJournalUpdate();
    testWalletJournalMissing();
}
