// Tests/Editor/test_workspace_shell.cpp
// Comprehensive tests for Phase 1 workspace core stabilization:
//   IHostedTool, ToolRegistry, PanelRegistry, WorkspaceShell
//   ISharedPanel, SharedPanels (Phase 3 shared panel extraction)
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/ToolRegistry.h"
#include "NF/Workspace/PanelRegistry.h"
#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/SharedPanels.h"

// ── Concrete test tool ────────────────────────────────────────────
// Minimal IHostedTool implementation for test purposes.

class StubHostedTool : public NF::IHostedTool {
public:
    StubHostedTool(const std::string& id, const std::string& name,
                   NF::HostedToolCategory cat = NF::HostedToolCategory::Utility,
                   bool primary = true)
    {
        m_desc.toolId      = id;
        m_desc.displayName = name;
        m_desc.category    = cat;
        m_desc.isPrimary   = primary;
    }

    const NF::HostedToolDescriptor& descriptor() const override { return m_desc; }
    const std::string& toolId() const override { return m_desc.toolId; }

    bool initialize() override {
        if (m_state != NF::HostedToolState::Unloaded) return false;
        m_state = NF::HostedToolState::Ready;
        ++m_initCount;
        return true;
    }
    void shutdown() override { m_state = NF::HostedToolState::Unloaded; ++m_shutdownCount; }
    void activate() override { m_state = NF::HostedToolState::Active; }
    void suspend()  override { m_state = NF::HostedToolState::Suspended; }
    void update(float dt) override { m_totalDt += dt; ++m_updateCount; }

    NF::HostedToolState state() const override { return m_state; }

    void onProjectLoaded(const std::string& pid) override { m_lastProjectId = pid; }
    void onProjectUnloaded() override { m_lastProjectId.clear(); }

    // Test accessors
    size_t initCount()     const { return m_initCount; }
    size_t shutdownCount() const { return m_shutdownCount; }
    size_t updateCount()   const { return m_updateCount; }
    float  totalDt()       const { return m_totalDt; }
    const std::string& lastProjectId() const { return m_lastProjectId; }

    void addSupportedPanel(const std::string& pid) { m_desc.supportedPanels.push_back(pid); }

private:
    NF::HostedToolDescriptor m_desc;
    NF::HostedToolState      m_state = NF::HostedToolState::Unloaded;
    size_t m_initCount     = 0;
    size_t m_shutdownCount = 0;
    size_t m_updateCount   = 0;
    float  m_totalDt       = 0.f;
    std::string m_lastProjectId;
};

// ── Concrete test adapter ─────────────────────────────────────────

class StubProjectAdapter : public NF::IGameProjectAdapter {
public:
    StubProjectAdapter(const std::string& pid, const std::string& name)
        : m_pid(pid), m_name(name) {}

    std::string projectId()          const override { return m_pid; }
    std::string projectDisplayName() const override { return m_name; }
    bool initialize() override { m_inited = true; return true; }
    void shutdown()   override { m_inited = false; }

    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        NF::GameplaySystemPanelDescriptor d;
        d.id          = "test.economy";
        d.displayName = "Economy";
        d.hostToolId  = NF::HostToolId::ProjectSystems;
        d.category    = "gameplay";
        d.projectId   = m_pid;
        d.enabled     = true;
        return { d };
    }

    bool isInited() const { return m_inited; }

private:
    std::string m_pid, m_name;
    bool m_inited = false;
};

// ═══════════════════════════════════════════════════════════════════
// IHostedTool + HostedToolDescriptor
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("IHostedTool: descriptor identity", "[workspace][ihostedtool]") {
    StubHostedTool tool("workspace.scene_editor", "Scene Editor",
                        NF::HostedToolCategory::SceneEditing);
    REQUIRE(tool.toolId() == "workspace.scene_editor");
    REQUIRE(tool.descriptor().displayName == "Scene Editor");
    REQUIRE(tool.descriptor().category == NF::HostedToolCategory::SceneEditing);
    REQUIRE(tool.descriptor().isValid());
}

TEST_CASE("IHostedTool: lifecycle transitions", "[workspace][ihostedtool]") {
    StubHostedTool tool("test.tool", "Test Tool");
    REQUIRE(tool.state() == NF::HostedToolState::Unloaded);

    REQUIRE(tool.initialize());
    REQUIRE(tool.state() == NF::HostedToolState::Ready);
    REQUIRE(tool.initCount() == 1);

    // double-init should fail
    REQUIRE_FALSE(tool.initialize());

    tool.activate();
    REQUIRE(tool.state() == NF::HostedToolState::Active);

    tool.suspend();
    REQUIRE(tool.state() == NF::HostedToolState::Suspended);

    tool.activate();
    REQUIRE(tool.state() == NF::HostedToolState::Active);

    tool.update(0.016f);
    REQUIRE(tool.updateCount() == 1);

    tool.shutdown();
    REQUIRE(tool.state() == NF::HostedToolState::Unloaded);
    REQUIRE(tool.shutdownCount() == 1);
}

TEST_CASE("IHostedTool: project hooks", "[workspace][ihostedtool]") {
    StubHostedTool tool("test.tool", "Test Tool");
    tool.onProjectLoaded("novaforge");
    REQUIRE(tool.lastProjectId() == "novaforge");
    tool.onProjectUnloaded();
    REQUIRE(tool.lastProjectId().empty());
}

TEST_CASE("HostedToolDescriptor: validation", "[workspace][ihostedtool]") {
    NF::HostedToolDescriptor good;
    good.toolId      = "test.tool";
    good.displayName = "Test";
    REQUIRE(good.isValid());

    NF::HostedToolDescriptor badId;
    badId.displayName = "Test";
    REQUIRE_FALSE(badId.isValid());

    NF::HostedToolDescriptor badName;
    badName.toolId = "test.tool";
    REQUIRE_FALSE(badName.isValid());
}

TEST_CASE("HostedToolCategory: name round-trip", "[workspace][ihostedtool]") {
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::ProjectBrowser)) == "ProjectBrowser");
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::SceneEditing)) == "SceneEditing");
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::AssetAuthoring)) == "AssetAuthoring");
    REQUIRE(std::string(NF::hostedToolCategoryName(NF::HostedToolCategory::AIAssistant)) == "AIAssistant");
}

