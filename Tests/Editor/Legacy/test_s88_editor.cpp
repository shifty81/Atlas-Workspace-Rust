#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SequenceRecorder.h"
#include "NF/Editor/CameraPathEditor.h"
#include "NF/Editor/CinematicsEditor.h"

using namespace NF;

// ── S88: CinematicsEditor + CameraPathEditor + SequenceRecorder ──

// ── CinematicsEditor ─────────────────────────────────────────────

TEST_CASE("CinematicShotType names are correct", "[Editor][S88]") {
    REQUIRE(std::string(cinematicShotTypeName(CinematicShotType::Cut))       == "Cut");
    REQUIRE(std::string(cinematicShotTypeName(CinematicShotType::Dissolve))  == "Dissolve");
    REQUIRE(std::string(cinematicShotTypeName(CinematicShotType::FadeIn))    == "FadeIn");
    REQUIRE(std::string(cinematicShotTypeName(CinematicShotType::FadeOut))   == "FadeOut");
    REQUIRE(std::string(cinematicShotTypeName(CinematicShotType::Wipe))      == "Wipe");
    REQUIRE(std::string(cinematicShotTypeName(CinematicShotType::CrossFade)) == "CrossFade");
}

TEST_CASE("CinematicPlayMode names are correct", "[Editor][S88]") {
    REQUIRE(std::string(cinematicPlayModeName(CinematicPlayMode::Once))     == "Once");
    REQUIRE(std::string(cinematicPlayModeName(CinematicPlayMode::Loop))     == "Loop");
    REQUIRE(std::string(cinematicPlayModeName(CinematicPlayMode::PingPong)) == "PingPong");
    REQUIRE(std::string(cinematicPlayModeName(CinematicPlayMode::Manual))   == "Manual");
}

TEST_CASE("CinematicTrackBind names are correct", "[Editor][S88]") {
    REQUIRE(std::string(cinematicTrackBindName(CinematicTrackBind::Camera))      == "Camera");
    REQUIRE(std::string(cinematicTrackBindName(CinematicTrackBind::Actor))       == "Actor");
    REQUIRE(std::string(cinematicTrackBindName(CinematicTrackBind::Light))       == "Light");
    REQUIRE(std::string(cinematicTrackBindName(CinematicTrackBind::Audio))       == "Audio");
    REQUIRE(std::string(cinematicTrackBindName(CinematicTrackBind::Subtitle))    == "Subtitle");
    REQUIRE(std::string(cinematicTrackBindName(CinematicTrackBind::PostProcess)) == "PostProcess");
}

TEST_CASE("CinematicShot stores properties", "[Editor][S88]") {
    CinematicShot shot("Opening", CinematicShotType::FadeIn, 3.0f);
    shot.setStartTime(0.0f);
    shot.setBoundCamera("MainCam");
    REQUIRE(shot.name()        == "Opening");
    REQUIRE(shot.type()        == CinematicShotType::FadeIn);
    REQUIRE(shot.duration()    == 3.0f);
    REQUIRE(shot.boundCamera() == "MainCam");
    REQUIRE(shot.isEnabled());
}

TEST_CASE("CinematicsEditor addShot and findShot", "[Editor][S88]") {
    CinematicsEditor editor;
    CinematicShot s1("S1", CinematicShotType::Cut, 2.0f);
    CinematicShot s2("S2", CinematicShotType::Dissolve, 1.5f);
    REQUIRE(editor.addShot(s1));
    REQUIRE(editor.addShot(s2));
    REQUIRE(editor.shotCount() == 2);
    REQUIRE(editor.findShot("S1") != nullptr);
}

TEST_CASE("CinematicsEditor rejects duplicate shot name", "[Editor][S88]") {
    CinematicsEditor editor;
    CinematicShot s("Dup", CinematicShotType::Cut, 1.0f);
    editor.addShot(s);
    REQUIRE_FALSE(editor.addShot(s));
}

TEST_CASE("CinematicsEditor setActiveShot remove and playhead", "[Editor][S88]") {
    CinematicsEditor editor;
    CinematicShot s("Mid", CinematicShotType::Wipe, 2.5f);
    editor.addShot(s);
    REQUIRE(editor.setActiveShot("Mid"));
    REQUIRE(editor.activeShot() == "Mid");
    editor.setPlayhead(1.0f);
    editor.setPlaying(true);
    REQUIRE(editor.isPlaying());
    REQUIRE(editor.playhead() == 1.0f);
}

