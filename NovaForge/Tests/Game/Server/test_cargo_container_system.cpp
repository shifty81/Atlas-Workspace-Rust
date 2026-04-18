// Tests for: CargoContainerSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/cargo_container_system.h"

using namespace atlas;

// ==================== CargoContainerSystem Tests ====================

static void testCargoContainerInit() {
    std::cout << "\n=== CargoContainer: Init ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getItemCount("e1") == 0, "Zero items initially");
    assertTrue(approxEqual(sys.getUsedVolume("e1"), 0.0f), "Zero used volume");
    assertTrue(sys.getRemainingVolume("e1") > 0.0f, "Has remaining volume");
    assertTrue(!sys.isAnchored("e1"), "Not anchored initially");
    assertTrue(sys.getTimeRemaining("e1") > 0.0f, "Has time remaining");
    assertTrue(sys.getOwner("e1") == "", "No owner initially");
    assertTrue(!sys.isPasswordProtected("e1"), "Not password protected");
    assertTrue(sys.getTotalItemsAdded("e1") == 0, "Zero added");
    assertTrue(sys.getTotalItemsRemoved("e1") == 0, "Zero removed");
    assertTrue(sys.getContainerType("e1") == components::CargoContainerState::ContainerType::Jetcan,
               "Default type is Jetcan");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCargoContainerConfiguration() {
    std::cout << "\n=== CargoContainer: Configuration ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using CT = components::CargoContainerState::ContainerType;
    assertTrue(sys.setContainerType("e1", CT::SmallSecure), "Set type to SmallSecure");
    assertTrue(sys.getContainerType("e1") == CT::SmallSecure, "Type is SmallSecure");

    assertTrue(sys.setOwner("e1", "player123"), "Set owner");
    assertTrue(sys.getOwner("e1") == "player123", "Owner is player123");

    assertTrue(sys.setPassword("e1", "secret"), "Set password");
    assertTrue(sys.isPasswordProtected("e1"), "Is password protected");
    assertTrue(sys.checkPassword("e1", "secret"), "Correct password accepted");
    assertTrue(!sys.checkPassword("e1", "wrong"), "Wrong password rejected");

    assertTrue(sys.setCapacity("e1", 50000.0f), "Set capacity");
    assertTrue(approxEqual(sys.getRemainingVolume("e1"), 50000.0f), "50k remaining");

    assertTrue(!sys.setCapacity("e1", 0.0f), "Zero capacity rejected");
    assertTrue(!sys.setCapacity("e1", -100.0f), "Negative capacity rejected");

    assertTrue(sys.setLifetime("e1", 3600.0f), "Set lifetime");
    assertTrue(approxEqual(sys.getTimeRemaining("e1"), 3600.0f), "Time remaining 3600");
    assertTrue(!sys.setLifetime("e1", 0.0f), "Zero lifetime rejected");

    assertTrue(!sys.setContainerType("missing", CT::Freight), "Type on missing fails");
    assertTrue(!sys.setOwner("missing", "x"), "Owner on missing fails");
    assertTrue(!sys.setPassword("missing", "x"), "Password on missing fails");
    assertTrue(!sys.setCapacity("missing", 100.0f), "Capacity on missing fails");
    assertTrue(!sys.setLifetime("missing", 100.0f), "Lifetime on missing fails");
}

static void testCargoContainerAddItem() {
    std::cout << "\n=== CargoContainer: AddItem ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCapacity("e1", 1000.0f);

    assertTrue(sys.addItem("e1", "tritanium", "Tritanium", 100, 1.0f), "Add 100 Tritanium");
    assertTrue(sys.getItemCount("e1") == 1, "1 item type");
    assertTrue(sys.hasItem("e1", "tritanium"), "Has Tritanium");
    assertTrue(sys.getItemQuantity("e1", "tritanium") == 100, "100 Tritanium");
    assertTrue(approxEqual(sys.getUsedVolume("e1"), 100.0f), "100 volume used");
    assertTrue(approxEqual(sys.getRemainingVolume("e1"), 900.0f), "900 remaining");
    assertTrue(sys.getTotalItemsAdded("e1") == 100, "100 total added");

    // Stack on existing item
    assertTrue(sys.addItem("e1", "tritanium", "Tritanium", 50, 1.0f), "Stack 50 more Tritanium");
    assertTrue(sys.getItemCount("e1") == 1, "Still 1 item type (stacked)");
    assertTrue(sys.getItemQuantity("e1", "tritanium") == 150, "150 Tritanium total");
    assertTrue(approxEqual(sys.getUsedVolume("e1"), 150.0f), "150 volume used");
    assertTrue(sys.getTotalItemsAdded("e1") == 150, "150 total added");

    // Add different item
    assertTrue(sys.addItem("e1", "pyerite", "Pyerite", 200, 1.0f), "Add Pyerite");
    assertTrue(sys.getItemCount("e1") == 2, "2 item types");

    // Empty ID rejected
    assertTrue(!sys.addItem("e1", "", "X", 10, 1.0f), "Empty ID rejected");

    // Zero/negative quantity rejected
    assertTrue(!sys.addItem("e1", "x", "X", 0, 1.0f), "Zero quantity rejected");
    assertTrue(!sys.addItem("e1", "x", "X", -5, 1.0f), "Negative quantity rejected");

    // Negative volume rejected
    assertTrue(!sys.addItem("e1", "x", "X", 10, -1.0f), "Negative volume rejected");

    // Exceeds capacity
    assertTrue(!sys.addItem("e1", "huge", "Huge", 1, 10000.0f), "Exceeds capacity rejected");

    assertTrue(!sys.addItem("missing", "x", "X", 10, 1.0f), "Add on missing fails");
}

static void testCargoContainerRemoveItem() {
    std::cout << "\n=== CargoContainer: RemoveItem ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCapacity("e1", 1000.0f);
    sys.addItem("e1", "trit", "Tritanium", 100, 1.0f);

    assertTrue(sys.removeItem("e1", "trit", 30), "Remove 30 Tritanium");
    assertTrue(sys.getItemQuantity("e1", "trit") == 70, "70 remaining");
    assertTrue(approxEqual(sys.getUsedVolume("e1"), 70.0f), "70 volume used");
    assertTrue(sys.getTotalItemsRemoved("e1") == 30, "30 total removed");

    // Remove all remaining
    assertTrue(sys.removeItem("e1", "trit", 70), "Remove all 70");
    assertTrue(sys.getItemCount("e1") == 0, "0 items (stack removed)");
    assertTrue(!sys.hasItem("e1", "trit"), "No Tritanium");

    // Remove too many
    sys.addItem("e1", "pye", "Pyerite", 50, 1.0f);
    assertTrue(!sys.removeItem("e1", "pye", 100), "Remove > quantity rejected");

    // Zero/negative quantity
    assertTrue(!sys.removeItem("e1", "pye", 0), "Zero quantity rejected");
    assertTrue(!sys.removeItem("e1", "pye", -5), "Negative quantity rejected");

    // Non-existent item
    assertTrue(!sys.removeItem("e1", "noItem", 10), "Non-existent item fails");
    assertTrue(!sys.removeItem("missing", "pye", 10), "Remove on missing fails");
}

static void testCargoContainerClearItems() {
    std::cout << "\n=== CargoContainer: ClearItems ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addItem("e1", "trit", "Tritanium", 100, 1.0f);
    sys.addItem("e1", "pye", "Pyerite", 50, 1.0f);

    assertTrue(sys.clearItems("e1"), "Clear items");
    assertTrue(sys.getItemCount("e1") == 0, "Zero items");
    assertTrue(approxEqual(sys.getUsedVolume("e1"), 0.0f), "Zero volume");
    assertTrue(!sys.clearItems("missing"), "Clear on missing fails");
}

static void testCargoContainerAnchoring() {
    std::cout << "\n=== CargoContainer: Anchoring ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Jetcan cannot be anchored
    assertTrue(!sys.anchor("e1"), "Jetcan anchor rejected");

    using CT = components::CargoContainerState::ContainerType;
    sys.setContainerType("e1", CT::SmallSecure);

    assertTrue(sys.anchor("e1"), "Anchor SmallSecure");
    assertTrue(sys.isAnchored("e1"), "Is anchored");
    assertTrue(!sys.anchor("e1"), "Already anchored rejected");

    assertTrue(sys.unanchor("e1"), "Unanchor");
    assertTrue(!sys.isAnchored("e1"), "Not anchored");
    assertTrue(!sys.unanchor("e1"), "Already unanchored rejected");

    assertTrue(!sys.anchor("missing"), "Anchor on missing fails");
    assertTrue(!sys.unanchor("missing"), "Unanchor on missing fails");
}

static void testCargoContainerLifetimeDecay() {
    std::cout << "\n=== CargoContainer: LifetimeDecay ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setLifetime("e1", 100.0f);

    sys.update(30.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("e1"), 70.0f), "70s remaining after 30s");

    sys.update(80.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("e1"), 0.0f), "0s remaining (clamped)");
}

