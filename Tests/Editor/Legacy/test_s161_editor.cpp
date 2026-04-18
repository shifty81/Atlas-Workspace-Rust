// S161 editor tests: AINavMeshEditorV1, AISpawnEditorV1, AIGoalEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AIGoalEditorV1.h"
#include "NF/Editor/AISpawnEditorV1.h"
#include "NF/Editor/AINavMeshEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── AINavMeshEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Anv1AgentConfig validity", "[Editor][S161]") {
    Anv1AgentConfig cfg;
    REQUIRE(cfg.isValid());
    cfg.radius = 0.f;
    REQUIRE(!cfg.isValid());
}

TEST_CASE("Anv1Zone validity", "[Editor][S161]") {
    Anv1Zone z;
    REQUIRE(!z.isValid());
    z.id = 1; z.name = "MainArea";
    REQUIRE(z.isValid());
}

TEST_CASE("AINavMeshEditorV1 addZone and zoneCount", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    REQUIRE(nm.zoneCount() == 0);
    Anv1Zone z; z.id = 1; z.name = "Floor";
    REQUIRE(nm.addZone(z));
    REQUIRE(nm.zoneCount() == 1);
}

TEST_CASE("AINavMeshEditorV1 addZone invalid fails", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    Anv1Zone z;
    REQUIRE(!nm.addZone(z));
}

TEST_CASE("AINavMeshEditorV1 addZone duplicate id fails", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    Anv1Zone z; z.id = 1; z.name = "A";
    nm.addZone(z);
    REQUIRE(!nm.addZone(z));
}

TEST_CASE("AINavMeshEditorV1 removeZone", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    Anv1Zone z; z.id = 2; z.name = "Bridge";
    nm.addZone(z);
    REQUIRE(nm.removeZone(2));
    REQUIRE(nm.zoneCount() == 0);
    REQUIRE(!nm.removeZone(2));
}

TEST_CASE("AINavMeshEditorV1 setAgentConfig valid", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    Anv1AgentConfig cfg; cfg.radius = 0.5f; cfg.height = 2.f;
    REQUIRE(nm.setAgentConfig(cfg));
    REQUIRE(nm.agentConfig().radius == Approx(0.5f));
}

TEST_CASE("AINavMeshEditorV1 setAgentConfig invalid fails", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    Anv1AgentConfig cfg; cfg.radius = 0.f;
    REQUIRE(!nm.setAgentConfig(cfg));
}

TEST_CASE("AINavMeshEditorV1 startBake and bakeStatus", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    REQUIRE(nm.bakeStatus() == Anv1BakeStatus::Idle);
    REQUIRE(nm.startBake());
    REQUIRE(nm.bakeStatus() == Anv1BakeStatus::Baking);
}

TEST_CASE("AINavMeshEditorV1 startBake while baking fails", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    nm.startBake();
    REQUIRE(!nm.startBake());
}

TEST_CASE("AINavMeshEditorV1 finishBake success", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    nm.startBake();
    REQUIRE(nm.finishBake(true));
    REQUIRE(nm.bakeStatus() == Anv1BakeStatus::Done);
}

TEST_CASE("AINavMeshEditorV1 finishBake failure", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    nm.startBake();
    REQUIRE(nm.finishBake(false));
    REQUIRE(nm.bakeStatus() == Anv1BakeStatus::Failed);
}

TEST_CASE("AINavMeshEditorV1 finishBake not baking fails", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    REQUIRE(!nm.finishBake(true));
}

TEST_CASE("AINavMeshEditorV1 cancelBake", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    nm.startBake();
    REQUIRE(nm.cancelBake());
    REQUIRE(nm.bakeStatus() == Anv1BakeStatus::Idle);
}

TEST_CASE("AINavMeshEditorV1 cancelBake not baking fails", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    REQUIRE(!nm.cancelBake());
}

