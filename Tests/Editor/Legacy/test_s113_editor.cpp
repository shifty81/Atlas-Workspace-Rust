// S113 editor tests: MiniGameEditor, PuzzleEditor, ArcadeGameEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ArcadeGameEditor.h"
#include "NF/Editor/PuzzleEditor.h"
#include "NF/Editor/MiniGameEditor.h"

using namespace NF;

// ── MiniGameEditor ───────────────────────────────────────────────────────────

TEST_CASE("MiniGameType names", "[Editor][S113]") {
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Racing))     == "Racing");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Shooting))   == "Shooting");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Fishing))    == "Fishing");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Cooking))    == "Cooking");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Dancing))    == "Dancing");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Memory))     == "Memory");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Crafting))   == "Crafting");
    REQUIRE(std::string(miniGameTypeName(MiniGameType::Platformer)) == "Platformer");
}

TEST_CASE("MiniGameDifficulty names", "[Editor][S113]") {
    REQUIRE(std::string(miniGameDifficultyName(MiniGameDifficulty::VeryEasy)) == "VeryEasy");
    REQUIRE(std::string(miniGameDifficultyName(MiniGameDifficulty::Easy))     == "Easy");
    REQUIRE(std::string(miniGameDifficultyName(MiniGameDifficulty::Normal))   == "Normal");
    REQUIRE(std::string(miniGameDifficultyName(MiniGameDifficulty::Hard))     == "Hard");
    REQUIRE(std::string(miniGameDifficultyName(MiniGameDifficulty::VeryHard)) == "VeryHard");
}

TEST_CASE("MiniGameTrigger names", "[Editor][S113]") {
    REQUIRE(std::string(miniGameTriggerName(MiniGameTrigger::Manual)) == "Manual");
    REQUIRE(std::string(miniGameTriggerName(MiniGameTrigger::NPC))    == "NPC");
    REQUIRE(std::string(miniGameTriggerName(MiniGameTrigger::Zone))   == "Zone");
    REQUIRE(std::string(miniGameTriggerName(MiniGameTrigger::Item))   == "Item");
    REQUIRE(std::string(miniGameTriggerName(MiniGameTrigger::Event))  == "Event");
    REQUIRE(std::string(miniGameTriggerName(MiniGameTrigger::Timer))  == "Timer");
}

TEST_CASE("MiniGame defaults", "[Editor][S113]") {
    MiniGame g(1, "fish_challenge", MiniGameType::Fishing);
    REQUIRE(g.id()           == 1u);
    REQUIRE(g.name()         == "fish_challenge");
    REQUIRE(g.type()         == MiniGameType::Fishing);
    REQUIRE(g.difficulty()   == MiniGameDifficulty::Normal);
    REQUIRE(g.trigger()      == MiniGameTrigger::Manual);
    REQUIRE(g.timeLimit()    == 60.0f);
    REQUIRE(g.isRepeatable());
    REQUIRE(g.isOptional());
}

TEST_CASE("MiniGame mutation", "[Editor][S113]") {
    MiniGame g(2, "boss_race", MiniGameType::Racing);
    g.setDifficulty(MiniGameDifficulty::Hard);
    g.setTrigger(MiniGameTrigger::NPC);
    g.setTimeLimit(120.0f);
    g.setRepeatable(false);
    g.setOptional(false);
    REQUIRE(g.difficulty()   == MiniGameDifficulty::Hard);
    REQUIRE(g.trigger()      == MiniGameTrigger::NPC);
    REQUIRE(g.timeLimit()    == 120.0f);
    REQUIRE(!g.isRepeatable());
    REQUIRE(!g.isOptional());
}

TEST_CASE("MiniGameEditor defaults", "[Editor][S113]") {
    MiniGameEditor ed;
    REQUIRE(ed.isShowBounds());
    REQUIRE(ed.isShowTriggers());
    REQUIRE(ed.previewScale() == 1.0f);
    REQUIRE(ed.gameCount()    == 0u);
}

TEST_CASE("MiniGameEditor add/remove games", "[Editor][S113]") {
    MiniGameEditor ed;
    REQUIRE(ed.addGame(MiniGame(1, "race1",   MiniGameType::Racing)));
    REQUIRE(ed.addGame(MiniGame(2, "fish1",   MiniGameType::Fishing)));
    REQUIRE(ed.addGame(MiniGame(3, "dance1",  MiniGameType::Dancing)));
    REQUIRE(!ed.addGame(MiniGame(1, "race1",  MiniGameType::Racing)));
    REQUIRE(ed.gameCount() == 3u);
    REQUIRE(ed.removeGame(2));
    REQUIRE(ed.gameCount() == 2u);
    REQUIRE(!ed.removeGame(99));
}

