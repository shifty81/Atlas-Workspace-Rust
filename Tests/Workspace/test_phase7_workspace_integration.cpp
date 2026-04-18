// Tests/Workspace/test_phase7_workspace_integration.cpp
// Phase 7 — Workspace Integration Surfaces
//
// Tests for:
//   1. FileIntakePipeline     — file type detection, ingest, handlers, batch
//   2. DropTargetHandler      — drag enter/over/leave/drop + pipeline binding
//   3. NotificationWorkflow   — WorkflowRule, RateLimiter, PriorityQueue, Engine
//   4. DockTreeSerializer     — DockTree, DockTreeNode, serialize/deserialize roundtrip
//   5. PanelStateSerializer   — PanelStateEntry, serialize/deserialize roundtrip
//   6. LayoutPersistence      — LayoutSerializer, LayoutPreset, LayoutPersistenceManager
//   7. ViewportHostContract   — ViewportHostRegistry slot lifecycle
//   8. TypographySystem       — TypographyRegistry defaults, enforcement

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Workspace/FileIntakePipeline.h"
#include "NF/Workspace/DropTargetHandler.h"
#include "NF/Workspace/NotificationWorkflow.h"
#include "NF/Workspace/DockTreeSerializer.h"
#include "NF/Workspace/PanelStateSerializer.h"
#include "NF/Workspace/LayoutPersistence.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/TypographySystem.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — FileIntakePipeline
// ═════════════════════════════════════════════════════════════════

TEST_CASE("intakeSourceName returns correct strings", "[Phase7][FileIntake]") {
    CHECK(std::string(intakeSourceName(IntakeSource::FileDrop))       == "FileDrop");
    CHECK(std::string(intakeSourceName(IntakeSource::FileDialog))     == "FileDialog");
    CHECK(std::string(intakeSourceName(IntakeSource::CLI))            == "CLI");
    CHECK(std::string(intakeSourceName(IntakeSource::URLScheme))      == "URLScheme");
    CHECK(std::string(intakeSourceName(IntakeSource::ClipboardPaste)) == "ClipboardPaste");
}

TEST_CASE("detectIntakeFileType maps extensions correctly", "[Phase7][FileIntake]") {
    CHECK(detectIntakeFileType(".png")  == IntakeFileType::Texture);
    CHECK(detectIntakeFileType(".jpg")  == IntakeFileType::Texture);
    CHECK(detectIntakeFileType(".exr")  == IntakeFileType::Texture);
    CHECK(detectIntakeFileType(".fbx")  == IntakeFileType::Mesh);
    CHECK(detectIntakeFileType(".gltf") == IntakeFileType::Mesh);
    CHECK(detectIntakeFileType(".wav")  == IntakeFileType::Audio);
    CHECK(detectIntakeFileType(".lua")  == IntakeFileType::Script);
    CHECK(detectIntakeFileType(".hlsl") == IntakeFileType::Shader);
    CHECK(detectIntakeFileType(".atlasscene")   == IntakeFileType::Scene);
    CHECK(detectIntakeFileType(".ttf")  == IntakeFileType::Font);
    CHECK(detectIntakeFileType(".mp4")  == IntakeFileType::Video);
    CHECK(detectIntakeFileType(".zip")  == IntakeFileType::Archive);
    CHECK(detectIntakeFileType(".atlasproject") == IntakeFileType::Project);
    CHECK(detectIntakeFileType(".unknown123")   == IntakeFileType::Unknown);
}

TEST_CASE("FileIntakePipeline default state", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    CHECK(pipe.pendingCount()  == 0);
    CHECK(pipe.ingestedCount() == 0);
    CHECK(pipe.rejectedCount() == 0);
    CHECK(pipe.handlerCount()  == 0);
}

TEST_CASE("FileIntakePipeline ingest adds to pending", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    REQUIRE(pipe.ingest("texture.png", IntakeSource::FileDrop));
    CHECK(pipe.pendingCount()  == 1);
    CHECK(pipe.ingestedCount() == 1);
    CHECK(pipe.rejectedCount() == 0);

    const auto* item = pipe.pendingItems().data();
    CHECK(item->fileType == IntakeFileType::Texture);
    CHECK(item->validated);
    CHECK_FALSE(item->rejected);
}

