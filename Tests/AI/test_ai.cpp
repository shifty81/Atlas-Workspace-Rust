#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/AI/AI.h"

using Catch::Matchers::WithinAbs;

// ── Blackboard ──────────────────────────────────────────────────

TEST_CASE("Blackboard set and get values", "[AI][Blackboard]") {
    NF::Blackboard bb;

    bb.set("health", 100);
    bb.set("speed", 3.5f);
    bb.set("alive", true);
    bb.set("name", std::string("agent"));

    REQUIRE(bb.entryCount() == 4);
    REQUIRE(bb.has("health"));

    auto* val = bb.get("health");
    REQUIRE(val != nullptr);
    REQUIRE(std::get<int32_t>(*val) == 100);

    auto* fval = bb.get("speed");
    REQUIRE(fval != nullptr);
    REQUIRE_THAT(std::get<float>(*fval), WithinAbs(3.5f, 1e-5f));
}

TEST_CASE("Blackboard remove entry", "[AI][Blackboard]") {
    NF::Blackboard bb;
    bb.set("key", 42);
    REQUIRE(bb.has("key"));

    bb.remove("key");
    REQUIRE_FALSE(bb.has("key"));
    REQUIRE(bb.entryCount() == 0);
}

TEST_CASE("Blackboard clear", "[AI][Blackboard]") {
    NF::Blackboard bb;
    bb.set("a", 1);
    bb.set("b", 2);
    bb.set("c", 3);
    REQUIRE(bb.entryCount() == 3);

    bb.clear();
    REQUIRE(bb.entryCount() == 0);
}

TEST_CASE("Blackboard get missing key returns nullptr", "[AI][Blackboard]") {
    NF::Blackboard bb;
    REQUIRE(bb.get("nonexistent") == nullptr);
}

TEST_CASE("Blackboard variant types", "[AI][Blackboard]") {
    NF::Blackboard bb;

    bb.set("int_val", 42);
    bb.set("float_val", 3.14f);
    bb.set("bool_val", true);
    bb.set("str_val", std::string("hello"));

    REQUIRE(std::holds_alternative<int32_t>(*bb.get("int_val")));
    REQUIRE(std::holds_alternative<float>(*bb.get("float_val")));
    REQUIRE(std::holds_alternative<bool>(*bb.get("bool_val")));
    REQUIRE(std::holds_alternative<std::string>(*bb.get("str_val")));
}

// ── UtilitySelector ─────────────────────────────────────────────

TEST_CASE("UtilitySelector evaluate returns highest score", "[AI][Utility]") {
    NF::UtilitySelector sel;
    sel.addAction({"attack", 0.5f, nullptr});
    sel.addAction({"flee",   0.9f, nullptr});
    sel.addAction({"idle",   0.1f, nullptr});

    auto* best = sel.evaluate();
    REQUIRE(best != nullptr);
    REQUIRE(best->name == "flee");
    REQUIRE(sel.actionCount() == 3);
}

TEST_CASE("UtilitySelector empty returns nullptr", "[AI][Utility]") {
    NF::UtilitySelector sel;
    REQUIRE(sel.evaluate() == nullptr);
}

TEST_CASE("UtilitySelector clear", "[AI][Utility]") {
    NF::UtilitySelector sel;
    sel.addAction({"a", 1.f, nullptr});
    sel.addAction({"b", 2.f, nullptr});
    REQUIRE(sel.actionCount() == 2);

    sel.clear();
    REQUIRE(sel.actionCount() == 0);
}

// ── evaluateCurve ───────────────────────────────────────────────

TEST_CASE("evaluateCurve Linear", "[AI][Utility]") {
    float result = NF::evaluateCurve(NF::UtilityCurve::Linear, 0.5f, 2.f);
    REQUIRE_THAT(result, WithinAbs(1.f, 1e-5f));
}

TEST_CASE("evaluateCurve Quadratic", "[AI][Utility]") {
    float result = NF::evaluateCurve(NF::UtilityCurve::Quadratic, 0.5f, 4.f);
    REQUIRE_THAT(result, WithinAbs(1.f, 1e-5f));
}

TEST_CASE("evaluateCurve Logistic at midpoint", "[AI][Utility]") {
    float result = NF::evaluateCurve(NF::UtilityCurve::Logistic, 0.5f, 1.f);
    REQUIRE_THAT(result, WithinAbs(0.5f, 1e-3f));
}

TEST_CASE("evaluateCurve Inverse", "[AI][Utility]") {
    float result = NF::evaluateCurve(NF::UtilityCurve::Inverse, 1.f, 2.f);
    REQUIRE_THAT(result, WithinAbs(2.f / 1.01f, 1e-3f));
}

