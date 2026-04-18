#pragma once
// NF::Editor — Platform publish editor v1: store listing and publish submission authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ppev1Platform    : uint8_t { PC, Console, Mobile, Web, VR, CloudStreaming };
enum class Ppev1StoreRegion : uint8_t { Global, NA, EU, APAC, CN, LATAM };
enum class Ppev1PublishState: uint8_t { Draft, Review, Approved, Live, Rejected, Archived };

inline const char* ppev1PlatformName(Ppev1Platform p) {
    switch (p) {
        case Ppev1Platform::PC:             return "PC";
        case Ppev1Platform::Console:        return "Console";
        case Ppev1Platform::Mobile:         return "Mobile";
        case Ppev1Platform::Web:            return "Web";
        case Ppev1Platform::VR:             return "VR";
        case Ppev1Platform::CloudStreaming: return "CloudStreaming";
    }
    return "Unknown";
}

inline const char* ppev1StoreRegionName(Ppev1StoreRegion r) {
    switch (r) {
        case Ppev1StoreRegion::Global: return "Global";
        case Ppev1StoreRegion::NA:     return "NA";
        case Ppev1StoreRegion::EU:     return "EU";
        case Ppev1StoreRegion::APAC:   return "APAC";
        case Ppev1StoreRegion::CN:     return "CN";
        case Ppev1StoreRegion::LATAM:  return "LATAM";
    }
    return "Unknown";
}

inline const char* ppev1PublishStateName(Ppev1PublishState s) {
    switch (s) {
        case Ppev1PublishState::Draft:    return "Draft";
        case Ppev1PublishState::Review:   return "Review";
        case Ppev1PublishState::Approved: return "Approved";
        case Ppev1PublishState::Live:     return "Live";
        case Ppev1PublishState::Rejected: return "Rejected";
        case Ppev1PublishState::Archived: return "Archived";
    }
    return "Unknown";
}

struct Ppev1StoreListing {
    uint64_t            id           = 0;
    std::string         name;
    std::string         version;
    Ppev1Platform       platform     = Ppev1Platform::PC;
    Ppev1StoreRegion    region       = Ppev1StoreRegion::Global;
    Ppev1PublishState   state        = Ppev1PublishState::Draft;
    std::string         storeId;
    std::string         buildArtifactRef;
    bool                ageRated     = false;
    bool                dlcEnabled   = false;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty() && !version.empty(); }
    [[nodiscard]] bool isLive()     const { return state == Ppev1PublishState::Live; }
    [[nodiscard]] bool isApproved() const { return state == Ppev1PublishState::Approved; }
    [[nodiscard]] bool isRejected() const { return state == Ppev1PublishState::Rejected; }
};

using Ppev1ChangeCallback = std::function<void(uint64_t)>;

class PlatformPublishEditorV1 {
public:
    static constexpr size_t MAX_LISTINGS = 128;

    bool addListing(const Ppev1StoreListing& listing) {
        if (!listing.isValid()) return false;
        for (const auto& l : m_listings) if (l.id == listing.id) return false;
        if (m_listings.size() >= MAX_LISTINGS) return false;
        m_listings.push_back(listing);
        return true;
    }

    bool removeListing(uint64_t id) {
        for (auto it = m_listings.begin(); it != m_listings.end(); ++it) {
            if (it->id == id) { m_listings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ppev1StoreListing* findListing(uint64_t id) {
        for (auto& l : m_listings) if (l.id == id) return &l;
        return nullptr;
    }

    bool setState(uint64_t id, Ppev1PublishState state) {
        auto* l = findListing(id);
        if (!l) return false;
        l->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool submitForReview(uint64_t id) {
        auto* l = findListing(id);
        if (!l || l->state != Ppev1PublishState::Draft) return false;
        l->state = Ppev1PublishState::Review;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setAgeRated(uint64_t id, bool rated) {
        auto* l = findListing(id);
        if (!l) return false;
        l->ageRated = rated;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t listingCount()   const { return m_listings.size(); }
    [[nodiscard]] size_t liveCount()      const {
        size_t c = 0; for (const auto& l : m_listings) if (l.isLive())     ++c; return c;
    }
    [[nodiscard]] size_t approvedCount()  const {
        size_t c = 0; for (const auto& l : m_listings) if (l.isApproved()) ++c; return c;
    }
    [[nodiscard]] size_t rejectedCount()  const {
        size_t c = 0; for (const auto& l : m_listings) if (l.isRejected()) ++c; return c;
    }
    [[nodiscard]] size_t countByPlatform(Ppev1Platform platform) const {
        size_t c = 0; for (const auto& l : m_listings) if (l.platform == platform) ++c; return c;
    }
    [[nodiscard]] size_t countByRegion(Ppev1StoreRegion region) const {
        size_t c = 0; for (const auto& l : m_listings) if (l.region == region) ++c; return c;
    }

    void setOnChange(Ppev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ppev1StoreListing> m_listings;
    Ppev1ChangeCallback            m_onChange;
};

} // namespace NF
