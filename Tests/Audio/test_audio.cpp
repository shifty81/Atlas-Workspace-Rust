#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Audio/Audio.h"

using Catch::Matchers::WithinAbs;

// ── Audio Clip ───────────────────────────────────────────────────

TEST_CASE("AudioClip properties", "[Audio][Clip]") {
    std::vector<uint8_t> data(44100 * 2, 0); // 1 second of Mono16 silence
    NF::AudioClip clip(NF::StringID("test"), NF::AudioFormat::Mono16, 44100, data);

    REQUIRE(clip.name() == NF::StringID("test"));
    REQUIRE(clip.format() == NF::AudioFormat::Mono16);
    REQUIRE(clip.sampleRate() == 44100);
    REQUIRE(clip.sizeBytes() == 44100 * 2);
    REQUIRE_THAT(clip.duration(), WithinAbs(1.f, 1e-3));
}

TEST_CASE("AudioClip stereo duration", "[Audio][Clip]") {
    // Stereo16: 2 bytes per sample × 2 channels = 4 bytes per frame
    std::vector<uint8_t> data(44100 * 4, 0); // 1 second of Stereo16
    NF::AudioClip clip(NF::StringID("stereo"), NF::AudioFormat::Stereo16, 44100, data);
    REQUIRE_THAT(clip.duration(), WithinAbs(1.f, 1e-3));
}

TEST_CASE("AudioClip empty has zero duration", "[Audio][Clip]") {
    NF::AudioClip clip;
    REQUIRE_THAT(clip.duration(), WithinAbs(0.f, 1e-5));
}

// ── Audio Source ──────────────────────────────────────────────────

TEST_CASE("AudioSource default state", "[Audio][Source]") {
    NF::AudioSource src;
    REQUIRE_FALSE(src.isPlaying());
    REQUIRE_THAT(src.volume(), WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(src.pitch(), WithinAbs(1.f, 1e-5));
    REQUIRE_FALSE(src.isLooping());
}

TEST_CASE("AudioSource play/stop/pause lifecycle", "[Audio][Source]") {
    std::vector<uint8_t> data(44100 * 2, 0);
    NF::AudioClip clip(NF::StringID("sfx"), NF::AudioFormat::Mono16, 44100, data);

    NF::AudioSource src(&clip);
    REQUIRE_FALSE(src.isPlaying());

    src.play();
    REQUIRE(src.isPlaying());
    REQUIRE_THAT(src.playbackPosition(), WithinAbs(0.f, 1e-5));

    src.advancePlayback(0.5f);
    REQUIRE_THAT(src.playbackPosition(), WithinAbs(0.5f, 1e-3));

    src.pause();
    REQUIRE_FALSE(src.isPlaying());
    REQUIRE_THAT(src.playbackPosition(), WithinAbs(0.5f, 1e-3)); // paused, not reset

    src.stop();
    REQUIRE_FALSE(src.isPlaying());
    REQUIRE_THAT(src.playbackPosition(), WithinAbs(0.f, 1e-5)); // reset to start
}

TEST_CASE("AudioSource looping", "[Audio][Source]") {
    std::vector<uint8_t> data(44100 * 2, 0); // 1 second
    NF::AudioClip clip(NF::StringID("loop"), NF::AudioFormat::Mono16, 44100, data);

    NF::AudioSource src(&clip);
    src.setLooping(true);
    src.play();

    src.advancePlayback(1.5f); // exceeds duration
    REQUIRE(src.isPlaying()); // still playing because looping
    REQUIRE_THAT(src.playbackPosition(), WithinAbs(0.f, 1e-5)); // wrapped
}

TEST_CASE("AudioSource non-looping stops at end", "[Audio][Source]") {
    std::vector<uint8_t> data(44100 * 2, 0); // 1 second
    NF::AudioClip clip(NF::StringID("oneshot"), NF::AudioFormat::Mono16, 44100, data);

    NF::AudioSource src(&clip);
    src.setLooping(false);
    src.play();

    src.advancePlayback(1.5f); // exceeds duration
    REQUIRE_FALSE(src.isPlaying()); // stopped
}

TEST_CASE("AudioSource volume and pitch", "[Audio][Source]") {
    NF::AudioSource src;
    src.setVolume(0.5f);
    src.setPitch(2.f);

    REQUIRE_THAT(src.volume(), WithinAbs(0.5f, 1e-5));
    REQUIRE_THAT(src.pitch(), WithinAbs(2.f, 1e-5));

    // Volume clamped to [0, 1]
    src.setVolume(2.f);
    REQUIRE_THAT(src.volume(), WithinAbs(1.f, 1e-5));

    src.setVolume(-1.f);
    REQUIRE_THAT(src.volume(), WithinAbs(0.f, 1e-5));
}

// ── Audio Mixer ──────────────────────────────────────────────────

TEST_CASE("AudioMixer channels", "[Audio][Mixer]") {
    NF::AudioMixer mixer;
    REQUIRE(mixer.channelCount() == 0);

    auto sfx = mixer.addChannel("SFX", 0.8f);
    auto music = mixer.addChannel("Music", 0.5f);

    REQUIRE(mixer.channelCount() == 2);
    REQUIRE_THAT(mixer.getEffectiveVolume(sfx), WithinAbs(0.8f, 1e-5));
    REQUIRE_THAT(mixer.getEffectiveVolume(music), WithinAbs(0.5f, 1e-5));
}

TEST_CASE("AudioMixer master volume affects channels", "[Audio][Mixer]") {
    NF::AudioMixer mixer;
    mixer.setMasterVolume(0.5f);
    auto ch = mixer.addChannel("Test", 1.f);

    REQUIRE_THAT(mixer.getEffectiveVolume(ch), WithinAbs(0.5f, 1e-5));
}

TEST_CASE("AudioMixer muted channel returns zero", "[Audio][Mixer]") {
    NF::AudioMixer mixer;
    auto ch = mixer.addChannel("Test", 1.f);

    mixer.setChannelMuted(ch, true);
    REQUIRE_THAT(mixer.getEffectiveVolume(ch), WithinAbs(0.f, 1e-5));
}

// ── Audio Device ─────────────────────────────────────────────────

TEST_CASE("AudioDevice init creates default channels", "[Audio][Device]") {
    NF::AudioDevice device;
    REQUIRE(device.init());
    REQUIRE(device.mixer().channelCount() == 4); // Master, SFX, Music, Voice

    auto& src = device.addSource();
    REQUIRE(device.sourceCount() == 1);

    device.update(0.016f);
    device.shutdown();
}
