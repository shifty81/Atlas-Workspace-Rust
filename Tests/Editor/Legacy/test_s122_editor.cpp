// S122 editor tests: GameEconomyEditor, VirtualCurrencyEditor, ItemShopEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ItemShopEditor.h"
#include "NF/Editor/VirtualCurrencyEditor.h"
#include "NF/Editor/GameEconomyEditor.h"

using namespace NF;

// ── GameEconomyEditor ─────────────────────────────────────────────────────────

TEST_CASE("EconomyZone names", "[Editor][S122]") {
    REQUIRE(std::string(economyZoneName(EconomyZone::EarlyGame)) == "EarlyGame");
    REQUIRE(std::string(economyZoneName(EconomyZone::MidGame))   == "MidGame");
    REQUIRE(std::string(economyZoneName(EconomyZone::LateGame))  == "LateGame");
    REQUIRE(std::string(economyZoneName(EconomyZone::Endgame))   == "Endgame");
    REQUIRE(std::string(economyZoneName(EconomyZone::Custom))    == "Custom");
}

TEST_CASE("BalanceParameter names", "[Editor][S122]") {
    REQUIRE(std::string(balanceParameterName(BalanceParameter::ResourceGain))    == "ResourceGain");
    REQUIRE(std::string(balanceParameterName(BalanceParameter::ResourceCost))    == "ResourceCost");
    REQUIRE(std::string(balanceParameterName(BalanceParameter::DropRate))        == "DropRate");
    REQUIRE(std::string(balanceParameterName(BalanceParameter::SpawnRate))       == "SpawnRate");
    REQUIRE(std::string(balanceParameterName(BalanceParameter::DifficultyScale)) == "DifficultyScale");
}

TEST_CASE("EconomyRule defaults", "[Editor][S122]") {
    EconomyRule r(1, "early_gold_gain", EconomyZone::EarlyGame, BalanceParameter::ResourceGain);
    REQUIRE(r.id()         == 1u);
    REQUIRE(r.name()       == "early_gold_gain");
    REQUIRE(r.zone()       == EconomyZone::EarlyGame);
    REQUIRE(r.parameter()  == BalanceParameter::ResourceGain);
    REQUIRE(r.multiplier() == 1.0f);
    REQUIRE(r.isActive());
}

TEST_CASE("EconomyRule mutation", "[Editor][S122]") {
    EconomyRule r(2, "late_drop_rate", EconomyZone::LateGame, BalanceParameter::DropRate);
    r.setMultiplier(2.5f);
    r.setIsActive(false);
    REQUIRE(r.multiplier() == 2.5f);
    REQUIRE(!r.isActive());
}

TEST_CASE("GameEconomyEditor defaults", "[Editor][S122]") {
    GameEconomyEditor ed;
    REQUIRE(!ed.isShowInactive());
    REQUIRE(!ed.isGroupByZone());
    REQUIRE(ed.globalMultiplier() == 1.0f);
    REQUIRE(ed.ruleCount()        == 0u);
}

TEST_CASE("GameEconomyEditor add/remove rules", "[Editor][S122]") {
    GameEconomyEditor ed;
    REQUIRE(ed.addRule(EconomyRule(1, "r1", EconomyZone::EarlyGame, BalanceParameter::ResourceGain)));
    REQUIRE(ed.addRule(EconomyRule(2, "r2", EconomyZone::MidGame,   BalanceParameter::DropRate)));
    REQUIRE(ed.addRule(EconomyRule(3, "r3", EconomyZone::Endgame,   BalanceParameter::SpawnRate)));
    REQUIRE(!ed.addRule(EconomyRule(1, "r1", EconomyZone::EarlyGame, BalanceParameter::ResourceGain)));
    REQUIRE(ed.ruleCount() == 3u);
    REQUIRE(ed.removeRule(2));
    REQUIRE(ed.ruleCount() == 2u);
    REQUIRE(!ed.removeRule(99));
}

