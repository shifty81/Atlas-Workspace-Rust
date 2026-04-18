// S114 editor tests: ReplaySystemEditor, MatchReplayEditor, SpectatorEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SpectatorEditor.h"
#include "NF/Editor/MatchReplayEditor.h"
#include "NF/Editor/ReplaySystemEditor.h"

using namespace NF;

// ── ReplaySystemEditor ───────────────────────────────────────────────────────

TEST_CASE("ReplayRecordingMode names", "[Editor][S114]") {
    REQUIRE(std::string(replayRecordingModeName(ReplayRecordingMode::Continuous)) == "Continuous");
    REQUIRE(std::string(replayRecordingModeName(ReplayRecordingMode::OnEvent))    == "OnEvent");
    REQUIRE(std::string(replayRecordingModeName(ReplayRecordingMode::Periodic))   == "Periodic");
    REQUIRE(std::string(replayRecordingModeName(ReplayRecordingMode::Manual))     == "Manual");
    REQUIRE(std::string(replayRecordingModeName(ReplayRecordingMode::Compressed)) == "Compressed");
}

TEST_CASE("ReplayPlaybackSpeed names", "[Editor][S114]") {
    REQUIRE(std::string(replayPlaybackSpeedName(ReplayPlaybackSpeed::Eighth))    == "Eighth");
    REQUIRE(std::string(replayPlaybackSpeedName(ReplayPlaybackSpeed::Quarter))   == "Quarter");
    REQUIRE(std::string(replayPlaybackSpeedName(ReplayPlaybackSpeed::Half))      == "Half");
    REQUIRE(std::string(replayPlaybackSpeedName(ReplayPlaybackSpeed::Normal))    == "Normal");
    REQUIRE(std::string(replayPlaybackSpeedName(ReplayPlaybackSpeed::Double))    == "Double");
    REQUIRE(std::string(replayPlaybackSpeedName(ReplayPlaybackSpeed::Quadruple)) == "Quadruple");
}

TEST_CASE("ReplayDataChannel names", "[Editor][S114]") {
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Transform))  == "Transform");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Physics))    == "Physics");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Animation))  == "Animation");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Audio))      == "Audio");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Input))      == "Input");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Network))    == "Network");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::Custom))     == "Custom");
    REQUIRE(std::string(replayDataChannelName(ReplayDataChannel::All))        == "All");
}

TEST_CASE("ReplayClip defaults", "[Editor][S114]") {
    ReplayClip rc(1, "match_clip_01", ReplayRecordingMode::Continuous);
    REQUIRE(rc.id()              == 1u);
    REQUIRE(rc.name()            == "match_clip_01");
    REQUIRE(rc.recordingMode()   == ReplayRecordingMode::Continuous);
    REQUIRE(rc.playbackSpeed()   == ReplayPlaybackSpeed::Normal);
    REQUIRE(rc.durationSeconds() == 0.0f);
    REQUIRE(rc.fileSizeKB()      == 0u);
    REQUIRE(!rc.isLooping());
    REQUIRE(rc.hasAudio());
}

TEST_CASE("ReplayClip mutation", "[Editor][S114]") {
    ReplayClip rc(2, "highlight_reel", ReplayRecordingMode::OnEvent);
    rc.setPlaybackSpeed(ReplayPlaybackSpeed::Half);
    rc.setDurationSeconds(30.5f);
    rc.setFileSizeKB(1024u);
    rc.setLooping(true);
    rc.setAudioEnabled(false);
    REQUIRE(rc.playbackSpeed()   == ReplayPlaybackSpeed::Half);
    REQUIRE(rc.durationSeconds() == 30.5f);
    REQUIRE(rc.fileSizeKB()      == 1024u);
    REQUIRE(rc.isLooping());
    REQUIRE(!rc.hasAudio());
}

TEST_CASE("ReplaySystemEditor defaults", "[Editor][S114]") {
    ReplaySystemEditor ed;
    REQUIRE(ed.activeChannel()  == ReplayDataChannel::All);
    REQUIRE(ed.isShowTimeline());
    REQUIRE(ed.isShowMarkers());
    REQUIRE(ed.playbackRate()   == 1.0f);
    REQUIRE(ed.clipCount()      == 0u);
}