TEST_CASE("AINavMeshEditorV1 onBake callback fires", "[Editor][S161]") {
    AINavMeshEditorV1 nm;
    Anv1BakeStatus reported = Anv1BakeStatus::Idle;
    nm.setOnBake([&](Anv1BakeStatus s){ reported = s; });
    nm.startBake();
    REQUIRE(reported == Anv1BakeStatus::Baking);
    nm.finishBake(true);
    REQUIRE(reported == Anv1BakeStatus::Done);
}

// ── AISpawnEditorV1 ───────────────────────────────────────────────────────────

TEST_CASE("Asv1SpawnPoint validity", "[Editor][S161]") {
    Asv1SpawnPoint sp;
    REQUIRE(!sp.isValid());
    sp.id = 1; sp.name = "PointA";
    REQUIRE(sp.isValid());
}

TEST_CASE("Asv1Wave validity", "[Editor][S161]") {
    Asv1Wave w;
    REQUIRE(!w.isValid());
    w.id = 1; w.label = "Wave1"; w.count = 3;
    REQUIRE(w.isValid());
}

TEST_CASE("AISpawnEditorV1 addSpawnPoint and spawnPointCount", "[Editor][S161]") {
    AISpawnEditorV1 se;
    REQUIRE(se.spawnPointCount() == 0);
    Asv1SpawnPoint sp; sp.id = 1; sp.name = "Spawn1";
    REQUIRE(se.addSpawnPoint(sp));
    REQUIRE(se.spawnPointCount() == 1);
}

TEST_CASE("AISpawnEditorV1 addSpawnPoint invalid fails", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1SpawnPoint sp;
    REQUIRE(!se.addSpawnPoint(sp));
}

TEST_CASE("AISpawnEditorV1 addSpawnPoint duplicate id fails", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1SpawnPoint sp; sp.id = 1; sp.name = "A";
    se.addSpawnPoint(sp);
    REQUIRE(!se.addSpawnPoint(sp));
}

TEST_CASE("AISpawnEditorV1 removeSpawnPoint", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1SpawnPoint sp; sp.id = 2; sp.name = "B";
    se.addSpawnPoint(sp);
    REQUIRE(se.removeSpawnPoint(2));
    REQUIRE(se.spawnPointCount() == 0);
    REQUIRE(!se.removeSpawnPoint(2));
}

TEST_CASE("AISpawnEditorV1 addWave and waveCount", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1Wave w; w.id = 1; w.label = "W1"; w.count = 5;
    REQUIRE(se.addWave(w));
    REQUIRE(se.waveCount() == 1);
}

TEST_CASE("AISpawnEditorV1 addWave invalid count fails", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1Wave w; w.id = 1; w.label = "W"; w.count = 0;
    REQUIRE(!se.addWave(w));
}

TEST_CASE("AISpawnEditorV1 removeWave", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1Wave w; w.id = 2; w.label = "W2"; w.count = 1;
    se.addWave(w);
    REQUIRE(se.removeWave(2));
    REQUIRE(se.waveCount() == 0);
    REQUIRE(!se.removeWave(2));
}

TEST_CASE("AISpawnEditorV1 triggerWave fires callback", "[Editor][S161]") {
    AISpawnEditorV1 se;
    Asv1Wave w; w.id = 3; w.label = "W3"; w.count = 2;
    se.addWave(w);
    uint64_t triggered = 0;
    se.setOnSpawn([&](const Asv1Wave& wv){ triggered = wv.id; });
    REQUIRE(se.triggerWave(3));
    REQUIRE(triggered == 3);
}

TEST_CASE("AISpawnEditorV1 triggerWave unknown id fails", "[Editor][S161]") {
    AISpawnEditorV1 se;
    REQUIRE(!se.triggerWave(99));
}

// ── AIGoalEditorV1 ────────────────────────────────────────────────────────────

TEST_CASE("Agv1Scorer validity", "[Editor][S161]") {
    Agv1Scorer sc;
    REQUIRE(!sc.isValid());
    sc.id = 1; sc.name = "HealthScore";
    REQUIRE(sc.isValid());
}

