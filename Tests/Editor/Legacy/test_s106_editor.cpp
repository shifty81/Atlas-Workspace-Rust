// S106 editor tests: AudioMixerEditor, SoundEffectEditor, MusicSequencer
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/MusicSequencer.h"
#include "NF/Editor/SoundEffectEditor.h"
#include "NF/Editor/AudioMixerEditor.h"

using namespace NF;

// ── AudioMixerEditor ─────────────────────────────────────────────────────────

TEST_CASE("AudioBusType names", "[Editor][S106]") {
    REQUIRE(std::string(audioBusTypeName(AudioBusType::Master))  == "Master");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::Music))   == "Music");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::SFX))     == "SFX");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::Voice))   == "Voice");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::Ambient)) == "Ambient");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::UI))      == "UI");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::Reverb))  == "Reverb");
    REQUIRE(std::string(audioBusTypeName(AudioBusType::Submix))  == "Submix");
}

TEST_CASE("AudioEffectType names", "[Editor][S106]") {
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::EQ))         == "EQ");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Compressor)) == "Compressor");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Reverb))     == "Reverb");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Delay))      == "Delay");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Chorus))     == "Chorus");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Distortion)) == "Distortion");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Limiter))    == "Limiter");
    REQUIRE(std::string(audioEffectTypeName(AudioEffectType::Filter))     == "Filter");
}

TEST_CASE("AudioMixerView names", "[Editor][S106]") {
    REQUIRE(std::string(audioMixerViewName(AudioMixerView::Channels)) == "Channels");
    REQUIRE(std::string(audioMixerViewName(AudioMixerView::Routing))  == "Routing");
    REQUIRE(std::string(audioMixerViewName(AudioMixerView::Effects))  == "Effects");
    REQUIRE(std::string(audioMixerViewName(AudioMixerView::Sends))    == "Sends");
}

TEST_CASE("AudioBus defaults", "[Editor][S106]") {
    AudioBus bus("Master", AudioBusType::Master);
    REQUIRE(bus.name()       == "Master");
    REQUIRE(bus.type()       == AudioBusType::Master);
    REQUIRE(bus.volume()     == 1.0f);
    REQUIRE(bus.pitch()      == 1.0f);
    REQUIRE(!bus.isMuted());
    REQUIRE(!bus.isSolo());
    REQUIRE(!bus.isBypassed());
    REQUIRE(bus.effectCount() == 0u);
}

TEST_CASE("AudioBus mutation", "[Editor][S106]") {
    AudioBus bus("SFX", AudioBusType::SFX);
    bus.setVolume(0.75f);
    bus.setPitch(1.1f);
    bus.setMute(true);
    bus.setSolo(true);
    bus.setBypass(true);
    REQUIRE(bus.volume()     == 0.75f);
    REQUIRE(bus.pitch()      == 1.1f);
    REQUIRE(bus.isMuted());
    REQUIRE(bus.isSolo());
    REQUIRE(bus.isBypassed());
}

TEST_CASE("AudioBus effects", "[Editor][S106]") {
    AudioBus bus("Music", AudioBusType::Music);
    bus.addEffect(AudioEffectType::EQ);
    bus.addEffect(AudioEffectType::Compressor);
    bus.addEffect(AudioEffectType::Reverb);
    REQUIRE(bus.effectCount() == 3u);
    REQUIRE(bus.removeEffect(AudioEffectType::Compressor));
    REQUIRE(bus.effectCount() == 2u);
    REQUIRE(!bus.removeEffect(AudioEffectType::Delay));
}

TEST_CASE("AudioMixerEditor defaults", "[Editor][S106]") {
    AudioMixerEditor ed;
    REQUIRE(ed.view()      == AudioMixerView::Channels);
    REQUIRE(ed.busCount()  == 0u);
    REQUIRE(ed.mutedCount()== 0u);
    REQUIRE(ed.soloCount() == 0u);
}