TEST_CASE("FileIntakePipeline handler can reject items", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    pipe.addHandler("no_video", [](IntakeItem& item) -> bool {
        if (item.fileType == IntakeFileType::Video) {
            item.rejectReason = "Video files are not allowed";
            return false;
        }
        return true;
    });

    CHECK_FALSE(pipe.ingest("clip.mp4", IntakeSource::FileDrop));
    CHECK(pipe.rejectedCount() == 1);
    CHECK(pipe.pendingCount()  == 0);

    // Non-video should pass
    CHECK(pipe.ingest("model.fbx", IntakeSource::FileDrop));
    CHECK(pipe.pendingCount()  == 1);
}

TEST_CASE("FileIntakePipeline addHandler rejects duplicates", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    CHECK(pipe.addHandler("h1", [](IntakeItem&) { return true; }));
    CHECK_FALSE(pipe.addHandler("h1", [](IntakeItem&) { return true; }));
    CHECK(pipe.handlerCount() == 1);
}

TEST_CASE("FileIntakePipeline ingestBatch returns accepted count", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    size_t accepted = pipe.ingestBatch({"a.png", "b.fbx", "c.mp4"}, IntakeSource::FileDialog);
    CHECK(accepted == 3);
    CHECK(pipe.pendingCount() == 3);
}

TEST_CASE("FileIntakePipeline findById returns correct item", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    pipe.ingest("tex.png", IntakeSource::FileDrop);
    const auto& items = pipe.pendingItems();
    REQUIRE_FALSE(items.empty());
    uint32_t id = items[0].id;
    const auto* found = pipe.findById(id);
    REQUIRE(found != nullptr);
    CHECK(found->path == "tex.png");
    CHECK(pipe.findById(99999) == nullptr);
}

TEST_CASE("FileIntakePipeline clearPending empties pending list", "[Phase7][FileIntake]") {
    FileIntakePipeline pipe;
    pipe.ingest("a.png", IntakeSource::FileDrop);
    pipe.ingest("b.png", IntakeSource::FileDrop);
    CHECK(pipe.pendingCount() == 2);
    pipe.clearPending();
    CHECK(pipe.pendingCount() == 0);
    CHECK(pipe.ingestedCount() == 2); // counter is not reset
}

TEST_CASE("IntakeItem filename extracts basename", "[Phase7][FileIntake]") {
    IntakeItem item;
    item.path = "/some/path/texture.png";
    CHECK(item.filename() == "texture.png");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — DropTargetHandler
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DropTargetHandler default state is Idle", "[Phase7][DropTarget]") {
    DropTargetHandler dth;
    CHECK(dth.state()       == DropState::Idle);
    CHECK(dth.dropCount()   == 0);
    CHECK_FALSE(dth.isDragActive());
    CHECK(dth.acceptUnknown() == false);
}

TEST_CASE("dropStateName and dropEffectName return correct strings", "[Phase7][DropTarget]") {
    CHECK(std::string(dropStateName(DropState::Idle))      == "Idle");
    CHECK(std::string(dropStateName(DropState::DragOver))  == "DragOver");
    CHECK(std::string(dropStateName(DropState::Dropped))   == "Dropped");
    CHECK(std::string(dropEffectName(DropEffect::Copy))    == "Copy");
    CHECK(std::string(dropEffectName(DropEffect::Move))    == "Move");
    CHECK(std::string(dropEffectName(DropEffect::Link))    == "Link");
    CHECK(std::string(dropEffectName(DropEffect::None))    == "None");
}

TEST_CASE("DropTargetHandler onDragEnter changes state", "[Phase7][DropTarget]") {
    DropTargetHandler dth;
    dth.setAcceptUnknown(true);
    auto eff = dth.onDragEnter({"texture.png"});
    CHECK(eff == DropEffect::Copy);
    CHECK(dth.state() == DropState::DragOver);
    CHECK(dth.isDragActive());
    CHECK(dth.enterCount() == 1);
}

TEST_CASE("DropTargetHandler rejects unknown files when acceptUnknown=false",
          "[Phase7][DropTarget]") {
    DropTargetHandler dth;
    auto eff = dth.onDragEnter({"file.unknown_ext"});
    CHECK(eff == DropEffect::None);
    CHECK(dth.state() == DropState::Rejected);
}

TEST_CASE("DropTargetHandler onDragLeave clears state", "[Phase7][DropTarget]") {
    DropTargetHandler dth;
    dth.setAcceptUnknown(true);
    dth.onDragEnter({"a.png"});
    dth.onDragLeave();
    CHECK(dth.state() == DropState::DragLeave);
    CHECK(dth.leaveCount() == 1);
    CHECK(dth.hoveredPaths().empty());
}

