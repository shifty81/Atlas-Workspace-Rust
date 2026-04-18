// Phase 5 — Hosted Project Support
//
// Tests for:
//   - ProjectLoadContract (state machine, validation, build-readiness)
//   - ProjectRegistry (factory model, load/unload lifecycle, multi-project model)
//   - BuildGateController (gate rules, blocking error accumulation, status codes)
//   - Integration: shell-style project loading flow end-to-end

#include <catch2/catch_test_macros.hpp>

#include "NF/Workspace/ProjectLoadContract.h"
#include "NF/Workspace/ProjectRegistry.h"
#include "NF/Workspace/BuildGateController.h"
#include "NF/Workspace/IGameProjectAdapter.h"

#include <memory>
#include <string>
#include <vector>

// ── Minimal test adapters ─────────────────────────────────────────────────

namespace {

struct OkAdapter final : public NF::IGameProjectAdapter {
    std::string projectId()          const override { return "ok_project"; }
    std::string projectDisplayName() const override { return "OK Project"; }
    bool  initialize() override { return true; }
    void  shutdown()   override {}
    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        NF::GameplaySystemPanelDescriptor d;
        d.id          = "ok_project.panel_a";
        d.displayName = "Panel A";
        d.hostToolId  = "workspace.scene";
        d.category    = "Test";
        d.projectId   = "ok_project";
        return {d};
    }
    std::vector<std::string> contentRoots()    const override { return {"Content/"}; }
    std::vector<std::string> customCommands()  const override { return {"ok.build"}; }
};

struct FailAdapter final : public NF::IGameProjectAdapter {
    std::string projectId()          const override { return "fail_project"; }
    std::string projectDisplayName() const override { return "Fail Project"; }
    bool  initialize() override { return false; } // always fails
    void  shutdown()   override {}
    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }
    std::vector<std::string> contentRoots()   const override { return {}; }
    std::vector<std::string> customCommands() const override { return {}; }
};

struct NoRootsAdapter final : public NF::IGameProjectAdapter {
    std::string projectId()          const override { return "no_roots_project"; }
    std::string projectDisplayName() const override { return "No Roots Project"; }
    bool  initialize() override { return true; }
    void  shutdown()   override {}
    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }
    std::vector<std::string> contentRoots()   const override { return {}; } // empty roots
};

struct MultiPanelAdapter final : public NF::IGameProjectAdapter {
    std::string projectId()          const override { return "multi_panel_project"; }
    std::string projectDisplayName() const override { return "Multi Panel Project"; }
    bool  initialize() override { return true; }
    void  shutdown()   override {}
    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        std::vector<NF::GameplaySystemPanelDescriptor> panels;
        for (int i = 0; i < 4; ++i) {
            NF::GameplaySystemPanelDescriptor d;
            d.id        = "multi_panel_project.panel_" + std::to_string(i);
            d.projectId = "multi_panel_project";
            panels.push_back(d);
        }
        return panels;
    }
    std::vector<std::string> contentRoots()   const override { return {"Content/", "Data/"}; }
    std::vector<std::string> customCommands() const override {
        return {"multi.build_client", "multi.build_server"};
    }
};

} // anonymous namespace

// ═════════════════════════════════════════════════════════════════════════════
// ProjectLoadContract tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("ProjectLoadContract: default state is Unloaded", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    CHECK(c.state == NF::ProjectLoadState::Unloaded);
    CHECK(c.projectId.empty());
    CHECK(c.projectDisplayName.empty());
    CHECK(c.panelCount == 0);
    CHECK(c.loadTimestampMs == 0);
    CHECK(c.contentRoots.empty());
    CHECK(c.customCommands.empty());
    CHECK(c.validationEntries.empty());
}

TEST_CASE("ProjectLoadContract: isLoaded reflects Ready state", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    CHECK_FALSE(c.isLoaded());
    c.state = NF::ProjectLoadState::Loading;
    CHECK_FALSE(c.isLoaded());
    c.state = NF::ProjectLoadState::Ready;
    CHECK(c.isLoaded());
    c.state = NF::ProjectLoadState::Failed;
    CHECK_FALSE(c.isLoaded());
}

TEST_CASE("ProjectLoadContract: hasFailed reflects Failed state", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    CHECK_FALSE(c.hasFailed());
    c.state = NF::ProjectLoadState::Failed;
    CHECK(c.hasFailed());
    c.state = NF::ProjectLoadState::Ready;
    CHECK_FALSE(c.hasFailed());
}

