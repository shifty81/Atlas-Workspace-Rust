// Tests for: TitanCountermeasureSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/titan_countermeasure_system.h"

using namespace atlas;
using CM = components::CountermeasureType;

static void testTitanCMInit() {
    std::cout << "\n=== TitanCountermeasure: Init ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getOperationCount("e1") == 0, "Zero operations");
    assertTrue(approxEqual(sys.getTitanDelayScore("e1"), 0.0f), "Delay score = 0");
    assertTrue(approxEqual(sys.getPirateAwareness("e1"), 0.0f), "Awareness = 0");
    assertTrue(!sys.isAwarenessHigh("e1"), "Awareness not high initially");
    assertTrue(sys.getTotalOpsExecuted("e1") == 0, "Zero ops executed");
    assertTrue(sys.getTotalSabotageOps("e1") == 0, "Zero sabotage ops");
    assertTrue(sys.getTotalIntelOps("e1") == 0, "Zero intel ops");
    assertTrue(sys.getRetaliationEvents("e1") == 0, "Zero retaliation events");
    assertTrue(sys.getPlayerId("e1").empty(), "Empty player id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testTitanCMExecuteHarassment() {
    std::cout << "\n=== TitanCountermeasure: ExecuteHarassment ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.executeOp("e1", "op1", CM::HarassmentRaid, "node_alpha", 0.8f),
               "executeOp harassment succeeds");
    assertTrue(sys.getOperationCount("e1") == 1, "Op count = 1");
    assertTrue(sys.hasOp("e1", "op1"), "hasOp op1");
    assertTrue(sys.getTotalOpsExecuted("e1") == 1, "Total ops = 1");
    assertTrue(sys.getTitanDelayScore("e1") > 0.0f, "Delay score > 0");
    assertTrue(sys.getPirateAwareness("e1") > 0.0f, "Awareness increased");

    // Effectiveness stored
    assertTrue(approxEqual(sys.getOpEffectiveness("e1", "op1"), 0.8f),
               "Op effectiveness = 0.8");
    // Delay contributed: 0.8 × 10 × (1 - awareness_penalty)
    assertTrue(sys.getOpDelayContributed("e1", "op1") > 0.0f, "Delay contributed > 0");

    // Duplicate op id rejected
    assertTrue(!sys.executeOp("e1", "op1", CM::HarassmentRaid, "node_beta", 0.5f),
               "Duplicate op id rejected");

    // Invalid inputs
    assertTrue(!sys.executeOp("e1", "", CM::HarassmentRaid, "node", 0.5f),
               "Empty op_id rejected");
    assertTrue(!sys.executeOp("e1", "op2", CM::HarassmentRaid, "", 0.5f),
               "Empty target_node rejected");
    assertTrue(!sys.executeOp("e1", "op2", CM::HarassmentRaid, "node", -0.1f),
               "Negative effectiveness rejected");
    assertTrue(!sys.executeOp("e1", "op2", CM::HarassmentRaid, "node", 1.1f),
               "Over-1 effectiveness rejected");

    assertTrue(!sys.executeOp("missing", "op99", CM::HarassmentRaid, "node", 0.5f),
               "Missing entity rejected");
}

static void testTitanCMSabotageOp() {
    std::cout << "\n=== TitanCountermeasure: SabotageOp ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.executeOp("e1", "op1", CM::SabotageOp, "reactor_node", 0.8f);
    assertTrue(sys.getTotalSabotageOps("e1") == 1, "Total sabotage = 1");
    assertTrue(sys.getCountByType("e1", CM::SabotageOp) == 1, "Count by type = 1");

    // Sabotage delay should be double of standard ops (base 0.8×10×2 = 16)
    float delay = sys.getOpDelayContributed("e1", "op1");
    assertTrue(delay > 10.0f, "Sabotage delay > 10 (2x multiplier)");
    assertTrue(sys.getTotalOpsExecuted("e1") == 1, "Total ops = 1");
}

static void testTitanCMIntelOp() {
    std::cout << "\n=== TitanCountermeasure: IntelOp ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Intel ops generate less awareness than other op types
    sys.executeOp("e1", "op1", CM::IntelGathering, "assembly_site", 0.9f);
    assertTrue(sys.getTotalIntelOps("e1") == 1, "Total intel ops = 1");
    assertTrue(sys.getCountByType("e1", CM::IntelGathering) == 1,
               "Count by IntelGathering = 1");

    // Compare awareness from harassment vs intel
    float intel_awareness = sys.getPirateAwareness("e1");

    ecs::World world2;
    systems::TitanCountermeasureSystem sys2(&world2);
    world2.createEntity("e2");
    sys2.initialize("e2");
    sys2.executeOp("e2", "op2", CM::HarassmentRaid, "assembly_site", 0.9f);
    float harassment_awareness = sys2.getPirateAwareness("e2");

    assertTrue(intel_awareness < harassment_awareness,
               "Intel ops generate less awareness than harassment");
}

