// S170 editor tests: DataBindingSystemV1, LivePreviewControllerV1, HotReloadManagerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/DataBindingSystemV1.h"
#include "NF/Editor/LivePreviewControllerV1.h"
#include "NF/Editor/HotReloadManagerV1.h"

using namespace NF;
using Catch::Approx;

// ── DataBindingSystemV1 ──────────────────────────────────────────────────────

TEST_CASE("Dbsv1Binding validity", "[Editor][S170]") {
    Dbsv1Binding b;
    REQUIRE(!b.isValid());
    b.id = 1; b.sourceId = "EntityA"; b.targetId = "InspectorPanel";
    REQUIRE(b.isValid());
}

TEST_CASE("DataBindingSystemV1 addBinding and bindingCount", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    REQUIRE(dbs.bindingCount() == 0);
    Dbsv1Binding b; b.id = 1; b.sourceId = "S"; b.targetId = "T";
    REQUIRE(dbs.addBinding(b));
    REQUIRE(dbs.bindingCount() == 1);
}

TEST_CASE("DataBindingSystemV1 addBinding invalid fails", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    REQUIRE(!dbs.addBinding(Dbsv1Binding{}));
}

TEST_CASE("DataBindingSystemV1 addBinding duplicate fails", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b; b.id = 1; b.sourceId = "A"; b.targetId = "B";
    dbs.addBinding(b);
    REQUIRE(!dbs.addBinding(b));
}

TEST_CASE("DataBindingSystemV1 removeBinding", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b; b.id = 2; b.sourceId = "A"; b.targetId = "B";
    dbs.addBinding(b);
    REQUIRE(dbs.removeBinding(2));
    REQUIRE(dbs.bindingCount() == 0);
    REQUIRE(!dbs.removeBinding(2));
}

TEST_CASE("DataBindingSystemV1 bind and boundCount", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b; b.id = 1; b.sourceId = "A"; b.targetId = "B";
    dbs.addBinding(b);
    REQUIRE(dbs.bind(1));
    REQUIRE(dbs.boundCount() == 1);
    REQUIRE(dbs.findBinding(1)->isBound());
}

TEST_CASE("DataBindingSystemV1 unbind", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b; b.id = 1; b.sourceId = "A"; b.targetId = "B";
    dbs.addBinding(b);
    dbs.bind(1);
    REQUIRE(dbs.unbind(1));
    REQUIRE(dbs.boundCount() == 0);
}

TEST_CASE("DataBindingSystemV1 setState errorCount", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b; b.id = 1; b.sourceId = "A"; b.targetId = "B";
    dbs.addBinding(b);
    dbs.setState(1, Dbsv1BindingState::Error);
    REQUIRE(dbs.errorCount() == 1);
    REQUIRE(dbs.findBinding(1)->hasError());
}

TEST_CASE("DataBindingSystemV1 staleCount", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b; b.id = 1; b.sourceId = "A"; b.targetId = "B";
    dbs.addBinding(b);
    dbs.setState(1, Dbsv1BindingState::Stale);
    REQUIRE(dbs.staleCount() == 1);
    REQUIRE(dbs.findBinding(1)->isStale());
}

TEST_CASE("DataBindingSystemV1 setEnabled enabledCount", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b1; b1.id = 1; b1.sourceId = "A"; b1.targetId = "B"; b1.isEnabled = true;
    Dbsv1Binding b2; b2.id = 2; b2.sourceId = "C"; b2.targetId = "D"; b2.isEnabled = false;
    dbs.addBinding(b1); dbs.addBinding(b2);
    REQUIRE(dbs.enabledCount() == 1);
    dbs.setEnabled(2, true);
    REQUIRE(dbs.enabledCount() == 2);
}

TEST_CASE("DataBindingSystemV1 countByMode", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b1; b1.id = 1; b1.sourceId = "A"; b1.targetId = "B"; b1.mode = Dbsv1BindingMode::TwoWay;
    Dbsv1Binding b2; b2.id = 2; b2.sourceId = "C"; b2.targetId = "D"; b2.mode = Dbsv1BindingMode::OneTime;
    dbs.addBinding(b1); dbs.addBinding(b2);
    REQUIRE(dbs.countByMode(Dbsv1BindingMode::TwoWay)  == 1);
    REQUIRE(dbs.countByMode(Dbsv1BindingMode::OneTime) == 1);
}

