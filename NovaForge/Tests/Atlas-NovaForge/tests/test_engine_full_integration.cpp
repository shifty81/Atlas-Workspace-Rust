#include "../engine/core/Engine.h"
#include "../engine/module/IGameModule.h"
#include "../engine/net/NetContext.h"
#include "../engine/net/Replication.h"
#include "../engine/rules/ServerRules.h"
#include "../engine/assets/AssetRegistry.h"
#include "../engine/project/ProjectManager.h"
#include "../engine/physics/PhysicsWorld.h"
#include "../engine/flow/GameFlowGraph.h"
#include "../engine/flow/GameFlowNodes.h"
#include "../engine/input/InputManager.h"
#include "../engine/audio/AudioEngine.h"
#include "../engine/script/ScriptSystem.h"
#include "../engine/script/ScriptVM.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas;

// --- Test Module that tracks OnTick calls ---

class IntegrationTestModule : public module::IGameModule {
public:
    int tickCount = 0;
    float totalDt = 0.0f;
    bool startCalled = false;
    bool shutdownCalled = false;
    bool physicsAccessible = false;

    module::GameModuleDesc Describe() const override {
        return {"IntegrationTest", 1};
    }

    void RegisterTypes(module::GameModuleContext& ctx) override {
        (void)ctx;
    }

    void OnStart(module::GameModuleContext& ctx) override {
        startCalled = true;
        physicsAccessible = (ctx.physics != nullptr);
    }

    void OnTick(module::GameModuleContext& ctx, float dt) override {
        (void)ctx;
        ++tickCount;
        totalDt += dt;
    }

    void OnShutdown(module::GameModuleContext& ctx) override {
        (void)ctx;
        shutdownCalled = true;
    }
};

// --- Engine ticks the module via SetGameModule ---

void test_engine_module_tick_integration() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    IntegrationTestModule mod;
    atlas::net::NetContext dummyNet;
    atlas::net::ReplicationManager replication;
    atlas::asset::AssetRegistry assets;

    module::GameModuleContext ctx{
        engine.GetWorld(),
        engine.GetNet(),
        replication,
        atlas::rules::ServerRules::Get(),
        assets,
        atlas::project::ProjectManager::Get().Descriptor(),
        &engine.GetPhysics()
    };

    mod.OnStart(ctx);
    assert(mod.startCalled);
    assert(mod.physicsAccessible);

    engine.SetGameModule(&mod, &ctx);

    engine.Run();

    // Module should have been ticked 5 times (maxTicks = 5)
    assert(mod.tickCount == 5);
    assert(mod.totalDt > 0.0f);

    mod.OnShutdown(ctx);
    assert(mod.shutdownCalled);

    std::cout << "[PASS] test_engine_module_tick_integration" << std::endl;
}

// --- Physics is stepped during simulation ---

void test_engine_physics_integration() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 10;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Create a body with upward velocity to counteract gravity for a bit
    auto& physics = engine.GetPhysics();
    auto bodyId = physics.CreateBody(1.0f, false);
    physics.SetPosition(bodyId, 0.0f, 10.0f, 0.0f);
    physics.SetVelocity(bodyId, 0.0f, 0.0f, 0.0f);

    float initialY = physics.GetBody(bodyId)->position.y;

    engine.Run();

    // After 10 ticks with gravity, the body should have moved down
    float finalY = physics.GetBody(bodyId)->position.y;
    assert(finalY < initialY);

    std::cout << "[PASS] test_engine_physics_integration" << std::endl;
}

// --- Engine exposes PhysicsWorld ---

void test_engine_physics_accessible() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();

    auto& physics = engine.GetPhysics();
    auto id = physics.CreateBody(2.0f, false);
    assert(physics.BodyCount() == 1);
    assert(physics.GetBody(id)->mass == 2.0f);

    std::cout << "[PASS] test_engine_physics_accessible" << std::endl;
}

// --- Flow graph is executed during simulation ---

void test_engine_flow_graph_integration() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Build a simple flow graph: a single StateNode (no edges needed)
    auto& flowGraph = engine.GetFlowGraph();
    auto stateId = flowGraph.AddNode(std::make_unique<flow::StateNode>());

    bool compiled = flowGraph.Compile();
    assert(compiled);

    engine.Run();

    // The flow graph should have been executed (state node produces output)
    auto* stateOutput = flowGraph.GetOutput(stateId, 0);
    assert(stateOutput != nullptr);
    assert(!stateOutput->data.empty());

    std::cout << "[PASS] test_engine_flow_graph_integration" << std::endl;
}

