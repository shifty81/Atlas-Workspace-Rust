#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S74: Collaboration ───────────────────────────────────────────

TEST_CASE("CollabUserRole names are correct", "[Editor][S74]") {
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Owner))    == "Owner");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Admin))    == "Admin");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Editor))   == "Editor");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Reviewer)) == "Reviewer");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Viewer))   == "Viewer");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Builder))  == "Builder");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Tester))   == "Tester");
    REQUIRE(std::string(collabUserRoleName(CollabUserRole::Guest))    == "Guest");
}

TEST_CASE("CollabEditType names are correct", "[Editor][S74]") {
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Insert))  == "Insert");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Delete))  == "Delete");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Modify))  == "Modify");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Move))    == "Move");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Rename))  == "Rename");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Create))  == "Create");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Lock))    == "Lock");
    REQUIRE(std::string(collabEditTypeName(CollabEditType::Unlock))  == "Unlock");
}

TEST_CASE("CollabUser canEdit for Owner", "[Editor][S74]") {
    CollabUser u;
    u.userId = "u1";
    u.role = CollabUserRole::Owner;
    REQUIRE(u.canEdit());
    REQUIRE(u.canReview());
}

TEST_CASE("CollabUser canEdit for Admin and Editor", "[Editor][S74]") {
    CollabUser admin; admin.userId = "a"; admin.role = CollabUserRole::Admin;
    CollabUser editor; editor.userId = "e"; editor.role = CollabUserRole::Editor;
    REQUIRE(admin.canEdit());
    REQUIRE(editor.canEdit());
}

TEST_CASE("CollabUser cannot edit as Viewer or Guest", "[Editor][S74]") {
    CollabUser v; v.userId = "v"; v.role = CollabUserRole::Viewer;
    CollabUser g; g.userId = "g"; g.role = CollabUserRole::Guest;
    REQUIRE_FALSE(v.canEdit());
    REQUIRE_FALSE(g.canEdit());
}

TEST_CASE("CollabUser Reviewer canReview but not edit", "[Editor][S74]") {
    CollabUser r; r.userId = "r"; r.role = CollabUserRole::Reviewer;
    REQUIRE_FALSE(r.canEdit());
    REQUIRE(r.canReview());
}

TEST_CASE("CollabUser connect sets connected and activity time", "[Editor][S74]") {
    CollabUser u; u.userId = "u";
    REQUIRE_FALSE(u.isConnected());
    u.connect(100.0);
    REQUIRE(u.isConnected());
    REQUIRE(u.lastActivityTime == 100.0);
}

TEST_CASE("CollabUser disconnect clears connected flag", "[Editor][S74]") {
    CollabUser u; u.userId = "u";
    u.connect(1.0);
    u.disconnect();
    REQUIRE_FALSE(u.isConnected());
}

TEST_CASE("CollabUser touch updates activity time", "[Editor][S74]") {
    CollabUser u; u.userId = "u";
    u.touch(200.0);
    REQUIRE(u.lastActivityTime == 200.0);
}

TEST_CASE("CollabEditAction defaults are invalid", "[Editor][S74]") {
    CollabEditAction a;
    REQUIRE_FALSE(a.isValid());
}

TEST_CASE("CollabEditAction valid when actionId, userId, targetPath set", "[Editor][S74]") {
    CollabEditAction a;
    a.actionId = "act1";
    a.userId = "user1";
    a.targetPath = "/scene/entity";
    REQUIRE(a.isValid());
}

TEST_CASE("CollabEditAction markApplied and markConflicted work", "[Editor][S74]") {
    CollabEditAction a;
    REQUIRE_FALSE(a.applied);
    a.markApplied();
    REQUIRE(a.applied);
    REQUIRE_FALSE(a.conflicted);
    a.markConflicted();
    REQUIRE(a.conflicted);
}

TEST_CASE("CollabSession starts empty", "[Editor][S74]") {
    CollabSession s("session1");
    REQUIRE(s.name() == "session1");
    REQUIRE(s.userCount() == 0);
    REQUIRE(s.actionCount() == 0);
    REQUIRE(s.conflictCount() == 0);
}

TEST_CASE("CollabSession addUser increases userCount", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser u; u.userId = "u1"; u.displayName = "Alice"; u.role = CollabUserRole::Editor;
    REQUIRE(s.addUser(u));
    REQUIRE(s.userCount() == 1);
}

