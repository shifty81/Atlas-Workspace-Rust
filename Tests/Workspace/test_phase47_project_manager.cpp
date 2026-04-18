// Tests/Workspace/test_phase47_project_manager.cpp
// Phase 47 — ProjectManager
//
// Tests for:
//   1. ProjectManagerState — enum values and name helper
//   2. RecentProjectEntry  — isValid
//   3. ProjectManagerConfig — defaults
//   4. ProjectManager       — state machine, newProject/openProject, save,
//                             closeProject, error handling, dirty flag,
//                             auto-save, recent list, config
//   5. Integration          — full open/dirty/save/close cycle, auto-save
//                             fires on accumulated time, recent list capped

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/ProjectManager.h"
#include <string>

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// 1. ProjectManagerState
// ─────────────────────────────────────────────────────────────────

TEST_CASE("ProjectManagerState – all values have names", "[phase47][ProjectManagerState]") {
    CHECK(std::string(projectManagerStateName(ProjectManagerState::Idle))    == "Idle");
    CHECK(std::string(projectManagerStateName(ProjectManagerState::Opening)) == "Opening");
    CHECK(std::string(projectManagerStateName(ProjectManagerState::Open))    == "Open");
    CHECK(std::string(projectManagerStateName(ProjectManagerState::Saving))  == "Saving");
    CHECK(std::string(projectManagerStateName(ProjectManagerState::Closing)) == "Closing");
    CHECK(std::string(projectManagerStateName(ProjectManagerState::Error))   == "Error");
}

// ─────────────────────────────────────────────────────────────────
// 2. RecentProjectEntry
// ─────────────────────────────────────────────────────────────────

TEST_CASE("RecentProjectEntry – default is not valid", "[phase47][RecentProjectEntry]") {
    RecentProjectEntry e;
    CHECK_FALSE(e.isValid());
}

TEST_CASE("RecentProjectEntry – isValid when path set", "[phase47][RecentProjectEntry]") {
    RecentProjectEntry e;
    e.path        = "/projects/my_game";
    e.displayName = "My Game";
    CHECK(e.isValid());
}

// ─────────────────────────────────────────────────────────────────
// 3. ProjectManagerConfig
// ─────────────────────────────────────────────────────────────────

TEST_CASE("ProjectManagerConfig – defaults", "[phase47][ProjectManagerConfig]") {
    ProjectManagerConfig cfg;
    CHECK(cfg.maxRecentProjects   == 10);
    CHECK(cfg.autoSaveIntervalSec == 300.f);
    CHECK(cfg.autoSaveEnabled     == true);
}

// ─────────────────────────────────────────────────────────────────
// 4. ProjectManager
// ─────────────────────────────────────────────────────────────────

TEST_CASE("ProjectManager – default state is Idle", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK(pm.isIdle());
    CHECK(pm.state() == ProjectManagerState::Idle);
    CHECK_FALSE(pm.isOpen());
    CHECK_FALSE(pm.hasError());
    CHECK(pm.isDirty() == false);
    CHECK(pm.activePath().empty());
    CHECK(pm.recentCount() == 0);
}

TEST_CASE("ProjectManager – newProject transitions to Open", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK(pm.newProject("/projects/test", "Test Project"));
    CHECK(pm.isOpen());
    CHECK(pm.state() == ProjectManagerState::Open);
    CHECK(pm.activePath() == "/projects/test");
    CHECK(pm.activeDisplay() == "Test Project");
    CHECK_FALSE(pm.isDirty());
}

TEST_CASE("ProjectManager – newProject empty path rejected", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK_FALSE(pm.newProject(""));
    CHECK(pm.isIdle());
}

TEST_CASE("ProjectManager – newProject rejected when already open", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK(pm.newProject("/projects/a", "A"));
    CHECK_FALSE(pm.newProject("/projects/b", "B"));
    CHECK(pm.activePath() == "/projects/a");
}

TEST_CASE("ProjectManager – openProject works same as newProject", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK(pm.openProject("/projects/existing", "Existing"));
    CHECK(pm.isOpen());
    CHECK(pm.activePath() == "/projects/existing");
}

TEST_CASE("ProjectManager – markDirty sets dirty flag", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/p/q");
    pm.markDirty();
    CHECK(pm.isDirty());
}

TEST_CASE("ProjectManager – markDirty no-op when not open", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.markDirty();
    CHECK_FALSE(pm.isDirty());
}

TEST_CASE("ProjectManager – save clears dirty flag", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/p/q");
    pm.markDirty();
    CHECK(pm.save());
    CHECK_FALSE(pm.isDirty());
    CHECK(pm.isOpen());
    CHECK(pm.saveCount() == 1);
}

