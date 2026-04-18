// S115 editor tests: LeaderboardEditor, AchievementEditor, TrophyEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TrophyEditor.h"
#include "NF/Editor/AchievementEditor.h"
#include "NF/Editor/LeaderboardEditor.h"

using namespace NF;

// ── LeaderboardEditor ────────────────────────────────────────────────────────

TEST_CASE("LeaderboardScope names", "[Editor][S115]") {
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Global))   == "Global");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Regional)) == "Regional");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Friends))  == "Friends");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Guild))    == "Guild");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Season))   == "Season");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Weekly))   == "Weekly");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Daily))    == "Daily");
    REQUIRE(std::string(leaderboardScopeName(LeaderboardScope::Custom))   == "Custom");
}

TEST_CASE("LeaderboardSortOrder names", "[Editor][S115]") {
    REQUIRE(std::string(leaderboardSortOrderName(LeaderboardSortOrder::Descending)) == "Descending");
    REQUIRE(std::string(leaderboardSortOrderName(LeaderboardSortOrder::Ascending))  == "Ascending");
}

TEST_CASE("LeaderboardScoreType names", "[Editor][S115]") {
    REQUIRE(std::string(leaderboardScoreTypeName(LeaderboardScoreType::Integer))    == "Integer");
    REQUIRE(std::string(leaderboardScoreTypeName(LeaderboardScoreType::Float))      == "Float");
    REQUIRE(std::string(leaderboardScoreTypeName(LeaderboardScoreType::Time))       == "Time");
    REQUIRE(std::string(leaderboardScoreTypeName(LeaderboardScoreType::Distance))   == "Distance");
    REQUIRE(std::string(leaderboardScoreTypeName(LeaderboardScoreType::Percentage)) == "Percentage");
}

TEST_CASE("LeaderboardDef defaults", "[Editor][S115]") {
    LeaderboardDef lb(1, "global_score", LeaderboardScope::Global);
    REQUIRE(lb.id()              == 1u);
    REQUIRE(lb.name()            == "global_score");
    REQUIRE(lb.scope()           == LeaderboardScope::Global);
    REQUIRE(lb.sortOrder()       == LeaderboardSortOrder::Descending);
    REQUIRE(lb.scoreType()       == LeaderboardScoreType::Integer);
    REQUIRE(lb.maxEntries()      == 100u);
    REQUIRE(lb.isPublic());
    REQUIRE(lb.resetPeriodDays() == 0u);
}

TEST_CASE("LeaderboardDef mutation", "[Editor][S115]") {
    LeaderboardDef lb(2, "weekly_time", LeaderboardScope::Weekly);
    lb.setSortOrder(LeaderboardSortOrder::Ascending);
    lb.setScoreType(LeaderboardScoreType::Time);
    lb.setMaxEntries(50u);
    lb.setIsPublic(false);
    lb.setResetPeriodDays(7u);
    REQUIRE(lb.sortOrder()       == LeaderboardSortOrder::Ascending);
    REQUIRE(lb.scoreType()       == LeaderboardScoreType::Time);
    REQUIRE(lb.maxEntries()      == 50u);
    REQUIRE(!lb.isPublic());
    REQUIRE(lb.resetPeriodDays() == 7u);
}

TEST_CASE("LeaderboardEditor defaults", "[Editor][S115]") {
    LeaderboardEditor ed;
    REQUIRE(ed.isShowPreview());
    REQUIRE(ed.isShowRankBand());
    REQUIRE(ed.pageSize()   == 25u);
    REQUIRE(ed.boardCount() == 0u);
}

TEST_CASE("LeaderboardEditor add/remove boards", "[Editor][S115]") {
    LeaderboardEditor ed;
    REQUIRE(ed.addBoard(LeaderboardDef(1, "global", LeaderboardScope::Global)));
    REQUIRE(ed.addBoard(LeaderboardDef(2, "weekly", LeaderboardScope::Weekly)));
    REQUIRE(ed.addBoard(LeaderboardDef(3, "friends", LeaderboardScope::Friends)));
    REQUIRE(!ed.addBoard(LeaderboardDef(1, "global", LeaderboardScope::Global)));
    REQUIRE(ed.boardCount() == 3u);
    REQUIRE(ed.removeBoard(2));
    REQUIRE(ed.boardCount() == 2u);
    REQUIRE(!ed.removeBoard(99));
}

