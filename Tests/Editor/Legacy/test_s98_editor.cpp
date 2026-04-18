// S98 editor tests: TimelineSequencer, CutsceneDirector, ActorDirector
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ActorDirector.h"
#include "NF/Editor/CutsceneDirector.h"
#include "NF/Editor/TimelineSequencer.h"

using namespace NF;

// ── TimelineSequencer ────────────────────────────────────────────────────────

TEST_CASE("SeqTrackKind names", "[Editor][S98]") {
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Event))     == "Event");
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Animation)) == "Animation");
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Audio))     == "Audio");
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Property))  == "Property");
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Camera))    == "Camera");
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Script))    == "Script");
    REQUIRE(std::string(seqTrackKindName(SeqTrackKind::Subtitle))  == "Subtitle");
}

TEST_CASE("SeqPlaybackState names", "[Editor][S98]") {
    REQUIRE(std::string(seqPlaybackStateName(SeqPlaybackState::Idle))      == "Idle");
    REQUIRE(std::string(seqPlaybackStateName(SeqPlaybackState::Playing))   == "Playing");
    REQUIRE(std::string(seqPlaybackStateName(SeqPlaybackState::Paused))    == "Paused");
    REQUIRE(std::string(seqPlaybackStateName(SeqPlaybackState::Stopped))   == "Stopped");
    REQUIRE(std::string(seqPlaybackStateName(SeqPlaybackState::Scrubbing)) == "Scrubbing");
}

TEST_CASE("SeqLoopMode names", "[Editor][S98]") {
    REQUIRE(std::string(seqLoopModeName(SeqLoopMode::None))     == "None");
    REQUIRE(std::string(seqLoopModeName(SeqLoopMode::Loop))     == "Loop");
    REQUIRE(std::string(seqLoopModeName(SeqLoopMode::PingPong)) == "PingPong");
    REQUIRE(std::string(seqLoopModeName(SeqLoopMode::Hold))     == "Hold");
}

TEST_CASE("SeqTrack defaults", "[Editor][S98]") {
    SeqTrack t("anim_track", SeqTrackKind::Animation);
    REQUIRE(t.name()     == "anim_track");
    REQUIRE(t.kind()     == SeqTrackKind::Animation);
    REQUIRE(t.isEnabled());
    REQUIRE(!t.isMuted());
    REQUIRE(!t.isLocked());
    REQUIRE(t.keyCount() == 0u);
}

TEST_CASE("SeqTrack mutation", "[Editor][S98]") {
    SeqTrack t("audio_fx", SeqTrackKind::Audio);
    t.setMuted(true);
    t.setLocked(true);
    t.setEnabled(false);
    t.setKeyCount(12);
    REQUIRE(t.isMuted());
    REQUIRE(t.isLocked());
    REQUIRE(!t.isEnabled());
    REQUIRE(t.keyCount() == 12u);
}

TEST_CASE("TimelineSequencer add/remove track", "[Editor][S98]") {
    TimelineSequencer seq;
    SeqTrack t("cam", SeqTrackKind::Camera);
    REQUIRE(seq.addTrack(t));
    REQUIRE(seq.trackCount() == 1u);
    REQUIRE(!seq.addTrack(t));
    REQUIRE(seq.removeTrack("cam"));
    REQUIRE(seq.trackCount() == 0u);
}

TEST_CASE("TimelineSequencer state and playback", "[Editor][S98]") {
    TimelineSequencer seq;
    REQUIRE(seq.state() == SeqPlaybackState::Idle);
    REQUIRE(!seq.isPlaying());
    seq.setState(SeqPlaybackState::Playing);
    REQUIRE(seq.isPlaying());
    seq.setCurrentTime(2.5f);
    REQUIRE(seq.currentTime() == 2.5f);
    seq.setDuration(10.0f);
    REQUIRE(seq.duration() == 10.0f);
    seq.setLoopMode(SeqLoopMode::Loop);
    REQUIRE(seq.loopMode() == SeqLoopMode::Loop);
}

TEST_CASE("TimelineSequencer track counts", "[Editor][S98]") {
    TimelineSequencer seq;
    SeqTrack t1("a", SeqTrackKind::Animation);
    SeqTrack t2("b", SeqTrackKind::Audio); t2.setMuted(true);
    SeqTrack t3("c", SeqTrackKind::Animation);
    seq.addTrack(t1); seq.addTrack(t2); seq.addTrack(t3);
    REQUIRE(seq.trackCount()                           == 3u);
    REQUIRE(seq.mutedTrackCount()                      == 1u);
    REQUIRE(seq.countByKind(SeqTrackKind::Animation)   == 2u);
    REQUIRE(seq.countByKind(SeqTrackKind::Audio)       == 1u);
}

