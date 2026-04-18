#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TimelineEditor.h"

using namespace NF;

TEST_CASE("TimelineEventType names cover all 8 values", "[Editor][S31]") {
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Keyframe)) == "Keyframe");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Marker))   == "Marker");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Clip))     == "Clip");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Trigger))  == "Trigger");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Label))    == "Label");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Camera))   == "Camera");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Audio))    == "Audio");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Custom))   == "Custom");
}

TEST_CASE("TimelineTrackKind names cover all 4 values", "[Editor][S31]") {
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Animation)) == "Animation");
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Audio))     == "Audio");
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Event))     == "Event");
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Camera))    == "Camera");
}

TEST_CASE("TimelineEvent select and deselect toggles selected", "[Editor][S31]") {
    TimelineEvent ev;
    ev.id = "ev1"; ev.time = 1.0f;

    REQUIRE_FALSE(ev.selected);
    ev.select();
    REQUIRE(ev.selected);
    ev.deselect();
    REQUIRE_FALSE(ev.selected);
}

TEST_CASE("TimelineEvent setTime and setDuration update fields", "[Editor][S31]") {
    TimelineEvent ev;
    ev.setTime(3.5f);
    ev.setDuration(2.0f);

    REQUIRE(ev.time     == Catch::Approx(3.5f));
    REQUIRE(ev.duration == Catch::Approx(2.0f));
}

TEST_CASE("TimelineTrack addEvent and duplicate id rejection", "[Editor][S31]") {
    TimelineTrack track("anim-track", TimelineTrackKind::Animation);

    TimelineEvent a; a.id = "ev-a"; a.time = 0.0f;
    TimelineEvent b; b.id = "ev-b"; b.time = 1.0f;
    TimelineEvent dup; dup.id = "ev-a"; dup.time = 2.0f;

    REQUIRE(track.addEvent(a));
    REQUIRE(track.addEvent(b));
    REQUIRE_FALSE(track.addEvent(dup));
    REQUIRE(track.eventCount() == 2);
}

TEST_CASE("TimelineTrack removeEvent returns correct result", "[Editor][S31]") {
    TimelineTrack track("audio-track", TimelineTrackKind::Audio);

    TimelineEvent ev; ev.id = "clip-1";
    track.addEvent(ev);

    REQUIRE(track.removeEvent("clip-1"));
    REQUIRE_FALSE(track.removeEvent("clip-1"));
    REQUIRE(track.eventCount() == 0);
}

TEST_CASE("TimelineTrack findEvent returns correct pointer or nullptr", "[Editor][S31]") {
    TimelineTrack track("event-track", TimelineTrackKind::Event);

    TimelineEvent ev; ev.id = "trigger-1"; ev.time = 5.0f;
    track.addEvent(ev);

    REQUIRE(track.findEvent("trigger-1") != nullptr);
    REQUIRE(track.findEvent("trigger-1")->time == Catch::Approx(5.0f));
    REQUIRE(track.findEvent("missing") == nullptr);
}

TEST_CASE("TimelineTrack selectAll and deselectAll update selectedCount", "[Editor][S31]") {
    TimelineTrack track("cam-track", TimelineTrackKind::Camera);

    TimelineEvent a; a.id = "a";
    TimelineEvent b; b.id = "b";
    TimelineEvent c; c.id = "c";
    track.addEvent(a);
    track.addEvent(b);
    track.addEvent(c);

    REQUIRE(track.selectedCount() == 0);
    track.selectAll();
    REQUIRE(track.selectedCount() == 3);
    track.deselectAll();
    REQUIRE(track.selectedCount() == 0);
}

TEST_CASE("TimelineTrack duration returns max event end time", "[Editor][S31]") {
    TimelineTrack track("dur-track", TimelineTrackKind::Animation);

    TimelineEvent a; a.id = "a"; a.time = 0.0f; a.duration = 2.0f;
    TimelineEvent b; b.id = "b"; b.time = 3.0f; b.duration = 4.0f; // ends at 7
    TimelineEvent c; c.id = "c"; c.time = 1.0f; c.duration = 1.0f;
    track.addEvent(a);
    track.addEvent(b);
    track.addEvent(c);

    REQUIRE(track.duration() == Catch::Approx(7.0f));
}

TEST_CASE("TimelineTrack muted flag set and read", "[Editor][S31]") {
    TimelineTrack track("mute-track", TimelineTrackKind::Audio);

    REQUIRE_FALSE(track.muted());
    track.setMuted(true);
    REQUIRE(track.muted());
    track.setMuted(false);
    REQUIRE_FALSE(track.muted());
}

