#include "../engine/gameplay/MechanicAsset.h"
#include <iostream>
#include <cassert>

using namespace atlas::gameplay;

void test_mechanic_register() {
    MechanicRegistry registry;
    registry.Init();

    uint32_t id = registry.Register("DoubleJump", MechanicType::Action, "graphs/double_jump.atlas");
    assert(id > 0);
    assert(registry.Count() == 1);

    auto* mechanic = registry.Get(id);
    assert(mechanic != nullptr);
    assert(mechanic->name == "DoubleJump");
    assert(mechanic->type == MechanicType::Action);
    assert(mechanic->graphRef == "graphs/double_jump.atlas");

    std::cout << "[PASS] test_mechanic_register" << std::endl;
}

void test_mechanic_unregister() {
    MechanicRegistry registry;
    registry.Init();

    uint32_t id = registry.Register("Dash", MechanicType::Action);
    registry.Unregister(id);
    assert(registry.Count() == 0);
    assert(registry.Get(id) == nullptr);

    std::cout << "[PASS] test_mechanic_unregister" << std::endl;
}

void test_mechanic_find_by_name() {
    MechanicRegistry registry;
    registry.Init();

    registry.Register("FireDamage", MechanicType::Modifier);
    registry.Register("IceSlow", MechanicType::Modifier);

    auto* found = registry.FindByName("IceSlow");
    assert(found != nullptr);
    assert(found->name == "IceSlow");

    assert(registry.FindByName("NonExistent") == nullptr);

    std::cout << "[PASS] test_mechanic_find_by_name" << std::endl;
}

void test_mechanic_params() {
    MechanicRegistry registry;
    registry.Init();

    uint32_t id = registry.Register("Heal", MechanicType::Action);
    registry.AddParam(id, "amount", "float", 25.0f);
    registry.AddParam(id, "cooldown", "float", 5.0f);

    auto* mechanic = registry.Get(id);
    assert(mechanic->params.size() == 2);
    assert(mechanic->params[0].name == "amount");
    assert(mechanic->params[0].defaultValue == 25.0f);
    assert(mechanic->params[1].name == "cooldown");

    std::cout << "[PASS] test_mechanic_params" << std::endl;
}

void test_mechanic_get_by_type() {
    MechanicRegistry registry;
    registry.Init();

    registry.Register("Jump", MechanicType::Action);
    registry.Register("OnDeath", MechanicType::Trigger);
    registry.Register("Sprint", MechanicType::Action);
    registry.Register("Poisoned", MechanicType::State);

    auto actions = registry.GetByType(MechanicType::Action);
    assert(actions.size() == 2);

    auto triggers = registry.GetByType(MechanicType::Trigger);
    assert(triggers.size() == 1);

    auto rules = registry.GetByType(MechanicType::Rule);
    assert(rules.size() == 0);

    std::cout << "[PASS] test_mechanic_get_by_type" << std::endl;
}
