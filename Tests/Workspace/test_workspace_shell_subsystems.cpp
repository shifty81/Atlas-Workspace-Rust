// Tests/Workspace/test_workspace_shell_subsystems.cpp
// Phase 2 — Workspace shell subsystem tests.
//
// Tests for the three newly wired OS-like subsystems:
//   1. ConsoleCommandBus  — command registration, dispatch, filtering
//   2. SelectionService   — entity selection model
//   3. EditorEventBus     — event posting, subscription, flush
//   4. WorkspaceShell     — accessor integration for new subsystems
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/ConsoleCommandBus.h"
#include "NF/Workspace/SelectionService.h"
#include "NF/Workspace/EditorEventBus.h"
#include "NF/Workspace/WorkspaceShell.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1: ConsoleCommandBus
// ═════════════════════════════════════════════════════════════════

TEST_CASE("consoleCmdScopeName returns expected strings", "[ConsoleCommandBus][scope]") {
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Global)) == "Global");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Editor)) == "Editor");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Game))   == "Game");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Server)) == "Server");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Client)) == "Client");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Plugin)) == "Plugin");
}

TEST_CASE("consoleCmdArgTypeName returns expected strings", "[ConsoleCommandBus][argtype]") {
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::None))   == "None");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Bool))   == "Bool");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Int))    == "Int");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Float))  == "Float");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::String)) == "String");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Enum))   == "Enum");
}

TEST_CASE("consoleCmdExecResultName returns expected strings", "[ConsoleCommandBus][result]") {
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Ok))               == "Ok");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::NotFound))         == "NotFound");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::InvalidArgs))      == "InvalidArgs");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::PermissionDenied)) == "PermissionDenied");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Error))            == "Error");
}

TEST_CASE("ConsoleCommand attributes", "[ConsoleCommandBus][command]") {
    ConsoleCommand cmd("scene.load", ConsoleCmdScope::Editor, ConsoleCmdArgType::String);
    CHECK(cmd.name()    == "scene.load");
    CHECK(cmd.scope()   == ConsoleCmdScope::Editor);
    CHECK(cmd.argType() == ConsoleCmdArgType::String);
    CHECK(cmd.isEnabled());
    CHECK_FALSE(cmd.isHidden());
    CHECK(cmd.description().empty());

    cmd.setDescription("Load a scene by path");
    CHECK(cmd.description() == "Load a scene by path");

    cmd.setEnabled(false);
    CHECK_FALSE(cmd.isEnabled());

    cmd.setHidden(true);
    CHECK(cmd.isHidden());
}

