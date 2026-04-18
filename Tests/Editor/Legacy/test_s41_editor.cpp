#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/VideoClipEditor.h"

using namespace NF;

TEST_CASE("VideoClipCodec names cover all 5 values", "[Editor][S41]") {
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::H264)) == "H264");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::H265)) == "H265");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::VP8))  == "VP8");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::VP9))  == "VP9");
    REQUIRE(std::string(videoClipCodecName(VideoClipCodec::AV1))  == "AV1");
}

TEST_CASE("VideoClipState names cover all 5 values", "[Editor][S41]") {
    REQUIRE(std::string(videoClipStateName(VideoClipState::Idle))     == "Idle");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Playing))  == "Playing");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Paused))   == "Paused");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Stopped))  == "Stopped");
    REQUIRE(std::string(videoClipStateName(VideoClipState::Finished)) == "Finished");
}

TEST_CASE("VideoAspectRatio names cover all 5 values", "[Editor][S41]") {
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio4x3))  == "4x3");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio16x9)) == "16x9");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio21x9)) == "21x9");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Ratio1x1))  == "1x1");
    REQUIRE(std::string(videoAspectRatioName(VideoAspectRatio::Custom))    == "Custom");
}

TEST_CASE("VideoClipAsset default values", "[Editor][S41]") {
    VideoClipAsset v("cutscene");
    REQUIRE(v.name()        == "cutscene");
    REQUIRE(v.durationSec() == Catch::Approx(1.0f));
    REQUIRE(v.fps()         == 30u);
    REQUIRE(v.width()       == 1920u);
    REQUIRE(v.height()      == 1080u);
    REQUIRE(v.codec()       == VideoClipCodec::H264);
    REQUIRE(v.state()       == VideoClipState::Idle);
    REQUIRE(v.aspectRatio() == VideoAspectRatio::Ratio16x9);
    REQUIRE_FALSE(v.isLooping());
    REQUIRE_FALSE(v.isStreaming());
    REQUIRE_FALSE(v.isDirty());
    REQUIRE_FALSE(v.isPlaying());
    REQUIRE_FALSE(v.isPaused());
    REQUIRE_FALSE(v.isFinished());
    REQUIRE(v.isHD()); // 1920x1080 is HD
}

TEST_CASE("VideoClipAsset setters round-trip", "[Editor][S41]") {
    VideoClipAsset v("trailer", 90.0f, 60);
    v.setCodec(VideoClipCodec::AV1);
    v.setState(VideoClipState::Playing);
    v.setAspectRatio(VideoAspectRatio::Ratio21x9);
    v.setWidth(3840);
    v.setHeight(2160);
    v.setLooping(true);
    v.setStreaming(true);
    v.setDirty(true);

    REQUIRE(v.codec()       == VideoClipCodec::AV1);
    REQUIRE(v.state()       == VideoClipState::Playing);
    REQUIRE(v.aspectRatio() == VideoAspectRatio::Ratio21x9);
    REQUIRE(v.width()       == 3840u);
    REQUIRE(v.height()      == 2160u);
    REQUIRE(v.durationSec() == Catch::Approx(90.0f));
    REQUIRE(v.fps()         == 60u);
    REQUIRE(v.isLooping());
    REQUIRE(v.isStreaming());
    REQUIRE(v.isDirty());
    REQUIRE(v.isPlaying());
    REQUIRE_FALSE(v.isPaused());
    REQUIRE_FALSE(v.isFinished());
    REQUIRE(v.isHD());
}

TEST_CASE("VideoClipAsset isHD requires width>=1280 and height>=720", "[Editor][S41]") {
    VideoClipAsset v("lowres");
    v.setWidth(640);
    v.setHeight(480);
    REQUIRE_FALSE(v.isHD());

    v.setWidth(1280);
    v.setHeight(720);
    REQUIRE(v.isHD());
}

