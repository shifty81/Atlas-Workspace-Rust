// S147 editor tests: SettingsControlPanelV1, ProjectSurfaceV1, RepoSurfaceV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── SettingsControlPanelV1 ────────────────────────────────────────────────────

TEST_CASE("CpSettingType names", "[Editor][S147]") {
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Bool))   == "Bool");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Int))    == "Int");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Float))  == "Float");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::String)) == "String");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Enum))   == "Enum");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Color))  == "Color");
    REQUIRE(std::string(cpSettingTypeName(CpSettingType::Path))   == "Path");
}

TEST_CASE("CpSettingEntry validity and resetToDefault", "[Editor][S147]") {
    CpSettingEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.key = "theme.font.size"; e.value = "14"; e.defaultValue = "12";
    REQUIRE(e.isValid());
    REQUIRE(!e.isAtDefault());
    e.resetToDefault();
    REQUIRE(e.isAtDefault());
    REQUIRE(e.value == "12");
}

TEST_CASE("CpSettingEntry readOnly prevents reset", "[Editor][S147]") {
    CpSettingEntry e;
    e.id = 1; e.key = "locked"; e.value = "42"; e.defaultValue = "0";
    e.readOnly = true;
    e.resetToDefault();
    REQUIRE(e.value == "42");  // unchanged
}

TEST_CASE("CpSection addEntry and dirtyCount", "[Editor][S147]") {
    CpSection sec;
    sec.id = 1; sec.title = "UI";
    CpSettingEntry e1; e1.id = 1; e1.key = "font.size"; e1.value = "14"; e1.defaultValue = "12";
    e1.dirty = true;
    CpSettingEntry e2; e2.id = 2; e2.key = "theme"; e2.value = "dark"; e2.defaultValue = "dark";
    sec.entries.push_back(e1);
    sec.entries.push_back(e2);
    REQUIRE(sec.dirtyCount() == 1u);
    REQUIRE(sec.findEntry("font.size") != nullptr);
    REQUIRE(sec.findEntry("nonexistent") == nullptr);
}

TEST_CASE("SettingsControlPanelV1 addSection and addEntry", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "Rendering";
    REQUIRE(panel.addSection(sec));
    REQUIRE(panel.sectionCount() == 1u);

    CpSettingEntry e; e.id = 1; e.key = "aa.mode"; e.value = "MSAA"; e.defaultValue = "None";
    REQUIRE(panel.addEntry(1, e));
    REQUIRE(panel.totalEntryCount() == 1u);
}

TEST_CASE("SettingsControlPanelV1 addSection duplicate rejected", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "Input";
    REQUIRE(panel.addSection(sec));
    REQUIRE(!panel.addSection(sec));
}

TEST_CASE("SettingsControlPanelV1 setValue and onChange callback", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "Editor";
    panel.addSection(sec);
    CpSettingEntry e; e.id = 1; e.key = "tab.size"; e.value = "4"; e.defaultValue = "4";
    panel.addEntry(1, e);

    std::string changedKey;
    panel.setOnChange([&](const CpSettingEntry& entry, const std::string&) {
        changedKey = entry.key;
    });

    REQUIRE(panel.setValue("tab.size", "2"));
    REQUIRE(changedKey == "tab.size");
    REQUIRE(panel.getValue("tab.size") == "2");
    REQUIRE(panel.totalDirtyCount() == 1u);
}

TEST_CASE("SettingsControlPanelV1 readOnly rejects setValue", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "System";
    panel.addSection(sec);
    CpSettingEntry e; e.id = 1; e.key = "engine.version"; e.value = "1.0"; e.defaultValue = "1.0";
    e.readOnly = true;
    panel.addEntry(1, e);
    REQUIRE(!panel.setValue("engine.version", "2.0"));
    REQUIRE(panel.getValue("engine.version") == "1.0");
}

TEST_CASE("SettingsControlPanelV1 resetToDefault for key", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "UI";
    panel.addSection(sec);
    CpSettingEntry e; e.id = 1; e.key = "ui.scale"; e.value = "2.0"; e.defaultValue = "1.0";
    panel.addEntry(1, e);
    REQUIRE(panel.resetToDefault("ui.scale"));
    REQUIRE(panel.getValue("ui.scale") == "1.0");
}

TEST_CASE("SettingsControlPanelV1 resetAllToDefault", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "General";
    CpSettingEntry e1; e1.id = 1; e1.key = "x"; e1.value = "99"; e1.defaultValue = "1";
    CpSettingEntry e2; e2.id = 2; e2.key = "y"; e2.value = "88"; e2.defaultValue = "2";
    sec.entries.push_back(e1); sec.entries.push_back(e2);
    panel.addSection(sec);
    panel.resetAllToDefault();
    REQUIRE(panel.getValue("x") == "1");
    REQUIRE(panel.getValue("y") == "2");
}

