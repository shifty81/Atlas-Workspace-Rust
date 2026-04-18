// S176 editor tests: ReplaySystemEditorV1, MatchReplayEditorV1, PlaytestRecorderV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ReplaySystemEditorV1.h"
#include "NF/Editor/MatchReplayEditorV1.h"
#include "NF/Editor/PlaytestRecorderV1.h"

using namespace NF;
using Catch::Approx;

// ── ReplaySystemEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Rsyv1Session validity", "[Editor][S176]") {
    Rsyv1Session s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Match1";
    REQUIRE(s.isValid());
}

TEST_CASE("ReplaySystemEditorV1 addSession and sessionCount", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    REQUIRE(rsy.sessionCount() == 0);
    Rsyv1Session s; s.id = 1; s.name = "S1";
    REQUIRE(rsy.addSession(s));
    REQUIRE(rsy.sessionCount() == 1);
}

TEST_CASE("ReplaySystemEditorV1 addSession invalid fails", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    REQUIRE(!rsy.addSession(Rsyv1Session{}));
}

TEST_CASE("ReplaySystemEditorV1 addSession duplicate fails", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s; s.id = 1; s.name = "A";
    rsy.addSession(s);
    REQUIRE(!rsy.addSession(s));
}

TEST_CASE("ReplaySystemEditorV1 removeSession", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s; s.id = 2; s.name = "B";
    rsy.addSession(s);
    REQUIRE(rsy.removeSession(2));
    REQUIRE(rsy.sessionCount() == 0);
    REQUIRE(!rsy.removeSession(2));
}

TEST_CASE("ReplaySystemEditorV1 setState recordingCount", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s; s.id = 1; s.name = "A";
    rsy.addSession(s);
    REQUIRE(rsy.setState(1, Rsyv1SessionState::Recording));
    REQUIRE(rsy.recordingCount() == 1);
    REQUIRE(rsy.findSession(1)->isRecording());
}

TEST_CASE("ReplaySystemEditorV1 playingCount", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s; s.id = 1; s.name = "A";
    rsy.addSession(s);
    rsy.setState(1, Rsyv1SessionState::Playing);
    REQUIRE(rsy.playingCount() == 1);
    REQUIRE(rsy.findSession(1)->isPlaying());
}

TEST_CASE("ReplaySystemEditorV1 error state", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s; s.id = 1; s.name = "A";
    rsy.addSession(s);
    rsy.setState(1, Rsyv1SessionState::Error);
    REQUIRE(rsy.findSession(1)->hasError());
}

TEST_CASE("ReplaySystemEditorV1 countByQuality", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s1; s1.id = 1; s1.name = "A"; s1.quality = Rsyv1ReplayQuality::High;
    Rsyv1Session s2; s2.id = 2; s2.name = "B"; s2.quality = Rsyv1ReplayQuality::Lossless;
    rsy.addSession(s1); rsy.addSession(s2);
    REQUIRE(rsy.countByQuality(Rsyv1ReplayQuality::High)     == 1);
    REQUIRE(rsy.countByQuality(Rsyv1ReplayQuality::Lossless) == 1);
}

TEST_CASE("ReplaySystemEditorV1 findSession returns ptr", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    Rsyv1Session s; s.id = 5; s.name = "S5";
    rsy.addSession(s);
    REQUIRE(rsy.findSession(5) != nullptr);
    REQUIRE(rsy.findSession(5)->name == "S5");
    REQUIRE(rsy.findSession(99) == nullptr);
}

TEST_CASE("rsyv1SessionStateName covers all values", "[Editor][S176]") {
    REQUIRE(std::string(rsyv1SessionStateName(Rsyv1SessionState::Idle))      == "Idle");
    REQUIRE(std::string(rsyv1SessionStateName(Rsyv1SessionState::Recording)) == "Recording");
    REQUIRE(std::string(rsyv1SessionStateName(Rsyv1SessionState::Error))     == "Error");
}

TEST_CASE("rsyv1ReplayQualityName covers all values", "[Editor][S176]") {
    REQUIRE(std::string(rsyv1ReplayQualityName(Rsyv1ReplayQuality::Low))      == "Low");
    REQUIRE(std::string(rsyv1ReplayQualityName(Rsyv1ReplayQuality::Lossless)) == "Lossless");
}

TEST_CASE("Rsyv1Frame validity", "[Editor][S176]") {
    Rsyv1Frame f;
    REQUIRE(!f.isValid());
    f.id = 1; f.sessionId = 10;
    REQUIRE(f.isValid());
}

TEST_CASE("ReplaySystemEditorV1 onStateChange callback", "[Editor][S176]") {
    ReplaySystemEditorV1 rsy;
    uint64_t notified = 0;
    rsy.setOnStateChange([&](uint64_t id) { notified = id; });
    Rsyv1Session s; s.id = 7; s.name = "G";
    rsy.addSession(s);
    rsy.setState(7, Rsyv1SessionState::Playing);
    REQUIRE(notified == 7);
}

// ── MatchReplayEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Mrpv1Match validity", "[Editor][S176]") {
    Mrpv1Match m;
    REQUIRE(!m.isValid());
    m.id = 1; m.name = "Round1";
    REQUIRE(m.isValid());
}

TEST_CASE("MatchReplayEditorV1 addMatch and matchCount", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    REQUIRE(mrp.matchCount() == 0);
    Mrpv1Match m; m.id = 1; m.name = "M1";
    REQUIRE(mrp.addMatch(m));
    REQUIRE(mrp.matchCount() == 1);
}

TEST_CASE("MatchReplayEditorV1 addMatch invalid fails", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    REQUIRE(!mrp.addMatch(Mrpv1Match{}));
}

TEST_CASE("MatchReplayEditorV1 addMatch duplicate fails", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Match m; m.id = 1; m.name = "A";
    mrp.addMatch(m);
    REQUIRE(!mrp.addMatch(m));
}

TEST_CASE("MatchReplayEditorV1 removeMatch", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Match m; m.id = 2; m.name = "B";
    mrp.addMatch(m);
    REQUIRE(mrp.removeMatch(2));
    REQUIRE(mrp.matchCount() == 0);
    REQUIRE(!mrp.removeMatch(2));
}

TEST_CASE("MatchReplayEditorV1 addBookmark and bookmarkCount", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Bookmark b; b.id = 1; b.matchId = 10; b.type = Mrpv1BookmarkType::Kill;
    REQUIRE(mrp.addBookmark(b));
    REQUIRE(mrp.bookmarkCount() == 1);
}

TEST_CASE("MatchReplayEditorV1 addBookmark invalid fails", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    REQUIRE(!mrp.addBookmark(Mrpv1Bookmark{}));
}

TEST_CASE("MatchReplayEditorV1 removeBookmark", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Bookmark b; b.id = 1; b.matchId = 5;
    mrp.addBookmark(b);
    REQUIRE(mrp.removeBookmark(1));
    REQUIRE(mrp.bookmarkCount() == 0);
    REQUIRE(!mrp.removeBookmark(1));
}

TEST_CASE("MatchReplayEditorV1 setOutcome winCount lossCount", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Match m1; m1.id = 1; m1.name = "A";
    Mrpv1Match m2; m2.id = 2; m2.name = "B";
    mrp.addMatch(m1); mrp.addMatch(m2);
    mrp.setOutcome(1, Mrpv1MatchOutcome::Win);
    mrp.setOutcome(2, Mrpv1MatchOutcome::Loss);
    REQUIRE(mrp.winCount()  == 1);
    REQUIRE(mrp.lossCount() == 1);
}

TEST_CASE("MatchReplayEditorV1 countByBookmarkType", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Bookmark b1; b1.id = 1; b1.matchId = 1; b1.type = Mrpv1BookmarkType::Kill;
    Mrpv1Bookmark b2; b2.id = 2; b2.matchId = 1; b2.type = Mrpv1BookmarkType::Objective;
    mrp.addBookmark(b1); mrp.addBookmark(b2);
    REQUIRE(mrp.countByBookmarkType(Mrpv1BookmarkType::Kill)      == 1);
    REQUIRE(mrp.countByBookmarkType(Mrpv1BookmarkType::Objective) == 1);
}

TEST_CASE("MatchReplayEditorV1 findMatch returns ptr", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    Mrpv1Match m; m.id = 9; m.name = "I";
    mrp.addMatch(m);
    REQUIRE(mrp.findMatch(9) != nullptr);
    REQUIRE(mrp.findMatch(9)->name == "I");
    REQUIRE(mrp.findMatch(99) == nullptr);
}

TEST_CASE("mrpv1MatchOutcomeName covers all values", "[Editor][S176]") {
    REQUIRE(std::string(mrpv1MatchOutcomeName(Mrpv1MatchOutcome::Win))       == "Win");
    REQUIRE(std::string(mrpv1MatchOutcomeName(Mrpv1MatchOutcome::Abandoned)) == "Abandoned");
}

TEST_CASE("mrpv1BookmarkTypeName covers all values", "[Editor][S176]") {
    REQUIRE(std::string(mrpv1BookmarkTypeName(Mrpv1BookmarkType::Kill))  == "Kill");
    REQUIRE(std::string(mrpv1BookmarkTypeName(Mrpv1BookmarkType::Error)) == "Error");
}

TEST_CASE("MatchReplayEditorV1 onChange callback", "[Editor][S176]") {
    MatchReplayEditorV1 mrp;
    uint64_t notified = 0;
    mrp.setOnChange([&](uint64_t id) { notified = id; });
    Mrpv1Match m; m.id = 3; m.name = "C";
    mrp.addMatch(m);
    mrp.setOutcome(3, Mrpv1MatchOutcome::Draw);
    REQUIRE(notified == 3);
}

