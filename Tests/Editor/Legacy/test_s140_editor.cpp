// S140 editor tests: FileIntakePipeline, DropTargetHandler, AssetImportQueue
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── FileIntakePipeline ────────────────────────────────────────────────────────

TEST_CASE("IntakeSource names", "[Editor][S140]") {
    REQUIRE(std::string(intakeSourceName(IntakeSource::FileDrop))       == "FileDrop");
    REQUIRE(std::string(intakeSourceName(IntakeSource::FileDialog))     == "FileDialog");
    REQUIRE(std::string(intakeSourceName(IntakeSource::CLI))            == "CLI");
    REQUIRE(std::string(intakeSourceName(IntakeSource::URLScheme))      == "URLScheme");
    REQUIRE(std::string(intakeSourceName(IntakeSource::ClipboardPaste)) == "ClipboardPaste");
}

TEST_CASE("IntakeFileType names", "[Editor][S140]") {
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Unknown)) == "Unknown");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Texture)) == "Texture");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Mesh))    == "Mesh");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Audio))   == "Audio");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Script))  == "Script");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Shader))  == "Shader");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Scene))   == "Scene");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Font))    == "Font");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Video))   == "Video");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Archive)) == "Archive");
    REQUIRE(std::string(intakeFileTypeName(IntakeFileType::Project)) == "Project");
}

TEST_CASE("detectIntakeFileType from extension", "[Editor][S140]") {
    REQUIRE(detectIntakeFileType(".png")         == IntakeFileType::Texture);
    REQUIRE(detectIntakeFileType(".jpg")         == IntakeFileType::Texture);
    REQUIRE(detectIntakeFileType(".exr")         == IntakeFileType::Texture);
    REQUIRE(detectIntakeFileType(".fbx")         == IntakeFileType::Mesh);
    REQUIRE(detectIntakeFileType(".obj")         == IntakeFileType::Mesh);
    REQUIRE(detectIntakeFileType(".gltf")        == IntakeFileType::Mesh);
    REQUIRE(detectIntakeFileType(".wav")         == IntakeFileType::Audio);
    REQUIRE(detectIntakeFileType(".mp3")         == IntakeFileType::Audio);
    REQUIRE(detectIntakeFileType(".lua")         == IntakeFileType::Script);
    REQUIRE(detectIntakeFileType(".cs")          == IntakeFileType::Script);
    REQUIRE(detectIntakeFileType(".hlsl")        == IntakeFileType::Shader);
    REQUIRE(detectIntakeFileType(".atlasscene")  == IntakeFileType::Scene);
    REQUIRE(detectIntakeFileType(".ttf")         == IntakeFileType::Font);
    REQUIRE(detectIntakeFileType(".mp4")         == IntakeFileType::Video);
    REQUIRE(detectIntakeFileType(".zip")         == IntakeFileType::Archive);
    REQUIRE(detectIntakeFileType(".atlasproject")== IntakeFileType::Project);
    REQUIRE(detectIntakeFileType(".xyz")         == IntakeFileType::Unknown);
}

TEST_CASE("IntakeItem filename helper", "[Editor][S140]") {
    IntakeItem item;
    item.id = 1;
    item.path = "C:/Assets/textures/rock_diffuse.png";
    REQUIRE(item.filename() == "rock_diffuse.png");
}

TEST_CASE("FileIntakePipeline ingest single file", "[Editor][S140]") {
    FileIntakePipeline pipe;
    REQUIRE(pipe.ingest("Assets/rock.png", IntakeSource::FileDrop));
    REQUIRE(pipe.pendingCount() == 1u);
    REQUIRE(pipe.ingestedCount() == 1u);

    const auto* item = pipe.pendingItems().front().id > 0 ? &pipe.pendingItems().front() : nullptr;
    REQUIRE(item != nullptr);
    REQUIRE(item->fileType == IntakeFileType::Texture);
    REQUIRE(item->validated == true);
}

TEST_CASE("FileIntakePipeline ingest batch", "[Editor][S140]") {
    FileIntakePipeline pipe;
    std::vector<std::string> paths = {
        "model.fbx", "sound.wav", "shader.hlsl"
    };
    size_t accepted = pipe.ingestBatch(paths, IntakeSource::FileDialog);
    REQUIRE(accepted == 3u);
    REQUIRE(pipe.pendingCount() == 3u);
}

