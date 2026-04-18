// S127 editor tests: MatchmakingEditor, LobbyEditor, TournamentEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TournamentEditor.h"
#include "NF/Editor/LobbyEditor.h"
#include "NF/Editor/MatchmakingEditor.h"

using namespace NF;

// ── MatchmakingEditor ─────────────────────────────────────────────────────────

TEST_CASE("MatchmakingStrategy names", "[Editor][S127]") {
    REQUIRE(std::string(matchmakingStrategyName(MatchmakingStrategy::Random))       == "Random");
    REQUIRE(std::string(matchmakingStrategyName(MatchmakingStrategy::SkillBased))   == "SkillBased");
    REQUIRE(std::string(matchmakingStrategyName(MatchmakingStrategy::RegionBased))  == "RegionBased");
    REQUIRE(std::string(matchmakingStrategyName(MatchmakingStrategy::LatencyBased)) == "LatencyBased");
    REQUIRE(std::string(matchmakingStrategyName(MatchmakingStrategy::Custom))       == "Custom");
}

TEST_CASE("MatchmakingStatus names", "[Editor][S127]") {
    REQUIRE(std::string(matchmakingStatusName(MatchmakingStatus::Idle))       == "Idle");
    REQUIRE(std::string(matchmakingStatusName(MatchmakingStatus::Searching))  == "Searching");
    REQUIRE(std::string(matchmakingStatusName(MatchmakingStatus::Found))      == "Found");
    REQUIRE(std::string(matchmakingStatusName(MatchmakingStatus::Connecting)) == "Connecting");
    REQUIRE(std::string(matchmakingStatusName(MatchmakingStatus::Failed))     == "Failed");
    REQUIRE(std::string(matchmakingStatusName(MatchmakingStatus::Cancelled))  == "Cancelled");
}

TEST_CASE("MatchmakingRule defaults", "[Editor][S127]") {
    MatchmakingRule r(1, "skill_rule", MatchmakingStrategy::SkillBased);
    REQUIRE(r.id()         == 1u);
    REQUIRE(r.name()       == "skill_rule");
    REQUIRE(r.strategy()   == MatchmakingStrategy::SkillBased);
    REQUIRE(r.status()     == MatchmakingStatus::Idle);
    REQUIRE(r.maxPlayers() == 4u);
    REQUIRE(r.timeoutSec() == 60.0f);
    REQUIRE(r.isEnabled());
}

TEST_CASE("MatchmakingRule mutation", "[Editor][S127]") {
    MatchmakingRule r(2, "region_rule", MatchmakingStrategy::RegionBased);
    r.setStatus(MatchmakingStatus::Searching);
    r.setMaxPlayers(8u);
    r.setTimeoutSec(30.0f);
    r.setIsEnabled(false);
    REQUIRE(r.status()     == MatchmakingStatus::Searching);
    REQUIRE(r.maxPlayers() == 8u);
    REQUIRE(r.timeoutSec() == 30.0f);
    REQUIRE(!r.isEnabled());
}

TEST_CASE("MatchmakingEditor defaults", "[Editor][S127]") {
    MatchmakingEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByStrategy());
    REQUIRE(ed.globalTimeoutSec() == 120.0f);
    REQUIRE(ed.ruleCount()        == 0u);
}

TEST_CASE("MatchmakingEditor add/remove rules", "[Editor][S127]") {
    MatchmakingEditor ed;
    REQUIRE(ed.addRule(MatchmakingRule(1, "r_a", MatchmakingStrategy::Random)));
    REQUIRE(ed.addRule(MatchmakingRule(2, "r_b", MatchmakingStrategy::SkillBased)));
    REQUIRE(ed.addRule(MatchmakingRule(3, "r_c", MatchmakingStrategy::RegionBased)));
    REQUIRE(!ed.addRule(MatchmakingRule(1, "r_a", MatchmakingStrategy::Random)));
    REQUIRE(ed.ruleCount() == 3u);
    REQUIRE(ed.removeRule(2));
    REQUIRE(ed.ruleCount() == 2u);
    REQUIRE(!ed.removeRule(99));
}