// ── AIAgent ─────────────────────────────────────────────────────

TEST_CASE("AIAgent default state", "[AI][Agent]") {
    NF::AIAgent agent(42);

    REQUIRE(agent.entityId() == 42);
    REQUIRE(agent.currentBehavior() == NF::AIBehavior::Idle);
    REQUIRE(agent.blackboard() == nullptr);
}

TEST_CASE("AIAgent set behavior", "[AI][Agent]") {
    NF::AIAgent agent(1);
    agent.setBehavior(NF::AIBehavior::Patrol);
    REQUIRE(agent.currentBehavior() == NF::AIBehavior::Patrol);
}

TEST_CASE("AIAgent memory access", "[AI][Agent]") {
    NF::AIAgent agent(1);
    agent.memory().threat = 0.5f;

    REQUIRE_THAT(agent.memory().threat, WithinAbs(0.5f, 1e-5f));
}

TEST_CASE("AIAgent blackboard attachment", "[AI][Agent]") {
    NF::Blackboard bb;
    bb.set("target", 99);

    NF::AIAgent agent(1);
    agent.setBlackboard(&bb);

    REQUIRE(agent.blackboard() != nullptr);
    REQUIRE(agent.blackboard()->has("target"));
}

TEST_CASE("AIAgent update decays threat", "[AI][Agent]") {
    NF::AIAgent agent(1);
    agent.memory().threat = 1.f;
    agent.memory().decayRate = 0.5f;

    agent.update(1.f);
    REQUIRE(agent.memory().threat < 1.f);
}

// ── AISystem ─────────────────────────────────────────────────────

TEST_CASE("AISystem register and find agents", "[AI][System]") {
    NF::AISystem sys;
    sys.init();

    NF::AIAgent a1(10);
    NF::AIAgent a2(20);
    sys.registerAgent(a1);
    sys.registerAgent(a2);

    REQUIRE(sys.agentCount() == 2);
    REQUIRE(sys.findAgent(10) != nullptr);
    REQUIRE(sys.findAgent(20) != nullptr);
    REQUIRE(sys.findAgent(99) == nullptr);

    sys.shutdown();
}

TEST_CASE("AISystem unregister agent", "[AI][System]") {
    NF::AISystem sys;
    sys.init();

    NF::AIAgent a(5);
    sys.registerAgent(a);
    REQUIRE(sys.agentCount() == 1);

    sys.unregisterAgent(5);
    REQUIRE(sys.agentCount() == 0);

    sys.shutdown();
}

TEST_CASE("AISystem update ticks all agents", "[AI][System]") {
    NF::AISystem sys;
    sys.init();

    NF::AIAgent a1(1);
    a1.memory().threat = 1.f;
    a1.memory().decayRate = 0.1f;
    NF::AIAgent a2(2);
    a2.memory().threat = 0.5f;
    a2.memory().decayRate = 0.1f;

    sys.registerAgent(a1);
    sys.registerAgent(a2);

    sys.update(1.f);

    auto* ra1 = sys.findAgent(1);
    auto* ra2 = sys.findAgent(2);
    REQUIRE(ra1 != nullptr);
    REQUIRE(ra2 != nullptr);
    REQUIRE(ra1->memory().threat < 1.f);
    REQUIRE(ra2->memory().threat < 0.5f);

    sys.shutdown();
}

// ── AtlasAIQueryTool ────────────────────────────────────────────

TEST_CASE("AtlasAIQueryTool lifecycle", "[AI][Tools]") {
    NF::AtlasAIQueryTool tool;
    REQUIRE(tool.name() == "AtlasAI.Query");
    REQUIRE_FALSE(tool.isInitialized());

    // Query before init returns empty
    REQUIRE(tool.processQuery("hello").empty());

    tool.init();
    REQUIRE(tool.isInitialized());

    auto resp = tool.processQuery("test query");
    REQUIRE_FALSE(resp.empty());
    REQUIRE(resp.find("test query") != std::string::npos);

    tool.shutdown();
    REQUIRE_FALSE(tool.isInitialized());
}

// ── AtlasAIRulesTool ────────────────────────────────────────────

TEST_CASE("AtlasAIRulesTool rules and evaluate", "[AI][Tools]") {
    NF::AtlasAIRulesTool tool;
    tool.init();
    REQUIRE(tool.isInitialized());

    tool.addRule("damage_check", "hp < 20", "flee");
    tool.addRule("aggro_check", "enemy_near", "attack");
    REQUIRE(tool.ruleCount() == 2);

    auto ctx = NF::JsonValue::object();
    ctx.set("hp", NF::JsonValue(15));
    auto result = tool.evaluate(ctx);
    REQUIRE(result.isObject());
    REQUIRE(result["status"].asString() == "evaluated");

    tool.shutdown();
    REQUIRE_FALSE(tool.isInitialized());
    REQUIRE(tool.ruleCount() == 0);
}

