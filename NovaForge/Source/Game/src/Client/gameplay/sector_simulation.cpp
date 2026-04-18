/**
 * @file sector_simulation.cpp
 * @brief SectorSimulation implementation.
 */

#include "gameplay/sector_simulation.h"
#include <algorithm>
#include <cmath>

namespace atlas::gameplay {

void SectorSimulation::AddSector(const Sector& sector) {
    m_sectors[sector.id] = sector;
}

Sector* SectorSimulation::GetSector(const std::string& id) {
    auto it = m_sectors.find(id);
    return it != m_sectors.end() ? &it->second : nullptr;
}

const Sector* SectorSimulation::GetSector(const std::string& id) const {
    auto it = m_sectors.find(id);
    return it != m_sectors.end() ? &it->second : nullptr;
}

void SectorSimulation::EmitHeat(const std::string& sectorID,
                                FactionID /*factionID*/,
                                float magnitude) {
    Sector* sector = GetSector(sectorID);
    if (!sector) return;

    magnitude = std::max(0.0f, std::min(1.0f, magnitude));
    sector->heat = std::min(1.0f, sector->heat + magnitude);

    // Reduce stability as heat increases
    sector->stability = std::max(0.0f,
        sector->stability - STABILITY_DECAY_RATE * magnitude);
}

void SectorSimulation::Tick(float deltaTime) {
    // Propagate heat to neighbours
    for (auto& [id, sector] : m_sectors) {
        for (const auto& nbID : sector.neighbours) {
            Sector* nb = GetSector(nbID);
            if (!nb) continue;
            float transfer = sector.heat * HEAT_PROPAGATION_RATE * deltaTime;
            nb->heat = std::min(1.0f, nb->heat + transfer);
            nb->stability = std::max(0.0f,
                nb->stability - STABILITY_DECAY_RATE * transfer);
        }
    }

    // Decay heat
    for (auto& [id, sector] : m_sectors) {
        sector.heat = std::max(0.0f, sector.heat - HEAT_DECAY_RATE * deltaTime);
    }

    // Apply drift stabilizers
    for (auto& [id, sector] : m_sectors) {
        ApplyFactionFatigue(sector);
        ApplyEconomicElasticity(sector);
        CheckPowerVacuum(sector);
    }

    // Accumulate galaxy pressure (average heat)
    float total = 0.0f;
    for (const auto& [id, sector] : m_sectors)
        total += sector.heat;
    m_galaxyPressure = m_sectors.empty() ? 0.0f :
                       total / static_cast<float>(m_sectors.size());

    // Era reset timer
    m_eraTimer += deltaTime;
    if (m_eraTimer >= ERA_RESET_INTERVAL) {
        m_eraTimer = 0.0f;
        TriggerEraReset();
    }
}

void SectorSimulation::TriggerEraReset() {
    // Rebalance all sectors toward neutral stability and moderate resources
    for (auto& [id, sector] : m_sectors) {
        sector.stability      = std::min(1.0f, sector.stability + 0.2f);
        sector.heat           = sector.heat * 0.5f;
        sector.resourceWealth = ECONOMIC_EQUILIBRIUM +
                                (sector.resourceWealth - ECONOMIC_EQUILIBRIUM) * 0.5f;
    }
    m_galaxyPressure *= 0.5f;
}

void SectorSimulation::SetFactionDoctrine(const FactionDoctrine& doctrine) {
    m_doctrines[static_cast<uint8_t>(doctrine.faction)] = doctrine;
}

const FactionDoctrine* SectorSimulation::GetFactionDoctrine(FactionID faction) const {
    auto it = m_doctrines.find(static_cast<uint8_t>(faction));
    return it != m_doctrines.end() ? &it->second : nullptr;
}

void SectorSimulation::ApplyFactionFatigue(Sector& sector) {
    const FactionDoctrine* doc =
        GetFactionDoctrine(sector.controllingFaction);
    if (!doc) return;

    // Dominant, highly aggressive factions generate internal instability
    if (doc->aggression > FACTION_FATIGUE_THRESHOLD &&
        doc->expansion  > FACTION_FATIGUE_THRESHOLD) {
        sector.stability = std::max(0.0f, sector.stability - 0.001f);
    }
}

void SectorSimulation::ApplyEconomicElasticity(Sector& sector) {
    // Markets drift toward equilibrium price
    float diff = ECONOMIC_EQUILIBRIUM - sector.resourceWealth;
    sector.resourceWealth += diff * 0.001f;
    sector.resourceWealth = std::max(0.0f, std::min(1.0f, sector.resourceWealth));
}

void SectorSimulation::CheckPowerVacuum(Sector& sector) {
    // Very low stability → pirate activity increases
    if (sector.stability < 0.1f &&
        sector.controllingFaction != FactionID::PIRATE_COALITION) {
        sector.hostility = std::min(1.0f, sector.hostility + 0.01f);
    }
}

} // namespace atlas::gameplay