TEST_CASE("LeaderboardEditor counts and find", "[Editor][S115]") {
    LeaderboardEditor ed;
    LeaderboardDef b1(1, "global_a", LeaderboardScope::Global);
    LeaderboardDef b2(2, "global_b", LeaderboardScope::Global);  b2.setScoreType(LeaderboardScoreType::Time);
    LeaderboardDef b3(3, "friends_a", LeaderboardScope::Friends); b3.setIsPublic(false);
    LeaderboardDef b4(4, "season_a",  LeaderboardScope::Season);  b4.setScoreType(LeaderboardScoreType::Time); b4.setIsPublic(false);
    ed.addBoard(b1); ed.addBoard(b2); ed.addBoard(b3); ed.addBoard(b4);
    REQUIRE(ed.countByScope(LeaderboardScope::Global)           == 2u);
    REQUIRE(ed.countByScope(LeaderboardScope::Friends)          == 1u);
    REQUIRE(ed.countByScope(LeaderboardScope::Daily)            == 0u);
    REQUIRE(ed.countByScoreType(LeaderboardScoreType::Integer)  == 2u);
    REQUIRE(ed.countByScoreType(LeaderboardScoreType::Time)     == 2u);
    REQUIRE(ed.countPublic()                                    == 2u);
    auto* found = ed.findBoard(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->scope() == LeaderboardScope::Friends);
    REQUIRE(ed.findBoard(99) == nullptr);
}

TEST_CASE("LeaderboardEditor settings mutation", "[Editor][S115]") {
    LeaderboardEditor ed;
    ed.setShowPreview(false);
    ed.setShowRankBand(false);
    ed.setPageSize(50u);
    REQUIRE(!ed.isShowPreview());
    REQUIRE(!ed.isShowRankBand());
    REQUIRE(ed.pageSize() == 50u);
}

// ── AchievementEditor ────────────────────────────────────────────────────────

TEST_CASE("AchievementCategory names", "[Editor][S115]") {
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Combat))      == "Combat");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Exploration)) == "Exploration");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Collection))  == "Collection");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Social))      == "Social");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Progression)) == "Progression");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Secret))      == "Secret");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Seasonal))    == "Seasonal");
    REQUIRE(std::string(achievementCategoryName(AchievementCategory::Challenge))   == "Challenge");
}

TEST_CASE("AchievementRarity names", "[Editor][S115]") {
    REQUIRE(std::string(achievementRarityName(AchievementRarity::Common))    == "Common");
    REQUIRE(std::string(achievementRarityName(AchievementRarity::Uncommon))  == "Uncommon");
    REQUIRE(std::string(achievementRarityName(AchievementRarity::Rare))      == "Rare");
    REQUIRE(std::string(achievementRarityName(AchievementRarity::Epic))      == "Epic");
    REQUIRE(std::string(achievementRarityName(AchievementRarity::Legendary)) == "Legendary");
}

TEST_CASE("AchievementTrigger names", "[Editor][S115]") {
    REQUIRE(std::string(achievementTriggerName(AchievementTrigger::Immediate))  == "Immediate");
    REQUIRE(std::string(achievementTriggerName(AchievementTrigger::Cumulative)) == "Cumulative");
    REQUIRE(std::string(achievementTriggerName(AchievementTrigger::Threshold))  == "Threshold");
    REQUIRE(std::string(achievementTriggerName(AchievementTrigger::Sequence))   == "Sequence");
    REQUIRE(std::string(achievementTriggerName(AchievementTrigger::Timed))      == "Timed");
    REQUIRE(std::string(achievementTriggerName(AchievementTrigger::Composite))  == "Composite");
}

TEST_CASE("AchievementDef defaults", "[Editor][S115]") {
    AchievementDef ad(1, "first_kill", AchievementCategory::Combat);
    REQUIRE(ad.id()           == 1u);
    REQUIRE(ad.name()         == "first_kill");
    REQUIRE(ad.category()     == AchievementCategory::Combat);
    REQUIRE(ad.rarity()       == AchievementRarity::Common);
    REQUIRE(ad.trigger()      == AchievementTrigger::Immediate);
    REQUIRE(ad.pointValue()   == 10u);
    REQUIRE(!ad.isHidden());
    REQUIRE(!ad.isRepeatable());
}

TEST_CASE("AchievementDef mutation", "[Editor][S115]") {
    AchievementDef ad(2, "world_explorer", AchievementCategory::Exploration);
    ad.setRarity(AchievementRarity::Epic);
    ad.setTrigger(AchievementTrigger::Cumulative);
    ad.setPointValue(100u);
    ad.setIsHidden(true);
    ad.setIsRepeatable(true);
    REQUIRE(ad.rarity()       == AchievementRarity::Epic);
    REQUIRE(ad.trigger()      == AchievementTrigger::Cumulative);
    REQUIRE(ad.pointValue()   == 100u);
    REQUIRE(ad.isHidden());
    REQUIRE(ad.isRepeatable());
}