TEST_CASE("ReplaySystemEditor add/remove clips", "[Editor][S114]") {
    ReplaySystemEditor ed;
    REQUIRE(ed.addClip(ReplayClip(1, "clip_a", ReplayRecordingMode::Continuous)));
    REQUIRE(ed.addClip(ReplayClip(2, "clip_b", ReplayRecordingMode::OnEvent)));
    REQUIRE(ed.addClip(ReplayClip(3, "clip_c", ReplayRecordingMode::Manual)));
    REQUIRE(!ed.addClip(ReplayClip(1, "clip_a", ReplayRecordingMode::Continuous)));
    REQUIRE(ed.clipCount() == 3u);
    REQUIRE(ed.removeClip(2));
    REQUIRE(ed.clipCount() == 2u);
    REQUIRE(!ed.removeClip(99));
}

TEST_CASE("ReplaySystemEditor counts and find", "[Editor][S114]") {
    ReplaySystemEditor ed;
    ReplayClip r1(1, "c1", ReplayRecordingMode::Continuous);
    ReplayClip r2(2, "c2", ReplayRecordingMode::Continuous); r2.setPlaybackSpeed(ReplayPlaybackSpeed::Half);
    ReplayClip r3(3, "c3", ReplayRecordingMode::OnEvent);    r3.setLooping(true);
    ReplayClip r4(4, "c4", ReplayRecordingMode::Manual);     r4.setLooping(true); r4.setPlaybackSpeed(ReplayPlaybackSpeed::Half);
    ed.addClip(r1); ed.addClip(r2); ed.addClip(r3); ed.addClip(r4);
    REQUIRE(ed.countByMode(ReplayRecordingMode::Continuous)     == 2u);
    REQUIRE(ed.countByMode(ReplayRecordingMode::OnEvent)        == 1u);
    REQUIRE(ed.countByMode(ReplayRecordingMode::Periodic)       == 0u);
    REQUIRE(ed.countBySpeed(ReplayPlaybackSpeed::Normal)        == 2u);
    REQUIRE(ed.countBySpeed(ReplayPlaybackSpeed::Half)          == 2u);
    REQUIRE(ed.countLooping()                                   == 2u);
    auto* found = ed.findClip(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->recordingMode() == ReplayRecordingMode::OnEvent);
    REQUIRE(ed.findClip(99) == nullptr);
}

TEST_CASE("ReplaySystemEditor settings mutation", "[Editor][S114]") {
    ReplaySystemEditor ed;
    ed.setActiveChannel(ReplayDataChannel::Physics);
    ed.setShowTimeline(false);
    ed.setShowMarkers(false);
    ed.setPlaybackRate(2.0f);
    REQUIRE(ed.activeChannel()  == ReplayDataChannel::Physics);
    REQUIRE(!ed.isShowTimeline());
    REQUIRE(!ed.isShowMarkers());
    REQUIRE(ed.playbackRate()   == 2.0f);
}

// ── MatchReplayEditor ────────────────────────────────────────────────────────

TEST_CASE("MatchPhase names", "[Editor][S114]") {
    REQUIRE(std::string(matchPhaseName(MatchPhase::PreGame))  == "PreGame");
    REQUIRE(std::string(matchPhaseName(MatchPhase::Opening))  == "Opening");
    REQUIRE(std::string(matchPhaseName(MatchPhase::MidGame))  == "MidGame");
    REQUIRE(std::string(matchPhaseName(MatchPhase::LateGame)) == "LateGame");
    REQUIRE(std::string(matchPhaseName(MatchPhase::Overtime)) == "Overtime");
    REQUIRE(std::string(matchPhaseName(MatchPhase::PostGame)) == "PostGame");
}