TEST_CASE("CinematicsEditor totalDuration and countByType", "[Editor][S88]") {
    CinematicsEditor editor;
    CinematicShot s1("A", CinematicShotType::Cut,  2.0f);
    CinematicShot s2("B", CinematicShotType::Cut,  3.0f);
    CinematicShot s3("C", CinematicShotType::Wipe, 1.5f);
    editor.addShot(s1); editor.addShot(s2); editor.addShot(s3);
    REQUIRE(editor.totalDuration()                    == 6.5f);
    REQUIRE(editor.countByType(CinematicShotType::Cut)  == 2);
    REQUIRE(editor.countByType(CinematicShotType::Wipe) == 1);
}

TEST_CASE("CinematicsEditor MAX_SHOTS is 256", "[Editor][S88]") {
    REQUIRE(CinematicsEditor::MAX_SHOTS == 256);
}

// ── CameraPathEditor ─────────────────────────────────────────────

TEST_CASE("CameraPathInterp names are correct", "[Editor][S88]") {
    REQUIRE(std::string(cameraPathInterpName(CameraPathInterp::Linear))     == "Linear");
    REQUIRE(std::string(cameraPathInterpName(CameraPathInterp::Bezier))     == "Bezier");
    REQUIRE(std::string(cameraPathInterpName(CameraPathInterp::CatmullRom)) == "CatmullRom");
    REQUIRE(std::string(cameraPathInterpName(CameraPathInterp::Hermite))    == "Hermite");
    REQUIRE(std::string(cameraPathInterpName(CameraPathInterp::Step))       == "Step");
}

TEST_CASE("CameraLookAtMode names are correct", "[Editor][S88]") {
    REQUIRE(std::string(cameraLookAtModeName(CameraLookAtMode::Free))   == "Free");
    REQUIRE(std::string(cameraLookAtModeName(CameraLookAtMode::Target)) == "Target");
    REQUIRE(std::string(cameraLookAtModeName(CameraLookAtMode::Path))   == "Path");
    REQUIRE(std::string(cameraLookAtModeName(CameraLookAtMode::Orbit))  == "Orbit");
    REQUIRE(std::string(cameraLookAtModeName(CameraLookAtMode::Fixed))  == "Fixed");
}

TEST_CASE("CameraFOVCurve names are correct", "[Editor][S88]") {
    REQUIRE(std::string(cameraFOVCurveName(CameraFOVCurve::Constant)) == "Constant");
    REQUIRE(std::string(cameraFOVCurveName(CameraFOVCurve::Linear))   == "Linear");
    REQUIRE(std::string(cameraFOVCurveName(CameraFOVCurve::EaseIn))   == "EaseIn");
    REQUIRE(std::string(cameraFOVCurveName(CameraFOVCurve::EaseOut))  == "EaseOut");
    REQUIRE(std::string(cameraFOVCurveName(CameraFOVCurve::Custom))   == "Custom");
}

TEST_CASE("CameraKeyframe stores time and interp", "[Editor][S88]") {
    CameraKeyframe kf(2.0f);
    kf.setInterp(CameraPathInterp::CatmullRom);
    kf.setFOV(75.0f);
    kf.setRoll(5.0f);
    REQUIRE(kf.time()   == 2.0f);
    REQUIRE(kf.fov()    == 75.0f);
    REQUIRE(kf.roll()   == 5.0f);
    REQUIRE(kf.interp() == CameraPathInterp::CatmullRom);
}

TEST_CASE("CameraPathEditor addKeyframe and findKeyframe", "[Editor][S88]") {
    CameraPathEditor editor;
    CameraKeyframe kf0(0.0f);
    CameraKeyframe kf1(1.0f);
    REQUIRE(editor.addKeyframe(kf0));
    REQUIRE(editor.addKeyframe(kf1));
    REQUIRE(editor.keyframeCount() == 2);
    REQUIRE(editor.findKeyframe(0.0f) != nullptr);
}

TEST_CASE("CameraPathEditor rejects duplicate time", "[Editor][S88]") {
    CameraPathEditor editor;
    CameraKeyframe kf(1.0f);
    editor.addKeyframe(kf);
    REQUIRE_FALSE(editor.addKeyframe(kf));
}

