#pragma once
/**
 * @file sector_simulation.h
 * @brief Faction ecosystem and heat propagation — Phase 24.
 *
 * Sectors are generated from seed with variable density:
 *   Stations     1–7 (rolled per system)
 *   Mining Belts 2–8 (biased by security level)
 *   Pirate Zone  1 (weighted by security)
 *   Anomalies    3–5
 *   AI Traffic   scaled to station + belt count
 *
 * Heat propagation:
 *   Player actions emit heat → pressure propagates to neighbours each tick.
 *
 * Drift stabilizers:
 *   Faction Fatigue    — dominant factions spawn internal instability
 *   Economic Elasticity — markets drift toward equilibrium
 *   Power Vacuum       — destroyed stations create temporary pirate nodes
 *   Era Resets         — periodic faction and resource rebalancing
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <functional>

namespace atlas::gameplay {

// ── Faction definitions ──────────────────────────────────────────────────────

enum class FactionID : uint8_t {
    PLAYER,
    TRADE_ALLIANCE,
    MINING_CORP,
    PIRATE_COALITION,
    SECURITY_FORCE,
    RESEARCH_ORDER,
    NEUTRAL
};

inline const char* FactionName(FactionID f) {
    switch (f) {
        case FactionID::PLAYER:           return "Player";
        case FactionID::TRADE_ALLIANCE:   return "Trade Alliance";
        case FactionID::MINING_CORP:      return "Mining Corp";
        case FactionID::PIRATE_COALITION: return "Pirate Coalition";
        case FactionID::SECURITY_FORCE:   return "Security Force";
        case FactionID::RESEARCH_ORDER:   return "Research Order";
        case FactionID::NEUTRAL:          return "Neutral";
    }
    return "Unknown";
}

// ── Faction doctrine ─────────────────────────────────────────────────────────

struct FactionDoctrine {
    FactionID   faction      = FactionID::NEUTRAL;
    float       aggression   = 0.3f;   ///< 0 = peaceful, 1 = war-like
    float       expansion    = 0.3f;   ///< 0 = isolationist, 1 = expansive
    float       lawfulness   = 0.7f;   ///< 0 = lawless, 1 = law-abiding
};

// ── Sector model ─────────────────────────────────────────────────────────────

struct Sector {
    std::string  id;
    std::string  name;
    FactionID    controllingFaction = FactionID::NEUTRAL;
    float        stability         = 1.0f;  ///< 0 = chaotic, 1 = stable
    float        hostility         = 0.2f;  ///< toward player and outsiders
    float        resourceWealth    = 0.5f;  ///< loot and supply potential
    float        heat              = 0.0f;  ///< accumulated player pressure

    std::vector<std::string> neighbours;    ///< adjacent sector IDs

    int stationCount   = 1;
    int miningBeltCount = 2;
    int pirateZones    = 1;
    int anomalyCount   = 3;
};

// ── SectorSimulation ─────────────────────────────────────────────────────────

/**
 * Manages sector state, heat propagation, and faction ecosystem dynamics.
 *
 * Callers register sectors and their neighbourhood graph, then feed player
 * actions via EmitHeat().  Tick() propagates heat and applies drift stabilizers.
 */
class SectorSimulation {
public:
    SectorSimulation() = default;

    // ── Sector registry ────────────────────────────────────────────────

    void AddSector(const Sector& sector);
    Sector* GetSector(const std::string& sectorID);
    const Sector* GetSector(const std::string& sectorID) const;

    int SectorCount() const { return static_cast<int>(m_sectors.size()); }

    // ── Heat propagation ───────────────────────────────────────────────

    /**
     * Emit heat in a sector from a player action.
     * @param sectorID  Sector where the action occurred.
     * @param factionID Faction targeted / offended.
     * @param magnitude 0–1 heat magnitude.
     */
    void EmitHeat(const std::string& sectorID,
                  FactionID factionID,
                  float magnitude);

    // ── Per-tick simulation ────────────────────────────────────────────

    /**
     * Propagate heat to neighbour sectors, apply drift stabilizers,
     * update faction control.
     * @param deltaTime Simulation seconds.
     */
    void Tick(float deltaTime);

    // ── Drift stabilizers ──────────────────────────────────────────────

    /**
     * Force an era reset — rebalances dominant factions and resources.
     * Called periodically or after major player-driven disruptions.
     */
    void TriggerEraReset();

    float GalaxyPressure() const { return m_galaxyPressure; }

    // ── Faction doctrine ───────────────────────────────────────────────

    void SetFactionDoctrine(const FactionDoctrine& doctrine);
    const FactionDoctrine* GetFactionDoctrine(FactionID faction) const;

    // ── Constants ─────────────────────────────────────────────────────

    static constexpr float HEAT_PROPAGATION_RATE = 0.3f;  ///< per neighbour per tick
    static constexpr float HEAT_DECAY_RATE        = 0.05f; ///< per second
    static constexpr float STABILITY_DECAY_RATE   = 0.01f; ///< per unit heat
    static constexpr float FACTION_FATIGUE_THRESHOLD = 0.8f;
    static constexpr float ECONOMIC_EQUILIBRIUM   = 0.5f;

private:
    void ApplyFactionFatigue(Sector& sector);
    void ApplyEconomicElasticity(Sector& sector);
    void CheckPowerVacuum(Sector& sector);

    std::unordered_map<std::string, Sector> m_sectors;
    std::unordered_map<uint8_t, FactionDoctrine> m_doctrines;
    float m_galaxyPressure = 0.0f;
    float m_eraTimer       = 0.0f;
    static constexpr float ERA_RESET_INTERVAL = 3600.0f; ///< seconds
};

} // namespace atlas::gameplay
