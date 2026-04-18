// NovaForge Game Initialization
// Entry point for the NovaForge Atlas project.
// Loaded by the Atlas Engine when the novaforge project is activated.

#include "GameInit.h"
#include <iostream>

namespace novaforge {

static bool s_initialized = false;
static const char* s_version = "0.1.0";

bool Initialize(const GameConfig& config) {
    if (s_initialized) {
        return true;
    }

    // Register NovaForge AI signal namespaces
    // These correspond to npc_config.json signalNamespaces:
    //   faction.morale, faction.hostility, sector.tension,
    //   sector.resources, economy.supply, economy.demand,
    //   pirate.threat, fleet.cohesion

    // Register NovaForge server rules
    // These correspond to config/runtime.json serverRules:
    //   miningYieldMultiplier, npcSpawnRate,
    //   marketUpdateInterval, pirateAggressionLevel

    s_initialized = true;
    return true;
}

void Shutdown() {
    if (!s_initialized) {
        return;
    }
    s_initialized = false;
}

const char* GetVersion() {
    return s_version;
}

}
