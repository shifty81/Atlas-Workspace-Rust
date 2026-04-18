/**
 * Tests for LegendSystem (Phase 24 — Legend & World Simulation):
 *   - PlayerReputation EWMA updates and clamping
 *   - Memory recording and retrieval
 *   - Memory tier promotion (RUN→CAMPAIGN→LEGEND)
 *   - ClearRunMemories
 *   - Boss mutation derivation
 */

#include <cassert>
#include <string>
#include <vector>
#include "../cpp_client/include/gameplay/legend_system.h"

using namespace atlas::gameplay;

void test_legend_memory_tier_names() {
    assert(std::string(MemoryTierName(MemoryTier::RUN))      == "Run");
    assert(std::string(MemoryTierName(MemoryTier::CAMPAIGN)) == "Campaign");
    assert(std::string(MemoryTierName(MemoryTier::LEGEND))   == "Legend");
}

void test_legend_reputation_initial() {
    LegendSystem sys;
    const auto& rep = sys.Reputation();
    assert(rep.aggression == 0.0f);
    assert(rep.stealth    == 0.0f);
    assert(rep.precision  == 0.0f);
    assert(rep.authority  == 0.0f);
    assert(rep.chaos      == 0.0f);
}

void test_legend_record_action_updates_reputation() {
    LegendSystem sys;
    // Heavy aggression
    for (int i = 0; i < 20; ++i)
        sys.RecordAction(1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    assert(sys.Reputation().aggression > 0.5f);
}

void test_legend_reputation_clamped_to_0_1() {
    LegendSystem sys;
    for (int i = 0; i < 100; ++i)
        sys.RecordAction(2.0f, -1.0f, 0.5f, 0.0f, 0.0f);
    assert(sys.Reputation().aggression <= 1.0f);
    assert(sys.Reputation().stealth    >= 0.0f);
}

void test_legend_record_memory_stored() {
    LegendSystem sys;
    sys.RecordMemory("high_explosives", MemoryTier::RUN, 0.8f, "asteroid_field");
    assert(sys.Memories().size() == 1);
    const auto* m = sys.FindMemory("high_explosives");
    assert(m != nullptr);
    assert(m->tier == MemoryTier::RUN);
    assert(m->strength == 0.8f);
}

void test_legend_record_memory_same_key_accumulates() {
    LegendSystem sys;
    sys.RecordMemory("precision_sniper", MemoryTier::RUN, 0.5f, "hangar");
    sys.RecordMemory("precision_sniper", MemoryTier::RUN, 0.9f, "hangar");
    // Run count should be 2, strength should be max(0.5, 0.9)
    const auto* m = sys.FindMemory("precision_sniper");
    assert(m != nullptr);
    assert(m->runCount == 2);
    assert(m->strength == 0.9f);
}

void test_legend_memory_count_by_tier() {
    LegendSystem sys;
    sys.RecordMemory("key1", MemoryTier::RUN,      0.5f);
    sys.RecordMemory("key2", MemoryTier::CAMPAIGN,  0.7f);
    sys.RecordMemory("key3", MemoryTier::LEGEND,    0.9f);
    assert(sys.MemoryCount(MemoryTier::RUN)      == 1);
    assert(sys.MemoryCount(MemoryTier::CAMPAIGN) == 1);
    assert(sys.MemoryCount(MemoryTier::LEGEND)   == 1);
}

void test_legend_clear_run_memories() {
    LegendSystem sys;
    sys.RecordMemory("run_key",      MemoryTier::RUN,      0.5f);
    sys.RecordMemory("campaign_key", MemoryTier::CAMPAIGN, 0.7f);
    sys.ClearRunMemories();
    assert(sys.MemoryCount(MemoryTier::RUN)      == 0);
    assert(sys.MemoryCount(MemoryTier::CAMPAIGN) == 1);
}

void test_legend_find_memory_null_for_unknown() {
    LegendSystem sys;
    assert(sys.FindMemory("nonexistent") == nullptr);
}

void test_legend_promote_run_to_campaign() {
    LegendSystem sys;
    // Record the same key multiple times (simulating multiple runs)
    sys.RecordMemory("run_habit", MemoryTier::RUN, 0.5f, "biome_a");
    sys.RecordMemory("run_habit", MemoryTier::RUN, 0.6f, "biome_b");
    // 2 runs recorded (runCount == 2)
    sys.PromoteMemories(2);

    const auto* m = sys.FindMemory("run_habit");
    assert(m != nullptr);
    assert(m->tier == MemoryTier::CAMPAIGN);
}

void test_legend_promote_campaign_to_legend() {
    LegendSystem sys;
    // Build up a memory with enough run count, biome count, and strength
    for (int i = 0; i < 4; ++i)
        sys.RecordMemory("legend_key", MemoryTier::CAMPAIGN,
                         0.9f, "biome_" + std::to_string(i));
    // First promote to campaign
    sys.PromoteMemories(4);
    const auto* m = sys.FindMemory("legend_key");
    assert(m != nullptr);
    assert(m->tier == MemoryTier::LEGEND);
}

void test_legend_boss_mutations_none_without_reputation() {
    LegendSystem sys;
    auto muts = sys.DeriveBossMutations();
    assert(muts.empty());
}

void test_legend_boss_mutation_counter_rush_from_aggression() {
    LegendSystem sys;
    for (int i = 0; i < 50; ++i)
        sys.RecordAction(1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    auto muts = sys.DeriveBossMutations();
    bool hasCounterRush = false;
    for (auto m : muts)
        if (m == BossMutation::COUNTER_RUSH) hasCounterRush = true;
    assert(hasCounterRush);
}

void test_legend_boss_mutation_max_2() {
    LegendSystem sys;
    // Drive all reputations high
    for (int i = 0; i < 50; ++i)
        sys.RecordAction(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    auto muts = sys.DeriveBossMutations();
    assert(static_cast<int>(muts.size()) <= LegendSystem::MAX_BOSS_MUTATIONS);
}

void test_legend_boss_mutation_names() {
    assert(std::string(BossMutationName(BossMutation::SHIELD_PHASING))    == "Shield Phasing");
    assert(std::string(BossMutationName(BossMutation::MOVING_WEAKPOINTS)) == "Moving Weakpoints");
    assert(std::string(BossMutationName(BossMutation::AREA_SCANS))        == "Area Scans");
    assert(std::string(BossMutationName(BossMutation::COUNTER_RUSH))      == "Counter Rush");
}