TEST_CASE("ProjectLoadContract: isValid with no entries returns true", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    CHECK(c.isValid());
}

TEST_CASE("ProjectLoadContract: isValid with Info/Warning entries returns true", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addInfo("info_code", "Just info");
    c.addWarning("warn_code", "Just a warning");
    CHECK(c.isValid());
}

TEST_CASE("ProjectLoadContract: isValid with Error entry returns false", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addError("err_code", "A blocking error");
    CHECK_FALSE(c.isValid());
}

TEST_CASE("ProjectLoadContract: isValid with Fatal entry returns false", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addFatal("fatal_code", "A fatal error");
    CHECK_FALSE(c.isValid());
}

TEST_CASE("ProjectLoadContract: isBuildReady requires Ready + isValid", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    CHECK(c.isBuildReady());

    c.addError("blocking", "blocks build");
    CHECK_FALSE(c.isBuildReady());
}

TEST_CASE("ProjectLoadContract: isBuildReady false when not Ready", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Unloaded;
    CHECK_FALSE(c.isBuildReady());
    c.state = NF::ProjectLoadState::Failed;
    CHECK_FALSE(c.isBuildReady());
    c.state = NF::ProjectLoadState::Loading;
    CHECK_FALSE(c.isBuildReady());
}

TEST_CASE("ProjectLoadContract: countBySeverity is accurate", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.addInfo("i1", "info 1");
    c.addInfo("i2", "info 2");
    c.addWarning("w1", "warn 1");
    c.addError("e1", "err 1");
    c.addFatal("f1", "fatal 1");
    CHECK(c.countBySeverity(NF::ProjectValidationSeverity::Info)    == 2);
    CHECK(c.countBySeverity(NF::ProjectValidationSeverity::Warning) == 1);
    CHECK(c.countBySeverity(NF::ProjectValidationSeverity::Error)   == 1);
    CHECK(c.countBySeverity(NF::ProjectValidationSeverity::Fatal)   == 1);
}

TEST_CASE("ProjectLoadContract: addEntry populates fields correctly", "[ProjectLoadContract]") {
    NF::ProjectLoadContract c;
    c.addEntry(NF::ProjectValidationSeverity::Warning, "my_code", "my message");
    REQUIRE(c.validationEntries.size() == 1);
    CHECK(c.validationEntries[0].severity == NF::ProjectValidationSeverity::Warning);
    CHECK(c.validationEntries[0].code    == "my_code");
    CHECK(c.validationEntries[0].message == "my message");
}

TEST_CASE("ProjectLoadContract: projectLoadStateName covers all states", "[ProjectLoadContract]") {
    CHECK(std::string(NF::projectLoadStateName(NF::ProjectLoadState::Unloaded)) == "Unloaded");
    CHECK(std::string(NF::projectLoadStateName(NF::ProjectLoadState::Loading))  == "Loading");
    CHECK(std::string(NF::projectLoadStateName(NF::ProjectLoadState::Ready))    == "Ready");
    CHECK(std::string(NF::projectLoadStateName(NF::ProjectLoadState::Failed))   == "Failed");
}

TEST_CASE("ProjectLoadContract: validationSeverityName covers all values", "[ProjectLoadContract]") {
    CHECK(std::string(NF::validationSeverityName(NF::ProjectValidationSeverity::Info))    == "Info");
    CHECK(std::string(NF::validationSeverityName(NF::ProjectValidationSeverity::Warning)) == "Warning");
    CHECK(std::string(NF::validationSeverityName(NF::ProjectValidationSeverity::Error))   == "Error");
    CHECK(std::string(NF::validationSeverityName(NF::ProjectValidationSeverity::Fatal))   == "Fatal");
}

// ═════════════════════════════════════════════════════════════════════════════
// ProjectRegistry tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("ProjectRegistry: default state is Idle", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    CHECK(reg.state() == NF::ProjectRegistryState::Idle);
    CHECK_FALSE(reg.isRunning());
    CHECK_FALSE(reg.hasActiveProject());
    CHECK(reg.registeredCount() == 0);
}

TEST_CASE("ProjectRegistry: initialize transitions to Running", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    CHECK(reg.state() == NF::ProjectRegistryState::Running);
    CHECK(reg.isRunning());
}

TEST_CASE("ProjectRegistry: shutdown transitions to Stopped", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.shutdown();
    CHECK(reg.state() == NF::ProjectRegistryState::Stopped);
    CHECK_FALSE(reg.isRunning());
}

