// Tests for: FleetCargoAggregatorSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_cargo_aggregator_system.h"

using namespace atlas;
using RT = components::FleetCargoAggregatorState::ResourceType;

static void testFleetCargoAggregatorInit() {
    std::cout << "\n=== FleetCargoAggregator: Init ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);

    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getContributionCount("e1") == 0, "No contributions initially");
    assertTrue(sys.getPoolCount("e1") == 0, "No pools initially");
    assertTrue(sys.getFleetId("e1") == "", "Fleet id empty initially");
    assertTrue(sys.getTotalTransfers("e1") == 0, "No transfers initially");
    assertTrue(approxEqual(sys.getTotalQuantityTransferred("e1"), 0.0f), "No quantity transferred initially");
    assertTrue(sys.getTotalContributionsAdded("e1") == 0, "No contributions added initially");
    assertTrue(sys.getMaxContributions("e1") == 100, "Default max contributions is 100");
    assertTrue(sys.getMaxPools("e1") == 20, "Default max pools is 20");
    assertTrue(approxEqual(sys.getTotalFleetQuantity("e1"), 0.0f), "Total fleet quantity is 0");
    assertTrue(approxEqual(sys.getTotalFleetCapacity("e1"), 0.0f), "Total fleet capacity is 0");

    // Missing entity
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetCargoAggregatorAddContribution() {
    std::cout << "\n=== FleetCargoAggregator: AddContribution ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addContribution("e1", "ship1", "Destroyer", RT::Ore, 50.0f, 100.0f),
               "Add contribution succeeds");
    assertTrue(sys.getContributionCount("e1") == 1, "One contribution");
    assertTrue(sys.hasContribution("e1", "ship1"), "Has ship1");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship1"), 50.0f), "Ship1 quantity is 50");
    assertTrue(approxEqual(sys.getContributionCapacity("e1", "ship1"), 100.0f), "Ship1 capacity is 100");
    assertTrue(sys.getShipName("e1", "ship1") == "Destroyer", "Ship1 name is Destroyer");
    assertTrue(sys.getTotalContributionsAdded("e1") == 1, "Total contributions added is 1");

    // Add second contribution
    assertTrue(sys.addContribution("e1", "ship2", "Cruiser", RT::Ice, 30.0f, 80.0f),
               "Add second contribution succeeds");
    assertTrue(sys.getContributionCount("e1") == 2, "Two contributions");
    assertTrue(sys.hasContribution("e1", "ship2"), "Has ship2");

    // Duplicate ship_id rejected
    assertTrue(!sys.addContribution("e1", "ship1", "Other", RT::Gas, 10.0f, 50.0f),
               "Duplicate ship_id rejected");
    assertTrue(sys.getContributionCount("e1") == 2, "Still two contributions after duplicate");

    // Zero quantity is valid
    assertTrue(sys.addContribution("e1", "ship3", "Frigate", RT::Fuel, 0.0f, 50.0f),
               "Zero quantity contribution succeeds");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship3"), 0.0f), "Ship3 quantity is 0");
}

static void testFleetCargoAggregatorAddContributionValidation() {
    std::cout << "\n=== FleetCargoAggregator: AddContribution Validation ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Empty ship_id
    assertTrue(!sys.addContribution("e1", "", "Name", RT::Ore, 10.0f, 50.0f),
               "Empty ship_id rejected");
    // Empty ship_name
    assertTrue(!sys.addContribution("e1", "s1", "", RT::Ore, 10.0f, 50.0f),
               "Empty ship_name rejected");
    // Negative quantity
    assertTrue(!sys.addContribution("e1", "s1", "Name", RT::Ore, -5.0f, 50.0f),
               "Negative quantity rejected");
    // Zero capacity
    assertTrue(!sys.addContribution("e1", "s1", "Name", RT::Ore, 0.0f, 0.0f),
               "Zero capacity rejected");
    // Negative capacity
    assertTrue(!sys.addContribution("e1", "s1", "Name", RT::Ore, 0.0f, -10.0f),
               "Negative capacity rejected");
    // Quantity > capacity
    assertTrue(!sys.addContribution("e1", "s1", "Name", RT::Ore, 60.0f, 50.0f),
               "Quantity > capacity rejected");
    // Missing entity
    assertTrue(!sys.addContribution("nope", "s1", "Name", RT::Ore, 10.0f, 50.0f),
               "Missing entity rejected");

    assertTrue(sys.getContributionCount("e1") == 0, "No contributions added after validation failures");
}

