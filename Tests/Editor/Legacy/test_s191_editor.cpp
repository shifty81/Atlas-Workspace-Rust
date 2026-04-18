// S191 editor tests: WeaponEditorV1, InventoryItemEditorV1, AbilityEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WeaponEditorV1.h"
#include "NF/Editor/InventoryItemEditorV1.h"
#include "NF/Editor/AbilityEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── WeaponEditorV1 ───────────────────────────────────────────────────────────

TEST_CASE("Weev1Weapon validity", "[Editor][S191]") {
    Weev1Weapon w;
    REQUIRE(!w.isValid());
    w.id = 1; w.name = "Sword";
    REQUIRE(w.isValid());
}

TEST_CASE("WeaponEditorV1 addWeapon and weaponCount", "[Editor][S191]") {
    WeaponEditorV1 we;
    REQUIRE(we.weaponCount() == 0);
    Weev1Weapon w; w.id = 1; w.name = "W1";
    REQUIRE(we.addWeapon(w));
    REQUIRE(we.weaponCount() == 1);
}

TEST_CASE("WeaponEditorV1 addWeapon invalid fails", "[Editor][S191]") {
    WeaponEditorV1 we;
    REQUIRE(!we.addWeapon(Weev1Weapon{}));
}

TEST_CASE("WeaponEditorV1 addWeapon duplicate fails", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w; w.id = 1; w.name = "A";
    we.addWeapon(w);
    REQUIRE(!we.addWeapon(w));
}

TEST_CASE("WeaponEditorV1 removeWeapon", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w; w.id = 2; w.name = "B";
    we.addWeapon(w);
    REQUIRE(we.removeWeapon(2));
    REQUIRE(we.weaponCount() == 0);
    REQUIRE(!we.removeWeapon(2));
}

TEST_CASE("WeaponEditorV1 findWeapon", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w; w.id = 3; w.name = "C";
    we.addWeapon(w);
    REQUIRE(we.findWeapon(3) != nullptr);
    REQUIRE(we.findWeapon(99) == nullptr);
}

TEST_CASE("WeaponEditorV1 addAttachment and attachmentCount", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Attachment a; a.id = 1; a.weaponId = 10; a.name = "Scope"; a.slot = "top";
    REQUIRE(we.addAttachment(a));
    REQUIRE(we.attachmentCount() == 1);
}

TEST_CASE("WeaponEditorV1 removeAttachment", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Attachment a; a.id = 1; a.weaponId = 10; a.name = "Grip"; a.slot = "bottom";
    we.addAttachment(a);
    REQUIRE(we.removeAttachment(1));
    REQUIRE(we.attachmentCount() == 0);
}

TEST_CASE("WeaponEditorV1 readyCount", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w1; w1.id = 1; w1.name = "A"; w1.state = Weev1WeaponState::Ready;
    Weev1Weapon w2; w2.id = 2; w2.name = "B";
    we.addWeapon(w1); we.addWeapon(w2);
    REQUIRE(we.readyCount() == 1);
}

TEST_CASE("WeaponEditorV1 lockedCount", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w1; w1.id = 1; w1.name = "A"; w1.state = Weev1WeaponState::Locked;
    Weev1Weapon w2; w2.id = 2; w2.name = "B"; w2.state = Weev1WeaponState::Ready;
    we.addWeapon(w1); we.addWeapon(w2);
    REQUIRE(we.lockedCount() == 1);
}

TEST_CASE("WeaponEditorV1 countByType", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w1; w1.id = 1; w1.name = "A"; w1.weaponType = Weev1WeaponType::Ranged;
    Weev1Weapon w2; w2.id = 2; w2.name = "B"; w2.weaponType = Weev1WeaponType::Melee;
    Weev1Weapon w3; w3.id = 3; w3.name = "C"; w3.weaponType = Weev1WeaponType::Ranged;
    we.addWeapon(w1); we.addWeapon(w2); we.addWeapon(w3);
    REQUIRE(we.countByType(Weev1WeaponType::Ranged) == 2);
    REQUIRE(we.countByType(Weev1WeaponType::Melee) == 1);
}

TEST_CASE("WeaponEditorV1 attachmentsForWeapon", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Attachment a1; a1.id = 1; a1.weaponId = 10; a1.name = "A"; a1.slot = "s";
    Weev1Attachment a2; a2.id = 2; a2.weaponId = 10; a2.name = "B"; a2.slot = "t";
    Weev1Attachment a3; a3.id = 3; a3.weaponId = 20; a3.name = "C"; a3.slot = "u";
    we.addAttachment(a1); we.addAttachment(a2); we.addAttachment(a3);
    REQUIRE(we.attachmentsForWeapon(10) == 2);
    REQUIRE(we.attachmentsForWeapon(20) == 1);
}

