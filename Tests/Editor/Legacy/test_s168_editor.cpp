// S168 editor tests: WorkspaceCommandManagerV1, WorkspaceSelectionManagerV1, WorkspaceNotificationCenterV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WorkspaceCommandManagerV1.h"
#include "NF/Editor/WorkspaceSelectionManagerV1.h"
#include "NF/Editor/WorkspaceNotificationCenterV1.h"

using namespace NF;
using Catch::Approx;

// ── WorkspaceCommandManagerV1 ────────────────────────────────────────────────

TEST_CASE("Wcmv1Command validity", "[Editor][S168]") {
    Wcmv1Command c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "DeleteEntity";
    REQUIRE(c.isValid());
}

TEST_CASE("WorkspaceCommandManagerV1 registerCommand and commandCount", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    REQUIRE(wcm.commandCount() == 0);
    Wcmv1Command c; c.id = 1; c.name = "Cmd1";
    REQUIRE(wcm.registerCommand(c));
    REQUIRE(wcm.commandCount() == 1);
}

TEST_CASE("WorkspaceCommandManagerV1 registerCommand invalid fails", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    REQUIRE(!wcm.registerCommand(Wcmv1Command{}));
}

TEST_CASE("WorkspaceCommandManagerV1 registerCommand duplicate fails", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 1; c.name = "A";
    wcm.registerCommand(c);
    REQUIRE(!wcm.registerCommand(c));
}

TEST_CASE("WorkspaceCommandManagerV1 unregisterCommand", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 2; c.name = "B";
    wcm.registerCommand(c);
    REQUIRE(wcm.unregisterCommand(2));
    REQUIRE(wcm.commandCount() == 0);
    REQUIRE(!wcm.unregisterCommand(2));
}

TEST_CASE("WorkspaceCommandManagerV1 execute sets Done", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 1; c.name = "A"; c.isUndoable = true;
    wcm.registerCommand(c);
    REQUIRE(wcm.execute(1));
    REQUIRE(wcm.findCommand(1)->isDone());
    REQUIRE(wcm.doneCount() == 1);
}

TEST_CASE("WorkspaceCommandManagerV1 execute unknown fails", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    REQUIRE(!wcm.execute(99));
}

TEST_CASE("WorkspaceCommandManagerV1 undo and canUndo", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 1; c.name = "A"; c.isUndoable = true;
    wcm.registerCommand(c);
    wcm.execute(1);
    REQUIRE(wcm.canUndo());
    REQUIRE(wcm.undo());
    REQUIRE(wcm.findCommand(1)->isUndone());
    REQUIRE(!wcm.canUndo());
}

TEST_CASE("WorkspaceCommandManagerV1 redo and canRedo", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 1; c.name = "A"; c.isUndoable = true;
    wcm.registerCommand(c);
    wcm.execute(1);
    wcm.undo();
    REQUIRE(wcm.canRedo());
    REQUIRE(wcm.redo());
    REQUIRE(!wcm.canRedo());
    REQUIRE(wcm.canUndo());
}

TEST_CASE("WorkspaceCommandManagerV1 undo empty stack returns false", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    REQUIRE(!wcm.undo());
}

TEST_CASE("WorkspaceCommandManagerV1 redo empty stack returns false", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    REQUIRE(!wcm.redo());
}

TEST_CASE("WorkspaceCommandManagerV1 non-undoable command skips undo stack", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 1; c.name = "A"; c.isUndoable = false;
    wcm.registerCommand(c);
    wcm.execute(1);
    REQUIRE(!wcm.canUndo());
    REQUIRE(wcm.undoDepth() == 0);
}

TEST_CASE("WorkspaceCommandManagerV1 setState and failedCount", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c; c.id = 1; c.name = "A";
    wcm.registerCommand(c);
    REQUIRE(wcm.setState(1, Wcmv1CommandState::Failed));
    REQUIRE(wcm.failedCount() == 1);
}