static void testCargoContainerPasswordNoProtection() {
    std::cout << "\n=== CargoContainer: NoPassword ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // No password set: checkPassword always returns true
    assertTrue(sys.checkPassword("e1", "anything"), "No password = always pass");
    assertTrue(sys.checkPassword("e1", ""), "No password = pass with empty");
    assertTrue(!sys.checkPassword("missing", "x"), "Check on missing fails");
}

static void testCargoContainerMissing() {
    std::cout << "\n=== CargoContainer: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CargoContainerSystem sys(&world);

    using CT = components::CargoContainerState::ContainerType;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.setContainerType("m", CT::Freight), "SetType fails");
    assertTrue(!sys.setOwner("m", "x"), "SetOwner fails");
    assertTrue(!sys.setPassword("m", "x"), "SetPassword fails");
    assertTrue(!sys.setCapacity("m", 100.0f), "SetCapacity fails");
    assertTrue(!sys.setLifetime("m", 100.0f), "SetLifetime fails");
    assertTrue(!sys.addItem("m", "x", "X", 10, 1.0f), "AddItem fails");
    assertTrue(!sys.removeItem("m", "x", 10), "RemoveItem fails");
    assertTrue(!sys.clearItems("m"), "ClearItems fails");
    assertTrue(!sys.anchor("m"), "Anchor fails");
    assertTrue(!sys.unanchor("m"), "Unanchor fails");
    assertTrue(sys.getItemCount("m") == 0, "getItemCount returns 0");
    assertTrue(!sys.hasItem("m", "x"), "hasItem returns false");
    assertTrue(sys.getItemQuantity("m", "x") == 0, "getItemQuantity returns 0");
    assertTrue(approxEqual(sys.getUsedVolume("m"), 0.0f), "getUsedVolume returns 0");
    assertTrue(approxEqual(sys.getRemainingVolume("m"), 0.0f), "getRemainingVolume returns 0");
    assertTrue(!sys.isAnchored("m"), "isAnchored returns false");
    assertTrue(approxEqual(sys.getTimeRemaining("m"), 0.0f), "getTimeRemaining returns 0");
    assertTrue(sys.getOwner("m") == "", "getOwner returns empty");
    assertTrue(!sys.isPasswordProtected("m"), "isPasswordProtected returns false");
    assertTrue(!sys.checkPassword("m", "x"), "checkPassword returns false");
    assertTrue(sys.getTotalItemsAdded("m") == 0, "getTotalItemsAdded returns 0");
    assertTrue(sys.getTotalItemsRemoved("m") == 0, "getTotalItemsRemoved returns 0");
    assertTrue(sys.getContainerType("m") == CT::Jetcan, "getContainerType returns Jetcan");
}

void run_cargo_container_system_tests() {
    testCargoContainerInit();
    testCargoContainerConfiguration();
    testCargoContainerAddItem();
    testCargoContainerRemoveItem();
    testCargoContainerClearItems();
    testCargoContainerAnchoring();
    testCargoContainerLifetimeDecay();
    testCargoContainerPasswordNoProtection();
    testCargoContainerMissing();
}
