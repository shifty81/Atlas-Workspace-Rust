#pragma once
// NF::Editor — Event bus editor v1: event channel and subscription management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ebev1EventPriority : uint8_t { Low, Normal, High, Critical };
enum class Ebev1ChannelState  : uint8_t { Inactive, Active, Paused, Deprecated };

inline const char* ebev1EventPriorityName(Ebev1EventPriority p) {
    switch (p) {
        case Ebev1EventPriority::Low:      return "Low";
        case Ebev1EventPriority::Normal:   return "Normal";
        case Ebev1EventPriority::High:     return "High";
        case Ebev1EventPriority::Critical: return "Critical";
    }
    return "Unknown";
}

inline const char* ebev1ChannelStateName(Ebev1ChannelState s) {
    switch (s) {
        case Ebev1ChannelState::Inactive:   return "Inactive";
        case Ebev1ChannelState::Active:     return "Active";
        case Ebev1ChannelState::Paused:     return "Paused";
        case Ebev1ChannelState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

struct Ebev1Channel {
    uint64_t             id       = 0;
    std::string          name;
    Ebev1ChannelState    state    = Ebev1ChannelState::Inactive;
    Ebev1EventPriority   priority = Ebev1EventPriority::Normal;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()     const { return state == Ebev1ChannelState::Active; }
    [[nodiscard]] bool isPaused()     const { return state == Ebev1ChannelState::Paused; }
    [[nodiscard]] bool isDeprecated() const { return state == Ebev1ChannelState::Deprecated; }
};

struct Ebev1Subscription {
    uint64_t    id        = 0;
    uint64_t    channelId = 0;
    std::string listener;

    [[nodiscard]] bool isValid() const { return id != 0 && channelId != 0 && !listener.empty(); }
};

using Ebev1ChangeCallback = std::function<void(uint64_t)>;

class EventBusEditorV1 {
public:
    static constexpr size_t MAX_CHANNELS      = 256;
    static constexpr size_t MAX_SUBSCRIPTIONS = 1024;

    bool addChannel(const Ebev1Channel& channel) {
        if (!channel.isValid()) return false;
        for (const auto& c : m_channels) if (c.id == channel.id) return false;
        if (m_channels.size() >= MAX_CHANNELS) return false;
        m_channels.push_back(channel);
        if (m_onChange) m_onChange(channel.id);
        return true;
    }

    bool removeChannel(uint64_t id) {
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            if (it->id == id) { m_channels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ebev1Channel* findChannel(uint64_t id) {
        for (auto& c : m_channels) if (c.id == id) return &c;
        return nullptr;
    }

    bool addSubscription(const Ebev1Subscription& sub) {
        if (!sub.isValid()) return false;
        for (const auto& s : m_subscriptions) if (s.id == sub.id) return false;
        if (m_subscriptions.size() >= MAX_SUBSCRIPTIONS) return false;
        m_subscriptions.push_back(sub);
        if (m_onChange) m_onChange(sub.channelId);
        return true;
    }

    bool removeSubscription(uint64_t id) {
        for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
            if (it->id == id) { m_subscriptions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t channelCount()      const { return m_channels.size(); }
    [[nodiscard]] size_t subscriptionCount() const { return m_subscriptions.size(); }

    [[nodiscard]] size_t activeChannelCount() const {
        size_t c = 0; for (const auto& ch : m_channels) if (ch.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t countByPriority(Ebev1EventPriority priority) const {
        size_t c = 0; for (const auto& ch : m_channels) if (ch.priority == priority) ++c; return c;
    }
    [[nodiscard]] size_t subscriptionsForChannel(uint64_t channelId) const {
        size_t c = 0; for (const auto& s : m_subscriptions) if (s.channelId == channelId) ++c; return c;
    }

    void setOnChange(Ebev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ebev1Channel>      m_channels;
    std::vector<Ebev1Subscription> m_subscriptions;
    Ebev1ChangeCallback            m_onChange;
};

} // namespace NF
