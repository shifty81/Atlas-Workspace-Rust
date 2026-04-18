/**
 * Tests for SectorSimulation (Phase 24 — Legend & World Simulation):
 *   - FactionID and FactionName helpers
 *   - Sector add / get
 *   - EmitHeat: sector heat increases, stability decreases
 *   - Tick: heat propagates to neighbours, decays
 *   - Faction doctrine storage and retrieval
 *   - Era reset
 *   - Galaxy pressure calculation
 */

#include <cassert>
#include <string>
#include <cmath>
#include "../cpp_client/include/gameplay/sector_simulation.h"

using namespace atlas::gameplay;

static Sector MakeSector(const std::string& id,
                          FactionID faction = FactionID::NEUTRAL,
                          float stability   = 1.0f) {
    Sector s;
    s.id                = id;
    s.controllingFaction = faction;
    s.stability         = stability;
    s.hostility         = 0.1f;
    s.resourceWealth    = 0.5f;
    s.heat              = 0.0f;
    return s;
}

void test_faction_names() {
    assert(std::string(FactionName(FactionID::PLAYER))           == "Player");
    assert(std::string(FactionName(FactionID::TRADE_ALLIANCE))   == "Trade Alliance");
    assert(std::string(FactionName(FactionID::PIRATE_COALITION)) == "Pirate Coalition");
    assert(std::string(FactionName(FactionID::SECURITY_FORCE))   == "Security Force");
}

void test_sector_add_and_get() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("sol"));
    assert(sim.SectorCount() == 1);
    assert(sim.GetSector("sol") != nullptr);
    assert(sim.GetSector("andromeda") == nullptr);
}

void test_sector_emit_heat_increases_heat() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("sol"));
    sim.EmitHeat("sol", FactionID::PIRATE_COALITION, 0.5f);
    const auto* s = sim.GetSector("sol");
    assert(s != nullptr);
    assert(s->heat == 0.5f);
}

void test_sector_emit_heat_reduces_stability() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("sol", FactionID::NEUTRAL, 1.0f));
    sim.EmitHeat("sol", FactionID::PIRATE_COALITION, 1.0f);
    const auto* s = sim.GetSector("sol");
    assert(s->stability < 1.0f);
}

void test_sector_emit_heat_clamped_at_1() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("sol"));
    sim.EmitHeat("sol", FactionID::NEUTRAL, 0.8f);
    sim.EmitHeat("sol", FactionID::NEUTRAL, 0.8f);
    const auto* s = sim.GetSector("sol");
    assert(s->heat <= 1.0f);
}

void test_sector_emit_heat_unknown_sector_noop() {
    SectorSimulation sim;
    sim.EmitHeat("nonexistent", FactionID::PLAYER, 1.0f);  // must not crash
    assert(sim.SectorCount() == 0);
}

void test_sector_tick_heat_decays() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("sol"));
    sim.EmitHeat("sol", FactionID::NEUTRAL, 0.5f);
    sim.Tick(1.0f);
    const auto* s = sim.GetSector("sol");
    assert(s->heat < 0.5f);
}

void test_sector_tick_propagates_to_neighbour() {
    SectorSimulation sim;
    Sector sol = MakeSector("sol");
    sol.neighbours.push_back("alpha");
    Sector alpha = MakeSector("alpha");
    sim.AddSector(sol);
    sim.AddSector(alpha);

    sim.EmitHeat("sol", FactionID::NEUTRAL, 1.0f);
    sim.Tick(1.0f);

    const auto* a = sim.GetSector("alpha");
    assert(a->heat > 0.0f);
}

void test_sector_galaxy_pressure_nonzero_after_heat() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("a"));
    sim.AddSector(MakeSector("b"));
    sim.EmitHeat("a", FactionID::PIRATE_COALITION, 1.0f);
    sim.Tick(0.1f);
    assert(sim.GalaxyPressure() > 0.0f);
}

void test_sector_doctrine_set_and_get() {
    SectorSimulation sim;
    FactionDoctrine doc;
    doc.faction    = FactionID::PIRATE_COALITION;
    doc.aggression = 0.9f;
    doc.expansion  = 0.8f;
    doc.lawfulness = 0.1f;
    sim.SetFactionDoctrine(doc);

    const auto* d = sim.GetFactionDoctrine(FactionID::PIRATE_COALITION);
    assert(d != nullptr);
    assert(d->aggression == 0.9f);
    assert(d->lawfulness == 0.1f);
}

void test_sector_doctrine_unknown_returns_null() {
    SectorSimulation sim;
    assert(sim.GetFactionDoctrine(FactionID::RESEARCH_ORDER) == nullptr);
}

void test_sector_era_reset_reduces_heat() {
    SectorSimulation sim;
    sim.AddSector(MakeSector("sol"));
    sim.EmitHeat("sol", FactionID::NEUTRAL, 1.0f);
    sim.TriggerEraReset();
    const auto* s = sim.GetSector("sol");
    assert(s->heat < 1.0f);
}

void test_sector_era_reset_improves_stability() {
    SectorSimulation sim;
    Sector s = MakeSector("sol");
    s.stability = 0.2f;
    sim.AddSector(s);
    sim.TriggerEraReset();
    const auto* updated = sim.GetSector("sol");
    assert(updated->stability > 0.2f);
}

void test_sector_economic_elasticity_moves_toward_equilibrium() {
    SectorSimulation sim;
    Sector rich = MakeSector("rich");
    rich.resourceWealth = 1.0f;
    sim.AddSector(rich);
    sim.Tick(1.0f);
    const auto* r = sim.GetSector("rich");
    assert(r->resourceWealth < 1.0f);  // drifted toward 0.5
}