// ── CutsceneDirector ─────────────────────────────────────────────────────────

TEST_CASE("CutsceneShotCut names", "[Editor][S98]") {
    REQUIRE(std::string(cutsceneShotCutName(CutsceneShotCut::Cut))     == "Cut");
    REQUIRE(std::string(cutsceneShotCutName(CutsceneShotCut::Dissolve))== "Dissolve");
    REQUIRE(std::string(cutsceneShotCutName(CutsceneShotCut::Fade))    == "Fade");
    REQUIRE(std::string(cutsceneShotCutName(CutsceneShotCut::Wipe))    == "Wipe");
    REQUIRE(std::string(cutsceneShotCutName(CutsceneShotCut::ZoomIn))  == "ZoomIn");
    REQUIRE(std::string(cutsceneShotCutName(CutsceneShotCut::ZoomOut)) == "ZoomOut");
}

TEST_CASE("CutsceneDirectorState names", "[Editor][S98]") {
    REQUIRE(std::string(cutsceneDirectorStateName(CutsceneDirectorState::Idle))       == "Idle");
    REQUIRE(std::string(cutsceneDirectorStateName(CutsceneDirectorState::Previewing)) == "Previewing");
    REQUIRE(std::string(cutsceneDirectorStateName(CutsceneDirectorState::Recording))  == "Recording");
    REQUIRE(std::string(cutsceneDirectorStateName(CutsceneDirectorState::Exporting))  == "Exporting");
    REQUIRE(std::string(cutsceneDirectorStateName(CutsceneDirectorState::Done))       == "Done");
}

TEST_CASE("CutsceneOutputFormat names", "[Editor][S98]") {
    REQUIRE(std::string(cutsceneOutputFormatName(CutsceneOutputFormat::InGame))        == "InGame");
    REQUIRE(std::string(cutsceneOutputFormatName(CutsceneOutputFormat::MP4))           == "MP4");
    REQUIRE(std::string(cutsceneOutputFormatName(CutsceneOutputFormat::AVI))           == "AVI");
    REQUIRE(std::string(cutsceneOutputFormatName(CutsceneOutputFormat::MOV))           == "MOV");
    REQUIRE(std::string(cutsceneOutputFormatName(CutsceneOutputFormat::ImageSequence)) == "ImageSequence");
}

TEST_CASE("CutsceneShot defaults", "[Editor][S98]") {
    CutsceneShot s("Opening", CutsceneShotCut::Cut);
    REQUIRE(s.name()       == "Opening");
    REQUIRE(s.transition() == CutsceneShotCut::Cut);
    REQUIRE(s.duration()   == 5.0f);
    REQUIRE(s.isEnabled());
    REQUIRE(s.cameraName() == "");
}

TEST_CASE("CutsceneShot mutation", "[Editor][S98]") {
    CutsceneShot s("Act2", CutsceneShotCut::Fade);
    s.setDuration(8.0f);
    s.setEnabled(false);
    s.setCameraName("cam_main");
    REQUIRE(s.duration()   == 8.0f);
    REQUIRE(!s.isEnabled());
    REQUIRE(s.cameraName() == "cam_main");
}

TEST_CASE("CutsceneDirector add/remove shot", "[Editor][S98]") {
    CutsceneDirector d;
    CutsceneShot s("S1", CutsceneShotCut::Cut);
    REQUIRE(d.addShot(s));
    REQUIRE(d.shotCount() == 1u);
    REQUIRE(!d.addShot(s));
    REQUIRE(d.removeShot("S1"));
    REQUIRE(d.shotCount() == 0u);
}

TEST_CASE("CutsceneDirector state and format", "[Editor][S98]") {
    CutsceneDirector d;
    d.setState(CutsceneDirectorState::Recording);
    REQUIRE(d.isRecording());
    d.setOutputFormat(CutsceneOutputFormat::MP4);
    REQUIRE(d.outputFormat() == CutsceneOutputFormat::MP4);
    d.setFPS(60.0f);
    REQUIRE(d.fps() == 60.0f);
}

TEST_CASE("CutsceneDirector counts", "[Editor][S98]") {
    CutsceneDirector d;
    CutsceneShot s1("A", CutsceneShotCut::Cut);  s1.setDuration(3.0f);
    CutsceneShot s2("B", CutsceneShotCut::Fade); s2.setDuration(2.0f);
    CutsceneShot s3("C", CutsceneShotCut::Cut);  s3.setEnabled(false); s3.setDuration(1.0f);
    d.addShot(s1); d.addShot(s2); d.addShot(s3);
    REQUIRE(d.enabledShotCount()                        == 2u);
    REQUIRE(d.countByTransition(CutsceneShotCut::Cut)   == 2u);
    REQUIRE(d.totalDuration()                           == 6.0f);
}