TEST_CASE("ProjectRegistry: registerProject returns true for new id", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    bool ok = reg.registerProject("proj_a", [] { return std::make_unique<OkAdapter>(); });
    CHECK(ok);
    CHECK(reg.isRegistered("proj_a"));
    CHECK(reg.registeredCount() == 1);
}

TEST_CASE("ProjectRegistry: registerProject rejects duplicate id", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.registerProject("proj_a", [] { return std::make_unique<OkAdapter>(); });
    bool dup = reg.registerProject("proj_a", [] { return std::make_unique<OkAdapter>(); });
    CHECK_FALSE(dup);
    CHECK(reg.registeredCount() == 1);
}

TEST_CASE("ProjectRegistry: registerProject rejects empty id", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    bool ok = reg.registerProject("", [] { return std::make_unique<OkAdapter>(); });
    CHECK_FALSE(ok);
    CHECK(reg.registeredCount() == 0);
}

TEST_CASE("ProjectRegistry: registerProject rejects null factory", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    bool ok = reg.registerProject("proj_b", nullptr);
    CHECK_FALSE(ok);
    CHECK_FALSE(reg.isRegistered("proj_b"));
}

TEST_CASE("ProjectRegistry: registeredProjectIds lists all registered ids", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.registerProject("proj_a", [] { return std::make_unique<OkAdapter>(); });
    reg.registerProject("proj_b", [] { return std::make_unique<OkAdapter>(); });
    auto ids = reg.registeredProjectIds();
    CHECK(ids.size() == 2);
}

TEST_CASE("ProjectRegistry: loadProject requires Running state", "[ProjectRegistry]") {
    NF::ProjectRegistry reg; // Idle
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    auto contract = reg.loadProject("ok_project");
    CHECK(contract.state == NF::ProjectLoadState::Failed);
    CHECK_FALSE(reg.hasActiveProject());
}

TEST_CASE("ProjectRegistry: loadProject unknown id returns Failed contract", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    auto contract = reg.loadProject("nonexistent");
    CHECK(contract.state == NF::ProjectLoadState::Failed);
    CHECK(contract.projectId == "nonexistent");
    CHECK_FALSE(reg.hasActiveProject());
    CHECK(contract.countBySeverity(NF::ProjectValidationSeverity::Fatal) >= 1);
}

TEST_CASE("ProjectRegistry: loadProject success sets contract to Ready", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    auto contract = reg.loadProject("ok_project");
    CHECK(contract.state == NF::ProjectLoadState::Ready);
    CHECK(contract.projectId == "ok_project");
    CHECK(contract.projectDisplayName == "OK Project");
    CHECK(reg.hasActiveProject());
}

TEST_CASE("ProjectRegistry: loadProject populates contentRoots and customCommands", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    auto contract = reg.loadProject("ok_project");
    CHECK(contract.contentRoots.size() == 1);
    CHECK(contract.contentRoots[0] == "Content/");
    CHECK(contract.customCommands.size() == 1);
    CHECK(contract.customCommands[0] == "ok.build");
}

TEST_CASE("ProjectRegistry: loadProject populates panelCount", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    auto contract = reg.loadProject("ok_project");
    CHECK(contract.panelCount == 1); // OkAdapter returns 1 panel
}

TEST_CASE("ProjectRegistry: loadProject with FailAdapter returns Failed contract", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("fail_project", [] { return std::make_unique<FailAdapter>(); });
    auto contract = reg.loadProject("fail_project");
    CHECK(contract.state == NF::ProjectLoadState::Failed);
    CHECK_FALSE(reg.hasActiveProject());
    CHECK(contract.countBySeverity(NF::ProjectValidationSeverity::Fatal) >= 1);
}

TEST_CASE("ProjectRegistry: loadProject with no content roots adds warning", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("no_roots_project", [] { return std::make_unique<NoRootsAdapter>(); });
    auto contract = reg.loadProject("no_roots_project");
    CHECK(contract.state == NF::ProjectLoadState::Ready);
    CHECK(contract.countBySeverity(NF::ProjectValidationSeverity::Warning) >= 1);
}

TEST_CASE("ProjectRegistry: activeAdapter is accessible after successful load", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    reg.loadProject("ok_project");
    REQUIRE(reg.activeAdapter() != nullptr);
    CHECK(reg.activeAdapter()->projectId() == "ok_project");
}

