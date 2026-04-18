#include "../engine/net/Replication.h"
#include "../engine/ecs/ECS.h"
#include <iostream>
#include <cassert>

using namespace atlas::net;
using namespace atlas::ecs;

struct ReplPosition {
    float x = 0.0f;
    float y = 0.0f;
};

struct ReplHealth {
    int hp = 100;
};

void test_replication_add_rule() {
    ReplicationManager mgr;

    ReplicationRule rule;
    rule.typeTag = 1;
    rule.componentName = "Position";
    rule.frequency = ReplicateFrequency::OnChange;
    rule.direction = ReplicateDirection::ServerToClient;

    mgr.AddRule(rule);

    assert(mgr.RuleCount() == 1);
    assert(mgr.HasRule(1));
    assert(!mgr.HasRule(2));

    std::cout << "[PASS] test_replication_add_rule" << std::endl;
}

void test_replication_remove_rule() {
    ReplicationManager mgr;

    ReplicationRule rule;
    rule.typeTag = 1;
    rule.componentName = "Position";
    mgr.AddRule(rule);

    assert(mgr.RuleCount() == 1);

    mgr.RemoveRule(1);
    assert(mgr.RuleCount() == 0);
    assert(!mgr.HasRule(1));

    std::cout << "[PASS] test_replication_remove_rule" << std::endl;
}

void test_replication_get_rule() {
    ReplicationManager mgr;

    ReplicationRule rule;
    rule.typeTag = 42;
    rule.componentName = "Health";
    rule.frequency = ReplicateFrequency::EveryTick;
    rule.reliable = false;
    rule.priority = 200;
    mgr.AddRule(rule);

    const auto* found = mgr.GetRule(42);
    assert(found != nullptr);
    assert(found->componentName == "Health");
    assert(found->frequency == ReplicateFrequency::EveryTick);
    assert(found->reliable == false);
    assert(found->priority == 200);

    assert(mgr.GetRule(99) == nullptr);

    std::cout << "[PASS] test_replication_get_rule" << std::endl;
}

void test_replication_replace_rule() {
    ReplicationManager mgr;

    ReplicationRule rule1;
    rule1.typeTag = 1;
    rule1.componentName = "Position";
    rule1.frequency = ReplicateFrequency::OnChange;
    mgr.AddRule(rule1);

    ReplicationRule rule2;
    rule2.typeTag = 1;
    rule2.componentName = "Position";
    rule2.frequency = ReplicateFrequency::EveryTick;
    mgr.AddRule(rule2);

    assert(mgr.RuleCount() == 1);
    assert(mgr.GetRule(1)->frequency == ReplicateFrequency::EveryTick);

    std::cout << "[PASS] test_replication_replace_rule" << std::endl;
}

void test_replication_dirty_tracking() {
    ReplicationManager mgr;

    mgr.MarkDirty(1, 10);
    mgr.MarkDirty(1, 20);
    mgr.MarkDirty(2, 10);

    assert(mgr.IsDirty(1, 10));
    assert(mgr.IsDirty(1, 20));
    assert(mgr.IsDirty(2, 10));
    assert(!mgr.IsDirty(2, 20));
    assert(!mgr.IsDirty(3, 10));

    mgr.ClearDirty();
    assert(!mgr.IsDirty(1, 10));
    assert(!mgr.IsDirty(1, 20));

    std::cout << "[PASS] test_replication_dirty_tracking" << std::endl;
}

void test_replication_dirty_no_duplicates() {
    ReplicationManager mgr;

    mgr.MarkDirty(1, 10);
    mgr.MarkDirty(1, 10);
    mgr.MarkDirty(1, 10);

    assert(mgr.IsDirty(1, 10));

    mgr.ClearDirty();
    assert(!mgr.IsDirty(1, 10));

    std::cout << "[PASS] test_replication_dirty_no_duplicates" << std::endl;
}

void test_replication_collect_delta() {
    World world;
    world.RegisterComponent<ReplPosition>(1);

    ReplicationManager mgr;
    mgr.SetWorld(&world);

    ReplicationRule rule;
    rule.typeTag = 1;
    rule.componentName = "Position";
    rule.frequency = ReplicateFrequency::OnChange;
    mgr.AddRule(rule);

    EntityID e = world.CreateEntity();
    world.AddComponent<ReplPosition>(e, {5.0f, 10.0f});

    mgr.MarkDirty(1, e);

    auto delta = mgr.CollectDelta(1);
    // Delta should have at least the header (tick + ruleCount = 8 bytes)
    assert(delta.size() >= 8);

    // After collecting, dirty flags should be cleared
    assert(!mgr.IsDirty(1, e));

    std::cout << "[PASS] test_replication_collect_delta" << std::endl;
}