TEST_CASE("CollabSession addUser rejects duplicate userId", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser u; u.userId = "u1";
    s.addUser(u);
    REQUIRE_FALSE(s.addUser(u));
    REQUIRE(s.userCount() == 1);
}

TEST_CASE("CollabSession removeUser removes entry", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser u; u.userId = "u1";
    s.addUser(u);
    REQUIRE(s.removeUser("u1"));
    REQUIRE(s.userCount() == 0);
}

TEST_CASE("CollabSession removeUser returns false for missing userId", "[Editor][S74]") {
    CollabSession s("s");
    REQUIRE_FALSE(s.removeUser("ghost"));
}

TEST_CASE("CollabSession findUser returns correct user", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser u; u.userId = "u1"; u.displayName = "Bob";
    s.addUser(u);
    REQUIRE(s.findUser("u1") != nullptr);
    REQUIRE(s.findUser("u1")->displayName == "Bob");
    REQUIRE(s.findUser("other") == nullptr);
}

TEST_CASE("CollabSession submitAction requires user with edit permission", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser viewer; viewer.userId = "viewer"; viewer.role = CollabUserRole::Viewer;
    s.addUser(viewer);

    CollabEditAction a;
    a.actionId = "a1"; a.userId = "viewer"; a.targetPath = "/x";
    REQUIRE_FALSE(s.submitAction(a));
}

TEST_CASE("CollabSession submitAction succeeds for Editor user", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser editor; editor.userId = "ed"; editor.role = CollabUserRole::Editor;
    s.addUser(editor);

    CollabEditAction a;
    a.actionId = "a1"; a.userId = "ed"; a.targetPath = "/scene/entity";
    a.timestamp = 10.0;
    REQUIRE(s.submitAction(a));
    REQUIRE(s.actionCount() == 1);
}

TEST_CASE("CollabSession submitAction rejects invalid action", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser owner; owner.userId = "owner"; owner.role = CollabUserRole::Owner;
    s.addUser(owner);

    CollabEditAction a; // invalid (no actionId/userId/targetPath)
    REQUIRE_FALSE(s.submitAction(a));
}

TEST_CASE("CollabSession connectedCount counts connected users", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser u1; u1.userId = "u1"; u1.connected = true;
    CollabUser u2; u2.userId = "u2"; u2.connected = false;
    s.addUser(u1);
    s.addUser(u2);
    REQUIRE(s.connectedCount() == 1);
}

TEST_CASE("CollabSession editorCount counts users who can edit", "[Editor][S74]") {
    CollabSession s("s");
    CollabUser owner; owner.userId = "o"; owner.role = CollabUserRole::Owner;
    CollabUser viewer; viewer.userId = "v"; viewer.role = CollabUserRole::Viewer;
    CollabUser editor; editor.userId = "e"; editor.role = CollabUserRole::Editor;
    s.addUser(owner);
    s.addUser(viewer);
    s.addUser(editor);
    REQUIRE(s.editorCount() == 2);
}

TEST_CASE("CollabSession kMaxUsers is 32", "[Editor][S74]") {
    REQUIRE(CollabSession::kMaxUsers == 32);
}

TEST_CASE("CollabConflictResolver resolve no_conflict for different paths", "[Editor][S74]") {
    CollabConflictResolver resolver;
    CollabEditAction a, b;
    a.actionId = "a1"; a.userId = "u1"; a.targetPath = "/scene/entity1"; a.type = CollabEditType::Modify;
    b.actionId = "b1"; b.userId = "u2"; b.targetPath = "/scene/entity2"; b.type = CollabEditType::Modify;
    auto res = resolver.resolve(a, b);
    REQUIRE(res.autoResolved);
    REQUIRE(res.strategy == "no_conflict");
    REQUIRE(resolver.totalResolutions() == 1);
    REQUIRE(resolver.autoResolved() == 1);
    REQUIRE(resolver.manualRequired() == 0);
}

TEST_CASE("CollabConflictResolver resolve last_writer_wins for same path and type", "[Editor][S74]") {
    CollabConflictResolver resolver;
    CollabEditAction a, b;
    a.actionId = "a1"; a.userId = "u1"; a.targetPath = "/x"; a.type = CollabEditType::Modify;
    b.actionId = "b1"; b.userId = "u2"; b.targetPath = "/x"; b.type = CollabEditType::Modify;
    auto res = resolver.resolve(a, b);
    REQUIRE(res.autoResolved);
    REQUIRE(res.strategy == "last_writer_wins");
}