TEST_CASE("DataBindingSystemV1 countByType", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    Dbsv1Binding b1; b1.id = 1; b1.sourceId = "A"; b1.targetId = "B"; b1.valueType = Dbsv1ValueType::Float;
    Dbsv1Binding b2; b2.id = 2; b2.sourceId = "C"; b2.targetId = "D"; b2.valueType = Dbsv1ValueType::Color;
    dbs.addBinding(b1); dbs.addBinding(b2);
    REQUIRE(dbs.countByType(Dbsv1ValueType::Float) == 1);
    REQUIRE(dbs.countByType(Dbsv1ValueType::Color) == 1);
}

TEST_CASE("dbsv1BindingModeName covers all values", "[Editor][S170]") {
    REQUIRE(std::string(dbsv1BindingModeName(Dbsv1BindingMode::OneWay))         == "OneWay");
    REQUIRE(std::string(dbsv1BindingModeName(Dbsv1BindingMode::OneWayToSource)) == "OneWayToSource");
}

TEST_CASE("dbsv1ValueTypeName covers all values", "[Editor][S170]") {
    REQUIRE(std::string(dbsv1ValueTypeName(Dbsv1ValueType::Bool))   == "Bool");
    REQUIRE(std::string(dbsv1ValueTypeName(Dbsv1ValueType::Object)) == "Object");
}

TEST_CASE("DataBindingSystemV1 onChange callback", "[Editor][S170]") {
    DataBindingSystemV1 dbs;
    uint64_t notified = 0;
    dbs.setOnChange([&](uint64_t id) { notified = id; });
    Dbsv1Binding b; b.id = 9; b.sourceId = "X"; b.targetId = "Y";
    dbs.addBinding(b);
    dbs.bind(9);
    REQUIRE(notified == 9);
}

// ── LivePreviewControllerV1 ──────────────────────────────────────────────────

TEST_CASE("Lpcv1PreviewSession validity", "[Editor][S170]") {
    Lpcv1PreviewSession s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "MatPreview"; s.width = 800; s.height = 600; s.fps = 30.f;
    REQUIRE(s.isValid());
}

TEST_CASE("Lpcv1PreviewSession zero width invalid", "[Editor][S170]") {
    Lpcv1PreviewSession s; s.id = 1; s.name = "X"; s.width = 0; s.height = 600; s.fps = 30.f;
    REQUIRE(!s.isValid());
}

TEST_CASE("LivePreviewControllerV1 addSession and sessionCount", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    REQUIRE(lpc.sessionCount() == 0);
    Lpcv1PreviewSession s; s.id = 1; s.name = "S1"; s.width = 1280; s.height = 720; s.fps = 60.f;
    REQUIRE(lpc.addSession(s));
    REQUIRE(lpc.sessionCount() == 1);
}

TEST_CASE("LivePreviewControllerV1 addSession invalid fails", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    REQUIRE(!lpc.addSession(Lpcv1PreviewSession{}));
}

TEST_CASE("LivePreviewControllerV1 addSession duplicate fails", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    REQUIRE(!lpc.addSession(s));
}

TEST_CASE("LivePreviewControllerV1 removeSession", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 2; s.name = "B"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    REQUIRE(lpc.removeSession(2));
    REQUIRE(lpc.sessionCount() == 0);
    REQUIRE(!lpc.removeSession(2));
}

TEST_CASE("LivePreviewControllerV1 startSession runningCount", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    REQUIRE(lpc.startSession(1));
    REQUIRE(lpc.runningCount() == 1);
    REQUIRE(lpc.activeId() == 1);
    REQUIRE(lpc.findSession(1)->isRunning());
}

TEST_CASE("LivePreviewControllerV1 pauseSession pausedCount", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    lpc.startSession(1);
    REQUIRE(lpc.pauseSession(1));
    REQUIRE(lpc.pausedCount() == 1);
    REQUIRE(lpc.findSession(1)->isPaused());
}

TEST_CASE("LivePreviewControllerV1 pauseSession when not running fails", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    REQUIRE(!lpc.pauseSession(1)); // still Stopped
}