TEST_CASE("ConsoleCommandBus: register and find", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    CHECK(bus.commandCount() == 0);

    ConsoleCommand cmd("workspace.save", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    CHECK(bus.registerCommand(cmd));
    CHECK(bus.commandCount() == 1);

    // Duplicate registration fails
    CHECK_FALSE(bus.registerCommand(cmd));
    CHECK(bus.commandCount() == 1);

    auto* found = bus.findCommand("workspace.save");
    REQUIRE(found != nullptr);
    CHECK(found->name() == "workspace.save");

    CHECK(bus.findCommand("nonexistent") == nullptr);
}

TEST_CASE("ConsoleCommandBus: execute", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("build.run", ConsoleCmdScope::Editor, ConsoleCmdArgType::None);
    CHECK(bus.registerCommand(cmd));

    CHECK(bus.execute("build.run") == ConsoleCmdExecResult::Ok);
    CHECK(bus.lastExec() == "build.run");

    CHECK(bus.execute("does.not.exist") == ConsoleCmdExecResult::NotFound);
}

TEST_CASE("ConsoleCommandBus: disabled command returns PermissionDenied", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("debug.break", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    cmd.setEnabled(false);
    CHECK(bus.registerCommand(cmd));

    CHECK(bus.execute("debug.break") == ConsoleCmdExecResult::PermissionDenied);
}

TEST_CASE("ConsoleCommandBus: unregister", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("tool.close", ConsoleCmdScope::Editor, ConsoleCmdArgType::None);
    CHECK(bus.registerCommand(cmd));
    CHECK(bus.commandCount() == 1);

    CHECK(bus.unregisterCommand("tool.close"));
    CHECK(bus.commandCount() == 0);
    CHECK_FALSE(bus.unregisterCommand("tool.close"));  // already gone
}

TEST_CASE("ConsoleCommandBus: countByScope and hiddenCount and enabledCount", "[ConsoleCommandBus]") {
    ConsoleCommandBus bus;

    ConsoleCommand g("x.global",  ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    ConsoleCommand e("x.editor",  ConsoleCmdScope::Editor, ConsoleCmdArgType::None);
    ConsoleCommand h("x.hidden",  ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    h.setHidden(true);
    ConsoleCommand d("x.disabled",ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    d.setEnabled(false);

    CHECK(bus.registerCommand(g));
    CHECK(bus.registerCommand(e));
    CHECK(bus.registerCommand(h));
    CHECK(bus.registerCommand(d));

    CHECK(bus.countByScope(ConsoleCmdScope::Global) == 3);
    CHECK(bus.countByScope(ConsoleCmdScope::Editor) == 1);
    CHECK(bus.hiddenCount()  == 1);
    CHECK(bus.enabledCount() == 3);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2: SelectionService
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SelectionService: starts empty", "[SelectionService]") {
    SelectionService svc;
    CHECK_FALSE(svc.hasSelection());
    CHECK(svc.selectionCount() == 0);
    CHECK(svc.primarySelection() == INVALID_ENTITY);
    CHECK(svc.version() == 0);
}

TEST_CASE("SelectionService: select and deselect", "[SelectionService]") {
    SelectionService svc;

    svc.select(42u);
    CHECK(svc.hasSelection());
    CHECK(svc.isSelected(42u));
    CHECK(svc.primarySelection() == 42u);
    CHECK(svc.selectionCount() == 1);
    CHECK(svc.version() == 1);

    svc.select(99u);
    CHECK(svc.selectionCount() == 2);
    CHECK(svc.primarySelection() == 99u); // last selected is primary

    svc.deselect(99u);
    CHECK(svc.selectionCount() == 1);
    CHECK(svc.isSelected(42u));
    CHECK_FALSE(svc.isSelected(99u));
    CHECK(svc.primarySelection() == 42u); // falls back to remaining
}

TEST_CASE("SelectionService: deselect only item clears primary", "[SelectionService]") {
    SelectionService svc;
    svc.select(10u);
    svc.deselect(10u);
    CHECK_FALSE(svc.hasSelection());
    CHECK(svc.primarySelection() == INVALID_ENTITY);
}

TEST_CASE("SelectionService: selectExclusive replaces selection", "[SelectionService]") {
    SelectionService svc;
    svc.select(1u);
    svc.select(2u);
    svc.select(3u);

    svc.selectExclusive(7u);
    CHECK(svc.selectionCount() == 1);
    CHECK(svc.isSelected(7u));
    CHECK_FALSE(svc.isSelected(1u));
    CHECK(svc.primarySelection() == 7u);
}

TEST_CASE("SelectionService: toggleSelect", "[SelectionService]") {
    SelectionService svc;
    uint32_t v = svc.version();
    svc.toggleSelect(5u);
    CHECK(svc.isSelected(5u));
    CHECK(svc.version() == v + 1);
    v = svc.version();
    svc.toggleSelect(5u);
    CHECK_FALSE(svc.isSelected(5u));
    CHECK(svc.version() == v + 1);
}

TEST_CASE("SelectionService: clearSelection", "[SelectionService]") {
    SelectionService svc;
    svc.select(1u); svc.select(2u); svc.select(3u);
    uint32_t vBefore = svc.version();

    svc.clearSelection();
    CHECK_FALSE(svc.hasSelection());
    CHECK(svc.primarySelection() == INVALID_ENTITY);
    CHECK(svc.version() > vBefore);
}

TEST_CASE("SelectionService: version increments on every mutation", "[SelectionService]") {
    SelectionService svc;
    uint32_t v = svc.version();
    svc.select(1u);     CHECK(svc.version() == v + 1); v = svc.version();
    svc.deselect(1u);   CHECK(svc.version() == v + 1); v = svc.version();
    svc.clearSelection();CHECK(svc.version() == v + 1);
}

// ── DockSlot / DockPanel (also in SelectionService.h) ──────────

TEST_CASE("DockSlot values are distinct", "[SelectionService][DockSlot]") {
    CHECK(static_cast<int>(DockSlot::Left)   != static_cast<int>(DockSlot::Right));
    CHECK(static_cast<int>(DockSlot::Top)    != static_cast<int>(DockSlot::Bottom));
    CHECK(static_cast<int>(DockSlot::Center) != static_cast<int>(DockSlot::Left));
}

TEST_CASE("DockPanel default construction", "[SelectionService][DockPanel]") {
    DockPanel p;
    CHECK(p.visible);
    CHECK(p.slot == DockSlot::Center);
    CHECK(p.name.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3: EditorEventBus
// ═════════════════════════════════════════════════════════════════

TEST_CASE("editorEventPriorityName returns correct strings", "[EditorEventBus][priority]") {
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Lowest))   == "Lowest");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Low))      == "Low");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Normal))   == "Normal");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::High))     == "High");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Highest))  == "Highest");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::System))   == "System");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Critical)) == "Critical");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Realtime)) == "Realtime");
}

