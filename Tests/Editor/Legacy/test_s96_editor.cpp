// S96 editor tests: DebugDrawEditor, ConsoleCommandBus, DiagnosticProfiler
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/DiagnosticProfiler.h"
#include "NF/Editor/DebugDrawEditor.h"

using namespace NF;

// ── DebugDrawEditor ──────────────────────────────────────────────────────────

TEST_CASE("DebugDrawPrimitiveType names", "[Editor][S96]") {
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Line))     == "Line");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Arrow))    == "Arrow");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Box))      == "Box");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Sphere))   == "Sphere");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Capsule))  == "Capsule");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Cylinder)) == "Cylinder");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Cone))     == "Cone");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Text))     == "Text");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Grid))     == "Grid");
    REQUIRE(std::string(debugDrawPrimitiveTypeName(DebugDrawPrimitiveType::Axis))     == "Axis");
}

TEST_CASE("DebugDrawLifetime names", "[Editor][S96]") {
    REQUIRE(std::string(debugDrawLifetimeName(DebugDrawLifetime::Frame))      == "Frame");
    REQUIRE(std::string(debugDrawLifetimeName(DebugDrawLifetime::Persistent)) == "Persistent");
    REQUIRE(std::string(debugDrawLifetimeName(DebugDrawLifetime::Timed))      == "Timed");
    REQUIRE(std::string(debugDrawLifetimeName(DebugDrawLifetime::OnSelected)) == "OnSelected");
}

TEST_CASE("DebugDrawCategory names", "[Editor][S96]") {
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::Physics))    == "Physics");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::AI))         == "AI");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::Navigation)) == "Navigation");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::Animation))  == "Animation");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::UI))         == "UI");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::Network))    == "Network");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::Gameplay))   == "Gameplay");
    REQUIRE(std::string(debugDrawCategoryName(DebugDrawCategory::Custom))     == "Custom");
}

TEST_CASE("DebugDrawEntry defaults", "[Editor][S96]") {
    DebugDrawEntry e("arrow", DebugDrawPrimitiveType::Arrow, DebugDrawCategory::Gameplay);
    REQUIRE(e.name()      == "arrow");
    REQUIRE(e.type()      == DebugDrawPrimitiveType::Arrow);
    REQUIRE(e.category()  == DebugDrawCategory::Gameplay);
    REQUIRE(e.lifetime()  == DebugDrawLifetime::Frame);
    REQUIRE(e.duration()  == 0.0f);
    REQUIRE(e.thickness() == 1.0f);
    REQUIRE(e.isEnabled());
    REQUIRE(e.depthTest());
}

TEST_CASE("DebugDrawEntry mutation", "[Editor][S96]") {
    DebugDrawEntry e("sphere", DebugDrawPrimitiveType::Sphere, DebugDrawCategory::Physics);
    e.setLifetime(DebugDrawLifetime::Timed);
    e.setDuration(2.5f);
    e.setThickness(3.0f);
    e.setEnabled(false);
    e.setDepthTest(false);
    REQUIRE(e.lifetime()  == DebugDrawLifetime::Timed);
    REQUIRE(e.duration()  == 2.5f);
    REQUIRE(e.thickness() == 3.0f);
    REQUIRE(!e.isEnabled());
    REQUIRE(!e.depthTest());
}

TEST_CASE("DebugDrawEditor add/remove", "[Editor][S96]") {
    DebugDrawEditor ed;
    DebugDrawEntry e("box", DebugDrawPrimitiveType::Box, DebugDrawCategory::AI);
    REQUIRE(ed.addEntry(e));
    REQUIRE(ed.entryCount() == 1u);
    REQUIRE(!ed.addEntry(e));
    REQUIRE(ed.removeEntry("box"));
    REQUIRE(ed.entryCount() == 0u);
    REQUIRE(!ed.removeEntry("box"));
}

TEST_CASE("DebugDrawEditor find", "[Editor][S96]") {
    DebugDrawEditor ed;
    DebugDrawEntry e("line", DebugDrawPrimitiveType::Line, DebugDrawCategory::Network);
    ed.addEntry(e);
    auto* found = ed.findEntry("line");
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == DebugDrawPrimitiveType::Line);
    REQUIRE(ed.findEntry("none") == nullptr);
}

TEST_CASE("DebugDrawEditor counts", "[Editor][S96]") {
    DebugDrawEditor ed;
    DebugDrawEntry e1("a", DebugDrawPrimitiveType::Line, DebugDrawCategory::Physics);
    DebugDrawEntry e2("b", DebugDrawPrimitiveType::Box,  DebugDrawCategory::Physics);
    e2.setEnabled(false);
    DebugDrawEntry e3("c", DebugDrawPrimitiveType::Line, DebugDrawCategory::AI);
    e3.setLifetime(DebugDrawLifetime::Persistent);
    ed.addEntry(e1); ed.addEntry(e2); ed.addEntry(e3);
    REQUIRE(ed.entryCount()   == 3u);
    REQUIRE(ed.enabledCount() == 2u);
    REQUIRE(ed.countByCategory(DebugDrawCategory::Physics)       == 2u);
    REQUIRE(ed.countByType(DebugDrawPrimitiveType::Line)         == 2u);
    REQUIRE(ed.countByLifetime(DebugDrawLifetime::Persistent)    == 1u);
}

