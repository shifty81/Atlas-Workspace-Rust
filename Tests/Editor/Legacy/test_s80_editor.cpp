#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/VideoClipEditor.h"
#include "NF/Editor/AudioClipEditor.h"

using namespace NF;

// ── S80: VoxelPaint + AudioClipEditor + VideoClipEditor ──────────

// ── VoxelPaint ───────────────────────────────────────────────────

TEST_CASE("VoxelBrushShape enum has expected values", "[Editor][S80]") {
    VoxelBrushShape s = VoxelBrushShape::Sphere;
    REQUIRE(s == VoxelBrushShape::Sphere);
    s = VoxelBrushShape::Cube;
    REQUIRE(s == VoxelBrushShape::Cube);
    s = VoxelBrushShape::Cylinder;
    REQUIRE(s == VoxelBrushShape::Cylinder);
}

TEST_CASE("VoxelBrushSettings default values", "[Editor][S80]") {
    VoxelBrushSettings b;
    REQUIRE(b.shape == VoxelBrushShape::Sphere);
    REQUIRE(b.radius == 1);
    REQUIRE(b.materialId == 0);
    REQUIRE(b.strength == 1.0f);
}

TEST_CASE("VoxelPaintTool starts empty", "[Editor][S80]") {
    VoxelPaintTool tool;
    REQUIRE(tool.strokeCount() == 0);
    REQUIRE_FALSE(tool.isStroking());
}

TEST_CASE("VoxelPaintTool setBrush updates brush", "[Editor][S80]") {
    VoxelPaintTool tool;
    VoxelBrushSettings b;
    b.shape = VoxelBrushShape::Cube;
    b.radius = 3;
    b.materialId = 5;
    tool.setBrush(b);
    REQUIRE(tool.brush().shape == VoxelBrushShape::Cube);
    REQUIRE(tool.brush().radius == 3);
    REQUIRE(tool.brush().materialId == 5);
}

TEST_CASE("VoxelPaintTool beginStroke sets isStroking true", "[Editor][S80]") {
    VoxelPaintTool tool;
    tool.beginStroke();
    REQUIRE(tool.isStroking());
}

TEST_CASE("VoxelPaintTool addToStroke adds positions while stroking", "[Editor][S80]") {
    VoxelPaintTool tool;
    tool.beginStroke();
    tool.addToStroke({1, 0, 0});
    tool.addToStroke({2, 0, 0});
    tool.addToStroke({3, 0, 0});
    // Still stroking, no committed strokes yet
    REQUIRE(tool.strokeCount() == 0);
    REQUIRE(tool.isStroking());
}

TEST_CASE("VoxelPaintTool endStroke commits stroke", "[Editor][S80]") {
    VoxelPaintTool tool;
    tool.beginStroke();
    tool.addToStroke({0, 0, 0});
    tool.endStroke();
    REQUIRE(tool.strokeCount() == 1);
    REQUIRE_FALSE(tool.isStroking());
}

TEST_CASE("VoxelPaintTool addToStroke does nothing when not stroking", "[Editor][S80]") {
    VoxelPaintTool tool;
    tool.addToStroke({1, 2, 3});
    REQUIRE(tool.strokeCount() == 0);
}

TEST_CASE("VoxelPaintTool addStroke adds a stroke directly", "[Editor][S80]") {
    VoxelPaintTool tool;
    PaintStroke stroke;
    stroke.positions = {{1,0,0}, {2,0,0}};
    stroke.materialId = 3;
    tool.addStroke(stroke);
    REQUIRE(tool.strokeCount() == 1);
}

TEST_CASE("VoxelPaintTool removeLastStroke removes last stroke", "[Editor][S80]") {
    VoxelPaintTool tool;
    PaintStroke stroke;
    stroke.positions = {{0,0,0}};
    tool.addStroke(stroke);
    REQUIRE(tool.removeLastStroke());
    REQUIRE(tool.strokeCount() == 0);
}

TEST_CASE("VoxelPaintTool removeLastStroke returns false when empty", "[Editor][S80]") {
    VoxelPaintTool tool;
    REQUIRE_FALSE(tool.removeLastStroke());
}

TEST_CASE("VoxelPaintTool clear removes all strokes", "[Editor][S80]") {
    VoxelPaintTool tool;
    PaintStroke s1, s2;
    tool.addStroke(s1);
    tool.addStroke(s2);
    tool.clear();
    REQUIRE(tool.strokeCount() == 0);
    REQUIRE_FALSE(tool.isStroking());
}

TEST_CASE("VoxelPaintTool setPaletteSlot and getPaletteSlot", "[Editor][S80]") {
    VoxelPaintTool tool;
    tool.setPaletteSlot(0, 42);
    tool.setPaletteSlot(3, 99);
    REQUIRE(tool.getPaletteSlot(0) == 42);
    REQUIRE(tool.getPaletteSlot(3) == 99);
    REQUIRE(tool.getPaletteSlot(10) == 0); // out-of-range returns 0
}