TEST_CASE("LivePreviewControllerV1 stopSession", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    lpc.startSession(1);
    REQUIRE(lpc.stopSession(1));
    REQUIRE(lpc.runningCount() == 0);
    REQUIRE(lpc.activeId() == 0);
}

TEST_CASE("LivePreviewControllerV1 setState errorCount", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    lpc.setState(1, Lpcv1SessionState::Error);
    REQUIRE(lpc.errorCount() == 1);
    REQUIRE(lpc.findSession(1)->hasError());
}

TEST_CASE("LivePreviewControllerV1 incrementFrames", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    REQUIRE(lpc.incrementFrames(1, 10));
    REQUIRE(lpc.findSession(1)->framesCaptured == 10);
}

TEST_CASE("LivePreviewControllerV1 setCaptureFormat", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s; s.id = 1; s.name = "A"; s.width = 640; s.height = 480; s.fps = 30.f;
    lpc.addSession(s);
    REQUIRE(lpc.setCaptureFormat(1, Lpcv1CaptureFormat::EXR));
    REQUIRE(lpc.findSession(1)->captureFormat == Lpcv1CaptureFormat::EXR);
}

TEST_CASE("LivePreviewControllerV1 countByMode", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    Lpcv1PreviewSession s1; s1.id = 1; s1.name = "A"; s1.width = 64; s1.height = 64; s1.fps = 30.f; s1.mode = Lpcv1PreviewMode::Thumbnail;
    Lpcv1PreviewSession s2; s2.id = 2; s2.name = "B"; s2.width = 64; s2.height = 64; s2.fps = 30.f; s2.mode = Lpcv1PreviewMode::Streaming;
    lpc.addSession(s1); lpc.addSession(s2);
    REQUIRE(lpc.countByMode(Lpcv1PreviewMode::Thumbnail) == 1);
    REQUIRE(lpc.countByMode(Lpcv1PreviewMode::Streaming) == 1);
}

TEST_CASE("lpcv1PreviewModeName covers all values", "[Editor][S170]") {
    REQUIRE(std::string(lpcv1PreviewModeName(Lpcv1PreviewMode::Realtime))  == "Realtime");
    REQUIRE(std::string(lpcv1PreviewModeName(Lpcv1PreviewMode::Streaming)) == "Streaming");
}

TEST_CASE("LivePreviewControllerV1 onChange callback", "[Editor][S170]") {
    LivePreviewControllerV1 lpc;
    uint64_t notified = 0;
    lpc.setOnChange([&](uint64_t id) { notified = id; });
    Lpcv1PreviewSession s; s.id = 4; s.name = "D"; s.width = 64; s.height = 64; s.fps = 30.f;
    lpc.addSession(s);
    lpc.startSession(4);
    REQUIRE(notified == 4);
}

// ── HotReloadManagerV1 ───────────────────────────────────────────────────────

TEST_CASE("Hrmv1WatchEntry validity", "[Editor][S170]") {
    Hrmv1WatchEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "ScriptWatcher"; e.watchPath = "Scripts/";
    REQUIRE(e.isValid());
}

TEST_CASE("HotReloadManagerV1 addEntry and entryCount", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    REQUIRE(hrm.entryCount() == 0);
    Hrmv1WatchEntry e; e.id = 1; e.name = "W1"; e.watchPath = "A/";
    REQUIRE(hrm.addEntry(e));
    REQUIRE(hrm.entryCount() == 1);
}

TEST_CASE("HotReloadManagerV1 addEntry invalid fails", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    REQUIRE(!hrm.addEntry(Hrmv1WatchEntry{}));
}

TEST_CASE("HotReloadManagerV1 addEntry duplicate fails", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e; e.id = 1; e.name = "A"; e.watchPath = "A/";
    hrm.addEntry(e);
    REQUIRE(!hrm.addEntry(e));
}

TEST_CASE("HotReloadManagerV1 removeEntry", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e; e.id = 2; e.name = "B"; e.watchPath = "B/";
    hrm.addEntry(e);
    REQUIRE(hrm.removeEntry(2));
    REQUIRE(hrm.entryCount() == 0);
    REQUIRE(!hrm.removeEntry(2));
}

