// Tests for: FactionDoctrineSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/faction_doctrine_system.h"

using namespace atlas;
using Phase = components::FactionDoctrineState::DoctrinePhase;

static void testFactionDoctrineInit() {
    std::cout << "\n=== FactionDoctrine: Init ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Accumulate, "Default phase Accumulate");
    assertTrue(sys.getDoctrinePhaseString("e1") == "Accumulate", "Phase string Accumulate");
    assertTrue(approxEqual(sys.getTitanCompletion("e1"), 0.0f), "Titan completion 0");
    assertTrue(approxEqual(sys.getDiscoveryRisk("e1"), 0.0f), "Discovery risk 0");
    assertTrue(approxEqual(sys.getResourceScarcity("e1"), 0.0f), "Resource scarcity 0");
    assertTrue(approxEqual(sys.getPlayerProximity("e1"), 0.0f), "Player proximity 0");
    assertTrue(sys.getTotalPhaseShifts("e1") == 0, "Zero phase shifts");
    assertTrue(sys.getFactionId("e1").empty(), "Empty faction_id");
    assertTrue(!sys.isLaunchImminent("e1"), "Not launch imminent");
    assertTrue(sys.isActive("e1"), "Active by default");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFactionDoctrinePhaseTransitions() {
    std::cout << "\n=== FactionDoctrine: Phase Transitions via Titan Completion ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Accumulate → Conceal at 20%
    assertTrue(sys.setTitanCompletion("e1", 0.20f), "setTitanCompletion(0.2)");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Conceal, "Phase = Conceal at 0.2");
    assertTrue(sys.getDoctrinePhaseString("e1") == "Conceal", "Phase string Conceal");
    assertTrue(sys.getTotalPhaseShifts("e1") == 1, "1 phase shift");

    // Conceal → Disrupt at 40%
    assertTrue(sys.setTitanCompletion("e1", 0.40f), "setTitanCompletion(0.4)");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Disrupt, "Phase = Disrupt at 0.4");
    assertTrue(sys.getDoctrinePhaseString("e1") == "Disrupt", "Phase string Disrupt");

    // Disrupt → Defend at 70%
    assertTrue(sys.setTitanCompletion("e1", 0.70f), "setTitanCompletion(0.7)");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Defend, "Phase = Defend at 0.7");
    assertTrue(sys.getDoctrinePhaseString("e1") == "Defend", "Phase string Defend");

    // Defend → PrepareLaunch at 90%
    assertTrue(sys.setTitanCompletion("e1", 0.90f), "setTitanCompletion(0.9)");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::PrepareLaunch, "Phase = PrepareLaunch at 0.9");
    assertTrue(sys.getDoctrinePhaseString("e1") == "PrepareLaunch", "Phase string PrepareLaunch");
    assertTrue(sys.isLaunchImminent("e1"), "isLaunchImminent = true");
}

static void testFactionDoctrineDiscoveryRiskTriggers() {
    std::cout << "\n=== FactionDoctrine: Discovery Risk Triggers ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Discovery risk >= 0.3 → Conceal (even if titan < 0.2)
    assertTrue(sys.setDiscoveryRisk("e1", 0.35f), "setDiscoveryRisk(0.35)");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Conceal, "Conceal from discovery risk");

    // Discovery risk >= 0.6 → Disrupt
    assertTrue(sys.setDiscoveryRisk("e1", 0.65f), "setDiscoveryRisk(0.65)");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Disrupt, "Disrupt from high discovery risk");

    assertTrue(!sys.setTitanCompletion("missing", 0.5f), "setTitanCompletion on missing fails");
    assertTrue(!sys.setDiscoveryRisk("missing", 0.5f), "setDiscoveryRisk on missing fails");
}