TEST_CASE("GameEconomyEditor counts and find", "[Editor][S122]") {
    GameEconomyEditor ed;
    EconomyRule r1(1, "r1", EconomyZone::EarlyGame, BalanceParameter::ResourceGain);
    EconomyRule r2(2, "r2", EconomyZone::EarlyGame, BalanceParameter::ResourceCost); r2.setIsActive(false);
    EconomyRule r3(3, "r3", EconomyZone::MidGame,   BalanceParameter::DropRate);
    EconomyRule r4(4, "r4", EconomyZone::LateGame,  BalanceParameter::DropRate);     r4.setIsActive(false);
    ed.addRule(r1); ed.addRule(r2); ed.addRule(r3); ed.addRule(r4);
    REQUIRE(ed.countByZone(EconomyZone::EarlyGame)              == 2u);
    REQUIRE(ed.countByZone(EconomyZone::MidGame)                == 1u);
    REQUIRE(ed.countByZone(EconomyZone::Custom)                 == 0u);
    REQUIRE(ed.countByParameter(BalanceParameter::DropRate)     == 2u);
    REQUIRE(ed.countByParameter(BalanceParameter::ResourceGain) == 1u);
    REQUIRE(ed.countByParameter(BalanceParameter::SpawnRate)    == 0u);
    REQUIRE(ed.countActive()                                    == 2u);
    auto* found = ed.findRule(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->zone() == EconomyZone::MidGame);
    REQUIRE(ed.findRule(99) == nullptr);
}

TEST_CASE("GameEconomyEditor settings mutation", "[Editor][S122]") {
    GameEconomyEditor ed;
    ed.setIsShowInactive(true);
    ed.setIsGroupByZone(true);
    ed.setGlobalMultiplier(0.5f);
    REQUIRE(ed.isShowInactive());
    REQUIRE(ed.isGroupByZone());
    REQUIRE(ed.globalMultiplier() == 0.5f);
}

// ── VirtualCurrencyEditor ─────────────────────────────────────────────────────

TEST_CASE("CurrencyType names", "[Editor][S122]") {
    REQUIRE(std::string(currencyTypeName(CurrencyType::Hard))     == "Hard");
    REQUIRE(std::string(currencyTypeName(CurrencyType::Soft))     == "Soft");
    REQUIRE(std::string(currencyTypeName(CurrencyType::Premium))  == "Premium");
    REQUIRE(std::string(currencyTypeName(CurrencyType::Seasonal)) == "Seasonal");
    REQUIRE(std::string(currencyTypeName(CurrencyType::Event))    == "Event");
}

TEST_CASE("CurrencyAcquisition names", "[Editor][S122]") {
    REQUIRE(std::string(currencyAcquisitionName(CurrencyAcquisition::Purchase))    == "Purchase");
    REQUIRE(std::string(currencyAcquisitionName(CurrencyAcquisition::Reward))      == "Reward");
    REQUIRE(std::string(currencyAcquisitionName(CurrencyAcquisition::Achievement)) == "Achievement");
    REQUIRE(std::string(currencyAcquisitionName(CurrencyAcquisition::Quest))       == "Quest");
    REQUIRE(std::string(currencyAcquisitionName(CurrencyAcquisition::Trade))       == "Trade");
}

TEST_CASE("CurrencyEntry defaults", "[Editor][S122]") {
    CurrencyEntry c(1, "GLD", CurrencyType::Soft, CurrencyAcquisition::Reward);
    REQUIRE(c.id()           == 1u);
    REQUIRE(c.code()         == "GLD");
    REQUIRE(c.type()         == CurrencyType::Soft);
    REQUIRE(c.acquisition()  == CurrencyAcquisition::Reward);
    REQUIRE(c.exchangeRate() == 1.0f);
    REQUIRE(c.isEnabled());
}

TEST_CASE("CurrencyEntry mutation", "[Editor][S122]") {
    CurrencyEntry c(2, "GEM", CurrencyType::Hard, CurrencyAcquisition::Purchase);
    c.setExchangeRate(0.01f);
    c.setIsEnabled(false);
    REQUIRE(c.exchangeRate() == 0.01f);
    REQUIRE(!c.isEnabled());
}

TEST_CASE("VirtualCurrencyEditor defaults", "[Editor][S122]") {
    VirtualCurrencyEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.baseExchangeRate() == 1.0f);
    REQUIRE(ed.currencyCount()    == 0u);
}