TEST_CASE("AchievementEditor defaults", "[Editor][S115]") {
    AchievementEditor ed;
    REQUIRE(!ed.isShowHidden());
    REQUIRE(ed.isShowProgress());
    REQUIRE(ed.isGroupByCategory());
    REQUIRE(ed.achievementCount() == 0u);
}

TEST_CASE("AchievementEditor add/remove achievements", "[Editor][S115]") {
    AchievementEditor ed;
    REQUIRE(ed.addAchievement(AchievementDef(1, "kill1",    AchievementCategory::Combat)));
    REQUIRE(ed.addAchievement(AchievementDef(2, "explore1", AchievementCategory::Exploration)));
    REQUIRE(ed.addAchievement(AchievementDef(3, "secret1",  AchievementCategory::Secret)));
    REQUIRE(!ed.addAchievement(AchievementDef(1, "kill1",   AchievementCategory::Combat)));
    REQUIRE(ed.achievementCount() == 3u);
    REQUIRE(ed.removeAchievement(2));
    REQUIRE(ed.achievementCount() == 2u);
    REQUIRE(!ed.removeAchievement(99));
}

TEST_CASE("AchievementEditor counts and find", "[Editor][S115]") {
    AchievementEditor ed;
    AchievementDef a1(1, "kill_a",    AchievementCategory::Combat);
    AchievementDef a2(2, "kill_b",    AchievementCategory::Combat);  a2.setRarity(AchievementRarity::Rare);
    AchievementDef a3(3, "explore_a", AchievementCategory::Exploration); a3.setIsHidden(true);
    AchievementDef a4(4, "secret_a",  AchievementCategory::Secret); a4.setRarity(AchievementRarity::Rare); a4.setIsHidden(true);
    ed.addAchievement(a1); ed.addAchievement(a2); ed.addAchievement(a3); ed.addAchievement(a4);
    REQUIRE(ed.countByCategory(AchievementCategory::Combat)      == 2u);
    REQUIRE(ed.countByCategory(AchievementCategory::Exploration) == 1u);
    REQUIRE(ed.countByCategory(AchievementCategory::Social)      == 0u);
    REQUIRE(ed.countByRarity(AchievementRarity::Common)          == 2u);
    REQUIRE(ed.countByRarity(AchievementRarity::Rare)            == 2u);
    REQUIRE(ed.countHidden()                                     == 2u);
    auto* found = ed.findAchievement(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == AchievementCategory::Exploration);
    REQUIRE(ed.findAchievement(99) == nullptr);
}

TEST_CASE("AchievementEditor settings mutation", "[Editor][S115]") {
    AchievementEditor ed;
    ed.setShowHidden(true);
    ed.setShowProgress(false);
    ed.setGroupByCategory(false);
    REQUIRE(ed.isShowHidden());
    REQUIRE(!ed.isShowProgress());
    REQUIRE(!ed.isGroupByCategory());
}

// ── TrophyEditor ─────────────────────────────────────────────────────────────

TEST_CASE("TrophyTier names", "[Editor][S115]") {
    REQUIRE(std::string(trophyTierName(TrophyTier::Bronze))   == "Bronze");
    REQUIRE(std::string(trophyTierName(TrophyTier::Silver))   == "Silver");
    REQUIRE(std::string(trophyTierName(TrophyTier::Gold))     == "Gold");
    REQUIRE(std::string(trophyTierName(TrophyTier::Platinum)) == "Platinum");
    REQUIRE(std::string(trophyTierName(TrophyTier::Diamond))  == "Diamond");
}

TEST_CASE("TrophyType names", "[Editor][S115]") {
    REQUIRE(std::string(trophyTypeName(TrophyType::Campaign))    == "Campaign");
    REQUIRE(std::string(trophyTypeName(TrophyType::Multiplayer)) == "Multiplayer");
    REQUIRE(std::string(trophyTypeName(TrophyType::Speedrun))    == "Speedrun");
    REQUIRE(std::string(trophyTypeName(TrophyType::Collection))  == "Collection");
    REQUIRE(std::string(trophyTypeName(TrophyType::Mastery))     == "Mastery");
    REQUIRE(std::string(trophyTypeName(TrophyType::Community))   == "Community");
    REQUIRE(std::string(trophyTypeName(TrophyType::Seasonal))    == "Seasonal");
    REQUIRE(std::string(trophyTypeName(TrophyType::Special))     == "Special");
}

TEST_CASE("TrophyDisplayMode names", "[Editor][S115]") {
    REQUIRE(std::string(trophyDisplayModeName(TrophyDisplayMode::Cabinet))  == "Cabinet");
    REQUIRE(std::string(trophyDisplayModeName(TrophyDisplayMode::Shelf))    == "Shelf");
    REQUIRE(std::string(trophyDisplayModeName(TrophyDisplayMode::Wall))     == "Wall");
    REQUIRE(std::string(trophyDisplayModeName(TrophyDisplayMode::Showcase)) == "Showcase");
}

