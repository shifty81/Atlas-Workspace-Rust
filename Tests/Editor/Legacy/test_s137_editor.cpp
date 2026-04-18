// S137 editor tests: AIPathfindEditor, AIPerceptionEditor, AIDecisionEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AIDecisionEditor.h"
#include "NF/Editor/AIPerceptionEditor.h"
#include "NF/Editor/AIPathfindEditor.h"

using namespace NF;

// ── AIPathfindEditor ──────────────────────────────────────────────────────────

TEST_CASE("AiPathAlgo names", "[Editor][S137]") {
    REQUIRE(std::string(aiPathAlgoName(AiPathAlgo::AStar))     == "AStar");
    REQUIRE(std::string(aiPathAlgoName(AiPathAlgo::Dijkstra))  == "Dijkstra");
    REQUIRE(std::string(aiPathAlgoName(AiPathAlgo::NavMesh))   == "NavMesh");
    REQUIRE(std::string(aiPathAlgoName(AiPathAlgo::FlowField)) == "FlowField");
    REQUIRE(std::string(aiPathAlgoName(AiPathAlgo::Custom))    == "Custom");
}

TEST_CASE("AiPathHeuristic names", "[Editor][S137]") {
    REQUIRE(std::string(aiPathHeuristicName(AiPathHeuristic::Manhattan)) == "Manhattan");
    REQUIRE(std::string(aiPathHeuristicName(AiPathHeuristic::Euclidean)) == "Euclidean");
    REQUIRE(std::string(aiPathHeuristicName(AiPathHeuristic::Chebyshev)) == "Chebyshev");
    REQUIRE(std::string(aiPathHeuristicName(AiPathHeuristic::Octile))    == "Octile");
    REQUIRE(std::string(aiPathHeuristicName(AiPathHeuristic::Zero))      == "Zero");
}

TEST_CASE("AiPathfindConfig defaults", "[Editor][S137]") {
    AiPathfindConfig c(1, "astar_nav", AiPathAlgo::AStar, AiPathHeuristic::Euclidean);
    REQUIRE(c.id()             == 1u);
    REQUIRE(c.name()           == "astar_nav");
    REQUIRE(c.algo()           == AiPathAlgo::AStar);
    REQUIRE(c.heuristic()      == AiPathHeuristic::Euclidean);
    REQUIRE(c.maxSearchNodes() == 1024u);
    REQUIRE(c.agentRadius()    == 0.5f);
    REQUIRE(c.isEnabled());
}

TEST_CASE("AiPathfindConfig mutation", "[Editor][S137]") {
    AiPathfindConfig c(2, "flow_field", AiPathAlgo::FlowField, AiPathHeuristic::Zero);
    c.setMaxSearchNodes(2048u);
    c.setAgentRadius(1.0f);
    c.setIsEnabled(false);
    REQUIRE(c.maxSearchNodes() == 2048u);
    REQUIRE(c.agentRadius()    == 1.0f);
    REQUIRE(!c.isEnabled());
}

TEST_CASE("AIPathfindEditor defaults", "[Editor][S137]") {
    AIPathfindEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByAlgo());
    REQUIRE(ed.defaultMaxSearchNodes() == 512u);
    REQUIRE(ed.configCount()           == 0u);
}