TEST_CASE("VirtualCurrencyEditor add/remove currencies", "[Editor][S122]") {
    VirtualCurrencyEditor ed;
    REQUIRE(ed.addCurrency(CurrencyEntry(1, "GLD", CurrencyType::Soft,    CurrencyAcquisition::Reward)));
    REQUIRE(ed.addCurrency(CurrencyEntry(2, "GEM", CurrencyType::Hard,    CurrencyAcquisition::Purchase)));
    REQUIRE(ed.addCurrency(CurrencyEntry(3, "EVT", CurrencyType::Event,   CurrencyAcquisition::Quest)));
    REQUIRE(!ed.addCurrency(CurrencyEntry(1, "GLD", CurrencyType::Soft,   CurrencyAcquisition::Reward)));
    REQUIRE(ed.currencyCount() == 3u);
    REQUIRE(ed.removeCurrency(2));
    REQUIRE(ed.currencyCount() == 2u);
    REQUIRE(!ed.removeCurrency(99));
}

TEST_CASE("VirtualCurrencyEditor counts and find", "[Editor][S122]") {
    VirtualCurrencyEditor ed;
    CurrencyEntry c1(1, "GLD", CurrencyType::Soft,    CurrencyAcquisition::Reward);
    CurrencyEntry c2(2, "GLD2",CurrencyType::Soft,    CurrencyAcquisition::Quest);   c2.setIsEnabled(false);
    CurrencyEntry c3(3, "GEM", CurrencyType::Hard,    CurrencyAcquisition::Purchase);
    CurrencyEntry c4(4, "EVT", CurrencyType::Event,   CurrencyAcquisition::Reward);  c4.setIsEnabled(false);
    ed.addCurrency(c1); ed.addCurrency(c2); ed.addCurrency(c3); ed.addCurrency(c4);
    REQUIRE(ed.countByType(CurrencyType::Soft)                     == 2u);
    REQUIRE(ed.countByType(CurrencyType::Hard)                     == 1u);
    REQUIRE(ed.countByType(CurrencyType::Premium)                  == 0u);
    REQUIRE(ed.countByAcquisition(CurrencyAcquisition::Reward)     == 2u);
    REQUIRE(ed.countByAcquisition(CurrencyAcquisition::Purchase)   == 1u);
    REQUIRE(ed.countByAcquisition(CurrencyAcquisition::Trade)      == 0u);
    REQUIRE(ed.countEnabled()                                      == 2u);
    auto* found = ed.findCurrency(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == CurrencyType::Hard);
    REQUIRE(ed.findCurrency(99) == nullptr);
}

TEST_CASE("VirtualCurrencyEditor settings mutation", "[Editor][S122]") {
    VirtualCurrencyEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(true);
    ed.setBaseExchangeRate(2.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.baseExchangeRate() == 2.0f);
}

// ── ItemShopEditor ────────────────────────────────────────────────────────────

TEST_CASE("ShopCategory names", "[Editor][S122]") {
    REQUIRE(std::string(shopCategoryName(ShopCategory::Cosmetic))   == "Cosmetic");
    REQUIRE(std::string(shopCategoryName(ShopCategory::Consumable)) == "Consumable");
    REQUIRE(std::string(shopCategoryName(ShopCategory::Equipment))  == "Equipment");
    REQUIRE(std::string(shopCategoryName(ShopCategory::Bundle))     == "Bundle");
    REQUIRE(std::string(shopCategoryName(ShopCategory::Currency))   == "Currency");
    REQUIRE(std::string(shopCategoryName(ShopCategory::Seasonal))   == "Seasonal");
}

TEST_CASE("PricingModel names", "[Editor][S122]") {
    REQUIRE(std::string(pricingModelName(PricingModel::Fixed))        == "Fixed");
    REQUIRE(std::string(pricingModelName(PricingModel::Dynamic))      == "Dynamic");
    REQUIRE(std::string(pricingModelName(PricingModel::Auction))      == "Auction");
    REQUIRE(std::string(pricingModelName(PricingModel::Tiered))       == "Tiered");
    REQUIRE(std::string(pricingModelName(PricingModel::Subscription)) == "Subscription");
}