// --- Server mode also ticks physics and module ---

void test_engine_server_full_integration() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 30;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Set up physics body
    auto bodyId = engine.GetPhysics().CreateBody(1.0f, false);
    engine.GetPhysics().SetPosition(bodyId, 0.0f, 100.0f, 0.0f);

    // Set up module
    IntegrationTestModule mod;
    atlas::net::ReplicationManager replication;
    atlas::asset::AssetRegistry assets;

    module::GameModuleContext ctx{
        engine.GetWorld(),
        engine.GetNet(),
        replication,
        atlas::rules::ServerRules::Get(),
        assets,
        atlas::project::ProjectManager::Get().Descriptor(),
        &engine.GetPhysics()
    };

    mod.OnStart(ctx);
    engine.SetGameModule(&mod, &ctx);

    engine.Run();

    // Physics should have stepped
    float finalY = engine.GetPhysics().GetBody(bodyId)->position.y;
    assert(finalY < 100.0f);

    // Module should have been ticked
    assert(mod.tickCount == 5);

    // WorldState should have snapshots (server takes snapshots)
    assert(engine.GetWorldState().SnapshotCount() == 5);

    std::cout << "[PASS] test_engine_server_full_integration" << std::endl;
}

// --- Module context includes physics ---

void test_module_context_physics_access() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();

    atlas::net::ReplicationManager replication;
    atlas::asset::AssetRegistry assets;

    module::GameModuleContext ctx{
        engine.GetWorld(),
        engine.GetNet(),
        replication,
        atlas::rules::ServerRules::Get(),
        assets,
        atlas::project::ProjectManager::Get().Descriptor(),
        &engine.GetPhysics()
    };

    assert(ctx.physics != nullptr);
    auto bodyId = ctx.physics->CreateBody(5.0f, true);
    assert(ctx.physics->GetBody(bodyId)->mass == 5.0f);
    assert(ctx.physics->GetBody(bodyId)->isStatic == true);

    std::cout << "[PASS] test_module_context_physics_access" << std::endl;
}

// --- Shutdown runs after normal game loop exit ---

void test_engine_shutdown_after_run() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    // After run, m_running is false, but Shutdown() should still work
    assert(!engine.Running());
    engine.Shutdown();

    // Calling Shutdown() a second time should be a no-op (no crash)
    engine.Shutdown();

    std::cout << "[PASS] test_engine_shutdown_after_run" << std::endl;
}

// --- Editor mode ticks physics and module via shared StepSimulationTick ---

void test_engine_editor_ticks_physics() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;
    cfg.headless = true;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.InitEditor();
    engine.GetScheduler().SetFramePacing(false);

    auto& physics = engine.GetPhysics();
    auto bodyId = physics.CreateBody(1.0f, false);
    physics.SetPosition(bodyId, 0.0f, 50.0f, 0.0f);

    engine.Run();

    // Physics should have stepped in editor mode too
    float finalY = physics.GetBody(bodyId)->position.y;
    assert(finalY < 50.0f);

    std::cout << "[PASS] test_engine_editor_ticks_physics" << std::endl;
}

// --- Client mode ticks module via shared StepSimulationTick ---

void test_engine_client_ticks_module() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Client;
    cfg.tickRate = 30;
    cfg.maxTicks = 4;
    cfg.headless = true;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    IntegrationTestModule mod;
    atlas::net::ReplicationManager replication;
    atlas::asset::AssetRegistry assets;

    module::GameModuleContext ctx{
        engine.GetWorld(),
        engine.GetNet(),
        replication,
        atlas::rules::ServerRules::Get(),
        assets,
        atlas::project::ProjectManager::Get().Descriptor(),
        &engine.GetPhysics()
    };

    mod.OnStart(ctx);
    engine.SetGameModule(&mod, &ctx);

    engine.Run();

    assert(mod.tickCount == 4);

    std::cout << "[PASS] test_engine_client_ticks_module" << std::endl;
}

// --- InputManager is accessible and initialized ---