TEST_CASE("AIPathfindEditor add/remove", "[Editor][S137]") {
    AIPathfindEditor ed;
    REQUIRE(ed.addConfig(AiPathfindConfig(1, "a", AiPathAlgo::AStar,     AiPathHeuristic::Manhattan)));
    REQUIRE(ed.addConfig(AiPathfindConfig(2, "b", AiPathAlgo::Dijkstra,  AiPathHeuristic::Euclidean)));
    REQUIRE(ed.addConfig(AiPathfindConfig(3, "c", AiPathAlgo::NavMesh,   AiPathHeuristic::Chebyshev)));
    REQUIRE(!ed.addConfig(AiPathfindConfig(1, "a", AiPathAlgo::AStar,    AiPathHeuristic::Manhattan)));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("AIPathfindEditor counts and find", "[Editor][S137]") {
    AIPathfindEditor ed;
    AiPathfindConfig c1(1, "a", AiPathAlgo::AStar,     AiPathHeuristic::Manhattan);
    AiPathfindConfig c2(2, "b", AiPathAlgo::AStar,     AiPathHeuristic::Euclidean);
    AiPathfindConfig c3(3, "c", AiPathAlgo::NavMesh,   AiPathHeuristic::Chebyshev);
    AiPathfindConfig c4(4, "d", AiPathAlgo::FlowField, AiPathHeuristic::Zero); c4.setIsEnabled(false);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countByAlgo(AiPathAlgo::AStar)              == 2u);
    REQUIRE(ed.countByAlgo(AiPathAlgo::NavMesh)            == 1u);
    REQUIRE(ed.countByAlgo(AiPathAlgo::Dijkstra)           == 0u);
    REQUIRE(ed.countByHeuristic(AiPathHeuristic::Manhattan) == 1u);
    REQUIRE(ed.countByHeuristic(AiPathHeuristic::Euclidean) == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->algo() == AiPathAlgo::NavMesh);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("AIPathfindEditor settings mutation", "[Editor][S137]") {
    AIPathfindEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByAlgo(false);
    ed.setDefaultMaxSearchNodes(256u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByAlgo());
    REQUIRE(ed.defaultMaxSearchNodes() == 256u);
}

// ── AIPerceptionEditor ────────────────────────────────────────────────────────

TEST_CASE("AiSensorType names", "[Editor][S137]") {
    REQUIRE(std::string(aiSensorTypeName(AiSensorType::Sight))   == "Sight");
    REQUIRE(std::string(aiSensorTypeName(AiSensorType::Hearing)) == "Hearing");
    REQUIRE(std::string(aiSensorTypeName(AiSensorType::Touch))   == "Touch");
    REQUIRE(std::string(aiSensorTypeName(AiSensorType::Smell))   == "Smell");
    REQUIRE(std::string(aiSensorTypeName(AiSensorType::Custom))  == "Custom");
}

TEST_CASE("AiSensorShape names", "[Editor][S137]") {
    REQUIRE(std::string(aiSensorShapeName(AiSensorShape::Cone))    == "Cone");
    REQUIRE(std::string(aiSensorShapeName(AiSensorShape::Sphere))  == "Sphere");
    REQUIRE(std::string(aiSensorShapeName(AiSensorShape::Box))     == "Box");
    REQUIRE(std::string(aiSensorShapeName(AiSensorShape::Capsule)) == "Capsule");
    REQUIRE(std::string(aiSensorShapeName(AiSensorShape::Frustum)) == "Frustum");
}

TEST_CASE("AiPerceptionConfig defaults", "[Editor][S137]") {
    AiPerceptionConfig c(1, "sight_cone", AiSensorType::Sight, AiSensorShape::Cone);
    REQUIRE(c.id()          == 1u);
    REQUIRE(c.name()        == "sight_cone");
    REQUIRE(c.sensorType()  == AiSensorType::Sight);
    REQUIRE(c.sensorShape() == AiSensorShape::Cone);
    REQUIRE(c.range()       == 10.0f);
    REQUIRE(c.fieldOfView() == 90.0f);
    REQUIRE(c.isEnabled());
}

TEST_CASE("AiPerceptionConfig mutation", "[Editor][S137]") {
    AiPerceptionConfig c(2, "hearing_sphere", AiSensorType::Hearing, AiSensorShape::Sphere);
    c.setRange(20.0f);
    c.setFieldOfView(360.0f);
    c.setIsEnabled(false);
    REQUIRE(c.range()       == 20.0f);
    REQUIRE(c.fieldOfView() == 360.0f);
    REQUIRE(!c.isEnabled());
}

TEST_CASE("AIPerceptionEditor defaults", "[Editor][S137]") {
    AIPerceptionEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupBySensorType());
    REQUIRE(ed.defaultRange() == 15.0f);
    REQUIRE(ed.configCount()  == 0u);
}

