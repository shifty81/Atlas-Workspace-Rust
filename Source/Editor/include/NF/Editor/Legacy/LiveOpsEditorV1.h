#pragma once
// NF::Editor — LiveOps editor v1: campaigns, events, push notifications management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lopsv1CampaignState : uint8_t { Draft, Scheduled, Active, Paused, Ended, Archived };
enum class Lopsv1EventType     : uint8_t { Push, InApp, Email, Discount, Challenge };

inline const char* lopsv1CampaignStateName(Lopsv1CampaignState s) {
    switch (s) {
        case Lopsv1CampaignState::Draft:     return "Draft";
        case Lopsv1CampaignState::Scheduled: return "Scheduled";
        case Lopsv1CampaignState::Active:    return "Active";
        case Lopsv1CampaignState::Paused:    return "Paused";
        case Lopsv1CampaignState::Ended:     return "Ended";
        case Lopsv1CampaignState::Archived:  return "Archived";
    }
    return "Unknown";
}

inline const char* lopsv1EventTypeName(Lopsv1EventType t) {
    switch (t) {
        case Lopsv1EventType::Push:      return "Push";
        case Lopsv1EventType::InApp:     return "InApp";
        case Lopsv1EventType::Email:     return "Email";
        case Lopsv1EventType::Discount:  return "Discount";
        case Lopsv1EventType::Challenge: return "Challenge";
    }
    return "Unknown";
}

struct Lopsv1Event {
    std::string     name;
    Lopsv1EventType type = Lopsv1EventType::Push;
    bool isValid() const { return !name.empty(); }
};

struct Lopsv1Campaign {
    uint64_t            id     = 0;
    std::string         name;
    Lopsv1CampaignState state  = Lopsv1CampaignState::Draft;
    Lopsv1EventType     type   = Lopsv1EventType::Push;
    std::vector<Lopsv1Event> events;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Lopsv1CampaignState::Active; }
    [[nodiscard]] bool isArchived() const { return state == Lopsv1CampaignState::Archived; }
    [[nodiscard]] bool isPaused()   const { return state == Lopsv1CampaignState::Paused; }

    bool addEvent(const Lopsv1Event& event) {
        if (!event.isValid()) return false;
        for (const auto& e : events) if (e.name == event.name) return false;
        events.push_back(event);
        return true;
    }
};

using Lopsv1ChangeCallback = std::function<void(uint64_t)>;

class LiveOpsEditorV1 {
public:
    static constexpr size_t MAX_CAMPAIGNS = 256;

    bool addCampaign(const Lopsv1Campaign& campaign) {
        if (!campaign.isValid()) return false;
        for (const auto& c : m_campaigns) if (c.id == campaign.id) return false;
        if (m_campaigns.size() >= MAX_CAMPAIGNS) return false;
        m_campaigns.push_back(campaign);
        return true;
    }

    bool removeCampaign(uint64_t id) {
        for (auto it = m_campaigns.begin(); it != m_campaigns.end(); ++it) {
            if (it->id == id) { m_campaigns.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lopsv1Campaign* findCampaign(uint64_t id) {
        for (auto& c : m_campaigns) if (c.id == id) return &c;
        return nullptr;
    }

    bool setState(uint64_t id, Lopsv1CampaignState state) {
        auto* c = findCampaign(id);
        if (!c) return false;
        c->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t campaignCount()  const { return m_campaigns.size(); }
    [[nodiscard]] size_t activeCount()    const {
        size_t c = 0; for (const auto& camp : m_campaigns) if (camp.isActive())   ++c; return c;
    }
    [[nodiscard]] size_t archivedCount()  const {
        size_t c = 0; for (const auto& camp : m_campaigns) if (camp.isArchived()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Lopsv1EventType type) const {
        size_t c = 0; for (const auto& camp : m_campaigns) if (camp.type == type) ++c; return c;
    }

    void setOnChange(Lopsv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lopsv1Campaign> m_campaigns;
    Lopsv1ChangeCallback        m_onChange;
};

} // namespace NF
