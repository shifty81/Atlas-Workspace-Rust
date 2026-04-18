// Tests/Editor/test_animation_editor_tool.cpp
// Tests for Phase 3 NF::AnimationEditorTool — fourth real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/AnimationEditorTool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// AnimationEditMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("animationEditModeName returns correct strings", "[AnimationEditMode]") {
    CHECK(std::string(animationEditModeName(AnimationEditMode::Timeline))    == "Timeline");
    CHECK(std::string(animationEditModeName(AnimationEditMode::Curves))      == "Curves");
    CHECK(std::string(animationEditModeName(AnimationEditMode::Retargeting)) == "Retargeting");
    CHECK(std::string(animationEditModeName(AnimationEditMode::Preview))     == "Preview");
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool descriptor is valid at construction", "[AnimationEditorTool][descriptor]") {
    AnimationEditorTool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == AnimationEditorTool::kToolId);
    CHECK(d.displayName == "Animation Editor");
    CHECK(d.category    == HostedToolCategory::AnimationAuthoring);
    CHECK(d.isPrimary   == true);
}

TEST_CASE("AnimationEditorTool toolId matches kToolId", "[AnimationEditorTool][descriptor]") {
    AnimationEditorTool tool;
    CHECK(tool.toolId() == std::string(AnimationEditorTool::kToolId));
    CHECK(tool.toolId() == "workspace.animation_editor");
}

TEST_CASE("AnimationEditorTool declares expected shared panels", "[AnimationEditorTool][descriptor]") {
    AnimationEditorTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 5);
    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    CHECK(has("panel.viewport_animation"));
    CHECK(has("panel.timeline"));
    CHECK(has("panel.outliner"));
    CHECK(has("panel.inspector"));
    CHECK(has("panel.console"));
}

TEST_CASE("AnimationEditorTool declares expected commands", "[AnimationEditorTool][descriptor]") {
    AnimationEditorTool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 7);
    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    CHECK(has("animation.play"));
    CHECK(has("animation.pause"));
    CHECK(has("animation.stop"));
    CHECK(has("animation.record"));
    CHECK(has("animation.add_keyframe"));
    CHECK(has("animation.delete_keyframe"));
    CHECK(has("animation.export_clip"));
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool initial state is Unloaded", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("AnimationEditorTool initialize transitions to Ready", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    CHECK(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AnimationEditorTool double initialize returns false", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    CHECK(tool.initialize());
    CHECK_FALSE(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AnimationEditorTool activate from Ready transitions to Active", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("AnimationEditorTool suspend from Active transitions to Suspended", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("AnimationEditorTool suspend stops playback and recording", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.play();
    tool.record(true);
    tool.suspend();
    CHECK_FALSE(tool.isPlaying());
    CHECK_FALSE(tool.isRecording());
}

TEST_CASE("AnimationEditorTool activate from Suspended transitions to Active", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("AnimationEditorTool shutdown from Active transitions to Unloaded", "[AnimationEditorTool][lifecycle]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — edit mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool default edit mode is Timeline", "[AnimationEditorTool][mode]") {
    AnimationEditorTool tool;
    tool.initialize();
    CHECK(tool.editMode() == AnimationEditMode::Timeline);
}

TEST_CASE("AnimationEditorTool setEditMode roundtrips all modes", "[AnimationEditorTool][mode]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.setEditMode(AnimationEditMode::Curves);
    CHECK(tool.editMode() == AnimationEditMode::Curves);
    tool.setEditMode(AnimationEditMode::Retargeting);
    CHECK(tool.editMode() == AnimationEditMode::Retargeting);
    tool.setEditMode(AnimationEditMode::Preview);
    CHECK(tool.editMode() == AnimationEditMode::Preview);
    tool.setEditMode(AnimationEditMode::Timeline);
    CHECK(tool.editMode() == AnimationEditMode::Timeline);
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — playback controls
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool initially not playing or recording", "[AnimationEditorTool][playback]") {
    AnimationEditorTool tool;
    tool.initialize();
    CHECK_FALSE(tool.isPlaying());
    CHECK_FALSE(tool.isRecording());
}

TEST_CASE("AnimationEditorTool play / pause cycle", "[AnimationEditorTool][playback]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.play();
    CHECK(tool.isPlaying());
    tool.pause();
    CHECK_FALSE(tool.isPlaying());
}

TEST_CASE("AnimationEditorTool stop clears playing and recording", "[AnimationEditorTool][playback]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.play();
    tool.record(true);
    tool.stop();
    CHECK_FALSE(tool.isPlaying());
    CHECK_FALSE(tool.isRecording());
}

TEST_CASE("AnimationEditorTool record toggle", "[AnimationEditorTool][playback]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.record(true);
    CHECK(tool.isRecording());
    tool.record(false);
    CHECK_FALSE(tool.isRecording());
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — dirty state
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool initially not dirty", "[AnimationEditorTool][dirty]") {
    AnimationEditorTool tool;
    tool.initialize();
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("AnimationEditorTool markDirty / clearDirty", "[AnimationEditorTool][dirty]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.markDirty();
    CHECK(tool.isDirty());
    tool.clearDirty();
    CHECK_FALSE(tool.isDirty());
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — stats
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool stats default to zero", "[AnimationEditorTool][stats]") {
    AnimationEditorTool tool;
    tool.initialize();
    const auto& s = tool.stats();
    CHECK(s.clipDurationMs    == 0.0f);
    CHECK(s.frameCount        == 0u);
    CHECK(s.selectedBoneCount == 0u);
    CHECK(s.isPlaying         == false);
    CHECK(s.isRecording       == false);
    CHECK(s.isDirty           == false);
}

TEST_CASE("AnimationEditorTool setClipDurationMs roundtrips", "[AnimationEditorTool][stats]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.setClipDurationMs(3333.0f);
    CHECK(tool.stats().clipDurationMs == 3333.0f);
}

TEST_CASE("AnimationEditorTool setFrameCount roundtrips", "[AnimationEditorTool][stats]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.setFrameCount(120);
    CHECK(tool.stats().frameCount == 120u);
}

TEST_CASE("AnimationEditorTool setSelectedBoneCount roundtrips", "[AnimationEditorTool][stats]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.setSelectedBoneCount(5);
    CHECK(tool.stats().selectedBoneCount == 5u);
}

// ─────────────────────────────────────────────────────────────────
// AnimationEditorTool — project hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AnimationEditorTool onProjectLoaded clears stats", "[AnimationEditorTool][project]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.setFrameCount(60);
    tool.markDirty();
    tool.onProjectLoaded("nova_forge");
    CHECK(tool.stats().frameCount == 0u);
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("AnimationEditorTool onProjectUnloaded clears stats", "[AnimationEditorTool][project]") {
    AnimationEditorTool tool;
    tool.initialize();
    tool.setFrameCount(60);
    tool.onProjectLoaded("nova_forge");
    tool.onProjectUnloaded();
    CHECK(tool.stats().frameCount == 0u);
}