TEST_CASE("AIPerceptionEditor add/remove", "[Editor][S137]") {
    AIPerceptionEditor ed;
    REQUIRE(ed.addConfig(AiPerceptionConfig(1, "a", AiSensorType::Sight,   AiSensorShape::Cone)));
    REQUIRE(ed.addConfig(AiPerceptionConfig(2, "b", AiSensorType::Hearing, AiSensorShape::Sphere)));
    REQUIRE(ed.addConfig(AiPerceptionConfig(3, "c", AiSensorType::Touch,   AiSensorShape::Capsule)));
    REQUIRE(!ed.addConfig(AiPerceptionConfig(1, "a", AiSensorType::Sight,  AiSensorShape::Cone)));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("AIPerceptionEditor counts and find", "[Editor][S137]") {
    AIPerceptionEditor ed;
    AiPerceptionConfig c1(1, "a", AiSensorType::Sight,   AiSensorShape::Cone);
    AiPerceptionConfig c2(2, "b", AiSensorType::Sight,   AiSensorShape::Frustum);
    AiPerceptionConfig c3(3, "c", AiSensorType::Hearing, AiSensorShape::Sphere);
    AiPerceptionConfig c4(4, "d", AiSensorType::Custom,  AiSensorShape::Box); c4.setIsEnabled(false);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countBySensorType(AiSensorType::Sight)      == 2u);
    REQUIRE(ed.countBySensorType(AiSensorType::Hearing)    == 1u);
    REQUIRE(ed.countBySensorType(AiSensorType::Touch)      == 0u);
    REQUIRE(ed.countBySensorShape(AiSensorShape::Cone)     == 1u);
    REQUIRE(ed.countBySensorShape(AiSensorShape::Frustum)  == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->sensorType() == AiSensorType::Hearing);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("AIPerceptionEditor settings mutation", "[Editor][S137]") {
    AIPerceptionEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupBySensorType(false);
    ed.setDefaultRange(25.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupBySensorType());
    REQUIRE(ed.defaultRange() == 25.0f);
}

// ── AIDecisionEditor ──────────────────────────────────────────────────────────

TEST_CASE("AiDecisionModel names", "[Editor][S137]") {
    REQUIRE(std::string(aiDecisionModelName(AiDecisionModel::BehaviorTree)) == "BehaviorTree");
    REQUIRE(std::string(aiDecisionModelName(AiDecisionModel::UtilityAI))    == "UtilityAI");
    REQUIRE(std::string(aiDecisionModelName(AiDecisionModel::FSM))          == "FSM");
    REQUIRE(std::string(aiDecisionModelName(AiDecisionModel::GOAP))         == "GOAP");
    REQUIRE(std::string(aiDecisionModelName(AiDecisionModel::Custom))       == "Custom");
}

TEST_CASE("AiDecisionPriority names", "[Editor][S137]") {
    REQUIRE(std::string(aiDecisionPriorityName(AiDecisionPriority::Low))      == "Low");
    REQUIRE(std::string(aiDecisionPriorityName(AiDecisionPriority::Normal))   == "Normal");
    REQUIRE(std::string(aiDecisionPriorityName(AiDecisionPriority::High))     == "High");
    REQUIRE(std::string(aiDecisionPriorityName(AiDecisionPriority::Critical)) == "Critical");
    REQUIRE(std::string(aiDecisionPriorityName(AiDecisionPriority::Override)) == "Override");
}

TEST_CASE("AiDecisionConfig defaults", "[Editor][S137]") {
    AiDecisionConfig c(1, "bt_main", AiDecisionModel::BehaviorTree, AiDecisionPriority::Normal);
    REQUIRE(c.id()                 == 1u);
    REQUIRE(c.name()               == "bt_main");
    REQUIRE(c.model()              == AiDecisionModel::BehaviorTree);
    REQUIRE(c.priority()           == AiDecisionPriority::Normal);
    REQUIRE(c.tickRateHz()         == 10u);
    REQUIRE(c.maxConcurrentGoals() == 3u);
    REQUIRE(c.isEnabled());
}

TEST_CASE("AiDecisionConfig mutation", "[Editor][S137]") {
    AiDecisionConfig c(2, "utility_high", AiDecisionModel::UtilityAI, AiDecisionPriority::High);
    c.setTickRateHz(30u);
    c.setMaxConcurrentGoals(5u);
    c.setIsEnabled(false);
    REQUIRE(c.tickRateHz()         == 30u);
    REQUIRE(c.maxConcurrentGoals() == 5u);
    REQUIRE(!c.isEnabled());
}

TEST_CASE("AIDecisionEditor defaults", "[Editor][S137]") {
    AIDecisionEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByModel());
    REQUIRE(ed.defaultTickRateHz() == 20u);
    REQUIRE(ed.configCount()       == 0u);
}

TEST_CASE("AIDecisionEditor add/remove", "[Editor][S137]") {
    AIDecisionEditor ed;
    REQUIRE(ed.addConfig(AiDecisionConfig(1, "a", AiDecisionModel::BehaviorTree, AiDecisionPriority::Low)));
    REQUIRE(ed.addConfig(AiDecisionConfig(2, "b", AiDecisionModel::UtilityAI,    AiDecisionPriority::Normal)));
    REQUIRE(ed.addConfig(AiDecisionConfig(3, "c", AiDecisionModel::GOAP,         AiDecisionPriority::High)));
    REQUIRE(!ed.addConfig(AiDecisionConfig(1, "a", AiDecisionModel::BehaviorTree, AiDecisionPriority::Low)));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("AIDecisionEditor counts and find", "[Editor][S137]") {
    AIDecisionEditor ed;
    AiDecisionConfig c1(1, "a", AiDecisionModel::BehaviorTree, AiDecisionPriority::Low);
    AiDecisionConfig c2(2, "b", AiDecisionModel::BehaviorTree, AiDecisionPriority::Normal);
    AiDecisionConfig c3(3, "c", AiDecisionModel::FSM,          AiDecisionPriority::High);
    AiDecisionConfig c4(4, "d", AiDecisionModel::Custom,       AiDecisionPriority::Critical); c4.setIsEnabled(false);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countByModel(AiDecisionModel::BehaviorTree)      == 2u);
    REQUIRE(ed.countByModel(AiDecisionModel::FSM)               == 1u);
    REQUIRE(ed.countByModel(AiDecisionModel::UtilityAI)         == 0u);
    REQUIRE(ed.countByPriority(AiDecisionPriority::Low)         == 1u);
    REQUIRE(ed.countByPriority(AiDecisionPriority::Normal)      == 1u);
    REQUIRE(ed.countEnabled()                                   == 3u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->model() == AiDecisionModel::FSM);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("AIDecisionEditor settings mutation", "[Editor][S137]") {
    AIDecisionEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByModel(false);
    ed.setDefaultTickRateHz(60u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByModel());
    REQUIRE(ed.defaultTickRateHz() == 60u);
}
