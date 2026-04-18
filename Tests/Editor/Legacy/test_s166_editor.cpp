// S166 editor tests: VirtualProductionEditorV1, MotionCaptureEditorV1, VirtualCameraEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/VirtualProductionEditorV1.h"
#include "NF/Editor/MotionCaptureEditorV1.h"
#include "NF/Editor/VirtualCameraEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── VirtualProductionEditorV1 ────────────────────────────────────────────────

TEST_CASE("Vpv1Take validity", "[Editor][S166]") {
    Vpv1Take t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "OpeningShot";
    REQUIRE(t.isValid());
}

TEST_CASE("VirtualProductionEditorV1 addTake and takeCount", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    REQUIRE(vpe.takeCount() == 0);
    Vpv1Take t; t.id = 1; t.name = "TakeA";
    REQUIRE(vpe.addTake(t));
    REQUIRE(vpe.takeCount() == 1);
}

TEST_CASE("VirtualProductionEditorV1 addTake invalid fails", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    REQUIRE(!vpe.addTake(Vpv1Take{}));
}

TEST_CASE("VirtualProductionEditorV1 addTake duplicate fails", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t; t.id = 1; t.name = "T";
    vpe.addTake(t);
    REQUIRE(!vpe.addTake(t));
}

TEST_CASE("VirtualProductionEditorV1 removeTake", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t; t.id = 2; t.name = "T2";
    vpe.addTake(t);
    REQUIRE(vpe.removeTake(2));
    REQUIRE(vpe.takeCount() == 0);
    REQUIRE(!vpe.removeTake(2));
}

TEST_CASE("VirtualProductionEditorV1 setActive", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t; t.id = 3; t.name = "T3";
    vpe.addTake(t);
    REQUIRE(vpe.setActive(3));
    REQUIRE(vpe.activeId() == 3);
    REQUIRE(!vpe.setActive(99));
}

TEST_CASE("VirtualProductionEditorV1 setStatus and completeCount", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t1; t1.id = 1; t1.name = "A";
    Vpv1Take t2; t2.id = 2; t2.name = "B";
    vpe.addTake(t1); vpe.addTake(t2);
    vpe.setStatus(1, Vpv1TakeStatus::Complete);
    vpe.setStatus(2, Vpv1TakeStatus::Complete);
    REQUIRE(vpe.completeCount() == 2);
}

TEST_CASE("VirtualProductionEditorV1 setFeatured and featuredCount", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t; t.id = 1; t.name = "Hero";
    vpe.addTake(t);
    REQUIRE(vpe.setFeatured(1, true));
    REQUIRE(vpe.featuredCount() == 1);
}

TEST_CASE("VirtualProductionEditorV1 countByStage", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t1; t1.id = 1; t1.name = "A"; t1.stageType = Vpv1StageType::VirtualStage;
    Vpv1Take t2; t2.id = 2; t2.name = "B"; t2.stageType = Vpv1StageType::SoundStage;
    vpe.addTake(t1); vpe.addTake(t2);
    REQUIRE(vpe.countByStage(Vpv1StageType::VirtualStage) == 1);
    REQUIRE(vpe.countByStage(Vpv1StageType::SoundStage)   == 1);
}

TEST_CASE("VirtualProductionEditorV1 setOutputFormat", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t; t.id = 1; t.name = "A";
    vpe.addTake(t);
    REQUIRE(vpe.setOutputFormat(1, Vpv1OutputFormat::Film4K));
    REQUIRE(vpe.findTake(1)->outputFormat == Vpv1OutputFormat::Film4K);
}

TEST_CASE("VirtualProductionEditorV1 onChange callback", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    uint64_t notified = 0;
    vpe.setOnChange([&](uint64_t id) { notified = id; });
    Vpv1Take t; t.id = 5; t.name = "X";
    vpe.addTake(t);
    vpe.setStatus(5, Vpv1TakeStatus::Active);
    REQUIRE(notified == 5);
}

TEST_CASE("vpv1StageTypeName covers all values", "[Editor][S166]") {
    REQUIRE(std::string(vpv1StageTypeName(Vpv1StageType::SoundStage))    == "SoundStage");
    REQUIRE(std::string(vpv1StageTypeName(Vpv1StageType::Greenscreen))   == "Greenscreen");
}

TEST_CASE("Vpv1Take isActive and isComplete", "[Editor][S166]") {
    Vpv1Take t; t.id = 1; t.name = "T";
    t.status = Vpv1TakeStatus::Active;
    REQUIRE(t.isActive());
    t.status = Vpv1TakeStatus::Complete;
    REQUIRE(t.isComplete());
}

TEST_CASE("VirtualProductionEditorV1 removeTake clears activeId", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t; t.id = 1; t.name = "X";
    vpe.addTake(t);
    vpe.setActive(1);
    REQUIRE(vpe.activeId() == 1);
    vpe.removeTake(1);
    REQUIRE(vpe.activeId() == 0);
}