TEST_CASE("Agv1Goal validity", "[Editor][S161]") {
    Agv1Goal g;
    REQUIRE(!g.isValid());
    g.id = 1; g.label = "Attack";
    REQUIRE(g.isValid());
}

TEST_CASE("AIGoalEditorV1 addGoal and goalCount", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    REQUIRE(ge.goalCount() == 0);
    Agv1Goal g; g.id = 1; g.label = "Patrol";
    REQUIRE(ge.addGoal(g));
    REQUIRE(ge.goalCount() == 1);
}

TEST_CASE("AIGoalEditorV1 addGoal invalid fails", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g;
    REQUIRE(!ge.addGoal(g));
}

TEST_CASE("AIGoalEditorV1 addGoal duplicate id fails", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 1; g.label = "A";
    ge.addGoal(g);
    REQUIRE(!ge.addGoal(g));
}

TEST_CASE("AIGoalEditorV1 removeGoal", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 2; g.label = "Flee";
    ge.addGoal(g);
    REQUIRE(ge.removeGoal(2));
    REQUIRE(ge.goalCount() == 0);
    REQUIRE(!ge.removeGoal(2));
}

TEST_CASE("AIGoalEditorV1 setGoalStatus", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 3; g.label = "Chase";
    ge.addGoal(g);
    REQUIRE(ge.setGoalStatus(3, Agv1GoalStatus::Active));
    auto* found = ge.findGoal(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->status == Agv1GoalStatus::Active);
}

TEST_CASE("AIGoalEditorV1 setGoalStatus unknown id fails", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    REQUIRE(!ge.setGoalStatus(99, Agv1GoalStatus::Failed));
}

TEST_CASE("AIGoalEditorV1 setGoalStatus fires callback", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 4; g.label = "Cover";
    ge.addGoal(g);
    uint64_t cbId = 0;
    Agv1GoalStatus cbStatus = Agv1GoalStatus::Idle;
    ge.setOnStatus([&](uint64_t id, Agv1GoalStatus s){ cbId = id; cbStatus = s; });
    ge.setGoalStatus(4, Agv1GoalStatus::Completed);
    REQUIRE(cbId == 4);
    REQUIRE(cbStatus == Agv1GoalStatus::Completed);
}

TEST_CASE("AIGoalEditorV1 addScorer", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 5; g.label = "Retreat";
    ge.addGoal(g);
    Agv1Scorer sc; sc.id = 1; sc.name = "DangerScore";
    REQUIRE(ge.addScorer(5, sc));
    auto* found = ge.findGoal(5);
    REQUIRE(found != nullptr);
    REQUIRE(found->scorers.size() == 1);
}

TEST_CASE("AIGoalEditorV1 addScorer invalid fails", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 6; g.label = "Idle";
    ge.addGoal(g);
    Agv1Scorer sc;
    REQUIRE(!ge.addScorer(6, sc));
}

TEST_CASE("AIGoalEditorV1 addScorer unknown goal fails", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Scorer sc; sc.id = 1; sc.name = "X";
    REQUIRE(!ge.addScorer(99, sc));
}

TEST_CASE("AIGoalEditorV1 enableGoal", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    Agv1Goal g; g.id = 7; g.label = "Defend";
    ge.addGoal(g);
    REQUIRE(ge.enableGoal(7, false));
    auto* found = ge.findGoal(7);
    REQUIRE(found != nullptr);
    REQUIRE(!found->enabled);
}

TEST_CASE("AIGoalEditorV1 enableGoal unknown id fails", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    REQUIRE(!ge.enableGoal(99, false));
}

TEST_CASE("AIGoalEditorV1 findGoal unknown returns null", "[Editor][S161]") {
    AIGoalEditorV1 ge;
    REQUIRE(ge.findGoal(999) == nullptr);
}
