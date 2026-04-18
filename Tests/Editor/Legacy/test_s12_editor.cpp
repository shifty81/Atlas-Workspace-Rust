#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S12 Tests: Version Control Integration ──────────────────────

TEST_CASE("VCSProviderType names", "[Editor][S12]") {
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Git))       == "Git");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::SVN))       == "SVN");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Perforce))  == "Perforce");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Mercurial)) == "Mercurial");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Plastic))   == "Plastic");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Fossil))    == "Fossil");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::Custom))    == "Custom");
    REQUIRE(std::string(vcsProviderTypeName(VCSProviderType::None))      == "None");
}

TEST_CASE("VCSFileStatus names", "[Editor][S12]") {
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Untracked))  == "Untracked");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Added))      == "Added");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Modified))   == "Modified");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Deleted))    == "Deleted");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Renamed))    == "Renamed");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Conflicted)) == "Conflicted");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Ignored))    == "Ignored");
    REQUIRE(std::string(vcsFileStatusName(VCSFileStatus::Unchanged))  == "Unchanged");
}

TEST_CASE("VCSCommitInfo defaults and validity", "[Editor][S12]") {
    VCSCommitInfo c;
    REQUIRE_FALSE(c.isValid());
    REQUIRE(c.isRoot());
    REQUIRE_FALSE(c.hasMessage());

    c.hash = "abc123";
    c.author = "dev";
    REQUIRE(c.isValid());

    c.message = "Initial commit";
    REQUIRE(c.hasMessage());

    c.parentHash = "parent1";
    REQUIRE_FALSE(c.isRoot());
}

TEST_CASE("VCSBranchInfo defaults", "[Editor][S12]") {
    VCSBranchInfo b;
    REQUIRE_FALSE(b.isActive);
    REQUIRE_FALSE(b.isRemote);
    REQUIRE(b.isLocal());
    REQUIRE(b.isSynced());
    REQUIRE_FALSE(b.hasCommits());

    b.lastCommitHash = "abc";
    REQUIRE(b.hasCommits());

    b.aheadCount = 3;
    REQUIRE_FALSE(b.isSynced());
}

TEST_CASE("VCSDiffEntry changes", "[Editor][S12]") {
    VCSDiffEntry d;
    REQUIRE_FALSE(d.hasChanges());
    REQUIRE(d.totalChanges() == 0);

    d.additions = 10;
    d.deletions = 5;
    REQUIRE(d.hasChanges());
    REQUIRE(d.totalChanges() == 15);
}

TEST_CASE("VCSRepository addBranch + duplicate rejection", "[Editor][S12]") {
    VCSRepository repo("TestRepo");
    VCSBranchInfo main; main.name = "main";
    VCSBranchInfo dup;  dup.name = "main";

    REQUIRE(repo.addBranch(main));
    REQUIRE(repo.branchCount() == 1);
    REQUIRE_FALSE(repo.addBranch(dup));
    REQUIRE(repo.branchCount() == 1);
}

TEST_CASE("VCSRepository switchBranch", "[Editor][S12]") {
    VCSRepository repo("TestRepo");
    VCSBranchInfo main; main.name = "main"; main.isActive = true;
    VCSBranchInfo dev;  dev.name = "dev";
    repo.addBranch(main);
    repo.addBranch(dev);

    REQUIRE(repo.switchBranch("dev"));
    REQUIRE(repo.activeBranch()->name == "dev");
    REQUIRE_FALSE(repo.switchBranch("nonexistent"));
}

TEST_CASE("VCSRepository removeBranch cannot remove active", "[Editor][S12]") {
    VCSRepository repo("TestRepo");
    VCSBranchInfo main; main.name = "main"; main.isActive = true;
    VCSBranchInfo dev;  dev.name = "dev";
    repo.addBranch(main);
    repo.addBranch(dev);

    REQUIRE_FALSE(repo.removeBranch("main")); // active
    REQUIRE(repo.removeBranch("dev"));
    REQUIRE(repo.branchCount() == 1);
}

TEST_CASE("VCSRepository addCommit + validation", "[Editor][S12]") {
    VCSRepository repo("TestRepo");
    VCSCommitInfo bad;
    REQUIRE_FALSE(repo.addCommit(bad)); // invalid

    VCSCommitInfo good;
    good.hash = "abc"; good.author = "dev"; good.message = "fix";
    REQUIRE(repo.addCommit(good));
    REQUIRE(repo.commitCount() == 1);
}

TEST_CASE("VCSRepository trackFile + findDiff", "[Editor][S12]") {
    VCSRepository repo("TestRepo");
    REQUIRE(repo.trackFile("src/main.cpp", VCSFileStatus::Modified));
    REQUIRE(repo.diffCount() == 1);
    REQUIRE(repo.modifiedFileCount() == 1);

    auto* diff = repo.findDiff("src/main.cpp");
    REQUIRE(diff != nullptr);
    REQUIRE(diff->status == VCSFileStatus::Modified);

    // Update existing file status
    REQUIRE(repo.trackFile("src/main.cpp", VCSFileStatus::Unchanged));
    REQUIRE(repo.diffCount() == 1); // no duplicate
    REQUIRE(repo.modifiedFileCount() == 0); // now unchanged
}

TEST_CASE("VersionControlSystem init/shutdown", "[Editor][S12]") {
    VersionControlSystem vcs;
    REQUIRE_FALSE(vcs.isInitialized());
    vcs.init();
    REQUIRE(vcs.isInitialized());
    vcs.shutdown();
    REQUIRE_FALSE(vcs.isInitialized());
}

TEST_CASE("VersionControlSystem openRepository + duplicate rejection", "[Editor][S12]") {
    VersionControlSystem vcs;
    vcs.init();
    REQUIRE(vcs.openRepository("Repo1") == 0);
    REQUIRE(vcs.openRepository("Repo2", VCSProviderType::SVN) == 1);
    REQUIRE(vcs.openRepository("Repo1") == -1); // duplicate
    REQUIRE(vcs.repositoryCount() == 2);
}

TEST_CASE("VersionControlSystem repositoryByName", "[Editor][S12]") {
    VersionControlSystem vcs;
    vcs.init();
    vcs.openRepository("MyRepo");
    REQUIRE(vcs.repositoryByName("MyRepo") != nullptr);
    REQUIRE(vcs.repositoryByName("MyRepo")->name() == "MyRepo");
    REQUIRE(vcs.repositoryByName("Other") == nullptr);
}

TEST_CASE("VersionControlSystem tick + totalBranches/Commits", "[Editor][S12]") {
    VersionControlSystem vcs;
    vcs.init();
    vcs.openRepository("R1");

    auto* r = vcs.repositoryByName("R1");
    VCSBranchInfo b; b.name = "main";
    r->addBranch(b);

    VCSCommitInfo c; c.hash = "abc"; c.author = "dev";
    r->addCommit(c);

    r->trackFile("a.txt", VCSFileStatus::Added);

    vcs.tick(0.016f);
    vcs.tick(0.016f);

    REQUIRE(vcs.tickCount() == 2);
    REQUIRE(vcs.totalBranches() == 1);
    REQUIRE(vcs.totalCommits() == 1);
    REQUIRE(vcs.totalModifiedFiles() == 1);
}

TEST_CASE("VersionControlSystem not initialized rejects operations", "[Editor][S12]") {
    VersionControlSystem vcs;
    REQUIRE(vcs.openRepository("Repo1") == -1);
}
