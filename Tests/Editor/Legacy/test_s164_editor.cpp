// S164 editor tests: CinematicEditorV1, SequencerTrackV1, TimelineMarkerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TimelineMarkerV1.h"
#include "NF/Editor/SequencerTrackV1.h"
#include "NF/Editor/CinematicEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── CinematicEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Cev1Shot validity", "[Editor][S164]") {
    Cev1Shot s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Intro"; s.duration = 5.f;
    REQUIRE(s.isValid());
}

TEST_CASE("Cev1Shot zero duration invalid", "[Editor][S164]") {
    Cev1Shot s; s.id = 1; s.name = "X"; s.duration = 0.f;
    REQUIRE(!s.isValid());
}

TEST_CASE("Cev1Shot negative duration invalid", "[Editor][S164]") {
    Cev1Shot s; s.id = 1; s.name = "X"; s.duration = -1.f;
    REQUIRE(!s.isValid());
}

TEST_CASE("Cev1Shot isApproved and endTime", "[Editor][S164]") {
    Cev1Shot s; s.id = 1; s.name = "S"; s.duration = 4.f; s.startTime = 2.f;
    REQUIRE(!s.isApproved());
    s.editState = Cev1EditState::Approved;
    REQUIRE(s.isApproved());
    REQUIRE(s.endTime() == Approx(6.f));
}

TEST_CASE("CinematicEditorV1 addShot and shotCount", "[Editor][S164]") {
    CinematicEditorV1 ce;
    REQUIRE(ce.shotCount() == 0);
    Cev1Shot s; s.id = 1; s.name = "A"; s.duration = 3.f;
    REQUIRE(ce.addShot(s));
    REQUIRE(ce.shotCount() == 1);
}

TEST_CASE("CinematicEditorV1 addShot invalid fails", "[Editor][S164]") {
    CinematicEditorV1 ce;
    REQUIRE(!ce.addShot(Cev1Shot{}));
}

TEST_CASE("CinematicEditorV1 addShot duplicate fails", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 1; s.name = "A"; s.duration = 1.f;
    ce.addShot(s);
    REQUIRE(!ce.addShot(s));
}

TEST_CASE("CinematicEditorV1 removeShot", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 2; s.name = "B"; s.duration = 2.f;
    ce.addShot(s);
    REQUIRE(ce.removeShot(2));
    REQUIRE(ce.shotCount() == 0);
    REQUIRE(!ce.removeShot(2));
}

TEST_CASE("CinematicEditorV1 setActive and activeId", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 3; s.name = "C"; s.duration = 1.f;
    ce.addShot(s);
    REQUIRE(ce.setActive(3));
    REQUIRE(ce.activeId() == 3);
    REQUIRE(!ce.setActive(99));
}

TEST_CASE("CinematicEditorV1 activeId clears on remove", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 4; s.name = "D"; s.duration = 1.f;
    ce.addShot(s); ce.setActive(4); ce.removeShot(4);
    REQUIRE(ce.activeId() == 0);
}

TEST_CASE("CinematicEditorV1 setEditState approvedCount", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s1; s1.id = 1; s1.name = "A"; s1.duration = 1.f;
    Cev1Shot s2; s2.id = 2; s2.name = "B"; s2.duration = 1.f;
    ce.addShot(s1); ce.addShot(s2);
    ce.setEditState(1, Cev1EditState::Approved);
    REQUIRE(ce.approvedCount() == 1);
}

TEST_CASE("CinematicEditorV1 setDuration valid", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 5; s.name = "E"; s.duration = 1.f;
    ce.addShot(s);
    REQUIRE(ce.setDuration(5, 10.f));
    REQUIRE(ce.findShot(5)->duration == Approx(10.f));
}

TEST_CASE("CinematicEditorV1 setDuration zero fails", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 6; s.name = "F"; s.duration = 1.f;
    ce.addShot(s);
    REQUIRE(!ce.setDuration(6, 0.f));
}

TEST_CASE("CinematicEditorV1 countByType and totalDuration", "[Editor][S164]") {
    CinematicEditorV1 ce;
    Cev1Shot s1; s1.id = 1; s1.name = "A"; s1.duration = 3.f; s1.type = Cev1ShotType::CutScene;
    Cev1Shot s2; s2.id = 2; s2.name = "B"; s2.duration = 5.f; s2.type = Cev1ShotType::Gameplay;
    ce.addShot(s1); ce.addShot(s2);
    REQUIRE(ce.countByType(Cev1ShotType::CutScene) == 1);
    REQUIRE(ce.totalDuration() == Approx(8.f));
}

