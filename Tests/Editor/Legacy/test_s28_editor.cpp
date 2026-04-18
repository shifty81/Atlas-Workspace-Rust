#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AnimationEditor.h"

using namespace NF;

TEST_CASE("KeyframeInterpolation names cover all 8 values", "[Editor][S28]") {
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Linear))      == "Linear");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Step))        == "Step");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Bezier))      == "Bezier");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::CubicSpline)) == "CubicSpline");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::EaseIn))      == "EaseIn");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::EaseOut))     == "EaseOut");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::EaseInOut))   == "EaseInOut");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Custom))      == "Custom");
}

TEST_CASE("AnimationTrackType names cover all 8 values", "[Editor][S28]") {
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Position)) == "Position");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Rotation)) == "Rotation");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Scale))    == "Scale");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Opacity))  == "Opacity");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Color))    == "Color");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Float))    == "Float");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Bool))     == "Bool");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Event))    == "Event");
}

TEST_CASE("Keyframe select and deselect toggles selected", "[Editor][S28]") {
    Keyframe kf;
    kf.time = 1.0f; kf.value = 0.5f;

    REQUIRE_FALSE(kf.selected);
    kf.select();
    REQUIRE(kf.selected);
    kf.deselect();
    REQUIRE_FALSE(kf.selected);
}

TEST_CASE("Keyframe setTime and setValue update fields", "[Editor][S28]") {
    Keyframe kf;
    kf.setTime(2.5f);
    kf.setValue(0.75f);

    REQUIRE(kf.time  == Catch::Approx(2.5f));
    REQUIRE(kf.value == Catch::Approx(0.75f));
}

TEST_CASE("AnimationTrack addKeyframe and duplicate time rejection", "[Editor][S28]") {
    AnimationTrack track("pos-x", AnimationTrackType::Position);

    Keyframe a; a.time = 0.0f; a.value = 0.f;
    Keyframe b; b.time = 1.0f; b.value = 1.f;
    Keyframe dup; dup.time = 0.0f; dup.value = 5.f;

    REQUIRE(track.addKeyframe(a));
    REQUIRE(track.addKeyframe(b));
    REQUIRE_FALSE(track.addKeyframe(dup));
    REQUIRE(track.keyframeCount() == 2);
}

TEST_CASE("AnimationTrack removeKeyframe returns correct result", "[Editor][S28]") {
    AnimationTrack track("opacity", AnimationTrackType::Opacity);

    Keyframe kf; kf.time = 0.5f; kf.value = 1.0f;
    track.addKeyframe(kf);

    REQUIRE(track.removeKeyframe(0.5f));
    REQUIRE_FALSE(track.removeKeyframe(0.5f));
    REQUIRE(track.keyframeCount() == 0);
}

TEST_CASE("AnimationTrack findKeyframe returns correct pointer or nullptr", "[Editor][S28]") {
    AnimationTrack track("scale", AnimationTrackType::Scale);

    Keyframe kf; kf.time = 2.0f; kf.value = 1.5f;
    track.addKeyframe(kf);

    REQUIRE(track.findKeyframe(2.0f) != nullptr);
    REQUIRE(track.findKeyframe(2.0f)->value == Catch::Approx(1.5f));
    REQUIRE(track.findKeyframe(9.9f) == nullptr);
}

TEST_CASE("AnimationTrack selectAll and deselectAll, selectedCount", "[Editor][S28]") {
    AnimationTrack track("rot", AnimationTrackType::Rotation);

    Keyframe a; a.time = 0.0f;
    Keyframe b; b.time = 1.0f;
    Keyframe c; c.time = 2.0f;
    track.addKeyframe(a);
    track.addKeyframe(b);
    track.addKeyframe(c);

    REQUIRE(track.selectedCount() == 0);
    track.selectAll();
    REQUIRE(track.selectedCount() == 3);
    track.deselectAll();
    REQUIRE(track.selectedCount() == 0);
}

TEST_CASE("AnimationTrack duration returns max keyframe time", "[Editor][S28]") {
    AnimationTrack track("float-track", AnimationTrackType::Float);

    Keyframe a; a.time = 0.0f;
    Keyframe b; b.time = 3.0f;
    Keyframe c; c.time = 1.5f;
    track.addKeyframe(a);
    track.addKeyframe(b);
    track.addKeyframe(c);

    REQUIRE(track.duration() == Catch::Approx(3.0f));
}