TEST_CASE("MiniGameEditor counts and find", "[Editor][S113]") {
    MiniGameEditor ed;
    MiniGame g1(1, "race_a",  MiniGameType::Racing);
    MiniGame g2(2, "race_b",  MiniGameType::Racing);  g2.setDifficulty(MiniGameDifficulty::Hard);
    MiniGame g3(3, "fish_a",  MiniGameType::Fishing); g3.setRepeatable(false);
    MiniGame g4(4, "cook_a",  MiniGameType::Cooking); g4.setDifficulty(MiniGameDifficulty::Hard);
    ed.addGame(g1); ed.addGame(g2); ed.addGame(g3); ed.addGame(g4);
    REQUIRE(ed.countByType(MiniGameType::Racing)          == 2u);
    REQUIRE(ed.countByType(MiniGameType::Fishing)         == 1u);
    REQUIRE(ed.countByType(MiniGameType::Memory)          == 0u);
    REQUIRE(ed.countByDifficulty(MiniGameDifficulty::Normal) == 2u);
    REQUIRE(ed.countByDifficulty(MiniGameDifficulty::Hard)   == 2u);
    REQUIRE(ed.countRepeatable()                          == 3u);
    auto* found = ed.findGame(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == MiniGameType::Fishing);
    REQUIRE(ed.findGame(99) == nullptr);
}

TEST_CASE("MiniGameEditor settings mutation", "[Editor][S113]") {
    MiniGameEditor ed;
    ed.setShowBounds(false);
    ed.setShowTriggers(false);
    ed.setPreviewScale(2.0f);
    REQUIRE(!ed.isShowBounds());
    REQUIRE(!ed.isShowTriggers());
    REQUIRE(ed.previewScale() == 2.0f);
}

// ── PuzzleEditor ─────────────────────────────────────────────────────────────

TEST_CASE("PuzzleType names", "[Editor][S113]") {
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Slider))      == "Slider");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Jigsaw))      == "Jigsaw");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Logic))       == "Logic");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Sequence))    == "Sequence");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Pattern))     == "Pattern");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Password))    == "Password");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Physics))     == "Physics");
    REQUIRE(std::string(puzzleTypeName(PuzzleType::Combination)) == "Combination");
}

TEST_CASE("PuzzleSolutionType names", "[Editor][S113]") {
    REQUIRE(std::string(puzzleSolutionTypeName(PuzzleSolutionType::Single))      == "Single");
    REQUIRE(std::string(puzzleSolutionTypeName(PuzzleSolutionType::Multiple))    == "Multiple");
    REQUIRE(std::string(puzzleSolutionTypeName(PuzzleSolutionType::Partial))     == "Partial");
    REQUIRE(std::string(puzzleSolutionTypeName(PuzzleSolutionType::Progressive)) == "Progressive");
    REQUIRE(std::string(puzzleSolutionTypeName(PuzzleSolutionType::Timed))       == "Timed");
}

TEST_CASE("PuzzleHintMode names", "[Editor][S113]") {
    REQUIRE(std::string(puzzleHintModeName(PuzzleHintMode::None))      == "None");
    REQUIRE(std::string(puzzleHintModeName(PuzzleHintMode::OnRequest)) == "OnRequest");
    REQUIRE(std::string(puzzleHintModeName(PuzzleHintMode::Automatic)) == "Automatic");
    REQUIRE(std::string(puzzleHintModeName(PuzzleHintMode::Penalty))   == "Penalty");
}

TEST_CASE("PuzzleDefinition defaults", "[Editor][S113]") {
    PuzzleDefinition pd(1, "slide_box", PuzzleType::Slider);
    REQUIRE(pd.id()           == 1u);
    REQUIRE(pd.name()         == "slide_box");
    REQUIRE(pd.type()         == PuzzleType::Slider);
    REQUIRE(pd.solutionType() == PuzzleSolutionType::Single);
    REQUIRE(pd.hintMode()     == PuzzleHintMode::OnRequest);
    REQUIRE(pd.maxAttempts()  == 3u);
    REQUIRE(!pd.isSkippable());
    REQUIRE(pd.pointValue()   == 100u);
}

TEST_CASE("PuzzleDefinition mutation", "[Editor][S113]") {
    PuzzleDefinition pd(2, "combo_lock", PuzzleType::Combination);
    pd.setSolutionType(PuzzleSolutionType::Progressive);
    pd.setHintMode(PuzzleHintMode::Penalty);
    pd.setMaxAttempts(5u);
    pd.setSkippable(true);
    pd.setPointValue(200u);
    REQUIRE(pd.solutionType() == PuzzleSolutionType::Progressive);
    REQUIRE(pd.hintMode()     == PuzzleHintMode::Penalty);
    REQUIRE(pd.maxAttempts()  == 5u);
    REQUIRE(pd.isSkippable());
    REQUIRE(pd.pointValue()   == 200u);
}