TEST_CASE("VoxelPaintTool setActivePaletteSlot updates brush materialId", "[Editor][S80]") {
    VoxelPaintTool tool;
    tool.setPaletteSlot(1, 77);
    tool.setActivePaletteSlot(1);
    REQUIRE(tool.activePaletteSlot() == 1);
    REQUIRE(tool.brush().materialId == 77);
}

TEST_CASE("VoxelPaintTool kMaxPaletteSize is 32", "[Editor][S80]") {
    REQUIRE(VoxelPaintTool::kMaxPaletteSize == 32);
}

// ── AudioClipEditor ──────────────────────────────────────────────

TEST_CASE("AudioClipFormat names are correct", "[Editor][S80]") {
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::WAV))  == "WAV");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::OGG))  == "OGG");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::MP3))  == "MP3");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::FLAC)) == "FLAC");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::AIFF)) == "AIFF");
}

TEST_CASE("AudioClipState names are correct", "[Editor][S80]") {
    REQUIRE(std::string(audioClipStateName(AudioClipState::Idle))     == "Idle");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Playing))  == "Playing");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Paused))   == "Paused");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Stopped))  == "Stopped");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Finished)) == "Finished");
}

TEST_CASE("AudioLoopMode names are correct", "[Editor][S80]") {
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::None))      == "None");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::Loop))      == "Loop");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::PingPong))  == "PingPong");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::LoopPoint)) == "LoopPoint");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::Shuffle))   == "Shuffle");
}

TEST_CASE("AudioClipAsset default format is WAV and state is Idle", "[Editor][S80]") {
    AudioClipAsset clip("gunshot", 0.5f, 44100);
    REQUIRE(clip.name() == "gunshot");
    REQUIRE(clip.format() == AudioClipFormat::WAV);
    REQUIRE(clip.state() == AudioClipState::Idle);
    REQUIRE(clip.loopMode() == AudioLoopMode::None);
    REQUIRE_FALSE(clip.isLooping());
}

TEST_CASE("AudioClipAsset isPlaying, isPaused, isFinished predicates", "[Editor][S80]") {
    AudioClipAsset clip("bg_music", 60.f, 44100);
    clip.setState(AudioClipState::Playing);
    REQUIRE(clip.isPlaying());
    REQUIRE_FALSE(clip.isPaused());
    clip.setState(AudioClipState::Paused);
    REQUIRE(clip.isPaused());
    clip.setState(AudioClipState::Finished);
    REQUIRE(clip.isFinished());
}

TEST_CASE("AudioClipAsset isLooping when loopMode is not None", "[Editor][S80]") {
    AudioClipAsset clip("track", 10.f, 44100);
    clip.setLoopMode(AudioLoopMode::Loop);
    REQUIRE(clip.isLooping());
}

TEST_CASE("AudioClipEditor addClip and clipCount", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset clip("shot", 0.5f, 44100);
    REQUIRE(editor.addClip(clip));
    REQUIRE(editor.clipCount() == 1);
}

TEST_CASE("AudioClipEditor addClip rejects duplicate name", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset clip("shot", 0.5f, 44100);
    editor.addClip(clip);
    REQUIRE_FALSE(editor.addClip(clip));
}

TEST_CASE("AudioClipEditor removeClip removes entry", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset clip("shot", 0.5f, 44100);
    editor.addClip(clip);
    REQUIRE(editor.removeClip("shot"));
    REQUIRE(editor.clipCount() == 0);
}

TEST_CASE("AudioClipEditor setActiveClip updates activeClip", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset clip("track", 10.f, 44100);
    editor.addClip(clip);
    REQUIRE(editor.setActiveClip("track"));
    REQUIRE(editor.activeClip() == "track");
}

TEST_CASE("AudioClipEditor dirtyCount and loopingCount", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset a("a", 1.f, 44100); a.setDirty(true);
    AudioClipAsset b("b", 2.f, 44100); b.setLoopMode(AudioLoopMode::Loop);
    AudioClipAsset c("c", 3.f, 44100);
    editor.addClip(a); editor.addClip(b); editor.addClip(c);
    REQUIRE(editor.dirtyCount() == 1);
    REQUIRE(editor.loopingCount() == 1);
}

TEST_CASE("AudioClipEditor playingCount and streamingCount", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset a("a", 1.f, 44100); a.setState(AudioClipState::Playing); a.setStreaming(true);
    AudioClipAsset b("b", 2.f, 44100); b.setState(AudioClipState::Paused);
    editor.addClip(a); editor.addClip(b);
    REQUIRE(editor.playingCount() == 1);
    REQUIRE(editor.streamingCount() == 1);
}

TEST_CASE("AudioClipEditor countByFormat", "[Editor][S80]") {
    AudioClipEditor editor;
    AudioClipAsset a("a", 1.f, 44100); a.setFormat(AudioClipFormat::OGG);
    AudioClipAsset b("b", 2.f, 44100); b.setFormat(AudioClipFormat::OGG);
    AudioClipAsset c("c", 3.f, 44100); c.setFormat(AudioClipFormat::WAV);
    editor.addClip(a); editor.addClip(b); editor.addClip(c);
    REQUIRE(editor.countByFormat(AudioClipFormat::OGG) == 2);
    REQUIRE(editor.countByFormat(AudioClipFormat::WAV) == 1);
}

