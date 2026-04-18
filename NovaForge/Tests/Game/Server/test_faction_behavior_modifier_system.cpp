// Tests for: FactionBehaviorModifierSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/faction_behavior_modifier_system.h"

using namespace atlas;
using FP = components::FactionBehaviorState::FactionProfile;
using MD = components::FactionBehaviorState::MoraleDriver;

static void testFactionBehaviorInit() {
    std::cout << "\n=== FactionBehavior: Init ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getProfile("e1") == FP::Neutral, "Default profile Neutral");
    assertTrue(sys.getMoraleDriver("e1") == MD::None, "Default driver None");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 0.0f), "Default morale bias 0");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 1.0f), "Default chatter rate 1.0");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.5f), "Default combat pref 0.5");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.5f), "Default mining pref 0.5");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.5f), "Default exploration pref 0.5");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.5f), "Default trade pref 0.5");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -50.0f), "Default departure -50");
    assertTrue(sys.getFactionId("e1").empty(), "Default faction id empty");
    assertTrue(sys.getTotalProfileChanges("e1") == 0, "Zero profile changes");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFactionBehaviorSetProfileIndustrial() {
    std::cout << "\n=== FactionBehavior: ProfileIndustrial ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    assertTrue(sys.setProfile("e1", FP::Industrial), "setProfile Industrial succeeds");
    assertTrue(sys.getProfile("e1") == FP::Industrial, "Profile is Industrial");
    assertTrue(sys.getMoraleDriver("e1") == MD::Efficiency, "Morale driver Efficiency");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 0.5f), "Chatter mult 0.5");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.2f), "Combat pref 0.2");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.9f), "Mining pref 0.9");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.4f), "Exploration pref 0.4");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.8f), "Trade pref 0.8");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -30.0f), "Departure -30");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 5.0f), "Morale bias 5.0");
    assertTrue(sys.getTotalProfileChanges("e1") == 1, "1 profile change");
}

static void testFactionBehaviorSetProfileMilitaristic() {
    std::cout << "\n=== FactionBehavior: ProfileMilitaristic ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    assertTrue(sys.setProfile("e1", FP::Militaristic), "setProfile Militaristic succeeds");
    assertTrue(sys.getProfile("e1") == FP::Militaristic, "Profile is Militaristic");
    assertTrue(sys.getMoraleDriver("e1") == MD::Victory, "Morale driver Victory");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 1.5f), "Chatter mult 1.5");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.9f), "Combat pref 0.9");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.2f), "Mining pref 0.2");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.3f), "Exploration pref 0.3");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.5f), "Trade pref 0.5");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -60.0f), "Departure -60");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 8.0f), "Morale bias 8.0");
}

static void testFactionBehaviorSetProfileNomadic() {
    std::cout << "\n=== FactionBehavior: ProfileNomadic ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    assertTrue(sys.setProfile("e1", FP::Nomadic), "setProfile Nomadic succeeds");
    assertTrue(sys.getProfile("e1") == FP::Nomadic, "Profile is Nomadic");
    assertTrue(sys.getMoraleDriver("e1") == MD::Exploration, "Morale driver Exploration");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 1.2f), "Chatter mult 1.2");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.4f), "Combat pref 0.4");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.3f), "Mining pref 0.3");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.95f), "Exploration pref 0.95");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.4f), "Trade pref 0.4");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -40.0f), "Departure -40");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 3.0f), "Morale bias 3.0");
}

static void testFactionBehaviorSetProfileCorporate() {
    std::cout << "\n=== FactionBehavior: ProfileCorporate ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    assertTrue(sys.setProfile("e1", FP::Corporate), "setProfile Corporate succeeds");
    assertTrue(sys.getProfile("e1") == FP::Corporate, "Profile is Corporate");
    assertTrue(sys.getMoraleDriver("e1") == MD::Success, "Morale driver Success");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 0.6f), "Chatter mult 0.6");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.5f), "Combat pref 0.5");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.4f), "Mining pref 0.4");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.3f), "Exploration pref 0.3");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.9f), "Trade pref 0.9");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -20.0f), "Departure -20");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 0.0f), "Morale bias 0.0");
}

static void testFactionBehaviorSetProfileNeutral() {
    std::cout << "\n=== FactionBehavior: ProfileNeutral ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    // Set non-neutral first, then back to neutral
    sys.setProfile("e1", FP::Militaristic);
    assertTrue(sys.setProfile("e1", FP::Neutral), "setProfile Neutral succeeds");
    assertTrue(sys.getProfile("e1") == FP::Neutral, "Profile is Neutral");
    assertTrue(sys.getMoraleDriver("e1") == MD::None, "Morale driver None");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 1.0f), "Chatter mult 1.0");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.5f), "Combat pref 0.5");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.5f), "Mining pref 0.5");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.5f), "Exploration pref 0.5");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.5f), "Trade pref 0.5");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -50.0f), "Departure -50");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 0.0f), "Morale bias 0.0");
    assertTrue(sys.getTotalProfileChanges("e1") == 2, "2 profile changes");
}