static void testFleetCargoAggregatorRemoveContribution() {
    std::cout << "\n=== FleetCargoAggregator: RemoveContribution ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addContribution("e1", "ship1", "Alpha", RT::Ore, 10.0f, 50.0f);
    sys.addContribution("e1", "ship2", "Beta", RT::Ice, 20.0f, 60.0f);

    assertTrue(sys.removeContribution("e1", "ship1"), "Remove ship1 succeeds");
    assertTrue(sys.getContributionCount("e1") == 1, "One contribution remains");
    assertTrue(!sys.hasContribution("e1", "ship1"), "ship1 no longer present");
    assertTrue(sys.hasContribution("e1", "ship2"), "ship2 still present");

    // Remove non-existent
    assertTrue(!sys.removeContribution("e1", "ship99"), "Remove nonexistent fails");
    // Remove from missing entity
    assertTrue(!sys.removeContribution("nope", "ship2"), "Remove from missing entity fails");

    // Clear
    sys.addContribution("e1", "ship3", "Gamma", RT::Gas, 5.0f, 30.0f);
    assertTrue(sys.getContributionCount("e1") == 2, "Two contributions before clear");
    assertTrue(sys.clearContributions("e1"), "Clear succeeds");
    assertTrue(sys.getContributionCount("e1") == 0, "No contributions after clear");
    assertTrue(!sys.clearContributions("nope"), "Clear on missing entity fails");
}

static void testFleetCargoAggregatorAddPool() {
    std::cout << "\n=== FleetCargoAggregator: AddPool ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addPool("e1", "pool1", RT::Ore, 500.0f), "Add pool succeeds");
    assertTrue(sys.getPoolCount("e1") == 1, "One pool");
    assertTrue(sys.hasPool("e1", "pool1"), "Has pool1");
    assertTrue(approxEqual(sys.getPoolQuantity("e1", "pool1"), 0.0f), "Pool quantity starts at 0");
    assertTrue(approxEqual(sys.getPoolCapacity("e1", "pool1"), 500.0f), "Pool capacity is 500");

    // Duplicate
    assertTrue(!sys.addPool("e1", "pool1", RT::Ice, 200.0f), "Duplicate pool_id rejected");

    // Validation
    assertTrue(!sys.addPool("e1", "", RT::Ore, 100.0f), "Empty pool_id rejected");
    assertTrue(!sys.addPool("e1", "p2", RT::Ore, 0.0f), "Zero capacity rejected");
    assertTrue(!sys.addPool("e1", "p2", RT::Ore, -10.0f), "Negative capacity rejected");
    assertTrue(!sys.addPool("nope", "p2", RT::Ore, 100.0f), "Missing entity rejected");

    // Remove pool
    assertTrue(sys.addPool("e1", "pool2", RT::Ice, 300.0f), "Add pool2 succeeds");
    assertTrue(sys.removePool("e1", "pool2"), "Remove pool2 succeeds");
    assertTrue(sys.getPoolCount("e1") == 1, "One pool after remove");
    assertTrue(!sys.removePool("e1", "pool99"), "Remove nonexistent pool fails");
    assertTrue(!sys.removePool("nope", "pool1"), "Remove from missing entity fails");

    // Clear pools
    sys.addPool("e1", "pool3", RT::Gas, 200.0f);
    assertTrue(sys.getPoolCount("e1") == 2, "Two pools before clear");
    assertTrue(sys.clearPools("e1"), "Clear pools succeeds");
    assertTrue(sys.getPoolCount("e1") == 0, "No pools after clear");
    assertTrue(!sys.clearPools("nope"), "Clear pools on missing entity fails");
}