// ── PlaytestRecorderV1 ───────────────────────────────────────────────────────

TEST_CASE("Ptrv1Session validity", "[Editor][S176]") {
    Ptrv1Session s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Playtest1";
    REQUIRE(s.isValid());
}

TEST_CASE("PlaytestRecorderV1 addSession and sessionCount", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    REQUIRE(ptr.sessionCount() == 0);
    Ptrv1Session s; s.id = 1; s.name = "S1";
    REQUIRE(ptr.addSession(s));
    REQUIRE(ptr.sessionCount() == 1);
}

TEST_CASE("PlaytestRecorderV1 addSession invalid fails", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    REQUIRE(!ptr.addSession(Ptrv1Session{}));
}

TEST_CASE("PlaytestRecorderV1 addSession duplicate fails", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Session s; s.id = 1; s.name = "A";
    ptr.addSession(s);
    REQUIRE(!ptr.addSession(s));
}

TEST_CASE("PlaytestRecorderV1 removeSession", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Session s; s.id = 2; s.name = "B";
    ptr.addSession(s);
    REQUIRE(ptr.removeSession(2));
    REQUIRE(ptr.sessionCount() == 0);
    REQUIRE(!ptr.removeSession(2));
}

TEST_CASE("PlaytestRecorderV1 addAnnotation and annotationCount", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Annotation a; a.id = 1; a.sessionId = 10; a.type = Ptrv1AnnotationType::Bug;
    REQUIRE(ptr.addAnnotation(a));
    REQUIRE(ptr.annotationCount() == 1);
}

TEST_CASE("PlaytestRecorderV1 addAnnotation invalid fails", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    REQUIRE(!ptr.addAnnotation(Ptrv1Annotation{}));
}

TEST_CASE("PlaytestRecorderV1 removeAnnotation", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Annotation a; a.id = 1; a.sessionId = 5;
    ptr.addAnnotation(a);
    REQUIRE(ptr.removeAnnotation(1));
    REQUIRE(ptr.annotationCount() == 0);
    REQUIRE(!ptr.removeAnnotation(1));
}

TEST_CASE("PlaytestRecorderV1 setState endedCount analyzedCount", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Session s1; s1.id = 1; s1.name = "A";
    Ptrv1Session s2; s2.id = 2; s2.name = "B";
    ptr.addSession(s1); ptr.addSession(s2);
    ptr.setState(1, Ptrv1SessionState::Ended);
    ptr.setState(2, Ptrv1SessionState::Analyzed);
    REQUIRE(ptr.endedCount()    == 1);
    REQUIRE(ptr.analyzedCount() == 1);
}

TEST_CASE("PlaytestRecorderV1 countByAnnotationType", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Annotation a1; a1.id = 1; a1.sessionId = 1; a1.type = Ptrv1AnnotationType::Bug;
    Ptrv1Annotation a2; a2.id = 2; a2.sessionId = 1; a2.type = Ptrv1AnnotationType::Crash;
    ptr.addAnnotation(a1); ptr.addAnnotation(a2);
    REQUIRE(ptr.countByAnnotationType(Ptrv1AnnotationType::Bug)   == 1);
    REQUIRE(ptr.countByAnnotationType(Ptrv1AnnotationType::Crash) == 1);
}

TEST_CASE("PlaytestRecorderV1 findSession returns ptr", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    Ptrv1Session s; s.id = 8; s.name = "H";
    ptr.addSession(s);
    REQUIRE(ptr.findSession(8) != nullptr);
    REQUIRE(ptr.findSession(8)->name == "H");
    REQUIRE(ptr.findSession(99) == nullptr);
}

TEST_CASE("ptrv1SessionStateName covers all values", "[Editor][S176]") {
    REQUIRE(std::string(ptrv1SessionStateName(Ptrv1SessionState::NotStarted)) == "NotStarted");
    REQUIRE(std::string(ptrv1SessionStateName(Ptrv1SessionState::Analyzed))   == "Analyzed");
}

TEST_CASE("ptrv1AnnotationTypeName covers all values", "[Editor][S176]") {
    REQUIRE(std::string(ptrv1AnnotationTypeName(Ptrv1AnnotationType::Bug)) == "Bug");
    REQUIRE(std::string(ptrv1AnnotationTypeName(Ptrv1AnnotationType::UX))  == "UX");
}

TEST_CASE("PlaytestRecorderV1 onChange callback", "[Editor][S176]") {
    PlaytestRecorderV1 ptr;
    uint64_t notified = 0;
    ptr.setOnChange([&](uint64_t id) { notified = id; });
    Ptrv1Session s; s.id = 6; s.name = "F";
    ptr.addSession(s);
    ptr.setState(6, Ptrv1SessionState::Running);
    REQUIRE(notified == 6);
}
