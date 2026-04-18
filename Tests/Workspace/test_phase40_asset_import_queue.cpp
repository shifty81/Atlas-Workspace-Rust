// Tests/Workspace/test_phase40_asset_import_queue.cpp
// Phase 40 — Asset Import Queue
//
// Tests for:
//   1. ImportJobStatus — enum name helpers
//   2. ImportJob       — status helpers (isDone/isFailed/isActive/isFinished)
//   3. AssetImportQueue — enqueue/cancel/startNext/advance/failJob/find; stats

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/AssetImportQueue.h"
#include <string>

using namespace NF;

// ── Helper ────────────────────────────────────────────────────────

static IntakeItem makeItem(const std::string& path) {
    IntakeItem item;
    item.path   = path;
    item.source = IntakeSource::FileDrop;
    return item;
}

// ─────────────────────────────────────────────────────────────────
// 1. ImportJobStatus
// ─────────────────────────────────────────────────────────────────

TEST_CASE("ImportJobStatus – all values have names", "[phase40][ImportJobStatus]") {
    CHECK(std::string(importJobStatusName(ImportJobStatus::Queued))      == "Queued");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Validating))  == "Validating");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Importing))   == "Importing");
    CHECK(std::string(importJobStatusName(ImportJobStatus::PostProcess)) == "PostProcess");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Done))        == "Done");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Failed))      == "Failed");
    CHECK(std::string(importJobStatusName(ImportJobStatus::Cancelled))   == "Cancelled");
}

// ─────────────────────────────────────────────────────────────────
// 2. ImportJob helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("ImportJob – default state is Queued", "[phase40][ImportJob]") {
    ImportJob j;
    CHECK(j.status == ImportJobStatus::Queued);
    CHECK(j.progress == 0.f);
    CHECK(j.id == 0);
}

TEST_CASE("ImportJob – isDone only for Done status", "[phase40][ImportJob]") {
    ImportJob j;
    j.status = ImportJobStatus::Done;
    CHECK(j.isDone());
    j.status = ImportJobStatus::Failed;
    CHECK_FALSE(j.isDone());
}

TEST_CASE("ImportJob – isFailed only for Failed status", "[phase40][ImportJob]") {
    ImportJob j;
    j.status = ImportJobStatus::Failed;
    CHECK(j.isFailed());
    j.status = ImportJobStatus::Cancelled;
    CHECK_FALSE(j.isFailed());
}

TEST_CASE("ImportJob – isActive during in-progress states", "[phase40][ImportJob]") {
    ImportJob j;
    j.status = ImportJobStatus::Validating;
    CHECK(j.isActive());
    j.status = ImportJobStatus::Importing;
    CHECK(j.isActive());
    j.status = ImportJobStatus::PostProcess;
    CHECK(j.isActive());
    j.status = ImportJobStatus::Queued;
    CHECK_FALSE(j.isActive());
    j.status = ImportJobStatus::Done;
    CHECK_FALSE(j.isActive());
}

TEST_CASE("ImportJob – isFinished for Done/Failed/Cancelled", "[phase40][ImportJob]") {
    ImportJob j;
    j.status = ImportJobStatus::Done;
    CHECK(j.isFinished());
    j.status = ImportJobStatus::Failed;
    CHECK(j.isFinished());
    j.status = ImportJobStatus::Cancelled;
    CHECK(j.isFinished());
    j.status = ImportJobStatus::Queued;
    CHECK_FALSE(j.isFinished());
    j.status = ImportJobStatus::Importing;
    CHECK_FALSE(j.isFinished());
}

// ─────────────────────────────────────────────────────────────────
// 3. AssetImportQueue
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetImportQueue – default empty", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    CHECK(q.totalCount()    == 0);
    CHECK(q.queuedCount()   == 0);
    CHECK(q.activeCount()   == 0);
    CHECK(q.totalEnqueued() == 0);
}

TEST_CASE("AssetImportQueue – enqueue adds job", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    CHECK(q.enqueue(makeItem("Textures/albedo.png")));
    CHECK(q.totalCount()    == 1);
    CHECK(q.queuedCount()   == 1);
    CHECK(q.totalEnqueued() == 1);
}

TEST_CASE("AssetImportQueue – find by id", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("Textures/albedo.png"));
    const ImportJob* j = q.find(1);
    REQUIRE(j != nullptr);
    CHECK(j->status == ImportJobStatus::Queued);
}

TEST_CASE("AssetImportQueue – find unknown id returns nullptr", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    CHECK(q.find(999) == nullptr);
}

TEST_CASE("AssetImportQueue – startNext advances first queued job", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("mesh.fbx"));
    CHECK(q.startNext());
    const ImportJob* j = q.find(1);
    REQUIRE(j != nullptr);
    CHECK(j->status == ImportJobStatus::Validating);
    CHECK(q.activeCount() == 1);
    CHECK(q.queuedCount() == 0);
}

TEST_CASE("AssetImportQueue – startNext returns false when none queued", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    CHECK_FALSE(q.startNext());
}