TEST_CASE("TrophyDef defaults", "[Editor][S115]") {
    TrophyDef td(1, "campaign_bronze", TrophyTier::Bronze);
    REQUIRE(td.id()          == 1u);
    REQUIRE(td.name()        == "campaign_bronze");
    REQUIRE(td.tier()        == TrophyTier::Bronze);
    REQUIRE(td.type()        == TrophyType::Campaign);
    REQUIRE(td.pointValue()  == 50u);
    REQUIRE(!td.isRare());
    REQUIRE(!td.isStackable());
    REQUIRE(td.modelPath()   == "");
}

TEST_CASE("TrophyDef mutation", "[Editor][S115]") {
    TrophyDef td(2, "speed_platinum", TrophyTier::Platinum);
    td.setType(TrophyType::Speedrun);
    td.setPointValue(500u);
    td.setIsRare(true);
    td.setIsStackable(true);
    td.setModelPath("models/trophy_platinum.fbx");
    REQUIRE(td.type()       == TrophyType::Speedrun);
    REQUIRE(td.pointValue() == 500u);
    REQUIRE(td.isRare());
    REQUIRE(td.isStackable());
    REQUIRE(td.modelPath()  == "models/trophy_platinum.fbx");
}

TEST_CASE("TrophyEditor defaults", "[Editor][S115]") {
    TrophyEditor ed;
    REQUIRE(ed.displayMode()      == TrophyDisplayMode::Cabinet);
    REQUIRE(ed.isShowLabels());
    REQUIRE(ed.isShowAnimations());
    REQUIRE(ed.previewScale()     == 1.0f);
    REQUIRE(ed.trophyCount()      == 0u);
}

TEST_CASE("TrophyEditor add/remove trophies", "[Editor][S115]") {
    TrophyEditor ed;
    REQUIRE(ed.addTrophy(TrophyDef(1, "bronze1",   TrophyTier::Bronze)));
    REQUIRE(ed.addTrophy(TrophyDef(2, "silver1",   TrophyTier::Silver)));
    REQUIRE(ed.addTrophy(TrophyDef(3, "gold1",     TrophyTier::Gold)));
    REQUIRE(!ed.addTrophy(TrophyDef(1, "bronze1",  TrophyTier::Bronze)));
    REQUIRE(ed.trophyCount() == 3u);
    REQUIRE(ed.removeTrophy(2));
    REQUIRE(ed.trophyCount() == 2u);
    REQUIRE(!ed.removeTrophy(99));
}

TEST_CASE("TrophyEditor counts and find", "[Editor][S115]") {
    TrophyEditor ed;
    TrophyDef t1(1, "bronze_a",   TrophyTier::Bronze);
    TrophyDef t2(2, "bronze_b",   TrophyTier::Bronze);  t2.setType(TrophyType::Multiplayer);
    TrophyDef t3(3, "gold_a",     TrophyTier::Gold);    t3.setIsRare(true);
    TrophyDef t4(4, "platinum_a", TrophyTier::Platinum); t4.setType(TrophyType::Multiplayer); t4.setIsRare(true);
    ed.addTrophy(t1); ed.addTrophy(t2); ed.addTrophy(t3); ed.addTrophy(t4);
    REQUIRE(ed.countByTier(TrophyTier::Bronze)           == 2u);
    REQUIRE(ed.countByTier(TrophyTier::Gold)             == 1u);
    REQUIRE(ed.countByTier(TrophyTier::Diamond)          == 0u);
    REQUIRE(ed.countByType(TrophyType::Campaign)         == 2u);
    REQUIRE(ed.countByType(TrophyType::Multiplayer)      == 2u);
    REQUIRE(ed.countRare()                               == 2u);
    auto* found = ed.findTrophy(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->tier() == TrophyTier::Gold);
    REQUIRE(ed.findTrophy(99) == nullptr);
}

TEST_CASE("TrophyEditor settings mutation", "[Editor][S115]") {
    TrophyEditor ed;
    ed.setDisplayMode(TrophyDisplayMode::Showcase);
    ed.setShowLabels(false);
    ed.setShowAnimations(false);
    ed.setPreviewScale(2.0f);
    REQUIRE(ed.displayMode()      == TrophyDisplayMode::Showcase);
    REQUIRE(!ed.isShowLabels());
    REQUIRE(!ed.isShowAnimations());
    REQUIRE(ed.previewScale()     == 2.0f);
}