TEST_CASE("HostedToolState: name round-trip", "[workspace][ihostedtool]") {
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Unloaded)) == "Unloaded");
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Ready)) == "Ready");
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Active)) == "Active");
    REQUIRE(std::string(NF::hostedToolStateName(NF::HostedToolState::Suspended)) == "Suspended");
}

// ═══════════════════════════════════════════════════════════════════
// ToolRegistry
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("ToolRegistry: register and find", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    REQUIRE(reg.empty());
    auto t = std::make_unique<StubHostedTool>("workspace.scene_editor", "Scene Editor");
    REQUIRE(reg.registerTool(std::move(t)));
    REQUIRE(reg.count() == 1);
    REQUIRE(reg.isRegistered("workspace.scene_editor"));
    REQUIRE_FALSE(reg.isRegistered("workspace.unknown"));
    auto* found = reg.find("workspace.scene_editor");
    REQUIRE(found != nullptr);
    REQUIRE(found->toolId() == "workspace.scene_editor");
}

TEST_CASE("ToolRegistry: reject invalid and duplicate", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    // null
    REQUIRE_FALSE(reg.registerTool(nullptr));
    // invalid descriptor (empty id)
    auto bad = std::make_unique<StubHostedTool>("", "Bad");
    REQUIRE_FALSE(reg.registerTool(std::move(bad)));
    // valid
    auto a = std::make_unique<StubHostedTool>("t.a", "A");
    REQUIRE(reg.registerTool(std::move(a)));
    // duplicate id
    auto a2 = std::make_unique<StubHostedTool>("t.a", "A2");
    REQUIRE_FALSE(reg.registerTool(std::move(a2)));
}

TEST_CASE("ToolRegistry: unregister", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    REQUIRE(reg.count() == 1);
    REQUIRE(reg.unregisterTool("t.a"));
    REQUIRE(reg.count() == 0);
    REQUIRE_FALSE(reg.unregisterTool("t.a"));
}

TEST_CASE("ToolRegistry: byCategory", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.scene", "Scene", NF::HostedToolCategory::SceneEditing));
    reg.registerTool(std::make_unique<StubHostedTool>("t.asset", "Asset", NF::HostedToolCategory::AssetAuthoring));
    reg.registerTool(std::make_unique<StubHostedTool>("t.scene2", "Scene2", NF::HostedToolCategory::SceneEditing));
    auto scenes = reg.byCategory(NF::HostedToolCategory::SceneEditing);
    REQUIRE(scenes.size() == 2);
    auto assets = reg.byCategory(NF::HostedToolCategory::AssetAuthoring);
    REQUIRE(assets.size() == 1);
}

TEST_CASE("ToolRegistry: primaryTools filter", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A", NF::HostedToolCategory::Utility, true));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B", NF::HostedToolCategory::Utility, false));
    auto primary = reg.primaryTools();
    REQUIRE(primary.size() == 1);
    REQUIRE(primary[0]->toolId() == "t.a");
}

TEST_CASE("ToolRegistry: activateTool and suspend previous", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    reg.initializeAll();

    REQUIRE(reg.activateTool("t.a"));
    REQUIRE(reg.activeToolId() == "t.a");
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Active);

    REQUIRE(reg.activateTool("t.b"));
    REQUIRE(reg.activeToolId() == "t.b");
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Active);
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Suspended);
}

TEST_CASE("ToolRegistry: activateTool fails if unloaded", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    // Not initialized => Unloaded
    REQUIRE_FALSE(reg.activateTool("t.a"));
}

TEST_CASE("ToolRegistry: initializeAll and shutdownAll", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    REQUIRE(reg.initializeAll());
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Ready);
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Ready);

    reg.activateTool("t.a");
    reg.shutdownAll();
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Unloaded);
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Unloaded);
    REQUIRE(reg.activeToolId().empty());
}

TEST_CASE("ToolRegistry: updateActive", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.initializeAll();
    reg.activateTool("t.a");
    reg.updateActive(0.1f);
    auto* tool = static_cast<StubHostedTool*>(reg.find("t.a"));
    REQUIRE(tool->updateCount() == 1);
}

TEST_CASE("ToolRegistry: allDescriptors", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    auto descs = reg.allDescriptors();
    REQUIRE(descs.size() == 2);
}

TEST_CASE("ToolRegistry: notifyProject events", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.notifyProjectLoaded("novaforge");
    auto* tool = static_cast<StubHostedTool*>(reg.find("t.a"));
    REQUIRE(tool->lastProjectId() == "novaforge");
    reg.notifyProjectUnloaded();
    REQUIRE(tool->lastProjectId().empty());
}

TEST_CASE("ToolRegistry: deactivateTool with no active tool returns false", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.initializeAll();
    // No tool activated yet
    REQUIRE_FALSE(reg.deactivateTool());
    REQUIRE(reg.activeToolId().empty());
}

TEST_CASE("ToolRegistry: deactivateTool suspends active tool and clears id", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.initializeAll();
    REQUIRE(reg.activateTool("t.a"));
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Active);
    REQUIRE(reg.activeToolId() == "t.a");

    REQUIRE(reg.deactivateTool());

    REQUIRE(reg.activeToolId().empty());
    REQUIRE(reg.activeTool() == nullptr);
    REQUIRE(reg.find("t.a")->state() == NF::HostedToolState::Suspended);
}

TEST_CASE("ToolRegistry: deactivateTool idempotent on second call", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.initializeAll();
    reg.activateTool("t.a");
    REQUIRE(reg.deactivateTool());
    // Second call: nothing active, returns false
    REQUIRE_FALSE(reg.deactivateTool());
}

TEST_CASE("ToolRegistry: can activate tool after deactivateTool", "[workspace][toolregistry]") {
    NF::ToolRegistry reg;
    reg.registerTool(std::make_unique<StubHostedTool>("t.a", "A"));
    reg.registerTool(std::make_unique<StubHostedTool>("t.b", "B"));
    reg.initializeAll();
    REQUIRE(reg.activateTool("t.a"));
    REQUIRE(reg.deactivateTool());
    REQUIRE(reg.activeToolId().empty());
    // Activate again (from Suspended state — activateTool checks for Unloaded only)
    REQUIRE(reg.activateTool("t.b"));
    REQUIRE(reg.activeToolId() == "t.b");
    REQUIRE(reg.find("t.b")->state() == NF::HostedToolState::Active);
}

