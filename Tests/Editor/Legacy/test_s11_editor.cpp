#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S11 Tests: Live Collaboration System ────────────────────────

TEST_CASE("CollabUserRole names", "[Editor][S11]") {
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Owner))    == "Owner");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Admin))    == "Admin");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Editor))   == "Editor");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Reviewer)) == "Reviewer");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Viewer))   == "Viewer");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Builder))  == "Builder");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Tester))   == "Tester");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Guest))    == "Guest");
}

TEST_CASE("CollabEditType names", "[Editor][S11]") {
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Insert))  == "Insert");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Delete))  == "Delete");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Modify))  == "Modify");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Move))    == "Move");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Rename))  == "Rename");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Create))  == "Create");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Lock))    == "Lock");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Unlock))  == "Unlock");
}

TEST_CASE("CollabUser defaults", "[Editor][S11]") {
    CollabUser u;
    REQUIRE(u.role == CollabUserRole::Guest);
    REQUIRE_FALSE(u.connected);
    REQUIRE_FALSE(u.canEdit());
    REQUIRE_FALSE(u.canReview());
}

TEST_CASE("CollabUser canEdit and canReview by role", "[Editor][S11]") {
    CollabUser owner;   owner.role = CollabUserRole::Owner;
    CollabUser admin;   admin.role = CollabUserRole::Admin;
    CollabUser editor;  editor.role = CollabUserRole::Editor;
    CollabUser reviewer; reviewer.role = CollabUserRole::Reviewer;
    CollabUser viewer;  viewer.role = CollabUserRole::Viewer;

    REQUIRE(owner.canEdit());
    REQUIRE(admin.canEdit());
    REQUIRE(editor.canEdit());
    REQUIRE_FALSE(reviewer.canEdit());
    REQUIRE_FALSE(viewer.canEdit());

    REQUIRE(owner.canReview());
    REQUIRE(editor.canReview());
    REQUIRE(reviewer.canReview());
    REQUIRE_FALSE(viewer.canReview());
}

TEST_CASE("CollabUser connect/disconnect/touch", "[Editor][S11]") {
    CollabUser u;
    REQUIRE_FALSE(u.isConnected());
    u.connect(10.0);
    REQUIRE(u.isConnected());
    REQUIRE(u.lastActivityTime == Catch::Approx(10.0));

    u.touch(20.0);
    REQUIRE(u.lastActivityTime == Catch::Approx(20.0));

    u.disconnect();
    REQUIRE_FALSE(u.isConnected());
}

TEST_CASE("CollabEditAction isValid", "[Editor][S11]") {
    CollabEditAction a;
    REQUIRE_FALSE(a.isValid());

    a.actionId = "act1";
    REQUIRE_FALSE(a.isValid());

    a.userId = "user1";
    REQUIRE_FALSE(a.isValid());

    a.targetPath = "scene/obj.nf";
    REQUIRE(a.isValid());
}

TEST_CASE("CollabSession addUser + duplicate rejection", "[Editor][S11]") {
    CollabSession sess("TestSession");
    CollabUser u1; u1.userId = "u1"; u1.displayName = "Alice";
    CollabUser u2; u2.userId = "u1"; u2.displayName = "Duplicate";

    REQUIRE(sess.addUser(u1));
    REQUIRE(sess.userCount() == 1);
    REQUIRE_FALSE(sess.addUser(u2));
    REQUIRE(sess.userCount() == 1);
}

TEST_CASE("CollabSession removeUser", "[Editor][S11]") {
    CollabSession sess("TestSession");
    CollabUser u; u.userId = "u1";
    sess.addUser(u);
    REQUIRE(sess.removeUser("u1"));
    REQUIRE(sess.userCount() == 0);
    REQUIRE_FALSE(sess.removeUser("nonexistent"));
}

