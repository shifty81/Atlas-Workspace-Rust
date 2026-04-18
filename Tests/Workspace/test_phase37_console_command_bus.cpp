// Tests/Workspace/test_phase37_console_command_bus.cpp
// Phase 37 — Console Command Bus
//
// Tests for:
//   1. ConsoleCmdScope      — enum name helpers
//   2. ConsoleCmdArgType    — enum name helpers
//   3. ConsoleCmdExecResult — enum name helpers
//   4. ConsoleCommand       — construction, setters, flags
//   5. ConsoleCommandBus    — register/unregister/find, execute, counts, filters
//   6. Integration          — multi-scope palette, execute flow, toggle-disable

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/ConsoleCommandBus.h"
#include <string>

using namespace NF;

// Helper: build a minimal command
static ConsoleCommand makeCmd(const std::string& name,
                              ConsoleCmdScope scope = ConsoleCmdScope::Global,
                              ConsoleCmdArgType argType = ConsoleCmdArgType::None) {
    return ConsoleCommand(name, scope, argType);
}

// ─────────────────────────────────────────────────────────────────
// 1. ConsoleCmdScope enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ConsoleCmdScope name helpers all values", "[ConsoleCmdScope]") {
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Global)) == "Global");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Editor)) == "Editor");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Game))   == "Game");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Server)) == "Server");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Client)) == "Client");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Plugin)) == "Plugin");
}

// ─────────────────────────────────────────────────────────────────
// 2. ConsoleCmdArgType enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ConsoleCmdArgType name helpers all values", "[ConsoleCmdArgType]") {
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::None))   == "None");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Bool))   == "Bool");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Int))    == "Int");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Float))  == "Float");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::String)) == "String");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Enum))   == "Enum");
}

// ─────────────────────────────────────────────────────────────────
// 3. ConsoleCmdExecResult enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ConsoleCmdExecResult name helpers all values", "[ConsoleCmdExecResult]") {
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Ok))               == "Ok");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::NotFound))         == "NotFound");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::InvalidArgs))      == "InvalidArgs");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::PermissionDenied)) == "PermissionDenied");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Error))            == "Error");
}

// ─────────────────────────────────────────────────────────────────
// 4. ConsoleCommand
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ConsoleCommand stores name scope argType", "[ConsoleCommand]") {
    auto c = makeCmd("r.vsync", ConsoleCmdScope::Editor, ConsoleCmdArgType::Bool);
    CHECK(c.name()    == "r.vsync");
    CHECK(c.scope()   == ConsoleCmdScope::Editor);
    CHECK(c.argType() == ConsoleCmdArgType::Bool);
}

TEST_CASE("ConsoleCommand default enabled and not hidden", "[ConsoleCommand]") {
    auto c = makeCmd("g.gravity");
    CHECK(c.isEnabled());
    CHECK_FALSE(c.isHidden());
}

TEST_CASE("ConsoleCommand setDescription stores description", "[ConsoleCommand]") {
    auto c = makeCmd("fps.cap");
    c.setDescription("Cap the frame rate");
    CHECK(c.description() == "Cap the frame rate");
}

TEST_CASE("ConsoleCommand setEnabled false disables command", "[ConsoleCommand]") {
    auto c = makeCmd("debug.crash");
    c.setEnabled(false);
    CHECK_FALSE(c.isEnabled());
}

TEST_CASE("ConsoleCommand setHidden true hides command", "[ConsoleCommand]") {
    auto c = makeCmd("internal.cmd");
    c.setHidden(true);
    CHECK(c.isHidden());
}

// ─────────────────────────────────────────────────────────────────
// 5. ConsoleCommandBus
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ConsoleCommandBus default is empty", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    CHECK(bus.commandCount() == 0u);
    CHECK(bus.lastExec().empty());
}

TEST_CASE("ConsoleCommandBus registerCommand adds command", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    CHECK(bus.registerCommand(makeCmd("r.bloom")));
    CHECK(bus.commandCount() == 1u);
}

TEST_CASE("ConsoleCommandBus registerCommand duplicate fails", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("r.bloom"));
    CHECK_FALSE(bus.registerCommand(makeCmd("r.bloom")));
    CHECK(bus.commandCount() == 1u);
}

TEST_CASE("ConsoleCommandBus unregisterCommand removes command", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("r.bloom"));
    CHECK(bus.unregisterCommand("r.bloom"));
    CHECK(bus.commandCount() == 0u);
}

TEST_CASE("ConsoleCommandBus unregisterCommand fails for unknown name", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    CHECK_FALSE(bus.unregisterCommand("nope"));
}