static void testFactionDoctrineAggressionProfile() {
    std::cout << "\n=== FactionDoctrine: Aggression Profile Per Phase ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Accumulate: low aggression, high stealth
    assertTrue(approxEqual(sys.getAggressionMult("e1"), 0.2f), "Accumulate aggression 0.2");
    assertTrue(approxEqual(sys.getStealthBias("e1"), 0.7f), "Accumulate stealth 0.7");
    assertTrue(approxEqual(sys.getRaidFrequency("e1"), 0.1f), "Accumulate raid 0.1");

    // Conceal: lowest aggression
    sys.setTitanCompletion("e1", 0.20f);
    assertTrue(approxEqual(sys.getAggressionMult("e1"), 0.1f), "Conceal aggression 0.1");
    assertTrue(approxEqual(sys.getStealthBias("e1"), 0.9f), "Conceal stealth 0.9");

    // Disrupt: high aggression
    sys.setTitanCompletion("e1", 0.40f);
    assertTrue(approxEqual(sys.getAggressionMult("e1"), 0.6f), "Disrupt aggression 0.6");
    assertTrue(approxEqual(sys.getRaidFrequency("e1"), 0.5f), "Disrupt raid 0.5");

    // Defend
    sys.setTitanCompletion("e1", 0.70f);
    assertTrue(approxEqual(sys.getAggressionMult("e1"), 0.8f), "Defend aggression 0.8");

    // PrepareLaunch: maximum aggression
    sys.setTitanCompletion("e1", 0.90f);
    assertTrue(approxEqual(sys.getAggressionMult("e1"), 1.0f), "PrepareLaunch aggression 1.0");
    assertTrue(approxEqual(sys.getStealthBias("e1"), 0.1f), "PrepareLaunch stealth 0.1");
    assertTrue(approxEqual(sys.getRaidFrequency("e1"), 0.9f), "PrepareLaunch raid 0.9");
}

static void testFactionDoctrineManualAdvance() {
    std::cout << "\n=== FactionDoctrine: Manual Advance ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.advancePhase("e1"), "Advance from Accumulate");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Conceal, "Phase = Conceal");
    assertTrue(sys.advancePhase("e1"), "Advance to Disrupt");
    assertTrue(sys.advancePhase("e1"), "Advance to Defend");
    assertTrue(sys.advancePhase("e1"), "Advance to PrepareLaunch");
    assertTrue(!sys.advancePhase("e1"), "Cannot advance past PrepareLaunch");
    assertTrue(sys.getTotalPhaseShifts("e1") == 4, "4 phase shifts");

    assertTrue(sys.resetToAccumulate("e1"), "Reset to Accumulate");
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Accumulate, "Back to Accumulate");
    assertTrue(approxEqual(sys.getTitanCompletion("e1"), 0.0f), "Titan completion reset");

    assertTrue(!sys.advancePhase("missing"), "advancePhase on missing fails");
    assertTrue(!sys.resetToAccumulate("missing"), "resetToAccumulate on missing fails");
}

static void testFactionDoctrineConfiguration() {
    std::cout << "\n=== FactionDoctrine: Configuration ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setFactionId("e1", "Crimson_Veil"), "setFactionId succeeds");
    assertTrue(sys.getFactionId("e1") == "Crimson_Veil", "FactionId matches");
    assertTrue(!sys.setFactionId("e1", ""), "Empty factionId rejected");

    assertTrue(sys.setResourceScarcity("e1", 0.6f), "setResourceScarcity succeeds");
    assertTrue(approxEqual(sys.getResourceScarcity("e1"), 0.6f), "ResourceScarcity = 0.6");

    assertTrue(sys.setPlayerProximity("e1", 0.3f), "setPlayerProximity succeeds");
    assertTrue(approxEqual(sys.getPlayerProximity("e1"), 0.3f), "PlayerProximity = 0.3");

    // Threshold configuration
    assertTrue(sys.setConcealThreshold("e1", 0.15f), "setConcealThreshold(0.15)");
    assertTrue(sys.setDisruptThreshold("e1", 0.35f), "setDisruptThreshold(0.35)");
    assertTrue(sys.setDefendThreshold("e1", 0.65f), "setDefendThreshold(0.65)");
    assertTrue(sys.setLaunchThreshold("e1", 0.85f), "setLaunchThreshold(0.85)");

    // Invalid thresholds
    assertTrue(!sys.setConcealThreshold("e1", -0.1f), "Negative threshold rejected");
    assertTrue(!sys.setLaunchThreshold("e1", 1.1f), "Over-1.0 threshold rejected");

    // Clamping of driver floats
    assertTrue(sys.setTitanCompletion("e1", 1.5f), "setTitanCompletion clamps high");
    assertTrue(approxEqual(sys.getTitanCompletion("e1"), 1.0f), "TitanCompletion clamped to 1.0");
    assertTrue(sys.setDiscoveryRisk("e1", -0.5f), "setDiscoveryRisk clamps low");
    assertTrue(approxEqual(sys.getDiscoveryRisk("e1"), 0.0f), "DiscoveryRisk clamped to 0.0");

    assertTrue(!sys.setFactionId("missing", "x"), "setFactionId on missing fails");
    assertTrue(!sys.setResourceScarcity("missing", 0.5f), "setResourceScarcity on missing fails");
    assertTrue(!sys.setPlayerProximity("missing", 0.5f), "setPlayerProximity on missing fails");
}