TEST_CASE("AudioMixerEditor add/remove buses", "[Editor][S106]") {
    AudioMixerEditor ed;
    REQUIRE(ed.addBus(AudioBus("Master", AudioBusType::Master)));
    REQUIRE(ed.addBus(AudioBus("Music",  AudioBusType::Music)));
    REQUIRE(ed.addBus(AudioBus("SFX",    AudioBusType::SFX)));
    REQUIRE(!ed.addBus(AudioBus("Master",AudioBusType::Master)));
    REQUIRE(ed.busCount() == 3u);
    REQUIRE(ed.removeBus("Music"));
    REQUIRE(ed.busCount() == 2u);
    REQUIRE(!ed.removeBus("Voice"));
}

TEST_CASE("AudioMixerEditor counts", "[Editor][S106]") {
    AudioMixerEditor ed;
    AudioBus b1("Master", AudioBusType::Master); b1.setMute(true);
    AudioBus b2("Music",  AudioBusType::Music);  b2.setSolo(true);
    AudioBus b3("SFX",    AudioBusType::SFX);    b3.setMute(true);
    AudioBus b4("Voice",  AudioBusType::Voice);
    ed.addBus(b1); ed.addBus(b2); ed.addBus(b3); ed.addBus(b4);
    REQUIRE(ed.mutedCount()            == 2u);
    REQUIRE(ed.soloCount()             == 1u);
    REQUIRE(ed.countByType(AudioBusType::Music) == 1u);
    REQUIRE(ed.countByType(AudioBusType::SFX)   == 1u);
    auto* found = ed.findBus("SFX");
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == AudioBusType::SFX);
    REQUIRE(ed.findBus("Reverb") == nullptr);
}

TEST_CASE("AudioMixerEditor view", "[Editor][S106]") {
    AudioMixerEditor ed;
    ed.setView(AudioMixerView::Routing);
    REQUIRE(ed.view() == AudioMixerView::Routing);
}

// ── SoundEffectEditor ────────────────────────────────────────────────────────

TEST_CASE("SfxTriggerMode names", "[Editor][S106]") {
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::OnEnter))    == "OnEnter");
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::OnExit))     == "OnExit");
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::OnLoop))     == "OnLoop");
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::OnDistance)) == "OnDistance");
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::OnEvent))    == "OnEvent");
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::OnTimer))    == "OnTimer");
    REQUIRE(std::string(sfxTriggerModeName(SfxTriggerMode::Manual))     == "Manual");
}

TEST_CASE("SfxAttenuationType names", "[Editor][S106]") {
    REQUIRE(std::string(sfxAttenuationTypeName(SfxAttenuationType::None))          == "None");
    REQUIRE(std::string(sfxAttenuationTypeName(SfxAttenuationType::Linear))        == "Linear");
    REQUIRE(std::string(sfxAttenuationTypeName(SfxAttenuationType::Logarithmic))   == "Logarithmic");
    REQUIRE(std::string(sfxAttenuationTypeName(SfxAttenuationType::InverseSquare)) == "InverseSquare");
    REQUIRE(std::string(sfxAttenuationTypeName(SfxAttenuationType::Custom))        == "Custom");
}

TEST_CASE("SfxPlaybackState names", "[Editor][S106]") {
    REQUIRE(std::string(sfxPlaybackStateName(SfxPlaybackState::Stopped))   == "Stopped");
    REQUIRE(std::string(sfxPlaybackStateName(SfxPlaybackState::Playing))   == "Playing");
    REQUIRE(std::string(sfxPlaybackStateName(SfxPlaybackState::Paused))    == "Paused");
    REQUIRE(std::string(sfxPlaybackStateName(SfxPlaybackState::FadingIn))  == "FadingIn");
    REQUIRE(std::string(sfxPlaybackStateName(SfxPlaybackState::FadingOut)) == "FadingOut");
}

TEST_CASE("SoundVariant defaults", "[Editor][S106]") {
    SoundVariant sv("explosion_01.wav");
    REQUIRE(sv.assetPath() == "explosion_01.wav");
    REQUIRE(sv.weight()    == 1.0f);
    REQUIRE(sv.pitch()     == 1.0f);
    REQUIRE(sv.volume()    == 1.0f);
    REQUIRE(sv.isEnabled());
}

TEST_CASE("SoundVariant mutation", "[Editor][S106]") {
    SoundVariant sv("gunshot.wav", 2.0f);
    sv.setPitch(1.1f);
    sv.setVolume(0.9f);
    sv.setEnabled(false);
    REQUIRE(sv.weight()    == 2.0f);
    REQUIRE(sv.pitch()     == 1.1f);
    REQUIRE(sv.volume()    == 0.9f);
    REQUIRE(!sv.isEnabled());
}

