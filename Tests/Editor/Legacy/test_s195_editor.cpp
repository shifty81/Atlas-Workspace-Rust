// S195 editor tests: SequencerTrackV1, TimelineMarkerV1, SoundMixerEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SequencerTrackV1.h"
#include "NF/Editor/TimelineMarkerV1.h"
#include "NF/Editor/SoundMixerEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── SequencerTrackV1 ─────────────────────────────────────────────────────────

TEST_CASE("Stv1Track validity", "[Editor][S195]") {
    Stv1Track t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "AnimTrack";
    REQUIRE(t.isValid());
}

TEST_CASE("SequencerTrackV1 addTrack and trackCount", "[Editor][S195]") {
    SequencerTrackV1 st;
    REQUIRE(st.trackCount() == 0);
    Stv1Track t; t.id = 1; t.name = "T1";
    REQUIRE(st.addTrack(t));
    REQUIRE(st.trackCount() == 1);
}

TEST_CASE("SequencerTrackV1 addTrack invalid fails", "[Editor][S195]") {
    SequencerTrackV1 st;
    REQUIRE(!st.addTrack(Stv1Track{}));
}

TEST_CASE("SequencerTrackV1 addTrack duplicate fails", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t; t.id = 1; t.name = "A";
    st.addTrack(t);
    REQUIRE(!st.addTrack(t));
}

TEST_CASE("SequencerTrackV1 removeTrack", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t; t.id = 2; t.name = "B";
    st.addTrack(t);
    REQUIRE(st.removeTrack(2));
    REQUIRE(st.trackCount() == 0);
    REQUIRE(!st.removeTrack(2));
}

TEST_CASE("SequencerTrackV1 findTrack", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t; t.id = 3; t.name = "C";
    st.addTrack(t);
    REQUIRE(st.findTrack(3) != nullptr);
    REQUIRE(st.findTrack(99) == nullptr);
}

TEST_CASE("SequencerTrackV1 setState", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t; t.id = 1; t.name = "T";
    st.addTrack(t);
    REQUIRE(st.setState(1, Stv1TrackState::Muted));
    REQUIRE(st.findTrack(1)->isMuted());
}

TEST_CASE("SequencerTrackV1 setBlendMode", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t; t.id = 1; t.name = "T";
    st.addTrack(t);
    REQUIRE(st.setBlendMode(1, Stv1BlendMode::Additive));
    REQUIRE(st.findTrack(1)->blendMode == Stv1BlendMode::Additive);
}

TEST_CASE("SequencerTrackV1 activeCount and mutedCount", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t1; t1.id = 1; t1.name = "A";
    Stv1Track t2; t2.id = 2; t2.name = "B";
    st.addTrack(t1); st.addTrack(t2);
    st.setState(2, Stv1TrackState::Muted);
    REQUIRE(st.activeCount() == 1);
    REQUIRE(st.mutedCount() == 1);
}

TEST_CASE("SequencerTrackV1 countByType", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t1; t1.id = 1; t1.name = "A"; t1.type = Stv1TrackType::Audio;
    Stv1Track t2; t2.id = 2; t2.name = "B"; t2.type = Stv1TrackType::Animation;
    Stv1Track t3; t3.id = 3; t3.name = "C"; t3.type = Stv1TrackType::Audio;
    st.addTrack(t1); st.addTrack(t2); st.addTrack(t3);
    REQUIRE(st.countByType(Stv1TrackType::Audio) == 2);
    REQUIRE(st.countByType(Stv1TrackType::Animation) == 1);
}

TEST_CASE("SequencerTrackV1 addClipToTrack", "[Editor][S195]") {
    SequencerTrackV1 st;
    Stv1Track t; t.id = 1; t.name = "T";
    st.addTrack(t);
    Stv1Clip clip; clip.id = 10; clip.name = "Walk"; clip.duration = 2.f;
    REQUIRE(st.addClipToTrack(1, clip));
    REQUIRE(st.findTrack(1)->clips.size() == 1);
}

TEST_CASE("SequencerTrackV1 onChange callback fires on setState", "[Editor][S195]") {
    SequencerTrackV1 st;
    uint64_t notified = 0;
    st.setOnChange([&](uint64_t id){ notified = id; });
    Stv1Track t; t.id = 5; t.name = "T";
    st.addTrack(t);
    st.setState(5, Stv1TrackState::Solo);
    REQUIRE(notified == 5);
}