TEST_CASE("KeyframeAnimationEditor addTrack and duplicate rejection", "[Editor][S28]") {
    KeyframeAnimationEditor editor;

    AnimationTrack a("track-a", AnimationTrackType::Position);
    AnimationTrack b("track-b", AnimationTrackType::Rotation);
    AnimationTrack dup("track-a", AnimationTrackType::Scale);

    REQUIRE(editor.addTrack(a));
    REQUIRE(editor.addTrack(b));
    REQUIRE_FALSE(editor.addTrack(dup));
    REQUIRE(editor.trackCount() == 2);
}

TEST_CASE("KeyframeAnimationEditor removeTrack reduces count", "[Editor][S28]") {
    KeyframeAnimationEditor editor;

    AnimationTrack t("my-track", AnimationTrackType::Opacity);
    editor.addTrack(t);

    REQUIRE(editor.trackCount() == 1);
    REQUIRE(editor.removeTrack("my-track"));
    REQUIRE(editor.trackCount() == 0);
    REQUIRE_FALSE(editor.removeTrack("my-track"));
}

TEST_CASE("KeyframeAnimationEditor play/pause/stop controls isPlaying and playhead", "[Editor][S28]") {
    KeyframeAnimationEditor editor;

    REQUIRE_FALSE(editor.isPlaying());
    editor.play();
    REQUIRE(editor.isPlaying());
    editor.pause();
    REQUIRE_FALSE(editor.isPlaying());

    editor.setPlayhead(5.0f);
    REQUIRE(editor.playhead() == Catch::Approx(5.0f));
    editor.stop();
    REQUIRE_FALSE(editor.isPlaying());
    REQUIRE(editor.playhead() == Catch::Approx(0.0f));
}

TEST_CASE("KeyframeAnimationEditor totalDuration spans all tracks", "[Editor][S28]") {
    KeyframeAnimationEditor editor;

    AnimationTrack ta("ta", AnimationTrackType::Float);
    Keyframe ka; ka.time = 4.0f; ta.addKeyframe(ka);

    AnimationTrack tb("tb", AnimationTrackType::Float);
    Keyframe kb; kb.time = 7.0f; tb.addKeyframe(kb);

    editor.addTrack(ta);
    editor.addTrack(tb);

    REQUIRE(editor.totalDuration() == Catch::Approx(7.0f));
}

TEST_CASE("KeyframeAnimationEditor selectAll and deselectAll propagate to all tracks", "[Editor][S28]") {
    KeyframeAnimationEditor editor;

    AnimationTrack ta("ta", AnimationTrackType::Float);
    Keyframe ka; ka.time = 0.f; ta.addKeyframe(ka);

    AnimationTrack tb("tb", AnimationTrackType::Float);
    Keyframe kb; kb.time = 1.f; tb.addKeyframe(kb);

    editor.addTrack(ta);
    editor.addTrack(tb);

    editor.selectAllKeyframes();
    REQUIRE(editor.findTrack("ta")->selectedCount() == 1);
    REQUIRE(editor.findTrack("tb")->selectedCount() == 1);

    editor.deselectAllKeyframes();
    REQUIRE(editor.findTrack("ta")->selectedCount() == 0);
    REQUIRE(editor.findTrack("tb")->selectedCount() == 0);
}

TEST_CASE("KeyframeAnimationEditor MAX_TRACKS limit is enforced", "[Editor][S28]") {
    KeyframeAnimationEditor editor;

    for (size_t i = 0; i < KeyframeAnimationEditor::MAX_TRACKS; ++i) {
        AnimationTrack t("track-" + std::to_string(i), AnimationTrackType::Float);
        REQUIRE(editor.addTrack(t));
    }

    AnimationTrack overflow("overflow", AnimationTrackType::Float);
    REQUIRE_FALSE(editor.addTrack(overflow));
    REQUIRE(editor.trackCount() == KeyframeAnimationEditor::MAX_TRACKS);
}
