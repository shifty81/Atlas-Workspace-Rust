#include "../editor/panels/AIDiffViewerPanel.h"
#include <iostream>
#include <cassert>

static atlas::editor::DiffSet MakeSampleDiff() {
    atlas::editor::DiffSet ds;
    ds.title = "AI Refactor";
    ds.description = "Refactor suggestion";
    ds.timestamp = 1000;

    atlas::editor::DiffFile f;
    f.path = "src/main.cpp";
    atlas::editor::DiffHunk h1;
    h1.lineStart = 10;
    h1.lineCount = 3;
    h1.oldContent = "int x = 0;";
    h1.newContent = "int x = 1;";
    atlas::editor::DiffHunk h2;
    h2.lineStart = 20;
    h2.lineCount = 1;
    h2.oldContent = "return 0;";
    h2.newContent = "return x;";
    f.hunks.push_back(h1);
    f.hunks.push_back(h2);
    ds.files.push_back(f);

    atlas::editor::DiffFile f2;
    f2.path = "src/util.h";
    f2.isNew = true;
    atlas::editor::DiffHunk h3;
    h3.lineStart = 1;
    h3.newContent = "#pragma once";
    f2.hunks.push_back(h3);
    ds.files.push_back(f2);

    return ds;
}

void test_diff_viewer_name() {
    atlas::editor::AIDiffViewerPanel panel;
    assert(std::string(panel.Name()) == "AI Diff Viewer");
    std::cout << "[PASS] test_diff_viewer_name" << std::endl;
}

void test_diff_viewer_no_diff() {
    atlas::editor::AIDiffViewerPanel panel;
    assert(!panel.HasDiff());
    assert(panel.CurrentDiff() == nullptr);
    std::cout << "[PASS] test_diff_viewer_no_diff" << std::endl;
}

void test_diff_viewer_load() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    assert(panel.HasDiff());
    assert(panel.CurrentDiff() != nullptr);
    assert(panel.CurrentDiff()->title == "AI Refactor");
    assert(panel.CurrentDiff()->files.size() == 2);
    std::cout << "[PASS] test_diff_viewer_load" << std::endl;
}

void test_diff_viewer_accept_hunk() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    assert(!panel.IsHunkAccepted(0, 0));
    panel.AcceptHunk(0, 0);
    assert(panel.IsHunkAccepted(0, 0));
    assert(!panel.IsHunkRejected(0, 0));
    std::cout << "[PASS] test_diff_viewer_accept_hunk" << std::endl;
}

void test_diff_viewer_reject_hunk() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    panel.RejectHunk(0, 1);
    assert(panel.IsHunkRejected(0, 1));
    assert(!panel.IsHunkAccepted(0, 1));
    std::cout << "[PASS] test_diff_viewer_reject_hunk" << std::endl;
}

void test_diff_viewer_accept_all() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    panel.AcceptAll();
    assert(panel.AcceptedHunkCount() == 3);
    assert(panel.TotalHunkCount() == 3);
    std::cout << "[PASS] test_diff_viewer_accept_all" << std::endl;
}

void test_diff_viewer_reject_all() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    panel.AcceptAll();
    panel.RejectAll();
    assert(panel.AcceptedHunkCount() == 0);
    std::cout << "[PASS] test_diff_viewer_reject_all" << std::endl;
}

void test_diff_viewer_apply() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    panel.AcceptAll();
    assert(panel.ApplyAccepted());
    assert(panel.CurrentDiff()->applied);
    std::cout << "[PASS] test_diff_viewer_apply" << std::endl;
}

void test_diff_viewer_history() {
    atlas::editor::AIDiffViewerPanel panel;
    auto ds = MakeSampleDiff();
    panel.AddToHistory(ds);
    panel.AddToHistory(ds);
    assert(panel.History().size() == 2);
    std::cout << "[PASS] test_diff_viewer_history" << std::endl;
}

void test_diff_viewer_clear() {
    atlas::editor::AIDiffViewerPanel panel;
    panel.LoadDiffSet(MakeSampleDiff());
    panel.AddToHistory(MakeSampleDiff());
    panel.Clear();
    assert(!panel.HasDiff());
    assert(panel.History().empty());
    std::cout << "[PASS] test_diff_viewer_clear" << std::endl;
}
