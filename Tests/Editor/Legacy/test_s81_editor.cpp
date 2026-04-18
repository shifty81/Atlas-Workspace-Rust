#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AnimationEditor.h"
#include "NF/Editor/AnimBlueprintEditor.h"

using namespace NF;

// ── S81: AnimBlueprintEditor + AnimationEditor ───────────────────

TEST_CASE("AnimBPNodeType names are correct", "[Editor][S81]") {
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::StateMachine)) == "StateMachine");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::BlendSpace))   == "BlendSpace");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::Selector))     == "Selector");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::Sequence))     == "Sequence");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::Pose))         == "Pose");
}

TEST_CASE("AnimBPState names are correct", "[Editor][S81]") {
    REQUIRE(std::string(animBPStateName(AnimBPState::Inactive))  == "Inactive");
    REQUIRE(std::string(animBPStateName(AnimBPState::Compiling)) == "Compiling");
    REQUIRE(std::string(animBPStateName(AnimBPState::Ready))     == "Ready");
    REQUIRE(std::string(animBPStateName(AnimBPState::Running))   == "Running");
    REQUIRE(std::string(animBPStateName(AnimBPState::Error))     == "Error");
}

TEST_CASE("AnimBPBlendMode names are correct", "[Editor][S81]") {
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Override)) == "Override");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Layered))  == "Layered");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Masked))   == "Masked");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Blended))  == "Blended");
}

TEST_CASE("AnimBlueprintAsset default state is Inactive", "[Editor][S81]") {
    AnimBlueprintAsset bp("HeroAnim", 5, 2);
    REQUIRE(bp.name() == "HeroAnim");
    REQUIRE(bp.state() == AnimBPState::Inactive);
    REQUIRE(bp.nodeType() == AnimBPNodeType::StateMachine);
    REQUIRE(bp.blendMode() == AnimBPBlendMode::Override);
    REQUIRE_FALSE(bp.isRunning());
    REQUIRE_FALSE(bp.hasError());
    REQUIRE_FALSE(bp.isReady());
}

TEST_CASE("AnimBlueprintAsset setState predicates", "[Editor][S81]") {
    AnimBlueprintAsset bp("Hero", 0, 0);
    bp.setState(AnimBPState::Running);
    REQUIRE(bp.isRunning());
    bp.setState(AnimBPState::Error);
    REQUIRE(bp.hasError());
    bp.setState(AnimBPState::Ready);
    REQUIRE(bp.isReady());
}

TEST_CASE("AnimBlueprintAsset isComplex for nodeCount >= 20", "[Editor][S81]") {
    AnimBlueprintAsset bp("X", 5, 1);
    REQUIRE_FALSE(bp.isComplex());
    bp.setNodeCount(25);
    REQUIRE(bp.isComplex());
}

TEST_CASE("AnimBlueprintEditor addBlueprint and blueprintCount", "[Editor][S81]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset bp("Walk", 3, 1);
    REQUIRE(editor.addBlueprint(bp));
    REQUIRE(editor.blueprintCount() == 1);
}

TEST_CASE("AnimBlueprintEditor addBlueprint rejects duplicate name", "[Editor][S81]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset bp("Walk", 3, 1);
    editor.addBlueprint(bp);
    REQUIRE_FALSE(editor.addBlueprint(bp));
}

TEST_CASE("AnimBlueprintEditor removeBlueprint removes entry", "[Editor][S81]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset bp("Run", 2, 1);
    editor.addBlueprint(bp);
    REQUIRE(editor.removeBlueprint("Run"));
    REQUIRE(editor.blueprintCount() == 0);
}

TEST_CASE("AnimBlueprintEditor setActiveBlueprint updates active", "[Editor][S81]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset bp("Jump", 4, 2);
    editor.addBlueprint(bp);
    REQUIRE(editor.setActiveBlueprint("Jump"));
    REQUIRE(editor.activeBlueprint() == "Jump");
}

TEST_CASE("AnimBlueprintEditor countByNodeType", "[Editor][S81]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset a("a", 1, 1); a.setNodeType(AnimBPNodeType::StateMachine);
    AnimBlueprintAsset b("b", 1, 1); b.setNodeType(AnimBPNodeType::BlendSpace);
    AnimBlueprintAsset c("c", 1, 1); c.setNodeType(AnimBPNodeType::StateMachine);
    editor.addBlueprint(a); editor.addBlueprint(b); editor.addBlueprint(c);
    REQUIRE(editor.countByNodeType(AnimBPNodeType::StateMachine) == 2);
    REQUIRE(editor.countByNodeType(AnimBPNodeType::BlendSpace) == 1);
}

TEST_CASE("AnimBlueprintEditor dirtyCount and runningCount", "[Editor][S81]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset a("a", 1, 1); a.setDirty(true);
    AnimBlueprintAsset b("b", 1, 1); b.setState(AnimBPState::Running);
    AnimBlueprintAsset c("c", 1, 1);
    editor.addBlueprint(a); editor.addBlueprint(b); editor.addBlueprint(c);
    REQUIRE(editor.dirtyCount() == 1);
    REQUIRE(editor.runningCount() == 1);
}

TEST_CASE("AnimBlueprintEditor MAX_BLUEPRINTS is 256", "[Editor][S81]") {
    REQUIRE(AnimBlueprintEditor::MAX_BLUEPRINTS == 256);
}

// ── AnimationEditor ──────────────────────────────────────────────