TEST_CASE("MatchmakingEditor counts and find", "[Editor][S127]") {
    MatchmakingEditor ed;
    MatchmakingRule r1(1, "r_a", MatchmakingStrategy::SkillBased);
    MatchmakingRule r2(2, "r_b", MatchmakingStrategy::SkillBased); r2.setStatus(MatchmakingStatus::Searching);
    MatchmakingRule r3(3, "r_c", MatchmakingStrategy::RegionBased);
    MatchmakingRule r4(4, "r_d", MatchmakingStrategy::Random);     r4.setIsEnabled(false);
    ed.addRule(r1); ed.addRule(r2); ed.addRule(r3); ed.addRule(r4);
    REQUIRE(ed.countByStrategy(MatchmakingStrategy::SkillBased)  == 2u);
    REQUIRE(ed.countByStrategy(MatchmakingStrategy::RegionBased) == 1u);
    REQUIRE(ed.countByStrategy(MatchmakingStrategy::Custom)      == 0u);
    REQUIRE(ed.countByStatus(MatchmakingStatus::Idle)            == 3u);
    REQUIRE(ed.countByStatus(MatchmakingStatus::Searching)       == 1u);
    REQUIRE(ed.countEnabled()                                    == 3u);
    auto* found = ed.findRule(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->strategy() == MatchmakingStrategy::RegionBased);
    REQUIRE(ed.findRule(99) == nullptr);
}

TEST_CASE("MatchmakingEditor settings mutation", "[Editor][S127]") {
    MatchmakingEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByStrategy(true);
    ed.setGlobalTimeoutSec(300.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByStrategy());
    REQUIRE(ed.globalTimeoutSec() == 300.0f);
}

// ── LobbyEditor ───────────────────────────────────────────────────────────────

TEST_CASE("LobbyVisibility names", "[Editor][S127]") {
    REQUIRE(std::string(lobbyVisibilityName(LobbyVisibility::Public))      == "Public");
    REQUIRE(std::string(lobbyVisibilityName(LobbyVisibility::Private))     == "Private");
    REQUIRE(std::string(lobbyVisibilityName(LobbyVisibility::FriendsOnly)) == "FriendsOnly");
    REQUIRE(std::string(lobbyVisibilityName(LobbyVisibility::InviteOnly))  == "InviteOnly");
}

TEST_CASE("LobbyState names", "[Editor][S127]") {
    REQUIRE(std::string(lobbyStateName(LobbyState::Open))      == "Open");
    REQUIRE(std::string(lobbyStateName(LobbyState::Full))      == "Full");
    REQUIRE(std::string(lobbyStateName(LobbyState::InGame))    == "InGame");
    REQUIRE(std::string(lobbyStateName(LobbyState::Closed))    == "Closed");
    REQUIRE(std::string(lobbyStateName(LobbyState::Disbanded)) == "Disbanded");
}

TEST_CASE("LobbyEntry defaults", "[Editor][S127]") {
    LobbyEntry e(1, "main_lobby", LobbyVisibility::Public);
    REQUIRE(e.id()             == 1u);
    REQUIRE(e.name()           == "main_lobby");
    REQUIRE(e.visibility()     == LobbyVisibility::Public);
    REQUIRE(e.state()          == LobbyState::Open);
    REQUIRE(e.maxSlots()       == 8u);
    REQUIRE(!e.isVoiceEnabled());
    REQUIRE(e.isEnabled());
}

TEST_CASE("LobbyEntry mutation", "[Editor][S127]") {
    LobbyEntry e(2, "private_lobby", LobbyVisibility::Private);
    e.setState(LobbyState::Full);
    e.setMaxSlots(16u);
    e.setIsVoiceEnabled(true);
    e.setIsEnabled(false);
    REQUIRE(e.state()          == LobbyState::Full);
    REQUIRE(e.maxSlots()       == 16u);
    REQUIRE(e.isVoiceEnabled());
    REQUIRE(!e.isEnabled());
}