// ── BlenderGeneratorTool ────────────────────────────────────────

TEST_CASE("BlenderGeneratorTool generate mesh", "[AI][Tools]") {
    NF::BlenderGeneratorTool tool;
    tool.init();
    REQUIRE(tool.isInitialized());

    auto params = NF::JsonValue::object();
    params.set("radius", NF::JsonValue(1.5f));

    auto mesh = tool.generateMesh("sphere", params);
    REQUIRE(mesh.isObject());
    REQUIRE(mesh["type"].asString() == "sphere");
    REQUIRE(mesh["generated"].asBool() == true);

    auto types = tool.supportedTypes();
    REQUIRE(types.size() == 5);

    tool.shutdown();
}

// ── ContractScannerTool ─────────────────────────────────────────

TEST_CASE("ContractScannerTool scan", "[AI][Tools]") {
    NF::ContractScannerTool tool;
    tool.init();

    auto issues = tool.scan("");
    REQUIRE(issues.size() == 1);
    REQUIRE(tool.issueCount() == 1);

    auto noIssues = tool.scan("valid code");
    REQUIRE(noIssues.empty());
    REQUIRE(tool.issueCount() == 0);

    tool.shutdown();
}

// ── ReplayMinimizerTool ─────────────────────────────────────────

TEST_CASE("ReplayMinimizerTool minimize", "[AI][Tools]") {
    NF::ReplayMinimizerTool tool;
    tool.init();

    std::vector<NF::JsonValue> frames;
    for (int i = 0; i < 10; ++i) {
        frames.push_back(NF::JsonValue(i));
    }

    auto minimized = tool.minimize(frames);
    REQUIRE(minimized.size() < frames.size());
    REQUIRE(tool.compressionRatio() < 1.f);

    tool.shutdown();
}

TEST_CASE("ReplayMinimizerTool empty input", "[AI][Tools]") {
    NF::ReplayMinimizerTool tool;
    tool.init();

    std::vector<NF::JsonValue> empty;
    auto result = tool.minimize(empty);
    REQUIRE(result.empty());
    REQUIRE_THAT(tool.compressionRatio(), WithinAbs(1.f, 1e-5f));

    tool.shutdown();
}

// ── ToolRegistry ────────────────────────────────────────────────

TEST_CASE("ToolRegistry register and find tools", "[AI][Tools]") {
    NF::ToolRegistry reg;

    reg.registerTool(std::make_unique<NF::AtlasAIQueryTool>());
    reg.registerTool(std::make_unique<NF::AtlasAIRulesTool>());
    reg.registerTool(std::make_unique<NF::BlenderGeneratorTool>());

    REQUIRE(reg.toolCount() == 3);

    auto* swiss = reg.findTool("AtlasAI.Query");
    REQUIRE(swiss != nullptr);
    REQUIRE(swiss->name() == "AtlasAI.Query");

    REQUIRE(reg.findTool("NonExistent") == nullptr);

    auto names = reg.allToolNames();
    REQUIRE(names.size() == 3);
}

TEST_CASE("ToolRegistry ITool interface", "[AI][Tools]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<NF::ContractScannerTool>());
    reg.registerTool(std::make_unique<NF::ReplayMinimizerTool>());

    REQUIRE(reg.toolCount() == 2);

    auto* tool = reg.findTool("ContractScanner");
    REQUIRE(tool != nullptr);
    REQUIRE_FALSE(tool->isInitialized());

    tool->init();
    REQUIRE(tool->isInitialized());
    tool->shutdown();
}

// ── EditorAIAssistant ───────────────────────────────────────────

TEST_CASE("EditorAIAssistant unavailable without registry", "[AI][Editor]") {
    NF::EditorAIAssistant assistant;
    REQUIRE_FALSE(assistant.isAvailable());
    REQUIRE(assistant.registeredToolCount() == 0);
    REQUIRE(assistant.query("test").empty());
}

TEST_CASE("EditorAIAssistant with registry", "[AI][Editor]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<NF::AtlasAIQueryTool>());
    reg.registerTool(std::make_unique<NF::AtlasAIRulesTool>());

    NF::EditorAIAssistant assistant;
    assistant.setToolRegistry(&reg);

    REQUIRE(assistant.isAvailable());
    REQUIRE(assistant.registeredToolCount() == 2);
    REQUIRE(assistant.toolRegistry() == &reg);

    auto resp = assistant.query("help");
    REQUIRE_FALSE(resp.empty());
    REQUIRE(resp.find("help") != std::string::npos);
}