TEST_CASE("WorkspaceCommandManagerV1 countByScope", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c1; c1.id = 1; c1.name = "A"; c1.scope = Wcmv1CommandScope::Global;
    Wcmv1Command c2; c2.id = 2; c2.name = "B"; c2.scope = Wcmv1CommandScope::Tool;
    wcm.registerCommand(c1); wcm.registerCommand(c2);
    REQUIRE(wcm.countByScope(Wcmv1CommandScope::Global) == 1);
    REQUIRE(wcm.countByScope(Wcmv1CommandScope::Tool)   == 1);
}

TEST_CASE("wcmv1CommandStateName covers all values", "[Editor][S168]") {
    REQUIRE(std::string(wcmv1CommandStateName(Wcmv1CommandState::Idle))      == "Idle");
    REQUIRE(std::string(wcmv1CommandStateName(Wcmv1CommandState::Redone))    == "Redone");
}

TEST_CASE("WorkspaceCommandManagerV1 onChange callback", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    uint64_t notified = 0;
    wcm.setOnChange([&](uint64_t id) { notified = id; });
    Wcmv1Command c; c.id = 7; c.name = "G"; c.isUndoable = true;
    wcm.registerCommand(c);
    wcm.execute(7);
    REQUIRE(notified == 7);
}

TEST_CASE("WorkspaceCommandManagerV1 redo clears redo after new execute", "[Editor][S168]") {
    WorkspaceCommandManagerV1 wcm;
    Wcmv1Command c1; c1.id = 1; c1.name = "A"; c1.isUndoable = true;
    Wcmv1Command c2; c2.id = 2; c2.name = "B"; c2.isUndoable = true;
    wcm.registerCommand(c1); wcm.registerCommand(c2);
    wcm.execute(1);
    wcm.undo();
    REQUIRE(wcm.canRedo());
    wcm.execute(2);
    REQUIRE(!wcm.canRedo());
}

// ── WorkspaceSelectionManagerV1 ──────────────────────────────────────────────

TEST_CASE("Wsmv1SelectionItem validity", "[Editor][S168]") {
    Wsmv1SelectionItem s;
    REQUIRE(!s.isValid());
    s.id = 1;
    REQUIRE(s.isValid());
}

TEST_CASE("WorkspaceSelectionManagerV1 select and selectionCount", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    REQUIRE(wsm.selectionCount() == 0);
    Wsmv1SelectionItem s; s.id = 1;
    REQUIRE(wsm.select(s));
    REQUIRE(wsm.selectionCount() == 1);
    REQUIRE(wsm.hasSelection());
}

TEST_CASE("WorkspaceSelectionManagerV1 select invalid fails", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    REQUIRE(!wsm.select(Wsmv1SelectionItem{}));
}

TEST_CASE("WorkspaceSelectionManagerV1 select duplicate fails", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s; s.id = 1;
    wsm.select(s);
    REQUIRE(!wsm.select(s));
}

TEST_CASE("WorkspaceSelectionManagerV1 deselect", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s; s.id = 2;
    wsm.select(s);
    REQUIRE(wsm.deselect(2));
    REQUIRE(wsm.selectionCount() == 0);
    REQUIRE(!wsm.hasSelection());
    REQUIRE(!wsm.deselect(2));
}

TEST_CASE("WorkspaceSelectionManagerV1 clearSelection", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s1; s1.id = 1;
    Wsmv1SelectionItem s2; s2.id = 2;
    wsm.select(s1); wsm.select(s2);
    wsm.clearSelection();
    REQUIRE(wsm.selectionCount() == 0);
}

TEST_CASE("WorkspaceSelectionManagerV1 setFlag lockedCount", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s; s.id = 1;
    wsm.select(s);
    REQUIRE(wsm.setFlag(1, Wsmv1SelectionFlag::Locked));
    REQUIRE(wsm.lockedCount() == 1);
    REQUIRE(wsm.findItem(1)->isLocked());
}

TEST_CASE("WorkspaceSelectionManagerV1 setFlag pinnedCount", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s; s.id = 1;
    wsm.select(s);
    wsm.setFlag(1, Wsmv1SelectionFlag::Pinned);
    REQUIRE(wsm.pinnedCount() == 1);
    REQUIRE(wsm.findItem(1)->isPinned());
}

