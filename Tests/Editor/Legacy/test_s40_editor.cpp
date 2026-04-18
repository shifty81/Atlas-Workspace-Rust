#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AudioClipEditor.h"

using namespace NF;

TEST_CASE("AudioClipFormat names cover all 5 values", "[Editor][S40]") {
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::WAV))  == "WAV");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::OGG))  == "OGG");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::MP3))  == "MP3");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::FLAC)) == "FLAC");
    REQUIRE(std::string(audioClipFormatName(AudioClipFormat::AIFF)) == "AIFF");
}

TEST_CASE("AudioClipState names cover all 5 values", "[Editor][S40]") {
    REQUIRE(std::string(audioClipStateName(AudioClipState::Idle))     == "Idle");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Playing))  == "Playing");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Paused))   == "Paused");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Stopped))  == "Stopped");
    REQUIRE(std::string(audioClipStateName(AudioClipState::Finished)) == "Finished");
}

TEST_CASE("AudioLoopMode names cover all 5 values", "[Editor][S40]") {
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::None))      == "None");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::Loop))      == "Loop");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::PingPong))  == "PingPong");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::LoopPoint)) == "LoopPoint");
    REQUIRE(std::string(audioLoopModeName(AudioLoopMode::Shuffle))   == "Shuffle");
}

TEST_CASE("AudioClipAsset default values", "[Editor][S40]") {
    AudioClipAsset c("explosion");
    REQUIRE(c.name()        == "explosion");
    REQUIRE(c.durationSec() == Catch::Approx(1.0f));
    REQUIRE(c.sampleRate()  == 44100u);
    REQUIRE(c.format()      == AudioClipFormat::WAV);
    REQUIRE(c.state()       == AudioClipState::Idle);
    REQUIRE(c.loopMode()    == AudioLoopMode::None);
    REQUIRE(c.volume()      == Catch::Approx(1.0f));
    REQUIRE(c.pitch()       == Catch::Approx(1.0f));
    REQUIRE_FALSE(c.isStreaming());
    REQUIRE_FALSE(c.isDirty());
    REQUIRE_FALSE(c.isPlaying());
    REQUIRE_FALSE(c.isPaused());
    REQUIRE_FALSE(c.isFinished());
    REQUIRE_FALSE(c.isLooping());
}

TEST_CASE("AudioClipAsset setters round-trip", "[Editor][S40]") {
    AudioClipAsset c("music", 120.0f, 48000);
    c.setFormat(AudioClipFormat::OGG);
    c.setState(AudioClipState::Playing);
    c.setLoopMode(AudioLoopMode::Loop);
    c.setVolume(0.8f);
    c.setPitch(1.2f);
    c.setStreaming(true);
    c.setDirty(true);

    REQUIRE(c.format()      == AudioClipFormat::OGG);
    REQUIRE(c.state()       == AudioClipState::Playing);
    REQUIRE(c.loopMode()    == AudioLoopMode::Loop);
    REQUIRE(c.volume()      == Catch::Approx(0.8f));
    REQUIRE(c.pitch()       == Catch::Approx(1.2f));
    REQUIRE(c.durationSec() == Catch::Approx(120.0f));
    REQUIRE(c.sampleRate()  == 48000u);
    REQUIRE(c.isStreaming());
    REQUIRE(c.isDirty());
    REQUIRE(c.isPlaying());
    REQUIRE_FALSE(c.isPaused());
    REQUIRE_FALSE(c.isFinished());
    REQUIRE(c.isLooping());
}

TEST_CASE("AudioClipAsset isLooping requires non-None loopMode", "[Editor][S40]") {
    AudioClipAsset c("sfx");
    REQUIRE_FALSE(c.isLooping());
    c.setLoopMode(AudioLoopMode::PingPong);
    REQUIRE(c.isLooping());
}

