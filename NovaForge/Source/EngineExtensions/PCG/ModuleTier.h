#pragma once
#include <cstdint>
#include <string>

namespace atlas::procedural {

/// Tier levels for module upgrades.
enum class TierLevel : uint8_t {
    Tier1 = 1,
    Tier2 = 2,
    Tier3 = 3,
    Tier4 = 4,
    Tier5 = 5
};

/// Stat multipliers granted by a tier.
struct TierBonus {
    float healthMultiplier = 1.0f;
    float efficiencyMultiplier = 1.0f;
    float powerMultiplier = 1.0f;
    std::string name;
};

/// Visual properties associated with a tier.
struct TierVisual {
    float colorR = 0.6f;
    float colorG = 0.6f;
    float colorB = 0.6f;
    float glowIntensity = 0.0f;
    float particleDensity = 0.0f;
    uint8_t decalLayer = 0;
};

/// Resources required to upgrade from one tier to another.
struct TierRequirement {
    uint64_t creditsCost = 0;
    uint32_t timeCostSeconds = 0;
    uint8_t requiredTechLevel = 0;
};

/// Provides tier-related bonuses, visuals, and upgrade rules for modules.
class ModuleTierSystem {
public:
    /// Get the stat bonus for a tier.
    TierBonus GetBonus(TierLevel tier) const;

    /// Get the visual descriptor for a tier.
    TierVisual GetVisual(TierLevel tier) const;

    /// Get the requirement to upgrade from one tier to another.
    TierRequirement GetUpgradeRequirement(TierLevel fromTier, TierLevel toTier) const;

    /// Check whether upgrading is affordable.
    bool CanUpgrade(TierLevel current, TierLevel target,
                    uint64_t credits, uint8_t techLevel) const;

    /// Apply the tier's health multiplier to a base value.
    float ApplyBonus(float baseValue, TierLevel tier) const;

    /// The maximum tier level.
    static TierLevel MaxTier() { return TierLevel::Tier5; }

    /// Human-readable name for a tier.
    static const char* TierName(TierLevel tier);

    /// Total number of tiers.
    static uint8_t TierCount() { return 5; }
};

} // namespace atlas::procedural