TEST_CASE("cev1ShotTypeName covers all values", "[Editor][S164]") {
    REQUIRE(std::string(cev1ShotTypeName(Cev1ShotType::CutScene))   == "CutScene");
    REQUIRE(std::string(cev1ShotTypeName(Cev1ShotType::StingerCut)) == "StingerCut");
}

// ── SequencerTrackV1 ─────────────────────────────────────────────────────────

TEST_CASE("Stv1Track validity", "[Editor][S164]") {
    Stv1Track t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Anim";
    REQUIRE(t.isValid());
}

TEST_CASE("Stv1Track isActive and isMuted", "[Editor][S164]") {
    Stv1Track t; t.id = 1; t.name = "T";
    REQUIRE(t.isActive());
    REQUIRE(!t.isMuted());
    t.state = Stv1TrackState::Muted;
    REQUIRE(t.isMuted());
    REQUIRE(!t.isActive());
}

TEST_CASE("Stv1Clip validity and endTime", "[Editor][S164]") {
    Stv1Clip c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Run"; c.startTime = 2.f; c.duration = 3.f;
    REQUIRE(c.isValid());
    REQUIRE(c.endTime() == Approx(5.f));
}

TEST_CASE("Stv1Track addClip and removeClip", "[Editor][S164]") {
    Stv1Track t; t.id = 1; t.name = "Audio";
    Stv1Clip c; c.id = 1; c.name = "Fx"; c.duration = 2.f;
    REQUIRE(t.addClip(c));
    REQUIRE(t.clips.size() == 1);
    REQUIRE(t.removeClip(1));
    REQUIRE(t.clips.empty());
}

TEST_CASE("SequencerTrackV1 addTrack and trackCount", "[Editor][S164]") {
    SequencerTrackV1 sq;
    REQUIRE(sq.trackCount() == 0);
    Stv1Track t; t.id = 1; t.name = "Main";
    REQUIRE(sq.addTrack(t));
    REQUIRE(sq.trackCount() == 1);
}

TEST_CASE("SequencerTrackV1 addTrack invalid fails", "[Editor][S164]") {
    SequencerTrackV1 sq;
    REQUIRE(!sq.addTrack(Stv1Track{}));
}

TEST_CASE("SequencerTrackV1 addTrack duplicate fails", "[Editor][S164]") {
    SequencerTrackV1 sq;
    Stv1Track t; t.id = 1; t.name = "A";
    sq.addTrack(t);
    REQUIRE(!sq.addTrack(t));
}

TEST_CASE("SequencerTrackV1 removeTrack", "[Editor][S164]") {
    SequencerTrackV1 sq;
    Stv1Track t; t.id = 2; t.name = "B";
    sq.addTrack(t);
    REQUIRE(sq.removeTrack(2));
    REQUIRE(sq.trackCount() == 0);
}

TEST_CASE("SequencerTrackV1 setState activeCount mutedCount", "[Editor][S164]") {
    SequencerTrackV1 sq;
    Stv1Track t1; t1.id = 1; t1.name = "A";
    Stv1Track t2; t2.id = 2; t2.name = "B";
    sq.addTrack(t1); sq.addTrack(t2);
    sq.setState(2, Stv1TrackState::Muted);
    REQUIRE(sq.activeCount() == 1);
    REQUIRE(sq.mutedCount()  == 1);
}

TEST_CASE("SequencerTrackV1 addClipToTrack", "[Editor][S164]") {
    SequencerTrackV1 sq;
    Stv1Track t; t.id = 1; t.name = "FX";
    sq.addTrack(t);
    Stv1Clip c; c.id = 1; c.name = "Boom"; c.duration = 0.5f;
    REQUIRE(sq.addClipToTrack(1, c));
    REQUIRE(sq.findTrack(1)->clips.size() == 1);
}

TEST_CASE("SequencerTrackV1 countByType", "[Editor][S164]") {
    SequencerTrackV1 sq;
    Stv1Track t1; t1.id = 1; t1.name = "A"; t1.type = Stv1TrackType::Animation;
    Stv1Track t2; t2.id = 2; t2.name = "B"; t2.type = Stv1TrackType::Audio;
    sq.addTrack(t1); sq.addTrack(t2);
    REQUIRE(sq.countByType(Stv1TrackType::Animation) == 1);
    REQUIRE(sq.countByType(Stv1TrackType::Audio)     == 1);
}

TEST_CASE("stv1TrackTypeName covers all values", "[Editor][S164]") {
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::Animation)) == "Animation");
    REQUIRE(std::string(stv1TrackTypeName(Stv1TrackType::FX))        == "FX");
}

