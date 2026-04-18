// S109 editor tests: AbilitySystemEditor, InventoryEditor, ProgressionEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ProgressionEditor.h"
#include "NF/Editor/InventoryEditor.h"
#include "NF/Editor/AbilitySystemEditor.h"

using namespace NF;

// ── AbilitySystemEditor ──────────────────────────────────────────────────────

TEST_CASE("AbilityTargetType names", "[Editor][S109]") {
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::Self))        == "Self");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::Single))      == "Single");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::MultiTarget)) == "MultiTarget");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::AoE))         == "AoE");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::Line))        == "Line");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::Cone))        == "Cone");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::Chain))       == "Chain");
    REQUIRE(std::string(abilityTargetTypeName(AbilityTargetType::Global))      == "Global");
}

TEST_CASE("AbilityResourceType names", "[Editor][S109]") {
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::Mana))    == "Mana");
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::Stamina)) == "Stamina");
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::Energy))  == "Energy");
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::Rage))    == "Rage");
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::Focus))   == "Focus");
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::Heat))    == "Heat");
    REQUIRE(std::string(abilityResourceTypeName(AbilityResourceType::None))    == "None");
}

TEST_CASE("AbilityCategory names", "[Editor][S109]") {
    REQUIRE(std::string(abilityCategoryName(AbilityCategory::Active))    == "Active");
    REQUIRE(std::string(abilityCategoryName(AbilityCategory::Passive))   == "Passive");
    REQUIRE(std::string(abilityCategoryName(AbilityCategory::Toggle))    == "Toggle");
    REQUIRE(std::string(abilityCategoryName(AbilityCategory::Channeled)) == "Channeled");
    REQUIRE(std::string(abilityCategoryName(AbilityCategory::Reaction))  == "Reaction");
}

TEST_CASE("AbilityEntry defaults", "[Editor][S109]") {
    AbilityEntry entry(1, "fireball", AbilityCategory::Active);
    REQUIRE(entry.id()           == 1u);
    REQUIRE(entry.name()         == "fireball");
    REQUIRE(entry.category()     == AbilityCategory::Active);
    REQUIRE(entry.targetType()   == AbilityTargetType::Single);
    REQUIRE(entry.resourceType() == AbilityResourceType::Mana);
    REQUIRE(entry.cooldown()     == 0.0f);
    REQUIRE(entry.resourceCost() == 0.0f);
}

TEST_CASE("AbilityEntry mutation", "[Editor][S109]") {
    AbilityEntry entry(2, "shield", AbilityCategory::Toggle);
    entry.setTargetType(AbilityTargetType::Self);
    entry.setResourceType(AbilityResourceType::Energy);
    entry.setCooldown(5.0f);
    entry.setResourceCost(20.0f);
    REQUIRE(entry.targetType()   == AbilityTargetType::Self);
    REQUIRE(entry.resourceType() == AbilityResourceType::Energy);
    REQUIRE(entry.cooldown()     == 5.0f);
    REQUIRE(entry.resourceCost() == 20.0f);
}

TEST_CASE("AbilitySystemEditor defaults", "[Editor][S109]") {
    AbilitySystemEditor ed;
    REQUIRE(ed.maxAbilities()            == 32u);
    REQUIRE(!ed.isGlobalCooldownEnabled());
    REQUIRE(!ed.isComboSystemEnabled());
    REQUIRE(ed.abilityCount()            == 0u);
}

TEST_CASE("AbilitySystemEditor add/remove abilities", "[Editor][S109]") {
    AbilitySystemEditor ed;
    REQUIRE(ed.addAbility(AbilityEntry(1, "fireball",   AbilityCategory::Active)));
    REQUIRE(ed.addAbility(AbilityEntry(2, "stealth",    AbilityCategory::Toggle)));
    REQUIRE(ed.addAbility(AbilityEntry(3, "regen",      AbilityCategory::Passive)));
    REQUIRE(!ed.addAbility(AbilityEntry(1, "fireball",  AbilityCategory::Active)));
    REQUIRE(ed.abilityCount() == 3u);
    REQUIRE(ed.removeAbility(2));
    REQUIRE(ed.abilityCount() == 2u);
    REQUIRE(!ed.removeAbility(99));
}