TEST_CASE("DebugDrawEditor visibility", "[Editor][S96]") {
    DebugDrawEditor ed;
    REQUIRE(ed.isVisible());
    ed.setVisible(false);
    REQUIRE(!ed.isVisible());
}

// ── ConsoleCommandBus ────────────────────────────────────────────────────────

TEST_CASE("ConsoleCmdScope names", "[Editor][S96]") {
    REQUIRE(std::string(consoleCmdScopeName(ConsoleCmdScope::Global)) == "Global");
    REQUIRE(std::string(consoleCmdScopeName(ConsoleCmdScope::Editor)) == "Editor");
    REQUIRE(std::string(consoleCmdScopeName(ConsoleCmdScope::Game))   == "Game");
    REQUIRE(std::string(consoleCmdScopeName(ConsoleCmdScope::Server)) == "Server");
    REQUIRE(std::string(consoleCmdScopeName(ConsoleCmdScope::Client)) == "Client");
    REQUIRE(std::string(consoleCmdScopeName(ConsoleCmdScope::Plugin)) == "Plugin");
}

TEST_CASE("ConsoleCmdArgType names", "[Editor][S96]") {
    REQUIRE(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::None))   == "None");
    REQUIRE(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Bool))   == "Bool");
    REQUIRE(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Int))    == "Int");
    REQUIRE(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Float))  == "Float");
    REQUIRE(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::String)) == "String");
    REQUIRE(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Enum))   == "Enum");
}

TEST_CASE("ConsoleCmdExecResult names", "[Editor][S96]") {
    REQUIRE(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Ok))               == "Ok");
    REQUIRE(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::NotFound))         == "NotFound");
    REQUIRE(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::InvalidArgs))      == "InvalidArgs");
    REQUIRE(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::PermissionDenied)) == "PermissionDenied");
    REQUIRE(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Error))            == "Error");
}

TEST_CASE("ConsoleCommand defaults", "[Editor][S96]") {
    ConsoleCommand cmd("r.vsync", ConsoleCmdScope::Game, ConsoleCmdArgType::Bool);
    REQUIRE(cmd.name()    == "r.vsync");
    REQUIRE(cmd.scope()   == ConsoleCmdScope::Game);
    REQUIRE(cmd.argType() == ConsoleCmdArgType::Bool);
    REQUIRE(cmd.isEnabled());
    REQUIRE(!cmd.isHidden());
}

TEST_CASE("ConsoleCommand mutation", "[Editor][S96]") {
    ConsoleCommand cmd("stat fps", ConsoleCmdScope::Editor, ConsoleCmdArgType::None);
    cmd.setDescription("Show FPS counter");
    cmd.setHidden(true);
    cmd.setEnabled(false);
    REQUIRE(cmd.description() == "Show FPS counter");
    REQUIRE(cmd.isHidden());
    REQUIRE(!cmd.isEnabled());
}

TEST_CASE("ConsoleCommandBus register/unregister", "[Editor][S96]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("quit", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    REQUIRE(bus.registerCommand(cmd));
    REQUIRE(bus.commandCount() == 1u);
    REQUIRE(!bus.registerCommand(cmd));
    REQUIRE(bus.unregisterCommand("quit"));
    REQUIRE(bus.commandCount() == 0u);
    REQUIRE(!bus.unregisterCommand("quit"));
}

TEST_CASE("ConsoleCommandBus execute", "[Editor][S96]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("r.fullscreen", ConsoleCmdScope::Game, ConsoleCmdArgType::Bool);
    bus.registerCommand(cmd);
    REQUIRE(bus.execute("r.fullscreen") == ConsoleCmdExecResult::Ok);
    REQUIRE(bus.lastExec() == "r.fullscreen");
    REQUIRE(bus.execute("missing") == ConsoleCmdExecResult::NotFound);
}

TEST_CASE("ConsoleCommandBus execute disabled", "[Editor][S96]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("cheat.noclip", ConsoleCmdScope::Game, ConsoleCmdArgType::None);
    bus.registerCommand(cmd);
    auto* found = bus.findCommand("cheat.noclip");
    REQUIRE(found != nullptr);
    found->setEnabled(false);
    REQUIRE(bus.execute("cheat.noclip") == ConsoleCmdExecResult::PermissionDenied);
}