TEST_CASE("WeaponEditorV1 totalBaseDamage", "[Editor][S191]") {
    WeaponEditorV1 we;
    Weev1Weapon w1; w1.id = 1; w1.name = "A"; w1.baseDamage = 10.0f;
    Weev1Weapon w2; w2.id = 2; w2.name = "B"; w2.baseDamage = 25.5f;
    we.addWeapon(w1); we.addWeapon(w2);
    REQUIRE(we.totalBaseDamage() == Approx(35.5f));
}

TEST_CASE("WeaponEditorV1 onChange callback", "[Editor][S191]") {
    WeaponEditorV1 we;
    uint64_t notified = 0;
    we.setOnChange([&](uint64_t id) { notified = id; });
    Weev1Weapon w; w.id = 6; w.name = "F";
    we.addWeapon(w);
    REQUIRE(notified == 6);
}

TEST_CASE("Weev1Weapon state helpers", "[Editor][S191]") {
    Weev1Weapon w; w.id = 1; w.name = "X";
    w.state = Weev1WeaponState::Ready;      REQUIRE(w.isReady());
    w.state = Weev1WeaponState::Deprecated; REQUIRE(w.isDeprecated());
    w.state = Weev1WeaponState::Locked;     REQUIRE(w.isLocked());
}

TEST_CASE("weev1WeaponTypeName all values", "[Editor][S191]") {
    REQUIRE(std::string(weev1WeaponTypeName(Weev1WeaponType::Melee))  == "Melee");
    REQUIRE(std::string(weev1WeaponTypeName(Weev1WeaponType::Ranged)) == "Ranged");
    REQUIRE(std::string(weev1WeaponTypeName(Weev1WeaponType::Thrown)) == "Thrown");
    REQUIRE(std::string(weev1WeaponTypeName(Weev1WeaponType::Magic))  == "Magic");
    REQUIRE(std::string(weev1WeaponTypeName(Weev1WeaponType::Hybrid)) == "Hybrid");
}

// ── InventoryItemEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Iiev1InventoryItem validity", "[Editor][S191]") {
    Iiev1InventoryItem item;
    REQUIRE(!item.isValid());
    item.id = 1; item.name = "HealthPotion";
    REQUIRE(item.isValid());
}

TEST_CASE("InventoryItemEditorV1 addItem and itemCount", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    REQUIRE(iie.itemCount() == 0);
    Iiev1InventoryItem item; item.id = 1; item.name = "I1";
    REQUIRE(iie.addItem(item));
    REQUIRE(iie.itemCount() == 1);
}

TEST_CASE("InventoryItemEditorV1 addItem invalid fails", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    REQUIRE(!iie.addItem(Iiev1InventoryItem{}));
}

TEST_CASE("InventoryItemEditorV1 addItem duplicate fails", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1InventoryItem item; item.id = 1; item.name = "A";
    iie.addItem(item);
    REQUIRE(!iie.addItem(item));
}

TEST_CASE("InventoryItemEditorV1 removeItem", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1InventoryItem item; item.id = 2; item.name = "B";
    iie.addItem(item);
    REQUIRE(iie.removeItem(2));
    REQUIRE(iie.itemCount() == 0);
    REQUIRE(!iie.removeItem(2));
}

TEST_CASE("InventoryItemEditorV1 findItem", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1InventoryItem item; item.id = 3; item.name = "C";
    iie.addItem(item);
    REQUIRE(iie.findItem(3) != nullptr);
    REQUIRE(iie.findItem(99) == nullptr);
}

TEST_CASE("InventoryItemEditorV1 addProperty and propertyCount", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1ItemProperty p; p.id = 1; p.itemId = 10; p.key = "weight"; p.value = "1.5";
    REQUIRE(iie.addProperty(p));
    REQUIRE(iie.propertyCount() == 1);
}

TEST_CASE("InventoryItemEditorV1 removeProperty", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1ItemProperty p; p.id = 1; p.itemId = 10; p.key = "durability"; p.value = "100";
    iie.addProperty(p);
    REQUIRE(iie.removeProperty(1));
    REQUIRE(iie.propertyCount() == 0);
}

TEST_CASE("InventoryItemEditorV1 activeCount", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1InventoryItem i1; i1.id = 1; i1.name = "A"; i1.state = Iiev1ItemState::Active;
    Iiev1InventoryItem i2; i2.id = 2; i2.name = "B";
    iie.addItem(i1); iie.addItem(i2);
    REQUIRE(iie.activeCount() == 1);
}

