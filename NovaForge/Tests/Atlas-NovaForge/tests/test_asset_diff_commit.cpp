#include "../editor/tools/AssetDiffCommitFlow.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;

void test_asset_diff_commit_empty() {
    AssetDiffCommitFlow flow;
    assert(!flow.HasChanges());
    assert(!flow.HasStagedChanges());
    assert(flow.Changes().empty());
    assert(flow.Status() == CommitStatus::Idle);
    assert(flow.CommitCount() == 0);
    std::cout << "[PASS] test_asset_diff_commit_empty" << std::endl;
}

void test_asset_diff_commit_track_change() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "color", "#000", "#FFF"});

    flow.TrackChange("materials/steel.mat", diff);
    assert(flow.HasChanges());
    assert(flow.Changes().size() == 1);
    assert(flow.Changes()[0].assetPath == "materials/steel.mat");

    std::cout << "[PASS] test_asset_diff_commit_track_change" << std::endl;
}

void test_asset_diff_commit_untrack() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    flow.TrackChange("a.mat", diff);
    flow.TrackChange("b.mat", diff);
    assert(flow.Changes().size() == 2);

    flow.UntrackChange("a.mat");
    assert(flow.Changes().size() == 1);
    assert(flow.Changes()[0].assetPath == "b.mat");

    std::cout << "[PASS] test_asset_diff_commit_untrack" << std::endl;
}

void test_asset_diff_commit_stage() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    flow.TrackChange("a.mat", diff);
    flow.TrackChange("b.mat", diff);

    assert(!flow.HasStagedChanges());

    flow.Stage("a.mat");
    assert(flow.HasStagedChanges());
    assert(flow.StagedChanges().size() == 1);

    flow.StageAll();
    assert(flow.StagedChanges().size() == 2);

    flow.Unstage("a.mat");
    assert(flow.StagedChanges().size() == 1);

    flow.UnstageAll();
    assert(!flow.HasStagedChanges());

    std::cout << "[PASS] test_asset_diff_commit_stage" << std::endl;
}

void test_asset_diff_commit_commit_success() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    flow.TrackChange("a.mat", diff);
    flow.StageAll();

    bool commitCalled = false;
    flow.SetCommitCallback([&](const std::vector<AssetChange>& changes, const std::string& msg) {
        commitCalled = true;
        return true;
    });

    bool result = flow.Commit("Update steel material");
    assert(result);
    assert(commitCalled);
    assert(flow.Status() == CommitStatus::Committed);
    assert(flow.CommitCount() == 1);
    assert(flow.Changes().empty()); // committed changes removed

    std::cout << "[PASS] test_asset_diff_commit_commit_success" << std::endl;
}

void test_asset_diff_commit_commit_no_staged() {
    AssetDiffCommitFlow flow;

    bool result = flow.Commit("Empty commit");
    assert(!result);
    assert(flow.Status() == CommitStatus::Failed);
    assert(flow.LastError() == "No staged changes to commit");

    std::cout << "[PASS] test_asset_diff_commit_commit_no_staged" << std::endl;
}

void test_asset_diff_commit_validation_failure() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    flow.TrackChange("a.mat", diff);
    flow.StageAll();

    flow.SetValidateCallback([](const std::vector<AssetChange>&) {
        return false;
    });

    bool result = flow.Commit("Should fail");
    assert(!result);
    assert(flow.Status() == CommitStatus::Failed);
    assert(flow.LastError() == "Validation failed");

    std::cout << "[PASS] test_asset_diff_commit_validation_failure" << std::endl;
}

void test_asset_diff_commit_clear() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    flow.TrackChange("a.mat", diff);
    flow.StageAll();
    flow.Clear();

    assert(!flow.HasChanges());
    assert(!flow.HasStagedChanges());
    assert(flow.Status() == CommitStatus::Idle);

    std::cout << "[PASS] test_asset_diff_commit_clear" << std::endl;
}

void test_asset_diff_commit_summary() {
    AssetDiffCommitFlow flow;

    DiffResult diff;
    diff.modifiedCount = 1;
    diff.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    flow.TrackChange("a.mat", diff);
    flow.Stage("a.mat");

    std::string summary = flow.Summary();
    assert(summary.find("Tracked: 1") != std::string::npos);
    assert(summary.find("Staged: 1") != std::string::npos);

    std::cout << "[PASS] test_asset_diff_commit_summary" << std::endl;
}

void test_asset_diff_commit_update_tracked() {
    AssetDiffCommitFlow flow;

    DiffResult diff1;
    diff1.modifiedCount = 1;
    diff1.entries.push_back({DiffEntryType::Modified, "k", "a", "b"});

    DiffResult diff2;
    diff2.modifiedCount = 2;
    diff2.entries.push_back({DiffEntryType::Modified, "k", "b", "c"});
    diff2.entries.push_back({DiffEntryType::Added, "k2", "", "d"});

    flow.TrackChange("a.mat", diff1);
    flow.TrackChange("a.mat", diff2); // update existing
    assert(flow.Changes().size() == 1);
    assert(flow.Changes()[0].diff.modifiedCount == 2);

    std::cout << "[PASS] test_asset_diff_commit_update_tracked" << std::endl;
}
