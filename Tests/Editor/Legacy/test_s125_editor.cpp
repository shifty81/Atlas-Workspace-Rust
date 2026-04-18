// S125 editor tests: RewardSystemEditor, DailyQuestEditor, SeasonPassEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SeasonPassEditor.h"
#include "NF/Editor/DailyQuestEditor.h"
#include "NF/Editor/RewardSystemEditor.h"

using namespace NF;

// ── RewardSystemEditor ────────────────────────────────────────────────────────

TEST_CASE("RewardType names", "[Editor][S125]") {
    REQUIRE(std::string(rewardTypeName(RewardType::Currency)) == "Currency");
    REQUIRE(std::string(rewardTypeName(RewardType::Item))     == "Item");
    REQUIRE(std::string(rewardTypeName(RewardType::XP))       == "XP");
    REQUIRE(std::string(rewardTypeName(RewardType::Cosmetic)) == "Cosmetic");
    REQUIRE(std::string(rewardTypeName(RewardType::Booster))  == "Booster");
    REQUIRE(std::string(rewardTypeName(RewardType::Custom))   == "Custom");
}

TEST_CASE("RewardTrigger names", "[Editor][S125]") {
    REQUIRE(std::string(rewardTriggerName(RewardTrigger::LevelUp))       == "LevelUp");
    REQUIRE(std::string(rewardTriggerName(RewardTrigger::QuestComplete)) == "QuestComplete");
    REQUIRE(std::string(rewardTriggerName(RewardTrigger::Achievement))   == "Achievement");
    REQUIRE(std::string(rewardTriggerName(RewardTrigger::Login))         == "Login");
    REQUIRE(std::string(rewardTriggerName(RewardTrigger::Purchase))      == "Purchase");
    REQUIRE(std::string(rewardTriggerName(RewardTrigger::Streak))        == "Streak");
}

TEST_CASE("RewardEntry defaults", "[Editor][S125]") {
    RewardEntry r(1, "gold_coins", RewardType::Currency, RewardTrigger::LevelUp);
    REQUIRE(r.id()          == 1u);
    REQUIRE(r.name()        == "gold_coins");
    REQUIRE(r.type()        == RewardType::Currency);
    REQUIRE(r.trigger()     == RewardTrigger::LevelUp);
    REQUIRE(r.quantity()    == 1u);
    REQUIRE(r.isStackable());
    REQUIRE(r.isEnabled());
}

TEST_CASE("RewardEntry mutation", "[Editor][S125]") {
    RewardEntry r(2, "rare_sword", RewardType::Item, RewardTrigger::Achievement);
    r.setQuantity(5u);
    r.setIsStackable(false);
    r.setIsEnabled(false);
    REQUIRE(r.quantity()    == 5u);
    REQUIRE(!r.isStackable());
    REQUIRE(!r.isEnabled());
}

TEST_CASE("RewardSystemEditor defaults", "[Editor][S125]") {
    RewardSystemEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.globalMultiplier() == 1.0f);
    REQUIRE(ed.rewardCount()      == 0u);
}

TEST_CASE("RewardSystemEditor add/remove rewards", "[Editor][S125]") {
    RewardSystemEditor ed;
    REQUIRE(ed.addReward(RewardEntry(1, "gold",  RewardType::Currency, RewardTrigger::LevelUp)));
    REQUIRE(ed.addReward(RewardEntry(2, "sword", RewardType::Item,     RewardTrigger::Achievement)));
    REQUIRE(ed.addReward(RewardEntry(3, "xp_up", RewardType::XP,      RewardTrigger::Login)));
    REQUIRE(!ed.addReward(RewardEntry(1, "gold", RewardType::Currency, RewardTrigger::LevelUp)));
    REQUIRE(ed.rewardCount() == 3u);
    REQUIRE(ed.removeReward(2));
    REQUIRE(ed.rewardCount() == 2u);
    REQUIRE(!ed.removeReward(99));
}