static void testFactionDoctrineTickTransition() {
    std::cout << "\n=== FactionDoctrine: Tick Phase Transition ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Manually set titan_completion to trigger transition on next tick
    auto* comp = world.getEntity("e1")->getComponent<components::FactionDoctrineState>();
    comp->titan_completion = 0.25f;

    sys.update(0.1f);
    assertTrue(sys.getDoctrinePhase("e1") == Phase::Conceal, "Tick transitions to Conceal");
}

static void testFactionDoctrineMissingEntity() {
    std::cout << "\n=== FactionDoctrine: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FactionDoctrineSystem sys(&world);

    assertTrue(sys.getDoctrinePhase("missing") == Phase::Accumulate, "getPhase = Accumulate");
    assertTrue(sys.getDoctrinePhaseString("missing").empty(), "getPhaseString = ''");
    assertTrue(approxEqual(sys.getTitanCompletion("missing"), 0.0f), "getTitanCompletion = 0");
    assertTrue(approxEqual(sys.getDiscoveryRisk("missing"), 0.0f), "getDiscoveryRisk = 0");
    assertTrue(approxEqual(sys.getResourceScarcity("missing"), 0.0f), "getResourceScarcity = 0");
    assertTrue(approxEqual(sys.getPlayerProximity("missing"), 0.0f), "getPlayerProximity = 0");
    assertTrue(approxEqual(sys.getAggressionMult("missing"), 0.0f), "getAggressionMult = 0");
    assertTrue(approxEqual(sys.getStealthBias("missing"), 0.0f), "getStealthBias = 0");
    assertTrue(approxEqual(sys.getRaidFrequency("missing"), 0.0f), "getRaidFrequency = 0");
    assertTrue(sys.getTotalPhaseShifts("missing") == 0, "getTotalPhaseShifts = 0");
    assertTrue(sys.getFactionId("missing").empty(), "getFactionId = ''");
    assertTrue(!sys.isLaunchImminent("missing"), "isLaunchImminent = false");
    assertTrue(!sys.isActive("missing"), "isActive = false");
    assertTrue(!sys.setTitanCompletion("missing", 0.5f), "setTitanCompletion = false");
    assertTrue(!sys.setDiscoveryRisk("missing", 0.5f), "setDiscoveryRisk = false");
    assertTrue(!sys.advancePhase("missing"), "advancePhase = false");
    assertTrue(!sys.resetToAccumulate("missing"), "resetToAccumulate = false");
}

void run_faction_doctrine_system_tests() {
    testFactionDoctrineInit();
    testFactionDoctrinePhaseTransitions();
    testFactionDoctrineDiscoveryRiskTriggers();
    testFactionDoctrineAggressionProfile();
    testFactionDoctrineManualAdvance();
    testFactionDoctrineConfiguration();
    testFactionDoctrineTickTransition();
    testFactionDoctrineMissingEntity();
}