TEST_CASE("ProjectManager – save fails when not open", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK_FALSE(pm.save());
    CHECK(pm.saveCount() == 0);
}

TEST_CASE("ProjectManager – closeProject from Open transitions to Idle", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/p/q", "Q");
    CHECK(pm.closeProject());
    CHECK(pm.isIdle());
    CHECK(pm.activePath().empty());
    CHECK(pm.activeDisplay().empty());
    CHECK_FALSE(pm.isDirty());
}

TEST_CASE("ProjectManager – closeProject fails when already Idle", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK_FALSE(pm.closeProject());
}

TEST_CASE("ProjectManager – setError transitions to Error state", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.setError("disk read failed");
    CHECK(pm.hasError());
    CHECK(pm.state() == ProjectManagerState::Error);
    CHECK(pm.errorMessage() == "disk read failed");
}

TEST_CASE("ProjectManager – clearError from Error returns to Idle", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.setError("oops");
    pm.clearError();
    CHECK(pm.isIdle());
    CHECK(pm.errorMessage().empty());
}

TEST_CASE("ProjectManager – clearError no-op when not in error state", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.clearError(); // should not change state
    CHECK(pm.isIdle());
}

TEST_CASE("ProjectManager – closeProject from Error resets state", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/p/q");
    pm.setError("boom");
    CHECK(pm.hasError());
    CHECK(pm.closeProject()); // allowed from Error
    CHECK(pm.isIdle());
    CHECK(pm.errorMessage().empty());
}

// ── Auto-save ─────────────────────────────────────────────────────

TEST_CASE("ProjectManager – tickAutoSave no-op when not open", "[phase47][ProjectManager]") {
    ProjectManager pm;
    CHECK_FALSE(pm.tickAutoSave(9999.f));
}

TEST_CASE("ProjectManager – tickAutoSave no-op when not dirty", "[phase47][ProjectManager]") {
    ProjectManagerConfig cfg;
    cfg.autoSaveIntervalSec = 5.f;
    ProjectManager pm(cfg);
    pm.newProject("/p");
    CHECK_FALSE(pm.tickAutoSave(10.f));
}

TEST_CASE("ProjectManager – tickAutoSave no-op when disabled", "[phase47][ProjectManager]") {
    ProjectManagerConfig cfg;
    cfg.autoSaveEnabled     = false;
    cfg.autoSaveIntervalSec = 1.f;
    ProjectManager pm(cfg);
    pm.newProject("/p");
    pm.markDirty();
    CHECK_FALSE(pm.tickAutoSave(5.f));
}

TEST_CASE("ProjectManager – tickAutoSave triggers after interval", "[phase47][ProjectManager]") {
    ProjectManagerConfig cfg;
    cfg.autoSaveIntervalSec = 10.f;
    ProjectManager pm(cfg);
    pm.newProject("/p");
    pm.markDirty();

    CHECK_FALSE(pm.tickAutoSave(5.f));   // not yet
    CHECK(pm.tickAutoSave(6.f));          // 11 total >= 10 → fires
    CHECK_FALSE(pm.isDirty());            // save() cleared it
}

TEST_CASE("ProjectManager – autoSaveAccum resets after trigger", "[phase47][ProjectManager]") {
    ProjectManagerConfig cfg;
    cfg.autoSaveIntervalSec = 10.f;
    ProjectManager pm(cfg);
    pm.newProject("/p");
    pm.markDirty();
    pm.tickAutoSave(15.f); // triggers
    CHECK(pm.autoSaveAccum() == 0.f);
}

TEST_CASE("ProjectManager – autoSave callback is invoked", "[phase47][ProjectManager]") {
    ProjectManagerConfig cfg;
    cfg.autoSaveIntervalSec = 5.f;
    ProjectManager pm(cfg);

    std::string cbPath;
    pm.setAutoSaveCallback([&](const std::string& p){ cbPath = p; });
    pm.newProject("/projects/cb_test");
    pm.markDirty();
    pm.tickAutoSave(6.f);

    CHECK(cbPath == "/projects/cb_test");
}

// ── Recent projects ───────────────────────────────────────────────

TEST_CASE("ProjectManager – newProject pushes to recent", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/a", "A");
    CHECK(pm.recentCount() == 1);
    CHECK(pm.recentProjects()[0].path == "/a");
}

TEST_CASE("ProjectManager – multiple projects populate recent in order", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/a", "A");
    pm.closeProject();
    pm.newProject("/b", "B");
    pm.closeProject();
    pm.newProject("/c", "C");
    pm.closeProject();

    REQUIRE(pm.recentCount() == 3);
    // Most recent first
    CHECK(pm.recentProjects()[0].path == "/c");
    CHECK(pm.recentProjects()[1].path == "/b");
    CHECK(pm.recentProjects()[2].path == "/a");
}