// ═══════════════════════════════════════════════════════════════════
// PanelRegistry
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("PanelRegistry: register and find", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    REQUIRE(reg.empty());
    NF::SharedPanelDescriptor d;
    d.panelId     = "inspector";
    d.displayName = "Inspector";
    d.category    = NF::SharedPanelCategory::Inspector;
    REQUIRE(reg.registerPanel(d));
    REQUIRE(reg.count() == 1);
    auto* found = reg.find("inspector");
    REQUIRE(found != nullptr);
    REQUIRE(found->displayName == "Inspector");
}

TEST_CASE("PanelRegistry: reject invalid and duplicate", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    NF::SharedPanelDescriptor bad;
    REQUIRE_FALSE(reg.registerPanel(bad)); // invalid
    bad.panelId = "x";
    REQUIRE_FALSE(reg.registerPanel(bad)); // missing displayName
    bad.displayName = "X";
    REQUIRE(reg.registerPanel(bad));
    REQUIRE_FALSE(reg.registerPanel(bad)); // duplicate
}

TEST_CASE("PanelRegistry: unregister", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    NF::SharedPanelDescriptor d;
    d.panelId = "x"; d.displayName = "X";
    reg.registerPanel(d);
    REQUIRE(reg.unregisterPanel("x"));
    REQUIRE(reg.count() == 0);
    REQUIRE_FALSE(reg.unregisterPanel("x"));
}

TEST_CASE("PanelRegistry: byCategory", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"inspector", "Inspector", NF::SharedPanelCategory::Inspector});
    reg.registerPanel({"outliner", "Outliner", NF::SharedPanelCategory::Navigation});
    reg.registerPanel({"content", "Content", NF::SharedPanelCategory::Navigation});
    auto navs = reg.byCategory(NF::SharedPanelCategory::Navigation);
    REQUIRE(navs.size() == 2);
    auto insp = reg.byCategory(NF::SharedPanelCategory::Inspector);
    REQUIRE(insp.size() == 1);
}

TEST_CASE("PanelRegistry: visibility", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"x", "X", NF::SharedPanelCategory::Inspector, true});
    reg.registerPanel({"y", "Y", NF::SharedPanelCategory::Inspector, false});
    REQUIRE(reg.isPanelVisible("x"));
    REQUIRE_FALSE(reg.isPanelVisible("y"));
    REQUIRE(reg.setPanelVisible("y", true));
    REQUIRE(reg.isPanelVisible("y"));
    auto visible = reg.visiblePanels();
    REQUIRE(visible.size() == 2);
}

TEST_CASE("PanelRegistry: panelsForTool", "[workspace][panelregistry]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"inspector", "Inspector", NF::SharedPanelCategory::Inspector});
    reg.registerPanel({"outliner", "Outliner", NF::SharedPanelCategory::Navigation});
    reg.registerPanel({"console", "Console", NF::SharedPanelCategory::Output});
    auto match = reg.panelsForTool({"inspector", "console"});
    REQUIRE(match.size() == 2);
    auto noMatch = reg.panelsForTool({"unknown"});
    REQUIRE(noMatch.empty());
}

TEST_CASE("SharedPanelCategory: name round-trip", "[workspace][panelregistry]") {
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Inspector)) == "Inspector");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Navigation)) == "Navigation");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Output)) == "Output");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::AI)) == "AI");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Preview)) == "Preview");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Editing)) == "Editing");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Status)) == "Status");
    REQUIRE(std::string(NF::sharedPanelCategoryName(NF::SharedPanelCategory::Custom)) == "Custom");
}

// ═══════════════════════════════════════════════════════════════════
// WorkspaceShell
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell: default state", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE(shell.phase() == NF::ShellPhase::Created);
    REQUIRE_FALSE(shell.hasProject());
}

TEST_CASE("WorkspaceShell: initialize registers default panels", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE(shell.initialize());
    REQUIRE(shell.phase() == NF::ShellPhase::Ready);
    // canonical descriptor-only panels
    REQUIRE(shell.panelRegistry().isRegistered("inspector"));
    REQUIRE(shell.panelRegistry().isRegistered("outliner"));
    REQUIRE(shell.panelRegistry().isRegistered("console"));
    REQUIRE(shell.panelRegistry().isRegistered("notifications"));
    REQUIRE(shell.panelRegistry().isRegistered("atlasai_chat"));
    REQUIRE(shell.panelRegistry().isRegistered("command_palette"));
    REQUIRE(shell.panelRegistry().isRegistered("asset_preview"));
    // factory-backed shared panels
    REQUIRE(shell.panelRegistry().isRegistered("content_browser"));
    REQUIRE(shell.panelRegistry().isRegistered("component_inspector"));
    REQUIRE(shell.panelRegistry().isRegistered("diagnostics"));
    REQUIRE(shell.panelRegistry().isRegistered("memory_profiler"));
    REQUIRE(shell.panelRegistry().isRegistered("pipeline_monitor"));
    REQUIRE(shell.panelRegistry().isRegistered("notification_center"));
    // newly added panels
    REQUIRE(shell.panelRegistry().isRegistered("preferences"));
    REQUIRE(shell.panelRegistry().isRegistered("project_settings"));
    REQUIRE(shell.panelRegistry().count() == 16);
}

TEST_CASE("WorkspaceShell: double-init fails", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE(shell.initialize());
    REQUIRE_FALSE(shell.initialize());
}

TEST_CASE("WorkspaceShell: shutdown transitions to Destroyed", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.shutdown();
    REQUIRE(shell.phase() == NF::ShellPhase::Destroyed);
}