TEST_CASE("LobbyEditor defaults", "[Editor][S127]") {
    LobbyEditor ed;
    REQUIRE(!ed.isShowClosed());
    REQUIRE(!ed.isGroupByVisibility());
    REQUIRE(ed.defaultMaxSlots() == 4u);
    REQUIRE(ed.lobbyCount()      == 0u);
}

TEST_CASE("LobbyEditor add/remove lobbies", "[Editor][S127]") {
    LobbyEditor ed;
    REQUIRE(ed.addLobby(LobbyEntry(1, "l_a", LobbyVisibility::Public)));
    REQUIRE(ed.addLobby(LobbyEntry(2, "l_b", LobbyVisibility::Private)));
    REQUIRE(ed.addLobby(LobbyEntry(3, "l_c", LobbyVisibility::InviteOnly)));
    REQUIRE(!ed.addLobby(LobbyEntry(1, "l_a", LobbyVisibility::Public)));
    REQUIRE(ed.lobbyCount() == 3u);
    REQUIRE(ed.removeLobby(2));
    REQUIRE(ed.lobbyCount() == 2u);
    REQUIRE(!ed.removeLobby(99));
}

TEST_CASE("LobbyEditor counts and find", "[Editor][S127]") {
    LobbyEditor ed;
    LobbyEntry e1(1, "l_a", LobbyVisibility::Public);
    LobbyEntry e2(2, "l_b", LobbyVisibility::Public);  e2.setState(LobbyState::Full);
    LobbyEntry e3(3, "l_c", LobbyVisibility::Private); e3.setState(LobbyState::InGame);
    LobbyEntry e4(4, "l_d", LobbyVisibility::InviteOnly); e4.setState(LobbyState::Closed); e4.setIsEnabled(false);
    ed.addLobby(e1); ed.addLobby(e2); ed.addLobby(e3); ed.addLobby(e4);
    REQUIRE(ed.countByVisibility(LobbyVisibility::Public)      == 2u);
    REQUIRE(ed.countByVisibility(LobbyVisibility::Private)     == 1u);
    REQUIRE(ed.countByVisibility(LobbyVisibility::FriendsOnly) == 0u);
    REQUIRE(ed.countByState(LobbyState::Open)                  == 1u);
    REQUIRE(ed.countByState(LobbyState::Full)                  == 1u);
    REQUIRE(ed.countEnabled()                                  == 3u);
    auto* found = ed.findLobby(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->visibility() == LobbyVisibility::Private);
    REQUIRE(ed.findLobby(99) == nullptr);
}

TEST_CASE("LobbyEditor settings mutation", "[Editor][S127]") {
    LobbyEditor ed;
    ed.setIsShowClosed(true);
    ed.setIsGroupByVisibility(true);
    ed.setDefaultMaxSlots(12u);
    REQUIRE(ed.isShowClosed());
    REQUIRE(ed.isGroupByVisibility());
    REQUIRE(ed.defaultMaxSlots() == 12u);
}

// ── TournamentEditor ──────────────────────────────────────────────────────────

TEST_CASE("TournamentFormat names", "[Editor][S127]") {
    REQUIRE(std::string(tournamentFormatName(TournamentFormat::SingleElim)) == "SingleElim");
    REQUIRE(std::string(tournamentFormatName(TournamentFormat::DoubleElim)) == "DoubleElim");
    REQUIRE(std::string(tournamentFormatName(TournamentFormat::RoundRobin)) == "RoundRobin");
    REQUIRE(std::string(tournamentFormatName(TournamentFormat::Swiss))      == "Swiss");
    REQUIRE(std::string(tournamentFormatName(TournamentFormat::FFA))        == "FFA");
}

TEST_CASE("TournamentStatus names", "[Editor][S127]") {
    REQUIRE(std::string(tournamentStatusName(TournamentStatus::Draft))        == "Draft");
    REQUIRE(std::string(tournamentStatusName(TournamentStatus::Registration)) == "Registration");
    REQUIRE(std::string(tournamentStatusName(TournamentStatus::Active))       == "Active");
    REQUIRE(std::string(tournamentStatusName(TournamentStatus::Completed))    == "Completed");
    REQUIRE(std::string(tournamentStatusName(TournamentStatus::Cancelled))    == "Cancelled");
}

