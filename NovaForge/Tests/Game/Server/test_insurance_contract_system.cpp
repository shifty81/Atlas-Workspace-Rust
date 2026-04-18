// Tests for: InsuranceContractSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/insurance_contract_system.h"

using namespace atlas;
using IT = components::InsuranceContractState::InsuranceTier;
using CS = components::InsuranceContractState::ContractStatus;

// ==================== InsuranceContractSystem Tests ====================

static void testInsuranceContractInit() {
    std::cout << "\n=== InsuranceContract: Init ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getContractCount("e1") == 0, "Zero contracts initially");
    assertTrue(approxEqual(sys.getTotalPremiumsPaid("e1"), 0.0f), "Zero premiums initially");
    assertTrue(approxEqual(sys.getTotalPayoutsReceived("e1"), 0.0f), "Zero payouts initially");
    assertTrue(sys.getTotalClaims("e1") == 0, "Zero claims initially");
    assertTrue(sys.getOwnerId("e1").empty(), "Empty owner initially");
    assertTrue(!sys.hasContract("e1", "c1"), "No contract c1");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testInsuranceContractPurchase() {
    std::cout << "\n=== InsuranceContract: Purchase ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.purchaseContract("e1", "c1", "ship1", IT::Basic, 1000.0f),
        "Purchase basic contract");
    assertTrue(sys.getContractCount("e1") == 1, "1 contract");
    assertTrue(sys.hasContract("e1", "c1"), "Has c1");
    assertTrue(sys.isContractActive("e1", "c1"), "c1 is active");
    // Basic premium = 0.1 * 1000 = 100
    assertTrue(approxEqual(sys.getTotalPremiumsPaid("e1"), 100.0f), "Premium = 100");
    // Basic payout = 0.4 * 1000 = 400
    assertTrue(approxEqual(sys.getContractPayout("e1", "c1"), 400.0f), "Payout = 400");

    // Second contract
    assertTrue(
        sys.purchaseContract("e1", "c2", "ship2", IT::Gold, 2000.0f),
        "Purchase gold contract");
    assertTrue(sys.getContractCount("e1") == 2, "2 contracts");
    // Gold premium = 0.5 * 2000 = 1000; total = 100 + 1000 = 1100
    assertTrue(approxEqual(sys.getTotalPremiumsPaid("e1"), 1100.0f), "Total premium = 1100");

    // Duplicate rejected
    assertTrue(!sys.purchaseContract("e1", "c1", "ship3", IT::Silver, 500.0f),
               "Duplicate contract id rejected");
    assertTrue(sys.getContractCount("e1") == 2, "Still 2 contracts");

    // Validation
    assertTrue(!sys.purchaseContract("e1", "", "ship3", IT::Basic, 500.0f),
               "Empty contract id rejected");
    assertTrue(!sys.purchaseContract("e1", "c3", "", IT::Basic, 500.0f),
               "Empty ship id rejected");
    assertTrue(!sys.purchaseContract("e1", "c3", "ship3", IT::Basic, 0.0f),
               "Zero base value rejected");
    assertTrue(!sys.purchaseContract("e1", "c3", "ship3", IT::Basic, -100.0f),
               "Negative base value rejected");
    assertTrue(!sys.purchaseContract("missing", "c3", "ship3", IT::Basic, 500.0f),
               "Purchase on missing entity fails");
}