TEST_CASE("WorkspaceShell: register tool before init, initialized by init", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    auto t = std::make_unique<StubHostedTool>("workspace.scene_editor", "Scene Editor",
                                               NF::HostedToolCategory::SceneEditing);
    auto* raw = t.get();
    shell.toolRegistry().registerTool(std::move(t));
    shell.initialize();
    REQUIRE(raw->state() == NF::HostedToolState::Ready);
    REQUIRE(shell.toolRegistry().count() == 1); // only the manually registered stub
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: activate tool through shell", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("workspace.scene_editor", "Scene Editor"));
    shell.initialize();
    REQUIRE(shell.toolRegistry().activateTool("workspace.scene_editor"));
    REQUIRE(shell.toolRegistry().activeToolId() == "workspace.scene_editor");
    auto* active = shell.toolRegistry().activeTool();
    REQUIRE(active != nullptr);
    REQUIRE(active->state() == NF::HostedToolState::Active);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: update forwards to active tool", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.a", "A"));
    shell.initialize();
    shell.toolRegistry().activateTool("t.a");
    shell.update(0.05f);
    auto* tool = static_cast<StubHostedTool*>(shell.toolRegistry().find("t.a"));
    REQUIRE(tool->updateCount() == 1);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: loadProject", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    auto adapter = std::make_unique<StubProjectAdapter>("novaforge", "NovaForge");
    REQUIRE(shell.loadProject(std::move(adapter)));
    REQUIRE(shell.hasProject());
    REQUIRE(shell.projectAdapter()->projectId() == "novaforge");
    REQUIRE(shell.projectSystemsTool().panelCount() == 1);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: unloadProject", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.loadProject(std::make_unique<StubProjectAdapter>("nf", "NF"));
    REQUIRE(shell.hasProject());
    shell.unloadProject();
    REQUIRE_FALSE(shell.hasProject());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: loadProject null fails", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE_FALSE(shell.loadProject(nullptr));
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: loadProject before init fails", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    REQUIRE_FALSE(shell.loadProject(
        std::make_unique<StubProjectAdapter>("nf", "NF")));
}

TEST_CASE("WorkspaceShell: loadProject notifies tools", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.a", "A"));
    shell.initialize();
    shell.loadProject(std::make_unique<StubProjectAdapter>("nf", "NF"));
    auto* tool = static_cast<StubHostedTool*>(shell.toolRegistry().find("t.a"));
    REQUIRE(tool->lastProjectId() == "nf");
    shell.unloadProject();
    REQUIRE(tool->lastProjectId().empty());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: shell contract is wired", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE(shell.shellContract().isInitialized());
    shell.shutdown();
}

TEST_CASE("ShellPhase: name round-trip", "[workspace][shell]") {
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Created)) == "Created");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Initializing)) == "Initializing");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Ready)) == "Ready");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::ShuttingDown)) == "ShuttingDown");
    REQUIRE(std::string(NF::shellPhaseName(NF::ShellPhase::Destroyed)) == "Destroyed");
}

TEST_CASE("WorkspaceShell: accessors return correct registries", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    // Panel registry has default panels (10 descriptor-only + 6 factory-backed)
    REQUIRE(shell.panelRegistry().count() == 16);
    // Tool registry has no auto-registered tools (factories must be added externally)
    REQUIRE(shell.toolRegistry().count() == 0);
    // App registry is accessible
    REQUIRE(shell.appRegistry().empty());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: multiple tools activate/suspend", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.a", "A", NF::HostedToolCategory::SceneEditing));
    shell.toolRegistry().registerTool(
        std::make_unique<StubHostedTool>("t.b", "B", NF::HostedToolCategory::AssetAuthoring));
    shell.initialize();

    shell.toolRegistry().activateTool("t.a");
    REQUIRE(shell.toolRegistry().find("t.a")->state() == NF::HostedToolState::Active);

    shell.toolRegistry().activateTool("t.b");
    REQUIRE(shell.toolRegistry().find("t.b")->state() == NF::HostedToolState::Active);
    REQUIRE(shell.toolRegistry().find("t.a")->state() == NF::HostedToolState::Suspended);

    shell.toolRegistry().activateTool("t.a");
    REQUIRE(shell.toolRegistry().find("t.a")->state() == NF::HostedToolState::Active);
    REQUIRE(shell.toolRegistry().find("t.b")->state() == NF::HostedToolState::Suspended);

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: update with no active tool does not crash", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.update(0.016f); // no crash
    shell.shutdown();
}

TEST_CASE("WorkspaceShell: update before init does nothing", "[workspace][shell]") {
    NF::WorkspaceShell shell;
    shell.update(0.016f); // no crash
}

// ─────────────────────────────────────────────────────────────────────────────
// Phase 3 Consolidation: SceneEditorTool tests
// These test the first real NF::IHostedTool from Phase 3 consolidation.
// ─────────────────────────────────────────────────────────────────────────────
#include "NF/Editor/SceneEditorTool.h"

// ── SceneEditorTool identity ──────────────────────────────────────

TEST_CASE("SceneEditorTool: toolId matches constant", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    REQUIRE(tool.toolId() == NF::SceneEditorTool::kToolId);
    REQUIRE(tool.toolId() == "workspace.scene_editor");
}

TEST_CASE("SceneEditorTool: descriptor is valid and primary", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    const auto& d = tool.descriptor();
    REQUIRE(d.isValid());
    REQUIRE(d.isPrimary);
    REQUIRE(d.category == NF::HostedToolCategory::SceneEditing);
    REQUIRE(d.displayName == "Scene Editor");
    REQUIRE(d.acceptsProjectExtensions);
}

TEST_CASE("SceneEditorTool: declares expected shared panels", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 4);

    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    REQUIRE(has("panel.viewport"));
    REQUIRE(has("panel.outliner"));
    REQUIRE(has("panel.inspector"));
    REQUIRE(has("panel.console"));
}

TEST_CASE("SceneEditorTool: declares expected commands", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 6);

    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    REQUIRE(has("scene.create_entity"));
    REQUIRE(has("scene.delete_entity"));
    REQUIRE(has("scene.save_scene"));
    REQUIRE(has("scene.enter_play"));
    REQUIRE(has("scene.exit_play"));
}

// ── SceneEditorTool lifecycle ─────────────────────────────────────

TEST_CASE("SceneEditorTool: initial state is Unloaded", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    REQUIRE(tool.state() == NF::HostedToolState::Unloaded);
}

TEST_CASE("SceneEditorTool: initialize transitions to Ready", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    REQUIRE(tool.initialize());
    REQUIRE(tool.state() == NF::HostedToolState::Ready);
}

TEST_CASE("SceneEditorTool: double initialize returns false", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    REQUIRE(tool.initialize());
    REQUIRE_FALSE(tool.initialize());
    REQUIRE(tool.state() == NF::HostedToolState::Ready);
}

TEST_CASE("SceneEditorTool: activate transitions Ready to Active", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    REQUIRE(tool.state() == NF::HostedToolState::Active);
}

TEST_CASE("SceneEditorTool: suspend transitions Active to Suspended", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    REQUIRE(tool.state() == NF::HostedToolState::Suspended);
}