static void testFleetCargoAggregatorTransferToPool() {
    std::cout << "\n=== FleetCargoAggregator: TransferToPool ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addContribution("e1", "ship1", "Alpha", RT::Ore, 100.0f, 200.0f);
    sys.addPool("e1", "pool1", RT::Ore, 500.0f);

    // Successful transfer
    assertTrue(sys.transferToPool("e1", "ship1", "pool1", 40.0f), "Transfer 40 succeeds");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship1"), 60.0f), "Ship1 has 60 remaining");
    assertTrue(approxEqual(sys.getPoolQuantity("e1", "pool1"), 40.0f), "Pool1 has 40");
    assertTrue(sys.getTotalTransfers("e1") == 1, "One transfer recorded");
    assertTrue(approxEqual(sys.getTotalQuantityTransferred("e1"), 40.0f), "40 total transferred");

    // Transfer more
    assertTrue(sys.transferToPool("e1", "ship1", "pool1", 30.0f), "Transfer 30 more succeeds");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship1"), 30.0f), "Ship1 has 30 remaining");
    assertTrue(approxEqual(sys.getPoolQuantity("e1", "pool1"), 70.0f), "Pool1 has 70");
    assertTrue(sys.getTotalTransfers("e1") == 2, "Two transfers recorded");
    assertTrue(approxEqual(sys.getTotalQuantityTransferred("e1"), 70.0f), "70 total transferred");

    // Transfer too much from ship
    assertTrue(!sys.transferToPool("e1", "ship1", "pool1", 50.0f), "Transfer exceeding ship quantity fails");

    // Transfer zero or negative
    assertTrue(!sys.transferToPool("e1", "ship1", "pool1", 0.0f), "Transfer zero fails");
    assertTrue(!sys.transferToPool("e1", "ship1", "pool1", -5.0f), "Transfer negative fails");

    // Transfer to nonexistent pool/ship
    assertTrue(!sys.transferToPool("e1", "noship", "pool1", 10.0f), "Transfer from nonexistent ship fails");
    assertTrue(!sys.transferToPool("e1", "ship1", "nopool", 10.0f), "Transfer to nonexistent pool fails");
    assertTrue(!sys.transferToPool("nope", "ship1", "pool1", 10.0f), "Transfer on missing entity fails");

    // Transfer exceeding pool capacity
    sys.addPool("e1", "pool_small", RT::Ore, 10.0f);
    assertTrue(!sys.transferToPool("e1", "ship1", "pool_small", 20.0f), "Transfer exceeding pool capacity fails");
    assertTrue(sys.transferToPool("e1", "ship1", "pool_small", 10.0f), "Transfer exactly to pool capacity succeeds");
    assertTrue(!sys.transferToPool("e1", "ship1", "pool_small", 1.0f), "Transfer to full pool fails");
}

static void testFleetCargoAggregatorUpdateQuantity() {
    std::cout << "\n=== FleetCargoAggregator: UpdateQuantity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addContribution("e1", "ship1", "Alpha", RT::Ore, 50.0f, 100.0f);

    assertTrue(sys.updateContributionQuantity("e1", "ship1", 75.0f), "Update to 75 succeeds");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship1"), 75.0f), "Quantity is now 75");

    assertTrue(sys.updateContributionQuantity("e1", "ship1", 0.0f), "Update to 0 succeeds");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship1"), 0.0f), "Quantity is now 0");

    assertTrue(sys.updateContributionQuantity("e1", "ship1", 100.0f), "Update to capacity succeeds");
    assertTrue(approxEqual(sys.getContributionQuantity("e1", "ship1"), 100.0f), "Quantity is now 100");

    // Exceeds capacity
    assertTrue(!sys.updateContributionQuantity("e1", "ship1", 101.0f), "Update exceeding capacity fails");

    // Negative
    assertTrue(!sys.updateContributionQuantity("e1", "ship1", -1.0f), "Update negative fails");

    // Nonexistent ship
    assertTrue(!sys.updateContributionQuantity("e1", "noship", 10.0f), "Update nonexistent ship fails");

    // Missing entity
    assertTrue(!sys.updateContributionQuantity("nope", "ship1", 10.0f), "Update on missing entity fails");
}

static void testFleetCargoAggregatorConfiguration() {
    std::cout << "\n=== FleetCargoAggregator: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setFleetId
    assertTrue(sys.setFleetId("e1", "fleet_alpha"), "Set fleet id succeeds");
    assertTrue(sys.getFleetId("e1") == "fleet_alpha", "Fleet id is fleet_alpha");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleet id rejected");
    assertTrue(!sys.setFleetId("nope", "f1"), "Set fleet id on missing entity fails");

    // setMaxContributions
    assertTrue(sys.setMaxContributions("e1", 5), "Set max contributions to 5");
    assertTrue(sys.getMaxContributions("e1") == 5, "Max contributions is 5");
    assertTrue(!sys.setMaxContributions("e1", 0), "Zero max contributions rejected");
    assertTrue(!sys.setMaxContributions("e1", -1), "Negative max contributions rejected");
    assertTrue(!sys.setMaxContributions("nope", 10), "Set max on missing entity fails");

    // setMaxPools
    assertTrue(sys.setMaxPools("e1", 3), "Set max pools to 3");
    assertTrue(sys.getMaxPools("e1") == 3, "Max pools is 3");
    assertTrue(!sys.setMaxPools("e1", 0), "Zero max pools rejected");
    assertTrue(!sys.setMaxPools("e1", -1), "Negative max pools rejected");
    assertTrue(!sys.setMaxPools("nope", 10), "Set max pools on missing entity fails");
}