TEST_CASE("CollabConflictResolver resolve manual for same path different types", "[Editor][S74]") {
    CollabConflictResolver resolver;
    CollabEditAction a, b;
    a.actionId = "a1"; a.userId = "u1"; a.targetPath = "/x"; a.type = CollabEditType::Modify;
    b.actionId = "b1"; b.userId = "u2"; b.targetPath = "/x"; b.type = CollabEditType::Delete;
    auto res = resolver.resolve(a, b);
    REQUIRE_FALSE(res.autoResolved);
    REQUIRE(res.strategy == "manual");
    REQUIRE(resolver.manualRequired() == 1);
}

TEST_CASE("CollabConflictResolver reset clears counts", "[Editor][S74]") {
    CollabConflictResolver resolver;
    CollabEditAction a, b;
    a.actionId = "a"; a.userId = "u"; a.targetPath = "/x"; a.type = CollabEditType::Modify;
    b.actionId = "b"; b.userId = "v"; b.targetPath = "/y"; b.type = CollabEditType::Modify;
    resolver.resolve(a, b);
    resolver.reset();
    REQUIRE(resolver.totalResolutions() == 0);
    REQUIRE(resolver.autoResolved() == 0);
    REQUIRE(resolver.manualRequired() == 0);
}

TEST_CASE("LiveCollaborationSystem starts uninitialized", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.sessionCount() == 0);
}

TEST_CASE("LiveCollaborationSystem init sets initialized", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    REQUIRE(sys.isInitialized());
}

TEST_CASE("LiveCollaborationSystem createSession before init returns -1", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    REQUIRE(sys.createSession("s") == -1);
}

TEST_CASE("LiveCollaborationSystem createSession returns index", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    int idx = sys.createSession("session1");
    REQUIRE(idx == 0);
    REQUIRE(sys.sessionCount() == 1);
}

TEST_CASE("LiveCollaborationSystem createSession rejects duplicate name", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    sys.createSession("s");
    REQUIRE(sys.createSession("s") == -1);
    REQUIRE(sys.sessionCount() == 1);
}

TEST_CASE("LiveCollaborationSystem session returns valid pointer", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    int idx = sys.createSession("game");
    REQUIRE(sys.session(idx) != nullptr);
    REQUIRE(sys.session(idx)->name() == "game");
}

TEST_CASE("LiveCollaborationSystem session returns nullptr for invalid index", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    REQUIRE(sys.session(-1) == nullptr);
    REQUIRE(sys.session(99) == nullptr);
}

TEST_CASE("LiveCollaborationSystem joinSession adds user", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    sys.createSession("s");
    CollabUser u; u.userId = "alice"; u.role = CollabUserRole::Editor;
    REQUIRE(sys.joinSession("s", u));
    REQUIRE(sys.session(0)->userCount() == 1);
}

TEST_CASE("LiveCollaborationSystem leaveSession removes user", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    sys.createSession("s");
    CollabUser u; u.userId = "bob"; u.role = CollabUserRole::Editor;
    sys.joinSession("s", u);
    REQUIRE(sys.leaveSession("s", "bob"));
    REQUIRE(sys.session(0)->userCount() == 0);
}

TEST_CASE("LiveCollaborationSystem tick increments tickCount", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    REQUIRE(sys.tickCount() == 0);
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 1);
}

TEST_CASE("LiveCollaborationSystem shutdown clears sessions", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    sys.createSession("s");
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.sessionCount() == 0);
}

TEST_CASE("LiveCollaborationSystem totalConnectedUsers sums across sessions", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    sys.createSession("s1");
    sys.createSession("s2");
    CollabUser u1; u1.userId = "u1"; u1.connected = true; u1.role = CollabUserRole::Editor;
    CollabUser u2; u2.userId = "u2"; u2.connected = true; u2.role = CollabUserRole::Editor;
    sys.joinSession("s1", u1);
    sys.joinSession("s2", u2);
    REQUIRE(sys.totalConnectedUsers() == 2);
}

TEST_CASE("LiveCollaborationSystem totalConflicts sums across sessions", "[Editor][S74]") {
    LiveCollaborationSystem sys;
    sys.init();
    REQUIRE(sys.totalConflicts() == 0);
}