// ── ActorDirector ────────────────────────────────────────────────────────────

TEST_CASE("ActorBindingType names", "[Editor][S98]") {
    REQUIRE(std::string(actorBindingTypeName(ActorBindingType::Transform))  == "Transform");
    REQUIRE(std::string(actorBindingTypeName(ActorBindingType::Animation))  == "Animation");
    REQUIRE(std::string(actorBindingTypeName(ActorBindingType::Visibility)) == "Visibility");
    REQUIRE(std::string(actorBindingTypeName(ActorBindingType::Material))   == "Material");
    REQUIRE(std::string(actorBindingTypeName(ActorBindingType::Physics))    == "Physics");
    REQUIRE(std::string(actorBindingTypeName(ActorBindingType::Script))     == "Script");
}

TEST_CASE("ActorDirectorMode names", "[Editor][S98]") {
    REQUIRE(std::string(actorDirectorModeName(ActorDirectorMode::Edit))    == "Edit");
    REQUIRE(std::string(actorDirectorModeName(ActorDirectorMode::Preview)) == "Preview");
    REQUIRE(std::string(actorDirectorModeName(ActorDirectorMode::Record))  == "Record");
    REQUIRE(std::string(actorDirectorModeName(ActorDirectorMode::Bake))    == "Bake");
}

TEST_CASE("ActorDirectorPoseMode names", "[Editor][S98]") {
    REQUIRE(std::string(actorDirectorPoseModeName(ActorDirectorPoseMode::Authored))  == "Authored");
    REQUIRE(std::string(actorDirectorPoseModeName(ActorDirectorPoseMode::Override))  == "Override");
    REQUIRE(std::string(actorDirectorPoseModeName(ActorDirectorPoseMode::Additive))  == "Additive");
    REQUIRE(std::string(actorDirectorPoseModeName(ActorDirectorPoseMode::Reset))     == "Reset");
}

TEST_CASE("ActorBinding defaults", "[Editor][S98]") {
    ActorBinding b("hero", ActorBindingType::Animation);
    REQUIRE(b.actorName() == "hero");
    REQUIRE(b.type()      == ActorBindingType::Animation);
    REQUIRE(b.weight()    == 1.0f);
    REQUIRE(b.keyCount()  == 0u);
    REQUIRE(b.isEnabled());
}

TEST_CASE("ActorBinding mutation", "[Editor][S98]") {
    ActorBinding b("npc_1", ActorBindingType::Transform);
    b.setWeight(0.5f);
    b.setKeyCount(24);
    b.setEnabled(false);
    REQUIRE(b.weight()   == 0.5f);
    REQUIRE(b.keyCount() == 24u);
    REQUIRE(!b.isEnabled());
}

TEST_CASE("ActorDirector add/remove binding", "[Editor][S98]") {
    ActorDirector dir;
    ActorBinding b("hero", ActorBindingType::Animation);
    REQUIRE(dir.addBinding(b));
    REQUIRE(dir.bindingCount() == 1u);
    REQUIRE(!dir.addBinding(b));
    REQUIRE(dir.removeBinding("hero", ActorBindingType::Animation));
    REQUIRE(dir.bindingCount() == 0u);
}

TEST_CASE("ActorDirector counts", "[Editor][S98]") {
    ActorDirector dir;
    ActorBinding b1("hero", ActorBindingType::Animation);
    ActorBinding b2("hero", ActorBindingType::Transform);
    ActorBinding b3("npc",  ActorBindingType::Animation); b3.setEnabled(false);
    dir.addBinding(b1); dir.addBinding(b2); dir.addBinding(b3);
    REQUIRE(dir.bindingCount()                                  == 3u);
    REQUIRE(dir.enabledBindingCount()                           == 2u);
    REQUIRE(dir.countByBindingType(ActorBindingType::Animation) == 2u);
    REQUIRE(dir.uniqueActorCount()                              == 2u);
}

TEST_CASE("ActorDirector mode settings", "[Editor][S98]") {
    ActorDirector dir;
    dir.setMode(ActorDirectorMode::Bake);
    REQUIRE(dir.mode() == ActorDirectorMode::Bake);
    dir.setPoseMode(ActorDirectorPoseMode::Additive);
    REQUIRE(dir.poseMode() == ActorDirectorPoseMode::Additive);
}