static void testInsuranceContractFileClaim() {
    std::cout << "\n=== InsuranceContract: FileClaim ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.purchaseContract("e1", "c1", "ship1", IT::Silver, 1000.0f);
    // Silver payout = 0.7 * 1000 = 700
    assertTrue(sys.fileClaim("e1", "c1"), "File claim succeeds");
    assertTrue(!sys.isContractActive("e1", "c1"), "c1 no longer active");
    assertTrue(approxEqual(sys.getTotalPayoutsReceived("e1"), 700.0f), "Payout = 700");
    assertTrue(sys.getTotalClaims("e1") == 1, "1 claim");

    // Claim already claimed
    assertTrue(!sys.fileClaim("e1", "c1"), "Cannot claim again");
    assertTrue(sys.getTotalClaims("e1") == 1, "Still 1 claim");

    // Claim expired contract
    sys.purchaseContract("e1", "c2", "ship2", IT::Basic, 500.0f);
    sys.cancelContract("e1", "c2");
    assertTrue(!sys.fileClaim("e1", "c2"), "Cannot claim cancelled contract");

    // Claim missing contract
    assertTrue(!sys.fileClaim("e1", "nonexistent"), "Claim missing contract fails");
    assertTrue(!sys.fileClaim("missing", "c1"), "Claim on missing entity fails");

    // Claim expired
    sys.purchaseContract("e1", "c3", "ship3", IT::Basic, 500.0f);
    // Force expiry via update
    sys.update(700000.0f);
    assertTrue(!sys.isContractActive("e1", "c3"), "c3 expired");
    assertTrue(!sys.fileClaim("e1", "c3"), "Cannot claim expired contract");
}

static void testInsuranceContractCancel() {
    std::cout << "\n=== InsuranceContract: Cancel ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.purchaseContract("e1", "c1", "ship1", IT::Standard, 800.0f);
    assertTrue(sys.cancelContract("e1", "c1"), "Cancel succeeds");
    assertTrue(!sys.isContractActive("e1", "c1"), "c1 no longer active");

    // Cancel already cancelled
    assertTrue(!sys.cancelContract("e1", "c1"), "Cannot cancel again");

    // Cancel claimed contract
    sys.purchaseContract("e1", "c2", "ship2", IT::Bronze, 1000.0f);
    sys.fileClaim("e1", "c2");
    assertTrue(!sys.cancelContract("e1", "c2"), "Cannot cancel claimed contract");

    // Cancel missing
    assertTrue(!sys.cancelContract("e1", "nonexistent"), "Cancel missing fails");
    assertTrue(!sys.cancelContract("missing", "c1"), "Cancel on missing entity fails");

    // Verify contract still exists after cancel
    assertTrue(sys.hasContract("e1", "c1"), "Cancelled contract still in list");
    assertTrue(sys.getContractCount("e1") == 2, "2 contracts remain");
}

static void testInsuranceContractExpiry() {
    std::cout << "\n=== InsuranceContract: Expiry ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.purchaseContract("e1", "c1", "ship1", IT::Basic, 1000.0f);
    assertTrue(sys.isContractActive("e1", "c1"), "c1 active before expiry");

    // Update with enough time to expire (> 604800)
    sys.update(604801.0f);
    assertTrue(!sys.isContractActive("e1", "c1"), "c1 expired after update");
    assertTrue(sys.hasContract("e1", "c1"), "c1 still in list");
    assertTrue(sys.getCountByStatus("e1", CS::Expired) == 1, "1 expired");
    assertTrue(sys.getCountByStatus("e1", CS::Active) == 0, "0 active");

    // Cannot claim expired
    assertTrue(!sys.fileClaim("e1", "c1"), "Cannot claim expired");
}

static void testInsuranceContractRemove() {
    std::cout << "\n=== InsuranceContract: Remove ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.purchaseContract("e1", "c1", "ship1", IT::Basic, 500.0f);
    sys.purchaseContract("e1", "c2", "ship2", IT::Gold, 1000.0f);
    assertTrue(sys.removeContract("e1", "c1"), "Remove c1 succeeds");
    assertTrue(!sys.hasContract("e1", "c1"), "c1 gone");
    assertTrue(sys.getContractCount("e1") == 1, "1 contract left");
    assertTrue(!sys.removeContract("e1", "c1"), "Remove again fails");
    assertTrue(!sys.removeContract("missing", "c2"), "Remove on missing entity fails");

    // Clear all
    assertTrue(sys.clearContracts("e1"), "Clear succeeds");
    assertTrue(sys.getContractCount("e1") == 0, "0 contracts after clear");
}