TEST_CASE("AbilitySystemEditor counts and find", "[Editor][S109]") {
    AbilitySystemEditor ed;
    ed.addAbility(AbilityEntry(1, "fireball",  AbilityCategory::Active));
    ed.addAbility(AbilityEntry(2, "blizzard",  AbilityCategory::Active));
    AbilityEntry e3(3, "stealth", AbilityCategory::Toggle);
    e3.setTargetType(AbilityTargetType::AoE);
    ed.addAbility(e3);
    ed.addAbility(AbilityEntry(4, "regen",     AbilityCategory::Passive));
    REQUIRE(ed.countByCategory(AbilityCategory::Active)           == 2u);
    REQUIRE(ed.countByCategory(AbilityCategory::Toggle)           == 1u);
    REQUIRE(ed.countByCategory(AbilityCategory::Reaction)         == 0u);
    REQUIRE(ed.countByTargetType(AbilityTargetType::Single)       == 3u);
    REQUIRE(ed.countByTargetType(AbilityTargetType::AoE)          == 1u);
    auto* found = ed.findAbility(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == AbilityCategory::Toggle);
    REQUIRE(ed.findAbility(99) == nullptr);
}

TEST_CASE("AbilitySystemEditor mutation", "[Editor][S109]") {
    AbilitySystemEditor ed;
    ed.setMaxAbilities(64);
    ed.setGlobalCooldownEnabled(true);
    ed.setComboSystemEnabled(true);
    REQUIRE(ed.maxAbilities()            == 64u);
    REQUIRE(ed.isGlobalCooldownEnabled());
    REQUIRE(ed.isComboSystemEnabled());
}

// ── InventoryEditor ──────────────────────────────────────────────────────────

TEST_CASE("ItemRarity names", "[Editor][S109]") {
    REQUIRE(std::string(itemRarityName(ItemRarity::Common))    == "Common");
    REQUIRE(std::string(itemRarityName(ItemRarity::Uncommon))  == "Uncommon");
    REQUIRE(std::string(itemRarityName(ItemRarity::Rare))      == "Rare");
    REQUIRE(std::string(itemRarityName(ItemRarity::Epic))      == "Epic");
    REQUIRE(std::string(itemRarityName(ItemRarity::Legendary)) == "Legendary");
    REQUIRE(std::string(itemRarityName(ItemRarity::Unique))    == "Unique");
}

TEST_CASE("ItemCategory names", "[Editor][S109]") {
    REQUIRE(std::string(itemCategoryName(ItemCategory::Weapon))     == "Weapon");
    REQUIRE(std::string(itemCategoryName(ItemCategory::Armor))      == "Armor");
    REQUIRE(std::string(itemCategoryName(ItemCategory::Consumable)) == "Consumable");
    REQUIRE(std::string(itemCategoryName(ItemCategory::Material))   == "Material");
    REQUIRE(std::string(itemCategoryName(ItemCategory::QuestItem))  == "QuestItem");
    REQUIRE(std::string(itemCategoryName(ItemCategory::Misc))       == "Misc");
    REQUIRE(std::string(itemCategoryName(ItemCategory::Accessory))  == "Accessory");
}

TEST_CASE("InventorySlotType names", "[Editor][S109]") {
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::MainHand)) == "MainHand");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::OffHand))  == "OffHand");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::Head))     == "Head");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::Chest))    == "Chest");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::Legs))     == "Legs");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::Feet))     == "Feet");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::Ring))     == "Ring");
    REQUIRE(std::string(inventorySlotTypeName(InventorySlotType::General))  == "General");
}

TEST_CASE("InventoryItem defaults", "[Editor][S109]") {
    InventoryItem item(1, "sword", ItemRarity::Common, ItemCategory::Weapon);
    REQUIRE(item.id()           == 1u);
    REQUIRE(item.name()         == "sword");
    REQUIRE(item.rarity()       == ItemRarity::Common);
    REQUIRE(item.category()     == ItemCategory::Weapon);
    REQUIRE(item.stackSize()    == 1u);
    REQUIRE(item.maxStack()     == 1u);
    REQUIRE(item.weight()       == 1.0f);
    REQUIRE(!item.isEquippable());
}

TEST_CASE("InventoryItem mutation", "[Editor][S109]") {
    InventoryItem item(2, "health_potion", ItemRarity::Uncommon, ItemCategory::Consumable);
    item.setRarity(ItemRarity::Rare);
    item.setCategory(ItemCategory::Consumable);
    item.setStackSize(5);
    item.setMaxStack(20);
    item.setWeight(0.2f);
    item.setEquippable(false);
    REQUIRE(item.rarity()    == ItemRarity::Rare);
    REQUIRE(item.stackSize() == 5u);
    REQUIRE(item.maxStack()  == 20u);
    REQUIRE(item.weight()    == 0.2f);
    REQUIRE(!item.isEquippable());
}