TEST_CASE("SceneEditorTool: activate from Suspended transitions to Active", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    REQUIRE(tool.state() == NF::HostedToolState::Active);
}

TEST_CASE("SceneEditorTool: shutdown resets to Unloaded", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    REQUIRE(tool.state() == NF::HostedToolState::Unloaded);
}

TEST_CASE("SceneEditorTool: update while Active records frame time", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.update(0.016f);
    REQUIRE(tool.stats().lastFrameMs > 0.0f);
}

TEST_CASE("SceneEditorTool: update while Suspended does not change frame time", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.update(0.016f);
    tool.suspend();
    tool.update(0.032f);
    // Should not have been updated while suspended
    REQUIRE(tool.stats().lastFrameMs == Catch::Approx(16.0f));
}

// ── SceneEditorTool scene state ───────────────────────────────────

TEST_CASE("SceneEditorTool: default edit mode is Select", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    REQUIRE(tool.editMode() == NF::SceneEditMode::Select);
    REQUIRE(std::string(NF::sceneEditModeName(tool.editMode())) == "Select");
}

TEST_CASE("SceneEditorTool: setEditMode changes mode", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.setEditMode(NF::SceneEditMode::Translate);
    REQUIRE(tool.editMode() == NF::SceneEditMode::Translate);
    tool.setEditMode(NF::SceneEditMode::Rotate);
    REQUIRE(tool.editMode() == NF::SceneEditMode::Rotate);
}

TEST_CASE("SceneEditorTool: SceneEditMode names cover all 6 values", "[scene_editor][phase3]") {
    REQUIRE(std::string(NF::sceneEditModeName(NF::SceneEditMode::Select))    == "Select");
    REQUIRE(std::string(NF::sceneEditModeName(NF::SceneEditMode::Translate)) == "Translate");
    REQUIRE(std::string(NF::sceneEditModeName(NF::SceneEditMode::Rotate))    == "Rotate");
    REQUIRE(std::string(NF::sceneEditModeName(NF::SceneEditMode::Scale))     == "Scale");
    REQUIRE(std::string(NF::sceneEditModeName(NF::SceneEditMode::Paint))     == "Paint");
    REQUIRE(std::string(NF::sceneEditModeName(NF::SceneEditMode::Play))      == "Play");
}

TEST_CASE("SceneEditorTool: dirty flag", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    REQUIRE_FALSE(tool.isDirty());
    tool.markDirty();
    REQUIRE(tool.isDirty());
    tool.clearDirty();
    REQUIRE_FALSE(tool.isDirty());
}

TEST_CASE("SceneEditorTool: selection count tracking", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    REQUIRE(tool.selectionCount() == 0);
    tool.setSelectionCount(5);
    REQUIRE(tool.selectionCount() == 5);
    REQUIRE(tool.stats().selectionCount == 5);
}

TEST_CASE("SceneEditorTool: entity count tracking", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    REQUIRE(tool.stats().entityCount == 0);
    tool.setEntityCount(128);
    REQUIRE(tool.stats().entityCount == 128);
}

TEST_CASE("SceneEditorTool: shutdown resets dirty and counts", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.setEntityCount(10);
    tool.setSelectionCount(3);
    tool.markDirty();
    tool.shutdown();
    REQUIRE(tool.stats().entityCount == 0);
    REQUIRE(tool.stats().selectionCount == 0);
    REQUIRE_FALSE(tool.stats().isDirty);
}

// ── SceneEditorTool project adapter hooks ─────────────────────────

TEST_CASE("SceneEditorTool: onProjectLoaded seeds default scene stats", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.setEntityCount(99);
    tool.markDirty();
    tool.onProjectLoaded("my_project");
    // onProjectLoaded resets dirty state and seeds a default starter scene
    // (Camera_Main, DirectionalLight, Player, Environment, SkyDome).
    REQUIRE(tool.stats().entityCount == 5);
    REQUIRE_FALSE(tool.isDirty());
}

TEST_CASE("SceneEditorTool: onProjectUnloaded clears stats", "[scene_editor][phase3]") {
    NF::SceneEditorTool tool;
    tool.initialize();
    tool.onProjectLoaded("proj");
    tool.setEntityCount(50);
    tool.onProjectUnloaded();
    REQUIRE(tool.stats().entityCount == 0);
}

// ── SceneEditorTool registered with WorkspaceShell ────────────────

TEST_CASE("SceneEditorTool: registers with ToolRegistry", "[scene_editor][phase3][integration]") {
    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::SceneEditorTool>());
    REQUIRE(registry.count() == 1);
    REQUIRE(registry.isRegistered(NF::SceneEditorTool::kToolId));
    auto* tool = registry.find(NF::SceneEditorTool::kToolId);
    REQUIRE(tool != nullptr);
    REQUIRE(tool->descriptor().isPrimary);
}

TEST_CASE("SceneEditorTool: full lifecycle through ToolRegistry", "[scene_editor][phase3][integration]") {
    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::SceneEditorTool>());
    registry.initializeAll();
    REQUIRE(registry.find(NF::SceneEditorTool::kToolId)->state() == NF::HostedToolState::Ready);

    registry.activateTool(NF::SceneEditorTool::kToolId);
    REQUIRE(registry.find(NF::SceneEditorTool::kToolId)->state() == NF::HostedToolState::Active);
    REQUIRE(registry.activeToolId() == NF::SceneEditorTool::kToolId);

    registry.updateActive(0.016f);

    registry.shutdownAll();
    REQUIRE(registry.find(NF::SceneEditorTool::kToolId)->state() == NF::HostedToolState::Unloaded);
}

TEST_CASE("SceneEditorTool: in WorkspaceShell via ToolRegistry", "[scene_editor][phase3][integration]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(std::make_unique<NF::SceneEditorTool>());
    shell.initialize();

    REQUIRE(shell.toolRegistry().isRegistered(NF::SceneEditorTool::kToolId));
    shell.toolRegistry().activateTool(NF::SceneEditorTool::kToolId);
    REQUIRE(shell.toolRegistry().activeToolId() == NF::SceneEditorTool::kToolId);

    shell.update(0.016f);
    shell.shutdown();
}

