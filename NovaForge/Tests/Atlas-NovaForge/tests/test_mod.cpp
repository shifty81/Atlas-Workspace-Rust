#include "../engine/mod/ModAssetRegistry.h"
#include <iostream>
#include <cassert>

using namespace atlas::mod;

void test_mod_register_graph() {
    AssetRegistry registry;
    assert(registry.GraphCount() == 0);

    registry.RegisterGraph("mods/ships/frigate.assetgraph");
    assert(registry.GraphCount() == 1);
    assert(registry.Graphs()[0] == "mods/ships/frigate.assetgraph");

    registry.RegisterGraph("mods/ships/cruiser.assetgraph");
    assert(registry.GraphCount() == 2);
    std::cout << "[PASS] test_mod_register_graph" << std::endl;
}

void test_mod_register_archetype() {
    AssetRegistry registry;
    assert(registry.ArchetypeCount() == 0);

    registry.RegisterArchetype("mods/archetypes/frigate.json");
    assert(registry.ArchetypeCount() == 1);
    assert(registry.Archetypes()[0] == "mods/archetypes/frigate.json");
    std::cout << "[PASS] test_mod_register_archetype" << std::endl;
}

void test_mod_registry_mixed() {
    AssetRegistry registry;
    registry.RegisterGraph("graph1.assetgraph");
    registry.RegisterArchetype("arch1.json");
    registry.RegisterGraph("graph2.assetgraph");

    assert(registry.GraphCount() == 2);
    assert(registry.ArchetypeCount() == 1);
    std::cout << "[PASS] test_mod_registry_mixed" << std::endl;
}