TEST_CASE("MatchEventType names", "[Editor][S114]") {
    REQUIRE(std::string(matchEventTypeName(MatchEventType::Kill))       == "Kill");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::Death))      == "Death");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::Assist))     == "Assist");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::Capture))    == "Capture");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::Score))      == "Score");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::PowerUp))    == "PowerUp");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::TeamAction)) == "TeamAction");
    REQUIRE(std::string(matchEventTypeName(MatchEventType::Custom))     == "Custom");
}

TEST_CASE("ReplayCameraMode names", "[Editor][S114]") {
    REQUIRE(std::string(replayCameraModeName(ReplayCameraMode::Free))     == "Free");
    REQUIRE(std::string(replayCameraModeName(ReplayCameraMode::Follow))   == "Follow");
    REQUIRE(std::string(replayCameraModeName(ReplayCameraMode::Overview)) == "Overview");
    REQUIRE(std::string(replayCameraModeName(ReplayCameraMode::Fixed))    == "Fixed");
    REQUIRE(std::string(replayCameraModeName(ReplayCameraMode::Dynamic))  == "Dynamic");
}

TEST_CASE("MatchEvent defaults", "[Editor][S114]") {
    MatchEvent ev(1, "first_kill", MatchEventType::Kill);
    REQUIRE(ev.id()           == 1u);
    REQUIRE(ev.name()         == "first_kill");
    REQUIRE(ev.eventType()    == MatchEventType::Kill);
    REQUIRE(ev.phase()        == MatchPhase::MidGame);
    REQUIRE(ev.timestampSec() == 0.0f);
    REQUIRE(ev.teamId()       == 0u);
    REQUIRE(ev.playerId()     == 0u);
    REQUIRE(!ev.isHighlight());
}

TEST_CASE("MatchEvent mutation", "[Editor][S114]") {
    MatchEvent ev(2, "game_winner", MatchEventType::Score);
    ev.setPhase(MatchPhase::LateGame);
    ev.setTimestampSec(120.5f);
    ev.setTeamId(1u);
    ev.setPlayerId(42u);
    ev.setHighlight(true);
    REQUIRE(ev.phase()        == MatchPhase::LateGame);
    REQUIRE(ev.timestampSec() == 120.5f);
    REQUIRE(ev.teamId()       == 1u);
    REQUIRE(ev.playerId()     == 42u);
    REQUIRE(ev.isHighlight());
}

TEST_CASE("MatchReplayEditor defaults", "[Editor][S114]") {
    MatchReplayEditor ed;
    REQUIRE(ed.cameraMode()    == ReplayCameraMode::Dynamic);
    REQUIRE(ed.isShowMinimap());
    REQUIRE(ed.isShowStats());
    REQUIRE(ed.replaySpeed()   == 1.0f);
    REQUIRE(ed.eventCount()    == 0u);
}

TEST_CASE("MatchReplayEditor add/remove events", "[Editor][S114]") {
    MatchReplayEditor ed;
    REQUIRE(ed.addEvent(MatchEvent(1, "kill_1",  MatchEventType::Kill)));
    REQUIRE(ed.addEvent(MatchEvent(2, "death_1", MatchEventType::Death)));
    REQUIRE(ed.addEvent(MatchEvent(3, "score_1", MatchEventType::Score)));
    REQUIRE(!ed.addEvent(MatchEvent(1, "kill_1", MatchEventType::Kill)));
    REQUIRE(ed.eventCount() == 3u);
    REQUIRE(ed.removeEvent(2));
    REQUIRE(ed.eventCount() == 2u);
    REQUIRE(!ed.removeEvent(99));
}