TEST_CASE("InventoryItemEditorV1 lockedCount", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1InventoryItem i1; i1.id = 1; i1.name = "A"; i1.state = Iiev1ItemState::Locked;
    Iiev1InventoryItem i2; i2.id = 2; i2.name = "B"; i2.state = Iiev1ItemState::Active;
    iie.addItem(i1); iie.addItem(i2);
    REQUIRE(iie.lockedCount() == 1);
}

TEST_CASE("InventoryItemEditorV1 countByRarity", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1InventoryItem i1; i1.id = 1; i1.name = "A"; i1.rarity = Iiev1ItemRarity::Epic;
    Iiev1InventoryItem i2; i2.id = 2; i2.name = "B"; i2.rarity = Iiev1ItemRarity::Common;
    Iiev1InventoryItem i3; i3.id = 3; i3.name = "C"; i3.rarity = Iiev1ItemRarity::Epic;
    iie.addItem(i1); iie.addItem(i2); iie.addItem(i3);
    REQUIRE(iie.countByRarity(Iiev1ItemRarity::Epic) == 2);
    REQUIRE(iie.countByRarity(Iiev1ItemRarity::Common) == 1);
}

TEST_CASE("InventoryItemEditorV1 propertiesForItem", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    Iiev1ItemProperty p1; p1.id = 1; p1.itemId = 10; p1.key = "a"; p1.value = "1";
    Iiev1ItemProperty p2; p2.id = 2; p2.itemId = 10; p2.key = "b"; p2.value = "2";
    Iiev1ItemProperty p3; p3.id = 3; p3.itemId = 20; p3.key = "c"; p3.value = "3";
    iie.addProperty(p1); iie.addProperty(p2); iie.addProperty(p3);
    REQUIRE(iie.propertiesForItem(10) == 2);
    REQUIRE(iie.propertiesForItem(20) == 1);
}

TEST_CASE("InventoryItemEditorV1 onChange callback", "[Editor][S191]") {
    InventoryItemEditorV1 iie;
    uint64_t notified = 0;
    iie.setOnChange([&](uint64_t id) { notified = id; });
    Iiev1InventoryItem item; item.id = 8; item.name = "H";
    iie.addItem(item);
    REQUIRE(notified == 8);
}

TEST_CASE("Iiev1InventoryItem state helpers", "[Editor][S191]") {
    Iiev1InventoryItem item; item.id = 1; item.name = "X";
    item.state = Iiev1ItemState::Active;     REQUIRE(item.isActive());
    item.state = Iiev1ItemState::Deprecated; REQUIRE(item.isDeprecated());
    item.state = Iiev1ItemState::Locked;     REQUIRE(item.isLocked());
}

TEST_CASE("iiev1ItemRarityName all values", "[Editor][S191]") {
    REQUIRE(std::string(iiev1ItemRarityName(Iiev1ItemRarity::Common))    == "Common");
    REQUIRE(std::string(iiev1ItemRarityName(Iiev1ItemRarity::Uncommon))  == "Uncommon");
    REQUIRE(std::string(iiev1ItemRarityName(Iiev1ItemRarity::Rare))      == "Rare");
    REQUIRE(std::string(iiev1ItemRarityName(Iiev1ItemRarity::Epic))      == "Epic");
    REQUIRE(std::string(iiev1ItemRarityName(Iiev1ItemRarity::Legendary)) == "Legendary");
}

// ── AbilityEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Aeev1Ability validity", "[Editor][S191]") {
    Aeev1Ability a;
    REQUIRE(!a.isValid());
    a.id = 1; a.name = "Fireball";
    REQUIRE(a.isValid());
}

TEST_CASE("AbilityEditorV1 addAbility and abilityCount", "[Editor][S191]") {
    AbilityEditorV1 ae;
    REQUIRE(ae.abilityCount() == 0);
    Aeev1Ability a; a.id = 1; a.name = "A1";
    REQUIRE(ae.addAbility(a));
    REQUIRE(ae.abilityCount() == 1);
}

TEST_CASE("AbilityEditorV1 addAbility invalid fails", "[Editor][S191]") {
    AbilityEditorV1 ae;
    REQUIRE(!ae.addAbility(Aeev1Ability{}));
}

TEST_CASE("AbilityEditorV1 addAbility duplicate fails", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1Ability a; a.id = 1; a.name = "A";
    ae.addAbility(a);
    REQUIRE(!ae.addAbility(a));
}

TEST_CASE("AbilityEditorV1 removeAbility", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1Ability a; a.id = 2; a.name = "B";
    ae.addAbility(a);
    REQUIRE(ae.removeAbility(2));
    REQUIRE(ae.abilityCount() == 0);
    REQUIRE(!ae.removeAbility(2));
}