TEST_CASE("RewardSystemEditor counts and find", "[Editor][S125]") {
    RewardSystemEditor ed;
    RewardEntry r1(1, "gold_a",    RewardType::Currency, RewardTrigger::LevelUp);
    RewardEntry r2(2, "gold_b",    RewardType::Currency, RewardTrigger::LevelUp);   r2.setIsEnabled(false);
    RewardEntry r3(3, "sword_a",   RewardType::Item,     RewardTrigger::Achievement);
    RewardEntry r4(4, "booster_a", RewardType::Booster,  RewardTrigger::Streak);     r4.setIsEnabled(false);
    ed.addReward(r1); ed.addReward(r2); ed.addReward(r3); ed.addReward(r4);
    REQUIRE(ed.countByType(RewardType::Currency)          == 2u);
    REQUIRE(ed.countByType(RewardType::Item)              == 1u);
    REQUIRE(ed.countByType(RewardType::XP)                == 0u);
    REQUIRE(ed.countByTrigger(RewardTrigger::LevelUp)     == 2u);
    REQUIRE(ed.countByTrigger(RewardTrigger::Achievement) == 1u);
    REQUIRE(ed.countEnabled()                             == 2u);
    auto* found = ed.findReward(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == RewardType::Item);
    REQUIRE(ed.findReward(99) == nullptr);
}

TEST_CASE("RewardSystemEditor settings mutation", "[Editor][S125]") {
    RewardSystemEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(true);
    ed.setGlobalMultiplier(2.5f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.globalMultiplier() == 2.5f);
}

// ── DailyQuestEditor ──────────────────────────────────────────────────────────

TEST_CASE("DailyQuestDifficulty names", "[Editor][S125]") {
    REQUIRE(std::string(dailyQuestDifficultyName(DailyQuestDifficulty::Trivial)) == "Trivial");
    REQUIRE(std::string(dailyQuestDifficultyName(DailyQuestDifficulty::Easy))    == "Easy");
    REQUIRE(std::string(dailyQuestDifficultyName(DailyQuestDifficulty::Medium))  == "Medium");
    REQUIRE(std::string(dailyQuestDifficultyName(DailyQuestDifficulty::Hard))    == "Hard");
    REQUIRE(std::string(dailyQuestDifficultyName(DailyQuestDifficulty::Epic))    == "Epic");
}

TEST_CASE("DailyQuestStatus names", "[Editor][S125]") {
    REQUIRE(std::string(dailyQuestStatusName(DailyQuestStatus::Locked))    == "Locked");
    REQUIRE(std::string(dailyQuestStatusName(DailyQuestStatus::Available)) == "Available");
    REQUIRE(std::string(dailyQuestStatusName(DailyQuestStatus::Active))    == "Active");
    REQUIRE(std::string(dailyQuestStatusName(DailyQuestStatus::Completed)) == "Completed");
    REQUIRE(std::string(dailyQuestStatusName(DailyQuestStatus::Expired))   == "Expired");
}

TEST_CASE("DailyQuestEntry defaults", "[Editor][S125]") {
    DailyQuestEntry q(1, "kill_ten_goblins", DailyQuestDifficulty::Easy);
    REQUIRE(q.id()         == 1u);
    REQUIRE(q.name()       == "kill_ten_goblins");
    REQUIRE(q.difficulty() == DailyQuestDifficulty::Easy);
    REQUIRE(q.status()     == DailyQuestStatus::Available);
    REQUIRE(q.xpReward()   == 100u);
    REQUIRE(!q.isPinned());
    REQUIRE(q.isEnabled());
}

TEST_CASE("DailyQuestEntry mutation", "[Editor][S125]") {
    DailyQuestEntry q(2, "slay_dragon", DailyQuestDifficulty::Epic);
    q.setStatus(DailyQuestStatus::Completed);
    q.setXpReward(5000u);
    q.setIsPinned(true);
    q.setIsEnabled(false);
    REQUIRE(q.status()   == DailyQuestStatus::Completed);
    REQUIRE(q.xpReward() == 5000u);
    REQUIRE(q.isPinned());
    REQUIRE(!q.isEnabled());
}

TEST_CASE("DailyQuestEditor defaults", "[Editor][S125]") {
    DailyQuestEditor ed;
    REQUIRE(!ed.isShowExpired());
    REQUIRE(ed.isGroupByDifficulty());
    REQUIRE(ed.refreshHours() == 24u);
    REQUIRE(ed.questCount()   == 0u);
}