TEST_CASE("MatchReplayEditor counts and find", "[Editor][S114]") {
    MatchReplayEditor ed;
    MatchEvent e1(1, "kill_a",  MatchEventType::Kill);
    MatchEvent e2(2, "kill_b",  MatchEventType::Kill);   e2.setPhase(MatchPhase::LateGame);
    MatchEvent e3(3, "score_a", MatchEventType::Score);  e3.setHighlight(true);
    MatchEvent e4(4, "power_a", MatchEventType::PowerUp); e4.setHighlight(true); e4.setPhase(MatchPhase::LateGame);
    ed.addEvent(e1); ed.addEvent(e2); ed.addEvent(e3); ed.addEvent(e4);
    REQUIRE(ed.countByPhase(MatchPhase::MidGame)              == 2u);
    REQUIRE(ed.countByPhase(MatchPhase::LateGame)             == 2u);
    REQUIRE(ed.countByPhase(MatchPhase::Overtime)             == 0u);
    REQUIRE(ed.countByEventType(MatchEventType::Kill)         == 2u);
    REQUIRE(ed.countByEventType(MatchEventType::Score)        == 1u);
    REQUIRE(ed.countHighlights()                              == 2u);
    auto* found = ed.findEvent(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->eventType() == MatchEventType::Score);
    REQUIRE(ed.findEvent(99) == nullptr);
}

TEST_CASE("MatchReplayEditor settings mutation", "[Editor][S114]") {
    MatchReplayEditor ed;
    ed.setCameraMode(ReplayCameraMode::Follow);
    ed.setShowMinimap(false);
    ed.setShowStats(false);
    ed.setReplaySpeed(2.0f);
    REQUIRE(ed.cameraMode()  == ReplayCameraMode::Follow);
    REQUIRE(!ed.isShowMinimap());
    REQUIRE(!ed.isShowStats());
    REQUIRE(ed.replaySpeed() == 2.0f);
}

// ── SpectatorEditor ──────────────────────────────────────────────────────────

TEST_CASE("SpectatorViewMode names", "[Editor][S114]") {
    REQUIRE(std::string(spectatorViewModeName(SpectatorViewMode::FirstPerson))  == "FirstPerson");
    REQUIRE(std::string(spectatorViewModeName(SpectatorViewMode::ThirdPerson))  == "ThirdPerson");
    REQUIRE(std::string(spectatorViewModeName(SpectatorViewMode::Overhead))     == "Overhead");
    REQUIRE(std::string(spectatorViewModeName(SpectatorViewMode::Director))     == "Director");
    REQUIRE(std::string(spectatorViewModeName(SpectatorViewMode::Broadcast))    == "Broadcast");
    REQUIRE(std::string(spectatorViewModeName(SpectatorViewMode::Free))         == "Free");
}

TEST_CASE("SpectatorTarget names", "[Editor][S114]") {
    REQUIRE(std::string(spectatorTargetName(SpectatorTarget::Player)) == "Player");
    REQUIRE(std::string(spectatorTargetName(SpectatorTarget::Team))   == "Team");
    REQUIRE(std::string(spectatorTargetName(SpectatorTarget::Ball))   == "Ball");
    REQUIRE(std::string(spectatorTargetName(SpectatorTarget::POI))    == "POI");
    REQUIRE(std::string(spectatorTargetName(SpectatorTarget::Auto))   == "Auto");
}

TEST_CASE("SpectatorHUDLayout names", "[Editor][S114]") {
    REQUIRE(std::string(spectatorHUDLayoutName(SpectatorHUDLayout::Minimal))   == "Minimal");
    REQUIRE(std::string(spectatorHUDLayoutName(SpectatorHUDLayout::Standard))  == "Standard");
    REQUIRE(std::string(spectatorHUDLayoutName(SpectatorHUDLayout::Broadcast)) == "Broadcast");
    REQUIRE(std::string(spectatorHUDLayoutName(SpectatorHUDLayout::Custom))    == "Custom");
}

TEST_CASE("SpectatorCamera defaults", "[Editor][S114]") {
    SpectatorCamera cam(1, "overview_cam", SpectatorViewMode::Overhead);
    REQUIRE(cam.id()             == 1u);
    REQUIRE(cam.name()           == "overview_cam");
    REQUIRE(cam.viewMode()       == SpectatorViewMode::Overhead);
    REQUIRE(cam.target()         == SpectatorTarget::Auto);
    REQUIRE(cam.hudLayout()      == SpectatorHUDLayout::Standard);
    REQUIRE(cam.fov()            == 90.0f);
    REQUIRE(cam.isAutoSwitch());
    REQUIRE(cam.switchInterval() == 5.0f);
}