TEST_CASE("SceneEditorTool: byCategory returns it under SceneEditing", "[scene_editor][phase3][integration]") {
    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::SceneEditorTool>());
    registry.initializeAll();
    auto tools = registry.byCategory(NF::HostedToolCategory::SceneEditing);
    REQUIRE(tools.size() == 1);
    REQUIRE(tools[0]->toolId() == NF::SceneEditorTool::kToolId);
}

TEST_CASE("SceneEditorTool: primaryTools includes it", "[scene_editor][phase3][integration]") {
    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::SceneEditorTool>());
    registry.initializeAll();
    auto primaries = registry.primaryTools();
    REQUIRE(!primaries.empty());
    REQUIRE(primaries[0]->descriptor().isPrimary);
}

TEST_CASE("SceneEditorTool: project events propagate through WorkspaceShell", "[scene_editor][phase3][integration]") {
    NF::WorkspaceShell shell;
    shell.toolRegistry().registerTool(std::make_unique<NF::SceneEditorTool>());
    shell.initialize();

    shell.toolRegistry().notifyProjectLoaded("atlas_project");
    shell.toolRegistry().notifyProjectUnloaded();

    shell.shutdown();
}

// ═══════════════════════════════════════════════════════════════════
// ISharedPanel + PanelRegistry factory tests
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("PanelRegistry: registerPanelWithFactory creates entry", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    bool ok = reg.registerPanelWithFactory(
        {"test_panel", "Test Panel", NF::SharedPanelCategory::Output},
        [] { return std::make_unique<NF::DiagnosticsSharedPanel>(); });
    REQUIRE(ok);
    REQUIRE(reg.isRegistered("test_panel"));
    REQUIRE(reg.hasFactory("test_panel"));
    REQUIRE_FALSE(reg.hasInstance("test_panel"));
}

TEST_CASE("PanelRegistry: getOrCreatePanel creates instance on demand", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    reg.registerPanelWithFactory(
        {"diagnostics", "Diagnostics", NF::SharedPanelCategory::Output},
        [] { return std::make_unique<NF::DiagnosticsSharedPanel>(); });

    REQUIRE_FALSE(reg.hasInstance("diagnostics"));
    auto* panel = reg.getOrCreatePanel("diagnostics");
    REQUIRE(panel != nullptr);
    REQUIRE(panel->panelId() == "diagnostics");
    REQUIRE(panel->displayName() == "Diagnostics");
    REQUIRE(reg.hasInstance("diagnostics"));
}

TEST_CASE("PanelRegistry: getOrCreatePanel returns same instance", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    reg.registerPanelWithFactory(
        {"mem", "Memory", NF::SharedPanelCategory::Output},
        [] { return std::make_unique<NF::MemoryProfilerSharedPanel>(); });

    auto* p1 = reg.getOrCreatePanel("mem");
    auto* p2 = reg.getOrCreatePanel("mem");
    REQUIRE(p1 == p2);
}

TEST_CASE("PanelRegistry: getOrCreatePanel returns nullptr for no factory", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    reg.registerPanel({"inspector", "Inspector", NF::SharedPanelCategory::Inspector});
    REQUIRE(reg.getOrCreatePanel("inspector") == nullptr);
    REQUIRE(reg.getOrCreatePanel("nonexistent") == nullptr);
}

TEST_CASE("PanelRegistry: shutdownAll destroys panel instances", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    reg.registerPanelWithFactory(
        {"diag", "Diag", NF::SharedPanelCategory::Output},
        [] { return std::make_unique<NF::DiagnosticsSharedPanel>(); });
    auto* panel = reg.getOrCreatePanel("diag");
    REQUIRE(panel != nullptr);
    REQUIRE(reg.hasInstance("diag"));

    reg.shutdownAll();
    REQUIRE_FALSE(reg.hasInstance("diag"));
}

TEST_CASE("PanelRegistry: notifyToolActivated propagates to instances", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    reg.registerPanelWithFactory(
        {"cb", "Content Browser", NF::SharedPanelCategory::Navigation},
        [] { return std::make_unique<NF::ContentBrowserSharedPanel>(); });

    auto* panel = reg.getOrCreatePanel("cb");
    REQUIRE(panel != nullptr);
    reg.notifyToolActivated("workspace.scene_editor");
    // ContentBrowserSharedPanel stores tool id internally
    auto* cbPanel = dynamic_cast<NF::ContentBrowserSharedPanel*>(panel);
    REQUIRE(cbPanel != nullptr);
}

TEST_CASE("PanelRegistry: notifySelectionChanged propagates to instances", "[workspace][panel][shared]") {
    NF::PanelRegistry reg;
    reg.registerPanelWithFactory(
        {"ci", "Component Inspector", NF::SharedPanelCategory::Inspector},
        [] { return std::make_unique<NF::ComponentInspectorSharedPanel>(); });

    auto* panel = reg.getOrCreatePanel("ci");
    auto* ciPanel = dynamic_cast<NF::ComponentInspectorSharedPanel*>(panel);
    REQUIRE(ciPanel != nullptr);
    REQUIRE_FALSE(ciPanel->isDirty());
    reg.notifySelectionChanged();
    REQUIRE(ciPanel->isDirty());
}

// ── ContentBrowserSharedPanel ────────────────────────────────────

TEST_CASE("ContentBrowserSharedPanel: lifecycle", "[workspace][panel][content_browser]") {
    NF::ContentBrowserSharedPanel panel;
    REQUIRE(panel.panelId() == "content_browser");
    REQUIRE(panel.displayName() == "Content Browser");
    REQUIRE_FALSE(panel.isInitialized());

    REQUIRE(panel.initialize());
    REQUIRE(panel.isInitialized());
    REQUIRE(panel.currentPath() == "/");

    panel.setCurrentPath("/assets/textures");
    REQUIRE(panel.currentPath() == "/assets/textures");

    panel.navigateUp();
    REQUIRE(panel.currentPath() == "/assets");

    panel.shutdown();
    REQUIRE_FALSE(panel.isInitialized());
}

// ── ComponentInspectorSharedPanel ────────────────────────────────