TEST_CASE("AudioClipEditor MAX_CLIPS is 512", "[Editor][S80]") {
    REQUIRE(AudioClipEditor::MAX_CLIPS == 512);
}

// ── VideoClipEditor ──────────────────────────────────────────────

TEST_CASE("VideoClipCodec names are correct", "[Editor][S80]") {
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::H264)) == "H264");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::H265)) == "H265");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::VP8))  == "VP8");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::VP9))  == "VP9");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::AV1))  == "AV1");
}

TEST_CASE("VideoClipState names are correct", "[Editor][S80]") {
    REQUIRE(std::string(videoClipStateName(VideoClipState::Idle))     == "Idle");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Playing))  == "Playing");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Paused))   == "Paused");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Stopped))  == "Stopped");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Finished)) == "Finished");
}

TEST_CASE("VideoAspectRatio names are correct", "[Editor][S80]") {
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio4x3))  == "4x3");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio16x9)) == "16x9");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio21x9)) == "21x9");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio1x1))  == "1x1");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Custom))    == "Custom");
}

TEST_CASE("VideoClipAsset default codec is H264 and state is Idle", "[Editor][S80]") {
    VideoClipAsset clip("intro", 5.f, 30);
    REQUIRE(clip.name() == "intro");
    REQUIRE(clip.codec() == VideoClipCodec::H264);
    REQUIRE(clip.state() == VideoClipState::Idle);
    REQUIRE(clip.fps() == 30);
}

TEST_CASE("VideoClipAsset default dimensions are 1920x1080", "[Editor][S80]") {
    VideoClipAsset clip("intro", 5.f, 30);
    REQUIRE(clip.width()  == 1920);
    REQUIRE(clip.height() == 1080);
    REQUIRE(clip.isHD());
}

TEST_CASE("VideoClipAsset isHD false for low resolution", "[Editor][S80]") {
    VideoClipAsset clip("thumb", 1.f, 30);
    clip.setWidth(640);
    clip.setHeight(480);
    REQUIRE_FALSE(clip.isHD());
}

TEST_CASE("VideoClipAsset isPlaying, isPaused, isFinished predicates", "[Editor][S80]") {
    VideoClipAsset clip("movie", 120.f, 24);
    clip.setState(VideoClipState::Playing);
    REQUIRE(clip.isPlaying());
    clip.setState(VideoClipState::Paused);
    REQUIRE(clip.isPaused());
    clip.setState(VideoClipState::Finished);
    REQUIRE(clip.isFinished());
}

TEST_CASE("VideoClipEditor addClip and clipCount", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset clip("intro", 5.f, 30);
    REQUIRE(editor.addClip(clip));
    REQUIRE(editor.clipCount() == 1);
}

TEST_CASE("VideoClipEditor addClip rejects duplicate name", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset clip("intro", 5.f, 30);
    editor.addClip(clip);
    REQUIRE_FALSE(editor.addClip(clip));
}

TEST_CASE("VideoClipEditor removeClip removes entry", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset clip("intro", 5.f, 30);
    editor.addClip(clip);
    REQUIRE(editor.removeClip("intro"));
    REQUIRE(editor.clipCount() == 0);
}

TEST_CASE("VideoClipEditor setActiveClip updates activeClip", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset clip("main", 10.f, 25);
    editor.addClip(clip);
    REQUIRE(editor.setActiveClip("main"));
    REQUIRE(editor.activeClip() == "main");
}

TEST_CASE("VideoClipEditor hdCount counts HD clips", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset hd("hd", 5.f, 30); // defaults 1920x1080
    VideoClipAsset sd("sd", 5.f, 30); sd.setWidth(320); sd.setHeight(240);
    editor.addClip(hd);
    editor.addClip(sd);
    REQUIRE(editor.hdCount() == 1);
}

TEST_CASE("VideoClipEditor loopingCount and streamingCount", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset a("a", 1.f, 30); a.setLooping(true); a.setStreaming(true);
    VideoClipAsset b("b", 2.f, 30);
    editor.addClip(a); editor.addClip(b);
    REQUIRE(editor.loopingCount() == 1);
    REQUIRE(editor.streamingCount() == 1);
}

TEST_CASE("VideoClipEditor countByCodec", "[Editor][S80]") {
    VideoClipEditor editor;
    VideoClipAsset a("a", 1.f, 30); a.setCodec(VideoClipCodec::VP9);
    VideoClipAsset b("b", 2.f, 30); b.setCodec(VideoClipCodec::VP9);
    VideoClipAsset c("c", 3.f, 30); c.setCodec(VideoClipCodec::AV1);
    editor.addClip(a); editor.addClip(b); editor.addClip(c);
    REQUIRE(editor.countByCodec(VideoClipCodec::VP9) == 2);
    REQUIRE(editor.countByCodec(VideoClipCodec::AV1) == 1);
    REQUIRE(editor.countByCodec(VideoClipCodec::H264) == 0);
}

TEST_CASE("VideoClipEditor MAX_CLIPS is 256", "[Editor][S80]") {
    REQUIRE(VideoClipEditor::MAX_CLIPS == 256);
}