TEST_CASE("SoundEffectEditor defaults", "[Editor][S106]") {
    SoundEffectEditor ed;
    REQUIRE(ed.triggerMode()     == SfxTriggerMode::OnEvent);
    REQUIRE(ed.attenuationType() == SfxAttenuationType::Logarithmic);
    REQUIRE(ed.playbackState()   == SfxPlaybackState::Stopped);
    REQUIRE(!ed.isLoop());
    REQUIRE(ed.volume()          == 1.0f);
    REQUIRE(ed.pitchVariance()   == 0.0f);
    REQUIRE(ed.minDistance()     == 1.0f);
    REQUIRE(ed.maxDistance()     == 50.0f);
    REQUIRE(ed.variantCount()    == 0u);
}

TEST_CASE("SoundEffectEditor mutation", "[Editor][S106]") {
    SoundEffectEditor ed;
    ed.setTriggerMode(SfxTriggerMode::OnLoop);
    ed.setAttenuationType(SfxAttenuationType::Linear);
    ed.setPlaybackState(SfxPlaybackState::Playing);
    ed.setLoop(true);
    ed.setVolume(0.8f);
    ed.setPitchVariance(0.1f);
    ed.setMinDistance(2.0f);
    ed.setMaxDistance(100.0f);
    REQUIRE(ed.triggerMode()     == SfxTriggerMode::OnLoop);
    REQUIRE(ed.attenuationType() == SfxAttenuationType::Linear);
    REQUIRE(ed.playbackState()   == SfxPlaybackState::Playing);
    REQUIRE(ed.isLoop());
    REQUIRE(ed.volume()          == 0.8f);
    REQUIRE(ed.pitchVariance()   == 0.1f);
    REQUIRE(ed.minDistance()     == 2.0f);
    REQUIRE(ed.maxDistance()     == 100.0f);
}

TEST_CASE("SoundEffectEditor variants", "[Editor][S106]") {
    SoundEffectEditor ed;
    SoundVariant v1("exp_01.wav", 2.0f);
    SoundVariant v2("exp_02.wav", 3.0f);
    SoundVariant v3("exp_03.wav", 1.0f); v3.setEnabled(false);
    ed.addVariant(v1); ed.addVariant(v2); ed.addVariant(v3);
    REQUIRE(ed.variantCount()       == 3u);
    REQUIRE(ed.enabledVariantCount()== 2u);
    REQUIRE(ed.totalWeight()        == 5.0f);
    ed.clearVariants();
    REQUIRE(ed.variantCount() == 0u);
}

// ── MusicSequencer ───────────────────────────────────────────────────────────

TEST_CASE("MusicLayerState names", "[Editor][S106]") {
    REQUIRE(std::string(musicLayerStateName(MusicLayerState::Inactive))  == "Inactive");
    REQUIRE(std::string(musicLayerStateName(MusicLayerState::FadingIn))  == "FadingIn");
    REQUIRE(std::string(musicLayerStateName(MusicLayerState::Active))    == "Active");
    REQUIRE(std::string(musicLayerStateName(MusicLayerState::FadingOut)) == "FadingOut");
    REQUIRE(std::string(musicLayerStateName(MusicLayerState::Looping))   == "Looping");
}

TEST_CASE("MusicSyncMode names", "[Editor][S106]") {
    REQUIRE(std::string(musicSyncModeName(MusicSyncMode::Immediate)) == "Immediate");
    REQUIRE(std::string(musicSyncModeName(MusicSyncMode::OnBeat))    == "OnBeat");
    REQUIRE(std::string(musicSyncModeName(MusicSyncMode::OnBar))     == "OnBar");
    REQUIRE(std::string(musicSyncModeName(MusicSyncMode::OnSection)) == "OnSection");
    REQUIRE(std::string(musicSyncModeName(MusicSyncMode::OnEnd))     == "OnEnd");
}