static void testTitanCMAwarenessDecay() {
    std::cout << "\n=== TitanCountermeasure: AwarenessDecay ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.applyAwarenessBoost("e1", 0.5f);
    assertTrue(approxEqual(sys.getPirateAwareness("e1"), 0.5f),
               "Awareness = 0.5 after boost");

    float before = sys.getPirateAwareness("e1");
    sys.update(10.0f);  // 10s × 0.005 = 0.05 decay
    assertTrue(sys.getPirateAwareness("e1") < before, "Awareness decays over time");

    // Full decay to zero
    sys.update(1000.0f);
    assertTrue(approxEqual(sys.getPirateAwareness("e1"), 0.0f),
               "Awareness decays to 0");
}

static void testTitanCMAwarenessThreshold() {
    std::cout << "\n=== TitanCountermeasure: AwarenessThreshold ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setAwarenessThreshold("e1", 0.5f);
    sys.applyAwarenessBoost("e1", 0.4f);
    assertTrue(!sys.isAwarenessHigh("e1"), "Not high at 0.4 / threshold 0.5");

    sys.applyAwarenessBoost("e1", 0.2f);  // now 0.6
    assertTrue(sys.isAwarenessHigh("e1"), "High at 0.6 / threshold 0.5");
}

static void testTitanCMRetaliation() {
    std::cout << "\n=== TitanCountermeasure: Retaliation ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Pre-set high awareness so next op triggers retaliation
    sys.setAwarenessThreshold("e1", 0.3f);
    sys.applyAwarenessBoost("e1", 0.6f);

    sys.executeOp("e1", "op1", CM::HarassmentRaid, "node", 1.0f);
    assertTrue(sys.getRetaliationEvents("e1") >= 1, "Retaliation recorded");

    // Manual retaliation record
    int before = sys.getRetaliationEvents("e1");
    assertTrue(sys.recordRetaliation("e1"), "recordRetaliation succeeds");
    assertTrue(sys.getRetaliationEvents("e1") == before + 1, "Retaliation count incremented");

    assertTrue(!sys.recordRetaliation("missing"), "recordRetaliation on missing fails");
}

static void testTitanCMApplyAwarenessBoost() {
    std::cout << "\n=== TitanCountermeasure: ApplyAwarenessBoost ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyAwarenessBoost("e1", 0.3f), "boost succeeds");
    assertTrue(approxEqual(sys.getPirateAwareness("e1"), 0.3f), "Awareness = 0.3");

    // Clamped at 1.0
    assertTrue(sys.applyAwarenessBoost("e1", 2.0f), "Large boost clamped");
    assertTrue(approxEqual(sys.getPirateAwareness("e1"), 1.0f), "Awareness capped at 1.0");

    // Negative boost rejected
    assertTrue(!sys.applyAwarenessBoost("e1", -0.1f), "Negative boost rejected");

    assertTrue(!sys.applyAwarenessBoost("missing", 0.1f), "Missing entity fails");
}

static void testTitanCMMultipleOpTypes() {
    std::cout << "\n=== TitanCountermeasure: MultipleOpTypes ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.executeOp("e1", "op1", CM::HarassmentRaid, "n1", 0.7f);
    sys.executeOp("e1", "op2", CM::ResourceDenial, "n2", 0.6f);
    sys.executeOp("e1", "op3", CM::IntelGathering, "n3", 0.9f);
    sys.executeOp("e1", "op4", CM::SabotageOp, "n4", 0.8f);
    sys.executeOp("e1", "op5", CM::PropagandaDisruption, "n5", 0.5f);
    sys.executeOp("e1", "op6", CM::AllianceSupport, "n6", 0.6f);

    assertTrue(sys.getOperationCount("e1") == 6, "6 operations");
    assertTrue(sys.getTotalOpsExecuted("e1") == 6, "Total executed = 6");
    assertTrue(sys.getTotalSabotageOps("e1") == 1, "1 sabotage op");
    assertTrue(sys.getTotalIntelOps("e1") == 1, "1 intel op");
    assertTrue(sys.getTitanDelayScore("e1") > 0.0f, "Delay > 0");

    assertTrue(sys.getCountByType("e1", CM::HarassmentRaid) == 1, "1 harassment");
    assertTrue(sys.getCountByType("e1", CM::ResourceDenial) == 1, "1 resource denial");
    assertTrue(sys.getCountByType("e1", CM::IntelGathering) == 1, "1 intel");
    assertTrue(sys.getCountByType("e1", CM::SabotageOp) == 1, "1 sabotage");
    assertTrue(sys.getCountByType("e1", CM::PropagandaDisruption) == 1, "1 propaganda");
    assertTrue(sys.getCountByType("e1", CM::AllianceSupport) == 1, "1 alliance");
}