TEST_CASE("ProjectRegistry: activeProjectId returns correct id", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    CHECK(reg.activeProjectId().empty());
    reg.loadProject("ok_project");
    CHECK(reg.activeProjectId() == "ok_project");
}

TEST_CASE("ProjectRegistry: activeContract is present after successful load", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    reg.loadProject("ok_project");
    REQUIRE(reg.activeContract().has_value());
    CHECK(reg.activeContract()->projectId == "ok_project");
    CHECK(reg.activeContract()->state == NF::ProjectLoadState::Ready);
}

TEST_CASE("ProjectRegistry: unloadActive clears active project", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    reg.loadProject("ok_project");
    CHECK(reg.hasActiveProject());
    reg.unloadActive();
    CHECK_FALSE(reg.hasActiveProject());
    CHECK(reg.activeProjectId().empty());
    CHECK_FALSE(reg.activeContract().has_value());
}

TEST_CASE("ProjectRegistry: loading second project unloads first", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project",    [] { return std::make_unique<OkAdapter>();       });
    reg.registerProject("no_roots_project", [] { return std::make_unique<NoRootsAdapter>(); });

    reg.loadProject("ok_project");
    CHECK(reg.activeProjectId() == "ok_project");

    reg.loadProject("no_roots_project");
    CHECK(reg.activeProjectId() == "no_roots_project");
}

TEST_CASE("ProjectRegistry: MultiPanelAdapter loads with correct panel count", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("multi_panel_project", [] { return std::make_unique<MultiPanelAdapter>(); });
    auto contract = reg.loadProject("multi_panel_project");
    CHECK(contract.state == NF::ProjectLoadState::Ready);
    CHECK(contract.panelCount == 4);
    CHECK(contract.contentRoots.size() == 2);
    CHECK(contract.customCommands.size() == 2);
}

TEST_CASE("ProjectRegistry: loadTimestampMs is set after successful load", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    auto before = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto contract = reg.loadProject("ok_project");
    auto after = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    CHECK(contract.loadTimestampMs >= before);
    CHECK(contract.loadTimestampMs <= after);
}

TEST_CASE("ProjectRegistry: shutdown unloads active project", "[ProjectRegistry]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });
    reg.loadProject("ok_project");
    CHECK(reg.hasActiveProject());
    reg.shutdown();
    CHECK_FALSE(reg.hasActiveProject());
}

// ═════════════════════════════════════════════════════════════════════════════
// BuildGateController tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("BuildGateController: default has no rules", "[BuildGateController]") {
    NF::BuildGateController gate;
    CHECK(gate.ruleCount() == 0);
}

TEST_CASE("BuildGateController: Unloaded contract → ClosedNoProject", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract empty;
    auto result = gate.evaluate(empty);
    CHECK(result.status == NF::BuildGateStatus::ClosedNoProject);
    CHECK_FALSE(result.isOpen());
    CHECK(result.hasBlockingErrors());
}

TEST_CASE("BuildGateController: Failed contract → ClosedNoProject", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Failed;
    c.projectId = "fail_project";
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::ClosedNoProject);
    CHECK_FALSE(result.isOpen());
}

TEST_CASE("BuildGateController: Loading contract → ClosedNoProject", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Loading;
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::ClosedNoProject);
}

TEST_CASE("BuildGateController: Ready contract with no errors → Open", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state     = NF::ProjectLoadState::Ready;
    c.projectId = "ok_project";
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::Open);
    CHECK(result.isOpen());
    CHECK_FALSE(result.hasBlockingErrors());
}

TEST_CASE("BuildGateController: Ready contract with Error entry → ClosedInvalid", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addError("missing_asset", "Required asset file not found.");
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::ClosedInvalid);
    CHECK_FALSE(result.isOpen());
    CHECK(result.hasBlockingErrors());
}

TEST_CASE("BuildGateController: Ready contract with Fatal entry → ClosedInvalid", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addFatal("corrupt_manifest", "Project manifest is corrupt.");
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::ClosedInvalid);
}

TEST_CASE("BuildGateController: Ready contract with only Warnings → Open", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addWarning("no_roots", "No content roots declared.");
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::Open);
    CHECK(result.isOpen());
}

TEST_CASE("BuildGateController: addRule with passing rule → Open", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("always_pass", [](const NF::ProjectLoadContract&) {
        return NF::BuildGateRuleResult{true, "always_pass", ""};
    });
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::Open);
    CHECK(result.ruleResults.size() == 1);
    CHECK(result.ruleResults[0].passed);
}