TEST_CASE("AbilityEditorV1 findAbility", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1Ability a; a.id = 3; a.name = "C";
    ae.addAbility(a);
    REQUIRE(ae.findAbility(3) != nullptr);
    REQUIRE(ae.findAbility(99) == nullptr);
}

TEST_CASE("AbilityEditorV1 addEffect and effectCount", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1AbilityEffect e; e.id = 1; e.abilityId = 10; e.name = "BurnDamage";
    REQUIRE(ae.addEffect(e));
    REQUIRE(ae.effectCount() == 1);
}

TEST_CASE("AbilityEditorV1 removeEffect", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1AbilityEffect e; e.id = 1; e.abilityId = 10; e.name = "Slow";
    ae.addEffect(e);
    REQUIRE(ae.removeEffect(1));
    REQUIRE(ae.effectCount() == 0);
}

TEST_CASE("AbilityEditorV1 enabledCount", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1Ability a1; a1.id = 1; a1.name = "A"; a1.state = Aeev1AbilityState::Enabled;
    Aeev1Ability a2; a2.id = 2; a2.name = "B";
    ae.addAbility(a1); ae.addAbility(a2);
    REQUIRE(ae.enabledCount() == 1);
}

TEST_CASE("AbilityEditorV1 lockedCount", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1Ability a1; a1.id = 1; a1.name = "A"; a1.state = Aeev1AbilityState::Locked;
    Aeev1Ability a2; a2.id = 2; a2.name = "B"; a2.state = Aeev1AbilityState::Enabled;
    ae.addAbility(a1); ae.addAbility(a2);
    REQUIRE(ae.lockedCount() == 1);
}

TEST_CASE("AbilityEditorV1 countByType", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1Ability a1; a1.id = 1; a1.name = "A"; a1.abilityType = Aeev1AbilityType::Active;
    Aeev1Ability a2; a2.id = 2; a2.name = "B"; a2.abilityType = Aeev1AbilityType::Passive;
    Aeev1Ability a3; a3.id = 3; a3.name = "C"; a3.abilityType = Aeev1AbilityType::Active;
    ae.addAbility(a1); ae.addAbility(a2); ae.addAbility(a3);
    REQUIRE(ae.countByType(Aeev1AbilityType::Active) == 2);
    REQUIRE(ae.countByType(Aeev1AbilityType::Passive) == 1);
}

TEST_CASE("AbilityEditorV1 effectsForAbility", "[Editor][S191]") {
    AbilityEditorV1 ae;
    Aeev1AbilityEffect e1; e1.id = 1; e1.abilityId = 10; e1.name = "A";
    Aeev1AbilityEffect e2; e2.id = 2; e2.abilityId = 10; e2.name = "B";
    Aeev1AbilityEffect e3; e3.id = 3; e3.abilityId = 20; e3.name = "C";
    ae.addEffect(e1); ae.addEffect(e2); ae.addEffect(e3);
    REQUIRE(ae.effectsForAbility(10) == 2);
    REQUIRE(ae.effectsForAbility(20) == 1);
}

TEST_CASE("AbilityEditorV1 onChange callback", "[Editor][S191]") {
    AbilityEditorV1 ae;
    uint64_t notified = 0;
    ae.setOnChange([&](uint64_t id) { notified = id; });
    Aeev1Ability a; a.id = 7; a.name = "G";
    ae.addAbility(a);
    REQUIRE(notified == 7);
}

TEST_CASE("Aeev1Ability state helpers", "[Editor][S191]") {
    Aeev1Ability a; a.id = 1; a.name = "X";
    a.state = Aeev1AbilityState::Enabled;  REQUIRE(a.isEnabled());
    a.state = Aeev1AbilityState::Disabled; REQUIRE(a.isDisabled());
    a.state = Aeev1AbilityState::Locked;   REQUIRE(a.isLocked());
}

TEST_CASE("aeev1AbilityTypeName all values", "[Editor][S191]") {
    REQUIRE(std::string(aeev1AbilityTypeName(Aeev1AbilityType::Active))     == "Active");
    REQUIRE(std::string(aeev1AbilityTypeName(Aeev1AbilityType::Passive))    == "Passive");
    REQUIRE(std::string(aeev1AbilityTypeName(Aeev1AbilityType::Toggle))     == "Toggle");
    REQUIRE(std::string(aeev1AbilityTypeName(Aeev1AbilityType::Channelled)) == "Channelled");
    REQUIRE(std::string(aeev1AbilityTypeName(Aeev1AbilityType::Reactive))   == "Reactive");
}