static void testFleetCargoAggregatorAggregateQueries() {
    std::cout << "\n=== FleetCargoAggregator: AggregateQueries ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addContribution("e1", "s1", "Ship1", RT::Ore, 50.0f, 100.0f);
    sys.addContribution("e1", "s2", "Ship2", RT::Ice, 30.0f, 80.0f);
    sys.addContribution("e1", "s3", "Ship3", RT::Ore, 20.0f, 60.0f);

    // Total fleet quantity = 50 + 30 + 20 = 100
    assertTrue(approxEqual(sys.getTotalFleetQuantity("e1"), 100.0f), "Total fleet quantity is 100");
    // Total fleet capacity = 100 + 80 + 60 = 240
    assertTrue(approxEqual(sys.getTotalFleetCapacity("e1"), 240.0f), "Total fleet capacity is 240");

    // Quantity by type: Ore = 50 + 20 = 70
    assertTrue(approxEqual(sys.getQuantityByType("e1", RT::Ore), 70.0f), "Ore quantity is 70");
    // Ice = 30
    assertTrue(approxEqual(sys.getQuantityByType("e1", RT::Ice), 30.0f), "Ice quantity is 30");
    // Gas = 0
    assertTrue(approxEqual(sys.getQuantityByType("e1", RT::Gas), 0.0f), "Gas quantity is 0");
}

static void testFleetCargoAggregatorCapacityCap() {
    std::cout << "\n=== FleetCargoAggregator: CapacityCap ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Set small max_contributions
    sys.setMaxContributions("e1", 2);
    assertTrue(sys.addContribution("e1", "s1", "A", RT::Ore, 10.0f, 50.0f), "Add 1st contrib");
    assertTrue(sys.addContribution("e1", "s2", "B", RT::Ice, 10.0f, 50.0f), "Add 2nd contrib");
    assertTrue(!sys.addContribution("e1", "s3", "C", RT::Gas, 10.0f, 50.0f),
               "3rd contrib rejected by cap");
    assertTrue(sys.getContributionCount("e1") == 2, "Still 2 contributions");

    // Set small max_pools
    sys.setMaxPools("e1", 1);
    assertTrue(sys.addPool("e1", "p1", RT::Ore, 100.0f), "Add 1st pool");
    assertTrue(!sys.addPool("e1", "p2", RT::Ice, 100.0f), "2nd pool rejected by cap");
    assertTrue(sys.getPoolCount("e1") == 1, "Still 1 pool");
}

static void testFleetCargoAggregatorCountByType() {
    std::cout << "\n=== FleetCargoAggregator: CountByType ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addContribution("e1", "s1", "A", RT::Ore, 10.0f, 50.0f);
    sys.addContribution("e1", "s2", "B", RT::Ore, 20.0f, 60.0f);
    sys.addContribution("e1", "s3", "C", RT::Ice, 15.0f, 40.0f);
    sys.addContribution("e1", "s4", "D", RT::Fuel, 5.0f, 30.0f);

    assertTrue(sys.getCountByType("e1", RT::Ore) == 2, "2 Ore contributions");
    assertTrue(sys.getCountByType("e1", RT::Ice) == 1, "1 Ice contribution");
    assertTrue(sys.getCountByType("e1", RT::Fuel) == 1, "1 Fuel contribution");
    assertTrue(sys.getCountByType("e1", RT::Gas) == 0, "0 Gas contributions");
    assertTrue(sys.getCountByType("e1", RT::Salvage) == 0, "0 Salvage contributions");
    assertTrue(sys.getCountByType("e1", RT::Ammunition) == 0, "0 Ammunition contributions");
    assertTrue(sys.getCountByType("e1", RT::Component) == 0, "0 Component contributions");
    assertTrue(sys.getCountByType("e1", RT::Mineral) == 0, "0 Mineral contributions");
}

