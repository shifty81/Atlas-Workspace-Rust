// Tests/Workspace/test_workspace_wiring.cpp
//
// Tests for the runtime wiring connections introduced in the workspace
// binding patch:
//
//   1. WorkspaceShell::loadProject() populates a real ProjectLoadContract
//   2. ProjectSystemsTool::getOrCreatePanel() calls onProjectLoaded on
//      lazily-created panels when a project is already loaded
//   3. notifyProjectLoaded stores the project root and clears on unload
//   4. IEditorPanel::summaryRows() virtual interface
//   5. NovaForge panel summaryRows() overrides return real data
//   6. WorkspaceRenderer renderProjectPanelView uses summaryRows (smoke only)

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceProjectState.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/ProjectSystemsTool.h"
#include "NF/Workspace/ProjectLoadContract.h"
#include "NF/Workspace/IGameProjectAdapter.h"
#include "NovaForge/EditorAdapter/Panels/EconomyPanel.h"
#include "NovaForge/EditorAdapter/Panels/InventoryRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/MissionRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ProgressionPanel.h"
#include "NovaForge/EditorAdapter/Panels/ShopPanel.h"
#include "NovaForge/EditorAdapter/Panels/CharacterRulesPanel.h"

using namespace NF;

// ── Stub adapter for wiring tests ────────────────────────────────────────

namespace {

struct WiringPanelTracker {
    std::string lastProjectRoot;
    bool        unloaded = false;
};

// A minimal IEditorPanel that tracks onProjectLoaded / onProjectUnloaded calls.
class TrackingPanel final : public IEditorPanel {
public:
    explicit TrackingPanel(WiringPanelTracker& tracker)
        : m_tracker(tracker) {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id;    }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    void onProjectLoaded(const std::string& root) override {
        m_tracker.lastProjectRoot = root;
        m_tracker.unloaded = false;
    }
    void onProjectUnloaded() override {
        m_tracker.unloaded = true;
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        return {{"Root", m_tracker.lastProjectRoot}};
    }

private:
    std::string m_id    = "test.tracking_panel";
    std::string m_title = "Tracking Panel";
    WiringPanelTracker& m_tracker;
};

class WiringStubAdapter final : public IGameProjectAdapter {
public:
    explicit WiringStubAdapter(WiringPanelTracker& tracker,
                                std::string root = "/proj/root")
        : m_tracker(tracker), m_root(std::move(root)) {}

    [[nodiscard]] std::string projectId()          const override { return "wiring.test"; }
    [[nodiscard]] std::string projectDisplayName() const override { return "Wiring Test"; }
    bool initialize() override { return true; }
    void shutdown()   override {}

    [[nodiscard]] std::vector<std::string> contentRoots() const override { return {m_root}; }
    [[nodiscard]] std::vector<std::string> customCommands() const override {
        return {"cmd.build", "cmd.run"};
    }

    [[nodiscard]] std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const override {
        GameplaySystemPanelDescriptor d;
        d.id          = "test.tracking_panel";
        d.displayName = "Tracking Panel";
        d.hostToolId  = "workspace.project_systems";
        d.category    = "Core";
        d.enabled     = true;
        d.createPanel = [this]() -> std::unique_ptr<IEditorPanel> {
            return std::make_unique<TrackingPanel>(m_tracker);
        };
        return {d};
    }

private:
    WiringPanelTracker& m_tracker;
    std::string         m_root;
};

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// 1. ProjectLoadContract is fully populated by loadProject()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("loadProject: contract state is Ready", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker)));

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.state == ProjectLoadState::Ready);
    REQUIRE(c.isLoaded());

    shell.shutdown();
}

TEST_CASE("loadProject: contract carries projectId and displayName", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker)));

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.projectId          == "wiring.test");
    REQUIRE(c.projectDisplayName == "Wiring Test");

    shell.shutdown();
}

TEST_CASE("loadProject: contract carries contentRoots", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker, "/some/root")));

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.contentRoots.size() == 1);
    REQUIRE(c.contentRoots[0] == "/some/root");

    shell.shutdown();
}

TEST_CASE("loadProject: contract carries customCommands", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker)));

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.customCommands.size() == 2);

    shell.shutdown();
}

