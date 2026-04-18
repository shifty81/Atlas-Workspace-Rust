#include "../engine/audio/AudioEngine.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::audio;

void test_audio_load_sound() {
    AudioEngine audio;
    audio.Init();

    SoundID id = audio.LoadSound("explosion.wav");
    assert(id > 0);
    assert(audio.HasSound(id));
    assert(audio.SoundCount() == 1);

    std::cout << "[PASS] test_audio_load_sound" << std::endl;
}

void test_audio_unload_sound() {
    AudioEngine audio;
    audio.Init();

    SoundID id = audio.LoadSound("music.ogg");
    audio.UnloadSound(id);
    assert(!audio.HasSound(id));
    assert(audio.SoundCount() == 0);

    std::cout << "[PASS] test_audio_unload_sound" << std::endl;
}

void test_audio_play_pause_stop() {
    AudioEngine audio;
    audio.Init();

    SoundID id = audio.LoadSound("bgm.ogg");
    assert(audio.GetState(id) == SoundState::Stopped);

    audio.Play(id);
    assert(audio.GetState(id) == SoundState::Playing);

    audio.Pause(id);
    assert(audio.GetState(id) == SoundState::Paused);

    audio.Play(id);
    audio.Stop(id);
    assert(audio.GetState(id) == SoundState::Stopped);

    std::cout << "[PASS] test_audio_play_pause_stop" << std::endl;
}

void test_audio_volume() {
    AudioEngine audio;
    audio.Init();

    SoundID id = audio.LoadSound("sfx.wav");
    audio.SetVolume(id, 0.5f);
    assert(std::abs(audio.GetVolume(id) - 0.5f) < 0.01f);

    audio.SetVolume(id, 2.0f);
    assert(audio.GetVolume(id) <= 1.0f);

    audio.SetVolume(id, -1.0f);
    assert(audio.GetVolume(id) >= 0.0f);

    std::cout << "[PASS] test_audio_volume" << std::endl;
}

void test_audio_master_volume() {
    AudioEngine audio;
    audio.Init();

    audio.SetMasterVolume(0.75f);
    assert(std::abs(audio.GetMasterVolume() - 0.75f) < 0.01f);

    audio.SetMasterVolume(5.0f);
    assert(audio.GetMasterVolume() <= 1.0f);

    std::cout << "[PASS] test_audio_master_volume" << std::endl;
}

void test_audio_looping() {
    AudioEngine audio;
    audio.Init();

    SoundID id = audio.LoadSound("ambience.ogg");
    assert(!audio.IsLooping(id));

    audio.SetLooping(id, true);
    assert(audio.IsLooping(id));

    std::cout << "[PASS] test_audio_looping" << std::endl;
}
