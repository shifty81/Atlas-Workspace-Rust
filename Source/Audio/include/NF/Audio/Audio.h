#pragma once
// NF::Audio — Device, spatial audio, mixer, cues
#include "NF/Core/Core.h"

namespace NF {

// ── Audio Clip ───────────────────────────────────────────────────

enum class AudioFormat : uint8_t {
    Mono8,
    Mono16,
    Stereo8,
    Stereo16
};

class AudioClip {
public:
    AudioClip() = default;
    AudioClip(StringID name, AudioFormat format, uint32_t sampleRate,
              std::vector<uint8_t> data)
        : m_name(name), m_format(format), m_sampleRate(sampleRate),
          m_data(std::move(data)) {}

    [[nodiscard]] StringID name() const { return m_name; }
    [[nodiscard]] AudioFormat format() const { return m_format; }
    [[nodiscard]] uint32_t sampleRate() const { return m_sampleRate; }
    [[nodiscard]] const std::vector<uint8_t>& data() const { return m_data; }
    [[nodiscard]] size_t sizeBytes() const { return m_data.size(); }

    [[nodiscard]] float duration() const {
        if (m_sampleRate == 0) return 0.f;
        size_t bytesPerSample = 1;
        size_t channels = 1;
        switch (m_format) {
        case AudioFormat::Mono8:    bytesPerSample = 1; channels = 1; break;
        case AudioFormat::Mono16:   bytesPerSample = 2; channels = 1; break;
        case AudioFormat::Stereo8:  bytesPerSample = 1; channels = 2; break;
        case AudioFormat::Stereo16: bytesPerSample = 2; channels = 2; break;
        }
        size_t totalSamples = m_data.size() / (bytesPerSample * channels);
        return static_cast<float>(totalSamples) / static_cast<float>(m_sampleRate);
    }

private:
    StringID m_name;
    AudioFormat m_format = AudioFormat::Mono16;
    uint32_t m_sampleRate = 44100;
    std::vector<uint8_t> m_data;
};

// ── Audio Source ──────────────────────────────────────────────────

class AudioSource {
public:
    AudioSource() = default;
    explicit AudioSource(const AudioClip* clip) : m_clip(clip) {}

    void play() { m_playing = true; m_playbackPosition = 0.f; }
    void stop() { m_playing = false; m_playbackPosition = 0.f; }
    void pause() { m_playing = false; }

    void setClip(const AudioClip* clip) { m_clip = clip; }
    void setVolume(float v) { m_volume = std::clamp(v, 0.f, 1.f); }
    void setPitch(float p) { m_pitch = std::max(0.01f, p); }
    void setLooping(bool loop) { m_looping = loop; }
    void setPosition(const Vec3& pos) { m_position = pos; }

    [[nodiscard]] const AudioClip* clip() const { return m_clip; }
    [[nodiscard]] bool isPlaying() const { return m_playing; }
    [[nodiscard]] float volume() const { return m_volume; }
    [[nodiscard]] float pitch() const { return m_pitch; }
    [[nodiscard]] bool isLooping() const { return m_looping; }
    [[nodiscard]] const Vec3& position() const { return m_position; }
    [[nodiscard]] float playbackPosition() const { return m_playbackPosition; }

    void advancePlayback(float dt) {
        if (!m_playing || !m_clip) return;
        m_playbackPosition += dt * m_pitch;
        if (m_playbackPosition >= m_clip->duration()) {
            if (m_looping) {
                m_playbackPosition = 0.f;
            } else {
                m_playing = false;
                m_playbackPosition = 0.f;
            }
        }
    }

private:
    const AudioClip* m_clip = nullptr;
    bool m_playing = false;
    bool m_looping = false;
    float m_volume = 1.f;
    float m_pitch = 1.f;
    Vec3 m_position;
    float m_playbackPosition = 0.f;
};

// ── Audio Mixer ──────────────────────────────────────────────────

class AudioMixer {
public:
    struct Channel {
        std::string name;
        float volume = 1.f;
        bool muted = false;
    };

    size_t addChannel(const std::string& name, float volume = 1.f) {
        size_t id = m_channels.size();
        m_channels.push_back({name, volume, false});
        return id;
    }

    void setChannelVolume(size_t id, float vol) {
        if (id < m_channels.size()) m_channels[id].volume = std::clamp(vol, 0.f, 1.f);
    }

    void setChannelMuted(size_t id, bool muted) {
        if (id < m_channels.size()) m_channels[id].muted = muted;
    }

    [[nodiscard]] float getEffectiveVolume(size_t id) const {
        if (id >= m_channels.size()) return 0.f;
        auto& ch = m_channels[id];
        return ch.muted ? 0.f : ch.volume * m_masterVolume;
    }

    void setMasterVolume(float vol) { m_masterVolume = std::clamp(vol, 0.f, 1.f); }
    [[nodiscard]] float masterVolume() const { return m_masterVolume; }
    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

private:
    std::vector<Channel> m_channels;
    float m_masterVolume = 1.f;
};

// ── Audio Listener (for spatial audio) ───────────────────────────

struct AudioListener {
    Vec3 position;
    Vec3 forward{0.f, 0.f, -1.f};
    Vec3 up{0.f, 1.f, 0.f};
};

// ── Audio Device ─────────────────────────────────────────────────

class AudioDevice {
public:
    bool init() {
        m_mixer.addChannel("Master");
        m_mixer.addChannel("SFX");
        m_mixer.addChannel("Music");
        m_mixer.addChannel("Voice");
        NF_LOG_INFO("Audio", "Audio device initialized");
        return true;
    }

    void shutdown() {
        m_sources.clear();
        NF_LOG_INFO("Audio", "Audio device shutdown");
    }

    void update(float dt) {
        for (auto& src : m_sources) {
            src.advancePlayback(dt);
        }
    }

    AudioSource& addSource() {
        m_sources.emplace_back();
        return m_sources.back();
    }

    void setListener(const AudioListener& listener) { m_listener = listener; }

    [[nodiscard]] AudioMixer& mixer() { return m_mixer; }
    [[nodiscard]] const AudioMixer& mixer() const { return m_mixer; }
    [[nodiscard]] const AudioListener& listener() const { return m_listener; }
    [[nodiscard]] size_t sourceCount() const { return m_sources.size(); }

private:
    AudioMixer m_mixer;
    AudioListener m_listener;
    std::vector<AudioSource> m_sources;
};

} // namespace NF