TEST_CASE("EditorBusEvent: basic properties", "[EditorEventBus][event]") {
    EditorBusEvent ev;
    ev.topic    = "scene.saved";
    ev.payload  = "my_scene.atlas";
    ev.priority = EditorEventPriority::High;

    CHECK(ev.topic    == "scene.saved");
    CHECK(ev.payload  == "my_scene.atlas");
    CHECK(ev.isHighPrio());
    CHECK_FALSE(ev.isCritical());
    CHECK_FALSE(ev.isConsumed());

    ev.consume();
    CHECK(ev.isConsumed());
}

TEST_CASE("EditorEventBus: starts idle with no subscribers", "[EditorEventBus]") {
    EditorEventBus bus;
    CHECK(bus.state() == EditorBusState::Idle);
    CHECK(bus.subscriptionCount() == 0);
    CHECK(bus.queueSize() == 0);
    CHECK_FALSE(bus.isSuspended());
}

TEST_CASE("EditorEventBus: post and flush delivers to subscriber", "[EditorEventBus]") {
    EditorEventBus bus;
    std::string received;

    bus.subscribe("build.done", EditorEventPriority::Normal,
        [&](const EditorBusEvent& ev) { received = ev.payload; });

    EditorBusEvent ev;
    ev.topic   = "build.done";
    ev.payload = "success";
    ev.priority = EditorEventPriority::Normal;

    CHECK(bus.post(ev));
    CHECK(bus.queueSize() == 1);

    bus.flush();
    CHECK(received == "success");
    CHECK(bus.queueSize() == 0);
}

TEST_CASE("EditorEventBus: wildcard subscriber receives all events", "[EditorEventBus]") {
    EditorEventBus bus;
    int count = 0;
    bus.subscribe("*", EditorEventPriority::Lowest,
        [&](const EditorBusEvent&) { ++count; });

    EditorBusEvent e1; e1.topic = "a"; e1.priority = EditorEventPriority::Normal;
    EditorBusEvent e2; e2.topic = "b"; e2.priority = EditorEventPriority::Low;
    bus.post(e1); bus.post(e2);
    bus.flush();
    CHECK(count == 2);
}

TEST_CASE("EditorEventBus: subscriber priority filter", "[EditorEventBus]") {
    EditorEventBus bus;
    int count = 0;
    bus.subscribe("evt", EditorEventPriority::High,
        [&](const EditorBusEvent&) { ++count; });

    EditorBusEvent low;  low.topic  = "evt"; low.priority  = EditorEventPriority::Low;
    EditorBusEvent high; high.topic = "evt"; high.priority = EditorEventPriority::High;
    bus.post(low);
    bus.post(high);
    bus.flush();
    CHECK(count == 1); // only high-priority passes filter
}

TEST_CASE("EditorEventBus: suspend blocks posts", "[EditorEventBus]") {
    EditorEventBus bus;
    bus.suspend();
    CHECK(bus.isSuspended());

    EditorBusEvent ev; ev.topic = "test"; ev.priority = EditorEventPriority::Normal;
    CHECK_FALSE(bus.post(ev));

    bus.resume();
    CHECK_FALSE(bus.isSuspended());
    CHECK(bus.post(ev)); // now accepted
}