TEST_CASE("VideoClipAsset isPaused and isFinished states", "[Editor][S41]") {
    VideoClipAsset v("clip");
    v.setState(VideoClipState::Paused);
    REQUIRE(v.isPaused());
    REQUIRE_FALSE(v.isPlaying());

    v.setState(VideoClipState::Finished);
    REQUIRE(v.isFinished());
    REQUIRE_FALSE(v.isPaused());
}

TEST_CASE("VideoClipEditor addClip and duplicate rejection", "[Editor][S41]") {
    VideoClipEditor editor;
    VideoClipAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addClip(a));
    REQUIRE(editor.addClip(b));
    REQUIRE_FALSE(editor.addClip(dup));
    REQUIRE(editor.clipCount() == 2);
}

TEST_CASE("VideoClipEditor removeClip clears activeClip", "[Editor][S41]") {
    VideoClipEditor editor;
    VideoClipAsset v("intro");
    editor.addClip(v);
    editor.setActiveClip("intro");
    REQUIRE(editor.activeClip() == "intro");

    editor.removeClip("intro");
    REQUIRE(editor.clipCount() == 0);
    REQUIRE(editor.activeClip().empty());
}

TEST_CASE("VideoClipEditor findClip returns pointer or nullptr", "[Editor][S41]") {
    VideoClipEditor editor;
    VideoClipAsset v("outro");
    editor.addClip(v);

    REQUIRE(editor.findClip("outro") != nullptr);
    REQUIRE(editor.findClip("outro")->name() == "outro");
    REQUIRE(editor.findClip("missing") == nullptr);
}

TEST_CASE("VideoClipEditor aggregate counts", "[Editor][S41]") {
    VideoClipEditor editor;

    VideoClipAsset a("a"); a.setDirty(true); a.setState(VideoClipState::Playing); a.setLooping(true); a.setStreaming(true); a.setWidth(1920); a.setHeight(1080);
    VideoClipAsset b("b"); b.setDirty(true); b.setWidth(640); b.setHeight(480);
    VideoClipAsset c("c"); c.setLooping(true); c.setStreaming(true); c.setWidth(1280); c.setHeight(720);

    editor.addClip(a); editor.addClip(b); editor.addClip(c);

    REQUIRE(editor.dirtyCount()     == 2);
    REQUIRE(editor.playingCount()   == 1);
    REQUIRE(editor.streamingCount() == 2);
    REQUIRE(editor.loopingCount()   == 2);
    REQUIRE(editor.hdCount()        == 2); // a and c
}

TEST_CASE("VideoClipEditor countByCodec and countByState", "[Editor][S41]") {
    VideoClipEditor editor;

    VideoClipAsset a("a"); a.setCodec(VideoClipCodec::H265); a.setState(VideoClipState::Playing);
    VideoClipAsset b("b"); b.setCodec(VideoClipCodec::H265); b.setState(VideoClipState::Idle);
    VideoClipAsset c("c"); c.setCodec(VideoClipCodec::VP9);  c.setState(VideoClipState::Playing);

    editor.addClip(a); editor.addClip(b); editor.addClip(c);

    REQUIRE(editor.countByCodec(VideoClipCodec::H265)       == 2);
    REQUIRE(editor.countByCodec(VideoClipCodec::VP9)        == 1);
    REQUIRE(editor.countByState(VideoClipState::Playing)    == 2);
    REQUIRE(editor.countByState(VideoClipState::Idle)       == 1);
}

TEST_CASE("VideoClipEditor setActiveClip returns false for missing", "[Editor][S41]") {
    VideoClipEditor editor;
    REQUIRE_FALSE(editor.setActiveClip("ghost"));
    REQUIRE(editor.activeClip().empty());
}

TEST_CASE("VideoClipEditor MAX_CLIPS limit enforced", "[Editor][S41]") {
    VideoClipEditor editor;
    for (size_t i = 0; i < VideoClipEditor::MAX_CLIPS; ++i) {
        VideoClipAsset v("Clip" + std::to_string(i));
        REQUIRE(editor.addClip(v));
    }
    VideoClipAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addClip(overflow));
    REQUIRE(editor.clipCount() == VideoClipEditor::MAX_CLIPS);
}