TEST_CASE("HotReloadManagerV1 setState watchingCount", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e; e.id = 1; e.name = "A"; e.watchPath = "A/";
    hrm.addEntry(e);
    REQUIRE(hrm.setState(1, Hrmv1WatchState::Watching));
    REQUIRE(hrm.watchingCount() == 1);
    REQUIRE(hrm.findEntry(1)->isWatching());
}

TEST_CASE("HotReloadManagerV1 triggerReload reloadingCount", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e; e.id = 1; e.name = "A"; e.watchPath = "A/";
    hrm.addEntry(e);
    REQUIRE(hrm.triggerReload(1));
    REQUIRE(hrm.reloadingCount() == 1);
    REQUIRE(hrm.findEntry(1)->reloadCount == 1);
    REQUIRE(hrm.findEntry(1)->isReloading());
}

TEST_CASE("HotReloadManagerV1 triggerReload disabled entry fails", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e; e.id = 1; e.name = "A"; e.watchPath = "A/"; e.isEnabled = false;
    hrm.addEntry(e);
    REQUIRE(!hrm.triggerReload(1));
}

TEST_CASE("HotReloadManagerV1 recordFailure failedCount", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e; e.id = 1; e.name = "A"; e.watchPath = "A/";
    hrm.addEntry(e);
    REQUIRE(hrm.recordFailure(1));
    REQUIRE(hrm.failedCount() == 1);
    REQUIRE(hrm.findEntry(1)->failCount == 1);
    REQUIRE(hrm.findEntry(1)->hasFailed());
}

TEST_CASE("HotReloadManagerV1 setEnabled enabledCount", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e1; e1.id = 1; e1.name = "A"; e1.watchPath = "A/";
    Hrmv1WatchEntry e2; e2.id = 2; e2.name = "B"; e2.watchPath = "B/"; e2.isEnabled = false;
    hrm.addEntry(e1); hrm.addEntry(e2);
    REQUIRE(hrm.enabledCount() == 1);
    hrm.setEnabled(2, true);
    REQUIRE(hrm.enabledCount() == 2);
}

TEST_CASE("HotReloadManagerV1 countByScope", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e1; e1.id = 1; e1.name = "A"; e1.watchPath = "A/"; e1.scope = Hrmv1ReloadScope::Shader;
    Hrmv1WatchEntry e2; e2.id = 2; e2.name = "B"; e2.watchPath = "B/"; e2.scope = Hrmv1ReloadScope::Plugin;
    hrm.addEntry(e1); hrm.addEntry(e2);
    REQUIRE(hrm.countByScope(Hrmv1ReloadScope::Shader) == 1);
    REQUIRE(hrm.countByScope(Hrmv1ReloadScope::Plugin) == 1);
}

TEST_CASE("HotReloadManagerV1 countByTrigger", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    Hrmv1WatchEntry e1; e1.id = 1; e1.name = "A"; e1.watchPath = "A/"; e1.triggerMode = Hrmv1TriggerMode::OnSave;
    Hrmv1WatchEntry e2; e2.id = 2; e2.name = "B"; e2.watchPath = "B/"; e2.triggerMode = Hrmv1TriggerMode::Manual;
    hrm.addEntry(e1); hrm.addEntry(e2);
    REQUIRE(hrm.countByTrigger(Hrmv1TriggerMode::OnSave) == 1);
    REQUIRE(hrm.countByTrigger(Hrmv1TriggerMode::Manual) == 1);
}

TEST_CASE("hrmv1ReloadScopeName covers all values", "[Editor][S170]") {
    REQUIRE(std::string(hrmv1ReloadScopeName(Hrmv1ReloadScope::Script)) == "Script");
    REQUIRE(std::string(hrmv1ReloadScopeName(Hrmv1ReloadScope::All))    == "All");
}

TEST_CASE("HotReloadManagerV1 onChange callback", "[Editor][S170]") {
    HotReloadManagerV1 hrm;
    uint64_t notified = 0;
    hrm.setOnChange([&](uint64_t id) { notified = id; });
    Hrmv1WatchEntry e; e.id = 6; e.name = "F"; e.watchPath = "F/";
    hrm.addEntry(e);
    hrm.triggerReload(6);
    REQUIRE(notified == 6);
}