TEST_CASE("BuildGateController: addRule with failing rule → ClosedByRule", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("always_fail", [](const NF::ProjectLoadContract&) {
        return NF::BuildGateRuleResult{false, "always_fail", "Rule always fails."};
    });
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::ClosedByRule);
    CHECK_FALSE(result.isOpen());
    CHECK(result.hasBlockingErrors());
    REQUIRE(result.ruleResults.size() == 1);
    CHECK_FALSE(result.ruleResults[0].passed);
}

TEST_CASE("BuildGateController: multiple rules all pass → Open", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("rule1", [](const NF::ProjectLoadContract&) { return NF::BuildGateRuleResult{true}; });
    gate.addRule("rule2", [](const NF::ProjectLoadContract&) { return NF::BuildGateRuleResult{true}; });
    gate.addRule("rule3", [](const NF::ProjectLoadContract&) { return NF::BuildGateRuleResult{true}; });
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::Open);
    CHECK(result.ruleResults.size() == 3);
}

TEST_CASE("BuildGateController: rule receives contract data", "[BuildGateController]") {
    NF::BuildGateController gate;
    std::string capturedProjectId;
    gate.addRule("inspector", [&](const NF::ProjectLoadContract& c) {
        capturedProjectId = c.projectId;
        return NF::BuildGateRuleResult{true};
    });
    NF::ProjectLoadContract c;
    c.state     = NF::ProjectLoadState::Ready;
    c.projectId = "checked_project";
    static_cast<void>(gate.evaluate(c));
    CHECK(capturedProjectId == "checked_project");
}

TEST_CASE("BuildGateController: rule requiring content roots blocks no-root projects", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("require_content_roots", [](const NF::ProjectLoadContract& c) {
        if (c.contentRoots.empty())
            return NF::BuildGateRuleResult{false, "require_content_roots", "Project must declare content roots."};
        return NF::BuildGateRuleResult{true};
    });
    NF::ProjectLoadContract okContract;
    okContract.state = NF::ProjectLoadState::Ready;
    okContract.contentRoots = {"Content/"};
    CHECK(gate.evaluate(okContract).isOpen());

    NF::ProjectLoadContract noRootContract;
    noRootContract.state = NF::ProjectLoadState::Ready;
    auto result = gate.evaluate(noRootContract);
    CHECK_FALSE(result.isOpen());
    CHECK(result.status == NF::BuildGateStatus::ClosedByRule);
}

TEST_CASE("BuildGateController: clearRules removes all rules", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("r1", [](const NF::ProjectLoadContract&) { return NF::BuildGateRuleResult{false}; });
    gate.addRule("r2", [](const NF::ProjectLoadContract&) { return NF::BuildGateRuleResult{false}; });
    CHECK(gate.ruleCount() == 2);
    gate.clearRules();
    CHECK(gate.ruleCount() == 0);
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    CHECK(gate.evaluate(c).isOpen()); // rules gone, gate opens
}

TEST_CASE("BuildGateController: addRule rejects empty ruleId", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("", [](const NF::ProjectLoadContract&) { return NF::BuildGateRuleResult{false}; });
    CHECK(gate.ruleCount() == 0);
}

TEST_CASE("BuildGateController: addRule rejects null rule callable", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("null_rule", nullptr);
    CHECK(gate.ruleCount() == 0);
}

TEST_CASE("BuildGateController: evaluateOptional with nullptr → ClosedNoProject", "[BuildGateController]") {
    NF::BuildGateController gate;
    auto result = gate.evaluateOptional(nullptr);
    CHECK(result.status == NF::BuildGateStatus::ClosedNoProject);
}

TEST_CASE("BuildGateController: evaluateOptional with valid contract → Open", "[BuildGateController]") {
    NF::BuildGateController gate;
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    auto result = gate.evaluateOptional(&c);
    CHECK(result.isOpen());
}

TEST_CASE("BuildGateController: buildGateStatusName covers all values", "[BuildGateController]") {
    CHECK(std::string(NF::buildGateStatusName(NF::BuildGateStatus::Open))            == "Open");
    CHECK(std::string(NF::buildGateStatusName(NF::BuildGateStatus::ClosedNoProject)) == "ClosedNoProject");
    CHECK(std::string(NF::buildGateStatusName(NF::BuildGateStatus::ClosedInvalid))   == "ClosedInvalid");
    CHECK(std::string(NF::buildGateStatusName(NF::BuildGateStatus::ClosedByRule))    == "ClosedByRule");
}

