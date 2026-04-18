#include <cassert>
#include <iostream>
#include "../engine/rules/ServerRules.h"

using namespace atlas::rules;

void test_server_rules_register() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    RuleDescriptor desc;
    desc.name = "mining_yield";
    desc.baseValue = 100.0f;
    desc.multiplier = 1.0f;
    rules.RegisterRule(desc);

    assert(rules.HasRule("mining_yield"));
    assert(rules.RuleCount() == 1);
    std::cout << "[PASS] test_server_rules_register" << std::endl;
}

void test_server_rules_get_value() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    RuleDescriptor desc;
    desc.name = "damage_modifier";
    desc.baseValue = 50.0f;
    desc.multiplier = 2.0f;
    rules.RegisterRule(desc);

    float val = rules.GetValue("damage_modifier");
    assert(val >= 99.9f && val <= 100.1f); // 50 * 2 = 100
    std::cout << "[PASS] test_server_rules_get_value" << std::endl;
}

void test_server_rules_set_multiplier() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    RuleDescriptor desc;
    desc.name = "speed_factor";
    desc.baseValue = 10.0f;
    desc.multiplier = 1.0f;
    desc.minMultiplier = 0.5f;
    desc.maxMultiplier = 3.0f;
    rules.RegisterRule(desc);

    assert(rules.SetMultiplier("speed_factor", 2.5f));
    assert(rules.GetMultiplier("speed_factor") >= 2.49f);
    assert(rules.GetMultiplier("speed_factor") <= 2.51f);

    std::cout << "[PASS] test_server_rules_set_multiplier" << std::endl;
}

void test_server_rules_clamp_multiplier() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    RuleDescriptor desc;
    desc.name = "armor_bonus";
    desc.baseValue = 100.0f;
    desc.multiplier = 1.0f;
    desc.minMultiplier = 0.1f;
    desc.maxMultiplier = 5.0f;
    rules.RegisterRule(desc);

    // Try to set above max
    rules.SetMultiplier("armor_bonus", 999.0f);
    assert(rules.GetMultiplier("armor_bonus") <= 5.01f);

    // Try to set below min
    rules.SetMultiplier("armor_bonus", -10.0f);
    assert(rules.GetMultiplier("armor_bonus") >= 0.09f);

    std::cout << "[PASS] test_server_rules_clamp_multiplier" << std::endl;
}

void test_server_rules_missing_rule() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    assert(!rules.HasRule("nonexistent"));
    assert(rules.GetValue("nonexistent") == 0.0f);
    assert(rules.GetMultiplier("nonexistent") == 1.0f);
    assert(!rules.SetMultiplier("nonexistent", 2.0f));
    assert(rules.GetDescriptor("nonexistent") == nullptr);

    std::cout << "[PASS] test_server_rules_missing_rule" << std::endl;
}

void test_server_rules_hot_reload_flag() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    RuleDescriptor desc;
    desc.name = "tick_rate_scale";
    desc.baseValue = 1.0f;
    desc.hotReloadable = true;
    desc.affectsReplay = false;
    rules.RegisterRule(desc);

    const RuleDescriptor* d = rules.GetDescriptor("tick_rate_scale");
    assert(d != nullptr);
    assert(d->hotReloadable == true);
    assert(d->affectsReplay == false);

    std::cout << "[PASS] test_server_rules_hot_reload_flag" << std::endl;
}

void test_server_rules_clear() {
    ServerRules& rules = ServerRules::Get();
    rules.Clear();

    RuleDescriptor desc;
    desc.name = "test_rule";
    desc.baseValue = 1.0f;
    rules.RegisterRule(desc);

    assert(rules.RuleCount() == 1);
    rules.Clear();
    assert(rules.RuleCount() == 0);
    assert(!rules.HasRule("test_rule"));

    std::cout << "[PASS] test_server_rules_clear" << std::endl;
}
