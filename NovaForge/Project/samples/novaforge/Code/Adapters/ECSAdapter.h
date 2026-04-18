// NovaForge ECS Adapter
// Bridges NovaForge game components and systems to the Atlas Engine ECS.

#pragma once

namespace novaforge {
namespace adapters {

// Registers all NovaForge-specific ECS components with the engine world.
// Components include ship, fleet, market, mining, and faction state.
void RegisterComponents();

// Registers all NovaForge-specific ECS systems with the engine world.
// Systems include combat, movement, AI, economy, and background simulation.
void RegisterSystems();

}
}