TEST_CASE("stv1TrackTypeName all values", "[Editor][S195]") {
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::Animation)) == "Animation");
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::Audio))     == "Audio");
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::Event))     == "Event");
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::Property))  == "Property");
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::Camera))    == "Camera");
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::FX))        == "FX");
}

TEST_CASE("Stv1Clip validity and endTime", "[Editor][S195]") {
    Stv1Clip clip;
    REQUIRE(!clip.isValid());
    clip.id = 1; clip.name = "Run"; clip.duration = 3.f; clip.startTime = 1.f;
    REQUIRE(clip.isValid());
    REQUIRE(clip.endTime() == Approx(4.f));
}

// ── TimelineMarkerV1 ─────────────────────────────────────────────────────────

TEST_CASE("Tmv1Marker validity", "[Editor][S195]") {
    Tmv1Marker m;
    REQUIRE(!m.isValid());
    m.id = 1; m.label = "Hit"; m.time = 1.5f;
    REQUIRE(m.isValid());
}

TEST_CASE("TimelineMarkerV1 addMarker and markerCount", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    REQUIRE(tm.markerCount() == 0);
    Tmv1Marker m; m.id = 1; m.label = "Start"; m.time = 0.f;
    REQUIRE(tm.addMarker(m));
    REQUIRE(tm.markerCount() == 1);
}

TEST_CASE("TimelineMarkerV1 addMarker invalid fails", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    REQUIRE(!tm.addMarker(Tmv1Marker{}));
}

TEST_CASE("TimelineMarkerV1 addMarker duplicate fails", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m; m.id = 1; m.label = "A"; m.time = 0.f;
    tm.addMarker(m);
    REQUIRE(!tm.addMarker(m));
}

TEST_CASE("TimelineMarkerV1 removeMarker", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m; m.id = 2; m.label = "B"; m.time = 1.f;
    tm.addMarker(m);
    REQUIRE(tm.removeMarker(2));
    REQUIRE(tm.markerCount() == 0);
    REQUIRE(!tm.removeMarker(2));
}

TEST_CASE("TimelineMarkerV1 findMarker", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m; m.id = 3; m.label = "C"; m.time = 2.f;
    tm.addMarker(m);
    REQUIRE(tm.findMarker(3) != nullptr);
    REQUIRE(tm.findMarker(99) == nullptr);
}

TEST_CASE("TimelineMarkerV1 addRange and rangeCount", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Range r; r.id = 1; r.label = "Intro"; r.startTime = 0.f; r.endTime = 5.f;
    REQUIRE(tm.addRange(r));
    REQUIRE(tm.rangeCount() == 1);
}

TEST_CASE("TimelineMarkerV1 removeRange", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Range r; r.id = 1; r.label = "Outro"; r.startTime = 10.f; r.endTime = 20.f;
    tm.addRange(r);
    REQUIRE(tm.removeRange(1));
    REQUIRE(tm.rangeCount() == 0);
}

TEST_CASE("TimelineMarkerV1 countMarkersByType", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m1; m1.id = 1; m1.label = "A"; m1.time = 0.f; m1.type = Tmv1MarkerType::Event;
    Tmv1Marker m2; m2.id = 2; m2.label = "B"; m2.time = 1.f; m2.type = Tmv1MarkerType::Bookmark;
    Tmv1Marker m3; m3.id = 3; m3.label = "C"; m3.time = 2.f; m3.type = Tmv1MarkerType::Event;
    tm.addMarker(m1); tm.addMarker(m2); tm.addMarker(m3);
    REQUIRE(tm.countMarkersByType(Tmv1MarkerType::Event) == 2);
    REQUIRE(tm.countMarkersByType(Tmv1MarkerType::Bookmark) == 1);
}

TEST_CASE("TimelineMarkerV1 pinnedCount", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m1; m1.id = 1; m1.label = "A"; m1.time = 0.f; m1.pinned = true;
    Tmv1Marker m2; m2.id = 2; m2.label = "B"; m2.time = 1.f;
    tm.addMarker(m1); tm.addMarker(m2);
    REQUIRE(tm.pinnedCount() == 1);
}

TEST_CASE("TimelineMarkerV1 markersInRange", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m1; m1.id = 1; m1.label = "A"; m1.time = 1.f;
    Tmv1Marker m2; m2.id = 2; m2.label = "B"; m2.time = 3.f;
    Tmv1Marker m3; m3.id = 3; m3.label = "C"; m3.time = 7.f;
    tm.addMarker(m1); tm.addMarker(m2); tm.addMarker(m3);
    auto results = tm.markersInRange(0.f, 5.f);
    REQUIRE(results.size() == 2);
}