static void testInsuranceContractTierPayouts() {
    std::cout << "\n=== InsuranceContract: Tier Payouts ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    float base = 1000.0f;

    // Basic: payout 0.4, premium 0.1
    sys.purchaseContract("e1", "t1", "s1", IT::Basic, base);
    assertTrue(approxEqual(sys.getContractPayout("e1", "t1"), 400.0f), "Basic payout = 400");
    assertTrue(approxEqual(systems::InsuranceContractSystem::getPremiumForTier(IT::Basic, base), 100.0f),
               "Basic premium = 100");

    // Standard: payout 0.5, premium 0.15
    sys.purchaseContract("e1", "t2", "s2", IT::Standard, base);
    assertTrue(approxEqual(sys.getContractPayout("e1", "t2"), 500.0f), "Standard payout = 500");
    assertTrue(approxEqual(systems::InsuranceContractSystem::getPremiumForTier(IT::Standard, base), 150.0f),
               "Standard premium = 150");

    // Bronze: payout 0.6, premium 0.2
    sys.purchaseContract("e1", "t3", "s3", IT::Bronze, base);
    assertTrue(approxEqual(sys.getContractPayout("e1", "t3"), 600.0f), "Bronze payout = 600");
    assertTrue(approxEqual(systems::InsuranceContractSystem::getPremiumForTier(IT::Bronze, base), 200.0f),
               "Bronze premium = 200");

    // Silver: payout 0.7, premium 0.3
    sys.purchaseContract("e1", "t4", "s4", IT::Silver, base);
    assertTrue(approxEqual(sys.getContractPayout("e1", "t4"), 700.0f), "Silver payout = 700");
    assertTrue(approxEqual(systems::InsuranceContractSystem::getPremiumForTier(IT::Silver, base), 300.0f),
               "Silver premium = 300");

    // Gold: payout 0.85, premium 0.5
    sys.purchaseContract("e1", "t5", "s5", IT::Gold, base);
    assertTrue(approxEqual(sys.getContractPayout("e1", "t5"), 850.0f), "Gold payout = 850");
    assertTrue(approxEqual(systems::InsuranceContractSystem::getPremiumForTier(IT::Gold, base), 500.0f),
               "Gold premium = 500");

    // Platinum: payout 1.0, premium 0.8
    sys.purchaseContract("e1", "t6", "s6", IT::Platinum, base);
    assertTrue(approxEqual(sys.getContractPayout("e1", "t6"), 1000.0f), "Platinum payout = 1000");
    assertTrue(approxEqual(systems::InsuranceContractSystem::getPremiumForTier(IT::Platinum, base), 800.0f),
               "Platinum premium = 800");
}