TEST_CASE("loadProject: contract carries panelCount", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker)));

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.panelCount == 1);

    shell.shutdown();
}

TEST_CASE("loadProject: contract timestamp is non-zero", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker)));

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.loadTimestampMs > 0);

    shell.shutdown();
}

TEST_CASE("unloadProject: contract resets to Unloaded", "[wiring][contract]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    shell.loadProject(std::make_unique<WiringStubAdapter>(tracker));
    shell.unloadProject();

    const ProjectLoadContract& c = shell.projectState().loadContract();
    REQUIRE(c.state == ProjectLoadState::Unloaded);
    REQUIRE_FALSE(c.isLoaded());

    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. ProjectSystemsTool panel lazy-creation calls onProjectLoaded
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ProjectSystemsTool: lazily-created panel receives onProjectLoaded", "[wiring][panel]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    REQUIRE(shell.loadProject(std::make_unique<WiringStubAdapter>(tracker, "/lazy/root")));

    // Panel is created lazily on first getOrCreatePanel() call.
    // It should immediately receive onProjectLoaded("/lazy/root").
    IEditorPanel* panel = shell.projectSystemsTool().getOrCreatePanel("test.tracking_panel");
    REQUIRE(panel != nullptr);
    REQUIRE(tracker.lastProjectRoot == "/lazy/root");
    REQUIRE_FALSE(tracker.unloaded);

    shell.shutdown();
}

TEST_CASE("ProjectSystemsTool: panel receives onProjectUnloaded when project unloads", "[wiring][panel]") {
    WorkspaceShell shell;
    shell.initialize();

    WiringPanelTracker tracker;
    shell.loadProject(std::make_unique<WiringStubAdapter>(tracker, "/proj/x"));
    // Instantiate the panel first
    shell.projectSystemsTool().getOrCreatePanel("test.tracking_panel");
    REQUIRE_FALSE(tracker.unloaded);

    shell.unloadProject();
    REQUIRE(tracker.unloaded);

    shell.shutdown();
}

TEST_CASE("ProjectSystemsTool: notifyProjectLoaded stores project root", "[wiring][panel]") {
    ProjectSystemsTool tool;

    // Build a descriptor with a tracking factory.
    WiringPanelTracker tracker;
    GameplaySystemPanelDescriptor d;
    d.id          = "track";
    d.displayName = "Track";
    d.createPanel = [&]() -> std::unique_ptr<IEditorPanel> {
        return std::make_unique<TrackingPanel>(tracker);
    };

    // Simulate adapter load then project loaded notification.
    struct TinyAdapter : IGameProjectAdapter {
        std::vector<GameplaySystemPanelDescriptor> descs;
        std::string projectId() const override { return "t"; }
        std::string projectDisplayName() const override { return "T"; }
        bool initialize() override { return true; }
        void shutdown() override {}
        std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const override { return descs; }
    };
    TinyAdapter adapter;
    adapter.descs.push_back(d);
    tool.loadFromAdapter(adapter);

    // notifyProjectLoaded should store the root.
    tool.notifyProjectLoaded("/stored/root");

    // A panel created after notifyProjectLoaded gets the stored root.
    IEditorPanel* panel = tool.getOrCreatePanel("track");
    REQUIRE(panel != nullptr);
    REQUIRE(tracker.lastProjectRoot == "/stored/root");
}