TEST_CASE("DailyQuestEditor add/remove quests", "[Editor][S125]") {
    DailyQuestEditor ed;
    REQUIRE(ed.addQuest(DailyQuestEntry(1, "q_a", DailyQuestDifficulty::Easy)));
    REQUIRE(ed.addQuest(DailyQuestEntry(2, "q_b", DailyQuestDifficulty::Medium)));
    REQUIRE(ed.addQuest(DailyQuestEntry(3, "q_c", DailyQuestDifficulty::Hard)));
    REQUIRE(!ed.addQuest(DailyQuestEntry(1, "q_a", DailyQuestDifficulty::Easy)));
    REQUIRE(ed.questCount() == 3u);
    REQUIRE(ed.removeQuest(2));
    REQUIRE(ed.questCount() == 2u);
    REQUIRE(!ed.removeQuest(99));
}

TEST_CASE("DailyQuestEditor counts and find", "[Editor][S125]") {
    DailyQuestEditor ed;
    DailyQuestEntry q1(1, "q_a", DailyQuestDifficulty::Easy);
    DailyQuestEntry q2(2, "q_b", DailyQuestDifficulty::Easy);   q2.setStatus(DailyQuestStatus::Completed);
    DailyQuestEntry q3(3, "q_c", DailyQuestDifficulty::Medium); q3.setIsEnabled(false);
    DailyQuestEntry q4(4, "q_d", DailyQuestDifficulty::Hard);   q4.setStatus(DailyQuestStatus::Completed); q4.setIsEnabled(false);
    ed.addQuest(q1); ed.addQuest(q2); ed.addQuest(q3); ed.addQuest(q4);
    REQUIRE(ed.countByDifficulty(DailyQuestDifficulty::Easy)      == 2u);
    REQUIRE(ed.countByDifficulty(DailyQuestDifficulty::Medium)    == 1u);
    REQUIRE(ed.countByDifficulty(DailyQuestDifficulty::Epic)      == 0u);
    REQUIRE(ed.countByStatus(DailyQuestStatus::Available)         == 2u);
    REQUIRE(ed.countByStatus(DailyQuestStatus::Completed)         == 2u);
    REQUIRE(ed.countEnabled()                                     == 2u);
    auto* found = ed.findQuest(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->difficulty() == DailyQuestDifficulty::Medium);
    REQUIRE(ed.findQuest(99) == nullptr);
}

TEST_CASE("DailyQuestEditor settings mutation", "[Editor][S125]") {
    DailyQuestEditor ed;
    ed.setIsShowExpired(true);
    ed.setIsGroupByDifficulty(false);
    ed.setRefreshHours(48u);
    REQUIRE(ed.isShowExpired());
    REQUIRE(!ed.isGroupByDifficulty());
    REQUIRE(ed.refreshHours() == 48u);
}

// ── SeasonPassEditor ──────────────────────────────────────────────────────────

TEST_CASE("SeasonTier names", "[Editor][S125]") {
    REQUIRE(std::string(seasonTierName(SeasonTier::Free))     == "Free");
    REQUIRE(std::string(seasonTierName(SeasonTier::Bronze))   == "Bronze");
    REQUIRE(std::string(seasonTierName(SeasonTier::Silver))   == "Silver");
    REQUIRE(std::string(seasonTierName(SeasonTier::Gold))     == "Gold");
    REQUIRE(std::string(seasonTierName(SeasonTier::Platinum)) == "Platinum");
    REQUIRE(std::string(seasonTierName(SeasonTier::Custom))   == "Custom");
}

TEST_CASE("SeasonPassState names", "[Editor][S125]") {
    REQUIRE(std::string(seasonPassStateName(SeasonPassState::Upcoming)) == "Upcoming");
    REQUIRE(std::string(seasonPassStateName(SeasonPassState::Active))   == "Active");
    REQUIRE(std::string(seasonPassStateName(SeasonPassState::Ending))   == "Ending");
    REQUIRE(std::string(seasonPassStateName(SeasonPassState::Ended))    == "Ended");
    REQUIRE(std::string(seasonPassStateName(SeasonPassState::Draft))    == "Draft");
}