TEST_CASE("WorkspaceSelectionManagerV1 setFlag highlightedCount", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s; s.id = 1;
    wsm.select(s);
    wsm.setFlag(1, Wsmv1SelectionFlag::Highlighted);
    REQUIRE(wsm.highlightedCount() == 1);
    REQUIRE(wsm.findItem(1)->isHighlighted());
}

TEST_CASE("WorkspaceSelectionManagerV1 countByType", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    Wsmv1SelectionItem s1; s1.id = 1; s1.objectType = Wsmv1ObjectType::Entity;
    Wsmv1SelectionItem s2; s2.id = 2; s2.objectType = Wsmv1ObjectType::Asset;
    wsm.select(s1); wsm.select(s2);
    REQUIRE(wsm.countByType(Wsmv1ObjectType::Entity) == 1);
    REQUIRE(wsm.countByType(Wsmv1ObjectType::Asset)  == 1);
}

TEST_CASE("WorkspaceSelectionManagerV1 setMode and mode", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    REQUIRE(wsm.mode() == Wsmv1SelectionMode::Single);
    wsm.setMode(Wsmv1SelectionMode::Multi);
    REQUIRE(wsm.mode() == Wsmv1SelectionMode::Multi);
}

TEST_CASE("wsmv1SelectionModeName covers all values", "[Editor][S168]") {
    REQUIRE(std::string(wsmv1SelectionModeName(Wsmv1SelectionMode::Single)) == "Single");
    REQUIRE(std::string(wsmv1SelectionModeName(Wsmv1SelectionMode::Invert)) == "Invert");
}

TEST_CASE("WorkspaceSelectionManagerV1 onChange callback", "[Editor][S168]") {
    WorkspaceSelectionManagerV1 wsm;
    int callCount = 0;
    wsm.setOnChange([&]() { ++callCount; });
    Wsmv1SelectionItem s; s.id = 3;
    wsm.select(s);
    REQUIRE(callCount == 1);
    wsm.deselect(3);
    REQUIRE(callCount == 2);
}

// ── WorkspaceNotificationCenterV1 ────────────────────────────────────────────

TEST_CASE("Wncv1Notification validity", "[Editor][S168]") {
    Wncv1Notification n;
    REQUIRE(!n.isValid());
    n.id = 1; n.title = "Build Complete";
    REQUIRE(n.isValid());
}

TEST_CASE("WorkspaceNotificationCenterV1 post and notifCount", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    REQUIRE(wnc.notifCount() == 0);
    Wncv1Notification n; n.id = 1; n.title = "T1";
    REQUIRE(wnc.post(n));
    REQUIRE(wnc.notifCount() == 1);
}

TEST_CASE("WorkspaceNotificationCenterV1 post invalid fails", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    REQUIRE(!wnc.post(Wncv1Notification{}));
}

TEST_CASE("WorkspaceNotificationCenterV1 post duplicate fails", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n; n.id = 1; n.title = "A";
    wnc.post(n);
    REQUIRE(!wnc.post(n));
}

TEST_CASE("WorkspaceNotificationCenterV1 newCount after post", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n1; n1.id = 1; n1.title = "A";
    Wncv1Notification n2; n2.id = 2; n2.title = "B";
    wnc.post(n1); wnc.post(n2);
    REQUIRE(wnc.newCount() == 2);
}

TEST_CASE("WorkspaceNotificationCenterV1 markRead", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n; n.id = 1; n.title = "A";
    wnc.post(n);
    REQUIRE(wnc.markRead(1));
    REQUIRE(wnc.findNotif(1)->state == Wncv1NotifState::Read);
    REQUIRE(wnc.newCount() == 0);
}

TEST_CASE("WorkspaceNotificationCenterV1 dismiss", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n; n.id = 1; n.title = "A";
    wnc.post(n);
    REQUIRE(wnc.dismiss(1));
    REQUIRE(wnc.findNotif(1)->state == Wncv1NotifState::Dismissed);
}

TEST_CASE("WorkspaceNotificationCenterV1 pin and pinnedCount", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n; n.id = 1; n.title = "A";
    wnc.post(n);
    REQUIRE(wnc.pin(1));
    REQUIRE(wnc.pinnedCount() == 1);
    REQUIRE(wnc.findNotif(1)->isPinned());
}