TEST_CASE("VirtualProductionEditorV1 countByStatus", "[Editor][S166]") {
    VirtualProductionEditorV1 vpe;
    Vpv1Take t1; t1.id = 1; t1.name = "A";
    Vpv1Take t2; t2.id = 2; t2.name = "B"; t2.status = Vpv1TakeStatus::Hold;
    vpe.addTake(t1); vpe.addTake(t2);
    REQUIRE(vpe.countByStatus(Vpv1TakeStatus::Pending) == 1);
    REQUIRE(vpe.countByStatus(Vpv1TakeStatus::Hold)    == 1);
}

// ── MotionCaptureEditorV1 ────────────────────────────────────────────────────

TEST_CASE("Mcv1Actor validity", "[Editor][S166]") {
    Mcv1Actor a;
    REQUIRE(!a.isValid());
    a.id = 1; a.name = "Performer01";
    REQUIRE(a.isValid());
}

TEST_CASE("MotionCaptureEditorV1 addActor and actorCount", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    REQUIRE(mc.actorCount() == 0);
    Mcv1Actor a; a.id = 1; a.name = "Actor01";
    REQUIRE(mc.addActor(a));
    REQUIRE(mc.actorCount() == 1);
}

TEST_CASE("MotionCaptureEditorV1 addActor invalid fails", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    REQUIRE(!mc.addActor(Mcv1Actor{}));
}

TEST_CASE("MotionCaptureEditorV1 addActor duplicate fails", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a; a.id = 1; a.name = "A";
    mc.addActor(a);
    REQUIRE(!mc.addActor(a));
}

TEST_CASE("MotionCaptureEditorV1 removeActor", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a; a.id = 2; a.name = "B";
    mc.addActor(a);
    REQUIRE(mc.removeActor(2));
    REQUIRE(mc.actorCount() == 0);
    REQUIRE(!mc.removeActor(2));
}

TEST_CASE("MotionCaptureEditorV1 setState recordingCount", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a; a.id = 1; a.name = "A";
    mc.addActor(a);
    REQUIRE(mc.setState(1, Mcv1SessionState::Recording));
    REQUIRE(mc.recordingCount() == 1);
}

TEST_CASE("MotionCaptureEditorV1 setState bakedCount", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a; a.id = 1; a.name = "A";
    mc.addActor(a);
    mc.setState(1, Mcv1SessionState::Baked);
    REQUIRE(mc.bakedCount() == 1);
}

TEST_CASE("MotionCaptureEditorV1 setCalibrated calibratedCount", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a; a.id = 1; a.name = "A";
    mc.addActor(a);
    REQUIRE(mc.setCalibrated(1, true));
    REQUIRE(mc.calibratedCount() == 1);
}

TEST_CASE("MotionCaptureEditorV1 addBoneMapping and removeBoneMapping", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a; a.id = 1; a.name = "A";
    mc.addActor(a);
    Mcv1BoneMapping bm; bm.sourceBone = "Spine"; bm.targetBone = "spine_01";
    REQUIRE(mc.addBoneMapping(1, bm));
    REQUIRE(mc.findActor(1)->boneMappings.size() == 1);
    REQUIRE(mc.removeBoneMapping(1, "Spine"));
    REQUIRE(mc.findActor(1)->boneMappings.empty());
}

TEST_CASE("Mcv1BoneMapping invalid when empty", "[Editor][S166]") {
    Mcv1BoneMapping bm;
    REQUIRE(!bm.isValid());
    bm.sourceBone = "S"; bm.targetBone = "T";
    REQUIRE(bm.isValid());
}

TEST_CASE("MotionCaptureEditorV1 countByRole", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    Mcv1Actor a1; a1.id = 1; a1.name = "A"; a1.role = Mcv1ActorRole::Lead;
    Mcv1Actor a2; a2.id = 2; a2.name = "B"; a2.role = Mcv1ActorRole::Background;
    mc.addActor(a1); mc.addActor(a2);
    REQUIRE(mc.countByRole(Mcv1ActorRole::Lead)       == 1);
    REQUIRE(mc.countByRole(Mcv1ActorRole::Background) == 1);
}

TEST_CASE("mcv1SessionStateName covers all values", "[Editor][S166]") {
    REQUIRE(std::string(mcv1SessionStateName(Mcv1SessionState::Idle))        == "Idle");
    REQUIRE(std::string(mcv1SessionStateName(Mcv1SessionState::Baked))       == "Baked");
}

TEST_CASE("MotionCaptureEditorV1 onChange callback", "[Editor][S166]") {
    MotionCaptureEditorV1 mc;
    uint64_t notified = 0;
    mc.setOnChange([&](uint64_t id) { notified = id; });
    Mcv1Actor a; a.id = 7; a.name = "G";
    mc.addActor(a);
    mc.setState(7, Mcv1SessionState::Recording);
    REQUIRE(notified == 7);
}

// ── VirtualCameraEditorV1 ────────────────────────────────────────────────────

TEST_CASE("Vcv1CameraRig validity", "[Editor][S166]") {
    Vcv1CameraRig r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "MainCam"; r.focalLength = 35.f;
    REQUIRE(r.isValid());
}