TEST_CASE("TimelineEditorPanel addTrack and duplicate name rejection", "[Editor][S31]") {
    TimelineEditorPanel panel;

    TimelineTrack ta("alpha", TimelineTrackKind::Animation);
    TimelineTrack tb("beta",  TimelineTrackKind::Audio);
    TimelineTrack dup("alpha", TimelineTrackKind::Event);

    REQUIRE(panel.addTrack(ta));
    REQUIRE(panel.addTrack(tb));
    REQUIRE_FALSE(panel.addTrack(dup));
    REQUIRE(panel.trackCount() == 2);
}

TEST_CASE("TimelineEditorPanel removeTrack reduces count", "[Editor][S31]") {
    TimelineEditorPanel panel;

    TimelineTrack t("my-track", TimelineTrackKind::Camera);
    panel.addTrack(t);

    REQUIRE(panel.trackCount() == 1);
    REQUIRE(panel.removeTrack("my-track"));
    REQUIRE(panel.trackCount() == 0);
    REQUIRE_FALSE(panel.removeTrack("my-track"));
}

TEST_CASE("TimelineEditorPanel setActiveTrack and activeTrack", "[Editor][S31]") {
    TimelineEditorPanel panel;

    TimelineTrack ta("motion", TimelineTrackKind::Animation);
    TimelineTrack tb("sound",  TimelineTrackKind::Audio);
    panel.addTrack(ta);
    panel.addTrack(tb);

    REQUIRE(panel.activeTrack().empty());
    REQUIRE(panel.setActiveTrack("motion"));
    REQUIRE(panel.activeTrack() == "motion");
    REQUIRE_FALSE(panel.setActiveTrack("nonexistent"));
    REQUIRE(panel.activeTrack() == "motion"); // unchanged
}

TEST_CASE("TimelineEditorPanel play pause stop controls", "[Editor][S31]") {
    TimelineEditorPanel panel;

    panel.setPlayhead(5.0f);
    REQUIRE(panel.playhead() == Catch::Approx(5.0f));
    REQUIRE_FALSE(panel.isPlaying());

    panel.play();
    REQUIRE(panel.isPlaying());

    panel.pause();
    REQUIRE_FALSE(panel.isPlaying());
    REQUIRE(panel.playhead() == Catch::Approx(5.0f)); // preserved on pause

    panel.play();
    panel.stop();
    REQUIRE_FALSE(panel.isPlaying());
    REQUIRE(panel.playhead() == Catch::Approx(0.0f)); // reset on stop
}

TEST_CASE("TimelineEditorPanel selectAllEvents and deselectAllEvents propagate to all tracks", "[Editor][S31]") {
    TimelineEditorPanel panel;

    TimelineTrack ta("ta", TimelineTrackKind::Animation);
    TimelineEvent p1; p1.id = "p1"; ta.addEvent(p1);

    TimelineTrack tb("tb", TimelineTrackKind::Audio);
    TimelineEvent p2; p2.id = "p2"; tb.addEvent(p2);

    panel.addTrack(ta);
    panel.addTrack(tb);

    panel.selectAllEvents();
    REQUIRE(panel.findTrack("ta")->selectedCount() == 1);
    REQUIRE(panel.findTrack("tb")->selectedCount() == 1);

    panel.deselectAllEvents();
    REQUIRE(panel.findTrack("ta")->selectedCount() == 0);
    REQUIRE(panel.findTrack("tb")->selectedCount() == 0);
}

TEST_CASE("TimelineEditorPanel MAX_TRACKS limit is enforced", "[Editor][S31]") {
    TimelineEditorPanel panel;

    for (size_t i = 0; i < TimelineEditorPanel::MAX_TRACKS; ++i) {
        TimelineTrack t("track-" + std::to_string(i), TimelineTrackKind::Animation);
        REQUIRE(panel.addTrack(t));
    }

    TimelineTrack overflow("overflow", TimelineTrackKind::Event);
    REQUIRE_FALSE(panel.addTrack(overflow));
    REQUIRE(panel.trackCount() == TimelineEditorPanel::MAX_TRACKS);
}

TEST_CASE("TimelineEditorPanel removeTrack clears activeTrack if active", "[Editor][S31]") {
    TimelineEditorPanel panel;

    TimelineTrack t("active-track", TimelineTrackKind::Event);
    panel.addTrack(t);
    panel.setActiveTrack("active-track");
    REQUIRE(panel.activeTrack() == "active-track");

    panel.removeTrack("active-track");
    REQUIRE(panel.activeTrack().empty());
}
