// Tests/Workspace/test_phase_g2_asset_editor.cpp
//
// Phase G.2 — Asset Editor: 3D preview, dependency viewer, reimport pipeline
//
// AssetPreviewState
//   - Default state: identity transform, zero material slots, zero attachments, not dirty
//   - setPosition marks dirty, transform reflects values
//   - setRotation marks dirty, transform reflects values
//   - setScale marks dirty, transform reflects values
//   - resetTransform returns to identity
//   - addMaterialSlot increases count, findMaterialSlot returns entry
//   - setMaterialOverride updates guid, marks dirty
//   - clearMaterialOverride empties guid
//   - findMaterialSlot returns nullptr for unknown index
//   - setActiveSlot tracks active slot index
//   - addAttachmentPoint increases count
//   - setAttachment updates guid on existing socket
//   - clearAttachment empties guid
//   - removeAttachmentPoint removes entry
//   - findAttachment returns nullptr for unknown socket
//   - clearDirty resets dirty flag
//   - reset() returns everything to defaults
//
// AssetDependencyViewer
//   - Default state: flat mode, zero entries
//   - loadFromDocument populates entries from AssetDocument
//   - totalCount matches document dependency count
//   - visibleEntries() returns all in Flat mode with no filter
//   - setRelationshipFilter narrows visibleEntries
//   - clearRelationshipFilter restores all entries
//   - setMode(Tree) combined with setMaxDepth limits depth
//   - highlightByGuid marks matching entries and returns count
//   - clearHighlights removes all highlights
//   - highlightedCount returns highlighted count
//   - rootGuid matches document guid after load
//   - setOnReload callback fires on loadFromDocument
//   - DependencyViewMode names
//
// AssetReimportPipeline
//   - Default state: zero counts
//   - enqueue returns a nonzero job ID
//   - enqueue empty guid or path returns 0
//   - enqueue same guid twice is no-op (returns 0)
//   - queuedCount reflects enqueued jobs
//   - cancel removes queued job, returns true
//   - cancel unknown guid returns false
//   - processNext returns false on empty queue
//   - processNext runs one job (Succeeded), moves to results
//   - processAll drains the queue, returns count
//   - statusOf queued guid returns Queued
//   - statusOf processed guid returns Succeeded
//   - statusOf unknown guid returns Skipped
//   - succeededCount / failedCount / processedCount track correctly
//   - setOnResult callback fires per completed job
//   - clearResults empties results list
//   - ImportJobStatus names

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/Editor/Asset/AssetPreviewState.h"
#include "NF/Editor/Asset/AssetDependencyViewer.h"
#include "NF/Editor/Asset/AssetReimportPipeline.h"
#include "NF/Editor/AssetDocument.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────────────────
// AssetPreviewState — defaults
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetPreviewState: default state is identity, clean, no slots, no attachments",
          "[phase_g][g2][preview]") {
    AssetPreviewState ps;
    CHECK(ps.transform().isIdentity());
    CHECK(!ps.isDirty());
    CHECK(ps.materialSlotCount() == 0);
    CHECK(ps.attachmentCount() == 0);
    CHECK(ps.activeSlotIndex() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetPreviewState — transform
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetPreviewState: setPosition marks dirty and stores values",
          "[phase_g][g2][preview][transform]") {
    AssetPreviewState ps;
    ps.setPosition(1.f, 2.f, 3.f);
    CHECK(ps.isDirty());
    CHECK(ps.transform().posX == Catch::Approx(1.f));
    CHECK(ps.transform().posY == Catch::Approx(2.f));
    CHECK(ps.transform().posZ == Catch::Approx(3.f));
}

TEST_CASE("AssetPreviewState: setRotation marks dirty and stores values",
          "[phase_g][g2][preview][transform]") {
    AssetPreviewState ps;
    ps.setRotation(10.f, 20.f, 30.f);
    CHECK(ps.isDirty());
    CHECK(ps.transform().rotX == Catch::Approx(10.f));
    CHECK(ps.transform().rotY == Catch::Approx(20.f));
    CHECK(ps.transform().rotZ == Catch::Approx(30.f));
}

TEST_CASE("AssetPreviewState: setScale marks dirty and stores values",
          "[phase_g][g2][preview][transform]") {
    AssetPreviewState ps;
    ps.setScale(2.f, 3.f, 4.f);
    CHECK(ps.isDirty());
    CHECK(ps.transform().scaleX == Catch::Approx(2.f));
    CHECK(ps.transform().scaleY == Catch::Approx(3.f));
    CHECK(ps.transform().scaleZ == Catch::Approx(4.f));
}

TEST_CASE("AssetPreviewState: resetTransform returns to identity",
          "[phase_g][g2][preview][transform]") {
    AssetPreviewState ps;
    ps.setPosition(5.f, 5.f, 5.f);
    ps.setRotation(45.f, 0.f, 0.f);
    ps.clearDirty();
    ps.resetTransform();
    CHECK(ps.transform().isIdentity());
    CHECK(ps.isDirty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetPreviewState — material slots
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetPreviewState: addMaterialSlot increases count and is findable",
          "[phase_g][g2][preview][material]") {
    AssetPreviewState ps;
    PreviewMaterialSlot slot;
    slot.slotIndex   = 0;
    slot.displayName = "Base Material";
    ps.addMaterialSlot(slot);
    CHECK(ps.materialSlotCount() == 1);
    CHECK(ps.findMaterialSlot(0) != nullptr);
    CHECK(ps.findMaterialSlot(0)->displayName == "Base Material");
}

TEST_CASE("AssetPreviewState: setMaterialOverride updates guid and marks dirty",
          "[phase_g][g2][preview][material]") {
    AssetPreviewState ps;
    PreviewMaterialSlot slot;
    slot.slotIndex = 0;
    ps.addMaterialSlot(slot);
    ps.clearDirty();

    bool ok = ps.setMaterialOverride(0, "mat-guid-001");
    CHECK(ok);
    CHECK(ps.isDirty());
    CHECK(ps.findMaterialSlot(0)->materialGuid == "mat-guid-001");
    CHECK(ps.findMaterialSlot(0)->hasOverride());
}

TEST_CASE("AssetPreviewState: clearMaterialOverride empties guid",
          "[phase_g][g2][preview][material]") {
    AssetPreviewState ps;
    PreviewMaterialSlot slot;
    slot.slotIndex   = 0;
    slot.materialGuid = "some-guid";
    ps.addMaterialSlot(slot);

    ps.clearMaterialOverride(0);
    CHECK(!ps.findMaterialSlot(0)->hasOverride());
}

TEST_CASE("AssetPreviewState: findMaterialSlot returns nullptr for unknown index",
          "[phase_g][g2][preview][material]") {
    AssetPreviewState ps;
    CHECK(ps.findMaterialSlot(99) == nullptr);
}

TEST_CASE("AssetPreviewState: setMaterialOverride returns false for unknown slot",
          "[phase_g][g2][preview][material]") {
    AssetPreviewState ps;
    CHECK(!ps.setMaterialOverride(5, "x"));
}

TEST_CASE("AssetPreviewState: setActiveSlot tracks active slot",
          "[phase_g][g2][preview][material]") {
    AssetPreviewState ps;
    ps.setActiveSlot(3);
    CHECK(ps.activeSlotIndex() == 3);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetPreviewState — attachment points
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetPreviewState: addAttachmentPoint increases count",
          "[phase_g][g2][preview][attachment]") {
    AssetPreviewState ps;
    PreviewAttachmentPoint ap;
    ap.socketName = "hand_r";
    ps.addAttachmentPoint(ap);
    CHECK(ps.attachmentCount() == 1);
    CHECK(ps.isDirty());
}

TEST_CASE("AssetPreviewState: setAttachment updates guid on existing socket",
          "[phase_g][g2][preview][attachment]") {
    AssetPreviewState ps;
    PreviewAttachmentPoint ap;
    ap.socketName = "hand_r";
    ps.addAttachmentPoint(ap);
    ps.clearDirty();

    bool ok = ps.setAttachment("hand_r", "prop-guid-001");
    CHECK(ok);
    CHECK(ps.isDirty());
    CHECK(ps.findAttachment("hand_r")->attachedAssetGuid == "prop-guid-001");
}

TEST_CASE("AssetPreviewState: clearAttachment empties guid",
          "[phase_g][g2][preview][attachment]") {
    AssetPreviewState ps;
    PreviewAttachmentPoint ap;
    ap.socketName         = "hand_r";
    ap.attachedAssetGuid  = "prop-001";
    ps.addAttachmentPoint(ap);

    ps.clearAttachment("hand_r");
    CHECK(ps.findAttachment("hand_r")->attachedAssetGuid.empty());
}

TEST_CASE("AssetPreviewState: removeAttachmentPoint removes entry",
          "[phase_g][g2][preview][attachment]") {
    AssetPreviewState ps;
    PreviewAttachmentPoint ap;
    ap.socketName = "hand_r";
    ps.addAttachmentPoint(ap);
    ps.clearDirty();

    bool ok = ps.removeAttachmentPoint("hand_r");
    CHECK(ok);
    CHECK(ps.attachmentCount() == 0);
    CHECK(ps.isDirty());
}

TEST_CASE("AssetPreviewState: findAttachment returns nullptr for unknown socket",
          "[phase_g][g2][preview][attachment]") {
    AssetPreviewState ps;
    CHECK(ps.findAttachment("no_such") == nullptr);
}

TEST_CASE("AssetPreviewState: setAttachment returns false for unknown socket",
          "[phase_g][g2][preview][attachment]") {
    AssetPreviewState ps;
    CHECK(!ps.setAttachment("missing", "x"));
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetPreviewState — dirty tracking and reset
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetPreviewState: clearDirty resets flag",
          "[phase_g][g2][preview][dirty]") {
    AssetPreviewState ps;
    ps.setPosition(1.f, 0.f, 0.f);
    REQUIRE(ps.isDirty());
    ps.clearDirty();
    CHECK(!ps.isDirty());
}

TEST_CASE("AssetPreviewState: reset() returns everything to defaults",
          "[phase_g][g2][preview][reset]") {
    AssetPreviewState ps;
    ps.setPosition(5.f, 5.f, 5.f);
    PreviewMaterialSlot slot; slot.slotIndex = 0;
    ps.addMaterialSlot(slot);
    PreviewAttachmentPoint ap; ap.socketName = "s";
    ps.addAttachmentPoint(ap);

    ps.reset();
    CHECK(ps.transform().isIdentity());
    CHECK(ps.materialSlotCount() == 0);
    CHECK(ps.attachmentCount() == 0);
    CHECK(!ps.isDirty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetDependencyViewer — DependencyViewMode names
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("DependencyViewMode names are correct",
          "[phase_g][g2][dep_viewer][names]") {
    CHECK(std::string(dependencyViewModeName(DependencyViewMode::Flat)) == "Flat");
    CHECK(std::string(dependencyViewModeName(DependencyViewMode::Tree)) == "Tree");
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetDependencyViewer — defaults
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetDependencyViewer: default state is flat mode, zero entries",
          "[phase_g][g2][dep_viewer]") {
    AssetDependencyViewer viewer;
    CHECK(viewer.mode() == DependencyViewMode::Flat);
    CHECK(viewer.totalCount() == 0);
    CHECK(viewer.visibleCount() == 0);
    CHECK(viewer.rootGuid().empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetDependencyViewer — loadFromDocument
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetDependencyViewer: loadFromDocument populates entries",
          "[phase_g][g2][dep_viewer]") {
    AssetDocument doc("asset-001", "/content/hero.asset");
    doc.addDependency({"tex-001", "texture"});
    doc.addDependency({"mat-001", "material"});
    doc.addDependency({"skel-001", "skeleton"});

    AssetDependencyViewer viewer;
    viewer.loadFromDocument(doc);

    CHECK(viewer.totalCount() == 3);
    CHECK(viewer.rootGuid() == "asset-001");
}

TEST_CASE("AssetDependencyViewer: visibleEntries returns all in Flat mode",
          "[phase_g][g2][dep_viewer]") {
    AssetDocument doc("g", "/p");
    doc.addDependency({"a", "texture"});
    doc.addDependency({"b", "material"});

    AssetDependencyViewer viewer;
    viewer.loadFromDocument(doc);

    CHECK(viewer.visibleEntries().size() == 2);
}

TEST_CASE("AssetDependencyViewer: setRelationshipFilter narrows visible entries",
          "[phase_g][g2][dep_viewer]") {
    AssetDocument doc("g", "/p");
    doc.addDependency({"a", "texture"});
    doc.addDependency({"b", "material"});
    doc.addDependency({"c", "texture"});

    AssetDependencyViewer viewer;
    viewer.loadFromDocument(doc);
    viewer.setRelationshipFilter("texture");

    auto visible = viewer.visibleEntries();
    CHECK(visible.size() == 2);
    for (const auto& e : visible)
        CHECK(e.relationship == "texture");
}

TEST_CASE("AssetDependencyViewer: clearRelationshipFilter restores all entries",
          "[phase_g][g2][dep_viewer]") {
    AssetDocument doc("g", "/p");
    doc.addDependency({"a", "texture"});
    doc.addDependency({"b", "material"});

    AssetDependencyViewer viewer;
    viewer.loadFromDocument(doc);
    viewer.setRelationshipFilter("texture");
    viewer.clearRelationshipFilter();

    CHECK(viewer.visibleCount() == 2);
}

TEST_CASE("AssetDependencyViewer: onReload callback fires on loadFromDocument",
          "[phase_g][g2][dep_viewer]") {
    AssetDocument doc("g", "/p");
    doc.addDependency({"a", "x"});

    AssetDependencyViewer viewer;
    bool fired = false;
    viewer.setOnReload([&]() { fired = true; });
    viewer.loadFromDocument(doc);
    CHECK(fired);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetDependencyViewer — highlights
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetDependencyViewer: highlightByGuid marks matching entries",
          "[phase_g][g2][dep_viewer][highlight]") {
    AssetDocument doc("g", "/p");
    doc.addDependency({"a", "x"});
    doc.addDependency({"b", "x"});
    doc.addDependency({"a", "y"});

    AssetDependencyViewer viewer;
    viewer.loadFromDocument(doc);

    uint32_t count = viewer.highlightByGuid("a");
    CHECK(count == 2);
    CHECK(viewer.highlightedCount() == 2);
}

TEST_CASE("AssetDependencyViewer: clearHighlights removes all highlights",
          "[phase_g][g2][dep_viewer][highlight]") {
    AssetDocument doc("g", "/p");
    doc.addDependency({"a", "x"});

    AssetDependencyViewer viewer;
    viewer.loadFromDocument(doc);
    viewer.highlightByGuid("a");
    viewer.clearHighlights();
    CHECK(viewer.highlightedCount() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// ImportJobStatus names
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ImportJobStatus names are correct",
          "[phase_g][g2][reimport][names]") {
    CHECK(std::string(importJobStatusName(ImportJobStatus::Queued))     == "Queued");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Processing)) == "Processing");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Succeeded))  == "Succeeded");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Failed))     == "Failed");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Skipped))    == "Skipped");
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetReimportPipeline — default state
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetReimportPipeline: default state is all zeros",
          "[phase_g][g2][reimport]") {
    AssetReimportPipeline pipeline;
    CHECK(pipeline.queuedCount()    == 0);
    CHECK(pipeline.processedCount() == 0);
    CHECK(pipeline.succeededCount() == 0);
    CHECK(pipeline.failedCount()    == 0);
    CHECK(pipeline.results().empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetReimportPipeline — enqueue
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetReimportPipeline: enqueue returns nonzero job ID",
          "[phase_g][g2][reimport][enqueue]") {
    AssetReimportPipeline pipeline;
    uint32_t id = pipeline.enqueue("asset-001", "/src/hero.fbx");
    CHECK(id > 0);
}

TEST_CASE("AssetReimportPipeline: enqueue with empty guid returns 0",
          "[phase_g][g2][reimport][enqueue]") {
    AssetReimportPipeline pipeline;
    CHECK(pipeline.enqueue("", "/src/file.fbx") == 0);
}

TEST_CASE("AssetReimportPipeline: enqueue with empty path returns 0",
          "[phase_g][g2][reimport][enqueue]") {
    AssetReimportPipeline pipeline;
    CHECK(pipeline.enqueue("asset-001", "") == 0);
}

TEST_CASE("AssetReimportPipeline: enqueue same guid twice is no-op",
          "[phase_g][g2][reimport][enqueue]") {
    AssetReimportPipeline pipeline;
    uint32_t id1 = pipeline.enqueue("asset-001", "/src/hero.fbx");
    uint32_t id2 = pipeline.enqueue("asset-001", "/src/hero.fbx");
    CHECK(id1 > 0);
    CHECK(id2 == 0);
    CHECK(pipeline.queuedCount() == 1);
}

TEST_CASE("AssetReimportPipeline: queuedCount reflects enqueued jobs",
          "[phase_g][g2][reimport][enqueue]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("a", "/a.fbx");
    pipeline.enqueue("b", "/b.fbx");
    pipeline.enqueue("c", "/c.fbx");
    CHECK(pipeline.queuedCount() == 3);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetReimportPipeline — cancel
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetReimportPipeline: cancel removes queued job",
          "[phase_g][g2][reimport][cancel]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("asset-001", "/src/hero.fbx");
    bool ok = pipeline.cancel("asset-001");
    CHECK(ok);
    CHECK(pipeline.queuedCount() == 0);
}

TEST_CASE("AssetReimportPipeline: cancel unknown guid returns false",
          "[phase_g][g2][reimport][cancel]") {
    AssetReimportPipeline pipeline;
    CHECK(!pipeline.cancel("no-such"));
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetReimportPipeline — processing
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetReimportPipeline: processNext returns false on empty queue",
          "[phase_g][g2][reimport][process]") {
    AssetReimportPipeline pipeline;
    CHECK(!pipeline.processNext());
}

TEST_CASE("AssetReimportPipeline: processNext processes one job successfully",
          "[phase_g][g2][reimport][process]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("asset-001", "/src/hero.fbx");

    bool ok = pipeline.processNext();
    CHECK(ok);
    CHECK(pipeline.queuedCount() == 0);
    CHECK(pipeline.processedCount() == 1);
    CHECK(pipeline.succeededCount() == 1);
    CHECK(pipeline.results().size() == 1);
    CHECK(pipeline.results()[0].succeeded());
}

TEST_CASE("AssetReimportPipeline: processAll drains the queue",
          "[phase_g][g2][reimport][process]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("a", "/a.fbx");
    pipeline.enqueue("b", "/b.fbx");
    pipeline.enqueue("c", "/c.fbx");

    uint32_t count = pipeline.processAll();
    CHECK(count == 3);
    CHECK(pipeline.queuedCount() == 0);
    CHECK(pipeline.processedCount() == 3);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetReimportPipeline — statusOf
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetReimportPipeline: statusOf queued guid returns Queued",
          "[phase_g][g2][reimport][status]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("asset-001", "/src/hero.fbx");
    CHECK(pipeline.statusOf("asset-001") == ImportJobStatus::Queued);
}

TEST_CASE("AssetReimportPipeline: statusOf processed guid returns Succeeded",
          "[phase_g][g2][reimport][status]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("asset-001", "/src/hero.fbx");
    pipeline.processNext();
    CHECK(pipeline.statusOf("asset-001") == ImportJobStatus::Succeeded);
}

TEST_CASE("AssetReimportPipeline: statusOf unknown guid returns Skipped",
          "[phase_g][g2][reimport][status]") {
    AssetReimportPipeline pipeline;
    CHECK(pipeline.statusOf("unknown") == ImportJobStatus::Skipped);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetReimportPipeline — callbacks and clearResults
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetReimportPipeline: onResult callback fires per completed job",
          "[phase_g][g2][reimport][callback]") {
    AssetReimportPipeline pipeline;
    std::vector<std::string> completedGuids;
    pipeline.setOnResult([&](const ImportJobResult& r) {
        completedGuids.push_back(r.guid);
    });

    pipeline.enqueue("a", "/a.fbx");
    pipeline.enqueue("b", "/b.fbx");
    pipeline.processAll();

    REQUIRE(completedGuids.size() == 2);
    CHECK(completedGuids[0] == "a");
    CHECK(completedGuids[1] == "b");
}

TEST_CASE("AssetReimportPipeline: clearResults empties results list",
          "[phase_g][g2][reimport][results]") {
    AssetReimportPipeline pipeline;
    pipeline.enqueue("a", "/a.fbx");
    pipeline.processAll();
    REQUIRE(pipeline.results().size() == 1);
    pipeline.clearResults();
    CHECK(pipeline.results().empty());
}