static void testFactionBehaviorFineGrainedConfig() {
    std::cout << "\n=== FactionBehavior: FineGrainedConfig ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setProfile("e1", FP::Industrial);

    // Override individual preferences
    assertTrue(sys.setMoraleBias("e1", 10.0f), "setMoraleBias succeeds");
    assertTrue(approxEqual(sys.getMoraleBias("e1"), 10.0f), "Morale bias is 10");

    assertTrue(sys.setChatterRateMult("e1", 2.0f), "setChatterRateMult succeeds");
    assertTrue(approxEqual(sys.getChatterRateMult("e1"), 2.0f), "Chatter rate 2.0");

    assertTrue(sys.setCombatPreference("e1", 0.7f), "setCombatPreference succeeds");
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 0.7f), "Combat pref 0.7");

    assertTrue(sys.setMiningPreference("e1", 0.1f), "setMiningPreference succeeds");
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.1f), "Mining pref 0.1");

    assertTrue(sys.setExplorationPreference("e1", 0.6f), "setExplorationPreference succeeds");
    assertTrue(approxEqual(sys.getExplorationPreference("e1"), 0.6f), "Exploration pref 0.6");

    assertTrue(sys.setTradePreference("e1", 0.3f), "setTradePreference succeeds");
    assertTrue(approxEqual(sys.getTradePreference("e1"), 0.3f), "Trade pref 0.3");

    assertTrue(sys.setDepartureThreshold("e1", -15.0f), "setDepartureThreshold succeeds");
    assertTrue(approxEqual(sys.getDepartureThreshold("e1"), -15.0f), "Departure -15");

    assertTrue(sys.setFactionId("e1", "Gallente"), "setFactionId succeeds");
    assertTrue(sys.getFactionId("e1") == "Gallente", "Faction id Gallente");

    // Clamp preference to [0,1]
    sys.setCombatPreference("e1", 1.5f);
    assertTrue(approxEqual(sys.getCombatPreference("e1"), 1.0f), "Combat pref clamped to 1.0");
    sys.setMiningPreference("e1", -0.5f);
    assertTrue(approxEqual(sys.getMiningPreference("e1"), 0.0f), "Mining pref clamped to 0.0");

    // Invalid chatter rate
    assertTrue(!sys.setChatterRateMult("e1", 0.0f), "Zero chatter mult rejected");
    assertTrue(!sys.setChatterRateMult("e1", -1.0f), "Negative chatter mult rejected");

    // Empty faction id rejected
    assertTrue(!sys.setFactionId("e1", ""), "Empty faction id rejected");
}

static void testFactionBehaviorApplyMoraleModifier() {
    std::cout << "\n=== FactionBehavior: ApplyMoraleModifier ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // No bias: returns base morale unchanged
    sys.setMoraleBias("e1", 0.0f);
    assertTrue(approxEqual(sys.applyMoraleModifier("e1", 50.0f), 50.0f), "No bias returns base");
    assertTrue(approxEqual(sys.applyMoraleModifier("e1", -20.0f), -20.0f), "No bias negative base");

    // Positive bias
    sys.setMoraleBias("e1", 10.0f);
    assertTrue(approxEqual(sys.applyMoraleModifier("e1", 30.0f), 40.0f), "Positive bias adds");

    // Negative bias
    sys.setMoraleBias("e1", -15.0f);
    assertTrue(approxEqual(sys.applyMoraleModifier("e1", 10.0f), -5.0f), "Negative bias subtracts");

    // Clamp at +100
    sys.setMoraleBias("e1", 50.0f);
    assertTrue(approxEqual(sys.applyMoraleModifier("e1", 90.0f), 100.0f), "Clamped at 100");

    // Clamp at -100
    sys.setMoraleBias("e1", -50.0f);
    assertTrue(approxEqual(sys.applyMoraleModifier("e1", -90.0f), -100.0f), "Clamped at -100");

    // Missing entity returns base morale unchanged
    assertTrue(approxEqual(sys.applyMoraleModifier("missing", 25.0f), 25.0f),
               "Missing entity returns base morale");
}

static void testFactionBehaviorGetDominantActivity() {
    std::cout << "\n=== FactionBehavior: GetDominantActivity ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Industrial: mining (0.9) > trade (0.8)
    sys.setProfile("e1", FP::Industrial);
    assertTrue(sys.getDominantActivity("e1") == "mining", "Industrial dominant is mining");

    // Militaristic: combat (0.9)
    sys.setProfile("e1", FP::Militaristic);
    assertTrue(sys.getDominantActivity("e1") == "combat", "Militaristic dominant is combat");

    // Nomadic: exploration (0.95)
    sys.setProfile("e1", FP::Nomadic);
    assertTrue(sys.getDominantActivity("e1") == "exploration", "Nomadic dominant is exploration");

    // Corporate: trade (0.9)
    sys.setProfile("e1", FP::Corporate);
    assertTrue(sys.getDominantActivity("e1") == "trade", "Corporate dominant is trade");

    // Custom: set combat highest
    sys.setProfile("e1", FP::Neutral);
    sys.setCombatPreference("e1", 0.8f);
    assertTrue(sys.getDominantActivity("e1") == "combat", "Custom highest combat");

    // Missing entity returns "None"
    assertTrue(sys.getDominantActivity("missing") == "None", "Missing entity returns None");
}