TEST_CASE("InventoryEditor defaults", "[Editor][S109]") {
    InventoryEditor ed;
    REQUIRE(ed.gridColumns()       == 10u);
    REQUIRE(ed.gridRows()          == 8u);
    REQUIRE(ed.isShowWeight());
    REQUIRE(ed.isShowRarityColor());
    REQUIRE(ed.itemCount()         == 0u);
}

TEST_CASE("InventoryEditor add/remove items", "[Editor][S109]") {
    InventoryEditor ed;
    REQUIRE(ed.addItem(InventoryItem(1, "sword",   ItemRarity::Common,    ItemCategory::Weapon)));
    REQUIRE(ed.addItem(InventoryItem(2, "shield",  ItemRarity::Rare,      ItemCategory::Armor)));
    REQUIRE(ed.addItem(InventoryItem(3, "potion",  ItemRarity::Common,    ItemCategory::Consumable)));
    REQUIRE(!ed.addItem(InventoryItem(1, "sword",  ItemRarity::Common,    ItemCategory::Weapon)));
    REQUIRE(ed.itemCount() == 3u);
    REQUIRE(ed.removeItem(2));
    REQUIRE(ed.itemCount() == 2u);
    REQUIRE(!ed.removeItem(99));
}

TEST_CASE("InventoryEditor counts and find", "[Editor][S109]") {
    InventoryEditor ed;
    InventoryItem i1(1, "sword",   ItemRarity::Rare,   ItemCategory::Weapon);   i1.setEquippable(true);
    InventoryItem i2(2, "armor",   ItemRarity::Epic,   ItemCategory::Armor);    i2.setEquippable(true);
    InventoryItem i3(3, "potion",  ItemRarity::Common, ItemCategory::Consumable);
    InventoryItem i4(4, "gem",     ItemRarity::Rare,   ItemCategory::Material);
    ed.addItem(i1); ed.addItem(i2); ed.addItem(i3); ed.addItem(i4);
    REQUIRE(ed.countByRarity(ItemRarity::Rare)           == 2u);
    REQUIRE(ed.countByRarity(ItemRarity::Epic)           == 1u);
    REQUIRE(ed.countByRarity(ItemRarity::Legendary)      == 0u);
    REQUIRE(ed.countByCategory(ItemCategory::Weapon)     == 1u);
    REQUIRE(ed.countByCategory(ItemCategory::Consumable) == 1u);
    REQUIRE(ed.countEquippable()                         == 2u);
    auto* found = ed.findItem(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == ItemCategory::Consumable);
    REQUIRE(ed.findItem(99) == nullptr);
}

TEST_CASE("InventoryEditor mutation", "[Editor][S109]") {
    InventoryEditor ed;
    ed.setGridColumns(12);
    ed.setGridRows(6);
    ed.setShowWeight(false);
    ed.setShowRarityColor(false);
    REQUIRE(ed.gridColumns()       == 12u);
    REQUIRE(ed.gridRows()          == 6u);
    REQUIRE(!ed.isShowWeight());
    REQUIRE(!ed.isShowRarityColor());
}

// ── ProgressionEditor ────────────────────────────────────────────────────────

TEST_CASE("ProgressionCurveType names", "[Editor][S109]") {
    REQUIRE(std::string(progressionCurveTypeName(ProgressionCurveType::Linear))      == "Linear");
    REQUIRE(std::string(progressionCurveTypeName(ProgressionCurveType::Exponential)) == "Exponential");
    REQUIRE(std::string(progressionCurveTypeName(ProgressionCurveType::Logarithmic)) == "Logarithmic");
    REQUIRE(std::string(progressionCurveTypeName(ProgressionCurveType::Flat))        == "Flat");
    REQUIRE(std::string(progressionCurveTypeName(ProgressionCurveType::Custom))      == "Custom");
}

TEST_CASE("AttributeType names", "[Editor][S109]") {
    REQUIRE(std::string(attributeTypeName(AttributeType::Health))       == "Health");
    REQUIRE(std::string(attributeTypeName(AttributeType::Mana))         == "Mana");
    REQUIRE(std::string(attributeTypeName(AttributeType::Strength))     == "Strength");
    REQUIRE(std::string(attributeTypeName(AttributeType::Agility))      == "Agility");
    REQUIRE(std::string(attributeTypeName(AttributeType::Intelligence)) == "Intelligence");
    REQUIRE(std::string(attributeTypeName(AttributeType::Defense))      == "Defense");
    REQUIRE(std::string(attributeTypeName(AttributeType::Speed))        == "Speed");
    REQUIRE(std::string(attributeTypeName(AttributeType::Luck))         == "Luck");
}