static void testFleetCargoAggregatorMissingEntity() {
    std::cout << "\n=== FleetCargoAggregator: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoAggregatorSystem sys(&world);

    // All query methods should return safe defaults on missing entity
    assertTrue(sys.getContributionCount("nope") == 0, "Missing: contribution count is 0");
    assertTrue(sys.getPoolCount("nope") == 0, "Missing: pool count is 0");
    assertTrue(!sys.hasContribution("nope", "s1"), "Missing: hasContribution is false");
    assertTrue(!sys.hasPool("nope", "p1"), "Missing: hasPool is false");
    assertTrue(approxEqual(sys.getContributionQuantity("nope", "s1"), 0.0f), "Missing: contribution quantity is 0");
    assertTrue(approxEqual(sys.getContributionCapacity("nope", "s1"), 0.0f), "Missing: contribution capacity is 0");
    assertTrue(approxEqual(sys.getPoolQuantity("nope", "p1"), 0.0f), "Missing: pool quantity is 0");
    assertTrue(approxEqual(sys.getPoolCapacity("nope", "p1"), 0.0f), "Missing: pool capacity is 0");
    assertTrue(approxEqual(sys.getTotalFleetQuantity("nope"), 0.0f), "Missing: total fleet quantity is 0");
    assertTrue(approxEqual(sys.getTotalFleetCapacity("nope"), 0.0f), "Missing: total fleet capacity is 0");
    assertTrue(approxEqual(sys.getQuantityByType("nope", RT::Ore), 0.0f), "Missing: quantity by type is 0");
    assertTrue(sys.getCountByType("nope", RT::Ore) == 0, "Missing: count by type is 0");
    assertTrue(sys.getFleetId("nope") == "", "Missing: fleet id is empty");
    assertTrue(sys.getTotalTransfers("nope") == 0, "Missing: total transfers is 0");
    assertTrue(approxEqual(sys.getTotalQuantityTransferred("nope"), 0.0f), "Missing: total quantity transferred is 0");
    assertTrue(sys.getTotalContributionsAdded("nope") == 0, "Missing: total contributions added is 0");
    assertTrue(sys.getMaxContributions("nope") == 0, "Missing: max contributions is 0");
    assertTrue(sys.getMaxPools("nope") == 0, "Missing: max pools is 0");
    assertTrue(sys.getShipName("nope", "s1") == "", "Missing: ship name is empty");

    // All mutating methods should return false
    assertTrue(!sys.addContribution("nope", "s1", "N", RT::Ore, 10.0f, 50.0f), "Missing: addContribution fails");
    assertTrue(!sys.removeContribution("nope", "s1"), "Missing: removeContribution fails");
    assertTrue(!sys.clearContributions("nope"), "Missing: clearContributions fails");
    assertTrue(!sys.updateContributionQuantity("nope", "s1", 10.0f), "Missing: updateQuantity fails");
    assertTrue(!sys.addPool("nope", "p1", RT::Ore, 100.0f), "Missing: addPool fails");
    assertTrue(!sys.removePool("nope", "p1"), "Missing: removePool fails");
    assertTrue(!sys.clearPools("nope"), "Missing: clearPools fails");
    assertTrue(!sys.transferToPool("nope", "s1", "p1", 10.0f), "Missing: transferToPool fails");
    assertTrue(!sys.setFleetId("nope", "f1"), "Missing: setFleetId fails");
    assertTrue(!sys.setMaxContributions("nope", 10), "Missing: setMaxContributions fails");
    assertTrue(!sys.setMaxPools("nope", 10), "Missing: setMaxPools fails");
}

void run_fleet_cargo_aggregator_system_tests() {
    testFleetCargoAggregatorInit();
    testFleetCargoAggregatorAddContribution();
    testFleetCargoAggregatorAddContributionValidation();
    testFleetCargoAggregatorRemoveContribution();
    testFleetCargoAggregatorAddPool();
    testFleetCargoAggregatorTransferToPool();
    testFleetCargoAggregatorUpdateQuantity();
    testFleetCargoAggregatorConfiguration();
    testFleetCargoAggregatorAggregateQueries();
    testFleetCargoAggregatorCapacityCap();
    testFleetCargoAggregatorCountByType();
    testFleetCargoAggregatorMissingEntity();
}