TEST_CASE("ComponentInspectorSharedPanel: lifecycle", "[workspace][panel][component_inspector]") {
    NF::ComponentInspectorSharedPanel panel;
    REQUIRE(panel.panelId() == "component_inspector");
    REQUIRE(panel.displayName() == "Component Inspector");
    REQUIRE_FALSE(panel.isInitialized());

    REQUIRE(panel.initialize());
    REQUIRE(panel.isInitialized());
    REQUIRE(panel.selectedEntityId() == 0);

    panel.selectEntity(42);
    REQUIRE(panel.selectedEntityId() == 42);
    REQUIRE(panel.isDirty());

    panel.clearDirty();
    REQUIRE_FALSE(panel.isDirty());

    panel.onSelectionChanged();
    REQUIRE(panel.isDirty());

    panel.shutdown();
    REQUIRE_FALSE(panel.isInitialized());
    REQUIRE(panel.selectedEntityId() == 0);
}

// ── DiagnosticsSharedPanel ───────────────────────────────────────

TEST_CASE("DiagnosticsSharedPanel: lifecycle", "[workspace][panel][diagnostics]") {
    NF::DiagnosticsSharedPanel panel;
    REQUIRE(panel.panelId() == "diagnostics");
    REQUIRE(panel.displayName() == "Diagnostics");

    REQUIRE(panel.initialize());
    REQUIRE(panel.entryCount() == 0);

    panel.addEntry({"test warning", NF::DiagnosticsSharedPanel::DiagLevel::Warning, "test"});
    panel.addEntry({"test error",   NF::DiagnosticsSharedPanel::DiagLevel::Error,   "test"});
    panel.addEntry({"test info",    NF::DiagnosticsSharedPanel::DiagLevel::Info,    "test"});

    REQUIRE(panel.entryCount() == 3);
    REQUIRE(panel.countByLevel(NF::DiagnosticsSharedPanel::DiagLevel::Warning) == 1);
    REQUIRE(panel.countByLevel(NF::DiagnosticsSharedPanel::DiagLevel::Error) == 1);
    REQUIRE(panel.countByLevel(NF::DiagnosticsSharedPanel::DiagLevel::Info) == 1);

    panel.clearEntries();
    REQUIRE(panel.entryCount() == 0);

    panel.shutdown();
}

// ── MemoryProfilerSharedPanel ────────────────────────────────────

TEST_CASE("MemoryProfilerSharedPanel: lifecycle", "[workspace][panel][memory_profiler]") {
    NF::MemoryProfilerSharedPanel panel;
    REQUIRE(panel.panelId() == "memory_profiler");
    REQUIRE(panel.displayName() == "Memory Profiler");

    REQUIRE(panel.initialize());
    REQUIRE(panel.totalAllocated() == 0);
    REQUIRE(panel.allocationCount() == 0);
    REQUIRE(panel.deallocationCount() == 0);

    panel.recordAllocation(1024);
    panel.recordAllocation(512);
    REQUIRE(panel.totalAllocated() == 1536);
    REQUIRE(panel.allocationCount() == 2);

    panel.recordDeallocation(512);
    REQUIRE(panel.totalAllocated() == 1024);
    REQUIRE(panel.deallocationCount() == 1);

    panel.update(0.016f);
    REQUIRE(panel.frameCount() == 1);

    panel.shutdown();
}

// ── PipelineMonitorSharedPanel ───────────────────────────────────

TEST_CASE("PipelineMonitorSharedPanel: lifecycle", "[workspace][panel][pipeline_monitor]") {
    NF::PipelineMonitorSharedPanel panel;
    REQUIRE(panel.panelId() == "pipeline_monitor");
    REQUIRE(panel.displayName() == "Pipeline Monitor");

    REQUIRE(panel.initialize());
    REQUIRE(panel.stageCount() == 0);

    panel.addStage({"compile", NF::PipelineMonitorSharedPanel::StageStatus::Idle, 0.f});
    panel.addStage({"link",    NF::PipelineMonitorSharedPanel::StageStatus::Idle, 0.f});
    REQUIRE(panel.stageCount() == 2);

    REQUIRE(panel.setStageStatus("compile", NF::PipelineMonitorSharedPanel::StageStatus::Running));
    REQUIRE(panel.setStageProgress("compile", 0.5f));
    REQUIRE(panel.stages()[0].status == NF::PipelineMonitorSharedPanel::StageStatus::Running);
    REQUIRE(panel.stages()[0].progress == 0.5f);

    REQUIRE_FALSE(panel.setStageStatus("nonexistent", NF::PipelineMonitorSharedPanel::StageStatus::Failed));

    panel.clearStages();
    REQUIRE(panel.stageCount() == 0);

    panel.shutdown();
}

// ── NotificationCenterSharedPanel ────────────────────────────────

TEST_CASE("NotificationCenterSharedPanel: lifecycle", "[workspace][panel][notification_center]") {
    NF::NotificationCenterSharedPanel panel;
    REQUIRE(panel.panelId() == "notification_center");
    REQUIRE(panel.displayName() == "Notification Center");

    REQUIRE(panel.initialize());

    NF::NotifEntry e1(1, "Build complete", NF::NotifChannel::InEditor, NF::NotifPriority::Normal);
    NF::NotifEntry e2(2, "Error detected", NF::NotifChannel::InEditor, NF::NotifPriority::High);

    REQUIRE(panel.editor().addNotif(e1));
    REQUIRE(panel.editor().addNotif(e2));
    REQUIRE(panel.editor().notifCount() == 2);
    REQUIRE(panel.editor().countUnread() == 2);

    auto* found = panel.editor().findNotif(1);
    REQUIRE(found != nullptr);
    found->setIsRead(true);
    REQUIRE(panel.editor().countUnread() == 1);

    REQUIRE(panel.editor().removeNotif(2));
    REQUIRE(panel.editor().notifCount() == 1);

    panel.shutdown();
}

// ── WorkspaceShell integration with shared panels ────────────────

TEST_CASE("WorkspaceShell: factory panels are created on getOrCreatePanel", "[workspace][shell][shared]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    // Before creation
    REQUIRE_FALSE(shell.panelRegistry().hasInstance("content_browser"));
    REQUIRE(shell.panelRegistry().hasFactory("content_browser"));

    // Create on demand
    auto* panel = shell.panelRegistry().getOrCreatePanel("content_browser");
    REQUIRE(panel != nullptr);
    REQUIRE(panel->panelId() == "content_browser");
    REQUIRE(shell.panelRegistry().hasInstance("content_browser"));

    shell.shutdown();
    // After shutdown, instances are destroyed
    REQUIRE_FALSE(shell.panelRegistry().hasInstance("content_browser"));
}