TEST_CASE("SpectatorCamera mutation", "[Editor][S114]") {
    SpectatorCamera cam(2, "broadcast_cam", SpectatorViewMode::Broadcast);
    cam.setTarget(SpectatorTarget::Player);
    cam.setHUDLayout(SpectatorHUDLayout::Broadcast);
    cam.setFOV(75.0f);
    cam.setAutoSwitch(false);
    cam.setSwitchInterval(10.0f);
    REQUIRE(cam.target()         == SpectatorTarget::Player);
    REQUIRE(cam.hudLayout()      == SpectatorHUDLayout::Broadcast);
    REQUIRE(cam.fov()            == 75.0f);
    REQUIRE(!cam.isAutoSwitch());
    REQUIRE(cam.switchInterval() == 10.0f);
}

TEST_CASE("SpectatorEditor defaults", "[Editor][S114]") {
    SpectatorEditor ed;
    REQUIRE(ed.activeLayoutId()      == 0u);
    REQUIRE(ed.isShowPlayerNames());
    REQUIRE(ed.isShowScoreboard());
    REQUIRE(ed.transitionDuration()  == 1.0f);
    REQUIRE(ed.cameraCount()         == 0u);
}

TEST_CASE("SpectatorEditor add/remove cameras", "[Editor][S114]") {
    SpectatorEditor ed;
    REQUIRE(ed.addCamera(SpectatorCamera(1, "cam_a", SpectatorViewMode::Free)));
    REQUIRE(ed.addCamera(SpectatorCamera(2, "cam_b", SpectatorViewMode::ThirdPerson)));
    REQUIRE(ed.addCamera(SpectatorCamera(3, "cam_c", SpectatorViewMode::Overhead)));
    REQUIRE(!ed.addCamera(SpectatorCamera(1, "cam_a", SpectatorViewMode::Free)));
    REQUIRE(ed.cameraCount() == 3u);
    REQUIRE(ed.removeCamera(2));
    REQUIRE(ed.cameraCount() == 2u);
    REQUIRE(!ed.removeCamera(99));
}

TEST_CASE("SpectatorEditor counts and find", "[Editor][S114]") {
    SpectatorEditor ed;
    SpectatorCamera c1(1, "free_a",      SpectatorViewMode::Free);
    SpectatorCamera c2(2, "free_b",      SpectatorViewMode::Free);  c2.setTarget(SpectatorTarget::Player);
    SpectatorCamera c3(3, "overhead_a",  SpectatorViewMode::Overhead); c3.setAutoSwitch(false);
    SpectatorCamera c4(4, "director_a",  SpectatorViewMode::Director); c4.setAutoSwitch(false); c4.setTarget(SpectatorTarget::Player);
    ed.addCamera(c1); ed.addCamera(c2); ed.addCamera(c3); ed.addCamera(c4);
    REQUIRE(ed.countByViewMode(SpectatorViewMode::Free)       == 2u);
    REQUIRE(ed.countByViewMode(SpectatorViewMode::Overhead)   == 1u);
    REQUIRE(ed.countByViewMode(SpectatorViewMode::Broadcast)  == 0u);
    REQUIRE(ed.countByTarget(SpectatorTarget::Auto)           == 2u);
    REQUIRE(ed.countByTarget(SpectatorTarget::Player)         == 2u);
    REQUIRE(ed.countAutoSwitch()                              == 2u);
    auto* found = ed.findCamera(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->viewMode() == SpectatorViewMode::Overhead);
    REQUIRE(ed.findCamera(99) == nullptr);
}

TEST_CASE("SpectatorEditor settings mutation", "[Editor][S114]") {
    SpectatorEditor ed;
    ed.setActiveLayoutId(5u);
    ed.setShowPlayerNames(false);
    ed.setShowScoreboard(false);
    ed.setTransitionDuration(2.5f);
    REQUIRE(ed.activeLayoutId()     == 5u);
    REQUIRE(!ed.isShowPlayerNames());
    REQUIRE(!ed.isShowScoreboard());
    REQUIRE(ed.transitionDuration() == 2.5f);
}
