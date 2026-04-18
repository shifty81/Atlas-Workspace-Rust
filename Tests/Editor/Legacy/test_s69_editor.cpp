#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S69: VersionControl ──────────────────────────────────────────

TEST_CASE("VCSProviderType names are correct", "[Editor][S69]") {
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Git))       == "Git");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::SVN))       == "SVN");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Perforce))  == "Perforce");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Mercurial)) == "Mercurial");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Plastic))   == "Plastic");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Fossil))    == "Fossil");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Custom))    == "Custom");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::None))      == "None");
}

TEST_CASE("VCSFileStatus names are correct", "[Editor][S69]") {
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Untracked))  == "Untracked");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Added))      == "Added");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Modified))   == "Modified");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Deleted))    == "Deleted");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Renamed))    == "Renamed");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Conflicted)) == "Conflicted");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Ignored))    == "Ignored");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Unchanged))  == "Unchanged");
}

TEST_CASE("VCSCommitInfo defaults are invalid", "[Editor][S69]") {
    VCSCommitInfo commit;
    REQUIRE_FALSE(commit.isValid());
}

TEST_CASE("VCSCommitInfo valid when hash and author set", "[Editor][S69]") {
    VCSCommitInfo commit;
    commit.hash = "abc123";
    commit.author = "dev@example.com";
    commit.message = "Initial commit";
    REQUIRE(commit.isValid());
}

TEST_CASE("VCSCommitInfo isRoot when no parentHash", "[Editor][S69]") {
    VCSCommitInfo commit;
    commit.hash = "abc123";
    commit.author = "dev";
    REQUIRE(commit.isRoot());
    commit.parentHash = "xyz";
    REQUIRE_FALSE(commit.isRoot());
}

TEST_CASE("VCSCommitInfo hasMessage checks message field", "[Editor][S69]") {
    VCSCommitInfo commit;
    commit.hash = "abc";
    commit.author = "dev";
    REQUIRE_FALSE(commit.hasMessage());
    commit.message = "fix bug";
    REQUIRE(commit.hasMessage());
}

TEST_CASE("VCSBranchInfo isSynced when ahead/behind are zero", "[Editor][S69]") {
    VCSBranchInfo branch;
    branch.name = "main";
    REQUIRE(branch.isSynced());
    branch.aheadCount = 1;
    REQUIRE_FALSE(branch.isSynced());
}

TEST_CASE("VCSBranchInfo isLocal is true for non-remote", "[Editor][S69]") {
    VCSBranchInfo branch;
    branch.name = "feature";
    branch.isRemote = false;
    REQUIRE(branch.isLocal());
}

TEST_CASE("VCSBranchInfo hasCommits checks lastCommitHash", "[Editor][S69]") {
    VCSBranchInfo branch;
    REQUIRE_FALSE(branch.hasCommits());
    branch.lastCommitHash = "deadbeef";
    REQUIRE(branch.hasCommits());
}

TEST_CASE("VCSDiffEntry totalChanges sums additions and deletions", "[Editor][S69]") {
    VCSDiffEntry entry;
    entry.additions = 5;
    entry.deletions = 3;
    REQUIRE(entry.totalChanges() == 8);
}

TEST_CASE("VCSDiffEntry hasChanges false when both zero", "[Editor][S69]") {
    VCSDiffEntry entry;
    REQUIRE_FALSE(entry.hasChanges());
    entry.additions = 1;
    REQUIRE(entry.hasChanges());
}

TEST_CASE("VCSRepository starts with no branches or commits", "[Editor][S69]") {
    VCSRepository repo("my-repo");
    REQUIRE(repo.branchCount() == 0);
    REQUIRE(repo.commitCount() == 0);
    REQUIRE(repo.diffCount() == 0);
    REQUIRE(repo.name() == "my-repo");
    REQUIRE(repo.provider() == VCSProviderType::Git);
}

TEST_CASE("VCSRepository addBranch increases count", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSBranchInfo b;
    b.name = "main";
    b.isActive = true;
    REQUIRE(repo.addBranch(b));
    REQUIRE(repo.branchCount() == 1);
}

TEST_CASE("VCSRepository addBranch rejects duplicate name", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSBranchInfo b;
    b.name = "main";
    repo.addBranch(b);
    REQUIRE_FALSE(repo.addBranch(b));
    REQUIRE(repo.branchCount() == 1);
}

TEST_CASE("VCSRepository switchBranch sets active", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSBranchInfo main;
    main.name = "main";
    main.isActive = true;
    VCSBranchInfo dev;
    dev.name = "dev";
    repo.addBranch(main);
    repo.addBranch(dev);
    REQUIRE(repo.switchBranch("dev"));
    REQUIRE(repo.activeBranch() != nullptr);
    REQUIRE(repo.activeBranch()->name == "dev");
}

TEST_CASE("VCSRepository switchBranch returns false for unknown branch", "[Editor][S69]") {
    VCSRepository repo("repo");
    REQUIRE_FALSE(repo.switchBranch("nonexistent"));
}

TEST_CASE("VCSRepository removeBranch removes inactive branch", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSBranchInfo b;
    b.name = "feature";
    b.isActive = false;
    repo.addBranch(b);
    REQUIRE(repo.removeBranch("feature"));
    REQUIRE(repo.branchCount() == 0);
}

TEST_CASE("VCSRepository removeBranch cannot remove active branch", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSBranchInfo b;
    b.name = "main";
    b.isActive = true;
    repo.addBranch(b);
    REQUIRE_FALSE(repo.removeBranch("main"));
}