TEST_CASE("ProjectSystemsTool: notifyProjectUnloaded clears stored root", "[wiring][panel]") {
    ProjectSystemsTool tool;

    WiringPanelTracker tracker;
    GameplaySystemPanelDescriptor d;
    d.id          = "track";
    d.displayName = "Track";
    d.createPanel = [&]() -> std::unique_ptr<IEditorPanel> {
        return std::make_unique<TrackingPanel>(tracker);
    };

    struct TinyAdapter : IGameProjectAdapter {
        std::vector<GameplaySystemPanelDescriptor> descs;
        std::string projectId() const override { return "t"; }
        std::string projectDisplayName() const override { return "T"; }
        bool initialize() override { return true; }
        void shutdown() override {}
        std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const override { return descs; }
    };
    TinyAdapter adapter;
    adapter.descs.push_back(d);
    tool.loadFromAdapter(adapter);

    tool.notifyProjectLoaded("/stored/root");
    tool.notifyProjectUnloaded();

    // After unload, a newly-created panel should NOT get onProjectLoaded.
    tool.reset();
    // Reset tracker so state from the first cycle's pre-instantiated panel
    // does not bleed into the second cycle's assertions.
    tracker.lastProjectRoot.clear();
    tracker.unloaded = false;
    // adapter.descs already contains d from the first push; reload without adding again.
    tool.loadFromAdapter(adapter);

    IEditorPanel* panel = tool.getOrCreatePanel("track");
    REQUIRE(panel != nullptr);
    // Root should be empty since no project is loaded after unload.
    REQUIRE(tracker.lastProjectRoot.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. IEditorPanel summaryRows() — base default returns empty
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("IEditorPanel: default summaryRows returns empty", "[wiring][summaryRows]") {
    class MinimalPanel final : public IEditorPanel {
        std::string m_id{"min"}, m_title{"Min"};
    public:
        [[nodiscard]] const std::string& panelId()    const override { return m_id;    }
        [[nodiscard]] const std::string& panelTitle() const override { return m_title; }
    };
    MinimalPanel p;
    REQUIRE(p.summaryRows().empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. NovaForge panel summaryRows() return non-empty real data after defaults
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("EconomyPanel: summaryRows returns non-empty after defaults", "[wiring][summaryRows][novaforge]") {
    NovaForge::EconomyPanel p;
    p.onProjectLoaded("/no/such/path"); // will fall back to defaults
    auto rows = p.summaryRows();
    REQUIRE_FALSE(rows.empty());
    // Ensure there is a "Currencies" row.
    bool found = false;
    for (const auto& [k, v] : rows)
        if (k == "Currencies") { found = true; break; }
    REQUIRE(found);
}

TEST_CASE("InventoryRulesPanel: summaryRows returns non-empty after defaults", "[wiring][summaryRows][novaforge]") {
    NovaForge::InventoryRulesPanel p;
    p.onProjectLoaded("/no/such/path");
    REQUIRE_FALSE(p.summaryRows().empty());
}

TEST_CASE("MissionRulesPanel: summaryRows returns non-empty after defaults", "[wiring][summaryRows][novaforge]") {
    NovaForge::MissionRulesPanel p;
    p.onProjectLoaded("/no/such/path");
    REQUIRE_FALSE(p.summaryRows().empty());
}

TEST_CASE("ProgressionPanel: summaryRows returns non-empty after defaults", "[wiring][summaryRows][novaforge]") {
    NovaForge::ProgressionPanel p;
    p.onProjectLoaded("/no/such/path");
    REQUIRE_FALSE(p.summaryRows().empty());
}

TEST_CASE("ShopPanel: summaryRows returns non-empty after defaults", "[wiring][summaryRows][novaforge]") {
    NovaForge::ShopPanel p;
    p.onProjectLoaded("/no/such/path");
    REQUIRE_FALSE(p.summaryRows().empty());
}

TEST_CASE("CharacterRulesPanel: summaryRows returns non-empty after defaults", "[wiring][summaryRows][novaforge]") {
    NovaForge::CharacterRulesPanel p;
    p.onProjectLoaded("/no/such/path");
    REQUIRE_FALSE(p.summaryRows().empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. TrackingPanel summaryRows() reflects the project root it received
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("TrackingPanel: summaryRows shows project root", "[wiring][summaryRows]") {
    WiringPanelTracker tracker;
    TrackingPanel p(tracker);
    p.onProjectLoaded("/test/root");
    auto rows = p.summaryRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].first  == "Root");
    REQUIRE(rows[0].second == "/test/root");
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. Viewport manager exists and can allocate a slot
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceShell: viewportManager can allocate a viewport slot", "[wiring][viewport]") {
    WorkspaceShell shell;
    shell.initialize();

    ViewportHandle vh = shell.viewportManager().requestViewport(
        "test.tool", {0.f, 0.f, 1280.f, 800.f});
    REQUIRE(vh != kInvalidViewportHandle);

    REQUIRE(shell.viewportManager().activateViewport(vh));
    REQUIRE(shell.viewportManager().releaseViewport(vh));

    shell.shutdown();
}