TEST_CASE("KeyframeInterpolation names are correct", "[Editor][S81]") {
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Linear))      == "Linear");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Step))        == "Step");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Bezier))      == "Bezier");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::CubicSpline)) == "CubicSpline");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::EaseIn))      == "EaseIn");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::EaseOut))     == "EaseOut");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::EaseInOut))   == "EaseInOut");
    REQUIRE(std::string(keyframeInterpolationName(KeyframeInterpolation::Custom))      == "Custom");
}

TEST_CASE("AnimationTrackType names are correct", "[Editor][S81]") {
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Position)) == "Position");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Rotation)) == "Rotation");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Scale))    == "Scale");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Opacity))  == "Opacity");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Color))    == "Color");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Float))    == "Float");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Bool))     == "Bool");
    REQUIRE(std::string(animationTrackTypeName(AnimationTrackType::Event))    == "Event");
}

TEST_CASE("Keyframe select and deselect", "[Editor][S81]") {
    Keyframe kf;
    kf.setTime(1.5f);
    kf.setValue(0.8f);
    REQUIRE_FALSE(kf.selected);
    kf.select();
    REQUIRE(kf.selected);
    kf.deselect();
    REQUIRE_FALSE(kf.selected);
    REQUIRE(kf.time == 1.5f);
    REQUIRE(kf.value == 0.8f);
}

TEST_CASE("AnimationTrack addKeyframe and keyframeCount", "[Editor][S81]") {
    AnimationTrack track("pos", AnimationTrackType::Position);
    Keyframe kf; kf.setTime(0.0f); kf.setValue(0.0f);
    REQUIRE(track.addKeyframe(kf));
    REQUIRE(track.keyframeCount() == 1);
}

TEST_CASE("AnimationTrack addKeyframe rejects duplicate time", "[Editor][S81]") {
    AnimationTrack track("pos", AnimationTrackType::Position);
    Keyframe kf; kf.setTime(1.0f);
    track.addKeyframe(kf);
    REQUIRE_FALSE(track.addKeyframe(kf));
    REQUIRE(track.keyframeCount() == 1);
}

TEST_CASE("AnimationTrack removeKeyframe", "[Editor][S81]") {
    AnimationTrack track("pos", AnimationTrackType::Position);
    Keyframe kf; kf.setTime(0.5f);
    track.addKeyframe(kf);
    REQUIRE(track.removeKeyframe(0.5f));
    REQUIRE(track.keyframeCount() == 0);
}

TEST_CASE("AnimationTrack selectAll and deselectAll", "[Editor][S81]") {
    AnimationTrack track("float", AnimationTrackType::Float);
    Keyframe k1; k1.setTime(0.f);
    Keyframe k2; k2.setTime(1.f);
    track.addKeyframe(k1); track.addKeyframe(k2);
    track.selectAll();
    REQUIRE(track.selectedCount() == 2);
    track.deselectAll();
    REQUIRE(track.selectedCount() == 0);
}

TEST_CASE("AnimationTrack duration is max keyframe time", "[Editor][S81]") {
    AnimationTrack track("scale", AnimationTrackType::Scale);
    Keyframe k1; k1.setTime(0.f);
    Keyframe k2; k2.setTime(3.0f);
    Keyframe k3; k3.setTime(1.5f);
    track.addKeyframe(k1); track.addKeyframe(k2); track.addKeyframe(k3);
    REQUIRE(track.duration() == 3.0f);
}

TEST_CASE("KeyframeAnimationEditor addTrack and trackCount", "[Editor][S81]") {
    KeyframeAnimationEditor editor;
    AnimationTrack track("pos", AnimationTrackType::Position);
    REQUIRE(editor.addTrack(track));
    REQUIRE(editor.trackCount() == 1);
}

TEST_CASE("KeyframeAnimationEditor removeTrack", "[Editor][S81]") {
    KeyframeAnimationEditor editor;
    AnimationTrack track("rot", AnimationTrackType::Rotation);
    editor.addTrack(track);
    REQUIRE(editor.removeTrack("rot"));
    REQUIRE(editor.trackCount() == 0);
}

TEST_CASE("KeyframeAnimationEditor play pause stop", "[Editor][S81]") {
    KeyframeAnimationEditor editor;
    editor.setPlayhead(2.0f);
    REQUIRE_FALSE(editor.isPlaying());
    editor.play();
    REQUIRE(editor.isPlaying());
    editor.pause();
    REQUIRE_FALSE(editor.isPlaying());
    REQUIRE(editor.playhead() == 2.0f);
    editor.play();
    editor.stop();
    REQUIRE_FALSE(editor.isPlaying());
    REQUIRE(editor.playhead() == 0.0f);
}

TEST_CASE("KeyframeAnimationEditor totalDuration is max track duration", "[Editor][S81]") {
    KeyframeAnimationEditor editor;
    AnimationTrack t1("pos", AnimationTrackType::Position);
    Keyframe k1; k1.setTime(5.0f); t1.addKeyframe(k1);
    AnimationTrack t2("rot", AnimationTrackType::Rotation);
    Keyframe k2; k2.setTime(3.0f); t2.addKeyframe(k2);
    editor.addTrack(t1); editor.addTrack(t2);
    REQUIRE(editor.totalDuration() == 5.0f);
}

TEST_CASE("KeyframeAnimationEditor MAX_TRACKS is 64", "[Editor][S81]") {
    REQUIRE(KeyframeAnimationEditor::MAX_TRACKS == 64);
}
