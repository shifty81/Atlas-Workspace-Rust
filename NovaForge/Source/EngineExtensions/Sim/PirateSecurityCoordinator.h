#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "sim/AIPirateStateMachine.h"
#include "sim/AISecurityStateMachine.h"

namespace atlas::sim {

/**
 * Describes a hauler trade route that pirates can target.
 */
struct TradeRoute {
    uint32_t haulerEntityId    = 0;
    std::string originSystem;
    std::string destinationSystem;
    float cargoValue           = 0.0f;   // Credits
    float securityLevel        = 1.0f;   // 0=lawless, 1=secure
};

/**
 * Represents loot dropped by a destroyed hauler.
 */
struct LootDrop {
    uint32_t fromEntityId      = 0;      // destroyed hauler
    uint32_t pirateEntityId    = 0;      // pirate that killed it
    float cargoValue           = 0.0f;   // Credits value of cargo
    float timestamp            = 0.0f;   // sim time when drop occurred
};

/**
 * Faction standing entry for pirate/security factions.
 */
struct FactionStandingEntry {
    std::string factionName;
    float standing = 0.0f;               // -10 to +10
};

/**
 * @brief Coordinates pirate AI and security response AI as an integrated loop.
 *
 * Implements Item 4 of the Immediate Action Plan:
 *   - Pirates patrol trade routes and attack haulers
 *   - Security responds to pirate activity
 *   - Faction standings: pirates = -10, security = +10
 *   - Dynamic spawn scaling: more pirates → more security
 *   - Loot drops from destroyed haulers
 *   - Economic impact: hauler deaths → price increases
 *
 * Usage:
 *   AIPirateStateMachine pirates;
 *   AISecurityStateMachine security;
 *   PirateSecurityCoordinator coord;
 *   coord.SetPirateStateMachine(&pirates);
 *   coord.SetSecurityStateMachine(&security);
 *   coord.AddTradeRoute({100, "Jita", "Amarr", 5000.0f, 0.8f});
 *   coord.Tick(deltaTime);
 */
class PirateSecurityCoordinator {
public:
    PirateSecurityCoordinator() = default;

    // ── Wire up state machines ──────────────────────────────────────
    void SetPirateStateMachine(AIPirateStateMachine* sm) { m_pirateSM = sm; }
    void SetSecurityStateMachine(AISecurityStateMachine* sm) { m_securitySM = sm; }

    AIPirateStateMachine* GetPirateStateMachine() const { return m_pirateSM; }
    AISecurityStateMachine* GetSecurityStateMachine() const { return m_securitySM; }

    // ── Trade route management ──────────────────────────────────────
    /** Add a trade route that pirates can target. */
    bool AddTradeRoute(const TradeRoute& route);

    /** Remove a trade route by hauler entity id. */
    bool RemoveTradeRoute(uint32_t haulerEntityId);

    /** Get a trade route by hauler entity id. Returns nullptr if not found. */
    const TradeRoute* GetTradeRoute(uint32_t haulerEntityId) const;

    /** Number of active trade routes. */
    size_t TradeRouteCount() const { return m_tradeRoutes.size(); }

    // ── Tick / simulation ───────────────────────────────────────────
    /** Advance the coordination loop by deltaSeconds. */
    void Tick(float deltaSeconds);

    // ── Pirate activity level ───────────────────────────────────────
    /** Set the current pirate activity level (0-1). */
    void SetPirateActivityLevel(float level);

    /** Get current pirate activity level. */
    float GetPirateActivityLevel() const { return m_pirateActivityLevel; }

    // ── Security scaling ────────────────────────────────────────────
    /** Recommended number of security guards based on pirate activity. */
    int GetRecommendedSecurityCount() const;

    /** Set security-to-pirate ratio (default 1.5: 3 guards per 2 pirates). */
    void SetSecurityPerPirateRatio(float ratio) { m_securityPerPirateRatio = ratio; }
    float GetSecurityPerPirateRatio() const { return m_securityPerPirateRatio; }