TEST_CASE("PuzzleEditor defaults", "[Editor][S113]") {
    PuzzleEditor ed;
    REQUIRE(!ed.isShowSolution());
    REQUIRE(ed.isShowHints());
    REQUIRE(ed.gridSnap()    == 1.0f);
    REQUIRE(ed.puzzleCount() == 0u);
}

TEST_CASE("PuzzleEditor add/remove puzzles", "[Editor][S113]") {
    PuzzleEditor ed;
    REQUIRE(ed.addPuzzle(PuzzleDefinition(1, "slider1", PuzzleType::Slider)));
    REQUIRE(ed.addPuzzle(PuzzleDefinition(2, "logic1",  PuzzleType::Logic)));
    REQUIRE(ed.addPuzzle(PuzzleDefinition(3, "combo1",  PuzzleType::Combination)));
    REQUIRE(!ed.addPuzzle(PuzzleDefinition(1, "slider1", PuzzleType::Slider)));
    REQUIRE(ed.puzzleCount() == 3u);
    REQUIRE(ed.removePuzzle(2));
    REQUIRE(ed.puzzleCount() == 2u);
    REQUIRE(!ed.removePuzzle(99));
}

TEST_CASE("PuzzleEditor counts and find", "[Editor][S113]") {
    PuzzleEditor ed;
    PuzzleDefinition p1(1, "slider_a", PuzzleType::Slider);
    PuzzleDefinition p2(2, "slider_b", PuzzleType::Slider); p2.setHintMode(PuzzleHintMode::Automatic);
    PuzzleDefinition p3(3, "logic_a",  PuzzleType::Logic);  p3.setSkippable(true);
    PuzzleDefinition p4(4, "combo_a",  PuzzleType::Combination); p4.setSkippable(true); p4.setHintMode(PuzzleHintMode::Penalty);
    ed.addPuzzle(p1); ed.addPuzzle(p2); ed.addPuzzle(p3); ed.addPuzzle(p4);
    REQUIRE(ed.countByType(PuzzleType::Slider)            == 2u);
    REQUIRE(ed.countByType(PuzzleType::Logic)             == 1u);
    REQUIRE(ed.countByType(PuzzleType::Physics)           == 0u);
    REQUIRE(ed.countByHintMode(PuzzleHintMode::OnRequest) == 2u);
    REQUIRE(ed.countByHintMode(PuzzleHintMode::Automatic) == 1u);
    REQUIRE(ed.countSkippable()                           == 2u);
    auto* found = ed.findPuzzle(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == PuzzleType::Logic);
    REQUIRE(ed.findPuzzle(99) == nullptr);
}

TEST_CASE("PuzzleEditor settings mutation", "[Editor][S113]") {
    PuzzleEditor ed;
    ed.setShowSolution(true);
    ed.setShowHints(false);
    ed.setGridSnap(0.5f);
    REQUIRE(ed.isShowSolution());
    REQUIRE(!ed.isShowHints());
    REQUIRE(ed.gridSnap() == 0.5f);
}

// ── ArcadeGameEditor ─────────────────────────────────────────────────────────

TEST_CASE("ArcadeGameGenre names", "[Editor][S113]") {
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Shooter))    == "Shooter");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Platformer)) == "Platformer");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Puzzle))     == "Puzzle");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Fighting))   == "Fighting");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Racing))     == "Racing");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Sports))     == "Sports");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::RPG))        == "RPG");
    REQUIRE(std::string(arcadeGameGenreName(ArcadeGameGenre::Survival))   == "Survival");
}

TEST_CASE("ArcadeControlScheme names", "[Editor][S113]") {
    REQUIRE(std::string(arcadeControlSchemeName(ArcadeControlScheme::Keyboard)) == "Keyboard");
    REQUIRE(std::string(arcadeControlSchemeName(ArcadeControlScheme::Gamepad))  == "Gamepad");
    REQUIRE(std::string(arcadeControlSchemeName(ArcadeControlScheme::Touch))    == "Touch");
    REQUIRE(std::string(arcadeControlSchemeName(ArcadeControlScheme::Motion))   == "Motion");
    REQUIRE(std::string(arcadeControlSchemeName(ArcadeControlScheme::Combined)) == "Combined");
}