TEST_CASE("ProjectManager – reopening same path bumps it to front", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/a", "A"); pm.closeProject();
    pm.newProject("/b", "B"); pm.closeProject();
    pm.newProject("/a", "A"); pm.closeProject(); // a should move to front

    REQUIRE(pm.recentCount() == 2);
    CHECK(pm.recentProjects()[0].path == "/a");
    CHECK(pm.recentProjects()[1].path == "/b");
}

TEST_CASE("ProjectManager – recent list capped at maxRecentProjects", "[phase47][ProjectManager]") {
    ProjectManagerConfig cfg;
    cfg.maxRecentProjects = 3;
    ProjectManager pm(cfg);

    for (int i = 0; i < 5; ++i) {
        pm.newProject("/p/" + std::to_string(i));
        pm.closeProject();
    }
    CHECK(pm.recentCount() == 3);
    // Most recent (4) at front
    CHECK(pm.recentProjects()[0].path == "/p/4");
}

TEST_CASE("ProjectManager – clearRecent empties list", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/a"); pm.closeProject();
    pm.newProject("/b"); pm.closeProject();
    pm.clearRecent();
    CHECK(pm.recentCount() == 0);
}

TEST_CASE("ProjectManager – removeRecent by path", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/a"); pm.closeProject();
    pm.newProject("/b"); pm.closeProject();
    CHECK(pm.removeRecent("/a"));
    CHECK(pm.recentCount() == 1);
    CHECK(pm.recentProjects()[0].path == "/b");
}

TEST_CASE("ProjectManager – removeRecent unknown returns false", "[phase47][ProjectManager]") {
    ProjectManager pm;
    pm.newProject("/a"); pm.closeProject();
    CHECK_FALSE(pm.removeRecent("/missing"));
    CHECK(pm.recentCount() == 1);
}

// ── Config ────────────────────────────────────────────────────────

TEST_CASE("ProjectManager – setConfig updates knobs", "[phase47][ProjectManager]") {
    ProjectManager pm;
    ProjectManagerConfig cfg;
    cfg.maxRecentProjects   = 5;
    cfg.autoSaveIntervalSec = 60.f;
    cfg.autoSaveEnabled     = false;
    pm.setConfig(cfg);
    CHECK(pm.config().maxRecentProjects   == 5);
    CHECK(pm.config().autoSaveIntervalSec == 60.f);
    CHECK(pm.config().autoSaveEnabled     == false);
}

// ─────────────────────────────────────────────────────────────────
// 5. Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Integration – full open/dirty/save/close cycle", "[phase47][integration]") {
    ProjectManager pm;
    REQUIRE(pm.isIdle());

    REQUIRE(pm.newProject("/workspace/my_game", "My Game"));
    CHECK(pm.isOpen());
    CHECK_FALSE(pm.isDirty());

    pm.markDirty();
    CHECK(pm.isDirty());

    REQUIRE(pm.save());
    CHECK_FALSE(pm.isDirty());
    CHECK(pm.saveCount() == 1);

    pm.markDirty(); // new change after save
    // Another save
    REQUIRE(pm.save());
    CHECK(pm.saveCount() == 2);

    REQUIRE(pm.closeProject());
    CHECK(pm.isIdle());
    CHECK(pm.activePath().empty());
    CHECK_FALSE(pm.isDirty());
}

TEST_CASE("Integration – auto-save fires multiple times", "[phase47][integration]") {
    ProjectManagerConfig cfg;
    cfg.autoSaveIntervalSec = 10.f;
    ProjectManager pm(cfg);
    pm.newProject("/p");

    int callCount = 0;
    pm.setAutoSaveCallback([&](const std::string&){ ++callCount; });

    pm.markDirty();
    pm.tickAutoSave(11.f); // first fire
    CHECK(callCount == 1);
    CHECK_FALSE(pm.isDirty()); // save() called

    pm.markDirty();
    pm.tickAutoSave(5.f);  // accumulate but not yet
    CHECK(callCount == 1);
    pm.tickAutoSave(6.f);  // second fire
    CHECK(callCount == 2);
}

TEST_CASE("Integration – error then reopen", "[phase47][integration]") {
    ProjectManager pm;
    pm.newProject("/p");
    pm.setError("load failed");
    CHECK(pm.hasError());

    pm.closeProject();
    CHECK(pm.isIdle());

    // After clearing from error, can open new project
    CHECK(pm.newProject("/q", "Q"));
    CHECK(pm.isOpen());
    CHECK(pm.activePath() == "/q");
}