TEST_CASE("AudioClipAsset isPaused and isFinished states", "[Editor][S40]") {
    AudioClipAsset c("ambient");
    c.setState(AudioClipState::Paused);
    REQUIRE(c.isPaused());
    REQUIRE_FALSE(c.isPlaying());

    c.setState(AudioClipState::Finished);
    REQUIRE(c.isFinished());
    REQUIRE_FALSE(c.isPaused());
}

TEST_CASE("AudioClipEditor addClip and duplicate rejection", "[Editor][S40]") {
    AudioClipEditor editor;
    AudioClipAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addClip(a));
    REQUIRE(editor.addClip(b));
    REQUIRE_FALSE(editor.addClip(dup));
    REQUIRE(editor.clipCount() == 2);
}

TEST_CASE("AudioClipEditor removeClip clears activeClip", "[Editor][S40]") {
    AudioClipEditor editor;
    AudioClipAsset c("theme");
    editor.addClip(c);
    editor.setActiveClip("theme");
    REQUIRE(editor.activeClip() == "theme");

    editor.removeClip("theme");
    REQUIRE(editor.clipCount() == 0);
    REQUIRE(editor.activeClip().empty());
}

TEST_CASE("AudioClipEditor findClip returns pointer or nullptr", "[Editor][S40]") {
    AudioClipEditor editor;
    AudioClipAsset c("ping");
    editor.addClip(c);

    REQUIRE(editor.findClip("ping") != nullptr);
    REQUIRE(editor.findClip("ping")->name() == "ping");
    REQUIRE(editor.findClip("missing") == nullptr);
}

TEST_CASE("AudioClipEditor dirtyCount, playingCount, streamingCount, loopingCount", "[Editor][S40]") {
    AudioClipEditor editor;

    AudioClipAsset a("a"); a.setDirty(true); a.setState(AudioClipState::Playing); a.setLoopMode(AudioLoopMode::Loop); a.setStreaming(true);
    AudioClipAsset b("b"); b.setDirty(true);
    AudioClipAsset c("c"); c.setLoopMode(AudioLoopMode::PingPong); c.setStreaming(true);

    editor.addClip(a); editor.addClip(b); editor.addClip(c);

    REQUIRE(editor.dirtyCount()     == 2);
    REQUIRE(editor.playingCount()   == 1);
    REQUIRE(editor.streamingCount() == 2);
    REQUIRE(editor.loopingCount()   == 2);
}

TEST_CASE("AudioClipEditor countByFormat and countByState", "[Editor][S40]") {
    AudioClipEditor editor;

    AudioClipAsset a("a"); a.setFormat(AudioClipFormat::OGG);  a.setState(AudioClipState::Playing);
    AudioClipAsset b("b"); b.setFormat(AudioClipFormat::OGG);  b.setState(AudioClipState::Idle);
    AudioClipAsset c("c"); c.setFormat(AudioClipFormat::FLAC); c.setState(AudioClipState::Playing);

    editor.addClip(a); editor.addClip(b); editor.addClip(c);

    REQUIRE(editor.countByFormat(AudioClipFormat::OGG)        == 2);
    REQUIRE(editor.countByFormat(AudioClipFormat::FLAC)       == 1);
    REQUIRE(editor.countByState(AudioClipState::Playing)      == 2);
    REQUIRE(editor.countByState(AudioClipState::Idle)         == 1);
}

TEST_CASE("AudioClipEditor setActiveClip returns false for missing", "[Editor][S40]") {
    AudioClipEditor editor;
    REQUIRE_FALSE(editor.setActiveClip("ghost"));
    REQUIRE(editor.activeClip().empty());
}

TEST_CASE("AudioClipEditor MAX_CLIPS limit enforced", "[Editor][S40]") {
    AudioClipEditor editor;
    for (size_t i = 0; i < AudioClipEditor::MAX_CLIPS; ++i) {
        AudioClipAsset c("Clip" + std::to_string(i));
        REQUIRE(editor.addClip(c));
    }
    AudioClipAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addClip(overflow));
    REQUIRE(editor.clipCount() == AudioClipEditor::MAX_CLIPS);
}