TEST_CASE("WorkspaceNotificationCenterV1 archive", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n; n.id = 1; n.title = "A";
    wnc.post(n);
    REQUIRE(wnc.archive(1));
    REQUIRE(wnc.findNotif(1)->isArchived());
}

TEST_CASE("WorkspaceNotificationCenterV1 remove", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n; n.id = 1; n.title = "A";
    wnc.post(n);
    REQUIRE(wnc.remove(1));
    REQUIRE(wnc.notifCount() == 0);
    REQUIRE(!wnc.remove(1));
}

TEST_CASE("WorkspaceNotificationCenterV1 clearDismissed", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n1; n1.id = 1; n1.title = "A";
    Wncv1Notification n2; n2.id = 2; n2.title = "B";
    wnc.post(n1); wnc.post(n2);
    wnc.dismiss(1);
    wnc.clearDismissed();
    REQUIRE(wnc.notifCount() == 1);
    REQUIRE(wnc.findNotif(2) != nullptr);
}

TEST_CASE("WorkspaceNotificationCenterV1 criticalCount", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n1; n1.id = 1; n1.title = "A"; n1.severity = Wncv1Severity::Critical;
    Wncv1Notification n2; n2.id = 2; n2.title = "B"; n2.severity = Wncv1Severity::Info;
    wnc.post(n1); wnc.post(n2);
    REQUIRE(wnc.criticalCount() == 1);
    REQUIRE(wnc.findNotif(1)->isCritical());
}

TEST_CASE("WorkspaceNotificationCenterV1 countBySeverity", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n1; n1.id = 1; n1.title = "A"; n1.severity = Wncv1Severity::Warning;
    Wncv1Notification n2; n2.id = 2; n2.title = "B"; n2.severity = Wncv1Severity::Error;
    wnc.post(n1); wnc.post(n2);
    REQUIRE(wnc.countBySeverity(Wncv1Severity::Warning) == 1);
    REQUIRE(wnc.countBySeverity(Wncv1Severity::Error)   == 1);
}

TEST_CASE("WorkspaceNotificationCenterV1 countByScope", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    Wncv1Notification n1; n1.id = 1; n1.title = "A"; n1.scope = Wncv1NotifScope::Build;
    Wncv1Notification n2; n2.id = 2; n2.title = "B"; n2.scope = Wncv1NotifScope::AtlasAI;
    wnc.post(n1); wnc.post(n2);
    REQUIRE(wnc.countByScope(Wncv1NotifScope::Build)   == 1);
    REQUIRE(wnc.countByScope(Wncv1NotifScope::AtlasAI) == 1);
}

TEST_CASE("wncv1SeverityName covers all values", "[Editor][S168]") {
    REQUIRE(std::string(wncv1SeverityName(Wncv1Severity::Info))     == "Info");
    REQUIRE(std::string(wncv1SeverityName(Wncv1Severity::Critical)) == "Critical");
}

TEST_CASE("wncv1NotifStateName covers all values", "[Editor][S168]") {
    REQUIRE(std::string(wncv1NotifStateName(Wncv1NotifState::New))      == "New");
    REQUIRE(std::string(wncv1NotifStateName(Wncv1NotifState::Archived)) == "Archived");
}

TEST_CASE("WorkspaceNotificationCenterV1 onPost callback", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    uint64_t posted = 0;
    wnc.setOnPost([&](uint64_t id) { posted = id; });
    Wncv1Notification n; n.id = 42; n.title = "Alert";
    wnc.post(n);
    REQUIRE(posted == 42);
}

TEST_CASE("WorkspaceNotificationCenterV1 onChange callback", "[Editor][S168]") {
    WorkspaceNotificationCenterV1 wnc;
    uint64_t changed = 0;
    wnc.setOnChange([&](uint64_t id) { changed = id; });
    Wncv1Notification n; n.id = 10; n.title = "X";
    wnc.post(n);
    wnc.pin(10);
    REQUIRE(changed == 10);
}