TEST_CASE("TimelineMarkerV1 onChange fires on add", "[Editor][S195]") {
    TimelineMarkerV1 tm;
    int changes = 0;
    tm.setOnChange([&]{ ++changes; });
    Tmv1Marker m; m.id = 1; m.label = "X"; m.time = 0.f;
    tm.addMarker(m);
    REQUIRE(changes == 1);
}

TEST_CASE("Tmv1Range duration", "[Editor][S195]") {
    Tmv1Range r; r.id = 1; r.label = "X"; r.startTime = 2.f; r.endTime = 6.f;
    REQUIRE(r.duration() == Approx(4.f));
}

TEST_CASE("tmv1MarkerTypeName all values", "[Editor][S195]") {
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Event))    == "Event");
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Bookmark)) == "Bookmark");
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Chapter))  == "Chapter");
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Warning))  == "Warning");
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Note))     == "Note");
}

// ── SoundMixerEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("SmvChannel validity", "[Editor][S195]") {
    SmvChannel c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Music";
    REQUIRE(c.isValid());
}

TEST_CASE("SoundMixerEditorV1 addChannel and channelCount", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    REQUIRE(sm.channelCount() == 0);
    SmvChannel c; c.id = 1; c.name = "C1";
    REQUIRE(sm.addChannel(c));
    REQUIRE(sm.channelCount() == 1);
}

TEST_CASE("SoundMixerEditorV1 addChannel invalid fails", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    REQUIRE(!sm.addChannel(SmvChannel{}));
}

TEST_CASE("SoundMixerEditorV1 addChannel duplicate fails", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "A";
    sm.addChannel(c);
    REQUIRE(!sm.addChannel(c));
}

TEST_CASE("SoundMixerEditorV1 removeChannel", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 2; c.name = "B";
    sm.addChannel(c);
    REQUIRE(sm.removeChannel(2));
    REQUIRE(sm.channelCount() == 0);
    REQUIRE(!sm.removeChannel(2));
}

TEST_CASE("SoundMixerEditorV1 setVolume and getMixedVolume", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "Music";
    sm.addChannel(c);
    REQUIRE(sm.setVolume(1, 0.5f));
    REQUIRE(sm.getMixedVolume(1) == Approx(0.5f));
}

TEST_CASE("SoundMixerEditorV1 volume clamped to [0,2]", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "SFX";
    sm.addChannel(c);
    sm.setVolume(1, 5.f);
    REQUIRE(sm.getMixedVolume(1) == Approx(2.f));
    sm.setVolume(1, -1.f);
    REQUIRE(sm.getMixedVolume(1) == Approx(0.f));
}

TEST_CASE("SoundMixerEditorV1 setPan", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "Voice";
    sm.addChannel(c);
    REQUIRE(sm.setPan(1, -0.5f));
}

TEST_CASE("SoundMixerEditorV1 muteChannel makes volume zero", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "Ambient";
    sm.addChannel(c);
    REQUIRE(sm.muteChannel(1, true));
    REQUIRE(sm.getMixedVolume(1) == Approx(0.f));
}

TEST_CASE("SoundMixerEditorV1 unmuteChannel restores volume", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "UI";
    sm.addChannel(c);
    sm.muteChannel(1, true);
    sm.muteChannel(1, false);
    REQUIRE(sm.getMixedVolume(1) > 0.f);
}

TEST_CASE("SoundMixerEditorV1 soloChannel", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "Music";
    sm.addChannel(c);
    REQUIRE(sm.soloChannel(1, true));
}

TEST_CASE("SoundMixerEditorV1 setMasterVolume affects mix", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    SmvChannel c; c.id = 1; c.name = "SFX";
    sm.addChannel(c);
    sm.setVolume(1, 1.f);
    sm.setMasterVolume(0.5f);
    REQUIRE(sm.getMixedVolume(1) == Approx(0.5f));
}

TEST_CASE("SoundMixerEditorV1 masterVolume default is 1", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    REQUIRE(sm.getMasterVolume() == Approx(1.f));
}

TEST_CASE("SoundMixerEditorV1 getMixedVolume unknown channel returns 0", "[Editor][S195]") {
    SoundMixerEditorV1 sm;
    REQUIRE(sm.getMixedVolume(99) == Approx(0.f));
}