TEST_CASE("SettingsControlPanelV1 search", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "Performance";
    CpSettingEntry e1; e1.id = 1; e1.key = "render.shadows"; e1.label = "Shadow Quality";
    e1.value = "High"; e1.defaultValue = "Medium";
    CpSettingEntry e2; e2.id = 2; e2.key = "render.aa"; e2.label = "Anti-Aliasing";
    e2.value = "4x"; e2.defaultValue = "Off";
    sec.entries.push_back(e1); sec.entries.push_back(e2);
    panel.addSection(sec);

    auto res = panel.search("Shadow");
    REQUIRE(res.size() == 1u);
    REQUIRE(res[0] == "render.shadows");

    auto res2 = panel.search("render");
    REQUIRE(res2.size() == 2u);
}

TEST_CASE("SettingsControlPanelV1 hasRestartRequired", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "System";
    CpSettingEntry e; e.id = 1; e.key = "gfx.api"; e.value = "Vulkan"; e.defaultValue = "D3D12";
    e.requiresRestart = true; e.dirty = true;
    sec.entries.push_back(e);
    panel.addSection(sec);
    REQUIRE(panel.hasRestartRequired());
}

TEST_CASE("SettingsControlPanelV1 removeSection", "[Editor][S147]") {
    SettingsControlPanelV1 panel;
    CpSection sec; sec.id = 1; sec.title = "S";
    panel.addSection(sec);
    REQUIRE(panel.removeSection(1));
    REQUIRE(panel.sectionCount() == 0u);
    REQUIRE(!panel.removeSection(99));
}

// ── ProjectSurfaceV1 ──────────────────────────────────────────────────────────

TEST_CASE("ProjectState names", "[Editor][S147]") {
    REQUIRE(std::string(projectStateName(ProjectState::Closed))  == "Closed");
    REQUIRE(std::string(projectStateName(ProjectState::Opening)) == "Opening");
    REQUIRE(std::string(projectStateName(ProjectState::Open))    == "Open");
    REQUIRE(std::string(projectStateName(ProjectState::Saving))  == "Saving");
    REQUIRE(std::string(projectStateName(ProjectState::Error))   == "Error");
}

TEST_CASE("ProjectMetadata isValid", "[Editor][S147]") {
    ProjectMetadata m;
    REQUIRE(!m.isValid());
    m.name = "Atlas Game"; m.rootPath = "/projects/atlas";
    REQUIRE(m.isValid());
}

TEST_CASE("ProjectSurfaceV1 openProject and closeProject", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    REQUIRE(surface.state() == ProjectState::Closed);
    REQUIRE(!surface.isOpen());

    ProjectMetadata m; m.name = "Demo"; m.rootPath = "/dev/demo";
    REQUIRE(surface.openProject(m));
    REQUIRE(surface.isOpen());
    REQUIRE(surface.openCount() == 1u);
    REQUIRE(surface.metadata().name == "Demo");

    surface.closeProject();
    REQUIRE(surface.state() == ProjectState::Closed);
}

TEST_CASE("ProjectSurfaceV1 openProject invalid rejected", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    ProjectMetadata bad;
    REQUIRE(!surface.openProject(bad));
}

TEST_CASE("ProjectSurfaceV1 saveProject", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    ProjectMetadata m; m.name = "P"; m.rootPath = "/p";
    surface.openProject(m);
    surface.saveProject();
    REQUIRE(surface.saveCount() == 1u);
    REQUIRE(surface.isOpen());
}

TEST_CASE("ProjectSurfaceV1 addFileNode and findFileNode", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    ProjectMetadata m; m.name = "P"; m.rootPath = "/p";
    surface.openProject(m);

    ProjectFileNode root; root.id = 1; root.name = "root"; root.path = "/"; root.isDir = true;
    ProjectFileNode file; file.id = 2; file.name = "main.cpp"; file.path = "/main.cpp"; file.parentId = 1;
    REQUIRE(surface.addFileNode(root));
    REQUIRE(surface.addFileNode(file));
    REQUIRE(surface.fileNodeCount() == 2u);
    REQUIRE(surface.findFileNode(2) != nullptr);
    REQUIRE(surface.findFileNodeByPath("/main.cpp") != nullptr);

    // root node's childIds should contain file 2
    REQUIRE(surface.findFileNode(1)->childIds.size() == 1u);
}