TEST_CASE("AssetImportQueue – advance progresses job through pipeline", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("audio/track.wav"));
    uint32_t id = q.find(1)->id;

    q.advance(id);                                    // Queued → Validating
    CHECK(q.find(id)->status == ImportJobStatus::Validating);

    q.advance(id);                                    // Validating → Importing
    CHECK(q.find(id)->status == ImportJobStatus::Importing);

    q.advance(id, 1.0f);                              // Importing → PostProcess (progress 1)
    CHECK(q.find(id)->status == ImportJobStatus::PostProcess);

    q.advance(id);                                    // PostProcess → Done
    const ImportJob* j = q.find(id);
    REQUIRE(j != nullptr);
    CHECK(j->status == ImportJobStatus::Done);
    CHECK(j->isDone());
    CHECK(j->progress == 1.f);
    CHECK_FALSE(j->outputPath.empty());
}

TEST_CASE("AssetImportQueue – advance updates totalCompleted", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("script.lua"));
    uint32_t id = 1;
    q.advance(id); q.advance(id); q.advance(id, 1.f); q.advance(id);
    CHECK(q.totalCompleted() == 1);
}

TEST_CASE("AssetImportQueue – failJob marks job as failed", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("bad.xyz"));
    q.startNext();
    CHECK(q.failJob(1, "Unsupported format"));
    const ImportJob* j = q.find(1);
    REQUIRE(j != nullptr);
    CHECK(j->isFailed());
    CHECK(j->errorMsg == "Unsupported format");
    CHECK(q.totalFailed() == 1);
}

TEST_CASE("AssetImportQueue – failJob ignores finished jobs", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("done.png"));
    uint32_t id = 1;
    q.advance(id); q.advance(id); q.advance(id, 1.f); q.advance(id); // → Done
    CHECK_FALSE(q.failJob(id, "too late"));
}

TEST_CASE("AssetImportQueue – cancel queued job", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("model.obj"));
    CHECK(q.cancel(1));
    const ImportJob* j = q.find(1);
    REQUIRE(j != nullptr);
    CHECK(j->status == ImportJobStatus::Cancelled);
    CHECK(q.totalCancelled() == 1);
}

TEST_CASE("AssetImportQueue – cancel unknown id returns false", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    CHECK_FALSE(q.cancel(42));
}

TEST_CASE("AssetImportQueue – clearFinished removes done/failed/cancelled jobs", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("a.png"));
    q.enqueue(makeItem("b.png"));
    // advance first to Done
    uint32_t id = 1;
    q.advance(id); q.advance(id); q.advance(id, 1.f); q.advance(id);
    CHECK(q.totalCount() == 2);
    q.clearFinished();
    CHECK(q.totalCount() == 1); // only b.png remains
}

TEST_CASE("AssetImportQueue – priority ordering: high priority first", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    q.enqueue(makeItem("low.png"),    0);
    q.enqueue(makeItem("high.png"), 100);
    const ImportJob* first = &q.jobs().front();
    CHECK(first->priority == 100);
}

TEST_CASE("AssetImportQueue – onComplete callback fires on Done", "[phase40][AssetImportQueue]") {
    AssetImportQueue q;
    bool called = false;
    q.setOnComplete([&](const ImportJob&) { called = true; });
    q.enqueue(makeItem("tex.png"));
    uint32_t id = 1;
    q.advance(id); q.advance(id); q.advance(id, 1.f); q.advance(id);
    CHECK(called);
}

TEST_CASE("AssetImportQueue – enqueueFromPipeline ingests pending items", "[phase40][AssetImportQueue]") {
    FileIntakePipeline pipeline;
    pipeline.ingest("Assets/tex.png",  IntakeSource::FileDrop);
    pipeline.ingest("Assets/mesh.fbx", IntakeSource::FileDrop);

    AssetImportQueue q;
    size_t count = q.enqueueFromPipeline(pipeline);
    CHECK(count == 2);
    CHECK(q.totalCount() == 2);
}

// ─────────────────────────────────────────────────────────────────
// Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetImportQueue – full pipeline: enqueue, advance, complete", "[phase40][integration]") {
    AssetImportQueue q;
    bool completedA = false;
    bool completedB = false;

    q.setOnComplete([&](const ImportJob& j) {
        if (j.intakeItem.path == "Textures/a.png") completedA = true;
        if (j.intakeItem.path == "Textures/b.png") completedB = true;
    });

    q.enqueue(makeItem("Textures/a.png"), 10);
    q.enqueue(makeItem("Textures/b.png"),  5);

    // Process job 1 fully
    q.advance(1); q.advance(1); q.advance(1, 1.f); q.advance(1);
    // Process job 2 fully
    q.advance(2); q.advance(2); q.advance(2, 1.f); q.advance(2);

    CHECK(completedA);
    CHECK(completedB);
    CHECK(q.totalCompleted() == 2);
    CHECK(q.totalFailed()    == 0);
}

TEST_CASE("AssetImportQueue – mixed: some complete, some fail, some cancel", "[phase40][integration]") {
    AssetImportQueue q;
    q.enqueue(makeItem("ok.png"));
    q.enqueue(makeItem("bad.png"));
    q.enqueue(makeItem("skip.png"));

    // ok.png → Done
    q.advance(1); q.advance(1); q.advance(1, 1.f); q.advance(1);
    // bad.png → Failed
    q.startNext();
    q.failJob(2, "parse error");
    // skip.png → Cancelled
    q.cancel(3);

    CHECK(q.totalCompleted() == 1);
    CHECK(q.totalFailed()    == 1);
    CHECK(q.totalCancelled() == 1);

    q.clearFinished();
    CHECK(q.totalCount() == 0);
}