TEST_CASE("FileIntakePipeline handler can reject", "[Editor][S140]") {
    FileIntakePipeline pipe;
    bool handlerCalled = false;
    pipe.addHandler("reject_unknown", [&](IntakeItem& item) -> bool {
        handlerCalled = true;
        if (item.fileType == IntakeFileType::Unknown) {
            item.rejectReason = "Unknown file type not accepted";
            return false;
        }
        return true;
    });

    REQUIRE(!pipe.ingest("file.xyz", IntakeSource::FileDrop));
    REQUIRE(handlerCalled);
    REQUIRE(pipe.rejectedCount() == 1u);
    REQUIRE(pipe.pendingCount()  == 0u);
}

TEST_CASE("FileIntakePipeline handler chain: all pass", "[Editor][S140]") {
    FileIntakePipeline pipe;
    int callCount = 0;
    pipe.addHandler("h1", [&](IntakeItem&) { ++callCount; return true; });
    pipe.addHandler("h2", [&](IntakeItem&) { ++callCount; return true; });
    REQUIRE(pipe.ingest("rock.png", IntakeSource::FileDrop));
    REQUIRE(callCount == 2);
}

TEST_CASE("FileIntakePipeline remove handler", "[Editor][S140]") {
    FileIntakePipeline pipe;
    bool called = false;
    pipe.addHandler("h1", [&](IntakeItem&) { called = true; return true; });
    REQUIRE(pipe.handlerCount() == 1u);
    REQUIRE(pipe.removeHandler("h1"));
    REQUIRE(pipe.handlerCount() == 0u);
    pipe.ingest("rock.png", IntakeSource::FileDrop);
    REQUIRE(!called);
}

TEST_CASE("FileIntakePipeline clearPending", "[Editor][S140]") {
    FileIntakePipeline pipe;
    pipe.ingest("a.png", IntakeSource::FileDrop);
    pipe.ingest("b.fbx", IntakeSource::FileDrop);
    REQUIRE(pipe.pendingCount() == 2u);
    pipe.clearPending();
    REQUIRE(pipe.pendingCount() == 0u);
    // ingestedCount preserved
    REQUIRE(pipe.ingestedCount() == 2u);
}

TEST_CASE("FileIntakePipeline findById", "[Editor][S140]") {
    FileIntakePipeline pipe;
    pipe.ingest("rock.png", IntakeSource::FileDrop);
    const auto* item = pipe.findById(1);
    REQUIRE(item != nullptr);
    REQUIRE(pipe.findById(99) == nullptr);
}

TEST_CASE("FileIntakePipeline duplicate handler name rejected", "[Editor][S140]") {
    FileIntakePipeline pipe;
    REQUIRE(pipe.addHandler("h1", [](IntakeItem&) { return true; }));
    REQUIRE(!pipe.addHandler("h1", [](IntakeItem&) { return true; }));
}

// ── DropTargetHandler ─────────────────────────────────────────────────────────

TEST_CASE("DropState names", "[Editor][S140]") {
    REQUIRE(std::string(dropStateName(DropState::Idle))      == "Idle");
    REQUIRE(std::string(dropStateName(DropState::DragOver))  == "DragOver");
    REQUIRE(std::string(dropStateName(DropState::DragLeave)) == "DragLeave");
    REQUIRE(std::string(dropStateName(DropState::Dropped))   == "Dropped");
    REQUIRE(std::string(dropStateName(DropState::Rejected))  == "Rejected");
}

TEST_CASE("DropEffect names", "[Editor][S140]") {
    REQUIRE(std::string(dropEffectName(DropEffect::None)) == "None");
    REQUIRE(std::string(dropEffectName(DropEffect::Copy)) == "Copy");
    REQUIRE(std::string(dropEffectName(DropEffect::Move)) == "Move");
    REQUIRE(std::string(dropEffectName(DropEffect::Link)) == "Link");
}

TEST_CASE("DropTargetHandler initial state", "[Editor][S140]") {
    DropTargetHandler h;
    REQUIRE(h.state()          == DropState::Idle);
    REQUIRE(h.defaultEffect()  == DropEffect::Copy);
    REQUIRE(!h.isDragActive());
    REQUIRE(h.enterCount()     == 0u);
    REQUIRE(h.dropCount()      == 0u);
}