TEST_CASE("ProjectSurfaceV1 rootFileNodes", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    ProjectMetadata m; m.name = "P"; m.rootPath = "/p";
    surface.openProject(m);

    ProjectFileNode r1; r1.id = 1; r1.name = "src"; r1.path = "/src"; r1.isDir = true;
    ProjectFileNode r2; r2.id = 2; r2.name = "build"; r2.path = "/build"; r2.isDir = true;
    ProjectFileNode child; child.id = 3; child.name = "a.cpp"; child.path = "/src/a.cpp"; child.parentId = 1;
    surface.addFileNode(r1); surface.addFileNode(r2); surface.addFileNode(child);

    auto roots = surface.rootFileNodes();
    REQUIRE(roots.size() == 2u);
}

TEST_CASE("ProjectSurfaceV1 recentProjects tracking", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    ProjectMetadata m1; m1.name = "Alpha"; m1.rootPath = "/proj/alpha";
    ProjectMetadata m2; m2.name = "Beta";  m2.rootPath = "/proj/beta";
    surface.openProject(m1);
    surface.openProject(m2);
    REQUIRE(surface.recentCount() == 2u);

    surface.openProject(m1);  // re-open same project
    REQUIRE(surface.recentCount() == 2u);  // no duplicate
}

TEST_CASE("ProjectSurfaceV1 onStateChange callback", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    std::vector<ProjectState> states;
    surface.setOnStateChange([&](ProjectState s) { states.push_back(s); });

    ProjectMetadata m; m.name = "P"; m.rootPath = "/p";
    surface.openProject(m);
    surface.saveProject();
    surface.closeProject();

    REQUIRE(states.size() >= 2u);
    REQUIRE(states.back() == ProjectState::Closed);
}

TEST_CASE("ProjectSurfaceV1 updateMetadata", "[Editor][S147]") {
    ProjectSurfaceV1 surface;
    ProjectMetadata m; m.name = "P"; m.rootPath = "/p";
    surface.openProject(m);

    ProjectMetadata updated; updated.name = "P2"; updated.rootPath = "/p2";
    REQUIRE(surface.updateMetadata(updated));
    REQUIRE(surface.metadata().name == "P2");
}

// ── RepoSurfaceV1 ─────────────────────────────────────────────────────────────

TEST_CASE("RepoState names", "[Editor][S147]") {
    REQUIRE(std::string(repoStateName(RepoState::Uninitialized)) == "Uninitialized");
    REQUIRE(std::string(repoStateName(RepoState::Clean))         == "Clean");
    REQUIRE(std::string(repoStateName(RepoState::Modified))      == "Modified");
    REQUIRE(std::string(repoStateName(RepoState::Merging))       == "Merging");
    REQUIRE(std::string(repoStateName(RepoState::Rebasing))      == "Rebasing");
    REQUIRE(std::string(repoStateName(RepoState::Detached))      == "Detached");
    REQUIRE(std::string(repoStateName(RepoState::Error))         == "Error");
}

TEST_CASE("FileChangeStatus names", "[Editor][S147]") {
    REQUIRE(std::string(fileChangeStatusName(FileChangeStatus::Modified))   == "Modified");
    REQUIRE(std::string(fileChangeStatusName(FileChangeStatus::Added))      == "Added");
    REQUIRE(std::string(fileChangeStatusName(FileChangeStatus::Deleted))    == "Deleted");
    REQUIRE(std::string(fileChangeStatusName(FileChangeStatus::Untracked))  == "Untracked");
    REQUIRE(std::string(fileChangeStatusName(FileChangeStatus::Conflicted)) == "Conflicted");
}

TEST_CASE("BranchInfo isValid and isSynced", "[Editor][S147]") {
    BranchInfo b;
    REQUIRE(!b.isValid());
    b.name = "main";
    REQUIRE(b.isValid());
    REQUIRE(b.isSynced());
    b.aheadBy = 2;
    REQUIRE(!b.isSynced());
}

TEST_CASE("CommitInfo isValid and isMergeCommit", "[Editor][S147]") {
    CommitInfo c;
    REQUIRE(!c.isValid());
    c.hash = "abc123"; c.message = "Initial commit";
    REQUIRE(c.isValid());
    REQUIRE(!c.isMergeCommit());
    c.parentHashes = {"p1", "p2"};
    REQUIRE(c.isMergeCommit());
}

TEST_CASE("RepoSurfaceV1 initialize", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    REQUIRE(!repo.isInitialized());
    REQUIRE(repo.state() == RepoState::Uninitialized);
    repo.initialize("/workspace/atlas");
    REQUIRE(repo.isInitialized());
    REQUIRE(repo.repoRoot() == "/workspace/atlas");
    REQUIRE(repo.state() == RepoState::Clean);
}