static void testTitanCMRemoveAndClear() {
    std::cout << "\n=== TitanCountermeasure: RemoveAndClear ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.executeOp("e1", "op1", CM::HarassmentRaid, "n1", 0.5f);
    sys.executeOp("e1", "op2", CM::SabotageOp, "n2", 0.5f);

    assertTrue(sys.removeOp("e1", "op1"), "removeOp succeeds");
    assertTrue(!sys.hasOp("e1", "op1"), "op1 no longer present");
    assertTrue(sys.hasOp("e1", "op2"), "op2 still present");
    assertTrue(sys.getOperationCount("e1") == 1, "Count = 1 after remove");

    assertTrue(!sys.removeOp("e1", "ghost"), "Remove nonexistent fails");

    assertTrue(sys.clearOps("e1"), "clearOps succeeds");
    assertTrue(sys.getOperationCount("e1") == 0, "Count = 0 after clear");

    assertTrue(!sys.removeOp("missing", "op1"), "Remove on missing entity fails");
    assertTrue(!sys.clearOps("missing"), "Clear on missing entity fails");
}

static void testTitanCMConfiguration() {
    std::cout << "\n=== TitanCountermeasure: Configuration ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setMaxOperations("e1", 30), "setMaxOperations(30) succeeds");
    assertTrue(!sys.setMaxOperations("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxOperations("e1", -1), "Negative max rejected");

    assertTrue(sys.setAwarenessDecayRate("e1", 0.01f), "setDecayRate succeeds");
    assertTrue(!sys.setAwarenessDecayRate("e1", -0.1f), "Negative rate rejected");

    assertTrue(sys.setAwarenessThreshold("e1", 0.6f), "setThreshold(0.6) succeeds");
    assertTrue(!sys.setAwarenessThreshold("e1", -0.1f), "Negative threshold rejected");
    assertTrue(!sys.setAwarenessThreshold("e1", 1.1f), "Over-1 threshold rejected");

    assertTrue(sys.setPlayerId("e1", "player_one"), "setPlayerId succeeds");
    assertTrue(sys.getPlayerId("e1") == "player_one", "PlayerId = player_one");
    assertTrue(!sys.setPlayerId("e1", ""), "Empty player id rejected");

    assertTrue(!sys.setMaxOperations("missing", 10), "setMaxOps on missing fails");
    assertTrue(!sys.setAwarenessDecayRate("missing", 0.1f), "setDecayRate on missing fails");
    assertTrue(!sys.setPlayerId("missing", "x"), "setPlayerId on missing fails");
}

static void testTitanCMMissingEntity() {
    std::cout << "\n=== TitanCountermeasure: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::TitanCountermeasureSystem sys(&world);

    assertTrue(sys.getOperationCount("missing") == 0, "getOperationCount = 0");
    assertTrue(!sys.hasOp("missing", "op1"), "hasOp = false");
    assertTrue(approxEqual(sys.getTitanDelayScore("missing"), 0.0f),
               "getTitanDelayScore = 0");
    assertTrue(approxEqual(sys.getPirateAwareness("missing"), 0.0f),
               "getPirateAwareness = 0");
    assertTrue(!sys.isAwarenessHigh("missing"), "isAwarenessHigh = false");
    assertTrue(approxEqual(sys.getOpEffectiveness("missing", "x"), 0.0f),
               "getOpEffectiveness = 0");
    assertTrue(approxEqual(sys.getOpDelayContributed("missing", "x"), 0.0f),
               "getOpDelayContributed = 0");
    assertTrue(sys.getTotalOpsExecuted("missing") == 0, "getTotalOps = 0");
    assertTrue(sys.getTotalSabotageOps("missing") == 0, "getTotalSabotage = 0");
    assertTrue(sys.getTotalIntelOps("missing") == 0, "getTotalIntel = 0");
    assertTrue(sys.getRetaliationEvents("missing") == 0, "getRetaliation = 0");
    assertTrue(sys.getCountByType("missing", CM::SabotageOp) == 0, "getCountByType = 0");
    assertTrue(sys.getPlayerId("missing").empty(), "getPlayerId = ''");
    assertTrue(!sys.executeOp("missing", "op", CM::HarassmentRaid, "n", 0.5f),
               "executeOp = false");
    assertTrue(!sys.removeOp("missing", "op"), "removeOp = false");
    assertTrue(!sys.recordRetaliation("missing"), "recordRetaliation = false");
    assertTrue(!sys.applyAwarenessBoost("missing", 0.1f), "applyBoost = false");
}

void run_titan_countermeasure_system_tests() {
    testTitanCMInit();
    testTitanCMExecuteHarassment();
    testTitanCMSabotageOp();
    testTitanCMIntelOp();
    testTitanCMAwarenessDecay();
    testTitanCMAwarenessThreshold();
    testTitanCMRetaliation();
    testTitanCMApplyAwarenessBoost();
    testTitanCMMultipleOpTypes();
    testTitanCMRemoveAndClear();
    testTitanCMConfiguration();
    testTitanCMMissingEntity();
}
