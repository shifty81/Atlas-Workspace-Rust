#pragma once
// NF::Editor — Sound mixer editor v1: channel volume, pan, mute, solo management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class SmvBusType : uint8_t { Master, Music, SFX, Voice, Ambient, UI };

struct SmvChannel {
    uint32_t    id     = 0;
    std::string name;
    SmvBusType  bus    = SmvBusType::SFX;
    float       volume = 1.f;
    float       pan    = 0.f;
    bool        muted  = false;
    bool        soloed = false;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

class SoundMixerEditorV1 {
public:
    bool addChannel(const SmvChannel& ch) {
        if (!ch.isValid()) return false;
        for (const auto& c : m_channels) if (c.id == ch.id) return false;
        m_channels.push_back(ch);
        return true;
    }

    bool removeChannel(uint32_t id) {
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            if (it->id == id) { m_channels.erase(it); return true; }
        }
        return false;
    }

    bool setVolume(uint32_t id, float vol) {
        SmvChannel* c = findChannel_(id);
        if (!c) return false;
        c->volume = std::max(0.f, std::min(vol, 2.f));
        return true;
    }

    bool setPan(uint32_t id, float pan) {
        SmvChannel* c = findChannel_(id);
        if (!c) return false;
        c->pan = std::max(-1.f, std::min(pan, 1.f));
        return true;
    }

    bool muteChannel(uint32_t id, bool mute) {
        SmvChannel* c = findChannel_(id);
        if (!c) return false;
        c->muted = mute;
        return true;
    }

    bool soloChannel(uint32_t id, bool solo) {
        SmvChannel* c = findChannel_(id);
        if (!c) return false;
        c->soloed = solo;
        return true;
    }

    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

    void  setMasterVolume(float v) { m_masterVolume = std::max(0.f, std::min(v, 2.f)); }
    float getMasterVolume()  const { return m_masterVolume; }

    float getMixedVolume(uint32_t id) const {
        for (const auto& c : m_channels) {
            if (c.id == id) {
                if (c.muted) return 0.f;
                return c.volume * m_masterVolume;
            }
        }
        return 0.f;
    }

private:
    SmvChannel* findChannel_(uint32_t id) {
        for (auto& c : m_channels) if (c.id == id) return &c;
        return nullptr;
    }

    std::vector<SmvChannel> m_channels;
    float                   m_masterVolume = 1.f;
};

} // namespace NF
