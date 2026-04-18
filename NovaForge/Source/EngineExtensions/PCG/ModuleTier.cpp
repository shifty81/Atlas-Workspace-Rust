#include "ModuleTier.h"

namespace atlas::procedural {

// ---- Static tier data ----

namespace {

struct TierData {
    TierBonus bonus;
    TierVisual visual;
};

const TierData& GetTierData(TierLevel tier) {
    static const TierData data[5] = {
        // Tier 1: basic grey, no glow
        {{1.0f, 1.0f, 1.0f, "Basic"},
         {0.6f, 0.6f, 0.6f, 0.0f, 0.0f, 0}},
        // Tier 2: green-tinted, faint glow
        {{1.2f, 1.2f, 1.2f, "Improved"},
         {0.4f, 0.75f, 0.4f, 0.15f, 0.1f, 1}},
        // Tier 3: blue, moderate glow
        {{1.5f, 1.5f, 1.5f, "Advanced"},
         {0.3f, 0.5f, 0.9f, 0.4f, 0.3f, 2}},
        // Tier 4: purple, strong glow
        {{2.0f, 2.0f, 2.0f, "Superior"},
         {0.7f, 0.3f, 0.9f, 0.7f, 0.6f, 3}},
        // Tier 5: gold, max glow
        {{3.0f, 3.0f, 3.0f, "Legendary"},
         {1.0f, 0.85f, 0.2f, 1.0f, 1.0f, 4}}
    };
    int idx = static_cast<int>(tier) - 1;
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return data[idx];
}

} // anonymous namespace

TierBonus ModuleTierSystem::GetBonus(TierLevel tier) const {
    return GetTierData(tier).bonus;
}

TierVisual ModuleTierSystem::GetVisual(TierLevel tier) const {
    return GetTierData(tier).visual;
}

TierRequirement ModuleTierSystem::GetUpgradeRequirement(TierLevel fromTier,
                                                         TierLevel toTier) const {
    if (static_cast<uint8_t>(toTier) <= static_cast<uint8_t>(fromTier)) {
        return {};  // No downgrade or same-tier requirement.
    }

    // Cumulative cost per tier step.
    static const uint64_t creditsCosts[5] = {0, 1000, 5000, 25000, 100000};
    static const uint32_t timeCosts[5]    = {0, 60, 300, 1200, 3600};
    static const uint8_t  techLevels[5]   = {0, 1, 2, 3, 5};

    uint64_t totalCredits = 0;
    uint32_t totalTime = 0;
    uint8_t maxTech = 0;

    for (int t = static_cast<int>(fromTier); t < static_cast<int>(toTier); ++t) {
        if (t >= 0 && t < 5) {
            totalCredits += creditsCosts[t];
            totalTime += timeCosts[t];
            if (techLevels[t] > maxTech) maxTech = techLevels[t];
        }
    }

    return {totalCredits, totalTime, maxTech};
}

bool ModuleTierSystem::CanUpgrade(TierLevel current, TierLevel target,
                                   uint64_t credits, uint8_t techLevel) const {
    if (static_cast<uint8_t>(target) <= static_cast<uint8_t>(current)) return false;
    if (static_cast<uint8_t>(target) > 5) return false;

    TierRequirement req = GetUpgradeRequirement(current, target);
    return credits >= req.creditsCost && techLevel >= req.requiredTechLevel;
}

float ModuleTierSystem::ApplyBonus(float baseValue, TierLevel tier) const {
    return baseValue * GetBonus(tier).healthMultiplier;
}

const char* ModuleTierSystem::TierName(TierLevel tier) {
    switch (tier) {
        case TierLevel::Tier1: return "Tier 1";
        case TierLevel::Tier2: return "Tier 2";
        case TierLevel::Tier3: return "Tier 3";
        case TierLevel::Tier4: return "Tier 4";
        case TierLevel::Tier5: return "Tier 5";
        default:               return "Unknown";
    }
}

} // namespace atlas::procedural