TEST_CASE("MusicTransitionType names", "[Editor][S106]") {
    REQUIRE(std::string(musicTransitionTypeName(MusicTransitionType::Cut))       == "Cut");
    REQUIRE(std::string(musicTransitionTypeName(MusicTransitionType::CrossFade)) == "CrossFade");
    REQUIRE(std::string(musicTransitionTypeName(MusicTransitionType::Stinger))   == "Stinger");
    REQUIRE(std::string(musicTransitionTypeName(MusicTransitionType::Segue))     == "Segue");
}

TEST_CASE("MusicLayer defaults", "[Editor][S106]") {
    MusicLayer layer(1, "Exploration");
    REQUIRE(layer.id()       == 1u);
    REQUIRE(layer.name()     == "Exploration");
    REQUIRE(layer.state()    == MusicLayerState::Inactive);
    REQUIRE(layer.volume()   == 1.0f);
    REQUIRE(!layer.isMuted());
    REQUIRE(!layer.isSolo());
    REQUIRE(layer.fadeTime() == 1.0f);
}

TEST_CASE("MusicLayer mutation", "[Editor][S106]") {
    MusicLayer layer(2, "Combat");
    layer.setState(MusicLayerState::Active);
    layer.setVolume(0.8f);
    layer.setMute(true);
    layer.setSolo(true);
    layer.setFadeTime(2.0f);
    REQUIRE(layer.state()    == MusicLayerState::Active);
    REQUIRE(layer.volume()   == 0.8f);
    REQUIRE(layer.isMuted());
    REQUIRE(layer.isSolo());
    REQUIRE(layer.fadeTime() == 2.0f);
}

TEST_CASE("MusicSequencer defaults", "[Editor][S106]") {
    MusicSequencer seq;
    REQUIRE(seq.syncMode()       == MusicSyncMode::OnBar);
    REQUIRE(seq.transitionType() == MusicTransitionType::CrossFade);
    REQUIRE(seq.tempo()          == 120.0f);
    REQUIRE(!seq.isPlaying());
    REQUIRE(seq.isLooping());
    REQUIRE(seq.layerCount()     == 0u);
}

TEST_CASE("MusicSequencer add/remove layers", "[Editor][S106]") {
    MusicSequencer seq;
    REQUIRE(seq.addLayer(MusicLayer(1, "Base")));
    REQUIRE(seq.addLayer(MusicLayer(2, "Combat")));
    REQUIRE(!seq.addLayer(MusicLayer(1, "Base")));
    REQUIRE(seq.layerCount() == 2u);
    REQUIRE(seq.removeLayer(1));
    REQUIRE(seq.layerCount() == 1u);
    REQUIRE(!seq.removeLayer(99));
}

TEST_CASE("MusicSequencer active and muted counts", "[Editor][S106]") {
    MusicSequencer seq;
    MusicLayer l1(1, "Base");      l1.setState(MusicLayerState::Active);
    MusicLayer l2(2, "Combat");    l2.setState(MusicLayerState::Looping);
    MusicLayer l3(3, "Ambient");   l3.setState(MusicLayerState::Inactive); l3.setMute(true);
    MusicLayer l4(4, "Tension");   l4.setState(MusicLayerState::FadingIn); l4.setMute(true);
    seq.addLayer(l1); seq.addLayer(l2); seq.addLayer(l3); seq.addLayer(l4);
    REQUIRE(seq.activeLayerCount() == 2u);
    REQUIRE(seq.mutedLayerCount()  == 2u);
    auto* found = seq.findLayer(2);
    REQUIRE(found != nullptr);
    REQUIRE(found->name() == "Combat");
    REQUIRE(seq.findLayer(99) == nullptr);
}

TEST_CASE("MusicSequencer mutation", "[Editor][S106]") {
    MusicSequencer seq;
    seq.setSyncMode(MusicSyncMode::OnBeat);
    seq.setTransitionType(MusicTransitionType::Stinger);
    seq.setTempo(140.0f);
    seq.setPlaying(true);
    seq.setLoopEnabled(false);
    REQUIRE(seq.syncMode()       == MusicSyncMode::OnBeat);
    REQUIRE(seq.transitionType() == MusicTransitionType::Stinger);
    REQUIRE(seq.tempo()          == 140.0f);
    REQUIRE(seq.isPlaying());
    REQUIRE(!seq.isLooping());
}