TEST_CASE("ShopItem defaults", "[Editor][S122]") {
    ShopItem item(1, "dragon_skin", ShopCategory::Cosmetic, PricingModel::Fixed);
    REQUIRE(item.id()           == 1u);
    REQUIRE(item.name()         == "dragon_skin");
    REQUIRE(item.category()     == ShopCategory::Cosmetic);
    REQUIRE(item.pricingModel() == PricingModel::Fixed);
    REQUIRE(item.basePrice()    == 100u);
    REQUIRE(!item.isLimited());
}

TEST_CASE("ShopItem mutation", "[Editor][S122]") {
    ShopItem item(2, "health_pack", ShopCategory::Consumable, PricingModel::Fixed);
    item.setBasePrice(50u);
    item.setIsLimited(true);
    REQUIRE(item.basePrice() == 50u);
    REQUIRE(item.isLimited());
}

TEST_CASE("ItemShopEditor defaults", "[Editor][S122]") {
    ItemShopEditor ed;
    REQUIRE(!ed.isShowOutOfStock());
    REQUIRE(!ed.isGroupByCategory());
    REQUIRE(ed.taxRate()   == 0.0f);
    REQUIRE(ed.itemCount() == 0u);
}

TEST_CASE("ItemShopEditor add/remove items", "[Editor][S122]") {
    ItemShopEditor ed;
    REQUIRE(ed.addItem(ShopItem(1, "item_a", ShopCategory::Cosmetic,   PricingModel::Fixed)));
    REQUIRE(ed.addItem(ShopItem(2, "item_b", ShopCategory::Consumable, PricingModel::Fixed)));
    REQUIRE(ed.addItem(ShopItem(3, "item_c", ShopCategory::Bundle,     PricingModel::Tiered)));
    REQUIRE(!ed.addItem(ShopItem(1, "item_a", ShopCategory::Cosmetic,  PricingModel::Fixed)));
    REQUIRE(ed.itemCount() == 3u);
    REQUIRE(ed.removeItem(2));
    REQUIRE(ed.itemCount() == 2u);
    REQUIRE(!ed.removeItem(99));
}

TEST_CASE("ItemShopEditor counts and find", "[Editor][S122]") {
    ItemShopEditor ed;
    ShopItem i1(1, "i1", ShopCategory::Cosmetic,   PricingModel::Fixed);
    ShopItem i2(2, "i2", ShopCategory::Cosmetic,   PricingModel::Dynamic);  i2.setIsLimited(true);
    ShopItem i3(3, "i3", ShopCategory::Consumable, PricingModel::Fixed);
    ShopItem i4(4, "i4", ShopCategory::Equipment,  PricingModel::Auction);  i4.setIsLimited(true);
    ed.addItem(i1); ed.addItem(i2); ed.addItem(i3); ed.addItem(i4);
    REQUIRE(ed.countByCategory(ShopCategory::Cosmetic)         == 2u);
    REQUIRE(ed.countByCategory(ShopCategory::Consumable)       == 1u);
    REQUIRE(ed.countByCategory(ShopCategory::Seasonal)         == 0u);
    REQUIRE(ed.countByPricingModel(PricingModel::Fixed)        == 2u);
    REQUIRE(ed.countByPricingModel(PricingModel::Dynamic)      == 1u);
    REQUIRE(ed.countByPricingModel(PricingModel::Subscription) == 0u);
    REQUIRE(ed.countLimited()                                  == 2u);
    auto* found = ed.findItem(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == ShopCategory::Consumable);
    REQUIRE(ed.findItem(99) == nullptr);
}

TEST_CASE("ItemShopEditor settings mutation", "[Editor][S122]") {
    ItemShopEditor ed;
    ed.setIsShowOutOfStock(true);
    ed.setIsGroupByCategory(true);
    ed.setTaxRate(0.15f);
    REQUIRE(ed.isShowOutOfStock());
    REQUIRE(ed.isGroupByCategory());
    REQUIRE(ed.taxRate() == 0.15f);
}