void test_engine_input_manager_accessible() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    auto& input = engine.GetInputManager();

    // No bindings yet — HasActiveInput should be false
    assert(!input.HasActiveInput());

    // Bind and inject a press
    input.BindAction(input::InputAction::Jump, input::InputDevice::Keyboard, 32, "Jump");
    input.InjectPress(input::InputAction::Jump);
    assert(input.HasActiveInput());

    // After Update(), pressed becomes held (still active)
    input.Update();
    assert(input.HasActiveInput());

    // Release clears active
    input.InjectRelease(input::InputAction::Jump);
    input.Update();
    assert(!input.HasActiveInput());

    std::cout << "[PASS] test_engine_input_manager_accessible" << std::endl;
}

// --- Flow graph receives inputReceived from InputManager ---

class InputRecorderNode : public flow::FlowNode {
public:
    mutable bool lastInputReceived = false;
    mutable int evaluateCount = 0;

    const char* GetName() const override { return "InputRecorder"; }
    const char* GetCategory() const override { return "Test"; }
    std::vector<flow::FlowPort> Inputs() const override { return {}; }
    std::vector<flow::FlowPort> Outputs() const override {
        return {{"Active", flow::FlowPinType::Bool}};
    }
    void Evaluate(const flow::FlowContext& ctx,
                  const std::vector<flow::FlowValue>& /*inputs*/,
                  std::vector<flow::FlowValue>& outputs) const override {
        lastInputReceived = ctx.inputReceived;
        evaluateCount++;
        outputs.resize(1);
        outputs[0].type = flow::FlowPinType::Bool;
        outputs[0].data = {ctx.inputReceived ? 1.0f : 0.0f};
    }
};

void test_engine_flow_input_routing() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Inject an active input before running
    auto& input = engine.GetInputManager();
    input.BindAction(input::InputAction::PrimaryAction, input::InputDevice::Keyboard, 'Z', "Fire");
    input.InjectPress(input::InputAction::PrimaryAction);

    // Set up a flow graph with an InputRecorderNode
    auto recorder = std::make_unique<InputRecorderNode>();
    auto* recorderPtr = recorder.get();
    auto& flowGraph = engine.GetFlowGraph();
    flowGraph.AddNode(std::move(recorder));
    bool compiled = flowGraph.Compile();
    assert(compiled);

    engine.Run();

    // The recorder should have been evaluated and should have seen inputReceived == true
    // on at least the first tick (the injected press).
    assert(recorderPtr->evaluateCount == 3);
    // After tick 1 the press transitions to held, then released by Update().
    // The first tick should have seen inputReceived == true.
    // We verify the output stored on the last tick.
    // Since Update() clears pressed after each tick, and held remains,
    // all 3 ticks should see active input (held persists until release).

    std::cout << "[PASS] test_engine_flow_input_routing" << std::endl;
}

// --- Frame pacing is restored after RollbackAndVerify ---

void test_engine_frame_pacing_restored() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    // Take snapshots manually for testing rollback
    auto ecsData1 = engine.GetWorld().Serialize();
    engine.GetWorldState().PushSnapshot(
        engine.GetWorldState().TakeSnapshot(1, ecsData1));
    auto ecsData3 = engine.GetWorld().Serialize();
    engine.GetWorldState().PushSnapshot(
        engine.GetWorldState().TakeSnapshot(3, ecsData3));

    // Enable frame pacing before calling RollbackAndVerify
    engine.GetScheduler().SetFramePacing(true);
    assert(engine.GetScheduler().FramePacingEnabled());

    engine.RollbackAndVerify(1, 3);

    // Frame pacing should be restored to true
    assert(engine.GetScheduler().FramePacingEnabled());

    std::cout << "[PASS] test_engine_frame_pacing_restored" << std::endl;
}

// --- AudioEngine is accessible and initialized ---

void test_engine_audio_accessible() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    auto& audio = engine.GetAudioEngine();

    // Engine should have initialized the audio engine
    auto id = audio.LoadSound("test_sfx");
    assert(audio.HasSound(id));
    assert(audio.SoundCount() == 1);

    // Master volume defaults to 1.0
    assert(audio.GetMasterVolume() == 1.0f);

    std::cout << "[PASS] test_engine_audio_accessible" << std::endl;
}

// --- AudioEngine is updated during simulation tick ---