TEST_CASE("CollabSession findUser", "[Editor][S11]") {
    CollabSession sess("TestSession");
    CollabUser u; u.userId = "u1"; u.displayName = "Alice";
    sess.addUser(u);
    REQUIRE(sess.findUser("u1") != nullptr);
    REQUIRE(sess.findUser("u1")->displayName == "Alice");
    REQUIRE(sess.findUser("u2") == nullptr);
}

TEST_CASE("CollabSession submitAction requires editor role", "[Editor][S11]") {
    CollabSession sess("TestSession");
    CollabUser viewer; viewer.userId = "v1"; viewer.role = CollabUserRole::Viewer;
    CollabUser editor; editor.userId = "e1"; editor.role = CollabUserRole::Editor;
    sess.addUser(viewer);
    sess.addUser(editor);

    CollabEditAction a;
    a.actionId = "a1"; a.userId = "v1"; a.targetPath = "obj.nf"; a.timestamp = 1.0;
    REQUIRE_FALSE(sess.submitAction(a)); // viewer can't edit

    a.userId = "e1";
    REQUIRE(sess.submitAction(a)); // editor can
    REQUIRE(sess.actionCount() == 1);
}

TEST_CASE("CollabSession submitAction invalid action rejected", "[Editor][S11]") {
    CollabSession sess("TestSession");
    CollabUser u; u.userId = "u1"; u.role = CollabUserRole::Owner;
    sess.addUser(u);

    CollabEditAction bad; // empty fields
    REQUIRE_FALSE(sess.submitAction(bad));
}

TEST_CASE("CollabSession conflict detection on same path within window", "[Editor][S11]") {
    CollabSession sess("TestSession");
    sess.setConflictWindow(5.0);

    CollabUser u1; u1.userId = "u1"; u1.role = CollabUserRole::Editor;
    CollabUser u2; u2.userId = "u2"; u2.role = CollabUserRole::Editor;
    sess.addUser(u1);
    sess.addUser(u2);

    CollabEditAction a1;
    a1.actionId = "a1"; a1.userId = "u1"; a1.targetPath = "scene.nf"; a1.timestamp = 1.0;
    REQUIRE(sess.submitAction(a1));
    REQUIRE(sess.conflictCount() == 0);

    CollabEditAction a2;
    a2.actionId = "a2"; a2.userId = "u2"; a2.targetPath = "scene.nf"; a2.timestamp = 3.0;
    REQUIRE(sess.submitAction(a2));
    REQUIRE(sess.conflictCount() == 1); // within 5s window
}

TEST_CASE("CollabSession no conflict outside window", "[Editor][S11]") {
    CollabSession sess("TestSession");
    sess.setConflictWindow(2.0);

    CollabUser u1; u1.userId = "u1"; u1.role = CollabUserRole::Editor;
    CollabUser u2; u2.userId = "u2"; u2.role = CollabUserRole::Editor;
    sess.addUser(u1);
    sess.addUser(u2);

    CollabEditAction a1;
    a1.actionId = "a1"; a1.userId = "u1"; a1.targetPath = "scene.nf"; a1.timestamp = 1.0;
    sess.submitAction(a1);

    CollabEditAction a2;
    a2.actionId = "a2"; a2.userId = "u2"; a2.targetPath = "scene.nf"; a2.timestamp = 10.0;
    sess.submitAction(a2);
    REQUIRE(sess.conflictCount() == 0); // outside 2s window
}

TEST_CASE("CollabSession connectedCount + editorCount", "[Editor][S11]") {
    CollabSession sess("TestSession");
    CollabUser u1; u1.userId = "u1"; u1.role = CollabUserRole::Owner; u1.connected = true;
    CollabUser u2; u2.userId = "u2"; u2.role = CollabUserRole::Viewer; u2.connected = true;
    CollabUser u3; u3.userId = "u3"; u3.role = CollabUserRole::Editor; u3.connected = false;
    sess.addUser(u1);
    sess.addUser(u2);
    sess.addUser(u3);

    REQUIRE(sess.connectedCount() == 2);
    REQUIRE(sess.editorCount() == 2); // Owner + Editor
}