TEST_CASE("DropTargetHandler drag enter and leave", "[Editor][S140]") {
    DropTargetHandler h;
    h.setAcceptUnknown(true);
    auto effect = h.onDragEnter({"rock.png", "model.fbx"});
    REQUIRE(effect == DropEffect::Copy);
    REQUIRE(h.state() == DropState::DragOver);
    REQUIRE(h.isDragActive());
    REQUIRE(h.hoveredPaths().size() == 2u);

    h.onDragLeave();
    REQUIRE(h.state() == DropState::DragLeave);
    REQUIRE(h.hoveredPaths().empty());
    REQUIRE(h.leaveCount() == 1u);
}

TEST_CASE("DropTargetHandler reject unknown file types", "[Editor][S140]") {
    DropTargetHandler h;
    // acceptUnknown defaults to false
    auto effect = h.onDragEnter({"file.xyz"});
    REQUIRE(effect == DropEffect::None);
    REQUIRE(h.state() == DropState::Rejected);
}

TEST_CASE("DropTargetHandler drop routes to pipeline", "[Editor][S140]") {
    FileIntakePipeline pipe;
    DropTargetHandler h(&pipe);

    size_t accepted = h.onDrop({"rock.png", "model.fbx"});
    REQUIRE(accepted == 2u);
    REQUIRE(h.state() == DropState::Dropped);
    REQUIRE(h.dropCount() == 1u);
    REQUIRE(h.totalDropped() == 2u);
    REQUIRE(pipe.pendingCount() == 2u);
}

TEST_CASE("DropTargetHandler drop without pipeline", "[Editor][S140]") {
    DropTargetHandler h;
    size_t accepted = h.onDrop({"rock.png"});
    REQUIRE(accepted == 0u);  // no pipeline bound
    REQUIRE(h.dropCount() == 1u);
}

TEST_CASE("DropTargetHandler reset clears state", "[Editor][S140]") {
    DropTargetHandler h;
    h.setAcceptUnknown(true);
    h.onDragEnter({"file.png"});
    h.reset();
    REQUIRE(h.state() == DropState::Idle);
    REQUIRE(h.hoveredPaths().empty());
}

TEST_CASE("DropTargetHandler bind pipeline after construction", "[Editor][S140]") {
    FileIntakePipeline pipe;
    DropTargetHandler h;
    REQUIRE(h.pipeline() == nullptr);
    h.bindPipeline(&pipe);
    REQUIRE(h.pipeline() == &pipe);
    h.onDrop({"a.png"});
    REQUIRE(pipe.pendingCount() == 1u);
}

// ── AssetImportQueue ──────────────────────────────────────────────────────────

TEST_CASE("ImportJobStatus names", "[Editor][S140]") {
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::Queued))      == "Queued");
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::Validating))  == "Validating");
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::Importing))   == "Importing");
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::PostProcess)) == "PostProcess");
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::Done))        == "Done");
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::Failed))      == "Failed");
    REQUIRE(std::string(importJobStatusName(ImportJobStatus::Cancelled))   == "Cancelled");
}

TEST_CASE("AssetImportQueue enqueue basic", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem item; item.id = 1; item.path = "rock.png";
    item.fileType = IntakeFileType::Texture; item.validated = true;
    REQUIRE(q.enqueue(item));
    REQUIRE(q.totalCount()   == 1u);
    REQUIRE(q.queuedCount()  == 1u);
    REQUIRE(q.totalEnqueued() == 1u);
}

TEST_CASE("AssetImportQueue advance through full lifecycle", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem item; item.id = 1; item.path = "rock.png"; item.validated = true;
    REQUIRE(q.enqueue(item));
    auto id = q.jobs().front().id;

    REQUIRE(q.advance(id));  // Queued → Validating
    REQUIRE(q.find(id)->status == ImportJobStatus::Validating);

    REQUIRE(q.advance(id));  // Validating → Importing
    REQUIRE(q.find(id)->status == ImportJobStatus::Importing);

    REQUIRE(q.advance(id, 1.f));  // Importing → PostProcess (progress=1)
    REQUIRE(q.find(id)->status == ImportJobStatus::PostProcess);

    REQUIRE(q.advance(id));  // PostProcess → Done
    REQUIRE(q.find(id)->status == ImportJobStatus::Done);
    REQUIRE(q.totalCompleted() == 1u);
    REQUIRE(!q.find(id)->outputPath.empty());
}