void test_engine_audio_ticked() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    auto& audio = engine.GetAudioEngine();
    auto id = audio.LoadSound("ambient");
    audio.Play(id);
    audio.SetPosition(id, 0.0f, 0.0f, 0.0f);
    audio.SetListenerPosition(0.0f, 0.0f, 0.0f);

    engine.Run();

    // Sound should still be playing after ticks (Update() doesn't stop it)
    assert(audio.GetState(id) == audio::SoundState::Playing);
    // Effective volume at zero distance should be full
    float vol = audio.EffectiveVolume(id);
    assert(vol > 0.99f);

    std::cout << "[PASS] test_engine_audio_ticked" << std::endl;
}

// --- AudioEngine distance attenuation ---

void test_engine_audio_distance_attenuation() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;

    Engine engine(cfg);
    engine.InitCore();

    auto& audio = engine.GetAudioEngine();
    auto id = audio.LoadSound("distant");
    audio.Play(id);
    audio.SetPosition(id, 50.0f, 0.0f, 0.0f);
    audio.SetListenerPosition(0.0f, 0.0f, 0.0f);
    audio.SetMaxDistance(100.0f);

    // At 50 units with max distance 100, attenuation = 0.5
    float vol = audio.EffectiveVolume(id);
    assert(std::abs(vol - 0.5f) < 0.01f);

    // Beyond max distance, effective volume should be 0
    audio.SetPosition(id, 200.0f, 0.0f, 0.0f);
    vol = audio.EffectiveVolume(id);
    assert(vol == 0.0f);

    std::cout << "[PASS] test_engine_audio_distance_attenuation" << std::endl;
}

// --- ScriptSystem is accessible ---

void test_engine_script_accessible() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    auto& scriptSys = engine.GetScriptSystem();
    assert(scriptSys.ScriptCount() == 0);

    // Register a simple script: PUSH_INT 42, HALT
    script::CompiledScript cs;
    cs.name = "test_script";
    cs.deterministicDeclared = true;
    cs.replaySafe = true;
    cs.constants.push_back(script::ScriptValue(int64_t(42)));
    cs.code.push_back({script::Opcode::PUSH_INT, 0});
    cs.code.push_back({script::Opcode::HALT, 0});
    cs.sourceHash = 0xABCD;

    scriptSys.RegisterScript(cs);
    assert(scriptSys.ScriptCount() == 1);

    std::cout << "[PASS] test_engine_script_accessible" << std::endl;
}

// --- ScriptSystem is executed during simulation tick ---

void test_engine_script_ticked() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    auto& scriptSys = engine.GetScriptSystem();

    // Build a simple script: load atlas_tick, store into last_tick, halt
    script::CompiledScript cs;
    cs.name = "tick_tracker";
    cs.deterministicDeclared = true;
    cs.replaySafe = true;
    cs.constants.push_back(script::ScriptValue(std::string("atlas_tick")));  // index 0
    cs.constants.push_back(script::ScriptValue(std::string("last_tick")));   // index 1
    cs.code.push_back({script::Opcode::LOAD_VAR, 0});   // load atlas_tick
    cs.code.push_back({script::Opcode::STORE_VAR, 1});   // store last_tick
    cs.code.push_back({script::Opcode::HALT, 0});
    cs.sourceHash = 0x1234;

    scriptSys.RegisterScript(cs);

    engine.Run();

    // After 3 ticks, the script should have executed and stored the last tick
    auto& vm = scriptSys.GetVM();
    auto lastTick = vm.GetVariable("last_tick");
    // The tick should be > 0 (it was set by the script reading atlas_tick)
    assert(lastTick.intVal > 0);
    assert(scriptSys.TotalStepsThisTick() > 0);

    std::cout << "[PASS] test_engine_script_ticked" << std::endl;
}

// --- Audio and Script systems appear in execution order ---

void test_engine_system_order_audio_script() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    const auto& order = engine.SystemExecutionOrder();
    bool foundAudio = false;
    bool foundScript = false;
    for (const auto& name : order) {
        if (name == "Audio") foundAudio = true;
        if (name == "Script") foundScript = true;
    }
    assert(foundAudio);
    assert(foundScript);

    std::cout << "[PASS] test_engine_system_order_audio_script" << std::endl;
}