TEST_CASE("TournamentEntry defaults", "[Editor][S127]") {
    TournamentEntry e(1, "spring_cup", TournamentFormat::SingleElim);
    REQUIRE(e.id()              == 1u);
    REQUIRE(e.name()            == "spring_cup");
    REQUIRE(e.format()          == TournamentFormat::SingleElim);
    REQUIRE(e.status()          == TournamentStatus::Draft);
    REQUIRE(e.maxParticipants() == 16u);
    REQUIRE(!e.isPrized());
    REQUIRE(e.isEnabled());
}

TEST_CASE("TournamentEntry mutation", "[Editor][S127]") {
    TournamentEntry e(2, "world_cup", TournamentFormat::DoubleElim);
    e.setStatus(TournamentStatus::Active);
    e.setMaxParticipants(32u);
    e.setIsPrized(true);
    e.setIsEnabled(false);
    REQUIRE(e.status()          == TournamentStatus::Active);
    REQUIRE(e.maxParticipants() == 32u);
    REQUIRE(e.isPrized());
    REQUIRE(!e.isEnabled());
}

TEST_CASE("TournamentEditor defaults", "[Editor][S127]") {
    TournamentEditor ed;
    REQUIRE(!ed.isShowCompleted());
    REQUIRE(!ed.isGroupByFormat());
    REQUIRE(ed.defaultMaxParticipants() == 32u);
    REQUIRE(ed.tournamentCount()        == 0u);
}

TEST_CASE("TournamentEditor add/remove tournaments", "[Editor][S127]") {
    TournamentEditor ed;
    REQUIRE(ed.addTournament(TournamentEntry(1, "t_a", TournamentFormat::SingleElim)));
    REQUIRE(ed.addTournament(TournamentEntry(2, "t_b", TournamentFormat::DoubleElim)));
    REQUIRE(ed.addTournament(TournamentEntry(3, "t_c", TournamentFormat::RoundRobin)));
    REQUIRE(!ed.addTournament(TournamentEntry(1, "t_a", TournamentFormat::SingleElim)));
    REQUIRE(ed.tournamentCount() == 3u);
    REQUIRE(ed.removeTournament(2));
    REQUIRE(ed.tournamentCount() == 2u);
    REQUIRE(!ed.removeTournament(99));
}

TEST_CASE("TournamentEditor counts and find", "[Editor][S127]") {
    TournamentEditor ed;
    TournamentEntry e1(1, "t_a", TournamentFormat::SingleElim);
    TournamentEntry e2(2, "t_b", TournamentFormat::SingleElim); e2.setStatus(TournamentStatus::Active);
    TournamentEntry e3(3, "t_c", TournamentFormat::RoundRobin); e3.setIsPrized(true);
    TournamentEntry e4(4, "t_d", TournamentFormat::Swiss);      e4.setIsPrized(true);
    ed.addTournament(e1); ed.addTournament(e2); ed.addTournament(e3); ed.addTournament(e4);
    REQUIRE(ed.countByFormat(TournamentFormat::SingleElim) == 2u);
    REQUIRE(ed.countByFormat(TournamentFormat::RoundRobin) == 1u);
    REQUIRE(ed.countByFormat(TournamentFormat::FFA)        == 0u);
    REQUIRE(ed.countByStatus(TournamentStatus::Draft)      == 3u);
    REQUIRE(ed.countByStatus(TournamentStatus::Active)     == 1u);
    REQUIRE(ed.countPrized()                               == 2u);
    auto* found = ed.findTournament(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->format() == TournamentFormat::RoundRobin);
    REQUIRE(ed.findTournament(99) == nullptr);
}

TEST_CASE("TournamentEditor settings mutation", "[Editor][S127]") {
    TournamentEditor ed;
    ed.setIsShowCompleted(true);
    ed.setIsGroupByFormat(true);
    ed.setDefaultMaxParticipants(64u);
    REQUIRE(ed.isShowCompleted());
    REQUIRE(ed.isGroupByFormat());
    REQUIRE(ed.defaultMaxParticipants() == 64u);
}
