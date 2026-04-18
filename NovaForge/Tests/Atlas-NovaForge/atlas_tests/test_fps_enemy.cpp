/**
 * Tests for FPS enemy archetypes and squad factory (Phase 23):
 *   - EnemyArchetype and SquadRole enum names
 *   - Default stats per archetype
 *   - EnemySquad factory methods (standard, support)
 *   - AliveCount, AllDown
 *   - ApplyDamage, FindMember
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/gameplay/fps_enemy.h"

using namespace atlas::gameplay;

void test_enemy_archetype_names() {
    assert(std::string(EnemyArchetypeName(EnemyArchetype::GRUNT))      == "Grunt");
    assert(std::string(EnemyArchetypeName(EnemyArchetype::BREACHER))   == "Breacher");
    assert(std::string(EnemyArchetypeName(EnemyArchetype::TECHNICIAN)) == "Technician");
}

void test_squad_role_names() {
    assert(std::string(SquadRoleName(SquadRole::BREACHER))   == "Breacher");
    assert(std::string(SquadRoleName(SquadRole::SUPPRESSOR)) == "Suppressor");
    assert(std::string(SquadRoleName(SquadRole::FLANKER))    == "Flanker");
}

void test_default_stats_grunt() {
    auto s = DefaultStatsForArchetype(EnemyArchetype::GRUNT);
    assert(s.health > 0.0f);
    assert(s.moveSpeed > 0.0f);
    assert(!s.hasShield);
}

void test_default_stats_breacher_has_shield() {
    auto s = DefaultStatsForArchetype(EnemyArchetype::BREACHER);
    assert(s.hasShield);
    assert(s.shieldHP > 0.0f);
    assert(s.moveSpeed > 4.0f);  // faster than grunt
}

void test_default_stats_technician_low_hp() {
    auto s = DefaultStatsForArchetype(EnemyArchetype::TECHNICIAN);
    auto g = DefaultStatsForArchetype(EnemyArchetype::GRUNT);
    assert(s.health < g.health);  // fragile support unit
}

void test_enemy_is_alive() {
    FPSEnemy e;
    e.stats.health = 100.0f;
    assert(e.IsAlive());
    e.stats.health = 0.0f;
    assert(!e.IsAlive());
}

void test_squad_standard_difficulty_1() {
    auto squad = EnemySquad::CreateStandard("sq1", 1);
    assert(squad.id == "sq1");
    assert(squad.members.size() == 2);  // suppressor + breacher (no flanker at diff 1)
    assert(squad.AliveCount() == 2);
    assert(!squad.AllDown());
}

void test_squad_standard_difficulty_2_has_flanker() {
    auto squad = EnemySquad::CreateStandard("sq2", 2);
    assert(squad.members.size() == 3);
    // Find flanker
    bool hasFlanker = false;
    for (const auto& m : squad.members)
        if (m.squadRole == SquadRole::FLANKER) hasFlanker = true;
    assert(hasFlanker);
}

void test_squad_support_has_technician() {
    auto squad = EnemySquad::CreateSupport("sup1");
    bool hasTech = false;
    for (const auto& m : squad.members)
        if (m.archetype == EnemyArchetype::TECHNICIAN) hasTech = true;
    assert(hasTech);
    assert(squad.members.size() == 3);
}

void test_squad_all_members_alive_initially() {
    auto squad = EnemySquad::CreateStandard("sq3", 2);
    assert(squad.AliveCount() == static_cast<int>(squad.members.size()));
}

void test_squad_apply_damage_kills_member() {
    auto squad = EnemySquad::CreateStandard("sq4", 1);
    const std::string id = squad.members[0].id;
    float hp = squad.members[0].stats.health;

    squad.ApplyDamage(id, hp);
    assert(!squad.FindMember(id)->IsAlive());
}

void test_squad_all_down_after_full_wipe() {
    auto squad = EnemySquad::CreateStandard("sq5", 1);
    for (auto& m : squad.members)
        m.stats.health = 0.0f;
    assert(squad.AllDown());
}

void test_squad_find_member_null_for_unknown() {
    auto squad = EnemySquad::CreateStandard("sq6", 1);
    assert(squad.FindMember("nonexistent") == nullptr);
}

void test_squad_difficulty_scales_health() {
    auto low  = EnemySquad::CreateStandard("a", 1);
    auto high = EnemySquad::CreateStandard("b", 3);
    // Same member index (suppressor at [0])
    assert(high.members[0].stats.health > low.members[0].stats.health);
}

void test_squad_squad_ids_match() {
    auto squad = EnemySquad::CreateStandard("mySquad", 1);
    for (const auto& m : squad.members)
        assert(m.squadID == "mySquad");
}