TEST_CASE("ConsoleCommandBus findCommand returns nullptr for unknown", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    CHECK(bus.findCommand("missing") == nullptr);
}

TEST_CASE("ConsoleCommandBus findCommand returns pointer to existing", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("r.bloom", ConsoleCmdScope::Editor));
    ConsoleCommand* c = bus.findCommand("r.bloom");
    REQUIRE(c != nullptr);
    CHECK(c->scope() == ConsoleCmdScope::Editor);
}

TEST_CASE("ConsoleCommandBus execute known command returns Ok", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("r.bloom"));
    CHECK(bus.execute("r.bloom") == ConsoleCmdExecResult::Ok);
    CHECK(bus.lastExec() == "r.bloom");
}

TEST_CASE("ConsoleCommandBus execute unknown command returns NotFound", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    CHECK(bus.execute("missing") == ConsoleCmdExecResult::NotFound);
}

TEST_CASE("ConsoleCommandBus execute disabled command returns PermissionDenied", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    auto cmd = makeCmd("debug.crash");
    cmd.setEnabled(false);
    (void)bus.registerCommand(cmd);
    CHECK(bus.execute("debug.crash") == ConsoleCmdExecResult::PermissionDenied);
}

TEST_CASE("ConsoleCommandBus countByScope filters correctly", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("r.bloom",   ConsoleCmdScope::Editor));
    (void)bus.registerCommand(makeCmd("g.gravity", ConsoleCmdScope::Game));
    (void)bus.registerCommand(makeCmd("r.shadow",  ConsoleCmdScope::Editor));
    CHECK(bus.countByScope(ConsoleCmdScope::Editor) == 2u);
    CHECK(bus.countByScope(ConsoleCmdScope::Game)   == 1u);
    CHECK(bus.countByScope(ConsoleCmdScope::Global) == 0u);
}

TEST_CASE("ConsoleCommandBus hiddenCount counts hidden commands", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    auto hidden = makeCmd("internal");
    hidden.setHidden(true);
    (void)bus.registerCommand(hidden);
    (void)bus.registerCommand(makeCmd("visible"));
    CHECK(bus.hiddenCount() == 1u);
}

TEST_CASE("ConsoleCommandBus enabledCount counts enabled commands", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    auto disabled = makeCmd("debug.crash");
    disabled.setEnabled(false);
    (void)bus.registerCommand(disabled);
    (void)bus.registerCommand(makeCmd("r.bloom"));
    CHECK(bus.enabledCount() == 1u);
}

// ─────────────────────────────────────────────────────────────────
// 6. Integration
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ConsoleCmdBus integration: multi-scope command palette", "[ConsoleCmdBusIntegration]") {
    ConsoleCommandBus bus;

    // Register commands across scopes
    (void)bus.registerCommand(makeCmd("quit",        ConsoleCmdScope::Global));
    (void)bus.registerCommand(makeCmd("r.bloom",     ConsoleCmdScope::Editor));
    (void)bus.registerCommand(makeCmd("g.gravity",   ConsoleCmdScope::Game));
    (void)bus.registerCommand(makeCmd("net.lag_sim", ConsoleCmdScope::Server));

    CHECK(bus.commandCount() == 4u);
    CHECK(bus.countByScope(ConsoleCmdScope::Global) == 1u);
    CHECK(bus.countByScope(ConsoleCmdScope::Editor) == 1u);
}

TEST_CASE("ConsoleCmdBus integration: execute and lastExec tracking", "[ConsoleCmdBusIntegration]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("r.bloom"));
    (void)bus.registerCommand(makeCmd("r.shadow"));

    (void)bus.execute("r.bloom");
    CHECK(bus.lastExec() == "r.bloom");
    (void)bus.execute("r.shadow");
    CHECK(bus.lastExec() == "r.shadow");
}

TEST_CASE("ConsoleCmdBus integration: disable then re-enable command", "[ConsoleCmdBusIntegration]") {
    ConsoleCommandBus bus;
    (void)bus.registerCommand(makeCmd("debug.crash"));
    CHECK(bus.execute("debug.crash") == ConsoleCmdExecResult::Ok);

    // Disable via findCommand
    ConsoleCommand* c = bus.findCommand("debug.crash");
    REQUIRE(c != nullptr);
    c->setEnabled(false);
    CHECK(bus.execute("debug.crash") == ConsoleCmdExecResult::PermissionDenied);

    // Re-enable
    c->setEnabled(true);
    CHECK(bus.execute("debug.crash") == ConsoleCmdExecResult::Ok);
}