TEST_CASE("ArcadeScoreMode names", "[Editor][S113]") {
    REQUIRE(std::string(arcadeScoreModeName(ArcadeScoreMode::Points))   == "Points");
    REQUIRE(std::string(arcadeScoreModeName(ArcadeScoreMode::Time))     == "Time");
    REQUIRE(std::string(arcadeScoreModeName(ArcadeScoreMode::Combo))    == "Combo");
    REQUIRE(std::string(arcadeScoreModeName(ArcadeScoreMode::Survival)) == "Survival");
}

TEST_CASE("ArcadeConfig defaults", "[Editor][S113]") {
    ArcadeConfig cfg(1, "space_shooter", ArcadeGameGenre::Shooter);
    REQUIRE(cfg.id()                 == 1u);
    REQUIRE(cfg.name()               == "space_shooter");
    REQUIRE(cfg.genre()              == ArcadeGameGenre::Shooter);
    REQUIRE(cfg.controlScheme()      == ArcadeControlScheme::Gamepad);
    REQUIRE(cfg.scoreMode()          == ArcadeScoreMode::Points);
    REQUIRE(cfg.lives()              == 3u);
    REQUIRE(cfg.isHighScoreEnabled());
    REQUIRE(!cfg.isTwoPlayer());
}

TEST_CASE("ArcadeConfig mutation", "[Editor][S113]") {
    ArcadeConfig cfg(2, "fighting_arena", ArcadeGameGenre::Fighting);
    cfg.setControlScheme(ArcadeControlScheme::Keyboard);
    cfg.setScoreMode(ArcadeScoreMode::Combo);
    cfg.setLives(5u);
    cfg.setHighScoreEnabled(false);
    cfg.setTwoPlayer(true);
    REQUIRE(cfg.controlScheme()      == ArcadeControlScheme::Keyboard);
    REQUIRE(cfg.scoreMode()          == ArcadeScoreMode::Combo);
    REQUIRE(cfg.lives()              == 5u);
    REQUIRE(!cfg.isHighScoreEnabled());
    REQUIRE(cfg.isTwoPlayer());
}

TEST_CASE("ArcadeGameEditor defaults", "[Editor][S113]") {
    ArcadeGameEditor ed;
    REQUIRE(!ed.isShowHitboxes());
    REQUIRE(ed.isShowSpawnPoints());
    REQUIRE(ed.simulationSpeed() == 1.0f);
    REQUIRE(ed.configCount()     == 0u);
}

TEST_CASE("ArcadeGameEditor add/remove configs", "[Editor][S113]") {
    ArcadeGameEditor ed;
    REQUIRE(ed.addConfig(ArcadeConfig(1, "shooter1",  ArcadeGameGenre::Shooter)));
    REQUIRE(ed.addConfig(ArcadeConfig(2, "racing1",   ArcadeGameGenre::Racing)));
    REQUIRE(ed.addConfig(ArcadeConfig(3, "platformer1", ArcadeGameGenre::Platformer)));
    REQUIRE(!ed.addConfig(ArcadeConfig(1, "shooter1", ArcadeGameGenre::Shooter)));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("ArcadeGameEditor counts and find", "[Editor][S113]") {
    ArcadeGameEditor ed;
    ArcadeConfig c1(1, "shooter_a", ArcadeGameGenre::Shooter);
    ArcadeConfig c2(2, "shooter_b", ArcadeGameGenre::Shooter); c2.setControlScheme(ArcadeControlScheme::Keyboard);
    ArcadeConfig c3(3, "racing_a",  ArcadeGameGenre::Racing);  c3.setTwoPlayer(true);
    ArcadeConfig c4(4, "fight_a",   ArcadeGameGenre::Fighting); c4.setTwoPlayer(true); c4.setControlScheme(ArcadeControlScheme::Keyboard);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countByGenre(ArcadeGameGenre::Shooter)                   == 2u);
    REQUIRE(ed.countByGenre(ArcadeGameGenre::Racing)                    == 1u);
    REQUIRE(ed.countByGenre(ArcadeGameGenre::Survival)                  == 0u);
    REQUIRE(ed.countByControlScheme(ArcadeControlScheme::Gamepad)       == 2u);
    REQUIRE(ed.countByControlScheme(ArcadeControlScheme::Keyboard)      == 2u);
    REQUIRE(ed.countTwoPlayer()                                         == 2u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->genre() == ArcadeGameGenre::Racing);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("ArcadeGameEditor settings mutation", "[Editor][S113]") {
    ArcadeGameEditor ed;
    ed.setShowHitboxes(true);
    ed.setShowSpawnPoints(false);
    ed.setSimulationSpeed(0.5f);
    REQUIRE(ed.isShowHitboxes());
    REQUIRE(!ed.isShowSpawnPoints());
    REQUIRE(ed.simulationSpeed() == 0.5f);
}
