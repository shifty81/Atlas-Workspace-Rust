#pragma once
#include "DeterministicRNG.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::procedural {

/// Types of zones in a planetary base.
enum class BaseZoneType : uint8_t {
    Landing,
    Habitat,
    Mining,
    Refinery,
    Defense,
    Research,
    Power,
    Communication,
    Storage,
    Medical,
    COUNT
};

/// A single zone in a planetary base layout.
struct BaseZone {
    uint32_t id = 0;
    BaseZoneType type = BaseZoneType::Landing;
    float x = 0.0f;
    float y = 0.0f;
    float radius = 10.0f;
    uint8_t level = 1;
    bool operational = true;
    std::string name;

    /// Area of the zone (π r²).
    float Area() const;
};

/// Configuration for planetary base generation.
struct PlanetaryBaseConfig {
    uint32_t maxZones = 20;
    float baseRadius = 200.0f;
    float minZoneSpacing = 5.0f;
    uint64_t seed = 42;

    static PlanetaryBaseConfig Default() { return {}; }
};

/// Manages a collection of zones forming a planetary base, with
/// support for procedural layout generation using DeterministicRNG.
class PlanetaryBase {
public:
    /// Initialize with the given configuration.
    void Init(const PlanetaryBaseConfig& cfg);

    /// Add a zone; returns its id, or 0 if placement is invalid.
    uint32_t AddZone(BaseZoneType type, float x, float y, float radius);

    /// Remove a zone by id.
    void RemoveZone(uint32_t zoneId);

    /// Get a zone by id (nullptr if not found).
    const BaseZone* GetZone(uint32_t zoneId) const;

    /// Number of zones in the base.
    size_t ZoneCount() const { return m_zones.size(); }

    /// Find all zone ids matching a type.
    std::vector<uint32_t> FindZonesByType(BaseZoneType type) const;

    /// Upgrade a zone's level (max 5).
    void UpgradeZone(uint32_t zoneId);

    /// Current configuration.
    const PlanetaryBaseConfig& GetConfig() const { return m_config; }

    /// True when the base has at least Landing + Power + Habitat.
    bool HasRequiredZones() const;

    /// Check whether a new zone can be placed without overlap.
    bool IsZoneSpacingValid(float x, float y, float radius) const;

    /// Procedurally generate a base layout from the given seed.
    void Generate(uint64_t seed);

    /// Remove all zones.
    void Clear();

    /// Number of operational zones.
    size_t OperationalZoneCount() const;

    /// Total area covered by all zones.
    float TotalArea() const;

private:
    PlanetaryBaseConfig m_config;
    std::vector<BaseZone> m_zones;
    uint32_t m_nextId = 1;
};

} // namespace atlas::procedural