TEST_CASE("Vcv1CameraRig zero focalLength invalid", "[Editor][S166]") {
    Vcv1CameraRig r; r.id = 1; r.name = "X"; r.focalLength = 0.f;
    REQUIRE(!r.isValid());
}

TEST_CASE("VirtualCameraEditorV1 addRig and rigCount", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    REQUIRE(vce.rigCount() == 0);
    Vcv1CameraRig r; r.id = 1; r.name = "Rig1"; r.focalLength = 50.f;
    REQUIRE(vce.addRig(r));
    REQUIRE(vce.rigCount() == 1);
}

TEST_CASE("VirtualCameraEditorV1 addRig invalid fails", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    REQUIRE(!vce.addRig(Vcv1CameraRig{}));
}

TEST_CASE("VirtualCameraEditorV1 addRig duplicate fails", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 1; r.name = "A"; r.focalLength = 50.f;
    vce.addRig(r);
    REQUIRE(!vce.addRig(r));
}

TEST_CASE("VirtualCameraEditorV1 removeRig", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 2; r.name = "B"; r.focalLength = 50.f;
    vce.addRig(r);
    REQUIRE(vce.removeRig(2));
    REQUIRE(vce.rigCount() == 0);
    REQUIRE(!vce.removeRig(2));
}

TEST_CASE("VirtualCameraEditorV1 setActive", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 1; r.name = "A"; r.focalLength = 50.f;
    vce.addRig(r);
    REQUIRE(vce.setActive(1));
    REQUIRE(vce.activeId() == 1);
    REQUIRE(!vce.setActive(99));
}

TEST_CASE("VirtualCameraEditorV1 setState streamingCount", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 1; r.name = "A"; r.focalLength = 50.f;
    vce.addRig(r);
    REQUIRE(vce.setState(1, Vcv1CameraState::Streaming));
    REQUIRE(vce.streamingCount() == 1);
}

TEST_CASE("VirtualCameraEditorV1 setLive liveCount", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 1; r.name = "A"; r.focalLength = 50.f;
    vce.addRig(r);
    REQUIRE(vce.setLive(1, true));
    REQUIRE(vce.liveCount() == 1);
}

TEST_CASE("VirtualCameraEditorV1 setFocalLength validation", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 1; r.name = "A"; r.focalLength = 50.f;
    vce.addRig(r);
    REQUIRE(!vce.setFocalLength(1, 0.f));
    REQUIRE(vce.setFocalLength(1, 85.f));
    REQUIRE(vce.findRig(1)->focalLength == Approx(85.f));
}

TEST_CASE("VirtualCameraEditorV1 setLensProfile countByLens", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r1; r1.id = 1; r1.name = "A"; r1.focalLength = 50.f; r1.lensProfile = Vcv1LensProfile::Wide;
    Vcv1CameraRig r2; r2.id = 2; r2.name = "B"; r2.focalLength = 85.f; r2.lensProfile = Vcv1LensProfile::Telephoto;
    vce.addRig(r1); vce.addRig(r2);
    REQUIRE(vce.countByLens(Vcv1LensProfile::Wide)      == 1);
    REQUIRE(vce.countByLens(Vcv1LensProfile::Telephoto) == 1);
}

TEST_CASE("VirtualCameraEditorV1 countByMode", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r1; r1.id = 1; r1.name = "A"; r1.focalLength = 50.f; r1.trackingMode = Vcv1TrackingMode::Orbit;
    Vcv1CameraRig r2; r2.id = 2; r2.name = "B"; r2.focalLength = 50.f; r2.trackingMode = Vcv1TrackingMode::Rail;
    vce.addRig(r1); vce.addRig(r2);
    REQUIRE(vce.countByMode(Vcv1TrackingMode::Orbit) == 1);
    REQUIRE(vce.countByMode(Vcv1TrackingMode::Rail)  == 1);
}

TEST_CASE("vcv1LensProfileName covers all values", "[Editor][S166]") {
    REQUIRE(std::string(vcv1LensProfileName(Vcv1LensProfile::Anamorphic)) == "Anamorphic");
    REQUIRE(std::string(vcv1LensProfileName(Vcv1LensProfile::Fisheye))    == "Fisheye");
}

TEST_CASE("VirtualCameraEditorV1 removeRig clears activeId", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    Vcv1CameraRig r; r.id = 1; r.name = "X"; r.focalLength = 50.f;
    vce.addRig(r);
    vce.setActive(1);
    vce.removeRig(1);
    REQUIRE(vce.activeId() == 0);
}

TEST_CASE("VirtualCameraEditorV1 onChange callback", "[Editor][S166]") {
    VirtualCameraEditorV1 vce;
    uint64_t notified = 0;
    vce.setOnChange([&](uint64_t id) { notified = id; });
    Vcv1CameraRig r; r.id = 9; r.name = "Z"; r.focalLength = 50.f;
    vce.addRig(r);
    vce.setState(9, Vcv1CameraState::Active);
    REQUIRE(notified == 9);
}
