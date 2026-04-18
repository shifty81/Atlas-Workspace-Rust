#include "../engine/gameplay/SkillTree.h"
#include <iostream>
#include <cassert>

using namespace atlas::gameplay;

void test_skill_add_node() {
    SkillTree tree;
    tree.Init();

    SkillID id = tree.AddNode("Fireball", {}, {{"Magic", ModifierType::Add, 10.0f}});
    assert(id > 0);
    assert(tree.NodeCount() == 1);

    const SkillNode* node = tree.GetNode(id);
    assert(node != nullptr);
    assert(node->name == "Fireball");
    assert(node->unlocked == false);

    std::cout << "[PASS] test_skill_add_node" << std::endl;
}

void test_skill_unlock() {
    SkillTree tree;
    tree.Init();

    SkillID id = tree.AddNode("Shield Bash");
    assert(!tree.IsUnlocked(id));

    bool ok = tree.Unlock(id);
    assert(ok);
    assert(tree.IsUnlocked(id));

    // Cannot unlock again
    assert(!tree.Unlock(id));

    std::cout << "[PASS] test_skill_unlock" << std::endl;
}

void test_skill_prerequisites() {
    SkillTree tree;
    tree.Init();

    SkillID base = tree.AddNode("Basic Attack");
    SkillID advanced = tree.AddNode("Power Strike", {base});

    // Cannot unlock advanced without base
    assert(!tree.CanUnlock(advanced));
    assert(!tree.Unlock(advanced));

    // Unlock base first
    assert(tree.Unlock(base));

    // Now advanced can be unlocked
    assert(tree.CanUnlock(advanced));
    assert(tree.Unlock(advanced));
    assert(tree.IsUnlocked(advanced));

    std::cout << "[PASS] test_skill_prerequisites" << std::endl;
}

void test_skill_effects() {
    SkillTree tree;
    tree.Init();

    std::vector<SkillEffect> effects = {
        {"Damage", ModifierType::Add, 5.0f},
        {"Speed", ModifierType::Multiply, 1.2f},
        {"Health", ModifierType::Clamp, 100.0f}
    };

    SkillID id = tree.AddNode("Berserker Rage", {}, effects);

    auto retrieved = tree.GetEffects(id);
    assert(retrieved.size() == 3);
    assert(retrieved[0].systemName == "Damage");
    assert(retrieved[0].modifierType == ModifierType::Add);
    assert(retrieved[0].value == 5.0f);
    assert(retrieved[1].modifierType == ModifierType::Multiply);
    assert(retrieved[2].modifierType == ModifierType::Clamp);

    // Non-existent node returns empty
    assert(tree.GetEffects(999).empty());

    std::cout << "[PASS] test_skill_effects" << std::endl;
}

void test_skill_tree_chain() {
    SkillTree tree;
    tree.Init();

    SkillID tier1 = tree.AddNode("Apprentice");
    SkillID tier2 = tree.AddNode("Journeyman", {tier1});
    SkillID tier3 = tree.AddNode("Master", {tier2});

    // Cannot skip tiers
    assert(!tree.CanUnlock(tier3));
    assert(!tree.CanUnlock(tier2));
    assert(tree.CanUnlock(tier1));

    // Unlock in order
    assert(tree.Unlock(tier1));
    assert(!tree.CanUnlock(tier3));
    assert(tree.CanUnlock(tier2));

    assert(tree.Unlock(tier2));
    assert(tree.CanUnlock(tier3));

    assert(tree.Unlock(tier3));
    assert(tree.IsUnlocked(tier1));
    assert(tree.IsUnlocked(tier2));
    assert(tree.IsUnlocked(tier3));

    assert(tree.NodeCount() == 3);

    std::cout << "[PASS] test_skill_tree_chain" << std::endl;
}