TEST_CASE("AssetImportQueue import progress delta", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem item; item.id = 1; item.path = "mesh.fbx"; item.validated = true;
    q.enqueue(item);
    auto id = q.jobs().front().id;
    q.advance(id); q.advance(id); // → Importing
    q.advance(id, 0.4f); // progress 0.4, still Importing
    REQUIRE(q.find(id)->progress == Catch::Approx(0.4f));
    REQUIRE(q.find(id)->status == ImportJobStatus::Importing);
    q.advance(id, 0.6f); // progress 1.0 → PostProcess
    REQUIRE(q.find(id)->status == ImportJobStatus::PostProcess);
}

TEST_CASE("AssetImportQueue cancel job", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem item; item.id = 1; item.path = "a.png"; item.validated = true;
    q.enqueue(item);
    auto id = q.jobs().front().id;
    REQUIRE(q.cancel(id));
    REQUIRE(q.find(id)->status == ImportJobStatus::Cancelled);
    REQUIRE(q.totalCancelled() == 1u);
}

TEST_CASE("AssetImportQueue fail job", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem item; item.id = 1; item.path = "corrupt.fbx"; item.validated = true;
    q.enqueue(item);
    auto id = q.jobs().front().id;
    q.advance(id); // Queued → Validating
    REQUIRE(q.failJob(id, "Format not supported"));
    REQUIRE(q.find(id)->status == ImportJobStatus::Failed);
    REQUIRE(q.find(id)->errorMsg == "Format not supported");
    REQUIRE(q.totalFailed() == 1u);
}

TEST_CASE("AssetImportQueue priority ordering", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem a; a.id = 1; a.path = "low.png";  a.validated = true;
    IntakeItem b; b.id = 2; b.path = "high.png"; b.validated = true;
    q.enqueue(a, 0);
    q.enqueue(b, 10);  // higher priority
    REQUIRE(q.jobs().front().intakeItem.path == "high.png");
}

TEST_CASE("AssetImportQueue enqueueFromPipeline", "[Editor][S140]") {
    FileIntakePipeline pipe;
    pipe.ingest("rock.png",  IntakeSource::FileDrop);
    pipe.ingest("model.fbx", IntakeSource::FileDrop);

    AssetImportQueue q;
    size_t count = q.enqueueFromPipeline(pipe);
    REQUIRE(count == 2u);
    REQUIRE(q.totalCount() == 2u);
}

TEST_CASE("AssetImportQueue clearFinished removes done/failed/cancelled", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem a; a.id = 1; a.path = "a.png"; a.validated = true;
    IntakeItem b; b.id = 2; b.path = "b.png"; b.validated = true;
    q.enqueue(a); q.enqueue(b);
    auto id_a = q.jobs()[0].id;
    auto id_b = q.jobs()[1].id;

    // Advance 'a' to done
    q.advance(id_a); q.advance(id_a); q.advance(id_a, 1.f); q.advance(id_a);
    // Cancel 'b'
    q.cancel(id_b);

    q.clearFinished();
    REQUIRE(q.totalCount() == 0u);
}

TEST_CASE("AssetImportQueue onComplete callback", "[Editor][S140]") {
    AssetImportQueue q;
    bool called = false;
    q.setOnComplete([&](const ImportJob& j) {
        called = true;
        REQUIRE(j.isDone());
    });

    IntakeItem item; item.id = 1; item.path = "a.png"; item.validated = true;
    q.enqueue(item);
    auto id = q.jobs().front().id;
    q.advance(id); q.advance(id); q.advance(id, 1.f); q.advance(id);
    REQUIRE(called);
}

TEST_CASE("AssetImportQueue startNext helper", "[Editor][S140]") {
    AssetImportQueue q;
    IntakeItem a; a.id = 1; a.path = "a.png"; a.validated = true;
    IntakeItem b; b.id = 2; b.path = "b.png"; b.validated = true;
    q.enqueue(a); q.enqueue(b);

    REQUIRE(q.startNext());
    REQUIRE(q.activeCount() == 1u);
    REQUIRE(q.queuedCount() == 1u);
}