TEST_CASE("ConsoleCommandBus counts", "[Editor][S96]") {
    ConsoleCommandBus bus;
    ConsoleCommand c1("a", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    ConsoleCommand c2("b", ConsoleCmdScope::Editor, ConsoleCmdArgType::Int);
    ConsoleCommand c3("c", ConsoleCmdScope::Global, ConsoleCmdArgType::Bool);
    c3.setHidden(true);
    bus.registerCommand(c1); bus.registerCommand(c2); bus.registerCommand(c3);
    REQUIRE(bus.countByScope(ConsoleCmdScope::Global) == 2u);
    REQUIRE(bus.hiddenCount()  == 1u);
    REQUIRE(bus.enabledCount() == 3u);
}

// ── DiagnosticProfiler ───────────────────────────────────────────────────────

TEST_CASE("DiagSamplerType names", "[Editor][S96]") {
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::CPU))     == "CPU");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::GPU))     == "GPU");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::Memory))  == "Memory");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::IO))      == "IO");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::Network)) == "Network");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::Script))  == "Script");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::Physics)) == "Physics");
    REQUIRE(std::string(diagSamplerTypeName(DiagSamplerType::Render))  == "Render");
}

TEST_CASE("DiagProfilerState names", "[Editor][S96]") {
    REQUIRE(std::string(diagProfilerStateName(DiagProfilerState::Idle))      == "Idle");
    REQUIRE(std::string(diagProfilerStateName(DiagProfilerState::Sampling))  == "Sampling");
    REQUIRE(std::string(diagProfilerStateName(DiagProfilerState::Paused))    == "Paused");
    REQUIRE(std::string(diagProfilerStateName(DiagProfilerState::Stopped))   == "Stopped");
    REQUIRE(std::string(diagProfilerStateName(DiagProfilerState::Exporting)) == "Exporting");
}

TEST_CASE("DiagAlertLevel names", "[Editor][S96]") {
    REQUIRE(std::string(diagAlertLevelName(DiagAlertLevel::None))     == "None");
    REQUIRE(std::string(diagAlertLevelName(DiagAlertLevel::Info))     == "Info");
    REQUIRE(std::string(diagAlertLevelName(DiagAlertLevel::Warning))  == "Warning");
    REQUIRE(std::string(diagAlertLevelName(DiagAlertLevel::Critical)) == "Critical");
}

TEST_CASE("DiagSampler defaults", "[Editor][S96]") {
    DiagSampler s("cpu_main", DiagSamplerType::CPU);
    REQUIRE(s.name()         == "cpu_main");
    REQUIRE(s.type()         == DiagSamplerType::CPU);
    REQUIRE(s.isEnabled());
    REQUIRE(s.sampleRateHz() == 60.0f);
    REQUIRE(s.alertLevel()   == DiagAlertLevel::None);
    REQUIRE(s.threshold()    == 0.0f);
}

TEST_CASE("DiagSampler mutation", "[Editor][S96]") {
    DiagSampler s("gpu_frame", DiagSamplerType::GPU);
    s.setEnabled(false);
    s.setSampleRateHz(120.0f);
    s.setAlertLevel(DiagAlertLevel::Warning);
    s.setThreshold(16.67f);
    REQUIRE(!s.isEnabled());
    REQUIRE(s.sampleRateHz() == 120.0f);
    REQUIRE(s.alertLevel()   == DiagAlertLevel::Warning);
    REQUIRE(s.threshold()    == 16.67f);
}

TEST_CASE("DiagnosticProfiler add/remove sampler", "[Editor][S96]") {
    DiagnosticProfiler p;
    DiagSampler s("mem", DiagSamplerType::Memory);
    REQUIRE(p.addSampler(s));
    REQUIRE(p.samplerCount() == 1u);
    REQUIRE(!p.addSampler(s));
    REQUIRE(p.removeSampler("mem"));
    REQUIRE(p.samplerCount() == 0u);
    REQUIRE(!p.removeSampler("mem"));
}

TEST_CASE("DiagnosticProfiler state and sampling", "[Editor][S96]") {
    DiagnosticProfiler p;
    REQUIRE(p.state()    == DiagProfilerState::Idle);
    REQUIRE(!p.isSampling());
    p.setState(DiagProfilerState::Sampling);
    REQUIRE(p.isSampling());
    p.setState(DiagProfilerState::Paused);
    REQUIRE(!p.isSampling());
}

TEST_CASE("DiagnosticProfiler sampler counts", "[Editor][S96]") {
    DiagnosticProfiler p;
    DiagSampler s1("c", DiagSamplerType::CPU);
    DiagSampler s2("g", DiagSamplerType::GPU);
    s2.setEnabled(false);
    DiagSampler s3("m", DiagSamplerType::Memory);
    s3.setAlertLevel(DiagAlertLevel::Critical);
    p.addSampler(s1); p.addSampler(s2); p.addSampler(s3);
    REQUIRE(p.samplerCount()        == 3u);
    REQUIRE(p.enabledSamplerCount() == 2u);
    REQUIRE(p.countByType(DiagSamplerType::CPU)               == 1u);
    REQUIRE(p.countByAlertLevel(DiagAlertLevel::Critical)     == 1u);
}

TEST_CASE("DiagnosticProfiler maxSampleMs", "[Editor][S96]") {
    DiagnosticProfiler p;
    REQUIRE(p.maxSampleMs() == 16.67f);
    p.setMaxSampleMs(8.33f);
    REQUIRE(p.maxSampleMs() == 8.33f);
}