TEST_CASE("CameraPathEditor settings", "[Editor][S88]") {
    CameraPathEditor editor;
    editor.setLookAtMode(CameraLookAtMode::Target);
    editor.setFOVCurve(CameraFOVCurve::EaseIn);
    editor.setLooped(true);
    editor.setPreviewEnabled(true);
    REQUIRE(editor.lookAtMode()       == CameraLookAtMode::Target);
    REQUIRE(editor.fovCurve()         == CameraFOVCurve::EaseIn);
    REQUIRE(editor.isLooped());
    REQUIRE(editor.isPreviewEnabled());
}

TEST_CASE("CameraPathEditor MAX_KEYFRAMES is 512", "[Editor][S88]") {
    REQUIRE(CameraPathEditor::MAX_KEYFRAMES == 512);
}

// ── SequenceRecorder ─────────────────────────────────────────────

TEST_CASE("RecordTarget names are correct", "[Editor][S88]") {
    REQUIRE(std::string(recordTargetName(RecordTarget::Transform))  == "Transform");
    REQUIRE(std::string(recordTargetName(RecordTarget::Animation))  == "Animation");
    REQUIRE(std::string(recordTargetName(RecordTarget::Physics))    == "Physics");
    REQUIRE(std::string(recordTargetName(RecordTarget::Audio))      == "Audio");
    REQUIRE(std::string(recordTargetName(RecordTarget::Custom))     == "Custom");
    REQUIRE(std::string(recordTargetName(RecordTarget::All))        == "All");
}

TEST_CASE("RecordQuality names are correct", "[Editor][S88]") {
    REQUIRE(std::string(recordQualityName(RecordQuality::Draft))   == "Draft");
    REQUIRE(std::string(recordQualityName(RecordQuality::Preview)) == "Preview");
    REQUIRE(std::string(recordQualityName(RecordQuality::High))    == "High");
    REQUIRE(std::string(recordQualityName(RecordQuality::Master))  == "Master");
}

TEST_CASE("RecorderState names are correct", "[Editor][S88]") {
    REQUIRE(std::string(recorderStateName(RecorderState::Idle))      == "Idle");
    REQUIRE(std::string(recorderStateName(RecorderState::Recording)) == "Recording");
    REQUIRE(std::string(recorderStateName(RecorderState::Paused))    == "Paused");
    REQUIRE(std::string(recorderStateName(RecorderState::Stopped))   == "Stopped");
    REQUIRE(std::string(recorderStateName(RecorderState::Exporting)) == "Exporting");
}

TEST_CASE("SequenceRecorderPanel addTrack and countByTarget", "[Editor][S88]") {
    SequenceRecorderPanel panel;
    RecordTrackConfig t1("Player", RecordTarget::Transform);
    RecordTrackConfig t2("Camera", RecordTarget::Animation);
    RecordTrackConfig t3("Rock",   RecordTarget::Physics);
    panel.addTrack(t1); panel.addTrack(t2); panel.addTrack(t3);
    REQUIRE(panel.trackCount() == 3);
    REQUIRE(panel.countByTarget(RecordTarget::Transform) == 1);
}

TEST_CASE("SequenceRecorderPanel rejects duplicate actor+target", "[Editor][S88]") {
    SequenceRecorderPanel panel;
    RecordTrackConfig t("Actor", RecordTarget::Animation);
    panel.addTrack(t);
    REQUIRE_FALSE(panel.addTrack(t));
}

TEST_CASE("SequenceRecorderPanel startRecord pauseRecord stopRecord", "[Editor][S88]") {
    SequenceRecorderPanel panel;
    REQUIRE(panel.state() == RecorderState::Idle);
    panel.startRecord();
    REQUIRE(panel.isRecording());
    panel.pauseRecord();
    REQUIRE(panel.isPaused());
    panel.stopRecord();
    REQUIRE(panel.state() == RecorderState::Stopped);
}

TEST_CASE("SequenceRecorderPanel quality and outputName", "[Editor][S88]") {
    SequenceRecorderPanel panel;
    panel.setQuality(RecordQuality::High);
    panel.setOutputName("Intro_Capture");
    REQUIRE(panel.quality()    == RecordQuality::High);
    REQUIRE(panel.outputName() == "Intro_Capture");
}

TEST_CASE("SequenceRecorderPanel MAX_TRACKS is 64", "[Editor][S88]") {
    REQUIRE(SequenceRecorderPanel::MAX_TRACKS == 64);
}