void test_replication_apply_delta() {
    ReplicationManager mgr;

    // Build a minimal delta: tick=1, ruleCount=0
    std::vector<uint8_t> delta(8, 0);
    uint32_t tick = 1;
    uint32_t ruleCount = 0;
    std::memcpy(delta.data(), &tick, 4);
    std::memcpy(delta.data() + 4, &ruleCount, 4);

    assert(mgr.ApplyDelta(delta));

    // Too small should fail
    std::vector<uint8_t> tooSmall(4, 0);
    assert(!mgr.ApplyDelta(tooSmall));

    std::cout << "[PASS] test_replication_apply_delta" << std::endl;
}

void test_replication_multiple_rules() {
    ReplicationManager mgr;

    ReplicationRule r1;
    r1.typeTag = 1;
    r1.componentName = "Position";
    r1.frequency = ReplicateFrequency::OnChange;
    mgr.AddRule(r1);

    ReplicationRule r2;
    r2.typeTag = 2;
    r2.componentName = "Health";
    r2.frequency = ReplicateFrequency::EveryTick;
    mgr.AddRule(r2);

    ReplicationRule r3;
    r3.typeTag = 3;
    r3.componentName = "Velocity";
    r3.direction = ReplicateDirection::Bidirectional;
    mgr.AddRule(r3);

    assert(mgr.RuleCount() == 3);
    assert(mgr.Rules().size() == 3);

    mgr.RemoveRule(2);
    assert(mgr.RuleCount() == 2);
    assert(!mgr.HasRule(2));
    assert(mgr.HasRule(1));
    assert(mgr.HasRule(3));

    std::cout << "[PASS] test_replication_multiple_rules" << std::endl;
}

void test_replication_delta_roundtrip() {
    // Create source world with entities and components
    World srcWorld;
    srcWorld.RegisterComponent<ReplPosition>(1);
    srcWorld.RegisterComponent<ReplHealth>(2);

    ReplicationManager srcMgr;
    srcMgr.SetWorld(&srcWorld);

    ReplicationRule posRule;
    posRule.typeTag = 1;
    posRule.componentName = "Position";
    posRule.frequency = ReplicateFrequency::OnChange;
    srcMgr.AddRule(posRule);

    ReplicationRule hpRule;
    hpRule.typeTag = 2;
    hpRule.componentName = "Health";
    hpRule.frequency = ReplicateFrequency::OnChange;
    srcMgr.AddRule(hpRule);

    EntityID e1 = srcWorld.CreateEntity();
    srcWorld.AddComponent<ReplPosition>(e1, {42.0f, 84.0f});
    srcWorld.AddComponent<ReplHealth>(e1, {75});

    srcMgr.MarkDirty(1, e1);
    srcMgr.MarkDirty(2, e1);

    auto delta = srcMgr.CollectDelta(10);
    assert(delta.size() > 8);

    // Create destination world and apply the delta
    World dstWorld;
    dstWorld.RegisterComponent<ReplPosition>(1);
    dstWorld.RegisterComponent<ReplHealth>(2);
    EntityID de1 = dstWorld.CreateEntity();
    dstWorld.AddComponent<ReplPosition>(de1, {0.0f, 0.0f});
    dstWorld.AddComponent<ReplHealth>(de1, {0});

    ReplicationManager dstMgr;
    dstMgr.SetWorld(&dstWorld);

    assert(dstMgr.ApplyDelta(delta));

    // Verify component data was applied
    auto* pos = dstWorld.GetComponent<ReplPosition>(de1);
    assert(pos != nullptr);
    assert(pos->x == 42.0f);
    assert(pos->y == 84.0f);

    auto* hp = dstWorld.GetComponent<ReplHealth>(de1);
    assert(hp != nullptr);
    assert(hp->hp == 75);

    std::cout << "[PASS] test_replication_delta_roundtrip" << std::endl;
}

void test_replication_delta_every_tick() {
    World world;
    world.RegisterComponent<ReplPosition>(1);

    ReplicationManager mgr;
    mgr.SetWorld(&world);

    ReplicationRule rule;
    rule.typeTag = 1;
    rule.componentName = "Position";
    rule.frequency = ReplicateFrequency::EveryTick;
    mgr.AddRule(rule);

    EntityID e = world.CreateEntity();
    world.AddComponent<ReplPosition>(e, {7.0f, 14.0f});

    // EveryTick frequency should include data even without marking dirty
    auto delta = mgr.CollectDelta(1);
    assert(delta.size() > 8);

    // Apply to another world
    World dst;
    dst.RegisterComponent<ReplPosition>(1);
    EntityID de = dst.CreateEntity();
    dst.AddComponent<ReplPosition>(de, {0.0f, 0.0f});

    ReplicationManager dstMgr;
    dstMgr.SetWorld(&dst);
    assert(dstMgr.ApplyDelta(delta));

    auto* pos = dst.GetComponent<ReplPosition>(de);
    assert(pos != nullptr);
    assert(pos->x == 7.0f);
    assert(pos->y == 14.0f);

    std::cout << "[PASS] test_replication_delta_every_tick" << std::endl;
}