TEST_CASE("EditorEventBus: clearQueue empties pending events", "[EditorEventBus]") {
    EditorEventBus bus;
    EditorBusEvent ev; ev.topic = "t"; ev.priority = EditorEventPriority::Normal;
    bus.post(ev); bus.post(ev);
    CHECK(bus.queueSize() == 2);
    bus.clearQueue();
    CHECK(bus.queueSize() == 0);
}

TEST_CASE("EditorEventSubscription: cancel prevents delivery", "[EditorEventBus][subscription]") {
    EditorEventBus bus;
    int count = 0;
    auto* sub = bus.subscribe("event.x", EditorEventPriority::Normal,
        [&](const EditorBusEvent&) { ++count; });

    REQUIRE(sub != nullptr);
    CHECK(sub->isActive());

    EditorBusEvent ev; ev.topic = "event.x"; ev.priority = EditorEventPriority::Normal;
    bus.post(ev);
    bus.flush();
    CHECK(count == 1);

    sub->cancel();
    CHECK_FALSE(sub->isActive());

    bus.post(ev);
    bus.flush();
    CHECK(count == 1); // still 1 — cancelled subscription not called
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4: WorkspaceShell — new subsystem accessors
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell exposes commandBus accessor", "[WorkspaceShell][commandBus]") {
    WorkspaceShell shell;
    CHECK(shell.commandBus().commandCount() == 0);

    ConsoleCommand cmd("ws.test.cmd", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    CHECK(shell.commandBus().registerCommand(cmd));
    CHECK(shell.commandBus().commandCount() == 1);

    // const accessor
    const WorkspaceShell& cshell = shell;
    CHECK(cshell.commandBus().commandCount() == 1);
}

TEST_CASE("WorkspaceShell exposes selectionService accessor", "[WorkspaceShell][selection]") {
    WorkspaceShell shell;
    CHECK_FALSE(shell.selectionService().hasSelection());

    shell.selectionService().select(100u);
    CHECK(shell.selectionService().isSelected(100u));

    // const accessor
    const WorkspaceShell& cshell = shell;
    CHECK(cshell.selectionService().isSelected(100u));
}

TEST_CASE("WorkspaceShell exposes eventBus accessor", "[WorkspaceShell][eventBus]") {
    WorkspaceShell shell;
    CHECK(shell.eventBus().subscriptionCount() == 0);
    CHECK_FALSE(shell.eventBus().isSuspended());

    shell.eventBus().suspend();
    CHECK(shell.eventBus().isSuspended());

    // const accessor
    const WorkspaceShell& cshell = shell;
    CHECK(cshell.eventBus().isSuspended());
}

TEST_CASE("WorkspaceShell: commandBus, selection, and eventBus survive initialize+shutdown", "[WorkspaceShell]") {
    WorkspaceShell shell;

    // Register a command and entity before initialize
    ConsoleCommand cmd("pre.init.cmd", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    CHECK(shell.commandBus().registerCommand(cmd));
    shell.selectionService().select(55u);

    shell.initialize();
    CHECK(shell.phase() == ShellPhase::Ready);

    // Subsystems still functional after init
    CHECK(shell.commandBus().commandCount() >= 1);
    CHECK(shell.selectionService().isSelected(55u));
    CHECK(shell.eventBus().state() == EditorBusState::Idle);

    shell.shutdown();
    CHECK(shell.phase() == ShellPhase::Destroyed);
}

TEST_CASE("WorkspaceShell: event posted through shell bus is delivered after flush", "[WorkspaceShell]") {
    WorkspaceShell shell;
    shell.initialize();

    std::string received;
    shell.eventBus().subscribe("project.loaded", EditorEventPriority::Normal,
        [&](const EditorBusEvent& ev) { received = ev.payload; });

    EditorBusEvent ev;
    ev.topic   = "project.loaded";
    ev.payload = "my_project";
    ev.priority = EditorEventPriority::Normal;
    shell.eventBus().post(ev);
    shell.eventBus().flush();

    CHECK(received == "my_project");

    shell.shutdown();
}