TEST_CASE("DropTargetHandler onDrop routes to pipeline", "[Phase7][DropTarget]") {
    FileIntakePipeline pipe;
    DropTargetHandler dth(&pipe);
    size_t accepted = dth.onDrop({"a.png", "b.fbx"});
    CHECK(accepted == 2);
    CHECK(dth.dropCount() == 1);
    CHECK(dth.totalDropped() == 2);
    CHECK(pipe.pendingCount() == 2);
    CHECK(dth.state() == DropState::Dropped);
}

TEST_CASE("DropTargetHandler onDrop with no pipeline returns 0",
          "[Phase7][DropTarget]") {
    DropTargetHandler dth;
    size_t accepted = dth.onDrop({"a.png"});
    CHECK(accepted == 0);
    CHECK(dth.dropCount() == 1);
}

TEST_CASE("DropTargetHandler reset returns to Idle", "[Phase7][DropTarget]") {
    DropTargetHandler dth;
    dth.setAcceptUnknown(true);
    dth.onDragEnter({"x.png"});
    dth.reset();
    CHECK(dth.state()     == DropState::Idle);
    CHECK(dth.isDragActive() == false);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — NotificationWorkflow
// ═════════════════════════════════════════════════════════════════

TEST_CASE("notificationActionName returns correct strings", "[Phase7][NotifWorkflow]") {
    CHECK(std::string(notificationActionName(NotificationAction::Show))        == "Show");
    CHECK(std::string(notificationActionName(NotificationAction::AutoDismiss)) == "AutoDismiss");
    CHECK(std::string(notificationActionName(NotificationAction::Pin))         == "Pin");
    CHECK(std::string(notificationActionName(NotificationAction::Escalate))    == "Escalate");
    CHECK(std::string(notificationActionName(NotificationAction::Suppress))    == "Suppress");
    CHECK(std::string(notificationActionName(NotificationAction::Log))         == "Log");
    CHECK(std::string(notificationActionName(NotificationAction::Sound))       == "Sound");
}

TEST_CASE("WorkflowRule matches on severity", "[Phase7][NotifWorkflow]") {
    WorkflowRule rule{"test", NotificationSeverity::Error, NotificationAction::Pin};
    CHECK(rule.matches(NotificationSeverity::Error));
    CHECK_FALSE(rule.matches(NotificationSeverity::Warning));
    rule.enabled = false;
    CHECK_FALSE(rule.matches(NotificationSeverity::Error));
}

TEST_CASE("NotificationRateLimiter allows up to max and then throttles",
          "[Phase7][NotifWorkflow]") {
    NotificationRateLimiter rl(3, 5.f);
    CHECK(rl.shouldAllow());
    CHECK(rl.shouldAllow());
    CHECK(rl.shouldAllow());
    CHECK_FALSE(rl.shouldAllow()); // throttled
    CHECK(rl.isThrottled());
    CHECK(rl.recentCount() == 3);
}

TEST_CASE("NotificationRateLimiter resets after window", "[Phase7][NotifWorkflow]") {
    NotificationRateLimiter rl(2, 1.f);
    rl.shouldAllow();
    rl.shouldAllow();
    CHECK(rl.isThrottled());
    rl.tick(1.1f); // past window
    CHECK_FALSE(rl.isThrottled());
    CHECK(rl.recentCount() == 0);
}

TEST_CASE("NotificationRateLimiter reset clears counters", "[Phase7][NotifWorkflow]") {
    NotificationRateLimiter rl(3, 5.f);
    rl.shouldAllow(); rl.shouldAllow(); rl.shouldAllow();
    CHECK(rl.isThrottled());
    rl.reset();
    CHECK_FALSE(rl.isThrottled());
    CHECK(rl.recentCount() == 0);
}

TEST_CASE("NotificationPriorityQueue orders by severity", "[Phase7][NotifWorkflow]") {
    NotificationPriorityQueue pq;
    Notification info, error;
    info.title    = "Info";
    info.severity = NotificationSeverity::Info;
    error.title   = "Error";
    error.severity = NotificationSeverity::Error;

    pq.enqueue(info);
    pq.enqueue(error);

    REQUIRE(pq.size() == 2);
    CHECK(pq.top().severity == NotificationSeverity::Error); // highest first
}

TEST_CASE("NotificationPriorityQueue dequeue removes top", "[Phase7][NotifWorkflow]") {
    NotificationPriorityQueue pq;
    Notification n;
    n.title = "X"; n.severity = NotificationSeverity::Warning;
    pq.enqueue(n);
    REQUIRE(pq.size() == 1);
    auto popped = pq.dequeue();
    CHECK(pq.empty());
    CHECK(popped.title == "X");
}

TEST_CASE("NotificationWorkflowEngine loadDefaults creates standard rules",
          "[Phase7][NotifWorkflow]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();
    CHECK(engine.ruleCount() >= 5);
    CHECK(engine.findRule("info_auto_dismiss") != nullptr);
    CHECK(engine.findRule("error_pin")         != nullptr);
    CHECK(engine.findRule("critical_pin")      != nullptr);
}

TEST_CASE("NotificationWorkflowEngine processNotification applies rule",
          "[Phase7][NotifWorkflow]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();

    Notification n;
    n.severity = NotificationSeverity::Error;
    auto action = engine.processNotification(n);
    CHECK(action == NotificationAction::Pin);
}

TEST_CASE("NotificationWorkflowEngine processes info as auto-dismiss",
          "[Phase7][NotifWorkflow]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();
    Notification n;
    n.severity = NotificationSeverity::Info;
    CHECK(engine.processNotification(n) == NotificationAction::AutoDismiss);
}

TEST_CASE("NotificationWorkflowEngine suppresses when rate limited",
          "[Phase7][NotifWorkflow]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();
    engine.rateLimiter().setMaxPerWindow(0);

    Notification n;
    n.severity = NotificationSeverity::Error;
    auto action = engine.processNotification(n);
    CHECK(action == NotificationAction::Suppress);
    CHECK(engine.suppressed() == 1);
}

TEST_CASE("NotificationWorkflowEngine addRule rejects duplicates",
          "[Phase7][NotifWorkflow]") {
    NotificationWorkflowEngine engine;
    WorkflowRule r{"my_rule", NotificationSeverity::Info, NotificationAction::Show};
    CHECK(engine.addRule(r));
    CHECK_FALSE(engine.addRule(r));
    CHECK(engine.ruleCount() == 1);
}

TEST_CASE("NotificationWorkflowEngine removeRule works", "[Phase7][NotifWorkflow]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();
    size_t before = engine.ruleCount();
    CHECK(engine.removeRule("info_auto_dismiss"));
    CHECK(engine.ruleCount() == before - 1);
    CHECK_FALSE(engine.removeRule("nonexistent"));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — DockTreeSerializer
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DockTree addNode and findNode", "[Phase7][DockTree]") {
    DockTree tree;
    DockTreeNode node;
    node.id   = 1;
    node.kind = DockNodeKind::TabStack;
    node.addPanel("inspector");
    node.addPanel("hierarchy");
    CHECK(tree.addNode(node));
    CHECK(tree.nodeCount() == 1);
    CHECK(tree.rootId() == 1);
    const auto* found = tree.findNode(1);
    REQUIRE(found != nullptr);
    CHECK(found->panelIds.size() == 2);
}

TEST_CASE("DockTree addNode rejects duplicates", "[Phase7][DockTree]") {
    DockTree tree;
    DockTreeNode n; n.id = 5;
    CHECK(tree.addNode(n));
    CHECK_FALSE(tree.addNode(n));
}

TEST_CASE("DockTree removeNode removes correctly", "[Phase7][DockTree]") {
    DockTree tree;
    DockTreeNode n; n.id = 3;
    tree.addNode(n);
    CHECK(tree.removeNode(3));
    CHECK(tree.nodeCount() == 0);
    CHECK_FALSE(tree.removeNode(99));
}

TEST_CASE("DockNodeKind names are correct", "[Phase7][DockTree]") {
    CHECK(std::string(dockNodeKindName(DockNodeKind::Split))     == "Split");
    CHECK(std::string(dockNodeKindName(DockNodeKind::TabStack))  == "TabStack");
    CHECK(std::string(dockSplitOrientationName(DockSplitOrientation::Horizontal)) == "Horizontal");
    CHECK(std::string(dockSplitOrientationName(DockSplitOrientation::Vertical))   == "Vertical");
}

TEST_CASE("DockTreeSerializer roundtrip for TabStack", "[Phase7][DockTree]") {
    DockTree original;
    DockTreeNode tabs;
    tabs.id = 1;
    tabs.kind = DockNodeKind::TabStack;
    tabs.activeTab = 1;
    tabs.addPanel("inspector");
    tabs.addPanel("hierarchy");
    original.addNode(tabs);

    std::string data = DockTreeSerializer::serialize(original);
    REQUIRE_FALSE(data.empty());

    DockTree restored;
    REQUIRE(DockTreeSerializer::deserialize(data, restored));
    CHECK(restored.rootId()    == 1);
    CHECK(restored.nodeCount() == 1);
    const auto* rn = restored.findNode(1);
    REQUIRE(rn != nullptr);
    CHECK(rn->kind == DockNodeKind::TabStack);
    CHECK(rn->panelIds.size() == 2);
    CHECK(rn->panelIds[0] == "inspector");
    CHECK(rn->panelIds[1] == "hierarchy");
    CHECK(rn->activeTab == 1);
}

TEST_CASE("DockTreeSerializer roundtrip for Split node", "[Phase7][DockTree]") {
    DockTree original;
    DockTreeNode split;
    split.id          = 10;
    split.kind        = DockNodeKind::Split;
    split.orientation = DockSplitOrientation::Vertical;
    split.splitRatio  = 0.3f;
    split.firstChild  = 11;
    split.secondChild = 12;
    original.addNode(split);

    std::string data = DockTreeSerializer::serialize(original);
    DockTree restored;
    REQUIRE(DockTreeSerializer::deserialize(data, restored));
    const auto* rn = restored.findNode(10);
    REQUIRE(rn != nullptr);
    CHECK(rn->kind        == DockNodeKind::Split);
    CHECK(rn->orientation == DockSplitOrientation::Vertical);
    CHECK(rn->firstChild  == 11);
    CHECK(rn->secondChild == 12);
}

TEST_CASE("DockTreeSerializer deserialize fails on empty", "[Phase7][DockTree]") {
    DockTree out;
    CHECK_FALSE(DockTreeSerializer::deserialize("", out));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — PanelStateSerializer
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PanelStateEntry set and get", "[Phase7][PanelState]") {
    PanelStateEntry e;
    e.panelId   = "inspector";
    e.panelType = "ComponentInspector";
    e.set("scrollY", 42.5f);
    e.set("pinned", true);
    e.set("width", 320);

    CHECK(e.getFloat("scrollY", 0.f) == Catch::Approx(42.5f));
    CHECK(e.getBool("pinned", false) == true);
    CHECK(e.getInt("width", 0)       == 320);
    CHECK(e.get("missing", "def")    == "def");
    CHECK(e.isValid());
}

TEST_CASE("PanelStateSerializer roundtrip", "[Phase7][PanelState]") {
    std::vector<PanelStateEntry> entries;

    PanelStateEntry e1;
    e1.panelId   = "inspector";
    e1.panelType = "ComponentInspector";
    e1.set("scrollY", 100.f);
    e1.set("visible", true);
    entries.push_back(e1);

    PanelStateEntry e2;
    e2.panelId   = "console";
    e2.panelType = "ConsolePanel";
    e2.set("filterLevel", 2);
    entries.push_back(e2);

    std::string data = PanelStateSerializer::serialize(entries);
    REQUIRE_FALSE(data.empty());

    std::vector<PanelStateEntry> restored;
    REQUIRE(PanelStateSerializer::deserialize(data, restored));
    REQUIRE(restored.size() == 2);

    CHECK(restored[0].panelId   == "inspector");
    CHECK(restored[0].panelType == "ComponentInspector");
    CHECK(restored[1].panelId   == "console");
}

TEST_CASE("PanelStateSerializer skips invalid entries", "[Phase7][PanelState]") {
    std::vector<PanelStateEntry> entries;
    PanelStateEntry invalid; // no panelId
    entries.push_back(invalid);

    std::string data = PanelStateSerializer::serialize(entries);
    // Empty is fine — no valid entries
    std::vector<PanelStateEntry> restored;
    PanelStateSerializer::deserialize(data, restored);
    CHECK(restored.empty());
}

TEST_CASE("PanelStateSerializer deserialize empty string fails gracefully",
          "[Phase7][PanelState]") {
    std::vector<PanelStateEntry> out;
    CHECK_FALSE(PanelStateSerializer::deserialize("", out));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — LayoutPersistence
// ═════════════════════════════════════════════════════════════════

TEST_CASE("LayoutPreset validity", "[Phase7][LayoutPersistence]") {
    LayoutPreset p;
    CHECK_FALSE(p.isValid());
    p.name = "default";
    CHECK_FALSE(p.isValid()); // still no data
    p.serializedData = "layout:default\n";
    CHECK(p.isValid());
}

TEST_CASE("LayoutPreset markModified and clearModified", "[Phase7][LayoutPersistence]") {
    LayoutPreset p;
    p.name = "x";
    p.serializedData = "layout:x\n";
    CHECK_FALSE(p.isModified);
    p.markModified();
    CHECK(p.isModified);
    p.clearModified();
    CHECK_FALSE(p.isModified);
}

TEST_CASE("LayoutPersistenceManager savePreset and findPreset",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("default");

    CHECK(mgr.savePreset("default", layout));
    CHECK(mgr.presetCount() == 1);
    const auto* p = mgr.findPreset("default");
    REQUIRE(p != nullptr);
    CHECK(p->name == "default");
    CHECK_FALSE(p->isBuiltIn);
}

TEST_CASE("LayoutPersistenceManager savePreset overwrites existing",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("main");
    mgr.savePreset("main", layout);
    mgr.savePreset("main", layout); // overwrite
    CHECK(mgr.presetCount() == 1);
}

TEST_CASE("LayoutPersistenceManager loadPreset roundtrip", "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("my_layout");
    mgr.savePreset("my_layout", layout);

    WorkspaceLayout restored("restored");
    CHECK(mgr.loadPreset("my_layout", restored));
}

TEST_CASE("LayoutPersistenceManager loadPreset returns false for unknown name",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout out("out");
    CHECK_FALSE(mgr.loadPreset("nonexistent", out));
}

TEST_CASE("LayoutPersistenceManager removePreset removes user presets",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("temp");
    mgr.savePreset("temp", layout);
    CHECK(mgr.presetCount() == 1);
    CHECK(mgr.removePreset("temp"));
    CHECK(mgr.presetCount() == 0);
    CHECK_FALSE(mgr.removePreset("nonexistent"));
}

TEST_CASE("LayoutPersistenceManager built-in presets cannot be removed",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    mgr.addBuiltInPreset("default", "layout:default\n");
    CHECK(mgr.builtInCount() == 1);
    CHECK_FALSE(mgr.removePreset("default")); // cannot remove built-in
    CHECK(mgr.presetCount() == 1);
}

TEST_CASE("LayoutPersistenceManager renamePreset works for user presets",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("old_name");
    mgr.savePreset("old_name", layout);
    CHECK(mgr.renamePreset("old_name", "new_name"));
    CHECK(mgr.findPreset("new_name") != nullptr);
    CHECK(mgr.findPreset("old_name") == nullptr);
}

TEST_CASE("LayoutPersistenceManager autoSave and lastUsedPreset",
          "[Phase7][LayoutPersistence]") {
    LayoutPersistenceManager mgr;
    CHECK_FALSE(mgr.autoSave());
    mgr.setAutoSave(true);
    CHECK(mgr.autoSave());

    mgr.setLastUsedPreset("gaming");
    CHECK(mgr.lastUsedPreset() == "gaming");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 7 — ViewportHostContract
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ViewportBounds validity and aspect ratio", "[Phase7][Viewport]") {
    ViewportBounds b{0.f, 0.f, 0.f, 0.f};
    CHECK_FALSE(b.isValid());
    b.width = 1920.f; b.height = 1080.f;
    CHECK(b.isValid());
    CHECK(b.aspectRatio() == Catch::Approx(1920.f / 1080.f));
}

TEST_CASE("ViewportBounds contains", "[Phase7][Viewport]") {
    ViewportBounds b{100.f, 50.f, 800.f, 600.f};
    CHECK(b.contains(400.f, 300.f));
    CHECK_FALSE(b.contains(50.f, 300.f));
    CHECK_FALSE(b.contains(400.f, 10.f));
}

TEST_CASE("viewportStateName and viewportRenderModeName", "[Phase7][Viewport]") {
    CHECK(std::string(viewportStateName(ViewportState::Idle))       == "Idle");
    CHECK(std::string(viewportStateName(ViewportState::Active))     == "Active");
    CHECK(std::string(viewportStateName(ViewportState::Paused))     == "Paused");
    CHECK(std::string(viewportStateName(ViewportState::Released))   == "Released");
    CHECK(std::string(viewportRenderModeName(ViewportRenderMode::Lit))       == "Lit");
    CHECK(std::string(viewportRenderModeName(ViewportRenderMode::Wireframe)) == "Wireframe");
    CHECK(std::string(viewportRenderModeName(ViewportRenderMode::Normals))   == "Normals");
    CHECK(std::string(viewportRenderModeName(ViewportRenderMode::Overdraw))  == "Overdraw");
}

TEST_CASE("ViewportCameraDescriptor validity", "[Phase7][Viewport]") {
    ViewportCameraDescriptor cam;
    CHECK(cam.isValid()); // defaults are valid
    cam.fovDegrees = 0.f;
    CHECK_FALSE(cam.isValid());
    cam.fovDegrees = 60.f;
    cam.nearPlane = cam.farPlane; // near >= far
    CHECK_FALSE(cam.isValid());
}

TEST_CASE("ViewportHostRegistry requestSlot returns valid handle",
          "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 1920.f, 1080.f};
    auto h = reg.requestSlot("scene_editor", b);
    CHECK(h != kInvalidViewportHandle);
    CHECK(reg.slotCount() == 1);
    const auto* slot = reg.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->toolId == "scene_editor");
    CHECK(slot->state  == ViewportState::Idle);
}

TEST_CASE("ViewportHostRegistry requestSlot fails with invalid bounds",
          "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds bad{};
    auto h = reg.requestSlot("tool", bad);
    CHECK(h == kInvalidViewportHandle);
}

TEST_CASE("ViewportHostRegistry requestSlot fails with empty toolId",
          "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 800.f, 600.f};
    CHECK(reg.requestSlot("", b) == kInvalidViewportHandle);
}

TEST_CASE("ViewportHostRegistry activateSlot and pauseSlot", "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 1280.f, 720.f};
    auto h = reg.requestSlot("tool", b);

    CHECK(reg.activateSlot(h));
    CHECK(reg.findSlot(h)->state == ViewportState::Active);
    CHECK(reg.activeCount() == 1);

    CHECK(reg.pauseSlot(h));
    CHECK(reg.findSlot(h)->state == ViewportState::Paused);
    CHECK(reg.activeCount() == 0);

    CHECK(reg.resumeSlot(h));
    CHECK(reg.findSlot(h)->state == ViewportState::Active);
}

TEST_CASE("ViewportHostRegistry releaseSlot removes slot", "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 800.f, 600.f};
    auto h = reg.requestSlot("tool", b);
    REQUIRE(h != kInvalidViewportHandle);
    CHECK(reg.releaseSlot(h));
    CHECK(reg.slotCount() == 0);
    CHECK_FALSE(reg.releaseSlot(h)); // already removed
}

TEST_CASE("ViewportHostRegistry setRenderMode", "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 1920.f, 1080.f};
    auto h = reg.requestSlot("tool", b);
    CHECK(reg.setRenderMode(h, ViewportRenderMode::Wireframe));
    CHECK(reg.findSlot(h)->renderMode == ViewportRenderMode::Wireframe);
}

TEST_CASE("ViewportHostRegistry setCamera validates input", "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 800.f, 600.f};
    auto h = reg.requestSlot("tool", b);

    ViewportCameraDescriptor cam;
    cam.fovDegrees = 90.f;
    CHECK(reg.setCamera(h, cam));

    ViewportCameraDescriptor bad;
    bad.fovDegrees = 0.f;
    CHECK_FALSE(reg.setCamera(h, bad));
}

TEST_CASE("ViewportHostRegistry onFrameRendered increments frameCount",
          "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 800.f, 600.f};
    auto h = reg.requestSlot("tool", b);
    reg.activateSlot(h);
    reg.onFrameRendered(h);
    reg.onFrameRendered(h);
    CHECK(reg.findSlot(h)->frameCount == 2);
}

TEST_CASE("ViewportHostRegistry updateBounds rejects invalid", "[Phase7][Viewport]") {
    ViewportHostRegistry reg;
    ViewportBounds b{0.f, 0.f, 800.f, 600.f};
    auto h = reg.requestSlot("tool", b);
    CHECK_FALSE(reg.updateBounds(h, ViewportBounds{}));
    CHECK(reg.updateBounds(h, {0.f, 0.f, 1920.f, 1080.f}));
    CHECK(reg.findSlot(h)->bounds.width == 1920.f);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 8 — TypographySystem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("textRoleName returns correct strings", "[Phase7][Typography]") {
    CHECK(std::string(textRoleName(TextRole::Heading1))   == "Heading1");
    CHECK(std::string(textRoleName(TextRole::Body))       == "Body");
    CHECK(std::string(textRoleName(TextRole::Label))      == "Label");
    CHECK(std::string(textRoleName(TextRole::Caption))    == "Caption");
    CHECK(std::string(textRoleName(TextRole::Code))       == "Code");
    CHECK(std::string(textRoleName(TextRole::Icon))       == "Icon");
    CHECK(std::string(textRoleName(TextRole::Badge))      == "Badge");
}

TEST_CASE("fontWeightName returns correct strings", "[Phase7][Typography]") {
    CHECK(std::string(fontWeightName(FontWeight::Regular))  == "Regular");
    CHECK(std::string(fontWeightName(FontWeight::Bold))     == "Bold");
    CHECK(std::string(fontWeightName(FontWeight::Light))    == "Light");
    CHECK(std::string(fontWeightName(FontWeight::SemiBold)) == "SemiBold");
}

TEST_CASE("TypefaceDescriptor validity", "[Phase7][Typography]") {
    TypefaceDescriptor d;
    CHECK(d.isValid()); // defaults are valid
    d.family = "";
    CHECK_FALSE(d.isValid());
    d.family = "Segoe UI";
    d.size = 0.f;
    CHECK_FALSE(d.isValid());
}

TEST_CASE("TypefaceDescriptor effectiveLineHeight", "[Phase7][Typography]") {
    TypefaceDescriptor d;
    d.size = 12.f;
    CHECK(d.effectiveLineHeight() == Catch::Approx(12.f * 1.2f));
    d.lineHeight = 18.f;
    CHECK(d.effectiveLineHeight() == Catch::Approx(18.f));
}

TEST_CASE("TypographyRegistry loadDefaults creates all roles",
          "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    CHECK(reg.isLoaded());
    CHECK(reg.validate());
    CHECK(reg.roleCount() == 13); // all TextRole variants
}

TEST_CASE("TypographyRegistry getRole returns correct descriptor",
          "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    const auto& h1 = reg.getRole(TextRole::Heading1);
    CHECK(h1.family == "Segoe UI");
    CHECK(h1.size > reg.getRole(TextRole::Body).size); // heading bigger than body
    CHECK(h1.weight == FontWeight::Bold);
}

TEST_CASE("TypographyRegistry setRole overrides existing", "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    reg.setRole(TextRole::Body, {"Custom Font", 16.f, FontWeight::Medium});
    CHECK(reg.getRole(TextRole::Body).family == "Custom Font");
    CHECK(reg.getRole(TextRole::Body).size   == 16.f);
}

TEST_CASE("TypographyRegistry applyScale multiplies sizes", "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    float originalBodySize = reg.getRole(TextRole::Body).size;
    reg.applyScale(2.f);
    CHECK(reg.getRole(TextRole::Body).size == Catch::Approx(originalBodySize * 2.f));
}

TEST_CASE("TypographyRegistry applyScale ignores non-positive factor",
          "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    float originalBodySize = reg.getRole(TextRole::Body).size;
    reg.applyScale(0.f);  // no-op
    reg.applyScale(-1.f); // no-op
    CHECK(reg.getRole(TextRole::Body).size == Catch::Approx(originalBodySize));
}

TEST_CASE("TypographyEnforcer passes on valid registry", "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    auto report = TypographyEnforcer::enforce(reg);
    CHECK(report.passed);
    CHECK_FALSE(report.hasViolations());
}

TEST_CASE("TypographyEnforcer fails when body size is too small",
          "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    reg.setRole(TextRole::Body, {"Segoe UI", 5.f, FontWeight::Regular}); // too small
    auto report = TypographyEnforcer::enforce(reg);
    CHECK_FALSE(report.passed);
    CHECK(report.hasViolations());
}

TEST_CASE("TypographyEnforcer fails when heading is not larger than body",
          "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    // Make heading smaller than body
    reg.setRole(TextRole::Heading1, {"Segoe UI", 8.f, FontWeight::Bold});
    auto report = TypographyEnforcer::enforce(reg);
    CHECK_FALSE(report.passed);
    CHECK(report.count() > 0);
}

TEST_CASE("TypographyEnforcer flags non-monospace code roles",
          "[Phase7][Typography]") {
    TypographyRegistry reg;
    reg.loadDefaults();
    reg.setRole(TextRole::Code, {"Segoe UI", 12.f, FontWeight::Regular}); // not monospace
    auto report = TypographyEnforcer::enforce(reg);
    // Should have a violation for Code role
    bool codeViolation = false;
    for (const auto& v : report.violations)
        if (v.role == TextRole::Code) codeViolation = true;
    CHECK(codeViolation);
}

TEST_CASE("TypographyEnforcementReport addViolation", "[Phase7][Typography]") {
    TypographyEnforcementReport report;
    report.passed = true;
    report.addViolation(TextRole::Body, "test violation");
    CHECK(report.count() == 1);
    CHECK_FALSE(report.passed);
    CHECK(report.hasViolations());
}