static void testFactionBehaviorDepartureRisk() {
    std::cout << "\n=== FactionBehavior: DepartureRisk ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setDepartureThreshold("e1", -50.0f);

    // Morale above threshold — not a risk
    assertTrue(!sys.isDepartureRisk("e1", 0.0f), "0 morale not at risk");
    assertTrue(!sys.isDepartureRisk("e1", -49.9f), "Just above threshold not at risk");

    // Morale at threshold — is a risk
    assertTrue(sys.isDepartureRisk("e1", -50.0f), "At threshold is at risk");

    // Morale below threshold — is a risk
    assertTrue(sys.isDepartureRisk("e1", -100.0f), "Below threshold is at risk");
    assertTrue(sys.isDepartureRisk("e1", -51.0f), "Slightly below is at risk");

    // Different thresholds
    sys.setDepartureThreshold("e1", -20.0f);
    assertTrue(sys.isDepartureRisk("e1", -20.0f), "At -20 threshold is risk");
    assertTrue(!sys.isDepartureRisk("e1", -19.0f), "Above -20 threshold not risk");

    // Missing entity returns false
    assertTrue(!sys.isDepartureRisk("missing", -100.0f), "Missing entity not at risk");
}

static void testFactionBehaviorMissingEntity() {
    std::cout << "\n=== FactionBehavior: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::FactionBehaviorModifierSystem sys(&world);

    assertTrue(!sys.setProfile("missing", FP::Industrial), "setProfile missing fails");
    assertTrue(!sys.setFactionId("missing", "x"), "setFactionId missing fails");
    assertTrue(!sys.setMoraleBias("missing", 5.0f), "setMoraleBias missing fails");
    assertTrue(!sys.setChatterRateMult("missing", 1.5f), "setChatterRateMult missing fails");
    assertTrue(!sys.setCombatPreference("missing", 0.5f), "setCombatPreference missing fails");
    assertTrue(!sys.setMiningPreference("missing", 0.5f), "setMiningPreference missing fails");
    assertTrue(!sys.setExplorationPreference("missing", 0.5f), "setExplorationPreference missing fails");
    assertTrue(!sys.setTradePreference("missing", 0.5f), "setTradePreference missing fails");
    assertTrue(!sys.setDepartureThreshold("missing", -30.0f), "setDepartureThreshold missing fails");

    assertTrue(sys.getProfile("missing") == FP::Neutral, "getProfile missing returns Neutral");
    assertTrue(sys.getMoraleDriver("missing") == MD::None, "getMoraleDriver missing returns None");
    assertTrue(approxEqual(sys.getMoraleBias("missing"), 0.0f), "getMoraleBias missing returns 0");
    assertTrue(approxEqual(sys.getChatterRateMult("missing"), 0.0f), "getChatterRateMult missing returns 0");
    assertTrue(approxEqual(sys.getCombatPreference("missing"), 0.0f), "getCombatPreference missing returns 0");
    assertTrue(approxEqual(sys.getMiningPreference("missing"), 0.0f), "getMiningPreference missing returns 0");
    assertTrue(approxEqual(sys.getExplorationPreference("missing"), 0.0f), "getExplorationPreference missing returns 0");
    assertTrue(approxEqual(sys.getTradePreference("missing"), 0.0f), "getTradePreference missing returns 0");
    assertTrue(approxEqual(sys.getDepartureThreshold("missing"), 0.0f), "getDepartureThreshold missing returns 0");
    assertTrue(sys.getFactionId("missing").empty(), "getFactionId missing returns empty");
    assertTrue(sys.getTotalProfileChanges("missing") == 0, "getTotalProfileChanges missing returns 0");
    assertTrue(!sys.isDepartureRisk("missing", -100.0f), "isDepartureRisk missing returns false");
    assertTrue(sys.getDominantActivity("missing") == "None", "getDominantActivity missing returns None");
}

void run_faction_behavior_modifier_system_tests() {
    testFactionBehaviorInit();
    testFactionBehaviorSetProfileIndustrial();
    testFactionBehaviorSetProfileMilitaristic();
    testFactionBehaviorSetProfileNomadic();
    testFactionBehaviorSetProfileCorporate();
    testFactionBehaviorSetProfileNeutral();
    testFactionBehaviorFineGrainedConfig();
    testFactionBehaviorApplyMoraleModifier();
    testFactionBehaviorGetDominantActivity();
    testFactionBehaviorDepartureRisk();
    testFactionBehaviorMissingEntity();
}