TEST_CASE("WorkspaceShell: all 6 factory panels can be created", "[workspace][shell][shared]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    const char* factoryPanels[] = {
        "content_browser", "component_inspector", "diagnostics",
        "memory_profiler", "pipeline_monitor", "notification_center"
    };

    for (const char* id : factoryPanels) {
        REQUIRE(shell.panelRegistry().hasFactory(id));
        auto* panel = shell.panelRegistry().getOrCreatePanel(id);
        REQUIRE(panel != nullptr);
        REQUIRE(panel->panelId() == id);
    }

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: descriptor-only panels have no factory", "[workspace][shell][shared]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    REQUIRE_FALSE(shell.panelRegistry().hasFactory("inspector"));
    REQUIRE_FALSE(shell.panelRegistry().hasFactory("outliner"));
    REQUIRE_FALSE(shell.panelRegistry().hasFactory("console"));
    REQUIRE(shell.panelRegistry().getOrCreatePanel("inspector") == nullptr);

    shell.shutdown();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Audit fix: Undo/Redo wiring through command bus
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell: undo/redo commands route through UndoRedoSystem", "[workspace][shell][undo]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    // Push an undo group
    NF::UndoGroup group("Test Action");
    NF::UndoAction action;
    action.id          = "a1";
    action.description = "Move entity";
    action.type        = NF::UndoActionType::Move;
    action.state       = NF::UndoActionState::Applied;
    group.addAction(action);
    REQUIRE(shell.undoRedoSystem().pushGroup(std::move(group)));
    REQUIRE(shell.undoRedoSystem().canUndo());

    // Execute workspace.undo through command bus
    auto result = shell.commandBus().execute("workspace.undo");
    REQUIRE(result == NF::ConsoleCmdExecResult::Ok);
    REQUIRE_FALSE(shell.undoRedoSystem().canUndo());
    REQUIRE(shell.undoRedoSystem().canRedo());

    // Execute workspace.redo through command bus
    result = shell.commandBus().execute("workspace.redo");
    REQUIRE(result == NF::ConsoleCmdExecResult::Ok);
    REQUIRE(shell.undoRedoSystem().canUndo());
    REQUIRE_FALSE(shell.undoRedoSystem().canRedo());

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: undo on empty stack succeeds with no-op", "[workspace][shell][undo]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    REQUIRE_FALSE(shell.undoRedoSystem().canUndo());
    auto result = shell.commandBus().execute("workspace.undo");
    REQUIRE(result == NF::ConsoleCmdExecResult::Ok);

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: undoRedoSystem accessor exists", "[workspace][shell][undo]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE_FALSE(shell.undoRedoSystem().canUndo());
    REQUIRE_FALSE(shell.undoRedoSystem().canRedo());
    REQUIRE(shell.undoRedoSystem().undoDepth() == 0);
    shell.shutdown();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Audit fix: Close document wiring
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell: close_document closes active document via projectState", "[workspace][shell][close]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    struct TestAdapter final : NF::IGameProjectAdapter {
        std::string projectId()          const override { return "test_close"; }
        std::string projectDisplayName() const override { return "Test Close"; }
        bool initialize() override { return true; }
        void shutdown()   override {}
        std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }
    };
    shell.loadProject(std::make_unique<TestAdapter>());

    NF::OpenDocumentEntry entry;
    entry.documentId   = "doc1";
    entry.displayTitle = "My Scene";
    entry.kind         = NF::DocumentKind::Scene;
    shell.projectState().openDocument(entry);
    REQUIRE(shell.projectState().openDocumentCount() == 1);
    REQUIRE(shell.projectState().activeDocumentId()  == "doc1");

    auto result = shell.commandBus().execute("workspace.close_document");
    REQUIRE(result == NF::ConsoleCmdExecResult::Ok);
    REQUIRE(shell.projectState().openDocumentCount() == 0);

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: close_document with no active doc succeeds", "[workspace][shell][close]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    struct TestAdapter final : NF::IGameProjectAdapter {
        std::string projectId()          const override { return "test_close2"; }
        std::string projectDisplayName() const override { return "Test Close 2"; }
        bool initialize() override { return true; }
        void shutdown()   override {}
        std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }
    };
    shell.loadProject(std::make_unique<TestAdapter>());

    auto result = shell.commandBus().execute("workspace.close_document");
    REQUIRE(result == NF::ConsoleCmdExecResult::Ok);

    shell.shutdown();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Audit fix: Recent files recording on project load
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell: loadProject records in recent files", "[workspace][shell][recent]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    // Clear any entries persisted from prior test runs.
    shell.recentFiles().clearAll();

    struct TestAdapter final : NF::IGameProjectAdapter {
        std::string projectId()          const override { return "recents_test"; }
        std::string projectDisplayName() const override { return "Recents Test"; }
        std::string projectRoot()        const override { return "/tmp/recents_test"; }
        bool initialize() override { return true; }
        void shutdown()   override {}
        std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }
    };

    REQUIRE(shell.recentFiles().globalRecent().empty());
    shell.loadProject(std::make_unique<TestAdapter>());

    auto recent = shell.recentFiles().globalRecent();
    REQUIRE(recent.size() >= 1);
    REQUIRE(recent[0].displayName == "Recents Test");
    REQUIRE(recent[0].path        == "/tmp/recents_test");
    REQUIRE(recent[0].kind        == NF::RecentFileKind::Project);

    shell.shutdown();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Audit fix: Preferences and project settings panels
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell: preferences panel toggles via command", "[workspace][shell][prefs]") {
    NF::WorkspaceShell shell;
    shell.initialize();

    REQUIRE(shell.panelRegistry().isRegistered("preferences"));
    REQUIRE_FALSE(shell.panelRegistry().isPanelVisible("preferences"));

    (void)shell.commandBus().execute("workspace.preferences");
    REQUIRE(shell.panelRegistry().isPanelVisible("preferences"));

    (void)shell.commandBus().execute("workspace.preferences");
    REQUIRE_FALSE(shell.panelRegistry().isPanelVisible("preferences"));

    shell.shutdown();
}

TEST_CASE("WorkspaceShell: project_settings panel registered", "[workspace][shell][prefs]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE(shell.panelRegistry().isRegistered("project_settings"));
    REQUIRE_FALSE(shell.panelRegistry().isPanelVisible("project_settings"));
    shell.shutdown();
}
