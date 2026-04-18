#include "sim/PirateSecurityCoordinator.h"
#include <algorithm>
#include <cmath>

namespace atlas::sim {

// ── Trade route management ──────────────────────────────────────────

bool PirateSecurityCoordinator::AddTradeRoute(const TradeRoute& route) {
    for (auto& r : m_tradeRoutes) {
        if (r.haulerEntityId == route.haulerEntityId) return false;
    }
    m_tradeRoutes.push_back(route);
    return true;
}

bool PirateSecurityCoordinator::RemoveTradeRoute(uint32_t haulerEntityId) {
    auto it = std::remove_if(m_tradeRoutes.begin(), m_tradeRoutes.end(),
        [haulerEntityId](const TradeRoute& r) { return r.haulerEntityId == haulerEntityId; });
    if (it == m_tradeRoutes.end()) return false;
    m_tradeRoutes.erase(it, m_tradeRoutes.end());
    return true;
}

const TradeRoute* PirateSecurityCoordinator::GetTradeRoute(uint32_t haulerEntityId) const {
    for (auto& r : m_tradeRoutes) {
        if (r.haulerEntityId == haulerEntityId) return &r;
    }
    return nullptr;
}

// ── Entity tracking ─────────────────────────────────────────────────

void PirateSecurityCoordinator::TrackPirate(uint32_t entityId) {
    if (std::find(m_trackedPirateIds.begin(), m_trackedPirateIds.end(), entityId)
            == m_trackedPirateIds.end()) {
        m_trackedPirateIds.push_back(entityId);
    }
}

void PirateSecurityCoordinator::TrackGuard(uint32_t entityId) {
    if (std::find(m_trackedGuardIds.begin(), m_trackedGuardIds.end(), entityId)
            == m_trackedGuardIds.end()) {
        m_trackedGuardIds.push_back(entityId);
    }
}

void PirateSecurityCoordinator::UntrackPirate(uint32_t entityId) {
    m_trackedPirateIds.erase(
        std::remove(m_trackedPirateIds.begin(), m_trackedPirateIds.end(), entityId),
        m_trackedPirateIds.end());
}

void PirateSecurityCoordinator::UntrackGuard(uint32_t entityId) {
    m_trackedGuardIds.erase(
        std::remove(m_trackedGuardIds.begin(), m_trackedGuardIds.end(), entityId),
        m_trackedGuardIds.end());
}

// ── Tick ─────────────────────────────────────────────────────────────

void PirateSecurityCoordinator::Tick(float deltaSeconds) {
    m_simTime += deltaSeconds;

    // 1. Feed hauler entity ids to pirate state machine as targets
    updatePirateTargets();

    // 2. Feed active attacking pirate ids to security as threats
    updateSecurityThreats();

    // 3. Tick both state machines
    if (m_pirateSM)  m_pirateSM->Tick(deltaSeconds);
    if (m_securitySM) m_securitySM->Tick(deltaSeconds);

    // 4. Check for hauler deaths (pirates scored kills)
    checkHaulerDeaths();

    // 5. Update activity level from pirate state machine
    updateActivityLevel();

    // 6. Calculate economic impact
    calculateEconomicImpact(deltaSeconds);
}

// ── Pirate activity ──────────────────────────────────────────────────

void PirateSecurityCoordinator::SetPirateActivityLevel(float level) {
    m_pirateActivityLevel = std::clamp(level, 0.0f, 1.0f);
}

// ── Security scaling ─────────────────────────────────────────────────

int PirateSecurityCoordinator::GetRecommendedSecurityCount() const {
    if (!m_pirateSM) return 0;
    int pirateCount = static_cast<int>(m_pirateSM->PirateCount());
    int recommended = static_cast<int>(std::ceil(pirateCount * m_securityPerPirateRatio));
    return std::min(recommended, m_maxSecurityGuards);
}

// ── Faction standings ────────────────────────────────────────────────

FactionStandingEntry PirateSecurityCoordinator::GetPirateStanding() const {
    return {"Venom Syndicate", PIRATE_STANDING};
}

FactionStandingEntry PirateSecurityCoordinator::GetSecurityStanding() const {
    return {"CONCORD", SECURITY_STANDING};
}

// ── Loot drops ───────────────────────────────────────────────────────

void PirateSecurityCoordinator::GenerateLootDrop(uint32_t haulerEntityId,
                                                  uint32_t pirateEntityId,
                                                  float cargoValue) {
    LootDrop drop;
    drop.fromEntityId   = haulerEntityId;
    drop.pirateEntityId = pirateEntityId;
    drop.cargoValue     = cargoValue;
    drop.timestamp      = m_simTime;
    m_pendingLootDrops.push_back(drop);
    m_totalCargoLost += cargoValue;
    m_totalLootRecovered += cargoValue;
    m_totalHaulersDestroyed++;
    // Hauler destruction increases prices
    m_priceImpact += PRICE_INCREASE_PER_HAULER;
}

// ── Internal: feed hauler ids to pirate target list ──────────────────

void PirateSecurityCoordinator::updatePirateTargets() {
    if (!m_pirateSM) return;

    // Pirates target haulers on low-security routes (sec < 0.5)
    std::vector<uint32_t> targets;
    for (auto& route : m_tradeRoutes) {
        if (route.securityLevel < 0.5f) {
            targets.push_back(route.haulerEntityId);
        }
    }
    m_pirateSM->SetTargetsInRange(targets);
}

// ── Internal: feed attacking pirate ids to security threat list ───────

void PirateSecurityCoordinator::updateSecurityThreats() {
    if (!m_pirateSM || !m_securitySM) return;

    // Collect all tracked pirate entity ids that are in hostile states
    std::vector<uint32_t> threats;
    for (uint32_t id : m_trackedPirateIds) {
        const PirateRuntime* rt = m_pirateSM->GetPirate(id);
        if (!rt) continue;
        if (rt->state == PirateState::Attack ||
            rt->state == PirateState::PursueTarget ||
            rt->state == PirateState::LootCargo) {
            threats.push_back(id);
        }
    }

    m_securitySM->SetActiveThreats(threats);
}

// ── Internal: detect hauler kills and generate loot ──────────────────

void PirateSecurityCoordinator::checkHaulerDeaths() {
    if (!m_pirateSM) return;

    // When a pirate transitions to LootCargo, a hauler was destroyed.
    // Check tracked pirates in LootCargo state and match to trade routes.
    for (auto& route : m_tradeRoutes) {
        for (uint32_t id : m_trackedPirateIds) {
            const PirateRuntime* rt = m_pirateSM->GetPirate(id);
            if (!rt) continue;
            if (rt->state == PirateState::LootCargo &&
                rt->targetEntityId == route.haulerEntityId) {
                // Check if we already generated a loot drop for this pair
                bool alreadyDropped = false;
                for (auto& drop : m_pendingLootDrops) {
                    if (drop.fromEntityId == route.haulerEntityId &&
                        drop.pirateEntityId == id) {
                        alreadyDropped = true;
                        break;
                    }
                }
                if (!alreadyDropped) {
                    GenerateLootDrop(route.haulerEntityId, id, route.cargoValue);
                }
            }
        }
    }
}

// ── Internal: update activity from pirate state ──────────────────────

void PirateSecurityCoordinator::updateActivityLevel() {
    if (!m_pirateSM) return;

    size_t total = m_pirateSM->PirateCount();
    if (total == 0) {
        m_pirateActivityLevel = 0.0f;
        return;
    }

    // Activity = fraction of tracked pirates in hostile or active states
    size_t active = 0;
    for (uint32_t id : m_trackedPirateIds) {
        const PirateRuntime* rt = m_pirateSM->GetPirate(id);
        if (!rt) continue;
        if (rt->state == PirateState::Attack ||
            rt->state == PirateState::PursueTarget ||
            rt->state == PirateState::LootCargo ||
            rt->state == PirateState::Patrol ||
            rt->state == PirateState::ScanForTargets) {
            ++active;
        }
    }
    m_pirateActivityLevel = std::clamp(
        static_cast<float>(active) / static_cast<float>(total), 0.0f, 1.0f);
}

// ── Internal: economic impact from hauler losses ─────────────────────

void PirateSecurityCoordinator::calculateEconomicImpact(float dt) {
    // Price impact decays towards 1.0 over time
    if (m_priceImpact > 1.0f) {
        m_priceImpact -= PRICE_DECAY_RATE * dt;
        if (m_priceImpact < 1.0f) m_priceImpact = 1.0f;
    }
}

} // namespace atlas::sim