TEST_CASE("CollabConflictResolver no_conflict for different paths", "[Editor][S11]") {
    CollabConflictResolver resolver;
    CollabEditAction a; a.actionId = "a1"; a.targetPath = "a.nf";
    CollabEditAction b; b.actionId = "b1"; b.targetPath = "b.nf";
    auto res = resolver.resolve(a, b);
    REQUIRE(res.autoResolved);
    REQUIRE(res.strategy == "no_conflict");
    REQUIRE(resolver.autoResolved() == 1);
}

TEST_CASE("CollabConflictResolver last_writer_wins for same type same path", "[Editor][S11]") {
    CollabConflictResolver resolver;
    CollabEditAction a; a.actionId = "a1"; a.targetPath = "x.nf"; a.type = CollabEditType::Modify;
    CollabEditAction b; b.actionId = "b1"; b.targetPath = "x.nf"; b.type = CollabEditType::Modify;
    auto res = resolver.resolve(a, b);
    REQUIRE(res.autoResolved);
    REQUIRE(res.strategy == "last_writer_wins");
}

TEST_CASE("CollabConflictResolver manual for different types same path", "[Editor][S11]") {
    CollabConflictResolver resolver;
    CollabEditAction a; a.actionId = "a1"; a.targetPath = "x.nf"; a.type = CollabEditType::Modify;
    CollabEditAction b; b.actionId = "b1"; b.targetPath = "x.nf"; b.type = CollabEditType::Delete;
    auto res = resolver.resolve(a, b);
    REQUIRE_FALSE(res.autoResolved);
    REQUIRE(res.strategy == "manual");
    REQUIRE(resolver.manualRequired() == 1);
}

TEST_CASE("LiveCollaborationSystem init/shutdown", "[Editor][S11]") {
    LiveCollaborationSystem lcs;
    REQUIRE_FALSE(lcs.isInitialized());
    lcs.init();
    REQUIRE(lcs.isInitialized());
    lcs.shutdown();
    REQUIRE_FALSE(lcs.isInitialized());
}

TEST_CASE("LiveCollaborationSystem createSession + duplicate rejection", "[Editor][S11]") {
    LiveCollaborationSystem lcs;
    lcs.init();
    REQUIRE(lcs.createSession("Session1") == 0);
    REQUIRE(lcs.createSession("Session2") == 1);
    REQUIRE(lcs.createSession("Session1") == -1); // duplicate
    REQUIRE(lcs.sessionCount() == 2);
}

TEST_CASE("LiveCollaborationSystem join/leave + totalConnectedUsers", "[Editor][S11]") {
    LiveCollaborationSystem lcs;
    lcs.init();
    lcs.createSession("S1");

    CollabUser u; u.userId = "u1"; u.displayName = "Alice"; u.role = CollabUserRole::Editor; u.connected = true;
    REQUIRE(lcs.joinSession("S1", u));
    REQUIRE(lcs.totalConnectedUsers() == 1);

    REQUIRE(lcs.leaveSession("S1", "u1"));
    REQUIRE(lcs.totalConnectedUsers() == 0);
}

TEST_CASE("LiveCollaborationSystem tick + totalActions", "[Editor][S11]") {
    LiveCollaborationSystem lcs;
    lcs.init();
    lcs.createSession("S1");

    CollabUser u; u.userId = "u1"; u.role = CollabUserRole::Owner;
    lcs.joinSession("S1", u);

    CollabEditAction a;
    a.actionId = "a1"; a.userId = "u1"; a.targetPath = "obj.nf"; a.timestamp = 1.0;
    lcs.sessionByName("S1")->submitAction(a);

    lcs.tick(0.016f);
    lcs.tick(0.016f);
    REQUIRE(lcs.tickCount() == 2);
    REQUIRE(lcs.totalActions() == 1);
}