TEST_CASE("RepoSurfaceV1 addBranch and setCurrentBranch", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");

    BranchInfo b1; b1.name = "main";
    BranchInfo b2; b2.name = "develop";
    REQUIRE(repo.addBranch(b1));
    REQUIRE(repo.addBranch(b2));
    REQUIRE(repo.branchCount() == 2u);

    repo.setCurrentBranch("main");
    REQUIRE(repo.currentBranch() == "main");
    REQUIRE(repo.findBranch("main")->isCurrent);
    REQUIRE(!repo.findBranch("develop")->isCurrent);
}

TEST_CASE("RepoSurfaceV1 addBranch duplicate rejected", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    BranchInfo b; b.name = "main";
    REQUIRE(repo.addBranch(b));
    REQUIRE(!repo.addBranch(b));
}

TEST_CASE("RepoSurfaceV1 localBranches and remoteBranches", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    BranchInfo local; local.name = "main"; local.isRemote = false;
    BranchInfo remote; remote.name = "origin/main"; remote.isRemote = true;
    repo.addBranch(local); repo.addBranch(remote);
    REQUIRE(repo.localBranches().size() == 1u);
    REQUIRE(repo.remoteBranches().size() == 1u);
}

TEST_CASE("RepoSurfaceV1 setFileStatus and changedFiles", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");

    std::vector<FileStatusEntry> status;
    FileStatusEntry f1; f1.path = "src/main.cpp"; f1.status = FileChangeStatus::Modified;
    FileStatusEntry f2; f2.path = "README.md"; f2.status = FileChangeStatus::Unmodified;
    FileStatusEntry f3; f3.path = "new.h"; f3.status = FileChangeStatus::Added;
    status.push_back(f1); status.push_back(f2); status.push_back(f3);
    repo.setFileStatus(status);

    REQUIRE(repo.fileStatusCount() == 3u);
    REQUIRE(repo.changedFiles().size() == 2u);
    REQUIRE(repo.state() == RepoState::Modified);
}

TEST_CASE("RepoSurfaceV1 conflictedFiles", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    FileStatusEntry f; f.path = "file.cpp"; f.status = FileChangeStatus::Conflicted;
    repo.addFileStatus(f);
    REQUIRE(repo.conflictedFiles().size() == 1u);
}

TEST_CASE("RepoSurfaceV1 addCommit and findCommit", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    CommitInfo c; c.hash = "abc123"; c.fullHash = "abc123def456"; c.message = "Add feature";
    c.author = "dev";
    REQUIRE(repo.addCommit(c));
    REQUIRE(repo.commitCount() == 1u);
    REQUIRE(repo.findCommit("abc123") != nullptr);
    REQUIRE(repo.findCommit("abc123def456") != nullptr);
    REQUIRE(repo.findCommit("zzz") == nullptr);

    REQUIRE(!repo.addCommit(c));  // duplicate hash
}

TEST_CASE("RepoSurfaceV1 requestDiff callback", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    DiffRequest req; req.filePath = "src/main.cpp"; req.fromRef = "HEAD~1"; req.toRef = "HEAD";

    std::string diffedFile;
    repo.setOnDiff([&](const DiffRequest& r) { diffedFile = r.filePath; });
    repo.requestDiff(req);
    REQUIRE(diffedFile == "src/main.cpp");
    REQUIRE(repo.diffRequestCount() == 1u);
}

TEST_CASE("RepoSurfaceV1 requestBlame callback", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    BlameRequest req; req.filePath = "src/core.cpp"; req.startLine = 10; req.endLine = 50;

    std::string blamedFile;
    repo.setOnBlame([&](const BlameRequest& r) { blamedFile = r.filePath; });
    repo.requestBlame(req);
    REQUIRE(blamedFile == "src/core.cpp");
    REQUIRE(repo.blameRequestCount() == 1u);
}

TEST_CASE("RepoSurfaceV1 removeBranch", "[Editor][S147]") {
    RepoSurfaceV1 repo;
    repo.initialize("/repo");
    BranchInfo b; b.name = "feature-x";
    repo.addBranch(b);
    REQUIRE(repo.removeBranch("feature-x"));
    REQUIRE(repo.branchCount() == 0u);
    REQUIRE(!repo.removeBranch("nonexistent"));
}

TEST_CASE("DiffRequest and BlameRequest isValid", "[Editor][S147]") {
    DiffRequest d;
    REQUIRE(!d.isValid());
    d.filePath = "main.cpp";
    REQUIRE(d.isValid());

    BlameRequest bl;
    REQUIRE(!bl.isValid());
    bl.filePath = "main.cpp";
    REQUIRE(bl.isValid());
}