TEST_CASE("SeasonPassEntry defaults", "[Editor][S125]") {
    SeasonPassEntry e(1, "season_1", SeasonTier::Free);
    REQUIRE(e.id()          == 1u);
    REQUIRE(e.name()        == "season_1");
    REQUIRE(e.tier()        == SeasonTier::Free);
    REQUIRE(e.state()       == SeasonPassState::Draft);
    REQUIRE(e.rewardCount() == 0u);
    REQUIRE(!e.isPremium());
    REQUIRE(e.isEnabled());
}

TEST_CASE("SeasonPassEntry mutation", "[Editor][S125]") {
    SeasonPassEntry e(2, "season_gold", SeasonTier::Gold);
    e.setState(SeasonPassState::Active);
    e.setRewardCount(50u);
    e.setIsPremium(true);
    e.setIsEnabled(false);
    REQUIRE(e.state()       == SeasonPassState::Active);
    REQUIRE(e.rewardCount() == 50u);
    REQUIRE(e.isPremium());
    REQUIRE(!e.isEnabled());
}

TEST_CASE("SeasonPassEditor defaults", "[Editor][S125]") {
    SeasonPassEditor ed;
    REQUIRE(ed.isShowDraft());
    REQUIRE(!ed.isGroupByTier());
    REQUIRE(ed.seasonDurationDays() == 90u);
    REQUIRE(ed.entryCount()         == 0u);
}

TEST_CASE("SeasonPassEditor add/remove entries", "[Editor][S125]") {
    SeasonPassEditor ed;
    REQUIRE(ed.addPassEntry(SeasonPassEntry(1, "sp_a", SeasonTier::Free)));
    REQUIRE(ed.addPassEntry(SeasonPassEntry(2, "sp_b", SeasonTier::Bronze)));
    REQUIRE(ed.addPassEntry(SeasonPassEntry(3, "sp_c", SeasonTier::Gold)));
    REQUIRE(!ed.addPassEntry(SeasonPassEntry(1, "sp_a", SeasonTier::Free)));
    REQUIRE(ed.entryCount() == 3u);
    REQUIRE(ed.removePassEntry(2));
    REQUIRE(ed.entryCount() == 2u);
    REQUIRE(!ed.removePassEntry(99));
}

TEST_CASE("SeasonPassEditor counts and find", "[Editor][S125]") {
    SeasonPassEditor ed;
    SeasonPassEntry e1(1, "sp_a", SeasonTier::Free);
    SeasonPassEntry e2(2, "sp_b", SeasonTier::Free);     e2.setState(SeasonPassState::Active);
    SeasonPassEntry e3(3, "sp_c", SeasonTier::Bronze);   e3.setIsPremium(true);
    SeasonPassEntry e4(4, "sp_d", SeasonTier::Gold);     e4.setState(SeasonPassState::Active); e4.setIsPremium(true);
    ed.addPassEntry(e1); ed.addPassEntry(e2); ed.addPassEntry(e3); ed.addPassEntry(e4);
    REQUIRE(ed.countByTier(SeasonTier::Free)              == 2u);
    REQUIRE(ed.countByTier(SeasonTier::Bronze)            == 1u);
    REQUIRE(ed.countByTier(SeasonTier::Platinum)          == 0u);
    REQUIRE(ed.countByState(SeasonPassState::Draft)       == 2u);
    REQUIRE(ed.countByState(SeasonPassState::Active)      == 2u);
    REQUIRE(ed.countPremium()                             == 2u);
    auto* found = ed.findPassEntry(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->tier() == SeasonTier::Bronze);
    REQUIRE(ed.findPassEntry(99) == nullptr);
}

TEST_CASE("SeasonPassEditor settings mutation", "[Editor][S125]") {
    SeasonPassEditor ed;
    ed.setIsShowDraft(false);
    ed.setIsGroupByTier(true);
    ed.setSeasonDurationDays(30u);
    REQUIRE(!ed.isShowDraft());
    REQUIRE(ed.isGroupByTier());
    REQUIRE(ed.seasonDurationDays() == 30u);
}