// ── TimelineMarkerV1 ─────────────────────────────────────────────────────────

TEST_CASE("Tmv1Marker validity", "[Editor][S164]") {
    Tmv1Marker m;
    REQUIRE(!m.isValid());
    m.id = 1; m.label = "Start"; m.time = 0.f;
    REQUIRE(m.isValid());
}

TEST_CASE("Tmv1Range validity and duration", "[Editor][S164]") {
    Tmv1Range r;
    REQUIRE(!r.isValid());
    r.id = 1; r.label = "Intro"; r.startTime = 1.f; r.endTime = 5.f;
    REQUIRE(r.isValid());
    REQUIRE(r.duration() == Approx(4.f));
}

TEST_CASE("Tmv1Range endTime <= startTime invalid", "[Editor][S164]") {
    Tmv1Range r; r.id = 1; r.label = "X"; r.startTime = 5.f; r.endTime = 3.f;
    REQUIRE(!r.isValid());
}

TEST_CASE("TimelineMarkerV1 addMarker and markerCount", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    REQUIRE(tm.markerCount() == 0);
    Tmv1Marker m; m.id = 1; m.label = "A"; m.time = 1.f;
    REQUIRE(tm.addMarker(m));
    REQUIRE(tm.markerCount() == 1);
}

TEST_CASE("TimelineMarkerV1 addMarker invalid fails", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    REQUIRE(!tm.addMarker(Tmv1Marker{}));
}

TEST_CASE("TimelineMarkerV1 addMarker duplicate fails", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m; m.id = 1; m.label = "A"; m.time = 1.f;
    tm.addMarker(m);
    REQUIRE(!tm.addMarker(m));
}

TEST_CASE("TimelineMarkerV1 removeMarker", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m; m.id = 2; m.label = "B"; m.time = 2.f;
    tm.addMarker(m);
    REQUIRE(tm.removeMarker(2));
    REQUIRE(tm.markerCount() == 0);
}

TEST_CASE("TimelineMarkerV1 addRange and rangeCount", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    Tmv1Range r; r.id = 1; r.label = "Section1"; r.startTime = 0.f; r.endTime = 10.f;
    REQUIRE(tm.addRange(r));
    REQUIRE(tm.rangeCount() == 1);
}

TEST_CASE("TimelineMarkerV1 removeRange", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    Tmv1Range r; r.id = 1; r.label = "R"; r.startTime = 0.f; r.endTime = 5.f;
    tm.addRange(r);
    REQUIRE(tm.removeRange(1));
    REQUIRE(tm.rangeCount() == 0);
}

TEST_CASE("TimelineMarkerV1 countMarkersByType pinnedCount", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m1; m1.id = 1; m1.label = "E1"; m1.time = 1.f; m1.type = Tmv1MarkerType::Event; m1.pinned = true;
    Tmv1Marker m2; m2.id = 2; m2.label = "B1"; m2.time = 2.f; m2.type = Tmv1MarkerType::Bookmark;
    tm.addMarker(m1); tm.addMarker(m2);
    REQUIRE(tm.countMarkersByType(Tmv1MarkerType::Event)    == 1);
    REQUIRE(tm.countMarkersByType(Tmv1MarkerType::Bookmark) == 1);
    REQUIRE(tm.pinnedCount() == 1);
}

TEST_CASE("TimelineMarkerV1 markersInRange", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    Tmv1Marker m1; m1.id = 1; m1.label = "A"; m1.time = 2.f;
    Tmv1Marker m2; m2.id = 2; m2.label = "B"; m2.time = 6.f;
    Tmv1Marker m3; m3.id = 3; m3.label = "C"; m3.time = 12.f;
    tm.addMarker(m1); tm.addMarker(m2); tm.addMarker(m3);
    auto result = tm.markersInRange(1.f, 7.f);
    REQUIRE(result.size() == 2);
}

TEST_CASE("tmv1MarkerTypeName covers all values", "[Editor][S164]") {
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Event))    == "Event");
    REQUIRE(std::string(tmv1MarkerTypeName(Tmv1MarkerType::Warning))  == "Warning");
}

TEST_CASE("TimelineMarkerV1 onChange callback fires on add", "[Editor][S164]") {
    TimelineMarkerV1 tm;
    int calls = 0;
    tm.setOnChange([&]() { ++calls; });
    Tmv1Marker m; m.id = 1; m.label = "X"; m.time = 0.f;
    tm.addMarker(m);
    REQUIRE(calls == 1);
}