TEST_CASE("BuildGateController: contract-level error takes priority over rule failure", "[BuildGateController]") {
    NF::BuildGateController gate;
    gate.addRule("fail_rule", [](const NF::ProjectLoadContract&) {
        return NF::BuildGateRuleResult{false, "fail_rule", "Always fails."};
    });
    NF::ProjectLoadContract c;
    c.state = NF::ProjectLoadState::Ready;
    c.addError("blocking", "This blocks the build.");
    // Contract-level errors are checked before rules
    auto result = gate.evaluate(c);
    CHECK(result.status == NF::BuildGateStatus::ClosedInvalid);
}

// ═════════════════════════════════════════════════════════════════════════════
// Integration tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("Phase5 integration: load project via registry and evaluate build gate", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });

    auto contract = reg.loadProject("ok_project");
    CHECK(contract.state == NF::ProjectLoadState::Ready);
    CHECK(contract.isBuildReady());

    NF::BuildGateController gate;
    auto result = gate.evaluateOptional(&contract);
    CHECK(result.isOpen());
}

TEST_CASE("Phase5 integration: failed load keeps build gate closed", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("fail_project", [] { return std::make_unique<FailAdapter>(); });

    auto contract = reg.loadProject("fail_project");
    CHECK(contract.state == NF::ProjectLoadState::Failed);

    NF::BuildGateController gate;
    auto result = gate.evaluateOptional(&contract);
    CHECK_FALSE(result.isOpen());
}

TEST_CASE("Phase5 integration: custom build gate rule blocks even a valid project", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });

    auto contract = reg.loadProject("ok_project");
    CHECK(contract.isBuildReady());

    NF::BuildGateController gate;
    gate.addRule("require_two_panels", [](const NF::ProjectLoadContract& c) {
        if (c.panelCount < 2)
            return NF::BuildGateRuleResult{false, "require_two_panels", "Need at least 2 panels."};
        return NF::BuildGateRuleResult{true};
    });

    auto result = gate.evaluate(contract);
    CHECK_FALSE(result.isOpen()); // OkAdapter only has 1 panel
    CHECK(result.status == NF::BuildGateStatus::ClosedByRule);
}

TEST_CASE("Phase5 integration: multi-panel project passes two-panel rule", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("multi_panel_project", [] { return std::make_unique<MultiPanelAdapter>(); });

    auto contract = reg.loadProject("multi_panel_project");
    CHECK(contract.state == NF::ProjectLoadState::Ready);
    CHECK(contract.panelCount == 4);

    NF::BuildGateController gate;
    gate.addRule("require_two_panels", [](const NF::ProjectLoadContract& c) {
        if (c.panelCount < 2)
            return NF::BuildGateRuleResult{false, "require_two_panels", "Need at least 2 panels."};
        return NF::BuildGateRuleResult{true};
    });

    auto result = gate.evaluate(contract);
    CHECK(result.isOpen());
}

TEST_CASE("Phase5 integration: swap projects and gate updates accordingly", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project",   [] { return std::make_unique<OkAdapter>();   });
    reg.registerProject("fail_project", [] { return std::make_unique<FailAdapter>(); });

    NF::BuildGateController gate;

    auto c1 = reg.loadProject("ok_project");
    CHECK(gate.evaluate(c1).isOpen());

    auto c2 = reg.loadProject("fail_project");
    CHECK_FALSE(gate.evaluate(c2).isOpen());
}

TEST_CASE("Phase5 integration: unload project and gate closes", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project", [] { return std::make_unique<OkAdapter>(); });

    reg.loadProject("ok_project");
    reg.unloadActive();
    CHECK_FALSE(reg.hasActiveProject());

    NF::BuildGateController gate;
    auto result = gate.evaluateOptional(nullptr);
    CHECK_FALSE(result.isOpen());
}

TEST_CASE("Phase5 integration: registry supports multiple registered projects", "[Phase5][integration]") {
    NF::ProjectRegistry reg;
    reg.initialize();
    reg.registerProject("ok_project",    [] { return std::make_unique<OkAdapter>();       });
    reg.registerProject("fail_project",  [] { return std::make_unique<FailAdapter>();      });
    reg.registerProject("no_roots_project", [] { return std::make_unique<NoRootsAdapter>(); });
    reg.registerProject("multi_panel_project", [] { return std::make_unique<MultiPanelAdapter>(); });

    CHECK(reg.registeredCount() == 4);
    auto ids = reg.registeredProjectIds();
    CHECK(ids.size() == 4);
}