TEST_CASE("LevelRewardType names", "[Editor][S109]") {
    REQUIRE(std::string(levelRewardTypeName(LevelRewardType::AttributePoint)) == "AttributePoint");
    REQUIRE(std::string(levelRewardTypeName(LevelRewardType::SkillPoint))     == "SkillPoint");
    REQUIRE(std::string(levelRewardTypeName(LevelRewardType::Ability))        == "Ability");
    REQUIRE(std::string(levelRewardTypeName(LevelRewardType::Item))           == "Item");
    REQUIRE(std::string(levelRewardTypeName(LevelRewardType::Currency))       == "Currency");
    REQUIRE(std::string(levelRewardTypeName(LevelRewardType::Feature))        == "Feature");
}

TEST_CASE("ProgressionLevel defaults", "[Editor][S109]") {
    ProgressionLevel lvl(1, 100u);
    REQUIRE(lvl.level()         == 1u);
    REQUIRE(lvl.requiredXP()    == 100u);
    REQUIRE(lvl.rewardType()    == LevelRewardType::AttributePoint);
    REQUIRE(lvl.rewardValue()   == 1u);
    REQUIRE(lvl.attributeType() == AttributeType::Health);
}

TEST_CASE("ProgressionLevel mutation", "[Editor][S109]") {
    ProgressionLevel lvl(5, 1500u);
    lvl.setRewardType(LevelRewardType::SkillPoint);
    lvl.setRewardValue(2u);
    lvl.setAttributeType(AttributeType::Strength);
    REQUIRE(lvl.rewardType()    == LevelRewardType::SkillPoint);
    REQUIRE(lvl.rewardValue()   == 2u);
    REQUIRE(lvl.attributeType() == AttributeType::Strength);
}

TEST_CASE("ProgressionEditor defaults", "[Editor][S109]") {
    ProgressionEditor ed;
    REQUIRE(ed.curveType()     == ProgressionCurveType::Exponential);
    REQUIRE(ed.baseXP()        == 100u);
    REQUIRE(ed.xpScaleFactor() == 1.5f);
    REQUIRE(ed.levelCount()    == 0u);
    REQUIRE(ed.maxLevel()      == 0u);
}

TEST_CASE("ProgressionEditor add/remove levels", "[Editor][S109]") {
    ProgressionEditor ed;
    REQUIRE(ed.addLevel(ProgressionLevel(1, 100u)));
    REQUIRE(ed.addLevel(ProgressionLevel(2, 250u)));
    REQUIRE(ed.addLevel(ProgressionLevel(3, 500u)));
    REQUIRE(!ed.addLevel(ProgressionLevel(1, 100u)));
    REQUIRE(ed.levelCount() == 3u);
    REQUIRE(ed.removeLevel(2));
    REQUIRE(ed.levelCount() == 2u);
    REQUIRE(!ed.removeLevel(99));
}

TEST_CASE("ProgressionEditor counts and find", "[Editor][S109]") {
    ProgressionEditor ed;
    ProgressionLevel l1(1, 100u);
    ProgressionLevel l2(2, 250u); l2.setRewardType(LevelRewardType::SkillPoint);
    ProgressionLevel l3(3, 500u); l3.setRewardType(LevelRewardType::Ability);
    ProgressionLevel l4(10, 5000u);
    ed.addLevel(l1); ed.addLevel(l2); ed.addLevel(l3); ed.addLevel(l4);
    REQUIRE(ed.countByRewardType(LevelRewardType::AttributePoint) == 2u);
    REQUIRE(ed.countByRewardType(LevelRewardType::SkillPoint)     == 1u);
    REQUIRE(ed.countByRewardType(LevelRewardType::Currency)       == 0u);
    REQUIRE(ed.maxLevel()                                         == 10u);
    auto* found = ed.findLevel(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->rewardType() == LevelRewardType::Ability);
    REQUIRE(ed.findLevel(99) == nullptr);
}

TEST_CASE("ProgressionEditor mutation", "[Editor][S109]") {
    ProgressionEditor ed;
    ed.setCurveType(ProgressionCurveType::Linear);
    ed.setBaseXP(200u);
    ed.setXpScaleFactor(2.0f);
    REQUIRE(ed.curveType()     == ProgressionCurveType::Linear);
    REQUIRE(ed.baseXP()        == 200u);
    REQUIRE(ed.xpScaleFactor() == 2.0f);
}
