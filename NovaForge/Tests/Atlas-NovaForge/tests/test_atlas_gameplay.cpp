#include "../modules/atlas_gameplay/FactionSystem.h"
#include "../modules/atlas_gameplay/CombatFramework.h"
#include "../modules/atlas_gameplay/EconomySystem.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::gameplay;

void test_faction_register() {
    FactionSystem factions;
    auto id = factions.RegisterFaction("Empire", "EMP");
    assert(id > 0);
    assert(factions.HasFaction(id));
    assert(factions.FactionCount() == 1);

    auto* desc = factions.GetFaction(id);
    assert(desc != nullptr);
    assert(desc->name == "Empire");
    assert(desc->tag == "EMP");

    std::cout << "[PASS] test_faction_register" << std::endl;
}

void test_faction_relations() {
    FactionSystem factions;
    auto empire = factions.RegisterFaction("Empire");
    auto rebels = factions.RegisterFaction("Rebels");

    assert(factions.GetRelation(empire, rebels) == FactionRelation::Neutral);

    factions.SetRelation(empire, rebels, FactionRelation::Hostile);
    assert(factions.GetRelation(empire, rebels) == FactionRelation::Hostile);
    assert(factions.GetRelation(rebels, empire) == FactionRelation::Hostile);

    std::cout << "[PASS] test_faction_relations" << std::endl;
}

void test_faction_clear() {
    FactionSystem factions;
    factions.RegisterFaction("Empire");
    factions.RegisterFaction("Rebels");
    assert(factions.FactionCount() == 2);

    factions.Clear();
    assert(factions.FactionCount() == 0);

    std::cout << "[PASS] test_faction_clear" << std::endl;
}

void test_combat_register_unit() {
    CombatFramework combat;
    CombatStats stats{10.0f, 2.0f, 100.0f, 1.0f};
    combat.RegisterUnit(1, stats);

    assert(combat.HasUnit(1));
    assert(combat.UnitCount() == 1);

    auto* s = combat.GetStats(1);
    assert(s != nullptr);
    assert(s->damage == 10.0f);
    assert(s->armor == 2.0f);

    std::cout << "[PASS] test_combat_register_unit" << std::endl;
}

void test_combat_resolve() {
    CombatFramework combat;
    combat.RegisterUnit(1, {20.0f, 0.0f, 100.0f, 1.0f}); // attacker
    combat.RegisterUnit(2, {10.0f, 5.0f, 30.0f, 1.0f});   // defender

    auto result = combat.ResolveCombat(1, 2);
    assert(result.damageDealt == 15.0f); // 20 - 5 armor
    assert(!result.targetDestroyed);

    auto* defStats = combat.GetStats(2);
    assert(defStats->health == 15.0f); // 30 - 15

    auto result2 = combat.ResolveCombat(1, 2);
    assert(result2.targetDestroyed);

    std::cout << "[PASS] test_combat_resolve" << std::endl;
}

void test_combat_clear() {
    CombatFramework combat;
    combat.RegisterUnit(1, {10.0f, 0.0f, 50.0f, 1.0f});
    assert(combat.UnitCount() == 1);
    combat.Clear();
    assert(combat.UnitCount() == 0);

    std::cout << "[PASS] test_combat_clear" << std::endl;
}

void test_economy_register() {
    EconomySystem economy;
    economy.RegisterResource("gold", 100.0, 1000.0);

    assert(economy.HasResource("gold"));
    assert(economy.ResourceCount() == 1);
    assert(economy.GetAmount("gold") == 100.0);
    assert(economy.GetCapacity("gold") == 1000.0);

    std::cout << "[PASS] test_economy_register" << std::endl;
}

void test_economy_transactions() {
    EconomySystem economy;
    economy.RegisterResource("gold", 100.0);

    assert(economy.CanAfford("gold", 50.0));
    assert(!economy.CanAfford("gold", 200.0));

    assert(economy.Spend("gold", 30.0));
    assert(economy.GetAmount("gold") == 70.0);

    assert(!economy.Spend("gold", 100.0)); // can't afford
    assert(economy.GetAmount("gold") == 70.0); // unchanged

    assert(economy.Add("gold", 50.0));
    assert(economy.GetAmount("gold") == 120.0);

    std::cout << "[PASS] test_economy_transactions" << std::endl;
}

void test_economy_capacity() {
    EconomySystem economy;
    economy.RegisterResource("ore", 0.0, 100.0);

    economy.Add("ore", 150.0);
    assert(economy.GetAmount("ore") == 100.0); // capped

    std::cout << "[PASS] test_economy_capacity" << std::endl;
}

void test_economy_clear() {
    EconomySystem economy;
    economy.RegisterResource("gold", 50.0);
    economy.RegisterResource("ore", 25.0);
    assert(economy.ResourceCount() == 2);

    economy.Clear();
    assert(economy.ResourceCount() == 0);

    std::cout << "[PASS] test_economy_clear" << std::endl;
}