TEST_CASE("VCSRepository addCommit rejects invalid commit", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSCommitInfo commit;
    REQUIRE_FALSE(repo.addCommit(commit));
}

TEST_CASE("VCSRepository addCommit accepts valid commit", "[Editor][S69]") {
    VCSRepository repo("repo");
    VCSCommitInfo commit;
    commit.hash = "abc";
    commit.author = "dev";
    REQUIRE(repo.addCommit(commit));
    REQUIRE(repo.commitCount() == 1);
}

TEST_CASE("VCSRepository trackFile adds diff entry", "[Editor][S69]") {
    VCSRepository repo("repo");
    REQUIRE(repo.trackFile("main.cpp", VCSFileStatus::Modified));
    REQUIRE(repo.diffCount() == 1);
}

TEST_CASE("VCSRepository trackFile updates existing entry status", "[Editor][S69]") {
    VCSRepository repo("repo");
    repo.trackFile("main.cpp", VCSFileStatus::Added);
    repo.trackFile("main.cpp", VCSFileStatus::Modified);
    REQUIRE(repo.diffCount() == 1);
    const auto* d = repo.findDiff("main.cpp");
    REQUIRE(d != nullptr);
    REQUIRE(d->status == VCSFileStatus::Modified);
}

TEST_CASE("VCSRepository findDiff returns nullptr for unknown path", "[Editor][S69]") {
    VCSRepository repo("repo");
    REQUIRE(repo.findDiff("unknown.cpp") == nullptr);
}

TEST_CASE("VCSRepository modifiedFileCount ignores Unchanged and Ignored", "[Editor][S69]") {
    VCSRepository repo("repo");
    repo.trackFile("a.cpp", VCSFileStatus::Modified);
    repo.trackFile("b.cpp", VCSFileStatus::Unchanged);
    repo.trackFile("c.cpp", VCSFileStatus::Ignored);
    repo.trackFile("d.cpp", VCSFileStatus::Added);
    REQUIRE(repo.modifiedFileCount() == 2);
}

TEST_CASE("VCSRepository kMaxBranches is 64", "[Editor][S69]") {
    REQUIRE(VCSRepository::kMaxBranches == 64);
}

TEST_CASE("VersionControlSystem starts uninitialized", "[Editor][S69]") {
    VersionControlSystem vcs;
    REQUIRE_FALSE(vcs.isInitialized());
    REQUIRE(vcs.repositoryCount() == 0);
}

TEST_CASE("VersionControlSystem init sets initialized", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    REQUIRE(vcs.isInitialized());
}

TEST_CASE("VersionControlSystem openRepository before init returns -1", "[Editor][S69]") {
    VersionControlSystem vcs;
    REQUIRE(vcs.openRepository("my-repo") == -1);
}

TEST_CASE("VersionControlSystem openRepository returns index", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    int idx = vcs.openRepository("my-repo");
    REQUIRE(idx == 0);
    REQUIRE(vcs.repositoryCount() == 1);
}

TEST_CASE("VersionControlSystem openRepository rejects duplicate name", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    vcs.openRepository("my-repo");
    REQUIRE(vcs.openRepository("my-repo") == -1);
    REQUIRE(vcs.repositoryCount() == 1);
}

TEST_CASE("VersionControlSystem repository returns valid pointer", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    int idx = vcs.openRepository("repo");
    VCSRepository* r = vcs.repository(idx);
    REQUIRE(r != nullptr);
    REQUIRE(r->name() == "repo");
}

TEST_CASE("VersionControlSystem repository returns nullptr for invalid index", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    REQUIRE(vcs.repository(-1) == nullptr);
    REQUIRE(vcs.repository(99) == nullptr);
}

TEST_CASE("VersionControlSystem repositoryByName finds repo", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    vcs.openRepository("project");
    REQUIRE(vcs.repositoryByName("project") != nullptr);
    REQUIRE(vcs.repositoryByName("missing") == nullptr);
}

TEST_CASE("VersionControlSystem tick increments tickCount", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    REQUIRE(vcs.tickCount() == 0);
    vcs.tick(0.016f);
    REQUIRE(vcs.tickCount() == 1);
}

TEST_CASE("VersionControlSystem tick does nothing when uninitialized", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.tick(0.016f);
    REQUIRE(vcs.tickCount() == 0);
}

TEST_CASE("VersionControlSystem shutdown clears repos", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    vcs.openRepository("repo1");
    vcs.openRepository("repo2");
    vcs.shutdown();
    REQUIRE_FALSE(vcs.isInitialized());
    REQUIRE(vcs.repositoryCount() == 0);
}

TEST_CASE("VersionControlSystem totalBranches sums across repos", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    int idx0 = vcs.openRepository("r0");
    int idx1 = vcs.openRepository("r1");
    VCSBranchInfo b1; b1.name = "main";
    VCSBranchInfo b2; b2.name = "dev";
    vcs.repository(idx0)->addBranch(b1);
    vcs.repository(idx1)->addBranch(b2);
    REQUIRE(vcs.totalBranches() == 2);
}

TEST_CASE("VersionControlSystem totalModifiedFiles sums across repos", "[Editor][S69]") {
    VersionControlSystem vcs;
    vcs.init();
    int idx = vcs.openRepository("r");
    vcs.repository(idx)->trackFile("a.cpp", VCSFileStatus::Modified);
    vcs.repository(idx)->trackFile("b.cpp", VCSFileStatus::Added);
    REQUIRE(vcs.totalModifiedFiles() == 2);
}