    /** Maximum security guards allowed. */
    void SetMaxSecurityGuards(int max) { m_maxSecurityGuards = max; }
    int GetMaxSecurityGuards() const { return m_maxSecurityGuards; }

    // ── Faction standings ───────────────────────────────────────────
    /** Get pirate faction standing (always negative). */
    FactionStandingEntry GetPirateStanding() const;

    /** Get security faction standing (always positive). */
    FactionStandingEntry GetSecurityStanding() const;

    // ── Loot drops ──────────────────────────────────────────────────
    /** Pending loot drops from destroyed haulers. */
    const std::vector<LootDrop>& GetPendingLootDrops() const { return m_pendingLootDrops; }

    /** Clear consumed loot drops. */
    void ClearLootDrops() { m_pendingLootDrops.clear(); }

    /** Force-generate a loot drop (e.g. hauler destroyed by pirate). */
    void GenerateLootDrop(uint32_t haulerEntityId, uint32_t pirateEntityId, float cargoValue);

    // ── Economic impact ─────────────────────────────────────────────
    /** Price impact multiplier (>1.0 = prices increased due to hauler losses). */
    float GetPriceImpact() const { return m_priceImpact; }

    // ── Aggregate stats ─────────────────────────────────────────────
    uint32_t TotalHaulersDestroyed() const { return m_totalHaulersDestroyed; }
    float TotalCargoLost() const { return m_totalCargoLost; }
    float TotalLootRecovered() const { return m_totalLootRecovered; }
    float SimTime() const { return m_simTime; }

    // ── Entity tracking ─────────────────────────────────────────────
    /** Register a pirate entity id so the coordinator can track its state. */
    void TrackPirate(uint32_t entityId);

    /** Register a security guard entity id. */
    void TrackGuard(uint32_t entityId);

    /** Untrack a pirate. */
    void UntrackPirate(uint32_t entityId);

    /** Untrack a guard. */
    void UntrackGuard(uint32_t entityId);

    /** Tracked pirate ids. */
    const std::vector<uint32_t>& TrackedPirateIds() const { return m_trackedPirateIds; }

    /** Tracked guard ids. */
    const std::vector<uint32_t>& TrackedGuardIds() const { return m_trackedGuardIds; }

    // ── Configuration ───────────────────────────────────────────────
    /** Attack chance per tick when pirate scans a low-sec route (default 0.1 = 10%). */
    void SetAttackChancePerScan(float chance) { m_attackChancePerScan = chance; }
    float GetAttackChancePerScan() const { return m_attackChancePerScan; }

private:
    void updatePirateTargets();
    void updateSecurityThreats();
    void checkHaulerDeaths();
    void calculateEconomicImpact(float dt);
    void updateActivityLevel();

    AIPirateStateMachine*  m_pirateSM  = nullptr;
    AISecurityStateMachine* m_securitySM = nullptr;

    std::vector<TradeRoute>  m_tradeRoutes;
    std::vector<LootDrop>    m_pendingLootDrops;
    std::vector<uint32_t>    m_trackedPirateIds;
    std::vector<uint32_t>    m_trackedGuardIds;

    float    m_pirateActivityLevel     = 0.0f;
    float    m_securityPerPirateRatio  = 1.5f;
    int      m_maxSecurityGuards       = 20;
    float    m_priceImpact             = 1.0f;
    float    m_attackChancePerScan     = 0.1f;  // 10%
    float    m_simTime                 = 0.0f;

    uint32_t m_totalHaulersDestroyed   = 0;
    float    m_totalCargoLost          = 0.0f;
    float    m_totalLootRecovered      = 0.0f;

    static constexpr float PRICE_DECAY_RATE         = 0.01f;  // per second towards 1.0
    static constexpr float PRICE_INCREASE_PER_HAULER = 0.1f;  // per hauler destroyed
    static constexpr float PIRATE_STANDING           = -10.0f;
    static constexpr float SECURITY_STANDING         = 10.0f;
};

} // namespace atlas::sim