static void testInsuranceContractConfiguration() {
    std::cout << "\n=== InsuranceContract: Configuration ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setOwnerId("e1", "player1"), "Set owner succeeds");
    assertTrue(sys.getOwnerId("e1") == "player1", "Owner is player1");
    assertTrue(!sys.setOwnerId("e1", ""), "Empty owner rejected");
    assertTrue(sys.getOwnerId("e1") == "player1", "Owner unchanged");

    assertTrue(sys.setMaxContracts("e1", 5), "Set max contracts succeeds");
    assertTrue(!sys.setMaxContracts("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxContracts("e1", -1), "Negative max rejected");

    assertTrue(!sys.setOwnerId("missing", "player1"), "Set owner on missing fails");
    assertTrue(!sys.setMaxContracts("missing", 5), "Set max on missing fails");
}

static void testInsuranceContractCountByTierStatus() {
    std::cout << "\n=== InsuranceContract: CountByTier/Status ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.purchaseContract("e1", "c1", "s1", IT::Basic, 500.0f);
    sys.purchaseContract("e1", "c2", "s2", IT::Basic, 600.0f);
    sys.purchaseContract("e1", "c3", "s3", IT::Gold, 1000.0f);
    sys.purchaseContract("e1", "c4", "s4", IT::Gold, 2000.0f);
    sys.purchaseContract("e1", "c5", "s5", IT::Platinum, 3000.0f);

    assertTrue(sys.getCountByTier("e1", IT::Basic) == 2, "2 Basic");
    assertTrue(sys.getCountByTier("e1", IT::Gold) == 2, "2 Gold");
    assertTrue(sys.getCountByTier("e1", IT::Platinum) == 1, "1 Platinum");
    assertTrue(sys.getCountByTier("e1", IT::Silver) == 0, "0 Silver");

    sys.cancelContract("e1", "c1");
    sys.fileClaim("e1", "c3");
    assertTrue(sys.getCountByStatus("e1", CS::Active) == 3, "3 active");
    assertTrue(sys.getCountByStatus("e1", CS::Cancelled) == 1, "1 cancelled");
    assertTrue(sys.getCountByStatus("e1", CS::ClaimedPaid) == 1, "1 claimed");
}

static void testInsuranceContractCapacity() {
    std::cout << "\n=== InsuranceContract: Capacity ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setMaxContracts("e1", 3);
    assertTrue(sys.purchaseContract("e1", "c1", "s1", IT::Basic, 100.0f), "Purchase 1");
    assertTrue(sys.purchaseContract("e1", "c2", "s2", IT::Basic, 100.0f), "Purchase 2");
    assertTrue(sys.purchaseContract("e1", "c3", "s3", IT::Basic, 100.0f), "Purchase 3");
    assertTrue(!sys.purchaseContract("e1", "c4", "s4", IT::Basic, 100.0f),
               "Purchase 4 rejected at capacity");
    assertTrue(sys.getContractCount("e1") == 3, "Still 3 contracts");
}

static void testInsuranceContractMissing() {
    std::cout << "\n=== InsuranceContract: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::InsuranceContractSystem sys(&world);

    assertTrue(sys.getContractCount("missing") == 0, "Missing: count = 0");
    assertTrue(!sys.hasContract("missing", "c1"), "Missing: hasContract = false");
    assertTrue(!sys.isContractActive("missing", "c1"), "Missing: isActive = false");
    assertTrue(approxEqual(sys.getContractPayout("missing", "c1"), 0.0f), "Missing: payout = 0");
    assertTrue(sys.getOwnerId("missing").empty(), "Missing: owner empty");
    assertTrue(approxEqual(sys.getTotalPremiumsPaid("missing"), 0.0f), "Missing: premiums = 0");
    assertTrue(approxEqual(sys.getTotalPayoutsReceived("missing"), 0.0f), "Missing: payouts = 0");
    assertTrue(sys.getTotalClaims("missing") == 0, "Missing: claims = 0");
    assertTrue(sys.getCountByTier("missing", IT::Basic) == 0, "Missing: countByTier = 0");
    assertTrue(sys.getCountByStatus("missing", CS::Active) == 0, "Missing: countByStatus = 0");
    assertTrue(!sys.purchaseContract("missing", "c1", "s1", IT::Basic, 100.0f),
               "Missing: purchase fails");
    assertTrue(!sys.fileClaim("missing", "c1"), "Missing: fileClaim fails");
    assertTrue(!sys.cancelContract("missing", "c1"), "Missing: cancel fails");
    assertTrue(!sys.removeContract("missing", "c1"), "Missing: remove fails");
    assertTrue(!sys.clearContracts("missing"), "Missing: clear fails");
}

void run_insurance_contract_system_tests() {
    testInsuranceContractInit();
    testInsuranceContractPurchase();
    testInsuranceContractFileClaim();
    testInsuranceContractCancel();
    testInsuranceContractExpiry();
    testInsuranceContractRemove();
    testInsuranceContractTierPayouts();
    testInsuranceContractConfiguration();
    testInsuranceContractCountByTierStatus();
    testInsuranceContractCapacity();
    testInsuranceContractMissing();
}
