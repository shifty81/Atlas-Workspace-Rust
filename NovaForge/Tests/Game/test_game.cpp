#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Game/Game.h"

TEST_CASE("Chunk voxel get/set", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Default is air
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Air);

    chunk.set(5, 5, 5, NF::VoxelType::Stone);
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);
    REQUIRE_FALSE(chunk.meshed);
}

TEST_CASE("Chunk out-of-bounds returns Air", "[Game][Voxel]") {
    NF::Chunk chunk;

    REQUIRE(chunk.get(-1, 0, 0) == NF::VoxelType::Air);
    REQUIRE(chunk.get(0, -1, 0) == NF::VoxelType::Air);
    REQUIRE(chunk.get(0, 0, 16) == NF::VoxelType::Air);
    REQUIRE(chunk.get(16, 0, 0) == NF::VoxelType::Air);
}

TEST_CASE("Chunk set out-of-bounds is safe", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Should not crash
    chunk.set(-1, 0, 0, NF::VoxelType::Stone);
    chunk.set(16, 0, 0, NF::VoxelType::Stone);
}

TEST_CASE("Inventory add and remove", "[Game][Inventory]") {
    NF::Inventory inv;

    inv.add(NF::VoxelType::Stone, 5);
    REQUIRE(inv.count(NF::VoxelType::Stone) == 5);

    REQUIRE(inv.remove(NF::VoxelType::Stone, 3));
    REQUIRE(inv.count(NF::VoxelType::Stone) == 2);

    REQUIRE_FALSE(inv.remove(NF::VoxelType::Stone, 5)); // not enough
    REQUIRE(inv.count(NF::VoxelType::Stone) == 2);      // unchanged
}

TEST_CASE("RigState defaults", "[Game][Rig]") {
    NF::RigState rig;
    REQUIRE(rig.health == 100.f);
    REQUIRE(rig.energy == 100.f);
    REQUIRE(rig.activeTool == 0);
    REQUIRE(rig.maxHealth == 100.f);
    REQUIRE(rig.maxEnergy == 100.f);
    REQUIRE(rig.oxygen == 100.f);
    REQUIRE(rig.stamina == 100.f);
    REQUIRE(rig.isAlive());
}

// ── Chunk dirty flag separation ──────────────────────────────────

TEST_CASE("Chunk separate mesh and collision dirty flags", "[Game][Voxel]") {
    NF::Chunk chunk;

    // Initial state: all dirty
    REQUIRE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);

    // Mark mesh clean, collision should still be dirty
    chunk.markMeshClean();
    REQUIRE_FALSE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);
    REQUIRE(chunk.meshed);

    // Mark collision clean
    chunk.markCollisionClean();
    REQUIRE_FALSE(chunk.collisionDirty);

    // Edit should dirty both
    chunk.set(0, 0, 0, NF::VoxelType::Stone);
    REQUIRE(chunk.meshDirty);
    REQUIRE(chunk.collisionDirty);
    REQUIRE_FALSE(chunk.meshed);

    // markAllClean should clear everything
    chunk.markAllClean();
    REQUIRE_FALSE(chunk.meshDirty);
    REQUIRE_FALSE(chunk.collisionDirty);
    REQUIRE(chunk.meshed);
}

// ── Chunk utility methods ────────────────────────────────────────

TEST_CASE("Chunk isFullyAir and solidCount", "[Game][Voxel]") {
    NF::Chunk chunk;
    REQUIRE(chunk.isFullyAir());
    REQUIRE(chunk.solidCount() == 0);

    chunk.set(0, 0, 0, NF::VoxelType::Stone);
    REQUIRE_FALSE(chunk.isFullyAir());
    REQUIRE(chunk.solidCount() == 1);

    chunk.set(1, 1, 1, NF::VoxelType::Dirt);
    REQUIRE(chunk.solidCount() == 2);
}

// ── Voxel type name round-trip ───────────────────────────────────

TEST_CASE("VoxelType name round-trip", "[Game][Voxel]") {
    // All named types should round-trip through name/fromName
    REQUIRE(NF::voxelTypeFromName("Stone") == NF::VoxelType::Stone);
    REQUIRE(NF::voxelTypeFromName("Dirt") == NF::VoxelType::Dirt);
    REQUIRE(NF::voxelTypeFromName("Grass") == NF::VoxelType::Grass);
    REQUIRE(NF::voxelTypeFromName("Metal") == NF::VoxelType::Metal);
    REQUIRE(NF::voxelTypeFromName("Glass") == NF::VoxelType::Glass);
    REQUIRE(NF::voxelTypeFromName("Water") == NF::VoxelType::Water);
    REQUIRE(NF::voxelTypeFromName("Ore_Iron") == NF::VoxelType::Ore_Iron);
    REQUIRE(NF::voxelTypeFromName("Ore_Gold") == NF::VoxelType::Ore_Gold);
    REQUIRE(NF::voxelTypeFromName("Ore_Crystal") == NF::VoxelType::Ore_Crystal);
    REQUIRE(NF::voxelTypeFromName("Unknown") == NF::VoxelType::Air);

    REQUIRE(std::string(NF::voxelTypeName(NF::VoxelType::Stone)) == "Stone");
    REQUIRE(std::string(NF::voxelTypeName(NF::VoxelType::Ore_Crystal)) == "Ore_Crystal");
}

// ── Chunk serialization: THE persistence proof ───────────────────

TEST_CASE("ChunkSerializer round-trip preserves all voxel data", "[Game][Serialization]") {
    NF::Chunk original;
    original.cx = 3;
    original.cy = -1;
    original.cz = 7;

    // Place diverse voxels
    original.set(0, 0, 0, NF::VoxelType::Stone);
    original.set(5, 5, 5, NF::VoxelType::Dirt);
    original.set(15, 15, 15, NF::VoxelType::Ore_Crystal);
    original.set(8, 0, 8, NF::VoxelType::Water);
    original.set(0, 15, 0, NF::VoxelType::Metal);

    // Serialize to JSON string
    std::string json = NF::ChunkSerializer::serialize(original);
    REQUIRE_FALSE(json.empty());

    // Deserialize from JSON string
    NF::Chunk loaded = NF::ChunkSerializer::deserialize(json);

    // Verify chunk coordinates
    REQUIRE(loaded.cx == original.cx);
    REQUIRE(loaded.cy == original.cy);
    REQUIRE(loaded.cz == original.cz);

    // Verify ALL voxels match — full 16x16x16 comparison
    for (int x = 0; x < NF::CHUNK_SIZE; ++x) {
        for (int y = 0; y < NF::CHUNK_SIZE; ++y) {
            for (int z = 0; z < NF::CHUNK_SIZE; ++z) {
                REQUIRE(loaded.get(x, y, z) == original.get(x, y, z));
            }
        }
    }
}

TEST_CASE("ChunkSerializer round-trip via JsonValue preserves data", "[Game][Serialization]") {
    NF::Chunk original;
    original.cx = 0;
    original.cy = 0;
    original.cz = 0;

    // Fill a layer with stone
    for (int x = 0; x < NF::CHUNK_SIZE; ++x)
        for (int z = 0; z < NF::CHUNK_SIZE; ++z)
            original.set(x, 0, z, NF::VoxelType::Stone);

    auto json = NF::ChunkSerializer::toJson(original);
    NF::Chunk loaded = NF::ChunkSerializer::fromJson(json);

    // Verify the stone layer
    for (int x = 0; x < NF::CHUNK_SIZE; ++x) {
        for (int z = 0; z < NF::CHUNK_SIZE; ++z) {
            REQUIRE(loaded.get(x, 0, z) == NF::VoxelType::Stone);
            REQUIRE(loaded.get(x, 1, z) == NF::VoxelType::Air);
        }
    }
}

TEST_CASE("ChunkSerializer empty chunk round-trip", "[Game][Serialization]") {
    NF::Chunk original;
    // Default chunk is all air
    std::string json = NF::ChunkSerializer::serialize(original);
    NF::Chunk loaded = NF::ChunkSerializer::deserialize(json);

    REQUIRE(loaded.isFullyAir());
    REQUIRE(loaded.cx == 0);
    REQUIRE(loaded.cy == 0);
    REQUIRE(loaded.cz == 0);
}

TEST_CASE("Voxel edit then save/reload round-trip", "[Game][Serialization][Persistence]") {
    // This is the critical editor-trust test: edit a voxel, save, reload, verify
    NF::Chunk chunk;
    chunk.cx = 1;
    chunk.cy = 2;
    chunk.cz = 3;

    // Step 1: Edit voxels
    chunk.set(7, 7, 7, NF::VoxelType::Ore_Gold);
    chunk.set(3, 12, 5, NF::VoxelType::Glass);
    REQUIRE(chunk.get(7, 7, 7) == NF::VoxelType::Ore_Gold);

    // Step 2: Save (serialize)
    std::string saved = NF::ChunkSerializer::serialize(chunk);

    // Step 3: Simulate process restart by deserializing into new chunk
    NF::Chunk reloaded = NF::ChunkSerializer::deserialize(saved);

    // Step 4: Verify edit persisted
    REQUIRE(reloaded.get(7, 7, 7) == NF::VoxelType::Ore_Gold);
    REQUIRE(reloaded.get(3, 12, 5) == NF::VoxelType::Glass);
    REQUIRE(reloaded.cx == 1);
    REQUIRE(reloaded.cy == 2);
    REQUIRE(reloaded.cz == 3);

    // Step 5: Edit again, save again, reload again (second round-trip)
    reloaded.set(7, 7, 7, NF::VoxelType::Air);  // undo the edit
    std::string saved2 = NF::ChunkSerializer::serialize(reloaded);
    NF::Chunk reloaded2 = NF::ChunkSerializer::deserialize(saved2);

    REQUIRE(reloaded2.get(7, 7, 7) == NF::VoxelType::Air);
    REQUIRE(reloaded2.get(3, 12, 5) == NF::VoxelType::Glass);
}

// ── VoxelEditCommand with undo/redo ──────────────────────────────

TEST_CASE("VoxelEditCommand undo/redo", "[Game][Undo]") {
    NF::Chunk chunk;
    NF::CommandStack stack;

    // Edit via command
    auto cmd = std::make_unique<NF::VoxelEditCommand>(&chunk, 5, 5, 5, NF::VoxelType::Stone);
    stack.execute(std::move(cmd));

    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(stack.undoCount() == 1);

    // Undo
    stack.undo();
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Air);
    REQUIRE(stack.undoCount() == 0);
    REQUIRE(stack.redoCount() == 1);

    // Redo
    stack.redo();
    REQUIRE(chunk.get(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(stack.undoCount() == 1);
    REQUIRE(stack.redoCount() == 0);
}

TEST_CASE("VoxelEditCommand multiple edits undo chain", "[Game][Undo]") {
    NF::Chunk chunk;
    NF::CommandStack stack;

    // Three sequential edits
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 0, 0, 0, NF::VoxelType::Stone));
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 1, 1, 1, NF::VoxelType::Dirt));
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 2, 2, 2, NF::VoxelType::Grass));

    REQUIRE(stack.undoCount() == 3);

    // Undo all three
    stack.undo();
    REQUIRE(chunk.get(2, 2, 2) == NF::VoxelType::Air);
    stack.undo();
    REQUIRE(chunk.get(1, 1, 1) == NF::VoxelType::Air);
    stack.undo();
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Air);

    REQUIRE(stack.undoCount() == 0);
    REQUIRE(stack.redoCount() == 3);

    // Redo first, then new edit should clear redo stack
    stack.redo();
    REQUIRE(chunk.get(0, 0, 0) == NF::VoxelType::Stone);
    stack.execute(std::make_unique<NF::VoxelEditCommand>(&chunk, 5, 5, 5, NF::VoxelType::Metal));
    REQUIRE(stack.redoCount() == 0);  // redo stack cleared by new edit
    REQUIRE(stack.undoCount() == 2);
}

// ── ChunkMesher tests ───────────────────────────────────────────

TEST_CASE("ChunkMesher empty chunk produces no geometry", "[Game][Mesher]") {
    NF::Chunk chunk;
    // All air - should produce nothing
    auto result = NF::ChunkMesher::buildMesh(chunk);
    REQUIRE(result.vertices.empty());
    REQUIRE(result.indices.empty());
    REQUIRE(result.faceCount == 0);
}

TEST_CASE("ChunkMesher single voxel produces 6 faces", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(8, 8, 8, NF::VoxelType::Stone);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    // A lone voxel surrounded by air should have 6 exposed faces
    REQUIRE(result.faceCount == 6);
    REQUIRE(result.vertices.size() == 24);   // 4 verts per face * 6 faces
    REQUIRE(result.indices.size() == 36);    // 6 indices per face * 6 faces
}

TEST_CASE("ChunkMesher adjacent voxels cull shared face", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(8, 8, 8, NF::VoxelType::Stone);
    chunk.set(9, 8, 8, NF::VoxelType::Stone);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    // Two adjacent voxels: 6+6 faces minus 2 shared faces = 10 exposed
    REQUIRE(result.faceCount == 10);
}

TEST_CASE("ChunkMesher voxel colors match type", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(0, 0, 0, NF::VoxelType::Grass);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    REQUIRE_FALSE(result.vertices.empty());

    NF::Vec4 grassColor = NF::voxelColor(NF::VoxelType::Grass);
    // All vertices should have the grass color
    for (const auto& v : result.vertices) {
        REQUIRE(v.color.x == grassColor.x);
        REQUIRE(v.color.y == grassColor.y);
        REQUIRE(v.color.z == grassColor.z);
    }
}

TEST_CASE("ChunkMesher buildRenderMesh returns valid Mesh", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.set(4, 4, 4, NF::VoxelType::Metal);

    NF::Mesh mesh = NF::ChunkMesher::buildRenderMesh(chunk);
    REQUIRE(mesh.vertexCount() == 24);
    REQUIRE(mesh.indexCount() == 36);
    REQUIRE(mesh.triangleCount() == 12);
}

TEST_CASE("ChunkMesher world offset is applied", "[Game][Mesher]") {
    NF::Chunk chunk;
    chunk.cx = 2; chunk.cy = 0; chunk.cz = 1;
    chunk.set(0, 0, 0, NF::VoxelType::Stone);

    auto result = NF::ChunkMesher::buildMesh(chunk);
    REQUIRE_FALSE(result.vertices.empty());

    // Vertices should be offset by chunk position * CHUNK_SIZE
    float expectedMinX = 2.f * NF::CHUNK_SIZE;
    float expectedMinZ = 1.f * NF::CHUNK_SIZE;
    bool foundOffsetVert = false;
    for (const auto& v : result.vertices) {
        if (v.position.x >= expectedMinX && v.position.z >= expectedMinZ) {
            foundOffsetVert = true;
            break;
        }
    }
    REQUIRE(foundOffsetVert);
}

// ── WorldState tests ─────────────────────────────────────────────

TEST_CASE("WorldState get/set world coordinates", "[Game][World]") {
    NF::WorldState world;

    world.setWorld(5, 5, 5, NF::VoxelType::Stone);
    REQUIRE(world.getWorld(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(world.getWorld(0, 0, 0) == NF::VoxelType::Air);
    REQUIRE(world.chunkCount() == 1);
}

TEST_CASE("WorldState negative coordinates", "[Game][World]") {
    NF::WorldState world;

    world.setWorld(-1, -1, -1, NF::VoxelType::Dirt);
    REQUIRE(world.getWorld(-1, -1, -1) == NF::VoxelType::Dirt);
    REQUIRE(world.chunkCount() == 1);
}

TEST_CASE("WorldState cross-chunk boundaries", "[Game][World]") {
    NF::WorldState world;

    // Set voxels in different chunks
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    world.setWorld(16, 0, 0, NF::VoxelType::Dirt);   // next chunk
    world.setWorld(-1, 0, 0, NF::VoxelType::Grass);  // previous chunk

    REQUIRE(world.getWorld(0, 0, 0) == NF::VoxelType::Stone);
    REQUIRE(world.getWorld(16, 0, 0) == NF::VoxelType::Dirt);
    REQUIRE(world.getWorld(-1, 0, 0) == NF::VoxelType::Grass);
    REQUIRE(world.chunkCount() == 3);
}

TEST_CASE("WorldState forEach iterates all chunks", "[Game][World]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    world.setWorld(16, 0, 0, NF::VoxelType::Dirt);

    int count = 0;
    world.forEach([&](const NF::Chunk&) { ++count; });
    REQUIRE(count == 2);
}

// ── WorldSerializer tests ────────────────────────────────────────

TEST_CASE("WorldSerializer round-trip preserves world", "[Game][Serialization]") {
    NF::WorldState original;
    original.setWorld(5, 5, 5, NF::VoxelType::Stone);
    original.setWorld(20, 0, 0, NF::VoxelType::Ore_Gold);
    original.setWorld(-5, 10, -5, NF::VoxelType::Glass);

    std::string json = NF::WorldSerializer::serialize(original);
    REQUIRE_FALSE(json.empty());

    NF::WorldState loaded = NF::WorldSerializer::deserialize(json);

    REQUIRE(loaded.chunkCount() == original.chunkCount());
    REQUIRE(loaded.getWorld(5, 5, 5) == NF::VoxelType::Stone);
    REQUIRE(loaded.getWorld(20, 0, 0) == NF::VoxelType::Ore_Gold);
    REQUIRE(loaded.getWorld(-5, 10, -5) == NF::VoxelType::Glass);
    REQUIRE(loaded.getWorld(0, 0, 0) == NF::VoxelType::Air);
}

TEST_CASE("WorldSerializer empty world round-trip", "[Game][Serialization]") {
    NF::WorldState original;
    std::string json = NF::WorldSerializer::serialize(original);
    NF::WorldState loaded = NF::WorldSerializer::deserialize(json);
    REQUIRE(loaded.chunkCount() == 0);
}

// ── VoxelPickService tests ───────────────────────────────────────

TEST_CASE("VoxelPickService hits solid voxel", "[Game][Pick]") {
    NF::WorldState world;
    // Place a stone block at (5, 5, 5)
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    // Cast from above, looking down
    NF::Vec3 origin{5.5f, 20.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto hit = NF::VoxelPickService::raycast(world, origin, dir);
    REQUIRE(hit.has_value());
    REQUIRE(hit->wx == 5);
    REQUIRE(hit->wy == 5);
    REQUIRE(hit->wz == 5);
    REQUIRE(hit->type == NF::VoxelType::Stone);
    REQUIRE(hit->face == 2); // hit the +Y face
}

TEST_CASE("VoxelPickService misses empty world", "[Game][Pick]") {
    NF::WorldState world;

    NF::Vec3 origin{0.f, 10.f, 0.f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto hit = NF::VoxelPickService::raycast(world, origin, dir, 50.f);
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("VoxelPickService adjacentVoxel calculation", "[Game][Pick]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::Vec3 origin{5.5f, 20.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto hit = NF::VoxelPickService::raycast(world, origin, dir);
    REQUIRE(hit.has_value());

    auto adj = hit->adjacentVoxel();
    // Hit +Y face of (5,5,5), adjacent should be (5,6,5)
    REQUIRE(adj.x == 5);
    REQUIRE(adj.y == 6);
    REQUIRE(adj.z == 5);
}

TEST_CASE("VoxelPickService respects maxDist", "[Game][Pick]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::Vec3 origin{5.5f, 100.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    // maxDist too short to reach
    auto hit = NF::VoxelPickService::raycast(world, origin, dir, 10.f);
    REQUIRE_FALSE(hit.has_value());

    // maxDist long enough
    auto hit2 = NF::VoxelPickService::raycast(world, origin, dir, 200.f);
    REQUIRE(hit2.has_value());
}

// ── G1: RigState expanded tests ─────────────────────────────────

TEST_CASE("RigState tick regens energy and stamina", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.energy = 50.f;
    rig.stamina = 50.f;
    rig.tick(1.f);
    REQUIRE(rig.energy == Catch::Approx(52.f));
    REQUIRE(rig.stamina == Catch::Approx(55.f));
}

TEST_CASE("RigState tick drains oxygen", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.tick(10.f);
    REQUIRE(rig.oxygen == Catch::Approx(90.f));
}

TEST_CASE("RigState oxygen depletion damages health", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.oxygen = 0.f;
    rig.tick(1.f);
    REQUIRE(rig.health < 100.f);
    REQUIRE(rig.isAlive());
}

TEST_CASE("RigState takeDamage and heal", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.takeDamage(30.f);
    REQUIRE(rig.health == Catch::Approx(70.f));
    rig.heal(50.f);
    REQUIRE(rig.health == Catch::Approx(100.f)); // clamped to max
}

TEST_CASE("RigState death stops ticking", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.takeDamage(100.f);
    REQUIRE_FALSE(rig.isAlive());
    float prevEnergy = rig.energy;
    rig.tick(1.f);
    REQUIRE(rig.energy == prevEnergy); // no regen when dead
}

TEST_CASE("RigState consumeEnergy and consumeStamina", "[Game][Rig][G1]") {
    NF::RigState rig;
    rig.consumeEnergy(60.f);
    REQUIRE(rig.energy == Catch::Approx(40.f));
    rig.consumeStamina(80.f);
    REQUIRE(rig.stamina == Catch::Approx(20.f));
    rig.consumeEnergy(999.f);
    REQUIRE(rig.energy == Catch::Approx(0.f)); // clamped at 0
}

// ── G1: ResourceType tests ──────────────────────────────────────

TEST_CASE("ResourceType name round-trip", "[Game][Resource][G1]") {
    REQUIRE(NF::resourceTypeFromName("RawStone") == NF::ResourceType::RawStone);
    REQUIRE(NF::resourceTypeFromName("RawIron") == NF::ResourceType::RawIron);
    REQUIRE(NF::resourceTypeFromName("RefinedGold") == NF::ResourceType::RefinedGold);
    REQUIRE(NF::resourceTypeFromName("SteelPlate") == NF::ResourceType::SteelPlate);
    REQUIRE(NF::resourceTypeFromName("EnergyCell") == NF::ResourceType::EnergyCell);
    REQUIRE(std::string(NF::resourceTypeName(NF::ResourceType::CircuitBoard)) == "CircuitBoard");
    REQUIRE(std::string(NF::resourceTypeName(NF::ResourceType::RawCrystal)) == "RawCrystal");
}

// ── G1: Resource Drop Table tests ───────────────────────────────

TEST_CASE("ResourceDrop table returns correct drops", "[Game][Resource][G1]") {
    auto stoneDrops = NF::getResourceDrops(NF::VoxelType::Stone);
    REQUIRE(stoneDrops.size() == 1);
    REQUIRE(stoneDrops[0].resource == NF::ResourceType::RawStone);

    auto ironDrops = NF::getResourceDrops(NF::VoxelType::Ore_Iron);
    REQUIRE(ironDrops.size() == 1);
    REQUIRE(ironDrops[0].minAmount == 1);
    REQUIRE(ironDrops[0].maxAmount == 2);

    auto crystalDrops = NF::getResourceDrops(NF::VoxelType::Ore_Crystal);
    REQUIRE(crystalDrops[0].maxAmount == 3);

    auto airDrops = NF::getResourceDrops(NF::VoxelType::Air);
    REQUIRE(airDrops.empty());

    auto glassDrops = NF::getResourceDrops(NF::VoxelType::Glass);
    REQUIRE(glassDrops.empty());
}

// ── G1: ResourceInventory tests ─────────────────────────────────

TEST_CASE("ResourceInventory add and remove", "[Game][Resource][G1]") {
    NF::ResourceInventory inv;
    REQUIRE(inv.isEmpty());
    REQUIRE(inv.totalItems() == 0);

    inv.add(NF::ResourceType::RawIron, 5);
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 5);
    REQUIRE(inv.totalItems() == 5);
    REQUIRE_FALSE(inv.isEmpty());

    REQUIRE(inv.remove(NF::ResourceType::RawIron, 3));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 2);

    REQUIRE_FALSE(inv.remove(NF::ResourceType::RawIron, 5));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 2);
}

TEST_CASE("ResourceInventory totalItems across types", "[Game][Resource][G1]") {
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawStone, 3);
    inv.add(NF::ResourceType::RefinedIron, 2);
    inv.add(NF::ResourceType::EnergyCell, 1);
    REQUIRE(inv.totalItems() == 6);
}

// ── G1: ToolState tests ─────────────────────────────────────────

TEST_CASE("ToolState isReady and use", "[Game][Tool][G1]") {
    NF::ToolState tool;
    tool.cooldownRate = 0.5f;
    REQUIRE(tool.isReady());

    tool.use();
    REQUIRE_FALSE(tool.isReady());
    REQUIRE(tool.cooldown == Catch::Approx(0.5f));
    REQUIRE(tool.durability == Catch::Approx(99.f));

    tool.tick(0.5f);
    REQUIRE(tool.isReady());
}

TEST_CASE("ToolState zero durability not ready", "[Game][Tool][G1]") {
    NF::ToolState tool;
    tool.durability = 0.f;
    REQUIRE_FALSE(tool.isReady());
}

TEST_CASE("ToolType name", "[Game][Tool][G1]") {
    REQUIRE(std::string(NF::toolTypeName(NF::ToolType::MiningLaser)) == "MiningLaser");
    REQUIRE(std::string(NF::toolTypeName(NF::ToolType::Scanner)) == "Scanner");
}

// ── G1: ToolBelt tests ──────────────────────────────────────────

TEST_CASE("ToolBelt init and slot access", "[Game][Tool][G1]") {
    NF::ToolBelt belt;
    belt.init();

    REQUIRE(belt.slotCount() == 4);
    REQUIRE(belt.activeSlot() == 0);
    REQUIRE(belt.activeTool().type == NF::ToolType::MiningLaser);
    REQUIRE(belt.slot(1).type == NF::ToolType::PlacementTool);
    REQUIRE(belt.slot(2).type == NF::ToolType::RepairTool);
    REQUIRE(belt.slot(3).type == NF::ToolType::Scanner);
}

TEST_CASE("ToolBelt selectSlot clamps", "[Game][Tool][G1]") {
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(-1);
    REQUIRE(belt.activeSlot() == 0);
    belt.selectSlot(99);
    REQUIRE(belt.activeSlot() == 3);
    belt.selectSlot(2);
    REQUIRE(belt.activeSlot() == 2);
}

TEST_CASE("ToolBelt nextTool and prevTool cycle", "[Game][Tool][G1]") {
    NF::ToolBelt belt;
    belt.init();
    belt.nextTool();
    REQUIRE(belt.activeSlot() == 1);
    belt.nextTool();
    belt.nextTool();
    belt.nextTool();
    REQUIRE(belt.activeSlot() == 0); // wrapped around

    belt.prevTool();
    REQUIRE(belt.activeSlot() == 3); // wrapped backward
}

// ── G1: HUDState tests ──────────────────────────────────────────

TEST_CASE("HUDState defaults", "[Game][HUD][G1]") {
    NF::HUDState hud;
    REQUIRE(hud.showCrosshair);
    REQUIRE_FALSE(hud.targetLocked);
    REQUIRE(hud.targetVoxel == NF::VoxelType::Air);
    REQUIRE(hud.notifications.empty());
}

TEST_CASE("HUDState add and expire notifications", "[Game][HUD][G1]") {
    NF::HUDState hud;
    hud.addNotification("Mined stone!", 2.f);
    hud.addNotification("Low energy", 5.f);
    REQUIRE(hud.notifications.size() == 2);

    hud.tick(3.f);
    REQUIRE(hud.notifications.size() == 1);
    REQUIRE(hud.notifications[0].message == "Low energy");

    hud.tick(3.f);
    REQUIRE(hud.notifications.empty());
}

TEST_CASE("HUDState clearNotifications", "[Game][HUD][G1]") {
    NF::HUDState hud;
    hud.addNotification("test1");
    hud.addNotification("test2");
    hud.clearNotifications();
    REQUIRE(hud.notifications.empty());
}

// ── G1: InteractionSystem mining tests ──────────────────────────

TEST_CASE("InteractionSystem tryMine success", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    NF::Vec3 origin{5.5f, 10.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv, origin, dir);
    REQUIRE(result.success);
    REQUIRE(result.minedType == NF::VoxelType::Stone);
    REQUIRE(world.getWorld(5, 5, 5) == NF::VoxelType::Air);
    REQUIRE(voxelInv.count(NF::VoxelType::Stone) == 1);
    REQUIRE(resInv.count(NF::ResourceType::RawStone) == 1);
    REQUIRE(rig.energy < 100.f);
}

TEST_CASE("InteractionSystem tryMine fails wrong tool", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(1); // placement tool
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv,
                                      {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE_FALSE(result.success);
}

TEST_CASE("InteractionSystem tryMine fails no energy", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    rig.energy = 0.f;
    NF::ToolBelt belt;
    belt.init();
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv,
                                      {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE_FALSE(result.success);
}

TEST_CASE("InteractionSystem tryMine ore drops resources", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Ore_Iron);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    NF::Inventory voxelInv;
    NF::ResourceInventory resInv;

    NF::InteractionSystem interaction;
    auto result = interaction.tryMine(world, rig, belt, voxelInv, resInv,
                                      {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE(result.success);
    REQUIRE_FALSE(result.drops.empty());
    REQUIRE(resInv.count(NF::ResourceType::RawIron) >= 1);
}

// ── G1: InteractionSystem placement tests ───────────────────────

TEST_CASE("InteractionSystem tryPlace success", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(1); // placement tool
    NF::Inventory voxelInv;
    voxelInv.add(NF::VoxelType::Dirt, 5);

    NF::InteractionSystem interaction;
    NF::Vec3 origin{5.5f, 10.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto result = interaction.tryPlace(world, rig, belt, voxelInv, origin, dir, NF::VoxelType::Dirt);
    REQUIRE(result.success);
    REQUIRE(result.placedType == NF::VoxelType::Dirt);
    REQUIRE(voxelInv.count(NF::VoxelType::Dirt) == 4);
    // Placed adjacent to stone (on top), at (5,6,5)
    REQUIRE(world.getWorld(5, 6, 5) == NF::VoxelType::Dirt);
}

TEST_CASE("InteractionSystem tryPlace fails no inventory", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::RigState rig;
    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(1);
    NF::Inventory voxelInv; // empty

    NF::InteractionSystem interaction;
    auto result = interaction.tryPlace(world, rig, belt, voxelInv,
                                       {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f},
                                       NF::VoxelType::Dirt);
    REQUIRE_FALSE(result.success);
}

// ── G1: InteractionSystem scan tests ────────────────────────────

TEST_CASE("InteractionSystem tryScan returns voxel info", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Ore_Gold);

    NF::ToolBelt belt;
    belt.init();
    belt.selectSlot(3); // scanner

    NF::InteractionSystem interaction;
    auto hit = interaction.tryScan(world, belt, {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE(hit.has_value());
    REQUIRE(hit->type == NF::VoxelType::Ore_Gold);
}

TEST_CASE("InteractionSystem tryScan fails wrong tool", "[Game][Interaction][G1]") {
    NF::WorldState world;
    world.setWorld(5, 5, 5, NF::VoxelType::Stone);

    NF::ToolBelt belt;
    belt.init(); // active is mining laser

    NF::InteractionSystem interaction;
    auto hit = interaction.tryScan(world, belt, {5.5f, 10.f, 5.5f}, {0.f, -1.f, 0.f});
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("InteractionSystem maxReach config", "[Game][Interaction][G1]") {
    NF::InteractionSystem interaction;
    REQUIRE(interaction.maxReach() == Catch::Approx(8.f));
    interaction.setMaxReach(12.f);
    REQUIRE(interaction.maxReach() == Catch::Approx(12.f));
}

// ── G1: GameSession tests ───────────────────────────────────────

TEST_CASE("GameSession init and shutdown", "[Game][Session][G1]") {
    NF::GameSession session;
    REQUIRE_FALSE(session.isActive());

    session.init();
    REQUIRE(session.isActive());
    REQUIRE(session.rig().health == 100.f);
    REQUIRE(session.toolBelt().slotCount() == 4);
    REQUIRE(session.voxelInventory().count(NF::VoxelType::Stone) == 0);
    REQUIRE(session.resourceInventory().isEmpty());

    session.shutdown();
    REQUIRE_FALSE(session.isActive());
}

TEST_CASE("GameSession tick updates rig and tools", "[Game][Session][G1]") {
    NF::GameSession session;
    session.init();

    session.rig().energy = 50.f;
    session.toolBelt().activeTool().use(); // put on cooldown
    float cd = session.toolBelt().activeTool().cooldown;
    REQUIRE(cd > 0.f);

    session.tick(1.f);
    REQUIRE(session.rig().energy > 50.f); // regen
    REQUIRE(session.toolBelt().activeTool().cooldown < cd); // reduced
}

TEST_CASE("GameSession tick expires HUD notifications", "[Game][Session][G1]") {
    NF::GameSession session;
    session.init();
    session.hud().addNotification("test", 1.f);
    REQUIRE(session.hud().notifications.size() == 1);

    session.tick(2.f);
    REQUIRE(session.hud().notifications.empty());
}

TEST_CASE("GameSession full mine interaction", "[Game][Session][G1]") {
    NF::GameSession session;
    session.init();

    session.world().setWorld(5, 5, 5, NF::VoxelType::Ore_Crystal);

    NF::Vec3 origin{5.5f, 10.f, 5.5f};
    NF::Vec3 dir{0.f, -1.f, 0.f};

    auto result = session.interaction().tryMine(
        session.world(), session.rig(), session.toolBelt(),
        session.voxelInventory(), session.resourceInventory(),
        origin, dir);

    REQUIRE(result.success);
    REQUIRE(session.voxelInventory().count(NF::VoxelType::Ore_Crystal) == 1);
    REQUIRE(session.resourceInventory().count(NF::ResourceType::RawCrystal) >= 1);
    REQUIRE(session.world().getWorld(5, 5, 5) == NF::VoxelType::Air);
}

// ── G2: ChunkRenderData tests ───────────────────────────────────

TEST_CASE("ChunkRenderData defaults", "[Game][G2]") {
    NF::ChunkRenderData data;
    REQUIRE_FALSE(data.valid);
    REQUIRE(data.version == 0);
    REQUIRE(data.mesh.vertexCount() == 0);
}

// ── G2: ChunkRenderCache tests ──────────────────────────────────

TEST_CASE("ChunkRenderCache update and get", "[Game][RenderCache][G2]") {
    NF::Chunk chunk;
    chunk.cx = 0; chunk.cy = 0; chunk.cz = 0;
    chunk.set(5, 5, 5, NF::VoxelType::Stone);

    NF::ChunkRenderCache cache;
    REQUIRE(cache.cacheSize() == 0);

    cache.update(chunk);
    REQUIRE(cache.cacheSize() == 1);

    auto* data = cache.get({0, 0, 0});
    REQUIRE(data != nullptr);
    REQUIRE(data->valid);
    REQUIRE(data->version == 1);
    REQUIRE(data->mesh.vertexCount() > 0);

    // Update again increments version
    cache.update(chunk);
    data = cache.get({0, 0, 0});
    REQUIRE(data->version == 2);
}

TEST_CASE("ChunkRenderCache remove and clear", "[Game][RenderCache][G2]") {
    NF::Chunk chunk;
    chunk.set(0, 0, 0, NF::VoxelType::Dirt);
    chunk.cx = 1; chunk.cy = 2; chunk.cz = 3;

    NF::ChunkRenderCache cache;
    cache.update(chunk);
    REQUIRE(cache.cacheSize() == 1);

    cache.remove({1, 2, 3});
    REQUIRE(cache.cacheSize() == 0);
    REQUIRE(cache.get({1, 2, 3}) == nullptr);

    cache.update(chunk);
    cache.clear();
    REQUIRE(cache.cacheSize() == 0);
}

TEST_CASE("ChunkRenderCache updateDirty", "[Game][RenderCache][G2]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    world.setWorld(16, 0, 0, NF::VoxelType::Dirt);

    NF::ChunkRenderCache cache;
    int rebuilt = cache.updateDirty(world);
    REQUIRE(rebuilt == 2);
    REQUIRE(cache.cacheSize() == 2);

    // All chunks now clean, no more rebuilds
    int rebuilt2 = cache.updateDirty(world);
    REQUIRE(rebuilt2 == 0);

    // Dirty one chunk
    world.setWorld(0, 0, 0, NF::VoxelType::Grass);
    int rebuilt3 = cache.updateDirty(world);
    REQUIRE(rebuilt3 == 1);
}

// ── G2: ChunkRenderer tests ────────────────────────────────────

TEST_CASE("ChunkRenderer init/shutdown", "[Game][ChunkRenderer][G2]") {
    NF::ChunkRenderer renderer;
    renderer.init();
    REQUIRE(renderer.voxelMaterial().name() == NF::StringID("voxel_material"));
    renderer.shutdown();
}

TEST_CASE("ChunkRenderer render submits commands", "[Game][ChunkRenderer][G2]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);

    NF::ChunkRenderCache cache;
    cache.updateDirty(world);

    NF::Camera cam;
    cam.position = {8, 8, 50};
    cam.target = {8, 8, 0};

    NF::RenderQueue queue;
    NF::ChunkRenderer renderer;
    renderer.init();

    int visible = renderer.render(world, cache, queue, cam, 1.f);
    REQUIRE(visible >= 1);
    REQUIRE(queue.size() >= 1);
}

TEST_CASE("ChunkRenderer countVisible", "[Game][ChunkRenderer][G2]") {
    NF::WorldState world;
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);

    NF::Camera cam;
    cam.position = {8, 8, 50};
    cam.target = {8, 8, 0};

    NF::ChunkRenderer renderer;
    renderer.init();

    int visible = renderer.countVisible(world, cam, 1.f);
    REQUIRE(visible >= 1);
}

TEST_CASE("ChunkRenderer frustum culling", "[Game][ChunkRenderer][G2]") {
    NF::WorldState world;
    // Chunk at origin
    world.setWorld(0, 0, 0, NF::VoxelType::Stone);
    // Chunk far behind camera
    world.setWorld(5000, 5000, 5000, NF::VoxelType::Dirt);

    NF::Camera cam;
    cam.position = {8, 8, 50};
    cam.target = {8, 8, 0};
    cam.farPlane = 200.f;

    NF::ChunkRenderer renderer;
    renderer.init();

    // Should see chunk at origin but not the one at 5000,5000,5000
    int visible = renderer.countVisible(world, cam, 1.f);
    REQUIRE(visible < static_cast<int>(world.chunkCount()));
}

// ── G3: FPSCamera tests ─────────────────────────────────────────

TEST_CASE("FPSCamera init and defaults", "[Game][FPSCamera][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 5, 0});
    REQUIRE(cam.position().x == Catch::Approx(0.f));
    REQUIRE(cam.position().y == Catch::Approx(5.f));
    REQUIRE(cam.yaw() == Catch::Approx(-90.f));
    REQUIRE(cam.pitch() == Catch::Approx(0.f));
    REQUIRE(cam.sensitivity() == Catch::Approx(0.1f));
}

TEST_CASE("FPSCamera mouse look yaw", "[Game][FPSCamera][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0});
    float initialYaw = cam.yaw();
    cam.processMouseLook(100.f, 0.f); // move mouse right
    REQUIRE(cam.yaw() > initialYaw);
    REQUIRE(cam.pitch() == Catch::Approx(0.f));
}

TEST_CASE("FPSCamera mouse look pitch clamping", "[Game][FPSCamera][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0});
    cam.processMouseLook(0.f, 10000.f); // extreme up
    REQUIRE(cam.pitch() <= 89.f);
    cam.processMouseLook(0.f, -20000.f); // extreme down
    REQUIRE(cam.pitch() >= -89.f);
}

TEST_CASE("FPSCamera forward vector calculation", "[Game][FPSCamera][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0}, -90.f, 0.f); // looking along -Z
    NF::Vec3 fwd = cam.forward();
    REQUIRE(fwd.x == Catch::Approx(0.f).margin(1e-5));
    REQUIRE(fwd.y == Catch::Approx(0.f).margin(1e-5));
    REQUIRE(fwd.z == Catch::Approx(-1.f).margin(1e-5));
}

TEST_CASE("FPSCamera toCamera conversion", "[Game][FPSCamera][G3]") {
    NF::FPSCamera fpsCam;
    fpsCam.init({1, 2, 3});
    NF::Camera cam = fpsCam.toCamera(90.f, 0.5f, 500.f);
    REQUIRE(cam.position.x == Catch::Approx(1.f));
    REQUIRE(cam.position.y == Catch::Approx(2.f));
    REQUIRE(cam.position.z == Catch::Approx(3.f));
    REQUIRE(cam.fov == Catch::Approx(90.f));
    REQUIRE(cam.nearPlane == Catch::Approx(0.5f));
    REQUIRE(cam.farPlane == Catch::Approx(500.f));
    // target should be position + forward
    NF::Vec3 diff = cam.target - cam.position;
    float len = diff.length();
    REQUIRE(len == Catch::Approx(1.f).margin(1e-4));
}

TEST_CASE("FPSCamera sensitivity", "[Game][FPSCamera][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0});
    cam.setSensitivity(0.5f);
    REQUIRE(cam.sensitivity() == Catch::Approx(0.5f));
    float yawBefore = cam.yaw();
    cam.processMouseLook(10.f, 0.f);
    REQUIRE(cam.yaw() == Catch::Approx(yawBefore + 10.f * 0.5f));
}

// ── G3: PlayerMovement tests ────────────────────────────────────

TEST_CASE("PlayerMovement init and defaults", "[Game][PlayerMovement][G3]") {
    NF::PlayerMovement pm;
    pm.init({1, 2, 3});
    REQUIRE(pm.position().x == Catch::Approx(1.f));
    REQUIRE(pm.position().y == Catch::Approx(2.f));
    REQUIRE(pm.walkSpeed() == Catch::Approx(5.f));
    REQUIRE(pm.sprintSpeed() == Catch::Approx(8.f));
    REQUIRE(pm.crouchSpeed() == Catch::Approx(2.5f));
    REQUIRE(pm.jumpForce() == Catch::Approx(7.f));
    REQUIRE(pm.gravity() == Catch::Approx(-20.f));
}

TEST_CASE("PlayerMovement forward movement", "[Game][PlayerMovement][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0}, -90.f, 0.f); // looking along -Z
    NF::PlayerMovement pm;
    pm.init({0, 0, 0});

    NF::MovementInput input;
    input.forward = true;

    // Start grounded
    pm.setGravity(0.f);
    pm.update(1.f, input, cam);

    // Should have moved in -Z direction
    REQUIRE(pm.position().z < 0.f);
}

TEST_CASE("PlayerMovement sprint speed", "[Game][PlayerMovement][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0}, -90.f, 0.f);
    NF::PlayerMovement pm;
    pm.init({0, 0, 0});
    pm.setGravity(0.f);

    NF::MovementInput input;
    input.forward = true;
    input.sprint = true;

    pm.update(1.f, input, cam);
    REQUIRE(pm.isSprinting());
    REQUIRE(pm.currentSpeed() == Catch::Approx(8.f).margin(0.1f));
}

TEST_CASE("PlayerMovement gravity and falling", "[Game][PlayerMovement][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 10, 0});
    NF::PlayerMovement pm;
    pm.init({0, 10, 0});

    NF::MovementInput input; // no input, just gravity
    pm.update(0.1f, input, cam);

    REQUIRE(pm.position().y < 10.f);
    REQUIRE(pm.velocity().y < 0.f);
}

TEST_CASE("PlayerMovement jump", "[Game][PlayerMovement][G3]") {
    NF::FPSCamera cam;
    cam.init({0, 0, 0});
    NF::PlayerMovement pm;
    pm.init({0, 0, 0});

    // First land on ground
    NF::MovementInput input;
    pm.update(0.1f, input, cam); // gravity pulls to y<0, snaps to 0, grounded=true
    REQUIRE(pm.isGrounded());

    // Now jump
    input.jump = true;
    pm.update(0.016f, input, cam);
    REQUIRE(pm.velocity().y > 0.f);
}

// ── G3: VoxelCollider tests ─────────────────────────────────────

TEST_CASE("VoxelCollider wouldCollide empty world", "[Game][VoxelCollider][G3]") {
    NF::WorldState world;
    NF::VoxelCollider collider;
    REQUIRE_FALSE(collider.wouldCollide(world, {5.f, 5.f, 5.f}));
}

TEST_CASE("VoxelCollider isOnGround", "[Game][VoxelCollider][G3]") {
    NF::WorldState world;
    // Place a solid voxel at y=0
    world.setWorld(5, 0, 5, NF::VoxelType::Stone);
    NF::VoxelCollider collider;
    // Player standing at y=1 (feet at y=1, voxel at y=0 is just below)
    REQUIRE(collider.isOnGround(world, {5.5f, 1.f, 5.5f}));
    // Player far above
    REQUIRE_FALSE(collider.isOnGround(world, {5.5f, 10.f, 5.5f}));
}

TEST_CASE("VoxelCollider resolveCollision", "[Game][VoxelCollider][G3]") {
    NF::WorldState world;
    // Wall of stone at x=3
    for (int y = 0; y < 3; ++y)
        for (int z = 0; z < 3; ++z)
            world.setWorld(3, y, z, NF::VoxelType::Stone);

    NF::VoxelCollider collider;
    NF::Vec3 pos{2.f, 0.5f, 1.f};
    NF::Vec3 vel{10.f, 0.f, 0.f}; // moving into the wall

    NF::Vec3 resolved = collider.resolveCollision(world, pos, vel, 0.1f);
    // X should be blocked (reverted), so resolved.x should equal original
    REQUIRE(resolved.x == Catch::Approx(pos.x));
}

// ── G3: PlayerController test ───────────────────────────────────

TEST_CASE("PlayerController init and update", "[Game][PlayerController][G3]") {
    NF::PlayerController ctrl;
    ctrl.init({0, 0, 0});
    REQUIRE(ctrl.position().x == Catch::Approx(0.f));

    NF::WorldState world;
    NF::MovementInput input;
    input.forward = true;

    ctrl.update(0.016f, input, 0.f, 0.f, world);

    // Camera should be at eye height
    REQUIRE(ctrl.camera().position().y > 0.f);
    // Look direction should be valid
    float len = ctrl.lookDirection().length();
    REQUIRE(len == Catch::Approx(1.f).margin(1e-4));
}

// ── G4: Ship Systems ─────────────────────────────────────────────

TEST_CASE("ShipClass name conversion", "[Game][Ship][G4]") {
    REQUIRE(std::string(NF::shipClassName(NF::ShipClass::Fighter))   == "Fighter");
    REQUIRE(std::string(NF::shipClassName(NF::ShipClass::Corvette))  == "Corvette");
    REQUIRE(std::string(NF::shipClassName(NF::ShipClass::Frigate))   == "Frigate");
    REQUIRE(std::string(NF::shipClassName(NF::ShipClass::Cruiser))   == "Cruiser");
    REQUIRE(std::string(NF::shipClassName(NF::ShipClass::Freighter)) == "Freighter");
    REQUIRE(std::string(NF::shipClassName(NF::ShipClass::Count))     == "Unknown");
}

TEST_CASE("ModuleSlotType name conversion", "[Game][Ship][G4]") {
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Weapon))  == "Weapon");
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Shield))  == "Shield");
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Engine))  == "Engine");
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Reactor)) == "Reactor");
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Cargo))   == "Cargo");
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Utility)) == "Utility");
    REQUIRE(std::string(NF::moduleSlotTypeName(NF::ModuleSlotType::Count))   == "Unknown");
}

TEST_CASE("ShipModule defaults and damage", "[Game][Ship][G4]") {
    NF::ShipModule mod;
    REQUIRE(mod.health == Catch::Approx(100.f));
    REQUIRE(mod.active == true);
    REQUIRE_FALSE(mod.isDestroyed());

    mod.takeDamage(60.f);
    REQUIRE(mod.health == Catch::Approx(40.f));
    REQUIRE_FALSE(mod.isDestroyed());

    mod.takeDamage(50.f);
    REQUIRE(mod.health == Catch::Approx(0.f));
    REQUIRE(mod.isDestroyed());
    REQUIRE(mod.active == false);
}

TEST_CASE("ShipModule repair", "[Game][Ship][G4]") {
    NF::ShipModule mod;
    mod.takeDamage(100.f);
    REQUIRE(mod.isDestroyed());
    REQUIRE(mod.active == false);

    mod.repair(50.f);
    REQUIRE(mod.health == Catch::Approx(50.f));
    REQUIRE(mod.active == true);
    REQUIRE_FALSE(mod.isDestroyed());

    mod.repair(200.f);
    REQUIRE(mod.health == Catch::Approx(100.f));
}

TEST_CASE("Ship init by class — Fighter", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("TestFighter"));
    REQUIRE(ship.shipClass() == NF::ShipClass::Fighter);
    REQUIRE(ship.maxHull() == Catch::Approx(100.f));
    REQUIRE(ship.maxShield() == Catch::Approx(50.f));
    REQUIRE(ship.maxModules() == 4);
    REQUIRE(ship.hull() == Catch::Approx(100.f));
    REQUIRE(ship.shield() == Catch::Approx(50.f));
}

TEST_CASE("Ship init by class — Corvette", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Corvette, NF::StringID("TestCorvette"));
    REQUIRE(ship.maxHull() == Catch::Approx(200.f));
    REQUIRE(ship.maxShield() == Catch::Approx(100.f));
    REQUIRE(ship.maxModules() == 6);
}

TEST_CASE("Ship init by class — Frigate", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Frigate, NF::StringID("TestFrigate"));
    REQUIRE(ship.maxHull() == Catch::Approx(400.f));
    REQUIRE(ship.maxShield() == Catch::Approx(200.f));
    REQUIRE(ship.maxModules() == 8);
}

TEST_CASE("Ship add/remove modules", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Modular"));
    REQUIRE(ship.moduleCount() == 0);

    NF::ShipModule eng;
    eng.slotType = NF::ModuleSlotType::Engine;
    eng.thrustPower = 50.f;
    REQUIRE(ship.addModule(eng));
    REQUIRE(ship.moduleCount() == 1);
    REQUIRE(ship.module(0)->slotType == NF::ModuleSlotType::Engine);

    ship.removeModule(0);
    REQUIRE(ship.moduleCount() == 0);
}

TEST_CASE("Ship max modules limit", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Full"));

    NF::ShipModule mod;
    for (int i = 0; i < 4; ++i)
        REQUIRE(ship.addModule(mod));
    REQUIRE(ship.moduleCount() == 4);
    REQUIRE_FALSE(ship.addModule(mod));
    REQUIRE(ship.moduleCount() == 4);
}

TEST_CASE("Ship hull damage — shield absorbs first", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("DmgTest"));
    REQUIRE(ship.shield() == Catch::Approx(50.f));
    REQUIRE(ship.hull() == Catch::Approx(100.f));

    ship.takeDamage(30.f);
    REQUIRE(ship.shield() == Catch::Approx(20.f));
    REQUIRE(ship.hull() == Catch::Approx(100.f));

    ship.takeDamage(40.f);
    REQUIRE(ship.shield() == Catch::Approx(0.f));
    REQUIRE(ship.hull() == Catch::Approx(80.f));
}

TEST_CASE("Ship destroyed state", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Doomed"));
    REQUIRE_FALSE(ship.isDestroyed());

    ship.takeDamage(50.f + 100.f);
    REQUIRE(ship.isDestroyed());
    REQUIRE(ship.hull() == Catch::Approx(0.f));
}

TEST_CASE("Ship computeStats", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Corvette, NF::StringID("Stats"));

    NF::ShipModule eng;
    eng.slotType = NF::ModuleSlotType::Engine;
    eng.thrustPower = 40.f;
    ship.addModule(eng);

    NF::ShipModule reactor;
    reactor.slotType = NF::ModuleSlotType::Reactor;
    reactor.powerOutput = 100.f;
    ship.addModule(reactor);

    NF::ShipModule weapon;
    weapon.slotType = NF::ModuleSlotType::Weapon;
    weapon.damage = 10.f;
    weapon.fireRate = 2.f;
    ship.addModule(weapon);

    NF::ShipStats stats = ship.computeStats();
    REQUIRE(stats.totalThrust == Catch::Approx(40.f));
    REQUIRE(stats.totalPowerOutput == Catch::Approx(100.f));
    REQUIRE(stats.maxWeaponDPS == Catch::Approx(20.f));
    REQUIRE(stats.activeModuleCount == 3);
    REQUIRE(stats.destroyedModuleCount == 0);
}

TEST_CASE("Ship shield recharge", "[Game][Ship][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Regen"));
    ship.takeDamage(30.f);
    REQUIRE(ship.shield() == Catch::Approx(20.f));

    ship.rechargeShield(1.f);
    REQUIRE(ship.shield() == Catch::Approx(25.f));

    ship.rechargeShield(100.f);
    REQUIRE(ship.shield() == Catch::Approx(50.f));
}

TEST_CASE("FlightState defaults", "[Game][Flight][G4]") {
    NF::FlightState fs;
    REQUIRE(fs.speed == Catch::Approx(0.f));
    REQUIRE(fs.maxSpeed == Catch::Approx(100.f));
    REQUIRE(fs.turnRate == Catch::Approx(90.f));
    REQUIRE(fs.boostMultiplier == Catch::Approx(2.f));
    REQUIRE(fs.boosting == false);
    REQUIRE(fs.forward.z == Catch::Approx(1.f));
}

TEST_CASE("FlightController init", "[Game][Flight][G4]") {
    NF::FlightController fc;
    fc.init({10, 20, 30});
    REQUIRE(fc.position().x == Catch::Approx(10.f));
    REQUIRE(fc.position().y == Catch::Approx(20.f));
    REQUIRE(fc.position().z == Catch::Approx(30.f));
    REQUIRE(fc.speed() == Catch::Approx(0.f));
}

TEST_CASE("FlightController update throttle", "[Game][Flight][G4]") {
    NF::FlightController fc;
    fc.init({0, 0, 0});

    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Fly"));
    NF::ShipModule eng;
    eng.slotType = NF::ModuleSlotType::Engine;
    eng.thrustPower = 50.f;
    ship.addModule(eng);

    NF::FlightInput input;
    input.throttle = 1.f;

    fc.update(1.f, input, ship);
    REQUIRE(fc.speed() > 0.f);
    REQUIRE(fc.position().z > 0.f);
}

TEST_CASE("FlightController boost", "[Game][Flight][G4]") {
    NF::FlightController fc;
    fc.init({0, 0, 0});

    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Boost"));
    NF::ShipModule eng;
    eng.slotType = NF::ModuleSlotType::Engine;
    eng.thrustPower = 50.f;
    ship.addModule(eng);

    NF::FlightInput input;
    input.throttle = 1.f;
    input.boost = true;

    fc.update(1.f, input, ship);
    REQUIRE(fc.state().boosting == true);
    REQUIRE(fc.speed() > 0.f);
}

TEST_CASE("WeaponState cooldown", "[Game][Combat][G4]") {
    NF::WeaponState ws;
    ws.cooldown = 1.f;
    REQUIRE_FALSE(ws.isReady());

    ws.tick(0.5f);
    REQUIRE(ws.cooldown == Catch::Approx(0.5f));
    REQUIRE_FALSE(ws.isReady());

    ws.tick(0.6f);
    REQUIRE(ws.cooldown == Catch::Approx(0.f));
    REQUIRE(ws.isReady());
}

TEST_CASE("CombatSystem calculate damage", "[Game][Combat][G4]") {
    NF::Ship ship;
    ship.init(NF::ShipClass::Fighter, NF::StringID("Gunner"));

    NF::ShipModule weapon;
    weapon.slotType = NF::ModuleSlotType::Weapon;
    weapon.damage = 25.f;
    weapon.fireRate = 2.f;
    ship.addModule(weapon);

    NF::CombatSystem cs;
    cs.init();

    float dmg = cs.calculateDamage(ship, 0);
    REQUIRE(dmg == Catch::Approx(25.f));

    float noDmg = cs.calculateDamage(ship, 5);
    REQUIRE(noDmg == Catch::Approx(0.f));
}

TEST_CASE("CombatSystem evaluate target in range", "[Game][Combat][G4]") {
    NF::CombatSystem cs;
    cs.init();

    NF::Vec3 shipPos{0, 0, 0};
    NF::Vec3 shipFwd{0, 0, 1};
    NF::Vec3 targetPos{0, 0, 50};

    NF::CombatTarget ct = cs.evaluateTarget(shipPos, shipFwd, targetPos, 100.f, 30.f);
    REQUIRE(ct.inRange == true);
    REQUIRE(ct.inFiringArc == true);
    REQUIRE(ct.distance == Catch::Approx(50.f));
}

TEST_CASE("CombatSystem evaluate target out of range", "[Game][Combat][G4]") {
    NF::CombatSystem cs;
    cs.init();

    NF::Vec3 shipPos{0, 0, 0};
    NF::Vec3 shipFwd{0, 0, 1};
    NF::Vec3 targetPos{200, 0, 0};

    NF::CombatTarget ct = cs.evaluateTarget(shipPos, shipFwd, targetPos, 100.f, 30.f);
    REQUIRE(ct.inRange == false);
    REQUIRE(ct.inFiringArc == false);
}

TEST_CASE("CombatSystem apply damage", "[Game][Combat][G4]") {
    NF::Ship attacker;
    attacker.init(NF::ShipClass::Fighter, NF::StringID("Attacker"));
    NF::ShipModule weapon;
    weapon.slotType = NF::ModuleSlotType::Weapon;
    weapon.damage = 30.f;
    weapon.fireRate = 1.f;
    attacker.addModule(weapon);

    NF::Ship target;
    target.init(NF::ShipClass::Fighter, NF::StringID("Target"));
    float initialShield = target.shield();

    NF::CombatSystem cs;
    cs.init();

    float applied = cs.applyDamage(attacker, target, 0);
    REQUIRE(applied == Catch::Approx(30.f));
    REQUIRE(target.shield() == Catch::Approx(initialShield - 30.f));
    REQUIRE(target.hull() == Catch::Approx(100.f));
}

// ── G5 Tests ─────────────────────────────────────────────────────

TEST_CASE("NF::FormationType name", "[g5]") {
    REQUIRE(std::string(NF::formationTypeName(NF::FormationType::Line)) == "Line");
    REQUIRE(std::string(NF::formationTypeName(NF::FormationType::Wedge)) == "Wedge");
    REQUIRE(std::string(NF::formationTypeName(NF::FormationType::Column)) == "Column");
    REQUIRE(std::string(NF::formationTypeName(NF::FormationType::Spread)) == "Spread");
    REQUIRE(std::string(NF::formationTypeName(NF::FormationType::Defensive)) == "Defensive");
}

TEST_CASE("NF::Formation init line", "[g5]") {
    NF::Formation f;
    f.init(NF::FormationType::Line, 4, 20.f);
    REQUIRE(f.slotCount() == 4);
    REQUIRE(f.slot(0).offset.x != f.slot(3).offset.x);
    REQUIRE(f.slot(0).offset.z == 0.f);
}

TEST_CASE("NF::Formation init wedge", "[g5]") {
    NF::Formation f;
    f.init(NF::FormationType::Wedge, 5, 10.f);
    REQUIRE(f.slotCount() == 5);
    REQUIRE(f.slot(2).offset.z == 0.f);
}

TEST_CASE("NF::Formation slot world position", "[g5]") {
    NF::Formation f;
    f.init(NF::FormationType::Line, 1, 20.f);
    NF::Vec3 leaderPos{10,0,10};
    NF::Vec3 leaderFwd{0,0,1};
    NF::Vec3 result = f.getSlotWorldPosition(0, leaderPos, leaderFwd);
    REQUIRE(result.x == Catch::Approx(10.f));
    REQUIRE(result.y == Catch::Approx(0.f));
    REQUIRE(result.z == Catch::Approx(10.f));
}

TEST_CASE("NF::CaptainPersonality defaults", "[g5]") {
    NF::CaptainPersonality p;
    REQUIRE(p.morale == Catch::Approx(1.0f));
    REQUIRE(p.confidence == Catch::Approx(1.0f));
    REQUIRE(p.aggression == Catch::Approx(0.5f));
}

TEST_CASE("NF::CaptainPersonality adjust morale", "[g5]") {
    NF::CaptainPersonality p;
    p.adjustMorale(-0.5f);
    REQUIRE(p.morale == Catch::Approx(0.5f));
    p.adjustMorale(-2.f);
    REQUIRE(p.morale == Catch::Approx(0.f));
    p.adjustMorale(5.f);
    REQUIRE(p.morale == Catch::Approx(1.f));
}

TEST_CASE("NF::CaptainPersonality willFlee", "[g5]") {
    NF::CaptainPersonality p;
    p.morale = 0.1f;
    p.caution = 0.8f;
    REQUIRE(p.willFlee() == true);
    p.morale = 0.5f;
    REQUIRE(p.willFlee() == false);
}

TEST_CASE("NF::CaptainPersonality willCharge", "[g5]") {
    NF::CaptainPersonality p;
    p.aggression = 0.9f;
    p.confidence = 0.8f;
    REQUIRE(p.willCharge() == true);
    p.aggression = 0.3f;
    REQUIRE(p.willCharge() == false);
}

TEST_CASE("NF::CaptainOrder name", "[g5]") {
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::HoldPosition)) == "HoldPosition");
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::AttackTarget)) == "AttackTarget");
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::DefendTarget)) == "DefendTarget");
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::FollowLeader)) == "FollowLeader");
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::Patrol)) == "Patrol");
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::Retreat)) == "Retreat");
    REQUIRE(std::string(NF::captainOrderName(NF::CaptainOrder::FreeEngage)) == "FreeEngage");
}

TEST_CASE("NF::AICaptain init", "[g5]") {
    NF::AICaptain cap;
    NF::CaptainPersonality p;
    p.aggression = 0.8f;
    NF::StringID name(NF::StringID("captain1"));
    cap.init(name, p);
    REQUIRE(cap.name() == name);
    REQUIRE(cap.personality().aggression == Catch::Approx(0.8f));
}

TEST_CASE("NF::AICaptain evaluate retreat", "[g5]") {
    NF::AICaptain cap;
    NF::CaptainPersonality p;
    p.morale = 0.1f;
    p.caution = 0.9f;
    cap.init(NF::StringID("cap"), p);
    auto order = cap.evaluate(0.2f, 1.f, 500.f, 1, 0);
    REQUIRE(order == NF::CaptainOrder::Retreat);
}

TEST_CASE("NF::AICaptain evaluate attack", "[g5]") {
    NF::AICaptain cap;
    NF::CaptainPersonality p;
    p.aggression = 0.9f;
    p.confidence = 0.8f;
    cap.init(NF::StringID("cap"), p);
    auto order = cap.evaluate(1.f, 1.f, 100.f, 1, 0);
    REQUIRE(order == NF::CaptainOrder::AttackTarget);
}

TEST_CASE("NF::AICaptain order override", "[g5]") {
    NF::AICaptain cap;
    NF::CaptainPersonality p;
    cap.init(NF::StringID("cap"), p);
    cap.setOrder(NF::CaptainOrder::Patrol);
    REQUIRE(cap.currentOrder() == NF::CaptainOrder::Patrol);
    cap.overrideOrder(NF::CaptainOrder::Retreat);
    REQUIRE(cap.currentOrder() == NF::CaptainOrder::Retreat);
    REQUIRE(cap.hasOverride() == true);
    cap.clearOverride();
    REQUIRE(cap.hasOverride() == false);
    REQUIRE(cap.currentOrder() == NF::CaptainOrder::Patrol);
}

TEST_CASE("NF::FleetShip defaults", "[g5]") {
    NF::FleetShip fs;
    REQUIRE(fs.active == true);
    REQUIRE(fs.formationSlot == -1);
}

TEST_CASE("NF::Fleet init and add ships", "[g5]") {
    NF::Fleet fleet;
    fleet.init(NF::StringID("fleet1"));
    NF::FleetShip s1, s2;
    fleet.addShip(std::move(s1));
    fleet.addShip(std::move(s2));
    REQUIRE(fleet.shipCount() == 2);
    REQUIRE(fleet.activeShipCount() == 2);
}

TEST_CASE("NF::Fleet remove ship", "[g5]") {
    NF::Fleet fleet;
    fleet.init(NF::StringID("fleet1"));
    NF::FleetShip s;
    fleet.addShip(std::move(s));
    REQUIRE(fleet.activeShipCount() == 1);
    fleet.removeShip(0);
    REQUIRE(fleet.activeShipCount() == 0);
}

TEST_CASE("NF::Fleet set formation", "[g5]") {
    NF::Fleet fleet;
    fleet.init(NF::StringID("fleet1"));
    NF::FleetShip s1, s2, s3;
    fleet.addShip(std::move(s1));
    fleet.addShip(std::move(s2));
    fleet.addShip(std::move(s3));
    fleet.setFormation(NF::FormationType::Wedge, 15.f);
    REQUIRE(fleet.formation().type() == NF::FormationType::Wedge);
    REQUIRE(fleet.formation().slotCount() == 3);
}

TEST_CASE("NF::Fleet issue order and morale", "[g5]") {
    NF::Fleet fleet;
    fleet.init(NF::StringID("fleet1"));
    NF::FleetShip s1, s2;
    fleet.addShip(std::move(s1));
    fleet.addShip(std::move(s2));
    fleet.issueOrder(NF::CaptainOrder::Patrol);
    REQUIRE(fleet.ship(0)->captain.currentOrder() == NF::CaptainOrder::Patrol);
    REQUIRE(fleet.ship(1)->captain.currentOrder() == NF::CaptainOrder::Patrol);
    REQUIRE(fleet.fleetMorale() == Catch::Approx(1.f));
}

// ── G6 Tests ─────────────────────────────────────────────────────

TEST_CASE("NF::MarketItem defaults", "[g6]") {
    NF::MarketItem item;
    REQUIRE(item.quantity == 0);
    REQUIRE(item.buyPrice == Catch::Approx(10.f));
    REQUIRE(item.sellPrice == Catch::Approx(8.f));
}

TEST_CASE("NF::Market list and find item", "[g6]") {
    NF::Market m;
    m.init();
    m.listItem(NF::ResourceType::RawIron, 100, 15.f, 10.f);
    const NF::MarketItem* item = m.findItem(NF::ResourceType::RawIron);
    REQUIRE(item != nullptr);
    REQUIRE(item->quantity == 100);
    REQUIRE(item->buyPrice == Catch::Approx(15.f));
    REQUIRE(m.itemCount() == 1);
}

TEST_CASE("NF::Market buy success", "[g6]") {
    NF::Market m;
    m.init();
    m.listItem(NF::ResourceType::RawIron, 100, 10.f, 8.f);
    NF::ResourceInventory inv;
    float credits = 200.f;
    bool ok = m.buy(inv, NF::ResourceType::RawIron, 5, credits);
    REQUIRE(ok == true);
    REQUIRE(credits == Catch::Approx(150.f));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 5);
}

TEST_CASE("NF::Market buy insufficient credits", "[g6]") {
    NF::Market m;
    m.init();
    m.listItem(NF::ResourceType::RawIron, 100, 10.f, 8.f);
    NF::ResourceInventory inv;
    float credits = 5.f;
    bool ok = m.buy(inv, NF::ResourceType::RawIron, 5, credits);
    REQUIRE(ok == false);
    REQUIRE(credits == Catch::Approx(5.f));
}

TEST_CASE("NF::Market buy insufficient stock", "[g6]") {
    NF::Market m;
    m.init();
    m.listItem(NF::ResourceType::RawIron, 2, 10.f, 8.f);
    NF::ResourceInventory inv;
    float credits = 1000.f;
    bool ok = m.buy(inv, NF::ResourceType::RawIron, 10, credits);
    REQUIRE(ok == false);
}

TEST_CASE("NF::Market sell success", "[g6]") {
    NF::Market m;
    m.init();
    m.listItem(NF::ResourceType::RawIron, 0, 10.f, 8.f);
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawIron, 10);
    float credits = 0.f;
    bool ok = m.sell(inv, NF::ResourceType::RawIron, 5, credits);
    REQUIRE(ok == true);
    REQUIRE(credits == Catch::Approx(40.f));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 5);
}

TEST_CASE("NF::Market sell insufficient inventory", "[g6]") {
    NF::Market m;
    m.init();
    NF::ResourceInventory inv;
    float credits = 0.f;
    bool ok = m.sell(inv, NF::ResourceType::RawIron, 5, credits);
    REQUIRE(ok == false);
}

TEST_CASE("NF::RefiningRecipe fields", "[g6]") {
    NF::RefiningRecipe r;
    r.input = NF::ResourceType::RawIron;
    r.inputAmount = 2;
    r.output = NF::ResourceType::RefinedIron;
    r.outputAmount = 1;
    r.timeRequired = 5.f;
    REQUIRE(r.inputAmount == 2);
    REQUIRE(r.outputAmount == 1);
    REQUIRE(r.timeRequired == Catch::Approx(5.f));
}

TEST_CASE("NF::Refinery add recipe and find", "[g6]") {
    NF::Refinery ref;
    NF::RefiningRecipe r;
    r.input = NF::ResourceType::RawIron;
    r.inputAmount = 2;
    r.output = NF::ResourceType::RefinedIron;
    r.outputAmount = 1;
    r.timeRequired = 3.f;
    ref.addRecipe(r);
    REQUIRE(ref.recipeCount() == 1);
    const NF::RefiningRecipe* found = ref.findRecipe(NF::ResourceType::RawIron);
    REQUIRE(found != nullptr);
    REQUIRE(found->outputAmount == 1);
}

TEST_CASE("NF::Refinery start and collect", "[g6]") {
    NF::Refinery ref;
    NF::RefiningRecipe r;
    r.input = NF::ResourceType::RawIron;
    r.inputAmount = 2;
    r.output = NF::ResourceType::RefinedIron;
    r.outputAmount = 1;
    r.timeRequired = 3.f;
    ref.addRecipe(r);
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawIron, 10);
    float t = ref.startRefining(inv, NF::ResourceType::RawIron, 4);
    REQUIRE(t == Catch::Approx(6.f));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 6);
    ref.collectOutput(inv);
    REQUIRE(inv.count(NF::ResourceType::RefinedIron) == 2);
}

TEST_CASE("NF::Refinery no recipe returns 0", "[g6]") {
    NF::Refinery ref;
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawIron, 10);
    float t = ref.startRefining(inv, NF::ResourceType::RawIron, 4);
    REQUIRE(t == Catch::Approx(0.f));
}

TEST_CASE("NF::ManufacturingRecipe fields", "[g6]") {
    NF::ManufacturingRecipe r;
    r.name = NF::StringID("cannon");
    r.output = NF::ResourceType::SteelPlate;
    r.outputAmount = 1;
    r.timeRequired = 10.f;
    REQUIRE(r.outputAmount == 1);
    REQUIRE(r.timeRequired == Catch::Approx(10.f));
}

TEST_CASE("NF::Manufacturer can craft", "[g6]") {
    NF::Manufacturer mfg;
    NF::ManufacturingRecipe r;
    r.name = NF::StringID("cannon");
    r.inputs.push_back({NF::ResourceType::RawIron, 3});
    r.inputs.push_back({NF::ResourceType::EnergyCell, 2});
    r.output = NF::ResourceType::SteelPlate;
    r.outputAmount = 1;
    r.timeRequired = 5.f;
    mfg.addRecipe(r);
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawIron, 5);
    inv.add(NF::ResourceType::EnergyCell, 5);
    REQUIRE(mfg.canCraft(inv, NF::StringID("cannon")) == true);
    inv.remove(NF::ResourceType::RawIron, 4);
    REQUIRE(mfg.canCraft(inv, NF::StringID("cannon")) == false);
}

TEST_CASE("NF::Manufacturer craft output", "[g6]") {
    NF::Manufacturer mfg;
    NF::ManufacturingRecipe r;
    r.name = NF::StringID("widget");
    r.inputs.push_back({NF::ResourceType::RawIron, 2});
    r.output = NF::ResourceType::SteelPlate;
    r.outputAmount = 1;
    r.timeRequired = 4.f;
    mfg.addRecipe(r);
    NF::ResourceInventory inv;
    inv.add(NF::ResourceType::RawIron, 10);
    float t = mfg.craft(inv, NF::StringID("widget"));
    REQUIRE(t == Catch::Approx(4.f));
    REQUIRE(inv.count(NF::ResourceType::RawIron) == 8);
    REQUIRE(inv.count(NF::ResourceType::SteelPlate) == 1);
}

// ── G7 Tests ─────────────────────────────────────────────────────

TEST_CASE("NF::SectorType name", "[g7]") {
    REQUIRE(std::string(NF::sectorTypeName(NF::SectorType::Normal)) == "Normal");
    REQUIRE(std::string(NF::sectorTypeName(NF::SectorType::Nebula)) == "Nebula");
    REQUIRE(std::string(NF::sectorTypeName(NF::SectorType::AsteroidField)) == "AsteroidField");
    REQUIRE(std::string(NF::sectorTypeName(NF::SectorType::DeepSpace)) == "DeepSpace");
    REQUIRE(std::string(NF::sectorTypeName(NF::SectorType::AncientRuins)) == "AncientRuins");
}

TEST_CASE("NF::SectorInfo defaults", "[g7]") {
    NF::SectorInfo s;
    REQUIRE(s.type == NF::SectorType::Normal);
    REQUIRE(s.scanProgress == Catch::Approx(0.f));
    REQUIRE(s.fullyScanned == false);
    REQUIRE(s.hasWormhole == false);
    REQUIRE(s.hasAncientTech == false);
}

TEST_CASE("NF::ProbeScanner init", "[g7]") {
    NF::ProbeScanner ps;
    ps.init(0.2f);
    REQUIRE(ps.scanRate() == Catch::Approx(0.2f));
    REQUIRE(ps.isActive() == false);
}

TEST_CASE("NF::ProbeScanner tick completes scan", "[g7]") {
    NF::ProbeScanner ps;
    ps.init(1.f);
    NF::SectorInfo s;
    ps.startScan(s);
    REQUIRE(ps.isActive() == true);
    bool done = ps.tick(1.f);
    REQUIRE(done == true);
    REQUIRE(s.fullyScanned == true);
    REQUIRE(s.scanProgress == Catch::Approx(1.f));
    REQUIRE(ps.isActive() == false);
}

TEST_CASE("NF::ProbeScanner stop scan", "[g7]") {
    NF::ProbeScanner ps;
    ps.init(0.1f);
    NF::SectorInfo s;
    ps.startScan(s);
    REQUIRE(ps.isActive() == true);
    ps.stopScan();
    REQUIRE(ps.isActive() == false);
}

TEST_CASE("NF::WormholeLink traversable", "[g7]") {
    NF::WormholeLink wh;
    wh.stability = 0.5f;
    REQUIRE(wh.isTraversable() == true);
    wh.stability = 0.05f;
    REQUIRE(wh.isTraversable() == false);
}

TEST_CASE("NF::WormholeLink degrade", "[g7]") {
    NF::WormholeLink wh;
    wh.stability = 1.f;
    wh.degrade(0.3f);
    REQUIRE(wh.stability == Catch::Approx(0.7f));
    wh.degrade(2.f);
    REQUIRE(wh.stability == Catch::Approx(0.f));
}

TEST_CASE("NF::StarMap add and find sector", "[g7]") {
    NF::StarMap sm;
    NF::SectorInfo s;
    s.name = NF::StringID("alpha");
    s.type = NF::SectorType::Nebula;
    sm.addSector(s);
    REQUIRE(sm.sectorCount() == 1);
    const NF::SectorInfo* found = sm.findSector(NF::StringID("alpha"));
    REQUIRE(found != nullptr);
    REQUIRE(found->type == NF::SectorType::Nebula);
}

TEST_CASE("NF::StarMap add wormhole", "[g7]") {
    NF::StarMap sm;
    NF::WormholeLink wh;
    wh.fromSector = NF::StringID("alpha");
    wh.toSector = NF::StringID("beta");
    sm.addWormhole(wh);
    REQUIRE(sm.wormholeCount() == 1);
}

TEST_CASE("NF::StarMap reachable sectors", "[g7]") {
    NF::StarMap sm;
    NF::SectorInfo s1; s1.name = NF::StringID("alpha");
    NF::SectorInfo s2; s2.name = NF::StringID("beta");
    sm.addSector(s1);
    sm.addSector(s2);
    NF::WormholeLink wh;
    wh.fromSector = NF::StringID("alpha");
    wh.toSector = NF::StringID("beta");
    wh.twoWay = true;
    wh.stability = 1.f;
    sm.addWormhole(wh);
    auto reachable = sm.getReachableSectors(NF::StringID("alpha"));
    REQUIRE(reachable.size() == 1);
    REQUIRE(reachable[0] == NF::StringID("beta"));
    auto fromBeta = sm.getReachableSectors(NF::StringID("beta"));
    REQUIRE(fromBeta.size() == 1);
    REQUIRE(fromBeta[0] == NF::StringID("alpha"));
}

TEST_CASE("NF::StarMap ancient tech sectors", "[g7]") {
    NF::StarMap sm;
    NF::SectorInfo s1; s1.name = NF::StringID("alpha"); s1.hasAncientTech = true;
    NF::SectorInfo s2; s2.name = NF::StringID("beta");  s2.hasAncientTech = false;
    NF::SectorInfo s3; s3.name = NF::StringID("gamma"); s3.hasAncientTech = true;
    sm.addSector(s1);
    sm.addSector(s2);
    sm.addSector(s3);
    auto techSectors = sm.getAncientTechSectors();
    REQUIRE(techSectors.size() == 2);
}

TEST_CASE("NF::AncientTechFragment defaults", "[g7]") {
    NF::AncientTechFragment f;
    REQUIRE(f.tier == 1);
    REQUIRE(f.analyzed == false);
    REQUIRE(f.damageBonus == Catch::Approx(0.f));
}

TEST_CASE("NF::AncientTechRegistry add and find", "[g7]") {
    NF::AncientTechRegistry reg;
    NF::AncientTechFragment f;
    f.name = NF::StringID("fragment1");
    f.tier = 3;
    reg.add(f);
    REQUIRE(reg.count() == 1);
    auto* found = reg.find(NF::StringID("fragment1"));
    REQUIRE(found != nullptr);
    REQUIRE(found->tier == 3);
}

TEST_CASE("NF::AncientTechRegistry analyze", "[g7]") {
    NF::AncientTechRegistry reg;
    NF::AncientTechFragment f;
    f.name = NF::StringID("frag1");
    f.tier = 2;
    reg.add(f);
    REQUIRE(reg.analyzedCount() == 0);
    reg.analyze(NF::StringID("frag1"));
    REQUIRE(reg.analyzedCount() == 1);
    auto* found = reg.find(NF::StringID("frag1"));
    REQUIRE(found->analyzed == true);
    REQUIRE(found->damageBonus == Catch::Approx(0.1f));
}

// ── G8 Tests ─────────────────────────────────────────────────────

TEST_CASE("NF::RoomType name", "[g8]") {
    REQUIRE(std::string(NF::roomTypeName(NF::RoomType::Bridge)) == "Bridge");
    REQUIRE(std::string(NF::roomTypeName(NF::RoomType::Engineering)) == "Engineering");
    REQUIRE(std::string(NF::roomTypeName(NF::RoomType::MedBay)) == "MedBay");
    REQUIRE(std::string(NF::roomTypeName(NF::RoomType::Cargo)) == "Cargo");
    REQUIRE(std::string(NF::roomTypeName(NF::RoomType::Airlock)) == "Airlock");
    REQUIRE(std::string(NF::roomTypeName(NF::RoomType::Corridor)) == "Corridor");
}

TEST_CASE("NF::ShipRoom defaults", "[g8]") {
    NF::ShipRoom r;
    REQUIRE(r.type == NF::RoomType::Corridor);
    REQUIRE(r.oxygenLevel == Catch::Approx(1.f));
    REQUIRE(r.temperature == Catch::Approx(20.f));
    REQUIRE(r.pressurized == true);
}

TEST_CASE("NF::ShipRoom connect and query", "[g8]") {
    NF::ShipRoom r;
    r.name = NF::StringID("bridge");
    r.connect(NF::StringID("corridor1"));
    r.connect(NF::StringID("airlock1"));
    REQUIRE(r.isConnectedTo(NF::StringID("corridor1")) == true);
    REQUIRE(r.isConnectedTo(NF::StringID("airlock1")) == true);
    REQUIRE(r.isConnectedTo(NF::StringID("engineering")) == false);
}

TEST_CASE("NF::ShipRoom habitable", "[g8]") {
    NF::ShipRoom r;
    r.pressurized = true;
    r.oxygenLevel = 0.8f;
    r.temperature = 20.f;
    REQUIRE(r.isHabitable() == true);
}

TEST_CASE("NF::ShipRoom not habitable vacuum", "[g8]") {
    NF::ShipRoom r;
    r.pressurized = false;
    r.oxygenLevel = 0.f;
    REQUIRE(r.isHabitable() == false);
}

TEST_CASE("NF::ShipInterior add and find room", "[g8]") {
    NF::ShipInterior interior;
    NF::ShipRoom bridge;
    bridge.name = NF::StringID("bridge");
    bridge.type = NF::RoomType::Bridge;
    interior.addRoom(bridge);
    REQUIRE(interior.roomCount() == 1);
    auto* found = interior.findRoom(NF::StringID("bridge"));
    REQUIRE(found != nullptr);
    REQUIRE(found->type == NF::RoomType::Bridge);
}

TEST_CASE("NF::ShipInterior decompress", "[g8]") {
    NF::ShipInterior interior;
    NF::ShipRoom airlock;
    airlock.name = NF::StringID("airlock");
    airlock.type = NF::RoomType::Airlock;
    interior.addRoom(airlock);
    interior.decompress(NF::StringID("airlock"));
    auto* r = interior.findRoom(NF::StringID("airlock"));
    REQUIRE(r->oxygenLevel == Catch::Approx(0.f));
    REQUIRE(r->pressurized == false);
}

TEST_CASE("NF::ShipInterior repressurize", "[g8]") {
    NF::ShipInterior interior;
    NF::ShipRoom airlock;
    airlock.name = NF::StringID("airlock");
    interior.addRoom(airlock);
    interior.decompress(NF::StringID("airlock"));
    interior.repressurize(NF::StringID("airlock"));
    auto* r = interior.findRoom(NF::StringID("airlock"));
    REQUIRE(r->oxygenLevel == Catch::Approx(1.f));
    REQUIRE(r->pressurized == true);
}

TEST_CASE("NF::ShipInterior habitable count", "[g8]") {
    NF::ShipInterior interior;
    NF::ShipRoom r1; r1.name = NF::StringID("r1");
    NF::ShipRoom r2; r2.name = NF::StringID("r2");
    NF::ShipRoom r3; r3.name = NF::StringID("r3");
    interior.addRoom(r1);
    interior.addRoom(r2);
    interior.addRoom(r3);
    REQUIRE(interior.habitableRoomCount() == 3);
    interior.decompress(NF::StringID("r1"));
    REQUIRE(interior.habitableRoomCount() == 2);
}

TEST_CASE("NF::EVAState defaults", "[g8]") {
    NF::EVAState eva;
    REQUIRE(eva.active == false);
    REQUIRE(eva.suitIntegrity == Catch::Approx(100.f));
    REQUIRE(eva.oxygenSupply == Catch::Approx(300.f));
    REQUIRE(eva.jetpackFuel == Catch::Approx(100.f));
    REQUIRE(eva.isAlive() == true);
}

TEST_CASE("NF::EVAState tick drains oxygen", "[g8]") {
    NF::EVAState eva;
    eva.active = true;
    eva.tick(10.f);
    REQUIRE(eva.oxygenSupply == Catch::Approx(290.f));
    eva.tick(300.f);
    REQUIRE(eva.oxygenSupply == Catch::Approx(0.f));
    REQUIRE(eva.isAlive() == false);
}

TEST_CASE("NF::EVAState use thruster", "[g8]") {
    NF::EVAState eva;
    eva.active = true;
    eva.useThruster(NF::Vec3{1.f, 0.f, 0.f}, 10.f);
    REQUIRE(eva.velocity.x == Catch::Approx(1.f));
    REQUIRE(eva.jetpackFuel == Catch::Approx(90.f));
    eva.useThruster(NF::Vec3{1.f, 0.f, 0.f}, 200.f);
    REQUIRE(eva.jetpackFuel == Catch::Approx(0.f));
    float prevVelX = eva.velocity.x;
    eva.useThruster(NF::Vec3{5.f, 0.f, 0.f}, 10.f);
    REQUIRE(eva.velocity.x == Catch::Approx(prevVelX));
}

TEST_CASE("NF::SurvivalStatus danger thresholds", "[g8]") {
    NF::SurvivalStatus ss;
    REQUIRE(ss.isInDanger() == false);
    ss.radiation = 60.f;
    REQUIRE(ss.isRadiationDangerous() == true);
    REQUIRE(ss.isInDanger() == true);
    ss.radiation = 0.f;
    ss.temperature = 34.f;
    REQUIRE(ss.isHypothermic() == true);
    ss.temperature = 41.f;
    REQUIRE(ss.isHyperthermic() == true);
}

TEST_CASE("NF::SurvivalStatus tick in vacuum", "[g8]") {
    NF::SurvivalStatus ss;
    ss.tick(1.f, nullptr);
    REQUIRE(ss.inVacuum == true);
    REQUIRE(ss.radiation == Catch::Approx(2.f));
    REQUIRE(ss.temperature == Catch::Approx(32.f));
}

// ── G9 Tests ─────────────────────────────────────────────────────

TEST_CASE("NF::ReputationTier from score", "[g9]") {
    REQUIRE(NF::reputationTierFromScore(-600.f) == NF::ReputationTier::Infamous);
    REQUIRE(NF::reputationTierFromScore(-200.f) == NF::ReputationTier::Outlaw);
    REQUIRE(NF::reputationTierFromScore(0.f) == NF::ReputationTier::Neutral);
    REQUIRE(NF::reputationTierFromScore(200.f) == NF::ReputationTier::Trusted);
    REQUIRE(NF::reputationTierFromScore(700.f) == NF::ReputationTier::Honored);
    REQUIRE(NF::reputationTierFromScore(1500.f) == NF::ReputationTier::Legend);
}

TEST_CASE("NF::ReputationTier name", "[g9]") {
    REQUIRE(std::string(NF::reputationTierName(NF::ReputationTier::Infamous)) == "Infamous");
    REQUIRE(std::string(NF::reputationTierName(NF::ReputationTier::Outlaw)) == "Outlaw");
    REQUIRE(std::string(NF::reputationTierName(NF::ReputationTier::Neutral)) == "Neutral");
    REQUIRE(std::string(NF::reputationTierName(NF::ReputationTier::Trusted)) == "Trusted");
    REQUIRE(std::string(NF::reputationTierName(NF::ReputationTier::Honored)) == "Honored");
    REQUIRE(std::string(NF::reputationTierName(NF::ReputationTier::Legend)) == "Legend");
}

TEST_CASE("NF::PlayerReputation adjust and get", "[g9]") {
    NF::PlayerReputation rep;
    NF::StringID pirates(NF::StringID("pirates"));
    rep.adjustReputation(pirates, 200.f);
    REQUIRE(rep.getReputation(pirates) == Catch::Approx(200.f));
    rep.adjustReputation(pirates, -50.f);
    REQUIRE(rep.getReputation(pirates) == Catch::Approx(150.f));
    REQUIRE(rep.factionCount() == 1);
}

TEST_CASE("NF::PlayerReputation tier", "[g9]") {
    NF::PlayerReputation rep;
    NF::StringID faction(NF::StringID("guild"));
    rep.adjustReputation(faction, 300.f);
    REQUIRE(rep.getTier(faction) == NF::ReputationTier::Trusted);
}

TEST_CASE("NF::PlayerReputation global fame", "[g9]") {
    NF::PlayerReputation rep;
    rep.adjustReputation(NF::StringID("a"), 500.f);
    rep.adjustReputation(NF::StringID("b"), -300.f);
    REQUIRE(rep.globalFame() == Catch::Approx(800.f));
}

TEST_CASE("NF::WorldBias friendly and hostile", "[g9]") {
    NF::WorldBias wb;
    wb.loyaltyToPlayer = 0.5f;
    REQUIRE(wb.isFriendly() == true);
    REQUIRE(wb.isHostile() == false);
    wb.loyaltyToPlayer = -0.5f;
    REQUIRE(wb.isFriendly() == false);
    REQUIRE(wb.isHostile() == true);
    wb.loyaltyToPlayer = 0.f;
    REQUIRE(wb.isFriendly() == false);
    REQUIRE(wb.isHostile() == false);
}

TEST_CASE("NF::WorldBiasMap set and get", "[g9]") {
    NF::WorldBiasMap wbm;
    NF::WorldBias wb;
    wb.sectorName = NF::StringID("sector1");
    wb.dangerLevel = 0.7f;
    wbm.setBias(wb);
    REQUIRE(wbm.biasCount() == 1);
    auto* found = wbm.getBias(NF::StringID("sector1"));
    REQUIRE(found != nullptr);
    REQUIRE(found->dangerLevel == Catch::Approx(0.7f));
}

TEST_CASE("NF::WorldBiasMap update from reputation", "[g9]") {
    NF::WorldBiasMap wbm;
    NF::WorldBias wb;
    wb.sectorName = NF::StringID("sector1");
    wb.loyaltyToPlayer = 0.f;
    wbm.setBias(wb);
    wbm.updateFromReputation(NF::StringID("faction"), 100.f);
    auto* found = wbm.getBias(NF::StringID("sector1"));
    REQUIRE(found->loyaltyToPlayer == Catch::Approx(1.f));
}

TEST_CASE("NF::NPCMemoryEntry defaults", "[g9]") {
    NF::NPCMemoryEntry e;
    REQUIRE(e.timestamp == Catch::Approx(0.f));
    REQUIRE(e.weight == Catch::Approx(1.f));
    REQUIRE(e.positive == true);
}

TEST_CASE("NF::NPCMemory remember and count", "[g9]") {
    NF::NPCMemory mem;
    NF::NPCMemoryEntry e1;
    e1.eventType = NF::StringID("helped");
    e1.positive = true;
    NF::NPCMemoryEntry e2;
    e2.eventType = NF::StringID("attacked");
    e2.positive = false;
    mem.remember(e1);
    mem.remember(e2);
    REQUIRE(mem.entryCount() == 2);
}

TEST_CASE("NF::NPCMemory disposition", "[g9]") {
    NF::NPCMemory mem;
    NF::NPCMemoryEntry pos;
    pos.eventType = NF::StringID("helped");
    pos.weight = 2.f;
    pos.positive = true;
    NF::NPCMemoryEntry neg;
    neg.eventType = NF::StringID("attacked");
    neg.weight = 1.f;
    neg.positive = false;
    mem.remember(pos);
    mem.remember(neg);
    REQUIRE(mem.dispositionTowardPlayer() == Catch::Approx(1.f));
}

TEST_CASE("NF::NPCMemory decay", "[g9]") {
    NF::NPCMemory mem;
    NF::NPCMemoryEntry e;
    e.eventType = NF::StringID("traded");
    e.weight = 1.f;
    mem.remember(e);
    REQUIRE(mem.entryCount() == 1);
    mem.decay(200.f, 0.01f);
    REQUIRE(mem.entryCount() == 0);
}

TEST_CASE("NF::NPCMemory remembers event", "[g9]") {
    NF::NPCMemory mem;
    NF::NPCMemoryEntry e;
    e.eventType = NF::StringID("traded");
    mem.remember(e);
    REQUIRE(mem.remembers(NF::StringID("traded")) == true);
    REQUIRE(mem.remembers(NF::StringID("attacked")) == false);
}

TEST_CASE("NF::LegendStatus init and is legend", "[g9]") {
    NF::LegendStatus ls;
    ls.init();
    REQUIRE(ls.isLegend() == false);
    ls.reputation().adjustReputation(NF::StringID("a"), 1000.f);
    ls.reputation().adjustReputation(NF::StringID("b"), 1500.f);
    REQUIRE(ls.reputation().globalFame() == Catch::Approx(2500.f));
    REQUIRE(ls.isLegend() == true);
}

// ── G10: Quest & Mission System ──────────────────────────────────

TEST_CASE("NF::missionObjectiveTypeName round-trip", "[g10]") {
    REQUIRE(std::string(NF::missionObjectiveTypeName(NF::MissionObjectiveType::Kill))    == "Kill");
    REQUIRE(std::string(NF::missionObjectiveTypeName(NF::MissionObjectiveType::Collect)) == "Collect");
    REQUIRE(std::string(NF::missionObjectiveTypeName(NF::MissionObjectiveType::Deliver)) == "Deliver");
    REQUIRE(std::string(NF::missionObjectiveTypeName(NF::MissionObjectiveType::Explore)) == "Explore");
    REQUIRE(std::string(NF::missionObjectiveTypeName(NF::MissionObjectiveType::Survive)) == "Survive");
    REQUIRE(std::string(NF::missionObjectiveTypeName(NF::MissionObjectiveType::Escort))  == "Escort");
}

TEST_CASE("NF::MissionObjective progress and isComplete", "[g10]") {
    NF::MissionObjective obj;
    obj.type = NF::MissionObjectiveType::Kill;
    obj.required = 3;
    obj.current = 0;
    REQUIRE(obj.isComplete() == false);
    obj.progress(2);
    REQUIRE(obj.current == 2);
    REQUIRE(obj.isComplete() == false);
    obj.progress(5);
    REQUIRE(obj.current == 3);
    REQUIRE(obj.isComplete() == true);
}

TEST_CASE("NF::ActiveMission init and objectives", "[g10]") {
    NF::ActiveMission m;
    m.init(NF::StringID("mission_01"), "Destroy Pirates");
    REQUIRE(m.title() == "Destroy Pirates");
    REQUIRE(m.status() == NF::MissionStatus::Active);
    REQUIRE(m.objectiveCount() == 0);

    NF::MissionObjective obj;
    obj.type = NF::MissionObjectiveType::Kill;
    obj.required = 5;
    m.addObjective(obj);
    REQUIRE(m.objectiveCount() == 1);
    REQUIRE(m.allObjectivesComplete() == false);

    m.objective(0).progress(5);
    REQUIRE(m.allObjectivesComplete() == true);
}

TEST_CASE("NF::ActiveMission complete and fail", "[g10]") {
    NF::ActiveMission m;
    m.init(NF::StringID("m1"), "Test");
    m.complete();
    REQUIRE(m.status() == NF::MissionStatus::Completed);

    NF::ActiveMission m2;
    m2.init(NF::StringID("m2"), "Test2");
    m2.fail();
    REQUIRE(m2.status() == NF::MissionStatus::Failed);
}

TEST_CASE("NF::MissionReward credits and resources", "[g10]") {
    NF::MissionReward r;
    r.credits = 500;
    r.resources[NF::ResourceType::RefinedIron] = 10;
    r.reputationFactionId = NF::StringID("pirates");
    r.reputationAmount = 25.f;

    REQUIRE(r.credits == 500);
    REQUIRE(r.resources[NF::ResourceType::RefinedIron] == 10);
    REQUIRE(r.reputationAmount == Catch::Approx(25.f));
}

TEST_CASE("NF::MissionLog accept and complete", "[g10]") {
    NF::MissionLog log;
    NF::ActiveMission m;
    m.init(NF::StringID("mission_01"), "Clear Pirates");
    log.acceptMission(m);
    REQUIRE(log.activeMissionCount() == 1);
    REQUIRE(log.completedMissionCount() == 0);

    bool ok = log.completeMission(NF::StringID("mission_01"));
    REQUIRE(ok == true);
    REQUIRE(log.activeMissionCount() == 0);
    REQUIRE(log.completedMissionCount() == 1);
}

TEST_CASE("NF::MissionLog fail mission", "[g10]") {
    NF::MissionLog log;
    NF::ActiveMission m;
    m.init(NF::StringID("m_fail"), "Risky Run");
    log.acceptMission(m);
    bool ok = log.failMission(NF::StringID("m_fail"));
    REQUIRE(ok == true);
    REQUIRE(log.activeMissionCount() == 0);
    REQUIRE(log.failedMissionCount() == 1);
}

TEST_CASE("NF::MissionLog findActive", "[g10]") {
    NF::MissionLog log;
    NF::ActiveMission m;
    m.init(NF::StringID("find_me"), "Find Me");
    log.acceptMission(m);
    REQUIRE(log.findActive(NF::StringID("find_me")) != nullptr);
    REQUIRE(log.findActive(NF::StringID("nope"))    == nullptr);
}

TEST_CASE("NF::QuestChain advance and isComplete", "[g10]") {
    NF::QuestChain chain;
    chain.init("Liberation Chain");
    chain.addMission(NF::StringID("q1"));
    chain.addMission(NF::StringID("q2"));
    chain.addMission(NF::StringID("q3"));
    REQUIRE(chain.missionCount() == 3);
    REQUIRE(chain.currentIndex() == 0);
    REQUIRE(chain.isComplete() == false);

    chain.advance();
    REQUIRE(chain.currentIndex() == 1);
    chain.advance();
    chain.advance();
    REQUIRE(chain.isComplete() == true);
}

TEST_CASE("NF::QuestChain currentMissionId", "[g10]") {
    NF::QuestChain chain;
    chain.init("Test Chain");
    chain.addMission(NF::StringID("alpha"));
    chain.addMission(NF::StringID("beta"));
    REQUIRE(chain.currentMissionId() == NF::StringID("alpha"));
    chain.advance();
    REQUIRE(chain.currentMissionId() == NF::StringID("beta"));
}

// ── G11: Dialogue System ─────────────────────────────────────────

TEST_CASE("NF::DialogueCondition Always evaluates true", "[g11]") {
    NF::DialogueCondition c;
    c.type = NF::DialogueConditionType::Always;
    REQUIRE(c.evaluate(0.f, 0, false, false) == true);
}

TEST_CASE("NF::DialogueCondition HasReputation", "[g11]") {
    NF::DialogueCondition c;
    c.type = NF::DialogueConditionType::HasReputation;
    c.minReputation = 50.f;
    REQUIRE(c.evaluate(49.f, 0, false, false) == false);
    REQUIRE(c.evaluate(50.f, 0, false, false) == true);
    REQUIRE(c.evaluate(100.f, 0, false, false) == true);
}

TEST_CASE("NF::DialogueCondition HasItem", "[g11]") {
    NF::DialogueCondition c;
    c.type = NF::DialogueConditionType::HasItem;
    c.itemAmount = 3;
    REQUIRE(c.evaluate(0.f, 2, false, false) == false);
    REQUIRE(c.evaluate(0.f, 3, false, false) == true);
}

TEST_CASE("NF::DialogueCondition MissionActive and MissionComplete", "[g11]") {
    NF::DialogueCondition ca;
    ca.type = NF::DialogueConditionType::MissionActive;
    REQUIRE(ca.evaluate(0.f, 0, false, false) == false);
    REQUIRE(ca.evaluate(0.f, 0, true, false) == true);

    NF::DialogueCondition cc;
    cc.type = NF::DialogueConditionType::MissionComplete;
    REQUIRE(cc.evaluate(0.f, 0, false, false) == false);
    REQUIRE(cc.evaluate(0.f, 0, false, true) == true);
}

TEST_CASE("NF::DialogueGraph addNode and getNode", "[g11]") {
    NF::DialogueGraph graph;
    graph.setStartNodeId(0);

    NF::DialogueNode node;
    node.nodeId = 0;
    node.speakerName = "Merchant";
    node.text = "Welcome, traveler.";
    graph.addNode(node);

    REQUIRE(graph.nodeCount() == 1);
    const NF::DialogueNode* n = graph.getNode(0);
    REQUIRE(n != nullptr);
    REQUIRE(n->speakerName == "Merchant");
    REQUIRE(graph.getNode(99) == nullptr);
}

TEST_CASE("NF::DialogueRunner traversal", "[g11]") {
    NF::DialogueGraph graph;
    graph.setStartNodeId(0);

    NF::DialogueNode n0;
    n0.nodeId = 0;
    n0.speakerName = "Guard";
    n0.text = "Halt! Who goes there?";
    NF::DialogueOption opt0;
    opt0.text = "A friend.";
    opt0.nextNodeId = 1;
    n0.options.push_back(opt0);
    graph.addNode(n0);

    NF::DialogueNode n1;
    n1.nodeId = 1;
    n1.speakerName = "Guard";
    n1.text = "Pass, friend.";
    NF::DialogueOption opt1;
    opt1.text = "Thank you.";
    opt1.nextNodeId = -1;
    n1.options.push_back(opt1);
    graph.addNode(n1);

    NF::DialogueRunner runner;
    runner.init(&graph);
    REQUIRE(runner.isComplete() == false);
    REQUIRE(runner.currentNode()->text == "Halt! Who goes there?");

    runner.selectOption(0);
    REQUIRE(runner.currentNodeId() == 1);
    REQUIRE(runner.currentNode()->text == "Pass, friend.");

    runner.selectOption(0);
    REQUIRE(runner.isComplete() == true);
}

TEST_CASE("NF::DialogueRunner selectOption with effect", "[g11]") {
    NF::DialogueGraph graph;
    graph.setStartNodeId(0);

    NF::DialogueNode n0;
    n0.nodeId = 0;
    n0.speakerName = "Broker";
    n0.text = "Deal?";
    NF::DialogueOption opt;
    opt.text = "Yes.";
    opt.nextNodeId = -1;
    opt.effect.reputationFactionId = NF::StringID("traders");
    opt.effect.reputationDelta = 10.f;
    n0.options.push_back(opt);
    graph.addNode(n0);

    NF::DialogueRunner runner;
    runner.init(&graph);
    const NF::DialogueEffect* fx = runner.selectOption(0);
    REQUIRE(fx != nullptr);
    REQUIRE(fx->reputationFactionId == NF::StringID("traders"));
    REQUIRE(fx->reputationDelta == Catch::Approx(10.f));
    REQUIRE(runner.isComplete() == true);
}

TEST_CASE("NF::DialogueRunner invalid option returns nullptr", "[g11]") {
    NF::DialogueGraph graph;
    graph.setStartNodeId(0);
    NF::DialogueNode n0;
    n0.nodeId = 0;
    n0.text = "Hi.";
    graph.addNode(n0);

    NF::DialogueRunner runner;
    runner.init(&graph);
    REQUIRE(runner.selectOption(5) == nullptr);
}

// ── G12: Save/Load System ─────────────────────────────────────────

TEST_CASE("SaveSystem init creates empty slots", "[Game][SaveSystem]") {
    NF::SaveSystem sys;
    sys.init();
    REQUIRE(sys.usedSlotCount() == 0);
    auto slots = sys.listSlots();
    REQUIRE((int)slots.size() == NF::SaveSystem::kMaxSlots);
    for (const auto& s : slots) REQUIRE(s.isEmpty);
}

TEST_CASE("SaveSystem saveGame and loadGame round-trip", "[Game][SaveSystem]") {
    NF::SaveSystem sys;
    sys.init();

    NF::SaveData data;
    data.playerHealth    = 75.f;
    data.playerEnergy    = 50.f;
    data.playerPosition  = {10.f, 2.f, -5.f};
    data.currentSector   = "Sector7";
    data.inventory["RawIron"] = 3;
    data.activeMissionIds.push_back("mission_01");
    data.reputation["pirates"] = -25.f;
    data.playtimeSeconds = 123.f;

    REQUIRE(sys.saveGame(1, data, "Save1"));
    REQUIRE_FALSE(sys.slot(1).isEmpty);
    REQUIRE(sys.slot(1).name == "Save1");
    REQUIRE(sys.usedSlotCount() == 1);

    const NF::SaveData* loaded = sys.loadGame(1);
    REQUIRE(loaded != nullptr);
    REQUIRE(loaded->playerHealth  == Catch::Approx(75.f));
    REQUIRE(loaded->playerEnergy  == Catch::Approx(50.f));
    REQUIRE(loaded->currentSector == "Sector7");
    REQUIRE(loaded->inventory.at("RawIron") == 3);
    REQUIRE(loaded->activeMissionIds.size() == 1);
    REQUIRE(loaded->activeMissionIds[0] == "mission_01");
    REQUIRE(loaded->reputation.at("pirates") == Catch::Approx(-25.f));
}

TEST_CASE("SaveSystem deleteSlot clears slot", "[Game][SaveSystem]") {
    NF::SaveSystem sys;
    sys.init();

    NF::SaveData d;
    d.playerHealth = 100.f;
    sys.saveGame(2, d, "SlotTwo");
    REQUIRE_FALSE(sys.slot(2).isEmpty);

    REQUIRE(sys.deleteSlot(2));
    REQUIRE(sys.slot(2).isEmpty);
    REQUIRE(sys.usedSlotCount() == 0);
}

TEST_CASE("SaveSystem loadGame returns nullptr for empty slot", "[Game][SaveSystem]") {
    NF::SaveSystem sys;
    sys.init();
    REQUIRE(sys.loadGame(3) == nullptr);
}

TEST_CASE("SaveSystem autoSave triggers after interval", "[Game][SaveSystem]") {
    NF::SaveSystem sys;
    sys.init();
    sys.enableAutoSave(true);
    sys.setAutoSaveInterval(10.f);  // 10 seconds for test

    NF::SaveData d;
    d.playerHealth = 80.f;

    // Tick just under the interval — no auto-save yet
    sys.tickAutoSave(9.f, d);
    REQUIRE(sys.slot(NF::SaveSystem::kAutoSaveSlot).isEmpty);

    // Tick past the interval — auto-save fires
    sys.tickAutoSave(2.f, d);
    REQUIRE_FALSE(sys.slot(NF::SaveSystem::kAutoSaveSlot).isEmpty);
    REQUIRE(sys.slot(NF::SaveSystem::kAutoSaveSlot).name == "AutoSave");
}

TEST_CASE("GameSaveSerializer toJson fromJson round-trip", "[Game][SaveSystem]") {
    NF::SaveData original;
    original.playerHealth    = 42.f;
    original.currentSector   = "AlphaZone";
    original.inventory["Circuits"] = 7;
    original.completedMissionIds.push_back("m_done");

    NF::JsonValue j = NF::GameSaveSerializer::toJson(original);
    NF::SaveData  rt = NF::GameSaveSerializer::fromJson(j);

    REQUIRE(rt.playerHealth == Catch::Approx(42.f));
    REQUIRE(rt.currentSector == "AlphaZone");
    REQUIRE(rt.inventory.at("Circuits") == 7);
    REQUIRE(rt.completedMissionIds.size() == 1);
    REQUIRE(rt.completedMissionIds[0] == "m_done");
}

// ── G13: World Events System ──────────────────────────────────────

TEST_CASE("WorldEventSystem spawn and query active events", "[Game][WorldEvents]") {
    NF::WorldEventSystem wes;
    wes.init();

    int id = wes.spawnEvent(NF::WorldEventType::PirateRaid, "SectorAlpha", 60.f, 0.8f, "Raiders!");
    REQUIRE(id > 0);
    REQUIRE(wes.activeEventCount() == 1);

    auto active = wes.getActiveEvents();
    REQUIRE(active.size() == 1);
    REQUIRE(active[0].type == NF::WorldEventType::PirateRaid);
    REQUIRE(active[0].sectorId == "SectorAlpha");
}

TEST_CASE("WorldEventSystem endEvent marks event inactive", "[Game][WorldEvents]") {
    NF::WorldEventSystem wes;
    wes.init();

    int id = wes.spawnEvent(NF::WorldEventType::TradeOpportunity, "Bazaar", 120.f, 0.5f, "Sale!");
    REQUIRE(wes.activeEventCount() == 1);

    REQUIRE(wes.endEvent(id));
    REQUIRE(wes.activeEventCount() == 0);
}

TEST_CASE("WorldEventSystem tick expires events by duration", "[Game][WorldEvents]") {
    NF::WorldEventSystem wes;
    wes.init();

    wes.spawnEvent(NF::WorldEventType::AsteroidStorm, "Belt", 5.f, 0.3f, "Storm!");
    REQUIRE(wes.activeEventCount() == 1);

    wes.tick(3.f);
    REQUIRE(wes.activeEventCount() == 1);  // not yet expired

    wes.tick(3.f);  // total 6s > 5s duration
    REQUIRE(wes.activeEventCount() == 0);
}

TEST_CASE("WorldEventSystem getEventsInSector filters correctly", "[Game][WorldEvents]") {
    NF::WorldEventSystem wes;
    wes.init();

    wes.spawnEvent(NF::WorldEventType::Plague,  "SectorA", 100.f, 0.4f, "Illness");
    wes.spawnEvent(NF::WorldEventType::FactionWar, "SectorB", 100.f, 0.9f, "War");

    auto sectorA = wes.getEventsInSector("SectorA");
    REQUIRE(sectorA.size() == 1);
    REQUIRE(sectorA[0].type == NF::WorldEventType::Plague);

    auto sectorB = wes.getEventsInSector("SectorB");
    REQUIRE(sectorB.size() == 1);
    REQUIRE(sectorB[0].type == NF::WorldEventType::FactionWar);

    REQUIRE(wes.getEventsInSector("SectorC").empty());
}

TEST_CASE("WorldEventType name conversion covers all types", "[Game][WorldEvents]") {
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::AsteroidStorm)    == "AsteroidStorm");
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::PirateRaid)       == "PirateRaid");
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::TechDiscovery)    == "TechDiscovery");
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::FactionWar)       == "FactionWar");
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::TradeOpportunity) == "TradeOpportunity");
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::Plague)           == "Plague");
    REQUIRE(NF::worldEventTypeName(NF::WorldEventType::CelestialAnomaly) == "CelestialAnomaly");
}

TEST_CASE("WorldEvent effect severity scales modifiers", "[Game][WorldEvents]") {
    NF::WorldEventSystem wes;
    wes.init();

    // PirateRaid at high severity should increase danger modifier
    int id = wes.spawnEvent(NF::WorldEventType::PirateRaid, "S", 60.f, 1.0f, "desc");
    const NF::WorldEvent* ev = wes.findEvent(id);
    REQUIRE(ev != nullptr);
    REQUIRE(ev->effect.dangerModifier > 1.f);
    REQUIRE(ev->effect.reputationChange < 0.f);

    // TradeOpportunity should reduce prices
    int id2 = wes.spawnEvent(NF::WorldEventType::TradeOpportunity, "S2", 60.f, 1.0f, "desc");
    const NF::WorldEvent* ev2 = wes.findEvent(id2);
    REQUIRE(ev2 != nullptr);
    REQUIRE(ev2->effect.priceModifier < 1.f);
}

TEST_CASE("WorldEvent remainingTime decreases on tick", "[Game][WorldEvents]") {
    NF::WorldEventSystem wes;
    wes.init();

    int id = wes.spawnEvent(NF::WorldEventType::CelestialAnomaly, "Void", 30.f, 0.5f, "Anomaly");
    wes.tick(10.f);
    const NF::WorldEvent* ev = wes.findEvent(id);
    REQUIRE(ev != nullptr);
    REQUIRE(ev->remainingTime() == Catch::Approx(20.f));
}

// ── G14: Tech Tree ───────────────────────────────────────────────

TEST_CASE("TechTree addNode and canResearch root node", "[Game][TechTree]") {
    NF::TechTree tree;

    NF::TechNode n;
    n.id = "basic_weapons";
    n.displayName = "Basic Weapons";
    n.category = NF::TechCategory::Weapons;
    n.tier = 1;
    n.cost = 50;
    tree.addNode(n);

    // Root node (no prereqs) should be researchable immediately
    REQUIRE(tree.canResearch("basic_weapons"));
    REQUIRE(tree.nodeCount() == 1);
    REQUIRE(tree.researchedCount() == 0);
}

TEST_CASE("TechTree unlock node and prerequisite chain", "[Game][TechTree]") {
    NF::TechTree tree;

    NF::TechNode root;
    root.id = "shields_t1";
    root.tier = 1;
    tree.addNode(root);

    NF::TechNode child;
    child.id = "shields_t2";
    child.tier = 2;
    child.prerequisites.push_back("shields_t1");
    tree.addNode(child);

    // Child not researchable yet
    REQUIRE_FALSE(tree.canResearch("shields_t2"));

    // Unlock root
    REQUIRE(tree.unlock("shields_t1"));
    REQUIRE(tree.isUnlocked("shields_t1"));
    REQUIRE(tree.researchedCount() == 1);

    // Now child is researchable
    REQUIRE(tree.canResearch("shields_t2"));
    REQUIRE(tree.unlock("shields_t2"));
    REQUIRE(tree.isUnlocked("shields_t2"));
    REQUIRE(tree.researchedCount() == 2);
}

TEST_CASE("TechTree unlock already-researched node returns false", "[Game][TechTree]") {
    NF::TechTree tree;

    NF::TechNode n;
    n.id = "prop_basic";
    tree.addNode(n);

    REQUIRE(tree.unlock("prop_basic"));
    REQUIRE_FALSE(tree.unlock("prop_basic"));  // already researched
}

TEST_CASE("TechTree getAvailable lists only researchable nodes", "[Game][TechTree]") {
    NF::TechTree tree;

    NF::TechNode a;
    a.id = "a";
    tree.addNode(a);

    NF::TechNode b;
    b.id = "b";
    b.prerequisites.push_back("a");
    tree.addNode(b);

    auto avail = tree.getAvailable();
    REQUIRE(avail.size() == 1);
    REQUIRE(avail[0] == "a");

    tree.unlock("a");
    avail = tree.getAvailable();
    REQUIRE(avail.size() == 1);
    REQUIRE(avail[0] == "b");
}

TEST_CASE("TechTree computeBonuses aggregates researched nodes", "[Game][TechTree]") {
    NF::TechTree tree;

    NF::TechNode n1;
    n1.id = "dmg1";
    n1.damageBonus = 5.f;
    tree.addNode(n1);

    NF::TechNode n2;
    n2.id = "dmg2";
    n2.damageBonus = 3.f;
    n2.miningBonus = 2.f;
    tree.addNode(n2);

    // Nothing researched yet
    auto b = tree.computeBonuses();
    REQUIRE(b.damage == Catch::Approx(0.f));

    tree.unlock("dmg1");
    tree.unlock("dmg2");

    b = tree.computeBonuses();
    REQUIRE(b.damage == Catch::Approx(8.f));
    REQUIRE(b.mining == Catch::Approx(2.f));
}

TEST_CASE("TechTree getByTier filters correctly", "[Game][TechTree]") {
    NF::TechTree tree;

    NF::TechNode t1a; t1a.id = "t1a"; t1a.tier = 1; tree.addNode(t1a);
    NF::TechNode t1b; t1b.id = "t1b"; t1b.tier = 1; tree.addNode(t1b);
    NF::TechNode t2a; t2a.id = "t2a"; t2a.tier = 2; tree.addNode(t2a);

    REQUIRE(tree.getByTier(1).size() == 2);
    REQUIRE(tree.getByTier(2).size() == 1);
    REQUIRE(tree.getByTier(3).empty());
}

TEST_CASE("TechCategory name round-trip", "[Game][TechTree]") {
    REQUIRE(NF::techCategoryName(NF::TechCategory::Weapons)      == "Weapons");
    REQUIRE(NF::techCategoryName(NF::TechCategory::Shields)      == "Shields");
    REQUIRE(NF::techCategoryName(NF::TechCategory::Propulsion)   == "Propulsion");
    REQUIRE(NF::techCategoryName(NF::TechCategory::Mining)       == "Mining");
    REQUIRE(NF::techCategoryName(NF::TechCategory::Construction) == "Construction");
    REQUIRE(NF::techCategoryName(NF::TechCategory::Biology)      == "Biology");
    REQUIRE(NF::techCategoryName(NF::TechCategory::Computing)    == "Computing");
}

// ── G15: Player Progression ───────────────────────────────────────

TEST_CASE("PlayerLevel init starts at given level", "[Game][Progression]") {
    NF::PlayerLevel pl;
    pl.init(1);
    REQUIRE(pl.currentLevel() == 1);
    REQUIRE(pl.xpThisLevel() == 0);
    REQUIRE_FALSE(pl.isMaxLevel());
}

TEST_CASE("PlayerLevel addXP causes level up", "[Game][Progression]") {
    NF::PlayerLevel pl;
    pl.init(1);

    int needed = pl.xpToNextLevel();
    REQUIRE(needed > 0);

    int gained = pl.addXP(needed);
    REQUIRE(gained == 1);
    REQUIRE(pl.currentLevel() == 2);
}

TEST_CASE("PlayerLevel progressToNextLevel returns 0-1 fraction", "[Game][Progression]") {
    NF::PlayerLevel pl;
    pl.init(1);

    int needed = pl.xpToNextLevel();
    pl.addXP(needed / 2);

    float progress = pl.progressToNextLevel();
    REQUIRE(progress > 0.f);
    REQUIRE(progress < 1.f);
}

TEST_CASE("PlayerLevel caps at max level", "[Game][Progression]") {
    NF::PlayerLevel pl;
    pl.init(NF::PlayerLevel::kMaxLevel);
    REQUIRE(pl.isMaxLevel());
    REQUIRE(pl.xpToNextLevel() == 0);

    int gained = pl.addXP(10000);
    REQUIRE(gained == 0);  // no more levels
}

TEST_CASE("SkillTree unlockSkill respects level requirement", "[Game][Progression]") {
    NF::SkillTree st;

    NF::SkillNode sk;
    sk.id = "heavy_mining";
    sk.requiredLevel = 5;
    sk.pointCost = 1;
    sk.miningBonus = 10.f;
    st.addSkill(sk);

    int points = 5;
    // Level too low
    REQUIRE_FALSE(st.unlockSkill("heavy_mining", 3, points));
    REQUIRE(points == 5);

    // Level sufficient
    REQUIRE(st.unlockSkill("heavy_mining", 5, points));
    REQUIRE(points == 4);
    REQUIRE(st.isUnlocked("heavy_mining"));
}

TEST_CASE("SkillTree computeBonuses aggregates unlocked skills", "[Game][Progression]") {
    NF::SkillTree st;

    NF::SkillNode s1; s1.id = "hp1"; s1.healthBonus = 20.f; s1.requiredLevel = 1; s1.pointCost = 1;
    NF::SkillNode s2; s2.id = "hp2"; s2.healthBonus = 15.f; s2.requiredLevel = 1; s2.pointCost = 1;
    st.addSkill(s1);
    st.addSkill(s2);

    int pts = 10;
    st.unlockSkill("hp1", 1, pts);
    st.unlockSkill("hp2", 1, pts);

    auto b = st.computeBonuses();
    REQUIRE(b.health == Catch::Approx(35.f));
}

TEST_CASE("ProgressionSystem awardXP grants skill points on level-up", "[Game][Progression]") {
    NF::ProgressionSystem prog;
    prog.init(1);

    REQUIRE(prog.skillPoints() == 0);

    int needed = prog.level().xpToNextLevel();
    prog.awardXP(needed);

    REQUIRE(prog.level().currentLevel() == 2);
    REQUIRE(prog.skillPoints() == 1);
}

TEST_CASE("ProgressionSystem spendSkillPoint unlocks skill", "[Game][Progression]") {
    NF::ProgressionSystem prog;
    prog.init(1);

    NF::SkillNode sk;
    sk.id = "fast_drill";
    sk.requiredLevel = 1;
    sk.pointCost = 1;
    sk.miningBonus = 5.f;
    prog.skillTree().addSkill(sk);

    // No skill points yet
    REQUIRE_FALSE(prog.spendSkillPoint("fast_drill"));

    // Level up to get a skill point
    prog.awardXP(prog.level().xpToNextLevel());
    REQUIRE(prog.skillPoints() == 1);

    REQUIRE(prog.spendSkillPoint("fast_drill"));
    REQUIRE(prog.skillPoints() == 0);
    REQUIRE(prog.bonuses().mining == Catch::Approx(5.f));
}

// ── G16: Crafting System ─────────────────────────────────────────

TEST_CASE("CraftingCategory names", "[Game][Crafting]") {
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Weapon)     == "Weapon");
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Armor)      == "Armor");
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Tool)       == "Tool");
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Component)  == "Component");
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Consumable) == "Consumable");
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Fuel)       == "Fuel");
    REQUIRE(NF::craftingCategoryName(NF::CraftingCategory::Decoration) == "Decoration");
}

TEST_CASE("CraftingQueue enqueue and tick to completion", "[Game][Crafting]") {
    NF::CraftingQueue q;
    REQUIRE(q.isEmpty());

    q.enqueue("iron_plate", 3.f);
    REQUIRE(q.pendingCount() == 1);

    q.tick(2.f);
    auto done = q.collectCompleted();
    REQUIRE(done.empty());

    q.tick(2.f);
    done = q.collectCompleted();
    REQUIRE(done.size() == 1);
    REQUIRE(done[0] == "iron_plate");
    REQUIRE(q.isEmpty());
}

TEST_CASE("CraftingQueue processes jobs in FIFO order", "[Game][Crafting]") {
    NF::CraftingQueue q;
    q.enqueue("a", 1.f);
    q.enqueue("b", 1.f);

    q.tick(1.5f);
    auto done = q.collectCompleted();
    REQUIRE(done.size() == 1);
    REQUIRE(done[0] == "a");

    q.tick(1.5f);
    done = q.collectCompleted();
    REQUIRE(done.size() == 1);
    REQUIRE(done[0] == "b");
}

TEST_CASE("CraftingSystem registerRecipe and canCraft", "[Game][Crafting]") {
    NF::CraftingSystem cs;

    NF::CraftingRecipe recipe;
    recipe.recipeId = "steel_bar";
    recipe.outputItemId = "steel";
    recipe.category = NF::CraftingCategory::Component;
    recipe.craftTime = 4.f;
    recipe.ingredients.push_back({"iron", 2});
    recipe.ingredients.push_back({"coal", 1});
    cs.registerRecipe(recipe);

    REQUIRE(cs.recipeCount() == 1);
    REQUIRE(cs.findRecipe("steel_bar") != nullptr);

    std::map<std::string, int> inv = {{"iron", 5}, {"coal", 3}};
    REQUIRE(cs.canCraft("steel_bar", inv));

    std::map<std::string, int> noCoal = {{"iron", 5}};
    REQUIRE_FALSE(cs.canCraft("steel_bar", noCoal));
}

TEST_CASE("CraftingSystem enqueue deducts ingredients", "[Game][Crafting]") {
    NF::CraftingSystem cs;

    NF::CraftingRecipe recipe;
    recipe.recipeId = "circuit";
    recipe.craftTime = 2.f;
    recipe.ingredients.push_back({"copper", 3});
    cs.registerRecipe(recipe);

    std::map<std::string, int> inv = {{"copper", 5}};
    REQUIRE(cs.enqueue("circuit", inv));
    REQUIRE(inv["copper"] == 2);

    cs.tick(3.f);
    auto done = cs.collectCompleted();
    REQUIRE(done.size() == 1);
    REQUIRE(done[0] == "circuit");
}

TEST_CASE("CraftingSystem canCraft respects required level", "[Game][Crafting]") {
    NF::CraftingSystem cs;

    NF::CraftingRecipe recipe;
    recipe.recipeId = "advanced_laser";
    recipe.requiredLevel = 10;
    recipe.craftTime = 1.f;
    cs.registerRecipe(recipe);

    std::map<std::string, int> inv;
    REQUIRE_FALSE(cs.canCraft("advanced_laser", inv, 5));
    REQUIRE(cs.canCraft("advanced_laser", inv, 10));
}

TEST_CASE("CraftingSystem recipesByCategory filters", "[Game][Crafting]") {
    NF::CraftingSystem cs;

    NF::CraftingRecipe r1; r1.recipeId = "sword"; r1.category = NF::CraftingCategory::Weapon;
    NF::CraftingRecipe r2; r2.recipeId = "potion"; r2.category = NF::CraftingCategory::Consumable;
    NF::CraftingRecipe r3; r3.recipeId = "axe"; r3.category = NF::CraftingCategory::Weapon;
    cs.registerRecipe(r1);
    cs.registerRecipe(r2);
    cs.registerRecipe(r3);

    auto weapons = cs.recipesByCategory(NF::CraftingCategory::Weapon);
    REQUIRE(weapons.size() == 2);
    REQUIRE(cs.recipesByCategory(NF::CraftingCategory::Consumable).size() == 1);
}

TEST_CASE("CraftingJob progress tracks correctly", "[Game][Crafting]") {
    NF::CraftingJob job;
    job.recipeId = "test";
    job.duration = 10.f;

    REQUIRE(job.progress() == Catch::Approx(0.f));
    job.tick(5.f);
    REQUIRE(job.progress() == Catch::Approx(0.5f));
    job.tick(5.f);
    REQUIRE(job.complete);
    REQUIRE(job.progress() == Catch::Approx(1.f));
}

// ── G17: Inventory & Equipment ───────────────────────────────────

TEST_CASE("ItemRarity and ItemSlot names", "[Game][Inventory]") {
    REQUIRE(NF::itemRarityName(NF::ItemRarity::Common)    == "Common");
    REQUIRE(NF::itemRarityName(NF::ItemRarity::Uncommon)  == "Uncommon");
    REQUIRE(NF::itemRarityName(NF::ItemRarity::Rare)      == "Rare");
    REQUIRE(NF::itemRarityName(NF::ItemRarity::Epic)      == "Epic");
    REQUIRE(NF::itemRarityName(NF::ItemRarity::Legendary) == "Legendary");

    REQUIRE(NF::itemSlotName(NF::ItemSlot::Head)   == "Head");
    REQUIRE(NF::itemSlotName(NF::ItemSlot::Weapon) == "Weapon");
    REQUIRE(NF::itemSlotName(NF::ItemSlot::Shield) == "Shield");
}

TEST_CASE("Inventory addItem and stacking", "[Game][Inventory]") {
    NF::PlayerInventory inv(10);
    REQUIRE(inv.freeSlots() == 10);

    NF::Item iron;
    iron.id = "iron_ore";
    iron.count = 5;
    iron.stackMax = 20;

    int leftover = inv.addItem(iron);
    REQUIRE(leftover == 0);
    REQUIRE(inv.usedSlots() == 1);
    REQUIRE(inv.countItem("iron_ore") == 5);

    // Stack onto same slot
    iron.count = 10;
    leftover = inv.addItem(iron);
    REQUIRE(leftover == 0);
    REQUIRE(inv.usedSlots() == 1);
    REQUIRE(inv.countItem("iron_ore") == 15);
}

TEST_CASE("Inventory removeItem", "[Game][Inventory]") {
    NF::PlayerInventory inv(10);

    NF::Item item;
    item.id = "gem";
    item.count = 8;
    item.stackMax = 99;
    inv.addItem(item);

    int removed = inv.removeItem("gem", 3);
    REQUIRE(removed == 3);
    REQUIRE(inv.countItem("gem") == 5);

    removed = inv.removeItem("gem", 10);
    REQUIRE(removed == 5);
    REQUIRE(inv.countItem("gem") == 0);
    REQUIRE(inv.usedSlots() == 0);
}

TEST_CASE("Inventory capacity limit", "[Game][Inventory]") {
    NF::PlayerInventory inv(2);

    NF::Item a; a.id = "a"; a.count = 1; a.stackMax = 1;
    NF::Item b; b.id = "b"; b.count = 1; b.stackMax = 1;
    NF::Item c; c.id = "c"; c.count = 1; c.stackMax = 1;

    REQUIRE(inv.addItem(a) == 0);
    REQUIRE(inv.addItem(b) == 0);
    REQUIRE(inv.isFull());

    int left = inv.addItem(c);
    REQUIRE(left == 1);   // couldn't fit
}

TEST_CASE("Inventory toCountMap", "[Game][Inventory]") {
    NF::PlayerInventory inv(10);

    NF::Item iron; iron.id = "iron"; iron.count = 5; iron.stackMax = 99;
    NF::Item coal; coal.id = "coal"; coal.count = 3; coal.stackMax = 99;
    inv.addItem(iron);
    inv.addItem(coal);

    auto m = inv.toCountMap();
    REQUIRE(m["iron"] == 5);
    REQUIRE(m["coal"] == 3);
}

TEST_CASE("EquipmentLoadout equip and unequip", "[Game][Equipment]") {
    NF::EquipmentLoadout eq;

    NF::Item helmet;
    helmet.id = "iron_helm";
    helmet.slot = NF::ItemSlot::Head;
    helmet.armorBonus = 5.f;

    eq.equip(helmet);
    REQUIRE(eq.isSlotOccupied(NF::ItemSlot::Head));
    REQUIRE(eq.equippedCount() == 1);

    auto prev = eq.unequip(NF::ItemSlot::Head);
    REQUIRE(prev == "iron_helm");
    REQUIRE_FALSE(eq.isSlotOccupied(NF::ItemSlot::Head));
}

TEST_CASE("EquipmentLoadout replaces existing and computes bonuses", "[Game][Equipment]") {
    NF::EquipmentLoadout eq;

    NF::Item sword1;
    sword1.id = "rusty_sword";
    sword1.slot = NF::ItemSlot::Weapon;
    sword1.damageBonus = 3.f;

    NF::Item sword2;
    sword2.id = "steel_sword";
    sword2.slot = NF::ItemSlot::Weapon;
    sword2.damageBonus = 8.f;

    eq.equip(sword1);
    auto prev = eq.equip(sword2);
    REQUIRE(prev == "rusty_sword");
    REQUIRE(eq.equippedCount() == 1);

    auto b = eq.computeBonuses();
    REQUIRE(b.damage == Catch::Approx(8.f));
}

TEST_CASE("EquipmentLoadout computeBonuses aggregates all slots", "[Game][Equipment]") {
    NF::EquipmentLoadout eq;

    NF::Item helm;  helm.id = "h";  helm.slot = NF::ItemSlot::Head;   helm.armorBonus = 5.f;
    NF::Item chest; chest.id = "c"; chest.slot = NF::ItemSlot::Chest;  chest.armorBonus = 10.f; chest.healthBonus = 20.f;
    NF::Item wpn;   wpn.id = "w";   wpn.slot = NF::ItemSlot::Weapon;  wpn.damageBonus = 7.f;

    eq.equip(helm);
    eq.equip(chest);
    eq.equip(wpn);

    auto b = eq.computeBonuses();
    REQUIRE(b.armor  == Catch::Approx(15.f));
    REQUIRE(b.health == Catch::Approx(20.f));
    REQUIRE(b.damage == Catch::Approx(7.f));
    REQUIRE(eq.equippedCount() == 3);
}

// ── G18 Status Effects ───────────────────────────────────────────

TEST_CASE("StatusEffectType names", "[Game][G18][StatusEffects]") {
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Poison))     == "Poison");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Burn))       == "Burn");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Freeze))     == "Freeze");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Radiation))  == "Radiation");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Bleed))      == "Bleed");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Stun))       == "Stun");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Blind))      == "Blind");
    REQUIRE(std::string(NF::statusEffectTypeName(NF::StatusEffectType::Overcharge)) == "Overcharge");
}

TEST_CASE("StatusEffect tick and expiry", "[Game][G18][StatusEffects]") {
    NF::StatusEffect e;
    e.type = NF::StatusEffectType::Poison;
    e.damage = 5.f; e.duration = 3.f; e.tickRate = 1.f;
    REQUIRE_FALSE(e.isExpired());
    float dmg = e.tick(1.0f);
    REQUIRE(dmg == Catch::Approx(5.f));
    e.tick(1.0f); e.tick(1.0f);
    REQUIRE(e.isExpired());
}

TEST_CASE("AilmentStack apply and has", "[Game][G18][StatusEffects]") {
    NF::AilmentStack stack;
    NF::StatusEffect burn;
    burn.type = NF::StatusEffectType::Burn;
    burn.damage = 3.f; burn.duration = 2.f; burn.tickRate = 1.f;
    stack.apply(burn);
    REQUIRE(stack.has(NF::StatusEffectType::Burn));
    REQUIRE_FALSE(stack.has(NF::StatusEffectType::Freeze));
    REQUIRE(stack.count() == 1);
}

TEST_CASE("AilmentStack removes expired", "[Game][G18][StatusEffects]") {
    NF::AilmentStack stack;
    NF::StatusEffect e; e.type=NF::StatusEffectType::Stun; e.duration=0.5f; e.tickRate=1.f;
    stack.apply(e);
    stack.tick(1.0f); // expires
    REQUIRE(stack.count() == 0);
}

TEST_CASE("AilmentStack remove by type", "[Game][G18][StatusEffects]") {
    NF::AilmentStack stack;
    NF::StatusEffect e; e.type=NF::StatusEffectType::Blind; e.duration=5.f; e.tickRate=1.f;
    stack.apply(e);
    REQUIRE(stack.has(NF::StatusEffectType::Blind));
    stack.remove(NF::StatusEffectType::Blind);
    REQUIRE_FALSE(stack.has(NF::StatusEffectType::Blind));
}

TEST_CASE("StatusEffectSystem multi-entity", "[Game][G18][StatusEffects]") {
    NF::StatusEffectSystem sys;
    NF::StatusEffect bleed; bleed.type=NF::StatusEffectType::Bleed;
    bleed.damage=4.f; bleed.duration=2.f; bleed.tickRate=0.5f;
    sys.applyEffect(1, bleed);
    sys.applyEffect(2, bleed);
    REQUIRE(sys.entityCount() == 2);
    REQUIRE(sys.hasEffect(1, NF::StatusEffectType::Bleed));
    sys.clearEntity(1);
    REQUIRE_FALSE(sys.hasEffect(1, NF::StatusEffectType::Bleed));
    REQUIRE(sys.entityCount() == 1);
}

TEST_CASE("StatusEffectSystem tick returns damage map", "[Game][G18][StatusEffects]") {
    NF::StatusEffectSystem sys;
    NF::StatusEffect e; e.type=NF::StatusEffectType::Burn;
    e.damage=10.f; e.duration=5.f; e.tickRate=1.f;
    sys.applyEffect(42, e);
    auto dmgMap = sys.tick(1.0f);
    REQUIRE(dmgMap.count(42) == 1);
    REQUIRE(dmgMap[42] == Catch::Approx(10.f));
}

TEST_CASE("AilmentStack refresh on duplicate type", "[Game][G18][StatusEffects]") {
    NF::AilmentStack stack;
    NF::StatusEffect e; e.type=NF::StatusEffectType::Freeze;
    e.duration=2.f; e.tickRate=5.f; e.intensity=0.5f;
    stack.apply(e);
    NF::StatusEffect e2=e; e2.intensity=0.9f; e2.duration=4.f;
    stack.apply(e2);
    REQUIRE(stack.count() == 1); // refreshed, not doubled
}

// ── G19 Contracts & Bounties ─────────────────────────────────────

TEST_CASE("ContractType names", "[Game][G19][Contracts]") {
    REQUIRE(std::string(NF::contractTypeName(NF::ContractType::Delivery))      == "Delivery");
    REQUIRE(std::string(NF::contractTypeName(NF::ContractType::Assassination)) == "Assassination");
    REQUIRE(std::string(NF::contractTypeName(NF::ContractType::Escort))        == "Escort");
    REQUIRE(std::string(NF::contractTypeName(NF::ContractType::Salvage))       == "Salvage");
    REQUIRE(std::string(NF::contractTypeName(NF::ContractType::Patrol))        == "Patrol");
    REQUIRE(std::string(NF::contractTypeName(NF::ContractType::Mining))        == "Mining");
}

TEST_CASE("Contract lifecycle", "[Game][G19][Contracts]") {
    NF::Contract c;
    c.contractId="C001"; c.type=NF::ContractType::Delivery;
    c.creditsReward=5000; c.requiredLevel=3;
    REQUIRE(c.status == NF::ContractStatus::Available);
    c.accept();
    REQUIRE(c.status == NF::ContractStatus::Accepted);
    c.start();
    REQUIRE(c.status == NF::ContractStatus::InProgress);
    REQUIRE(c.isActive());
    c.complete();
    REQUIRE(c.status == NF::ContractStatus::Completed);
    REQUIRE_FALSE(c.isActive());
}

TEST_CASE("Contract expiry via tick", "[Game][G19][Contracts]") {
    NF::Contract c;
    c.contractId="C002"; c.timeLimit=2.f;
    c.accept(); c.start();
    c.tick(1.f); REQUIRE(c.status == NF::ContractStatus::InProgress);
    c.tick(1.5f); REQUIRE(c.status == NF::ContractStatus::Expired);
}

TEST_CASE("BountyTarget claim", "[Game][G19][Contracts]") {
    NF::BountyTarget b;
    b.targetId="pirate_boss"; b.creditsReward=12000;
    REQUIRE(b.isClaimable());
    b.claim();
    REQUIRE_FALSE(b.isClaimable());
    REQUIRE(b.claimed);
}

TEST_CASE("ContractBoard available contracts by level", "[Game][G19][Contracts]") {
    NF::ContractBoard board;
    NF::Contract c1; c1.contractId="A"; c1.requiredLevel=1;
    NF::Contract c2; c2.contractId="B"; c2.requiredLevel=10;
    board.addContract(c1); board.addContract(c2);
    auto avail = board.availableContracts(5);
    REQUIRE(avail.size() == 1);
    REQUIRE(avail[0]->contractId == "A");
}

TEST_CASE("ContractBoard bounty list", "[Game][G19][Contracts]") {
    NF::ContractBoard board;
    NF::BountyTarget b1; b1.targetId="T1";
    NF::BountyTarget b2; b2.targetId="T2"; b2.claimed=true;
    board.addBounty(b1); board.addBounty(b2);
    auto active = board.activeBounties();
    REQUIRE(active.size() == 1);
    REQUIRE(active[0]->targetId == "T1");
}

TEST_CASE("ContractBoard remove expired", "[Game][G19][Contracts]") {
    NF::ContractBoard board;
    NF::Contract c; c.contractId="X"; c.timeLimit=0.5f;
    c.accept(); c.start();
    board.addContract(c);
    board.tick(1.0f); // expires
    board.removeExpired();
    REQUIRE(board.contractCount() == 0);
}

// ── G20 Companion System ─────────────────────────────────────────

TEST_CASE("CompanionRole names", "[Game][G20][Companion]") {
    REQUIRE(std::string(NF::companionRoleName(NF::CompanionRole::Combat))   == "Combat");
    REQUIRE(std::string(NF::companionRoleName(NF::CompanionRole::Engineer)) == "Engineer");
    REQUIRE(std::string(NF::companionRoleName(NF::CompanionRole::Medic))    == "Medic");
    REQUIRE(std::string(NF::companionRoleName(NF::CompanionRole::Scout))    == "Scout");
    REQUIRE(std::string(NF::companionRoleName(NF::CompanionRole::Pilot))    == "Pilot");
    REQUIRE(std::string(NF::companionRoleName(NF::CompanionRole::Trader))   == "Trader");
}

TEST_CASE("Companion init and basic state", "[Game][G20][Companion]") {
    NF::Companion c;
    c.init("Zara", NF::CompanionRole::Medic);
    REQUIRE(c.name() == "Zara");
    REQUIRE(c.role() == NF::CompanionRole::Medic);
    REQUIRE(c.isAlive());
    REQUIRE(c.isActive());
    REQUIRE(c.health() == Catch::Approx(100.f));
}

TEST_CASE("Companion take damage and heal", "[Game][G20][Companion]") {
    NF::Companion c; c.init("Rex", NF::CompanionRole::Combat);
    c.takeDamage(30.f);
    REQUIRE(c.health() == Catch::Approx(70.f));
    c.heal(20.f);
    REQUIRE(c.health() == Catch::Approx(90.f));
}

TEST_CASE("Companion death and recall", "[Game][G20][Companion]") {
    NF::Companion c; c.init("Nox", NF::CompanionRole::Scout);
    c.takeDamage(200.f);
    REQUIRE_FALSE(c.isAlive());
    REQUIRE_FALSE(c.isActive());
    c.heal(50.f);
    c.recall();
    REQUIRE(c.isActive());
}

TEST_CASE("Companion ability cooldown", "[Game][G20][Companion]") {
    NF::Companion c; c.init("Ada", NF::CompanionRole::Engineer);
    NF::CompanionAbility ab; ab.name="Deploy Turret"; ab.cooldown=5.f;
    ab.cooldownElapsed = 5.f; // ready
    c.addAbility(ab);
    REQUIRE(c.abilityCount() == 1);
    auto* found = c.findAbility("Deploy Turret");
    REQUIRE(found != nullptr);
    REQUIRE(found->isReady());
    found->use();
    REQUIRE_FALSE(found->isReady());
    found->tick(5.0f);
    REQUIRE(found->isReady());
}

TEST_CASE("CompanionManager add and find", "[Game][G20][Companion]") {
    NF::CompanionManager mgr;
    NF::Companion c1; c1.init("Kira", NF::CompanionRole::Medic);
    NF::Companion c2; c2.init("Bolt", NF::CompanionRole::Combat);
    mgr.addCompanion(std::move(c1));
    mgr.addCompanion(std::move(c2));
    REQUIRE(mgr.companionCount() == 2);
    REQUIRE(mgr.activeCount() == 2);
    REQUIRE(mgr.findCompanion("Kira") != nullptr);
    REQUIRE(mgr.hasRole(NF::CompanionRole::Medic));
    REQUIRE_FALSE(mgr.hasRole(NF::CompanionRole::Pilot));
}

TEST_CASE("CompanionManager max capacity", "[Game][G20][Companion]") {
    NF::CompanionManager mgr;
    for (int i = 0; i < 6; ++i) {
        NF::Companion c; c.init("C"+std::to_string(i), NF::CompanionRole::Combat);
        mgr.addCompanion(std::move(c));
    }
    REQUIRE(mgr.companionCount() == NF::CompanionManager::kMaxCompanions);
}

TEST_CASE("CompanionManager average morale", "[Game][G20][Companion]") {
    NF::CompanionManager mgr;
    NF::Companion c1; c1.init("A", NF::CompanionRole::Scout);
    c1.personality().morale = 0.8f;
    NF::Companion c2; c2.init("B", NF::CompanionRole::Pilot);
    c2.personality().morale = 0.4f;
    mgr.addCompanion(std::move(c1));
    mgr.addCompanion(std::move(c2));
    REQUIRE(mgr.averageMorale() == Catch::Approx(0.6f));
}

TEST_CASE("CompanionPersonality trust and loyalty", "[Game][G20][Companion]") {
    NF::CompanionPersonality p;
    REQUIRE_FALSE(p.isLoyal());
    p.gainTrust(30);
    REQUIRE_FALSE(p.isLoyal());
    p.gainTrust(25);
    REQUIRE(p.isLoyal());
    p.loseTrust(4);
    REQUIRE(p.isLoyal()); // still above threshold (55 - 4 = 51 >= 50)
}

TEST_CASE("CompanionManager remove companion", "[Game][G20][Companion]") {
    NF::CompanionManager mgr;
    NF::Companion c; c.init("Temp", NF::CompanionRole::Trader);
    mgr.addCompanion(std::move(c));
    REQUIRE(mgr.companionCount() == 1);
    mgr.removeCompanion("Temp");
    REQUIRE(mgr.companionCount() == 0);
}

// ── G21 Faction System Tests ──────────────────────────────────

TEST_CASE("FactionType names", "[Game][G21][Faction]") {
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Military))    == "Military");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Corporate))   == "Corporate");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Scientific))  == "Scientific");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Religious))   == "Religious");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Criminal))    == "Criminal");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Pirate))      == "Pirate");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Colonial))    == "Colonial");
    REQUIRE(std::string(NF::factionTypeName(NF::FactionType::Independent)) == "Independent");
    REQUIRE(static_cast<int>(NF::FactionType::Count) == 8);
}

TEST_CASE("FactionStanding names", "[Game][G21][Faction]") {
    REQUIRE(std::string(NF::factionStandingName(NF::FactionStanding::Hostile))    == "Hostile");
    REQUIRE(std::string(NF::factionStandingName(NF::FactionStanding::Unfriendly)) == "Unfriendly");
    REQUIRE(std::string(NF::factionStandingName(NF::FactionStanding::Neutral))    == "Neutral");
    REQUIRE(std::string(NF::factionStandingName(NF::FactionStanding::Friendly))   == "Friendly");
    REQUIRE(std::string(NF::factionStandingName(NF::FactionStanding::Allied))     == "Allied");
}

TEST_CASE("Faction init and basic state", "[Game][G21][Faction]") {
    NF::Faction f;
    f.init("fac_mil", "Iron Guard", NF::FactionType::Military);
    REQUIRE(f.factionId() == "fac_mil");
    REQUIRE(f.name() == "Iron Guard");
    REQUIRE(f.type() == NF::FactionType::Military);
    REQUIRE(f.influence() == Catch::Approx(0.5f));
    REQUIRE(f.wealth() == 0);
    REQUIRE(f.militaryPower() == 0);
    REQUIRE(f.territoryCount() == 0);
}

TEST_CASE("Faction territory management", "[Game][G21][Faction]") {
    NF::Faction f;
    f.init("fac1", "TestFac", NF::FactionType::Colonial);

    NF::FactionTerritory t1; t1.sectorId = "s1"; t1.sectorName = "Alpha"; t1.resourceOutput = 10;
    NF::FactionTerritory t2; t2.sectorId = "s2"; t2.sectorName = "Beta";  t2.resourceOutput = 20;
    f.addTerritory(t1);
    f.addTerritory(t2);
    REQUIRE(f.territoryCount() == 2);
    REQUIRE(f.totalResourceOutput() == 30);
    REQUIRE(f.findTerritory("s1") != nullptr);
    REQUIRE(f.findTerritory("s3") == nullptr);

    f.removeTerritory("s1");
    REQUIRE(f.territoryCount() == 1);
    REQUIRE(f.totalResourceOutput() == 20);
}

TEST_CASE("Faction wealth and influence", "[Game][G21][Faction]") {
    NF::Faction f;
    f.init("fac1", "Corp", NF::FactionType::Corporate);
    f.addWealth(100);
    REQUIRE(f.wealth() == 100);
    REQUIRE(f.spendWealth(40));
    REQUIRE(f.wealth() == 60);
    REQUIRE_FALSE(f.spendWealth(200));
    REQUIRE(f.wealth() == 60);

    f.adjustInfluence(0.3f);
    REQUIRE(f.influence() == Catch::Approx(0.8f));
    f.adjustInfluence(0.5f); // should clamp to 1.0
    REQUIRE(f.influence() == Catch::Approx(1.0f));
    f.adjustInfluence(-1.5f); // should clamp to 0.0
    REQUIRE(f.influence() == Catch::Approx(0.0f));
}

TEST_CASE("FactionTerritory erosion", "[Game][G21][Faction]") {
    NF::FactionTerritory t;
    t.sectorId = "s1";
    t.controlStrength = 0.5f;
    REQUIRE_FALSE(t.isLost());

    t.erode(0.3f);
    REQUIRE(t.controlStrength == Catch::Approx(0.2f));
    REQUIRE_FALSE(t.isLost());

    t.reinforce(0.6f);
    REQUIRE(t.controlStrength == Catch::Approx(0.8f));

    t.erode(1.0f); // clamp to 0
    REQUIRE(t.controlStrength == Catch::Approx(0.0f));
    REQUIRE(t.isLost());

    t.reinforce(2.0f); // clamp to 1
    REQUIRE(t.controlStrength == Catch::Approx(1.0f));
}

TEST_CASE("FactionRelation reputation and standing", "[Game][G21][Faction]") {
    NF::GameFactionRelation rel;
    rel.factionA = "a";
    rel.factionB = "b";
    REQUIRE(rel.standing == NF::FactionStanding::Neutral);
    REQUIRE(rel.reputation == 0);

    rel.improve(30);
    REQUIRE(rel.reputation == 30);
    REQUIRE(rel.standing == NF::FactionStanding::Friendly);

    rel.improve(50);
    REQUIRE(rel.reputation == 80);
    REQUIRE(rel.standing == NF::FactionStanding::Allied);

    rel.degrade(60);
    REQUIRE(rel.reputation == 20);
    REQUIRE(rel.standing == NF::FactionStanding::Neutral);

    rel.degrade(50);
    REQUIRE(rel.reputation == -30);
    REQUIRE(rel.standing == NF::FactionStanding::Unfriendly);

    rel.degrade(50);
    REQUIRE(rel.reputation == -80);
    REQUIRE(rel.standing == NF::FactionStanding::Hostile);
}

TEST_CASE("FactionRelation war and peace", "[Game][G21][Faction]") {
    NF::GameFactionRelation rel;
    rel.factionA = "x";
    rel.factionB = "y";

    rel.signTreaty("trade");
    REQUIRE(rel.hasTreaty);
    REQUIRE(rel.treatyType == "trade");

    rel.declareWar();
    REQUIRE(rel.atWar);
    REQUIRE(rel.standing == NF::FactionStanding::Hostile);
    REQUIRE_FALSE(rel.hasTreaty);
    REQUIRE(rel.treatyType.empty());

    rel.declarePeace();
    REQUIRE_FALSE(rel.atWar);
    REQUIRE(rel.standing == NF::FactionStanding::Unfriendly);

    rel.signTreaty("defense");
    REQUIRE(rel.hasTreaty);
    rel.breakTreaty();
    REQUIRE_FALSE(rel.hasTreaty);
}

TEST_CASE("FactionManager add and find", "[Game][G21][Faction]") {
    NF::GameFactionManager mgr;
    NF::Faction f1; f1.init("f1", "Alpha", NF::FactionType::Military);
    NF::Faction f2; f2.init("f2", "Beta",  NF::FactionType::Pirate);
    mgr.addFaction(std::move(f1));
    mgr.addFaction(std::move(f2));
    REQUIRE(mgr.factionCount() == 2);
    REQUIRE(mgr.findFaction("f1") != nullptr);
    REQUIRE(mgr.findFaction("f1")->name() == "Alpha");
    REQUIRE(mgr.findFaction("missing") == nullptr);

    mgr.removeFaction("f1");
    REQUIRE(mgr.factionCount() == 1);

    // max capacity
    NF::GameFactionManager mgr2;
    for (int i = 0; i < 20; ++i) {
        NF::Faction f; f.init("fac"+std::to_string(i), "F"+std::to_string(i), NF::FactionType::Independent);
        mgr2.addFaction(std::move(f));
    }
    REQUIRE(mgr2.factionCount() == static_cast<size_t>(NF::GameFactionManager::kMaxFactions));
}

TEST_CASE("FactionManager relations and alliances", "[Game][G21][Faction]") {
    NF::GameFactionManager mgr;
    NF::Faction f1; f1.init("a", "FacA", NF::FactionType::Military);
    NF::Faction f2; f2.init("b", "FacB", NF::FactionType::Corporate);
    NF::Faction f3; f3.init("c", "FacC", NF::FactionType::Pirate);
    mgr.addFaction(std::move(f1));
    mgr.addFaction(std::move(f2));
    mgr.addFaction(std::move(f3));

    NF::GameFactionRelation rel;
    rel.standing = NF::FactionStanding::Allied;
    mgr.setRelation("a", "b", rel);

    NF::GameFactionRelation rel2;
    rel2.standing = NF::FactionStanding::Hostile;
    rel2.atWar = true;
    mgr.setRelation("a", "c", rel2);

    auto* r = mgr.getRelation("a", "b");
    REQUIRE(r != nullptr);
    REQUIRE(r->standing == NF::FactionStanding::Allied);

    // reverse order should also find it
    auto* r2 = mgr.getRelation("b", "a");
    REQUIRE(r2 != nullptr);

    auto allies = mgr.alliedFactions("a");
    REQUIRE(allies.size() == 1);
    REQUIRE(allies[0]->factionId() == "b");

    auto hostiles = mgr.hostileFactions("a");
    REQUIRE(hostiles.size() == 1);
    REQUIRE(hostiles[0]->factionId() == "c");
}

// ── SP1 Voxel Material Table Tests ────────────────────────────

TEST_CASE("VoxelMaterialDef properties", "[Game][SP1][Material]") {
    NF::VoxelMaterialDef stone{NF::VoxelType::Stone, 2.5f, 8.f, false, false, "gravel", 2, 10.f, 6.f};
    REQUIRE(stone.isSolid());
    REQUIRE(stone.isMineable());
    REQUIRE(stone.yieldsItem());
    REQUIRE(stone.yieldMaterial == "gravel");

    NF::VoxelMaterialDef air{NF::VoxelType::Air, 0.f, 0.f, false, false, "", 0, 0.f, 0.f};
    REQUIRE_FALSE(air.isSolid());
    REQUIRE_FALSE(air.isMineable());
    REQUIRE_FALSE(air.yieldsItem());
}

TEST_CASE("VoxelMaterialTable load defaults", "[Game][SP1][Material]") {
    NF::VoxelMaterialTable table;
    table.loadDefaults();
    REQUIRE(table.materialCount() == static_cast<size_t>(NF::VoxelType::Count));

    auto& stone = table.get(NF::VoxelType::Stone);
    REQUIRE(stone.density == Catch::Approx(2.5f));
    REQUIRE(stone.hardness == Catch::Approx(8.f));
    REQUIRE_FALSE(stone.isLoose);

    auto& dirt = table.get(NF::VoxelType::Dirt);
    REQUIRE(dirt.isLoose);
    REQUIRE(dirt.canCollapse);
    REQUIRE(dirt.yieldMaterial == "soil");

    auto& iron = table.get(NF::VoxelType::Ore_Iron);
    REQUIRE(iron.yieldMaterial == "raw_iron");
    REQUIRE(iron.yieldsItem());
}

// ── SP2 Centrifuge System Tests ───────────────────────────────

TEST_CASE("CentrifugeState names", "[Game][SP2][Centrifuge]") {
    REQUIRE(std::string(NF::centrifugeStateName(NF::CentrifugeState::Idle)) == "Idle");
    REQUIRE(std::string(NF::centrifugeStateName(NF::CentrifugeState::Processing)) == "Processing");
    REQUIRE(std::string(NF::centrifugeStateName(NF::CentrifugeState::PowerStall)) == "PowerStall");
}

TEST_CASE("CentrifugeJob progress", "[Game][SP2][Centrifuge]") {
    NF::CentrifugeJob job;
    job.processingTime = 10.f;
    job.elapsed = 5.f;
    REQUIRE(job.progress() == Catch::Approx(0.5f));
    REQUIRE_FALSE(job.isComplete());
    job.elapsed = 10.f;
    REQUIRE(job.isComplete());
    REQUIRE(job.progress() == Catch::Approx(1.0f));
}

TEST_CASE("CentrifugeSystem tier and queue", "[Game][SP2][Centrifuge]") {
    NF::CentrifugeSystem cs;
    REQUIRE(cs.tier() == 1);
    REQUIRE(cs.maxQueueSize() == 4);
    REQUIRE(cs.state() == NF::CentrifugeState::Idle);

    cs.setTier(2);
    REQUIRE(cs.tier() == 2);
    REQUIRE(cs.maxQueueSize() == 6);

    cs.setTier(3);
    REQUIRE(cs.maxQueueSize() == 8);
}

TEST_CASE("CentrifugeSystem processing lifecycle", "[Game][SP2][Centrifuge]") {
    NF::CentrifugeSystem cs;
    NF::CentrifugeJob job;
    job.inputMaterial = "raw_iron";
    job.inputQuantity = 1;
    job.outputMaterial = "iron_ingot";
    job.outputQuantity = 1;
    job.processingTime = 2.0f;
    job.powerRequired = 5.f;

    REQUIRE(cs.addJob(job));
    REQUIRE(cs.queueSize() == 1);
    REQUIRE(cs.state() == NF::CentrifugeState::Loading);

    // Tick with enough power
    cs.tick(1.0f, 10.f);
    REQUIRE(cs.state() == NF::CentrifugeState::Processing);
    REQUIRE(cs.currentProgress() == Catch::Approx(0.5f));

    // Complete
    cs.tick(1.0f, 10.f);
    REQUIRE(cs.state() == NF::CentrifugeState::Complete);

    // Collect output
    cs.collectOutput();
    REQUIRE(cs.state() == NF::CentrifugeState::Idle);
    REQUIRE(cs.queueSize() == 0);
}

TEST_CASE("CentrifugeSystem power stall", "[Game][SP2][Centrifuge]") {
    NF::CentrifugeSystem cs;
    NF::CentrifugeJob job;
    job.processingTime = 5.f;
    job.powerRequired = 10.f;
    cs.addJob(job);
    cs.tick(1.f, 10.f); // Loading → Processing

    // Not enough power
    cs.tick(1.f, 5.f);
    REQUIRE(cs.state() == NF::CentrifugeState::PowerStall);

    // Power restored
    cs.tick(1.f, 10.f);
    REQUIRE(cs.state() != NF::CentrifugeState::PowerStall);
}

TEST_CASE("CentrifugeSystem queue capacity", "[Game][SP2][Centrifuge]") {
    NF::CentrifugeSystem cs;
    // Tier 1: max 4
    for (int i = 0; i < 4; ++i) {
        NF::CentrifugeJob j; j.processingTime = 10.f;
        REQUIRE(cs.addJob(j));
    }
    NF::CentrifugeJob extra; extra.processingTime = 10.f;
    REQUIRE_FALSE(cs.addJob(extra));
    REQUIRE(cs.queueSize() == 4);
}

// ── SP3 Interface Port Tests ──────────────────────────────────

TEST_CASE("LinkState names", "[Game][SP3][InterfacePort]") {
    REQUIRE(std::string(NF::linkStateName(NF::LinkState::Idle)) == "Idle");
    REQUIRE(std::string(NF::linkStateName(NF::LinkState::Linked)) == "Linked");
    REQUIRE(std::string(NF::linkStateName(NF::LinkState::Control)) == "Control");
}

TEST_CASE("InterfacePort state machine", "[Game][SP3][InterfacePort]") {
    NF::InterfacePort port;
    REQUIRE(port.state() == NF::LinkState::Idle);
    REQUIRE_FALSE(port.isLinked());
    REQUIRE_FALSE(port.hasControl());

    port.beginContact("terminal_01");
    REQUIRE(port.state() == NF::LinkState::Contact);
    REQUIRE(port.currentTarget() == "terminal_01");

    port.attemptLink();
    REQUIRE(port.state() == NF::LinkState::Linked);
    REQUIRE(port.isLinked());
    REQUIRE(port.linkQuality() == Catch::Approx(0.85f));

    port.enterControl();
    REQUIRE(port.state() == NF::LinkState::Control);
    REQUIRE(port.hasControl());

    port.disconnect();
    REQUIRE(port.state() == NF::LinkState::Idle);
    REQUIRE_FALSE(port.isLinked());
}

TEST_CASE("InterfacePort link failure and retry", "[Game][SP3][InterfacePort]") {
    NF::InterfacePort port;
    port.beginContact("secured_terminal");
    port.failLink();
    REQUIRE(port.state() == NF::LinkState::LinkFailed);

    port.retryFromFail();
    REQUIRE(port.state() == NF::LinkState::Contact);
}

// ── SP4 Sand Physics Tests ────────────────────────────────────

TEST_CASE("SandPhysicsSystem collapse detection", "[Game][SP4][SandPhysics]") {
    NF::VoxelMaterialTable table;
    table.loadDefaults();

    NF::Chunk chunk;
    chunk.set(5, 5, 5, NF::VoxelType::Dirt); // loose, above air

    NF::SandPhysicsSystem physics;
    physics.setMaterialTable(&table);

    REQUIRE(physics.wouldCollapse(chunk, 5, 5, 5)); // air below
    chunk.set(5, 4, 5, NF::VoxelType::Stone); // support below
    REQUIRE_FALSE(physics.wouldCollapse(chunk, 5, 5, 5));
}

TEST_CASE("SandPhysicsSystem simulate step", "[Game][SP4][SandPhysics]") {
    NF::VoxelMaterialTable table;
    table.loadDefaults();

    NF::Chunk chunk;
    // Place dirt at y=5 with air below
    chunk.set(3, 5, 3, NF::VoxelType::Dirt);

    NF::SandPhysicsSystem physics;
    physics.setMaterialTable(&table);

    int moved = physics.simulateStep(chunk);
    REQUIRE(moved == 1);
    REQUIRE(chunk.get(3, 5, 3) == NF::VoxelType::Air); // original position cleared
    REQUIRE(chunk.get(3, 0, 3) == NF::VoxelType::Dirt); // fell to bottom
    REQUIRE(physics.lastEvents().size() == 1);
    REQUIRE(physics.lastEvents()[0].fallDistance == 5);
}

TEST_CASE("SandPhysicsSystem stone does not collapse", "[Game][SP4][SandPhysics]") {
    NF::VoxelMaterialTable table;
    table.loadDefaults();

    NF::Chunk chunk;
    chunk.set(3, 5, 3, NF::VoxelType::Stone); // not loose

    NF::SandPhysicsSystem physics;
    physics.setMaterialTable(&table);

    REQUIRE_FALSE(physics.wouldCollapse(chunk, 3, 5, 3));
    int moved = physics.simulateStep(chunk);
    REQUIRE(moved == 0);
}

// ── SP5 Breach Minigame Tests ─────────────────────────────────

TEST_CASE("BreachState names", "[Game][SP5][Breach]") {
    REQUIRE(std::string(NF::breachStateName(NF::BreachState::Inactive)) == "Inactive");
    REQUIRE(std::string(NF::breachStateName(NF::BreachState::Active)) == "Active");
    REQUIRE(std::string(NF::breachStateName(NF::BreachState::Success)) == "Success");
}

TEST_CASE("BreachMinigame lifecycle", "[Game][SP5][Breach]") {
    NF::BreachMinigame game;
    REQUIRE(game.state() == NF::BreachState::Inactive);

    NF::BreachGrid grid{6, 6, 30.f, 3, 2};
    game.initiate(grid);
    REQUIRE(game.state() == NF::BreachState::Initiating);
    REQUIRE(game.timeRemaining() == Catch::Approx(30.f));

    game.start();
    REQUIRE(game.state() == NF::BreachState::Active);

    // Move trace
    game.moveTrace(1, 0);
    REQUIRE(game.traceX() == 1);
    game.moveTrace(0, 1);
    REQUIRE(game.traceY() == 1);

    // Collect data nodes to win
    game.collectDataNode();
    REQUIRE(game.dataCollected() == 1);
    game.collectDataNode();
    REQUIRE(game.state() == NF::BreachState::Success);
}

TEST_CASE("BreachMinigame timeout", "[Game][SP5][Breach]") {
    NF::BreachMinigame game;
    NF::BreachGrid grid{6, 6, 5.f, 3, 2};
    game.initiate(grid);
    game.start();

    game.tick(6.f); // exceed time limit
    REQUIRE(game.state() == NF::BreachState::Failure);
    REQUIRE(game.timeRemaining() == Catch::Approx(0.f));
}

TEST_CASE("BreachMinigame partial success", "[Game][SP5][Breach]") {
    NF::BreachMinigame game;
    NF::BreachGrid grid{6, 6, 5.f, 3, 2};
    game.initiate(grid);
    game.start();
    game.collectDataNode(); // got 1 of 2

    game.hitIce();
    REQUIRE(game.state() == NF::BreachState::Partial);
}

TEST_CASE("BreachMinigame bounds check", "[Game][SP5][Breach]") {
    NF::BreachMinigame game;
    NF::BreachGrid grid{6, 6, 30.f, 3, 2};
    game.initiate(grid);
    game.start();

    game.moveTrace(-1, 0); // can't go below 0
    REQUIRE(game.traceX() == 0);
    game.moveTrace(0, -1);
    REQUIRE(game.traceY() == 0);
}

// ── SP6 R.I.G. AI Core Tests ─────────────────────────────────

TEST_CASE("RigAIEvent names", "[Game][SP6][RigAI]") {
    REQUIRE(std::string(NF::rigAIEventName(NF::RigAIEvent::PowerChanged)) == "PowerChanged");
    REQUIRE(std::string(NF::rigAIEventName(NF::RigAIEvent::OxygenLow)) == "OxygenLow");
    REQUIRE(std::string(NF::rigAIEventName(NF::RigAIEvent::SystemFailure)) == "SystemFailure");
}

TEST_CASE("RigAIFeatures enabled count", "[Game][SP6][RigAI]") {
    NF::RigAIFeatures feat;
    REQUIRE(feat.enabledCount() == 1); // vitals only
    feat.scanning = true;
    feat.mapping = true;
    REQUIRE(feat.enabledCount() == 3);
}

TEST_CASE("RigAICore event routing", "[Game][SP6][RigAI]") {
    NF::RigAICore ai;
    NF::RigAIFeatures feat;
    feat.scanning = true;
    ai.init(feat);
    REQUIRE(ai.isInitialized());
    REQUIRE(ai.pendingAlerts() == 0);

    ai.onEvent(NF::RigAIEvent::OxygenLow);
    REQUIRE(ai.pendingAlerts() == 1);
    REQUIRE(ai.alerts()[0].severity == Catch::Approx(1.0f)); // critical

    ai.onEvent(NF::RigAIEvent::ScanResult); // scanning enabled
    REQUIRE(ai.pendingAlerts() == 2);

    ai.clearAlerts();
    REQUIRE(ai.pendingAlerts() == 0);
}

TEST_CASE("RigAICore feature gating", "[Game][SP6][RigAI]") {
    NF::RigAICore ai;
    NF::RigAIFeatures feat;
    // scanning disabled by default
    ai.init(feat);

    ai.onEvent(NF::RigAIEvent::ScanResult); // should be filtered
    REQUIRE(ai.pendingAlerts() == 0);

    ai.enableFeature("scanning");
    ai.onEvent(NF::RigAIEvent::ScanResult);
    REQUIRE(ai.pendingAlerts() == 1);

    ai.disableFeature("scanning");
    ai.onEvent(NF::RigAIEvent::ScanResult); // filtered again
    REQUIRE(ai.pendingAlerts() == 1); // no new alert
}

TEST_CASE("RigAICore enable/disable features", "[Game][SP6][RigAI]") {
    NF::RigAICore ai;
    NF::RigAIFeatures feat;
    ai.init(feat);
    REQUIRE(ai.features().enabledCount() == 1);

    ai.enableFeature("droneControl");
    ai.enableFeature("fleetCommand");
    REQUIRE(ai.features().enabledCount() == 3);

    ai.disableFeature("droneControl");
    REQUIRE(ai.features().enabledCount() == 2);
}

// ── G22 Weather System Tests ──────────────────────────────────

TEST_CASE("WeatherType names", "[Game][G22][Weather]") {
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::Clear))      == "Clear");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::Rain))       == "Rain");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::Storm))      == "Storm");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::Snow))       == "Snow");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::Fog))        == "Fog");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::Sandstorm))  == "Sandstorm");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::AcidRain))   == "Acid Rain");
    REQUIRE(std::string(NF::weatherTypeName(NF::WeatherType::SolarFlare)) == "Solar Flare");
}

TEST_CASE("WeatherCondition default is Clear", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    REQUIRE(cond.type == NF::WeatherType::Clear);
    REQUIRE(cond.intensity == 0.f);
    REQUIRE(cond.duration == 0.f);
    REQUIRE(cond.elapsed == 0.f);
    REQUIRE_FALSE(cond.isExpired());
    REQUIRE(cond.effectiveIntensity() == 0.f);
}

TEST_CASE("WeatherCondition expiration", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    cond.type = NF::WeatherType::Rain;
    cond.intensity = 0.8f;
    cond.duration = 10.f;
    cond.elapsed = 5.f;
    REQUIRE_FALSE(cond.isExpired());
    REQUIRE(cond.progress() == Catch::Approx(0.5f));

    cond.elapsed = 10.f;
    REQUIRE(cond.isExpired());
    REQUIRE(cond.progress() == Catch::Approx(1.0f));
}

TEST_CASE("WeatherCondition fade-in intensity", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    cond.type = NF::WeatherType::Storm;
    cond.intensity = 1.0f;
    cond.duration = 100.f;
    cond.transitionTime = 2.f;
    cond.elapsed = 1.f;  // halfway through fade-in

    float ei = cond.effectiveIntensity();
    REQUIRE(ei == Catch::Approx(0.5f));
}

TEST_CASE("WeatherCondition fade-out intensity", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    cond.type = NF::WeatherType::Fog;
    cond.intensity = 1.0f;
    cond.duration = 10.f;
    cond.transitionTime = 2.f;
    cond.elapsed = 9.f;  // 1 second remaining, within fade-out

    float ei = cond.effectiveIntensity();
    REQUIRE(ei == Catch::Approx(0.5f));
}

TEST_CASE("WeatherEffects for Clear", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    cond.type = NF::WeatherType::Clear;
    cond.intensity = 0.f;

    auto fx = NF::WeatherEffects::forCondition(cond);
    REQUIRE(fx.visibilityMultiplier == 1.f);
    REQUIRE(fx.movementMultiplier == 1.f);
    REQUIRE(fx.damagePerSecond == 0.f);
    REQUIRE_FALSE(fx.disablesScanner);
    REQUIRE_FALSE(fx.disablesNavigation);
}

TEST_CASE("WeatherEffects for AcidRain causes damage", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    cond.type = NF::WeatherType::AcidRain;
    cond.intensity = 1.0f;
    cond.duration = 100.f;
    cond.transitionTime = 0.f;  // no fade for clean test
    cond.elapsed = 50.f;        // mid-duration

    auto fx = NF::WeatherEffects::forCondition(cond);
    REQUIRE(fx.damagePerSecond == Catch::Approx(5.0f));
    REQUIRE(fx.visibilityMultiplier < 1.f);
}

TEST_CASE("WeatherEffects SolarFlare disables scanner", "[Game][G22][Weather]") {
    NF::WeatherCondition cond;
    cond.type = NF::WeatherType::SolarFlare;
    cond.intensity = 0.8f;
    cond.duration = 100.f;
    cond.transitionTime = 0.f;
    cond.elapsed = 50.f;

    auto fx = NF::WeatherEffects::forCondition(cond);
    REQUIRE(fx.disablesScanner);
    REQUIRE(fx.disablesNavigation);
    REQUIRE(fx.damagePerSecond > 0.f);
}

TEST_CASE("WeatherSystem default is Clear", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    REQUIRE(ws.isClear());
    REQUIRE(ws.currentType() == NF::WeatherType::Clear);
    REQUIRE(ws.forecastCount() == 0);
}

TEST_CASE("WeatherSystem setWeather changes active", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    ws.setWeather(NF::WeatherType::Storm, 0.9f, 30.f);

    REQUIRE(ws.currentType() == NF::WeatherType::Storm);
    REQUIRE_FALSE(ws.isClear());
    REQUIRE(ws.activeWeather().intensity == Catch::Approx(0.9f));
    REQUIRE(ws.activeWeather().duration == Catch::Approx(30.f));
}

TEST_CASE("WeatherSystem tick advances elapsed", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    ws.setWeather(NF::WeatherType::Rain, 0.5f, 10.f);

    ws.tick(3.f);
    REQUIRE(ws.activeWeather().elapsed == Catch::Approx(3.f));
}

TEST_CASE("WeatherSystem auto-transitions to Clear when expired", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    ws.setWeather(NF::WeatherType::Snow, 0.6f, 5.f);

    ws.tick(6.f);  // exceeds duration
    REQUIRE(ws.isClear());
}

TEST_CASE("WeatherSystem forecast transitions", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    ws.setWeather(NF::WeatherType::Rain, 0.5f, 5.f);

    NF::WeatherForecastEntry next;
    next.type = NF::WeatherType::Storm;
    next.intensity = 0.9f;
    next.duration = 10.f;
    next.delayUntilStart = 0.f;
    ws.addForecast(next);
    REQUIRE(ws.forecastCount() == 1);

    // Expire current → should pick up forecast
    ws.tick(6.f);
    REQUIRE(ws.currentType() == NF::WeatherType::Storm);
    REQUIRE(ws.forecastCount() == 0);
}

TEST_CASE("WeatherSystem clearForecast removes entries", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;

    NF::WeatherForecastEntry e;
    e.type = NF::WeatherType::Fog;
    e.intensity = 0.3f;
    e.duration = 10.f;
    ws.addForecast(e);
    ws.addForecast(e);
    REQUIRE(ws.forecastCount() == 2);

    ws.clearForecast();
    REQUIRE(ws.forecastCount() == 0);
}

TEST_CASE("WeatherSystem clearWeather forces Clear", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    ws.setWeather(NF::WeatherType::Sandstorm, 1.0f, 60.f);
    REQUIRE_FALSE(ws.isClear());

    ws.clearWeather();
    REQUIRE(ws.isClear());
}

TEST_CASE("WeatherSystem max forecast cap", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;

    NF::WeatherForecastEntry e;
    e.type = NF::WeatherType::Rain;
    e.intensity = 0.5f;
    e.duration = 10.f;

    for (int i = 0; i < 12; ++i)
        ws.addForecast(e);

    REQUIRE(ws.forecastCount() == static_cast<size_t>(NF::WeatherSystem::kMaxForecast));
}

TEST_CASE("WeatherSystem currentEffects reflects active weather", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    ws.setWeather(NF::WeatherType::Fog, 1.0f);
    // Advance past transition time
    ws.tick(5.f);

    auto fx = ws.currentEffects();
    REQUIRE(fx.visibilityMultiplier < 1.f);
    REQUIRE(fx.movementMultiplier == 1.f);  // Fog doesn't affect movement
}

TEST_CASE("WeatherSystem delayed forecast triggers when clear", "[Game][G22][Weather]") {
    NF::WeatherSystem ws;
    // Start clear

    NF::WeatherForecastEntry e;
    e.type = NF::WeatherType::Storm;
    e.intensity = 0.8f;
    e.duration = 20.f;
    e.delayUntilStart = 5.f;
    ws.addForecast(e);

    // Not ready yet
    ws.tick(3.f);
    REQUIRE(ws.isClear());

    // Delay elapsed
    ws.tick(3.f);
    REQUIRE(ws.currentType() == NF::WeatherType::Storm);
}

// ── G23 Trading System Tests ──────────────────────────────────

TEST_CASE("TradeGoodCategory names", "[Game][G23][Trading]") {
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Raw))        == "Raw");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Refined))    == "Refined");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Component))  == "Component");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Consumable)) == "Consumable");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Tech))       == "Tech");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Luxury))     == "Luxury");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Contraband)) == "Contraband");
    REQUIRE(std::string(NF::tradeGoodCategoryName(NF::TradeGoodCategory::Data))       == "Data");
}

TEST_CASE("TradeGood contraband flag", "[Game][G23][Trading]") {
    NF::TradeGood g;
    g.id = "spice";
    g.legal = false;
    REQUIRE(g.isContraband());
    g.legal = true;
    REQUIRE_FALSE(g.isContraband());
}

TEST_CASE("TradingPost add and query stock", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Station Alpha");
    REQUIRE(post.postId() == "hub1");
    REQUIRE(post.name() == "Station Alpha");

    post.addStock("iron", 50, 10.f);
    REQUIRE(post.stockOf("iron") == 50);
    REQUIRE(post.priceOf("iron") == 10.f);
    REQUIRE(post.stockCount() == 1);

    // Add more of same good updates quantity
    post.addStock("iron", 30, 12.f);
    REQUIRE(post.stockOf("iron") == 80);
    REQUIRE(post.priceOf("iron") == 12.f);  // price updated
}

TEST_CASE("TradingPost remove stock", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Hub");
    post.addStock("copper", 100, 5.f);

    int removed = post.removeStock("copper", 40);
    REQUIRE(removed == 40);
    REQUIRE(post.stockOf("copper") == 60);

    // Can't remove more than available
    removed = post.removeStock("copper", 200);
    REQUIRE(removed == 60);
    REQUIRE(post.stockOf("copper") == 0);

    // Non-existent good returns 0
    removed = post.removeStock("gold", 10);
    REQUIRE(removed == 0);
}

TEST_CASE("TradingPost buy transaction", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Hub");
    post.addStock("fuel", 100, 20.f);

    float cost = post.buy("fuel", 5);
    REQUIRE(cost == Catch::Approx(100.f));  // 5 × 20
    REQUIRE(post.stockOf("fuel") == 95);
    REQUIRE(post.totalSales() == Catch::Approx(100.f));
}

TEST_CASE("TradingPost buy insufficient stock", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Hub");
    post.addStock("fuel", 3, 20.f);

    float cost = post.buy("fuel", 10);  // not enough
    REQUIRE(cost == 0.f);
    REQUIRE(post.stockOf("fuel") == 3);  // unchanged
}

TEST_CASE("TradingPost sell transaction", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Hub");

    float revenue = post.sell("iron", 10, 50.f);
    REQUIRE(revenue == Catch::Approx(400.f));  // 10 × 50 × 0.8
    REQUIRE(post.stockOf("iron") == 10);  // added to stock
    REQUIRE(post.totalPurchases() == Catch::Approx(400.f));
}

TEST_CASE("TradingPost tax rate", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Hub");
    REQUIRE(post.taxRate() == Catch::Approx(0.05f));  // default 5%

    post.setTaxRate(0.1f);
    REQUIRE(post.taxRate() == Catch::Approx(0.1f));

    // Clamped to [0, 1]
    post.setTaxRate(-0.5f);
    REQUIRE(post.taxRate() == 0.f);
    post.setTaxRate(2.f);
    REQUIRE(post.taxRate() == 1.f);
}

TEST_CASE("TradingPost tick prices supply/demand", "[Game][G23][Trading]") {
    NF::TradingPost post("hub1", "Hub");
    post.addStock("rare_gem", 5, 100.f);  // low stock → price rises

    float priceBefore = post.priceOf("rare_gem");
    post.tickPrices(1.f);
    float priceAfter = post.priceOf("rare_gem");
    REQUIRE(priceAfter > priceBefore);  // low stock drives price up
}

TEST_CASE("TradingSystem register goods", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    NF::TradeGood g;
    g.id = "iron_ore";
    g.name = "Iron Ore";
    g.category = NF::TradeGoodCategory::Raw;
    g.basePrice = 15.f;
    ts.registerGood(g);
    REQUIRE(ts.goodCount() == 1);

    // No duplicate
    ts.registerGood(g);
    REQUIRE(ts.goodCount() == 1);

    auto* found = ts.findGood("iron_ore");
    REQUIRE(found != nullptr);
    REQUIRE(found->name == "Iron Ore");
}

TEST_CASE("TradingSystem add/remove posts", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    ts.addPost(NF::TradingPost("alpha", "Alpha Station"));
    ts.addPost(NF::TradingPost("beta", "Beta Station"));
    REQUIRE(ts.postCount() == 2);

    REQUIRE(ts.findPost("alpha") != nullptr);
    REQUIRE(ts.removePost("beta"));
    REQUIRE(ts.postCount() == 1);
    REQUIRE_FALSE(ts.removePost("beta"));  // already removed
}

TEST_CASE("TradingSystem add routes", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    NF::TradeRoute r;
    r.originId = "alpha";
    r.destinationId = "beta";
    r.goodId = "iron";
    r.distance = 50.f;
    ts.addRoute(r);
    REQUIRE(ts.routeCount() == 1);
}

TEST_CASE("TradingSystem executeBuy", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    NF::TradingPost post("alpha", "Alpha");
    post.addStock("fuel", 100, 25.f);
    ts.addPost(std::move(post));

    float cost = ts.executeBuy("alpha", "fuel", 4);
    REQUIRE(cost == Catch::Approx(100.f));  // 4 × 25
    REQUIRE(ts.totalVolume() == Catch::Approx(100.f));
}

TEST_CASE("TradingSystem executeSell", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    NF::TradeGood g;
    g.id = "copper";
    g.basePrice = 30.f;
    ts.registerGood(g);
    ts.addPost(NF::TradingPost("alpha", "Alpha"));

    float revenue = ts.executeSell("alpha", "copper", 10);
    REQUIRE(revenue == Catch::Approx(240.f));  // 10 × 30 × 0.8
}

TEST_CASE("TradingSystem executeBuy nonexistent post", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    float cost = ts.executeBuy("nowhere", "fuel", 1);
    REQUIRE(cost == 0.f);
}

TEST_CASE("TradingSystem tick advances prices", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    NF::TradingPost post("alpha", "Alpha");
    post.addStock("rare", 3, 100.f);  // low stock
    ts.addPost(std::move(post));

    ts.tick(1.f);
    REQUIRE(ts.findPost("alpha")->priceOf("rare") > 100.f);
}

TEST_CASE("TradingSystem route profit margin", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    NF::TradingPost origin("a", "Origin");
    origin.addStock("fuel", 50, 10.f);
    NF::TradingPost dest("b", "Dest");
    dest.addStock("fuel", 50, 20.f);
    ts.addPost(std::move(origin));
    ts.addPost(std::move(dest));

    NF::TradeRoute route;
    route.originId = "a";
    route.destinationId = "b";
    route.goodId = "fuel";

    float margin = ts.routeProfitMargin(route);
    // Buy at 10, sell at 20 × 0.8 = 16, margin = (16-10)/10 = 0.6
    REQUIRE(margin == Catch::Approx(0.6f));
}

TEST_CASE("TradingSystem max caps", "[Game][G23][Trading]") {
    NF::TradingSystem ts;
    REQUIRE(NF::TradingSystem::kMaxPosts == 32);
    REQUIRE(NF::TradingSystem::kMaxRoutes == 64);
    REQUIRE(NF::TradingSystem::kMaxGoods == 128);
}

// ── G24 Base Building System Tests ────────────────────────────

TEST_CASE("BasePartCategory names", "[Game][G24][Base]") {
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Foundation)) == "Foundation");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Wall))       == "Wall");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Floor))      == "Floor");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Ceiling))    == "Ceiling");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Door))       == "Door");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Window))     == "Window");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Utility))    == "Utility");
    REQUIRE(std::string(NF::basePartCategoryName(NF::BasePartCategory::Decoration)) == "Decoration");
}

TEST_CASE("BasePart requiresPower", "[Game][G24][Base]") {
    NF::BasePart p;
    p.powerDraw = 0.f;
    REQUIRE_FALSE(p.requiresPower());
    p.powerDraw = 5.f;
    REQUIRE(p.requiresPower());
}

TEST_CASE("BaseGridPos equality", "[Game][G24][Base]") {
    NF::BaseGridPos a{1, 2, 3};
    NF::BaseGridPos b{1, 2, 3};
    NF::BaseGridPos c{0, 2, 3};
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("BaseLayout place and remove", "[Game][G24][Base]") {
    NF::BaseLayout layout;
    REQUIRE(layout.partCount() == 0);

    REQUIRE(layout.placePart("foundation_1", {0, 0, 0}));
    REQUIRE(layout.partCount() == 1);

    // Can't place on occupied cell
    REQUIRE_FALSE(layout.placePart("wall_1", {0, 0, 0}));

    // Place adjacent
    REQUIRE(layout.placePart("wall_1", {1, 0, 0}));
    REQUIRE(layout.partCount() == 2);

    // Remove
    REQUIRE(layout.removePart({0, 0, 0}));
    REQUIRE(layout.partCount() == 1);

    // Can't remove non-existent
    REQUIRE_FALSE(layout.removePart({5, 5, 5}));
}

TEST_CASE("BaseLayout partAt", "[Game][G24][Base]") {
    NF::BaseLayout layout;
    layout.placePart("floor_1", {3, 0, 2});

    auto* p = layout.partAt({3, 0, 2});
    REQUIRE(p != nullptr);
    REQUIRE(p->partId == "floor_1");

    REQUIRE(layout.partAt({9, 9, 9}) == nullptr);
}

TEST_CASE("BaseLayout adjacency", "[Game][G24][Base]") {
    NF::BaseLayout layout;
    layout.placePart("center", {5, 5, 5});
    layout.placePart("left",   {4, 5, 5});
    layout.placePart("right",  {6, 5, 5});
    layout.placePart("above",  {5, 6, 5});

    REQUIRE(layout.adjacentCount({5, 5, 5}) == 3);
    REQUIRE(layout.adjacentCount({4, 5, 5}) == 1);  // only "center"
    REQUIRE(layout.adjacentCount({0, 0, 0}) == 0);  // no neighbors
}

TEST_CASE("BaseLayout structural integrity", "[Game][G24][Base]") {
    NF::BaseLayout layout;
    std::vector<NF::BasePart> defs;

    NF::BasePart found;
    found.id = "foundation";
    found.category = NF::BasePartCategory::Foundation;
    defs.push_back(found);

    NF::BasePart wall;
    wall.id = "wall";
    wall.category = NF::BasePartCategory::Wall;
    defs.push_back(wall);

    // Foundation alone is always valid
    layout.placePart("foundation", {0, 0, 0});
    REQUIRE(layout.isStructurallySound(defs));

    // Wall adjacent to foundation — valid
    layout.placePart("wall", {1, 0, 0});
    REQUIRE(layout.isStructurallySound(defs));

    // Floating wall (no neighbors) — invalid
    layout.placePart("wall", {10, 10, 10});
    REQUIRE_FALSE(layout.isStructurallySound(defs));
}

TEST_CASE("BaseLayout totalPowerDraw", "[Game][G24][Base]") {
    NF::BaseLayout layout;
    std::vector<NF::BasePart> defs;

    NF::BasePart generator;
    generator.id = "generator";
    generator.powerDraw = 0.f;
    defs.push_back(generator);

    NF::BasePart light;
    light.id = "light";
    light.powerDraw = 5.f;
    defs.push_back(light);

    layout.placePart("generator", {0, 0, 0});
    layout.placePart("light", {1, 0, 0});
    layout.placePart("light", {2, 0, 0});

    REQUIRE(layout.totalPowerDraw(defs) == Catch::Approx(10.f));
}

TEST_CASE("BaseDefense shield absorb", "[Game][G24][Base]") {
    NF::BaseDefense def;
    def.shieldStrength = 100.f;
    def.hullArmor = 0.25f;
    def.currentShield = 80.f;

    // Damage fully absorbed by shield
    float passthrough = def.takeDamage(30.f);
    REQUIRE(passthrough == 0.f);
    REQUIRE(def.currentShield == Catch::Approx(50.f));

    // Damage partially absorbed — remainder reduced by armor
    passthrough = def.takeDamage(70.f);
    // 50 absorbed by shield, 20 remaining, 25% armor → 15 pass-through
    REQUIRE(passthrough == Catch::Approx(15.f));
    REQUIRE(def.currentShield == 0.f);
}

TEST_CASE("BaseDefense shield regen", "[Game][G24][Base]") {
    NF::BaseDefense def;
    def.shieldStrength = 100.f;
    def.currentShield = 50.f;

    def.regenShield(30.f);
    REQUIRE(def.currentShield == Catch::Approx(80.f));

    // Capped at max
    def.regenShield(50.f);
    REQUIRE(def.currentShield == Catch::Approx(100.f));
}

TEST_CASE("BaseDefense reset shields", "[Game][G24][Base]") {
    NF::BaseDefense def;
    def.shieldStrength = 200.f;
    def.currentShield = 0.f;
    def.resetShields();
    REQUIRE(def.currentShield == Catch::Approx(200.f));
}

TEST_CASE("BaseSystem create and manage bases", "[Game][G24][Base]") {
    NF::BaseSystem sys;
    REQUIRE(sys.baseCount() == 0);

    int idx = sys.createBase("Alpha Outpost");
    REQUIRE(idx == 0);
    REQUIRE(sys.baseCount() == 1);
    REQUIRE(sys.baseName(0) == "Alpha Outpost");

    int idx2 = sys.createBase("Beta Station");
    REQUIRE(idx2 == 1);

    REQUIRE(sys.removeBase(0));
    REQUIRE(sys.baseCount() == 1);
    REQUIRE(sys.baseName(0) == "Beta Station");
}

TEST_CASE("BaseSystem register parts", "[Game][G24][Base]") {
    NF::BaseSystem sys;
    NF::BasePart p;
    p.id = "metal_wall";
    p.name = "Metal Wall";
    p.category = NF::BasePartCategory::Wall;
    sys.registerPart(p);
    REQUIRE(sys.partDefCount() == 1);

    // No duplicates
    sys.registerPart(p);
    REQUIRE(sys.partDefCount() == 1);

    auto* found = sys.findPart("metal_wall");
    REQUIRE(found != nullptr);
    REQUIRE(found->name == "Metal Wall");
}

TEST_CASE("BaseSystem power management", "[Game][G24][Base]") {
    NF::BaseSystem sys;
    NF::BasePart light;
    light.id = "light";
    light.powerDraw = 10.f;
    sys.registerPart(light);

    int base = sys.createBase("Test Base");
    sys.layout(base)->placePart("light", {0, 0, 0});
    sys.layout(base)->placePart("light", {1, 0, 0});

    // Default power = 100, draw = 20
    REQUIRE(sys.hasSufficientPower(base));
    REQUIRE(sys.availablePower(base) == Catch::Approx(80.f));

    // Reduce power output below draw
    sys.setPowerOutput(base, 15.f);
    REQUIRE_FALSE(sys.hasSufficientPower(base));
    REQUIRE(sys.availablePower(base) == Catch::Approx(-5.f));
}

TEST_CASE("BaseSystem max bases", "[Game][G24][Base]") {
    NF::BaseSystem sys;
    REQUIRE(NF::BaseSystem::kMaxBases == 8);

    for (int i = 0; i < 8; ++i)
        REQUIRE(sys.createBase("base" + std::to_string(i)) >= 0);

    // 9th should fail
    REQUIRE(sys.createBase("overflow") == -1);
}

TEST_CASE("BaseLayout max parts", "[Game][G24][Base]") {
    REQUIRE(NF::BaseLayout::kMaxParts == 256);
}

// ── G25 Habitat System Tests ──────────────────────────────────────

TEST_CASE("HabitatZoneType names", "[Game][G25][Habitat]") {
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Living))      == "Living");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Engineering)) == "Engineering");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Medical))     == "Medical");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Command))     == "Command");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Cargo))       == "Cargo");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Recreation))  == "Recreation");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Hydroponics)) == "Hydroponics");
    REQUIRE(std::string(NF::habitatZoneTypeName(NF::HabitatZoneType::Airlock))     == "Airlock");
}

TEST_CASE("HabitatZone habitability", "[Game][G25][Habitat]") {
    NF::HabitatZone zone;
    zone.id = "quarters";
    zone.name = "Crew Quarters";
    zone.sealed = true;
    zone.oxygenLevel = 0.95f;
    zone.pressure = 1.0f;
    zone.temperature = 22.f;
    REQUIRE(zone.isHabitable());

    // Breached → not habitable
    zone.sealed = false;
    REQUIRE_FALSE(zone.isHabitable());
    REQUIRE(zone.isBreached());

    // Low oxygen → not habitable
    zone.sealed = true;
    zone.oxygenLevel = 0.1f;
    REQUIRE_FALSE(zone.isHabitable());
}

TEST_CASE("HabitatZone capacity", "[Game][G25][Habitat]") {
    NF::HabitatZone zone;
    zone.capacity = 6;
    zone.occupants = 4;
    REQUIRE(zone.availableCapacity() == 2.f);
}

TEST_CASE("LifeSupportModule defaults", "[Game][G25][Habitat]") {
    NF::LifeSupportModule mod;
    REQUIRE(mod.isOperational());
    REQUIRE(mod.oxygenGenRate > 0.f);
    REQUIRE(mod.powerDraw > 0.f);

    mod.active = false;
    REQUIRE_FALSE(mod.isOperational());
}

TEST_CASE("HabitatLayout add and remove zones", "[Game][G25][Habitat]") {
    NF::HabitatLayout layout;
    NF::HabitatZone z1;
    z1.id = "bridge"; z1.name = "Bridge"; z1.type = NF::HabitatZoneType::Command;
    NF::HabitatZone z2;
    z2.id = "medbay"; z2.name = "Med Bay"; z2.type = NF::HabitatZoneType::Medical;

    REQUIRE(layout.addZone(z1));
    REQUIRE(layout.addZone(z2));
    REQUIRE(layout.zoneCount() == 2);

    // No duplicate
    REQUIRE_FALSE(layout.addZone(z1));

    REQUIRE(layout.findZone("bridge") != nullptr);
    REQUIRE(layout.removeZone("bridge"));
    REQUIRE(layout.zoneCount() == 1);
    REQUIRE(layout.findZone("bridge") == nullptr);
}

TEST_CASE("HabitatLayout connections", "[Game][G25][Habitat]") {
    NF::HabitatLayout layout;
    NF::HabitatZone z1, z2, z3;
    z1.id = "a"; z2.id = "b"; z3.id = "c";
    layout.addZone(z1);
    layout.addZone(z2);
    layout.addZone(z3);

    REQUIRE(layout.connect("a", "b"));
    REQUIRE(layout.connect("b", "c"));

    // No duplicate connection
    REQUIRE_FALSE(layout.connect("a", "b"));

    REQUIRE(layout.neighborCount("b") == 2);
    REQUIRE(layout.neighborCount("a") == 1);

    auto nbrs = layout.neighbors("b");
    REQUIRE(nbrs.size() == 2);
}

TEST_CASE("HabitatLayout habitable count and capacity", "[Game][G25][Habitat]") {
    NF::HabitatLayout layout;
    NF::HabitatZone h1, h2;
    h1.id = "x"; h1.capacity = 4; h1.sealed = true; h1.oxygenLevel = 0.9f;
    h2.id = "y"; h2.capacity = 6; h2.sealed = false; // breached
    layout.addZone(h1);
    layout.addZone(h2);

    REQUIRE(layout.habitableCount() == 1);
    REQUIRE(layout.totalCapacity() == 4);
}

TEST_CASE("HabitatSystem create habitat", "[Game][G25][Habitat]") {
    NF::HabitatSystem sys;
    int idx = sys.createHabitat("Station Alpha");
    REQUIRE(idx == 0);
    REQUIRE(sys.habitatCount() == 1);
    REQUIRE(sys.habitatName(0) == "Station Alpha");
}

TEST_CASE("HabitatSystem max habitats", "[Game][G25][Habitat]") {
    NF::HabitatSystem sys;
    REQUIRE(NF::HabitatSystem::kMaxHabitats == 4);
    for (int i = 0; i < 4; ++i)
        REQUIRE(sys.createHabitat("h" + std::to_string(i)) >= 0);
    REQUIRE(sys.createHabitat("overflow") == -1);
}

TEST_CASE("HabitatSystem life support power draw", "[Game][G25][Habitat]") {
    NF::HabitatSystem sys;
    int h = sys.createHabitat("TestHab");

    NF::LifeSupportModule mod;
    mod.id = "ls1";
    mod.powerDraw = 20.f;
    sys.addLifeSupport(h, mod);
    mod.id = "ls2";
    mod.powerDraw = 10.f;
    sys.addLifeSupport(h, mod);

    REQUIRE(sys.lifeSupportPowerDraw(h) == Catch::Approx(30.f));
}

TEST_CASE("HabitatSystem breach and repair", "[Game][G25][Habitat]") {
    NF::HabitatSystem sys;
    int h = sys.createHabitat("OutpostBeta");
    NF::HabitatZone zone;
    zone.id = "cargo";
    zone.name = "Cargo Bay";
    zone.type = NF::HabitatZoneType::Cargo;
    sys.layout(h)->addZone(zone);

    REQUIRE(sys.breachZone(h, "cargo"));
    auto* z = sys.layout(h)->findZone("cargo");
    REQUIRE_FALSE(z->sealed);
    REQUIRE(z->oxygenLevel == 0.f);
    REQUIRE(z->pressure == 0.f);

    REQUIRE(sys.repairBreach(h, "cargo"));
    REQUIRE(z->sealed);
    REQUIRE(z->pressure == 1.f);
}

TEST_CASE("HabitatSystem tickAtmosphere oxygen generation", "[Game][G25][Habitat]") {
    NF::HabitatSystem sys;
    int h = sys.createHabitat("AtmoTest");

    NF::HabitatZone zone;
    zone.id = "living";
    zone.name = "Living";
    zone.oxygenLevel = 0.5f;
    zone.sealed = true;
    zone.occupants = 0;
    sys.layout(h)->addZone(zone);

    NF::LifeSupportModule mod;
    mod.id = "ls";
    mod.oxygenGenRate = 0.1f;
    sys.addLifeSupport(h, mod);

    sys.tickAtmosphere(h, 1.f);

    auto* z = sys.layout(h)->findZone("living");
    REQUIRE(z->oxygenLevel > 0.5f);  // oxygen should increase
}

TEST_CASE("HabitatLayout max zones", "[Game][G25][Habitat]") {
    REQUIRE(NF::HabitatLayout::kMaxZones == 32);
}

// ---------------------------------------------------------------------------
// G26 — Power Grid System Tests
// ---------------------------------------------------------------------------

TEST_CASE("PowerSourceType names", "[Game][G26][Power]") {
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Solar))      == "Solar");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Nuclear))    == "Nuclear");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Fusion))     == "Fusion");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Geothermal)) == "Geothermal");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Wind))       == "Wind");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Battery))    == "Battery");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::FuelCell))   == "Fuel Cell");
    REQUIRE(std::string(NF::powerSourceTypeName(NF::PowerSourceType::Antimatter)) == "Antimatter");
}

TEST_CASE("PowerNode defaults", "[Game][G26][Power]") {
    NF::PowerNode node;
    REQUIRE(node.netPower() == 0.f);
    REQUIRE_FALSE(node.isGenerator());
    REQUIRE_FALSE(node.isConsumer());
}

TEST_CASE("PowerNode offline", "[Game][G26][Power]") {
    NF::PowerNode node;
    node.generationRate = 100.f;
    node.online = false;
    REQUIRE(node.netPower() == 0.f);
}

TEST_CASE("PowerConduit defaults", "[Game][G26][Power]") {
    NF::PowerConduit c;
    REQUIRE(c.availableCapacity() == 100.f);
    REQUIRE(c.loadFraction() == 0.f);
    REQUIRE_FALSE(c.isOverloaded());
}

TEST_CASE("PowerConduit overloaded", "[Game][G26][Power]") {
    NF::PowerConduit c;
    c.maxCapacity = 50.f;
    c.currentLoad = 80.f;
    REQUIRE(c.isOverloaded());
    REQUIRE(c.availableCapacity() == 0.f);
}

TEST_CASE("PowerGrid add/remove nodes", "[Game][G26][Power]") {
    NF::PowerGrid grid;
    NF::PowerNode a; a.id = "a";
    NF::PowerNode b; b.id = "b";
    REQUIRE(grid.addNode(a));
    REQUIRE(grid.addNode(b));
    REQUIRE(grid.nodeCount() == 2);
    REQUIRE(grid.removeNode("a"));
    REQUIRE(grid.nodeCount() == 1);
    REQUIRE(grid.findNode("a") == nullptr);
    REQUIRE(grid.findNode("b") != nullptr);
}

TEST_CASE("PowerGrid reject duplicate nodes", "[Game][G26][Power]") {
    NF::PowerGrid grid;
    NF::PowerNode a; a.id = "a";
    REQUIRE(grid.addNode(a));
    REQUIRE_FALSE(grid.addNode(a));
    REQUIRE(grid.nodeCount() == 1);
}

TEST_CASE("PowerGrid max nodes", "[Game][G26][Power]") {
    REQUIRE(NF::PowerGrid::kMaxNodes == 64);
}

TEST_CASE("PowerGrid add/remove conduits", "[Game][G26][Power]") {
    NF::PowerGrid grid;
    NF::PowerConduit c1; c1.id = "c1";
    NF::PowerConduit c2; c2.id = "c2";
    REQUIRE(grid.addConduit(c1));
    REQUIRE(grid.addConduit(c2));
    REQUIRE(grid.conduitCount() == 2);
    REQUIRE(grid.removeConduit("c1"));
    REQUIRE(grid.conduitCount() == 1);
}

TEST_CASE("PowerGrid total generation and consumption", "[Game][G26][Power]") {
    NF::PowerGrid grid;
    NF::PowerNode gen; gen.id = "gen"; gen.generationRate = 200.f;
    NF::PowerNode con; con.id = "con"; con.consumptionRate = 50.f;
    grid.addNode(gen);
    grid.addNode(con);
    REQUIRE(grid.totalGeneration() == 200.f);
    REQUIRE(grid.totalConsumption() == 50.f);
    REQUIRE(grid.netPower() == 150.f);
    REQUIRE(grid.generatorCount() == 1);
    REQUIRE(grid.consumerCount() == 1);
}

TEST_CASE("PowerGrid deficit detection", "[Game][G26][Power]") {
    NF::PowerGrid grid;
    NF::PowerNode gen; gen.id = "gen"; gen.generationRate = 10.f;
    NF::PowerNode con; con.id = "con"; con.consumptionRate = 100.f;
    grid.addNode(gen);
    grid.addNode(con);
    REQUIRE(grid.isDeficit());
}

TEST_CASE("PowerGridSystem create grid", "[Game][G26][Power]") {
    NF::PowerGridSystem sys;
    int idx = sys.createGrid("Main");
    REQUIRE(idx == 0);
    REQUIRE(sys.gridCount() == 1);
    REQUIRE(sys.gridName(0) == "Main");
    REQUIRE(sys.grid(0) != nullptr);
}

TEST_CASE("PowerGridSystem max grids", "[Game][G26][Power]") {
    REQUIRE(NF::PowerGridSystem::kMaxGrids == 8);
    NF::PowerGridSystem sys;
    for (int i = 0; i < 8; ++i) {
        REQUIRE(sys.createGrid("g" + std::to_string(i)) == i);
    }
    REQUIRE(sys.createGrid("overflow") == -1);
}

TEST_CASE("PowerGridSystem shed load", "[Game][G26][Power]") {
    NF::PowerGridSystem sys;
    int idx = sys.createGrid("Test");
    NF::PowerGrid* g = sys.grid(idx);

    NF::PowerNode gen; gen.id = "gen"; gen.generationRate = 50.f;
    NF::PowerNode c1; c1.id = "c1"; c1.consumptionRate = 40.f; c1.priority = 1;
    NF::PowerNode c2; c2.id = "c2"; c2.consumptionRate = 40.f; c2.priority = 2;
    g->addNode(gen);
    g->addNode(c1);
    g->addNode(c2);

    REQUIRE(g->isDeficit());
    int shed = sys.shedLoad(idx);
    REQUIRE(shed >= 1);
    REQUIRE_FALSE(g->isDeficit());
    // Low-priority consumer (c1, priority=1) should be shed first
    REQUIRE_FALSE(g->findNode("c1")->online);
}

TEST_CASE("PowerGridSystem restore all", "[Game][G26][Power]") {
    NF::PowerGridSystem sys;
    int idx = sys.createGrid("Test");
    NF::PowerGrid* g = sys.grid(idx);

    NF::PowerNode gen; gen.id = "gen"; gen.generationRate = 50.f;
    NF::PowerNode c1; c1.id = "c1"; c1.consumptionRate = 40.f; c1.priority = 1;
    g->addNode(gen);
    g->addNode(c1);

    sys.shedLoad(idx);
    int restored = sys.restoreAll(idx);
    REQUIRE(restored >= 0);
    REQUIRE(g->findNode("c1")->online);
}

// ---------------------------------------------------------------------------
// G27 — Vehicle System Tests
// ---------------------------------------------------------------------------

TEST_CASE("VehicleType names cover all 8 types", "[Game][G27][Vehicle]") {
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Rover))     == "Rover");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Hoverbike)) == "Hoverbike");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Mech))      == "Mech");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Shuttle))   == "Shuttle");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Crawler))   == "Crawler");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Speeder))   == "Speeder");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Tank))      == "Tank");
    REQUIRE(std::string(NF::vehicleTypeName(NF::VehicleType::Dropship))  == "Dropship");
}

TEST_CASE("VehicleSeat enter and exit", "[Game][G27][Vehicle]") {
    NF::VehicleSeat seat;
    seat.id = "driver";
    seat.label = "Driver";
    seat.isDriver = true;

    REQUIRE_FALSE(seat.occupied);
    seat.enter("player_1");
    REQUIRE(seat.occupied);
    REQUIRE(seat.occupantId == "player_1");

    seat.exit();
    REQUIRE_FALSE(seat.occupied);
    REQUIRE(seat.occupantId.empty());
}

TEST_CASE("VehicleComponent damage and repair", "[Game][G27][Vehicle]") {
    NF::VehicleComponent comp;
    comp.id = "engine";
    comp.name = "Engine";

    REQUIRE(comp.healthFraction() == Catch::Approx(1.f));
    comp.applyDamage(30.f);
    REQUIRE(comp.health == Catch::Approx(70.f));
    REQUIRE(comp.functional);

    comp.repair(10.f);
    REQUIRE(comp.health == Catch::Approx(80.f));
}

TEST_CASE("VehicleComponent destroyed state", "[Game][G27][Vehicle]") {
    NF::VehicleComponent comp;
    comp.id = "wheel";
    comp.name = "Wheel";

    comp.applyDamage(150.f);
    REQUIRE(comp.isDestroyed());
    REQUIRE_FALSE(comp.functional);

    comp.repair(50.f);
    REQUIRE_FALSE(comp.isDestroyed());
    REQUIRE(comp.functional);
}

TEST_CASE("Vehicle add and remove seats", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    NF::VehicleSeat s1; s1.id = "s1"; s1.label = "Driver"; s1.isDriver = true;
    NF::VehicleSeat s2; s2.id = "s2"; s2.label = "Passenger";

    REQUIRE(v.addSeat(s1));
    REQUIRE(v.addSeat(s2));
    REQUIRE(v.seatCount() == 2);

    REQUIRE(v.removeSeat("s1"));
    REQUIRE(v.seatCount() == 1);
    REQUIRE(v.findSeat("s1") == nullptr);
    REQUIRE(v.findSeat("s2") != nullptr);
}

TEST_CASE("Vehicle seat duplicate rejection", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    NF::VehicleSeat s; s.id = "s1"; s.label = "Driver";
    REQUIRE(v.addSeat(s));
    REQUIRE_FALSE(v.addSeat(s));
    REQUIRE(v.seatCount() == 1);
}

TEST_CASE("Vehicle max seats", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    for (size_t i = 0; i < NF::Vehicle::kMaxSeats; ++i) {
        NF::VehicleSeat s; s.id = "seat_" + std::to_string(i);
        REQUIRE(v.addSeat(s));
    }
    REQUIRE(v.seatCount() == NF::Vehicle::kMaxSeats);

    NF::VehicleSeat extra; extra.id = "extra";
    REQUIRE_FALSE(v.addSeat(extra));
}

TEST_CASE("Vehicle add and remove components", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    NF::VehicleComponent c1; c1.id = "engine"; c1.name = "Engine";
    NF::VehicleComponent c2; c2.id = "wheel";  c2.name = "Wheel";

    REQUIRE(v.addComponent(c1));
    REQUIRE(v.addComponent(c2));
    REQUIRE(v.componentCount() == 2);

    REQUIRE(v.removeComponent("engine"));
    REQUIRE(v.componentCount() == 1);
    REQUIRE(v.findComponent("engine") == nullptr);
}

TEST_CASE("Vehicle occupant count", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    NF::VehicleSeat s1; s1.id = "s1"; s1.label = "Driver"; s1.isDriver = true;
    NF::VehicleSeat s2; s2.id = "s2"; s2.label = "Passenger";
    v.addSeat(s1);
    v.addSeat(s2);

    REQUIRE(v.occupantCount() == 0);
    v.findSeat("s1")->enter("p1");
    REQUIRE(v.occupantCount() == 1);
    v.findSeat("s2")->enter("p2");
    REQUIRE(v.occupantCount() == 2);
}

TEST_CASE("Vehicle has driver", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    NF::VehicleSeat driver; driver.id = "d"; driver.label = "Driver"; driver.isDriver = true;
    NF::VehicleSeat passenger; passenger.id = "p"; passenger.label = "Passenger";
    v.addSeat(driver);
    v.addSeat(passenger);

    REQUIRE_FALSE(v.hasDriver());
    v.findSeat("p")->enter("p1");
    REQUIRE_FALSE(v.hasDriver());
    v.findSeat("d")->enter("p2");
    REQUIRE(v.hasDriver());
}

TEST_CASE("Vehicle fuel consumption", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    v.setFuel(100.f);
    v.setMaxFuel(100.f);
    REQUIRE(v.hasFuel());
    REQUIRE(v.fuelFraction() == Catch::Approx(1.f));

    v.consumeFuel(40.f);
    REQUIRE(v.fuel() == Catch::Approx(60.f));

    v.consumeFuel(200.f);
    REQUIRE(v.fuel() == Catch::Approx(0.f));
    REQUIRE_FALSE(v.hasFuel());
}

TEST_CASE("Vehicle canOperate requires fuel engine operational driver", "[Game][G27][Vehicle]") {
    NF::Vehicle v;
    NF::VehicleSeat driver; driver.id = "d"; driver.isDriver = true;
    v.addSeat(driver);
    NF::VehicleComponent eng; eng.id = "eng"; eng.name = "Engine";
    v.addComponent(eng);

    REQUIRE_FALSE(v.canOperate()); // no engine active, no driver

    v.setEngineActive(true);
    REQUIRE_FALSE(v.canOperate()); // no driver

    v.findSeat("d")->enter("player");
    REQUIRE(v.canOperate());

    v.setFuel(0.f);
    REQUIRE_FALSE(v.canOperate()); // no fuel
}

TEST_CASE("VehicleSystem create vehicle", "[Game][G27][Vehicle]") {
    NF::VehicleSystem sys;
    int idx = sys.createVehicle("Rover Alpha", NF::VehicleType::Rover);
    REQUIRE(idx == 0);
    REQUIRE(sys.vehicleCount() == 1);

    auto* v = sys.vehicle(idx);
    REQUIRE(v != nullptr);
    REQUIRE(v->name() == "Rover Alpha");
    REQUIRE(v->type() == NF::VehicleType::Rover);
}

TEST_CASE("VehicleSystem max vehicles", "[Game][G27][Vehicle]") {
    NF::VehicleSystem sys;
    for (size_t i = 0; i < NF::VehicleSystem::kMaxVehicles; ++i) {
        int idx = sys.createVehicle("V" + std::to_string(i), NF::VehicleType::Mech);
        REQUIRE(idx >= 0);
    }
    REQUIRE(sys.vehicleCount() == NF::VehicleSystem::kMaxVehicles);
    int overflow = sys.createVehicle("Extra", NF::VehicleType::Tank);
    REQUIRE(overflow == -1);
}

TEST_CASE("VehicleSystem tick physics moves position", "[Game][G27][Vehicle]") {
    NF::VehicleSystem sys;
    int idx = sys.createVehicle("Speeder", NF::VehicleType::Speeder);
    auto* v = sys.vehicle(idx);

    // Set up a fully operable vehicle
    NF::VehicleSeat driver; driver.id = "d"; driver.isDriver = true;
    v->addSeat(driver);
    v->findSeat("d")->enter("pilot");
    NF::VehicleComponent eng; eng.id = "eng"; eng.name = "Engine";
    v->addComponent(eng);
    v->setEngineActive(true);
    v->setFuel(100.f);
    v->setSpeed(10.f);

    float startX = v->position().x;
    sys.tickVehicle(idx, 1.f);

    REQUIRE(v->position().x > startX);
    REQUIRE(v->fuel() < 100.f);
}

// ── G28 Research System Tests ────────────────────────────────────

TEST_CASE("ResearchCategory all 8 names", "[Game][G28][Research]") {
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Physics)) == "Physics");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Biology)) == "Biology");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Engineering)) == "Engineering");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Computing)) == "Computing");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Materials)) == "Materials");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Energy)) == "Energy");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Weapons)) == "Weapons");
    REQUIRE(std::string(NF::researchCategoryName(NF::ResearchCategory::Xenotech)) == "Xenotech");
}

TEST_CASE("ResearchProject defaults and progressFraction", "[Game][G28][Research]") {
    NF::ResearchProject proj;
    REQUIRE(proj.cost == 100.f);
    REQUIRE(proj.progress == 0.f);
    REQUIRE(proj.durationSeconds == 60.f);
    REQUIRE(proj.completed == false);
    REQUIRE(proj.progressFraction() == Catch::Approx(0.f));
}

TEST_CASE("ResearchProject addProgress completes", "[Game][G28][Research]") {
    NF::ResearchProject proj;
    proj.id = "p1";
    proj.cost = 50.f;
    proj.addProgress(30.f);
    REQUIRE(proj.progress == Catch::Approx(30.f));
    REQUIRE_FALSE(proj.isComplete());
    proj.addProgress(25.f);
    REQUIRE(proj.progress == Catch::Approx(50.f));
    REQUIRE(proj.isComplete());
    REQUIRE(proj.completed);
}

TEST_CASE("ResearchProject isComplete check", "[Game][G28][Research]") {
    NF::ResearchProject proj;
    proj.cost = 10.f;
    REQUIRE_FALSE(proj.isComplete());
    proj.completed = true;
    REQUIRE(proj.isComplete());
    proj.completed = false;
    proj.progress = 10.f;
    REQUIRE(proj.isComplete());
}

TEST_CASE("ResearchLab assign/clear project", "[Game][G28][Research]") {
    NF::ResearchLab lab;
    lab.setId("lab0");
    lab.setName("Alpha Lab");
    REQUIRE(lab.id() == "lab0");
    REQUIRE(lab.name() == "Alpha Lab");
    REQUIRE_FALSE(lab.hasActiveProject());
    REQUIRE(lab.assignProject("proj_a"));
    REQUIRE(lab.activeProjectId() == "proj_a");
    REQUIRE(lab.hasActiveProject());
    REQUIRE_FALSE(lab.assignProject("proj_a")); // same project
    lab.clearProject();
    REQUIRE_FALSE(lab.hasActiveProject());
}

TEST_CASE("ResearchLab completed tracking", "[Game][G28][Research]") {
    NF::ResearchLab lab;
    REQUIRE(lab.completedCount() == 0);
    lab.markCompleted("p1");
    lab.markCompleted("p2");
    REQUIRE(lab.completedCount() == 2);
    REQUIRE(lab.hasCompleted("p1"));
    REQUIRE(lab.hasCompleted("p2"));
    REQUIRE_FALSE(lab.hasCompleted("p3"));
    REQUIRE(lab.completedProjects().size() == 2);
}

TEST_CASE("ResearchLab budget management", "[Game][G28][Research]") {
    NF::ResearchLab lab;
    REQUIRE(lab.budget() == Catch::Approx(1000.f));
    REQUIRE(lab.hasBudget());
    lab.spendBudget(999.f);
    REQUIRE(lab.budget() == Catch::Approx(1.f));
    lab.spendBudget(5.f);
    REQUIRE(lab.budget() == Catch::Approx(0.f));
    REQUIRE_FALSE(lab.hasBudget());
    lab.setBudget(500.f);
    REQUIRE(lab.budget() == Catch::Approx(500.f));
}

TEST_CASE("ResearchTree add/remove/find projects", "[Game][G28][Research]") {
    NF::ResearchTree tree;
    NF::ResearchProject p; p.id = "rp1"; p.name = "Laser";
    REQUIRE(tree.addProject(p));
    REQUIRE(tree.projectCount() == 1);
    REQUIRE(tree.findProject("rp1") != nullptr);
    REQUIRE(tree.findProject("rp1")->name == "Laser");
    REQUIRE(tree.findProject("nope") == nullptr);
    REQUIRE(tree.removeProject("rp1"));
    REQUIRE(tree.projectCount() == 0);
    REQUIRE_FALSE(tree.removeProject("rp1"));
}

TEST_CASE("ResearchTree reject duplicate ids", "[Game][G28][Research]") {
    NF::ResearchTree tree;
    NF::ResearchProject p; p.id = "dup";
    REQUIRE(tree.addProject(p));
    REQUIRE_FALSE(tree.addProject(p));
    REQUIRE(tree.projectCount() == 1);
}

TEST_CASE("ResearchTree max projects", "[Game][G28][Research]") {
    NF::ResearchTree tree;
    for (size_t i = 0; i < NF::ResearchTree::kMaxProjects; ++i) {
        NF::ResearchProject p;
        p.id = "p_" + std::to_string(i);
        REQUIRE(tree.addProject(p));
    }
    REQUIRE(tree.projectCount() == NF::ResearchTree::kMaxProjects);
    NF::ResearchProject overflow; overflow.id = "overflow";
    REQUIRE_FALSE(tree.addProject(overflow));
}

TEST_CASE("ResearchTree prerequisitesMet", "[Game][G28][Research]") {
    NF::ResearchTree tree;
    NF::ResearchProject a; a.id = "a";
    NF::ResearchProject b; b.id = "b"; b.prerequisites = {"a"};
    tree.addProject(a);
    tree.addProject(b);

    std::vector<std::string> none;
    REQUIRE(tree.prerequisitesMet("a", none));
    REQUIRE_FALSE(tree.prerequisitesMet("b", none));

    std::vector<std::string> doneA = {"a"};
    REQUIRE(tree.prerequisitesMet("b", doneA));
}

TEST_CASE("ResearchTree projectsInCategory", "[Game][G28][Research]") {
    NF::ResearchTree tree;
    NF::ResearchProject p1; p1.id = "e1"; p1.category = NF::ResearchCategory::Energy;
    NF::ResearchProject p2; p2.id = "e2"; p2.category = NF::ResearchCategory::Energy;
    NF::ResearchProject p3; p3.id = "w1"; p3.category = NF::ResearchCategory::Weapons;
    tree.addProject(p1);
    tree.addProject(p2);
    tree.addProject(p3);
    auto energy = tree.projectsInCategory(NF::ResearchCategory::Energy);
    REQUIRE(energy.size() == 2);
    auto weapons = tree.projectsInCategory(NF::ResearchCategory::Weapons);
    REQUIRE(weapons.size() == 1);
    auto bio = tree.projectsInCategory(NF::ResearchCategory::Biology);
    REQUIRE(bio.empty());
}

TEST_CASE("ResearchSystem create lab", "[Game][G28][Research]") {
    NF::ResearchSystem sys;
    REQUIRE(sys.labCount() == 0);
    int idx = sys.createLab("Lab Alpha");
    REQUIRE(idx == 0);
    REQUIRE(sys.labCount() == 1);
    REQUIRE(sys.lab(0)->name() == "Lab Alpha");
    REQUIRE(sys.lab(0)->id() == "lab_0");
    REQUIRE(sys.lab(-1) == nullptr);
    REQUIRE(sys.lab(99) == nullptr);
}

TEST_CASE("ResearchSystem max labs", "[Game][G28][Research]") {
    NF::ResearchSystem sys;
    for (size_t i = 0; i < NF::ResearchSystem::kMaxLabs; ++i) {
        int idx = sys.createLab("L" + std::to_string(i));
        REQUIRE(idx >= 0);
    }
    REQUIRE(sys.labCount() == NF::ResearchSystem::kMaxLabs);
    REQUIRE(sys.createLab("Extra") == -1);
}

TEST_CASE("ResearchSystem tick advances research", "[Game][G28][Research]") {
    NF::ResearchSystem sys;
    NF::ResearchProject proj;
    proj.id = "tp";
    proj.name = "Test Project";
    proj.cost = 10.f;
    sys.tree().addProject(proj);

    int labIdx = sys.createLab("Main");
    sys.lab(labIdx)->setResearchRate(5.f);
    REQUIRE(sys.assignProject(labIdx, "tp"));
    REQUIRE(sys.activeLabCount() == 1);

    sys.tick(1.f); // 5 points
    auto* p = sys.tree().findProject("tp");
    REQUIRE(p->progress == Catch::Approx(5.f));
    REQUIRE_FALSE(p->isComplete());

    sys.tick(1.f); // another 5 points -> 10 total
    REQUIRE(p->isComplete());
    REQUIRE(sys.discoveries() == 1);
    REQUIRE(sys.activeLabCount() == 0);
    REQUIRE(sys.lab(labIdx)->hasCompleted("tp"));
}

// ── G29 Tests: Diplomacy System ─────────────────────────────────

TEST_CASE("DiplomacyAction names", "[Game][G29][Diplomacy]") {
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::TradeAgreement))    == "TradeAgreement");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::NonAggression))     == "NonAggression");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::MilitaryAlliance))  == "MilitaryAlliance");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::TechSharing))       == "TechSharing");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::TerritoryExchange)) == "TerritoryExchange");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::Embargo))           == "Embargo");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::WarDeclaration))    == "WarDeclaration");
    REQUIRE(std::string(NF::diplomacyActionName(NF::DiplomacyAction::PeaceTreaty))       == "PeaceTreaty");
}

TEST_CASE("DiplomaticStance names", "[Game][G29][Diplomacy]") {
    REQUIRE(std::string(NF::diplomaticStanceName(NF::DiplomaticStance::Hostile))    == "Hostile");
    REQUIRE(std::string(NF::diplomaticStanceName(NF::DiplomaticStance::Unfriendly)) == "Unfriendly");
    REQUIRE(std::string(NF::diplomaticStanceName(NF::DiplomaticStance::Neutral))    == "Neutral");
    REQUIRE(std::string(NF::diplomaticStanceName(NF::DiplomaticStance::Friendly))   == "Friendly");
    REQUIRE(std::string(NF::diplomaticStanceName(NF::DiplomaticStance::Allied))     == "Allied");
}

TEST_CASE("DiplomaticRelation adjustOpinion clamping + stance", "[Game][G29][Diplomacy]") {
    NF::DiplomaticRelation rel;
    REQUIRE(rel.opinion == 0.f);
    REQUIRE(rel.stance == NF::DiplomaticStance::Neutral);

    rel.adjustOpinion(80.f);
    REQUIRE(rel.opinion == Catch::Approx(80.f));
    REQUIRE(rel.stance == NF::DiplomaticStance::Allied);

    rel.adjustOpinion(50.f); // clamps at 100
    REQUIRE(rel.opinion == Catch::Approx(100.f));
    REQUIRE(rel.stance == NF::DiplomaticStance::Allied);

    rel.adjustOpinion(-180.f); // 100 + (-180) = -80, clamped at -100
    REQUIRE(rel.opinion == Catch::Approx(-80.f));
    REQUIRE(rel.stance == NF::DiplomaticStance::Hostile);
}

TEST_CASE("DiplomaticRelation declareWar/declarePeace", "[Game][G29][Diplomacy]") {
    NF::DiplomaticRelation rel;
    rel.declareWar();
    REQUIRE(rel.atWar);
    REQUIRE(rel.opinion == Catch::Approx(-100.f));
    REQUIRE(rel.isHostile());

    rel.declarePeace();
    REQUIRE_FALSE(rel.atWar);
    REQUIRE(rel.opinion == Catch::Approx(-80.f)); // -100 + 20
    REQUIRE(rel.stance == NF::DiplomaticStance::Hostile); // still hostile at -80
}

TEST_CASE("Treaty tick + expiration", "[Game][G29][Diplomacy]") {
    NF::Treaty t;
    t.durationSeconds = 10.f;
    REQUIRE(t.isActive());
    REQUIRE(t.remainingSeconds() == Catch::Approx(10.f));

    t.tick(7.f);
    REQUIRE(t.isActive());
    REQUIRE(t.remainingSeconds() == Catch::Approx(3.f));

    t.tick(5.f); // exceeds duration
    REQUIRE_FALSE(t.isActive());
    REQUIRE(t.expired);
}

TEST_CASE("Treaty permanent stays active", "[Game][G29][Diplomacy]") {
    NF::Treaty t;
    t.durationSeconds = 0.f; // permanent
    REQUIRE(t.isPermanent());
    REQUIRE(t.remainingSeconds() == Catch::Approx(-1.f));

    t.tick(1000.f);
    REQUIRE(t.isActive()); // permanent never expires
}

TEST_CASE("Treaty revoke", "[Game][G29][Diplomacy]") {
    NF::Treaty t;
    t.durationSeconds = 100.f;
    REQUIRE(t.isActive());
    t.revoke();
    REQUIRE_FALSE(t.isActive());
}

TEST_CASE("DiplomaticChannel addRelation + duplicates rejected", "[Game][G29][Diplomacy]") {
    NF::DiplomaticChannel ch("Alpha");
    REQUIRE(ch.addRelation("Beta"));
    REQUIRE(ch.relationCount() == 1);
    REQUIRE_FALSE(ch.addRelation("Beta")); // duplicate
    REQUIRE(ch.relationCount() == 1);
}

TEST_CASE("DiplomaticChannel self-relation rejected", "[Game][G29][Diplomacy]") {
    NF::DiplomaticChannel ch("Alpha");
    REQUIRE_FALSE(ch.addRelation("Alpha"));
}

TEST_CASE("DiplomaticChannel proposeTreaty + opinion adjustment", "[Game][G29][Diplomacy]") {
    NF::DiplomaticChannel ch("Alpha");
    ch.addRelation("Beta");
    REQUIRE(ch.proposeTreaty("Beta", NF::DiplomacyAction::TradeAgreement));
    auto* rel = ch.findRelation("Beta");
    REQUIRE(rel->opinion == Catch::Approx(10.f));
    REQUIRE(ch.activeTreatyCount() == 1);
}

TEST_CASE("DiplomaticChannel war blocks non-peace treaties", "[Game][G29][Diplomacy]") {
    NF::DiplomaticChannel ch("Alpha");
    ch.addRelation("Beta");
    ch.proposeTreaty("Beta", NF::DiplomacyAction::WarDeclaration);
    auto* rel = ch.findRelation("Beta");
    REQUIRE(rel->atWar);

    REQUIRE_FALSE(ch.proposeTreaty("Beta", NF::DiplomacyAction::TradeAgreement));
    REQUIRE(ch.proposeTreaty("Beta", NF::DiplomacyAction::PeaceTreaty));
    REQUIRE_FALSE(rel->atWar);
}

TEST_CASE("DiplomaticChannel alliedCount/hostileCount", "[Game][G29][Diplomacy]") {
    NF::DiplomaticChannel ch("Alpha");
    ch.addRelation("Beta", 80.f);  // Allied
    ch.addRelation("Gamma", -60.f); // Hostile
    ch.addRelation("Delta", 0.f);   // Neutral
    REQUIRE(ch.alliedCount() == 1);
    REQUIRE(ch.hostileCount() == 1);
}

TEST_CASE("DiplomacySystem createChannel + duplicate rejection", "[Game][G29][Diplomacy]") {
    NF::DiplomacySystem sys;
    REQUIRE(sys.createChannel("Alpha") == 0);
    REQUIRE(sys.createChannel("Beta") == 1);
    REQUIRE(sys.createChannel("Alpha") == -1); // duplicate
    REQUIRE(sys.channelCount() == 2);
}

TEST_CASE("DiplomacySystem establishRelation bidirectional", "[Game][G29][Diplomacy]") {
    NF::DiplomacySystem sys;
    sys.createChannel("Alpha");
    sys.createChannel("Beta");
    REQUIRE(sys.establishRelation("Alpha", "Beta", 30.f));

    auto* chA = sys.channelByName("Alpha");
    auto* chB = sys.channelByName("Beta");
    REQUIRE(chA->findRelation("Beta") != nullptr);
    REQUIRE(chB->findRelation("Alpha") != nullptr);
    REQUIRE(chA->findRelation("Beta")->opinion == Catch::Approx(30.f));
}

TEST_CASE("DiplomacySystem treaty + tick expiration", "[Game][G29][Diplomacy]") {
    NF::DiplomacySystem sys;
    sys.createChannel("Alpha");
    sys.createChannel("Beta");
    sys.establishRelation("Alpha", "Beta");

    REQUIRE(sys.proposeTreaty("Alpha", "Beta", NF::DiplomacyAction::TradeAgreement, 5.f));
    REQUIRE(sys.totalActiveTreaties() == 1);

    sys.tick(3.f);
    REQUIRE(sys.totalActiveTreaties() == 1);

    sys.tick(3.f); // total 6 > 5 -> expired
    REQUIRE(sys.totalActiveTreaties() == 0);
    REQUIRE(sys.tickCount() == 2);
}

// ── G30 Tests: Espionage System ─────────────────────────────────

TEST_CASE("EspionageMissionType names", "[Game][G30][Espionage]") {
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::Infiltration)) == "Infiltration");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::Sabotage))     == "Sabotage");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::Surveillance))  == "Surveillance");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::DataTheft))    == "DataTheft");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::Assassination)) == "Assassination");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::Recruitment))  == "Recruitment");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::CounterIntel)) == "CounterIntel");
    REQUIRE(std::string(NF::espionageMissionTypeName(NF::EspionageMissionType::Extraction))   == "Extraction");
}

TEST_CASE("SpyAgent defaults + availability", "[Game][G30][Espionage]") {
    NF::SpyAgent agent;
    REQUIRE(agent.isAvailable());
    REQUIRE_FALSE(agent.isActive());
    REQUIRE(agent.skillLevel == Catch::Approx(1.f));
    REQUIRE(agent.loyalty == Catch::Approx(1.f));
    REQUIRE(agent.coverStrength == Catch::Approx(1.f));
}

TEST_CASE("SpyAgent deploy/recall/compromise/capture/rescue", "[Game][G30][Espionage]") {
    NF::SpyAgent agent;
    agent.deploy();
    REQUIRE(agent.isActive());
    REQUIRE_FALSE(agent.isAvailable());

    agent.recall();
    REQUIRE(agent.isAvailable());

    agent.compromise();
    REQUIRE_FALSE(agent.isAvailable());
    REQUIRE(agent.coverStrength == Catch::Approx(0.f));

    agent.capture();
    REQUIRE(agent.captured);
    REQUIRE_FALSE(agent.isAvailable());

    agent.rescue();
    REQUIRE(agent.isAvailable());
    REQUIRE(agent.coverStrength == Catch::Approx(0.3f));
}

TEST_CASE("EspionageMission advance + completion", "[Game][G30][Espionage]") {
    NF::EspionageMission m;
    m.durationSeconds = 10.f;
    REQUIRE(m.progressFraction() == Catch::Approx(0.f));

    m.advance(7.f);
    REQUIRE(m.progressFraction() == Catch::Approx(0.7f));
    REQUIRE(m.isInProgress());
    REQUIRE_FALSE(m.isComplete());

    m.advance(5.f); // exceeds duration
    REQUIRE(m.isComplete());
    REQUIRE(m.progress == Catch::Approx(1.f));
}

TEST_CASE("EspionageMission fail", "[Game][G30][Espionage]") {
    NF::EspionageMission m;
    m.durationSeconds = 10.f;
    m.advance(3.f);
    m.fail();
    REQUIRE(m.isFailed());
    m.advance(5.f); // no further progress after fail
    REQUIRE(m.elapsedSeconds == Catch::Approx(3.f));
}

TEST_CASE("IntelligenceNetwork addAgent + duplicate rejection", "[Game][G30][Espionage]") {
    NF::IntelligenceNetwork net("Alpha");
    NF::SpyAgent a1; a1.id = "spy1"; a1.name = "Agent A";
    NF::SpyAgent a2; a2.id = "spy1"; a2.name = "Duplicate";
    REQUIRE(net.addAgent(a1));
    REQUIRE(net.agentCount() == 1);
    REQUIRE_FALSE(net.addAgent(a2)); // duplicate id
    REQUIRE(net.agentCount() == 1);
}

TEST_CASE("IntelligenceNetwork removeAgent", "[Game][G30][Espionage]") {
    NF::IntelligenceNetwork net("Alpha");
    NF::SpyAgent a; a.id = "spy1";
    net.addAgent(a);
    REQUIRE(net.removeAgent("spy1"));
    REQUIRE(net.agentCount() == 0);
    REQUIRE_FALSE(net.removeAgent("nonexistent"));
}

TEST_CASE("IntelligenceNetwork launchMission + agent deployed", "[Game][G30][Espionage]") {
    NF::IntelligenceNetwork net("Alpha");
    NF::SpyAgent a; a.id = "spy1";
    net.addAgent(a);
    REQUIRE(net.launchMission("spy1", NF::EspionageMissionType::Surveillance, "Beta", 5.f));
    REQUIRE(net.missionCount() == 1);
    REQUIRE(net.activeMissionCount() == 1);
    REQUIRE(net.findAgent("spy1")->deployed);
    // can't launch again while deployed
    REQUIRE_FALSE(net.launchMission("spy1", NF::EspionageMissionType::Sabotage, "Gamma", 3.f));
}

TEST_CASE("IntelligenceNetwork tick completes mission", "[Game][G30][Espionage]") {
    NF::IntelligenceNetwork net("Alpha");
    NF::SpyAgent a; a.id = "spy1";
    net.addAgent(a);
    net.launchMission("spy1", NF::EspionageMissionType::DataTheft, "Beta", 5.f);

    net.tick(3.f);
    REQUIRE(net.activeMissionCount() == 1);
    REQUIRE(net.completedMissionCount() == 0);

    net.tick(3.f); // total 6 > 5 -> complete
    REQUIRE(net.completedMissionCount() == 1);
    REQUIRE(net.intelGathered() == 1);
    REQUIRE(net.findAgent("spy1")->missionsCompleted == 1);
    REQUIRE_FALSE(net.findAgent("spy1")->deployed); // recalled
}

TEST_CASE("IntelligenceNetwork availableAgentCount", "[Game][G30][Espionage]") {
    NF::IntelligenceNetwork net("Alpha");
    NF::SpyAgent a1; a1.id = "spy1";
    NF::SpyAgent a2; a2.id = "spy2";
    net.addAgent(a1);
    net.addAgent(a2);
    REQUIRE(net.availableAgentCount() == 2);
    net.launchMission("spy1", NF::EspionageMissionType::Infiltration, "Beta", 10.f);
    REQUIRE(net.availableAgentCount() == 1);
}

TEST_CASE("EspionageSystem createNetwork + duplicate rejection", "[Game][G30][Espionage]") {
    NF::EspionageSystem sys;
    REQUIRE(sys.createNetwork("Alpha") == 0);
    REQUIRE(sys.createNetwork("Beta") == 1);
    REQUIRE(sys.createNetwork("Alpha") == -1); // duplicate
    REQUIRE(sys.networkCount() == 2);
}

TEST_CASE("EspionageSystem recruit + launchMission + tick", "[Game][G30][Espionage]") {
    NF::EspionageSystem sys;
    sys.createNetwork("Alpha");
    NF::SpyAgent a; a.id = "spy1";
    REQUIRE(sys.recruitAgent("Alpha", a));

    REQUIRE(sys.launchMission("Alpha", "spy1", NF::EspionageMissionType::Sabotage, "Beta", 5.f));
    REQUIRE(sys.totalActiveMissions() == 1);

    sys.tick(3.f);
    REQUIRE(sys.totalActiveMissions() == 1);

    sys.tick(3.f); // total 6 > 5 -> complete
    REQUIRE(sys.totalActiveMissions() == 0);
    REQUIRE(sys.totalIntelGathered() == 1);
    REQUIRE(sys.tickCount() == 2);
}

TEST_CASE("EspionageSystem networkByName", "[Game][G30][Espionage]") {
    NF::EspionageSystem sys;
    sys.createNetwork("Alpha");
    sys.createNetwork("Beta");
    REQUIRE(sys.networkByName("Alpha") != nullptr);
    REQUIRE(sys.networkByName("Alpha")->owner() == "Alpha");
    REQUIRE(sys.networkByName("Gamma") == nullptr);
}

TEST_CASE("IntelligenceNetwork cannot remove deployed agent", "[Game][G30][Espionage]") {
    NF::IntelligenceNetwork net("Alpha");
    NF::SpyAgent a; a.id = "spy1";
    net.addAgent(a);
    net.launchMission("spy1", NF::EspionageMissionType::Infiltration, "Beta", 10.f);
    REQUIRE_FALSE(net.removeAgent("spy1")); // deployed
    REQUIRE(net.agentCount() == 1);
}

TEST_CASE("EspionageMission zero duration never completes via advance", "[Game][G30][Espionage]") {
    NF::EspionageMission m;
    m.durationSeconds = 0.f;
    m.advance(100.f);
    REQUIRE_FALSE(m.isComplete());
    REQUIRE(m.progressFraction() == Catch::Approx(0.f));
}

// ── G31 Colony Management Tests ─────────────────────────────────

TEST_CASE("ColonyRole all 8 names", "[Game][G31][Colony]") {
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Governor))  == "Governor");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Engineer))  == "Engineer");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Scientist)) == "Scientist");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Miner))     == "Miner");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Farmer))    == "Farmer");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Guard))     == "Guard");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Medic))     == "Medic");
    REQUIRE(std::string(NF::colonyRoleName(NF::ColonyRole::Explorer))  == "Explorer");
}

TEST_CASE("Colonist defaults and helpers", "[Game][G31][Colony]") {
    NF::Colonist c;
    REQUIRE(c.role == NF::ColonyRole::Farmer);
    REQUIRE(c.isHealthy());
    REQUIRE(c.isProductive());
    REQUIRE(c.effectiveOutput() == Catch::Approx(1.f));

    c.takeDamage(0.9f);
    REQUIRE_FALSE(c.isHealthy());
    REQUIRE_FALSE(c.isProductive());

    c.heal(0.5f);
    REQUIRE(c.isHealthy());
}

TEST_CASE("Colonist morale affects productivity", "[Game][G31][Colony]") {
    NF::Colonist c;
    c.reduceMorale(0.9f);
    REQUIRE_FALSE(c.isProductive());
    c.boostMorale(0.5f);
    REQUIRE(c.isProductive());
}

TEST_CASE("ColonyBuilding operational checks", "[Game][G31][Colony]") {
    NF::ColonyBuilding b;
    b.id = "b1"; b.name = "Habitat"; b.capacity = 10;
    REQUIRE(b.isOperational());
    REQUIRE_FALSE(b.isFull());
    REQUIRE(b.availableSlots() == 10);

    b.damage();
    REQUIRE_FALSE(b.isOperational()); // damaged
    b.repair();
    REQUIRE(b.isOperational());

    b.powered = false;
    REQUIRE_FALSE(b.isOperational()); // unpowered
}

TEST_CASE("Colony addColonist + duplicate rejection", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::Colonist c1; c1.id = "c1"; c1.name = "Alice";
    NF::Colonist c2; c2.id = "c1"; c2.name = "Duplicate";

    REQUIRE(colony.addColonist(c1));
    REQUIRE(colony.population() == 1);
    REQUIRE_FALSE(colony.addColonist(c2));
    REQUIRE(colony.population() == 1);
}

TEST_CASE("Colony removeColonist", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::Colonist c; c.id = "c1";
    colony.addColonist(c);
    REQUIRE(colony.removeColonist("c1"));
    REQUIRE(colony.population() == 0);
    REQUIRE_FALSE(colony.removeColonist("nonexistent"));
}

TEST_CASE("Colony findColonist", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::Colonist c; c.id = "c1"; c.name = "Bob";
    colony.addColonist(c);
    REQUIRE(colony.findColonist("c1") != nullptr);
    REQUIRE(colony.findColonist("c1")->name == "Bob");
    REQUIRE(colony.findColonist("c2") == nullptr);
}

TEST_CASE("Colony addBuilding + removeBuilding", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::ColonyBuilding b; b.id = "b1"; b.name = "Lab"; b.capacity = 5;
    REQUIRE(colony.addBuilding(b));
    REQUIRE(colony.buildingCount() == 1);

    REQUIRE(colony.removeBuilding("b1"));
    REQUIRE(colony.buildingCount() == 0);
}

TEST_CASE("Colony removeBuilding fails if occupied", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::ColonyBuilding b; b.id = "b1"; b.capacity = 5; b.occupants = 3;
    colony.addBuilding(b);
    REQUIRE_FALSE(colony.removeBuilding("b1")); // occupied
}

TEST_CASE("Colony tick computes morale and output", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::Colonist c1; c1.id = "c1"; c1.morale = 0.8f;
    NF::Colonist c2; c2.id = "c2"; c2.morale = 0.6f;
    colony.addColonist(c1);
    colony.addColonist(c2);

    colony.tick(1.f);
    REQUIRE(colony.averageMorale() == Catch::Approx(0.7f));
    REQUIRE(colony.productiveCount() == 2);
    REQUIRE(colony.resourceOutput() > 0.f);
}

TEST_CASE("Colony colonistsWithRole", "[Game][G31][Colony]") {
    NF::Colony colony("Alpha");
    NF::Colonist c1; c1.id = "c1"; c1.role = NF::ColonyRole::Miner;
    NF::Colonist c2; c2.id = "c2"; c2.role = NF::ColonyRole::Miner;
    NF::Colonist c3; c3.id = "c3"; c3.role = NF::ColonyRole::Scientist;
    colony.addColonist(c1);
    colony.addColonist(c2);
    colony.addColonist(c3);
    REQUIRE(colony.colonistsWithRole(NF::ColonyRole::Miner) == 2);
    REQUIRE(colony.colonistsWithRole(NF::ColonyRole::Scientist) == 1);
    REQUIRE(colony.colonistsWithRole(NF::ColonyRole::Governor) == 0);
}

TEST_CASE("ColonySystem createColony + duplicate rejection", "[Game][G31][Colony]") {
    NF::ColonySystem sys;
    REQUIRE(sys.createColony("Alpha") == 0);
    REQUIRE(sys.createColony("Beta") == 1);
    REQUIRE(sys.createColony("Alpha") == -1);
    REQUIRE(sys.colonyCount() == 2);
}

TEST_CASE("ColonySystem colonyByName", "[Game][G31][Colony]") {
    NF::ColonySystem sys;
    sys.createColony("Alpha");
    REQUIRE(sys.colonyByName("Alpha") != nullptr);
    REQUIRE(sys.colonyByName("Alpha")->name() == "Alpha");
    REQUIRE(sys.colonyByName("Gamma") == nullptr);
}

TEST_CASE("ColonySystem addColonistToColony + tick", "[Game][G31][Colony]") {
    NF::ColonySystem sys;
    sys.createColony("Alpha");

    NF::Colonist c; c.id = "c1"; c.name = "Alice";
    REQUIRE(sys.addColonistToColony("Alpha", c));
    REQUIRE(sys.totalPopulation() == 1);

    sys.tick(1.f);
    sys.tick(1.f);
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.totalResourceOutput() > 0.f);
}

// ── G32 Archaeology System Tests ────────────────────────────────

TEST_CASE("ArtifactRarity all 8 names", "[Game][G32][Archaeology]") {
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Common))    == "Common");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Uncommon))  == "Uncommon");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Rare))      == "Rare");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Epic))      == "Epic");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Legendary)) == "Legendary");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Ancient))   == "Ancient");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Mythic))    == "Mythic");
    REQUIRE(std::string(NF::artifactRarityName(NF::ArtifactRarity::Unique))    == "Unique");
}

TEST_CASE("Artifact defaults and research", "[Game][G32][Archaeology]") {
    NF::Artifact a;
    REQUIRE_FALSE(a.isDecoded());
    REQUIRE_FALSE(a.isResearching());
    REQUIRE(a.progressFraction() == Catch::Approx(0.f));

    a.advanceResearch(0.5f);
    REQUIRE(a.isResearching());
    REQUIRE(a.progressFraction() == Catch::Approx(0.5f));

    a.advanceResearch(0.6f); // exceeds 1.0
    REQUIRE(a.isDecoded());
    REQUIRE(a.progressFraction() == Catch::Approx(1.f));
}

TEST_CASE("Artifact no research after decoded", "[Game][G32][Archaeology]") {
    NF::Artifact a;
    a.advanceResearch(1.f);
    REQUIRE(a.isDecoded());

    a.advanceResearch(0.5f); // should be no-op
    REQUIRE(a.progressFraction() == Catch::Approx(1.f));
}

TEST_CASE("ExcavationSite lifecycle", "[Game][G32][Archaeology]") {
    NF::ExcavationSite site;
    site.id = "s1"; site.location = "Mars"; site.durationSeconds = 10.f;

    REQUIRE_FALSE(site.isActive());
    REQUIRE_FALSE(site.isComplete());

    site.activate();
    REQUIRE(site.isActive());

    site.advance(5.f);
    REQUIRE(site.progressFraction() == Catch::Approx(0.5f));

    site.advance(6.f); // total 11 > 10
    REQUIRE(site.isComplete());
    REQUIRE(site.progress == Catch::Approx(1.f));
}

TEST_CASE("ExcavationSite inactive does not advance", "[Game][G32][Archaeology]") {
    NF::ExcavationSite site;
    site.durationSeconds = 10.f;
    site.advance(5.f); // not activated
    REQUIRE(site.progressFraction() == Catch::Approx(0.f));
}

TEST_CASE("ArtifactCollection addArtifact + duplicate rejection", "[Game][G32][Archaeology]") {
    NF::ArtifactCollection coll("Player1");
    NF::Artifact a1; a1.id = "a1"; a1.name = "Shard";
    NF::Artifact a2; a2.id = "a1"; a2.name = "Duplicate";

    REQUIRE(coll.addArtifact(a1));
    REQUIRE(coll.artifactCount() == 1);
    REQUIRE_FALSE(coll.addArtifact(a2));
    REQUIRE(coll.artifactCount() == 1);
}

TEST_CASE("ArtifactCollection removeArtifact", "[Game][G32][Archaeology]") {
    NF::ArtifactCollection coll("Player1");
    NF::Artifact a; a.id = "a1";
    coll.addArtifact(a);
    REQUIRE(coll.removeArtifact("a1"));
    REQUIRE(coll.artifactCount() == 0);
    REQUIRE_FALSE(coll.removeArtifact("nonexistent"));
}

TEST_CASE("ArtifactCollection decodedCount + rarityCount", "[Game][G32][Archaeology]") {
    NF::ArtifactCollection coll("Player1");
    NF::Artifact a1; a1.id = "a1"; a1.rarity = NF::ArtifactRarity::Rare; a1.decoded = true;
    NF::Artifact a2; a2.id = "a2"; a2.rarity = NF::ArtifactRarity::Rare;
    NF::Artifact a3; a3.id = "a3"; a3.rarity = NF::ArtifactRarity::Epic; a3.decoded = true;
    coll.addArtifact(a1);
    coll.addArtifact(a2);
    coll.addArtifact(a3);

    REQUIRE(coll.decodedCount() == 2);
    REQUIRE(coll.rarityCount(NF::ArtifactRarity::Rare) == 2);
    REQUIRE(coll.rarityCount(NF::ArtifactRarity::Epic) == 1);
    REQUIRE(coll.rarityCount(NF::ArtifactRarity::Common) == 0);
}

TEST_CASE("ArchaeologySystem createSite + duplicate rejection", "[Game][G32][Archaeology]") {
    NF::ArchaeologySystem sys;
    REQUIRE(sys.createSite("s1", "Mars", 10.f) == 0);
    REQUIRE(sys.createSite("s2", "Europa", 20.f) == 1);
    REQUIRE(sys.createSite("s1", "Venus", 5.f) == -1);
    REQUIRE(sys.siteCount() == 2);
}

TEST_CASE("ArchaeologySystem activateSite + tick", "[Game][G32][Archaeology]") {
    NF::ArchaeologySystem sys;
    sys.createSite("s1", "Mars", 5.f);
    REQUIRE(sys.activateSite("s1"));
    REQUIRE(sys.activeSiteCount() == 1);

    sys.tick(3.f);
    REQUIRE(sys.activeSiteCount() == 1);
    REQUIRE(sys.completedSiteCount() == 0);

    sys.tick(3.f); // total 6 > 5
    REQUIRE(sys.completedSiteCount() == 1);
    REQUIRE(sys.totalArtifactsFound() == 1);
}

TEST_CASE("ArchaeologySystem addCollection + collectionByOwner", "[Game][G32][Archaeology]") {
    NF::ArchaeologySystem sys;
    REQUIRE(sys.addCollection("Player1"));
    REQUIRE(sys.collectionCount() == 1);
    REQUIRE_FALSE(sys.addCollection("Player1")); // duplicate

    auto* coll = sys.collectionByOwner("Player1");
    REQUIRE(coll != nullptr);
    REQUIRE(coll->owner() == "Player1");
    REQUIRE(sys.collectionByOwner("Player2") == nullptr);
}

TEST_CASE("ArchaeologySystem totalDecodedArtifacts", "[Game][G32][Archaeology]") {
    NF::ArchaeologySystem sys;
    sys.addCollection("Player1");
    auto* coll = sys.collectionByOwner("Player1");

    NF::Artifact a1; a1.id = "a1"; a1.decoded = true;
    NF::Artifact a2; a2.id = "a2";
    coll->addArtifact(a1);
    coll->addArtifact(a2);

    REQUIRE(sys.totalDecodedArtifacts() == 1);
}



// ── G33 Tests: Migration System ──────────────────────────────────────────────

TEST_CASE("MigrationTrigger names", "[Game][G33][Migration]") {
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Economic))      == "Economic");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Environmental)) == "Environmental");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Political))     == "Political");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Cultural))      == "Cultural");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::War))           == "War");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Famine))        == "Famine");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Disease))       == "Disease");
    REQUIRE(std::string(NF::migrationTriggerName(NF::MigrationTrigger::Opportunity))   == "Opportunity");
}

TEST_CASE("Migrant advance + arrive", "[Game][G33][Migration]") {
    NF::Migrant m;
    m.id = "m1"; m.name = "Alice";
    m.originRegion = "Zone A"; m.destinationRegion = "Zone B";
    REQUIRE_FALSE(m.isInTransit());
    REQUIRE_FALSE(m.hasArrived());

    m.advance(0.5f); // 0.5 * 0.1 handled by route, direct advance here
    // advance takes amount directly in Migrant
    NF::Migrant m2;
    m2.id = "m2";
    m2.advance(0.3f);
    REQUIRE(m2.isInTransit());
    REQUIRE_FALSE(m2.hasArrived());

    m2.advance(0.8f); // total > 1
    REQUIRE(m2.hasArrived());
    REQUIRE_FALSE(m2.isInTransit());
}

TEST_CASE("MigrationWave completionFraction + isComplete", "[Game][G33][Migration]") {
    NF::MigrationWave wave;
    wave.id = "wave1";
    wave.totalMigrants = 100;
    wave.arrivedCount = 0;
    REQUIRE_FALSE(wave.isComplete());
    REQUIRE(wave.completionFraction() == Catch::Approx(0.f));

    wave.arrivedCount = 50;
    REQUIRE(wave.completionFraction() == Catch::Approx(0.5f));

    wave.arrivedCount = 100;
    REQUIRE(wave.isComplete());
    REQUIRE(wave.completionFraction() == Catch::Approx(1.f));
}

TEST_CASE("MigrationRoute addMigrant + duplicate rejection", "[Game][G33][Migration]") {
    NF::MigrationRoute route("Zone A", "Zone B");
    NF::Migrant m1; m1.id = "m1";
    NF::Migrant m2; m2.id = "m1"; // duplicate id

    REQUIRE(route.addMigrant(m1));
    REQUIRE(route.migrantCount() == 1);
    REQUIRE_FALSE(route.addMigrant(m2));
    REQUIRE(route.migrantCount() == 1);
}

TEST_CASE("MigrationRoute removeMigrant", "[Game][G33][Migration]") {
    NF::MigrationRoute route("A", "B");
    NF::Migrant m; m.id = "m1";
    route.addMigrant(m);
    REQUIRE(route.removeMigrant("m1"));
    REQUIRE(route.migrantCount() == 0);
    REQUIRE_FALSE(route.removeMigrant("nonexistent"));
}

TEST_CASE("MigrationRoute arrivedCount + inTransitCount", "[Game][G33][Migration]") {
    NF::MigrationRoute route("A", "B");
    NF::Migrant m1; m1.id = "m1"; m1.journeyProgress = 0.5f; // in transit
    NF::Migrant m2; m2.id = "m2"; m2.arrived = true;
    NF::Migrant m3; m3.id = "m3"; m3.arrived = true;
    route.addMigrant(m1);
    route.addMigrant(m2);
    route.addMigrant(m3);

    REQUIRE(route.arrivedCount()  == 2);
    REQUIRE(route.inTransitCount() == 1);
}

TEST_CASE("MigrationRoute tick advances migrants", "[Game][G33][Migration]") {
    NF::MigrationRoute route("A", "B");
    NF::Migrant m; m.id = "m1"; m.journeyProgress = 0.f;
    route.addMigrant(m);

    route.tick(1.f); // advances by 0.1
    REQUIRE(route.tickCount() == 1);
    auto* found = route.findMigrant("m1");
    REQUIRE(found != nullptr);
    REQUIRE(found->journeyProgress == Catch::Approx(0.1f));
}

TEST_CASE("MigrationSystem createRoute + duplicate rejection", "[Game][G33][Migration]") {
    NF::MigrationSystem sys;
    REQUIRE(sys.createRoute("North", "South") == 0);
    REQUIRE(sys.createRoute("East",  "West")  == 1);
    REQUIRE(sys.createRoute("North", "South") == -1); // duplicate
    REQUIRE(sys.routeCount() == 2);
}

TEST_CASE("MigrationSystem routeByEndpoints", "[Game][G33][Migration]") {
    NF::MigrationSystem sys;
    sys.createRoute("A", "B");
    REQUIRE(sys.routeByEndpoints("A", "B") != nullptr);
    REQUIRE(sys.routeByEndpoints("A", "B")->origin() == "A");
    REQUIRE(sys.routeByEndpoints("X", "Y") == nullptr);
}

TEST_CASE("MigrationSystem addWave + completedWaveCount", "[Game][G33][Migration]") {
    NF::MigrationSystem sys;
    NF::MigrationWave w1; w1.id = "w1"; w1.totalMigrants = 10; w1.arrivedCount = 10;
    NF::MigrationWave w2; w2.id = "w2"; w2.totalMigrants = 20; w2.arrivedCount = 5;
    REQUIRE(sys.addWave(w1));
    REQUIRE(sys.addWave(w2));
    REQUIRE(sys.waveCount() == 2);
    REQUIRE_FALSE(sys.addWave(w1)); // duplicate
    REQUIRE(sys.completedWaveCount() == 1);
}

TEST_CASE("MigrationSystem tick + totalMigrantsInTransit", "[Game][G33][Migration]") {
    NF::MigrationSystem sys;
    sys.createRoute("A", "B");
    auto* r = sys.routeByEndpoints("A", "B");
    NF::Migrant m; m.id = "m1"; m.journeyProgress = 0.5f;
    r->addMigrant(m);

    sys.tick(1.f);
    REQUIRE(sys.tickCount() == 1);
    REQUIRE(sys.totalMigrantsInTransit() == 1);
}


// ── G34 Tests: Insurgency System ─────────────────────────────────────────────

TEST_CASE("InsurgencyType names", "[Game][G34][Insurgency]") {
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Political))   == "Political");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Religious))   == "Religious");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Economic))    == "Economic");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Military))    == "Military");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Cultural))    == "Cultural");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Ecological))  == "Ecological");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Corporate))   == "Corporate");
    REQUIRE(std::string(NF::insurgencyTypeName(NF::InsurgencyType::Territorial)) == "Territorial");
}

TEST_CASE("Insurgent status transitions", "[Game][G34][Insurgency]") {
    NF::Insurgent ins;
    ins.id   = "i1";
    ins.name = "Raven";
    REQUIRE(ins.isActive());
    REQUIRE_FALSE(ins.isCaptured());

    REQUIRE(ins.goUnderground());
    REQUIRE(ins.isUnderground());
    REQUIRE_FALSE(ins.isActive());

    REQUIRE(ins.capture()); // can capture from underground
    REQUIRE(ins.isCaptured());

    REQUIRE_FALSE(ins.capture()); // already captured
}

TEST_CASE("Insurgent eliminate lifecycle", "[Game][G34][Insurgency]") {
    NF::Insurgent ins;
    ins.id     = "i2";
    ins.status = NF::InsurgentStatus::Active;
    REQUIRE(ins.eliminate());
    REQUIRE(ins.isEliminated());
    REQUIRE_FALSE(ins.eliminate()); // already eliminated
}

TEST_CASE("Insurgent goUnderground only from Active", "[Game][G34][Insurgency]") {
    NF::Insurgent ins;
    ins.status = NF::InsurgentStatus::Captured;
    REQUIRE_FALSE(ins.goUnderground()); // cannot go underground from captured
}

TEST_CASE("InsurgencyCell isOperational + totalStrength", "[Game][G34][Insurgency]") {
    NF::InsurgencyCell cell;
    cell.id = "cell1"; cell.region = "Sector 7";
    REQUIRE_FALSE(cell.isOperational()); // no members, no level

    cell.addMembers(10);
    cell.operationalLevel = 0.8f;
    REQUIRE(cell.isOperational());
    REQUIRE(cell.totalStrength() == Catch::Approx(8.f));
}

TEST_CASE("InsurgencyCell resources", "[Game][G34][Insurgency]") {
    NF::InsurgencyCell cell;
    cell.id = "cell2";
    REQUIRE_FALSE(cell.isFunded());

    cell.addResources(100.f);
    REQUIRE(cell.isFunded());
    REQUIRE(cell.resourcePool == Catch::Approx(100.f));

    REQUIRE(cell.drainResources(40.f));
    REQUIRE(cell.resourcePool == Catch::Approx(60.f));

    REQUIRE_FALSE(cell.drainResources(200.f)); // not enough
    REQUIRE(cell.resourcePool == Catch::Approx(60.f));
}

TEST_CASE("InsurgencyMovement addCell + duplicate rejection", "[Game][G34][Insurgency]") {
    NF::InsurgencyMovement mv("FreedomFront");
    NF::InsurgencyCell c1; c1.id = "c1"; c1.region = "North";
    NF::InsurgencyCell c2; c2.id = "c1"; // duplicate

    REQUIRE(mv.addCell(c1));
    REQUIRE(mv.cellCount() == 1);
    REQUIRE_FALSE(mv.addCell(c2));
    REQUIRE(mv.cellCount() == 1);
}

TEST_CASE("InsurgencyMovement removeCell + activeCellCount + totalMembers", "[Game][G34][Insurgency]") {
    NF::InsurgencyMovement mv("RedDawn");
    NF::InsurgencyCell c1; c1.id = "c1"; c1.memberCount = 5; c1.operationalLevel = 1.f;
    NF::InsurgencyCell c2; c2.id = "c2"; c2.memberCount = 3; c2.operationalLevel = 0.f; // inactive
    mv.addCell(c1);
    mv.addCell(c2);

    REQUIRE(mv.totalMembers()    == 8);
    REQUIRE(mv.activeCellCount() == 1);

    REQUIRE(mv.removeCell("c1"));
    REQUIRE(mv.cellCount()    == 1);
    REQUIRE(mv.totalMembers() == 3);
    REQUIRE_FALSE(mv.removeCell("nonexistent"));
}

TEST_CASE("InsurgencySystem createMovement + duplicate rejection", "[Game][G34][Insurgency]") {
    NF::InsurgencySystem sys;
    REQUIRE(sys.createMovement("FreedomFront") != nullptr);
    REQUIRE(sys.createMovement("RedDawn")      != nullptr);
    REQUIRE(sys.createMovement("FreedomFront") == nullptr); // duplicate
    REQUIRE(sys.movementCount() == 2);
}

TEST_CASE("InsurgencySystem movementByName", "[Game][G34][Insurgency]") {
    NF::InsurgencySystem sys;
    sys.createMovement("Shadow");
    REQUIRE(sys.movementByName("Shadow") != nullptr);
    REQUIRE(sys.movementByName("Shadow")->name() == "Shadow");
    REQUIRE(sys.movementByName("Unknown") == nullptr);
}

TEST_CASE("InsurgencySystem addInsurgent + activeInsurgentCount", "[Game][G34][Insurgency]") {
    NF::InsurgencySystem sys;
    NF::Insurgent i1; i1.id = "i1"; i1.status = NF::InsurgentStatus::Active;
    NF::Insurgent i2; i2.id = "i2"; i2.status = NF::InsurgentStatus::Captured;
    NF::Insurgent i3; i3.id = "i3"; i3.status = NF::InsurgentStatus::Active;
    REQUIRE(sys.addInsurgent(i1));
    REQUIRE(sys.addInsurgent(i2));
    REQUIRE(sys.addInsurgent(i3));
    REQUIRE_FALSE(sys.addInsurgent(i1)); // duplicate

    REQUIRE(sys.totalInsurgentCount()    == 3);
    REQUIRE(sys.activeInsurgentCount()   == 2);
    REQUIRE(sys.capturedInsurgentCount() == 1);
}

TEST_CASE("InsurgencySystem tick + totalCells", "[Game][G34][Insurgency]") {
    NF::InsurgencySystem sys;
    auto* mv = sys.createMovement("TestMovement");
    NF::InsurgencyCell cell; cell.id = "c1"; cell.memberCount = 5; cell.operationalLevel = 1.f;
    mv->addCell(cell);

    sys.tick(1.f);
    sys.tick(1.f);
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.totalCells() == 1);
}


// ── G35 Tests: Plague System ─────────────────────────────────────────────────

TEST_CASE("PlagueType names", "[Game][G35][Plague]") {
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Bacterial)) == "Bacterial");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Viral))     == "Viral");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Fungal))    == "Fungal");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Parasitic)) == "Parasitic");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Prion))     == "Prion");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Genetic))   == "Genetic");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Chemical))  == "Chemical");
    REQUIRE(std::string(NF::plagueTypeName(NF::PlagueType::Radiation)) == "Radiation");
}

TEST_CASE("PlagueCarrier status transitions", "[Game][G35][Plague]") {
    NF::PlagueCarrier c;
    c.id   = "p1";
    c.name = "Alice";
    REQUIRE(c.isHealthy());

    REQUIRE(c.expose());
    REQUIRE(c.isExposed());

    REQUIRE(c.infect());
    REQUIRE(c.isInfected());

    REQUIRE(c.recover());
    REQUIRE(c.isRecovering());

    REQUIRE(c.becomeImmune());
    REQUIRE(c.isImmune());
    REQUIRE(c.immunity == Catch::Approx(1.f));
}

TEST_CASE("PlagueCarrier fully immune cannot be exposed", "[Game][G35][Plague]") {
    NF::PlagueCarrier c;
    c.id      = "p2";
    c.immunity = 1.f;
    REQUIRE_FALSE(c.expose()); // fully immune
    REQUIRE(c.isHealthy());
}

TEST_CASE("PlagueCarrier expose only from Healthy", "[Game][G35][Plague]") {
    NF::PlagueCarrier c;
    c.id = "p3"; c.status = NF::InfectionStatus::Infected;
    REQUIRE_FALSE(c.expose()); // not healthy
}

TEST_CASE("PlagueStat helpers", "[Game][G35][Plague]") {
    NF::PlagueStat p;
    p.id             = "flu1";
    p.region         = "North";
    p.type           = NF::PlagueType::Viral;
    p.transmissionRate = 2.5f;
    p.mortalityRate    = 0.02f;

    REQUIRE(p.isLethal());
    REQUIRE(p.isSpreading());
    REQUIRE_FALSE(p.isContained());

    p.contain();
    REQUIRE(p.isContained());
    REQUIRE_FALSE(p.isSpreading());

    p.release();
    REQUIRE_FALSE(p.isContained());
    REQUIRE(p.isSpreading());
}

TEST_CASE("PlagueRegion addCarrier + duplicate rejection", "[Game][G35][Plague]") {
    NF::PlagueRegion region("Sector A");
    NF::PlagueCarrier c1; c1.id = "c1"; c1.status = NF::InfectionStatus::Infected;
    NF::PlagueCarrier c2; c2.id = "c1"; // duplicate

    REQUIRE(region.addCarrier(c1));
    REQUIRE(region.carrierCount() == 1);
    REQUIRE_FALSE(region.addCarrier(c2));
    REQUIRE(region.carrierCount() == 1);
}

TEST_CASE("PlagueRegion infectedCount + immuneCount + healthyCount", "[Game][G35][Plague]") {
    NF::PlagueRegion region("Zone B");
    NF::PlagueCarrier c1; c1.id = "c1"; c1.status = NF::InfectionStatus::Infected;
    NF::PlagueCarrier c2; c2.id = "c2"; c2.status = NF::InfectionStatus::Immune;
    NF::PlagueCarrier c3; c3.id = "c3"; c3.status = NF::InfectionStatus::Healthy;
    region.addCarrier(c1);
    region.addCarrier(c2);
    region.addCarrier(c3);

    REQUIRE(region.infectedCount() == 1);
    REQUIRE(region.immuneCount()   == 1);
    REQUIRE(region.healthyCount()  == 1);
}

TEST_CASE("PlagueSystem createRegion + duplicate rejection", "[Game][G35][Plague]") {
    NF::PlagueSystem sys;
    REQUIRE(sys.createRegion("North") != nullptr);
    REQUIRE(sys.createRegion("South") != nullptr);
    REQUIRE(sys.createRegion("North") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("PlagueSystem addPlagueStat + activePlagueCount", "[Game][G35][Plague]") {
    NF::PlagueSystem sys;
    NF::PlagueStat p1; p1.id = "flu1"; p1.transmissionRate = 2.5f;
    NF::PlagueStat p2; p2.id = "flu2"; p2.contained = true;
    REQUIRE(sys.addPlagueStat(p1));
    REQUIRE(sys.addPlagueStat(p2));
    REQUIRE_FALSE(sys.addPlagueStat(p1)); // duplicate
    REQUIRE(sys.plagueCount() == 2);
    REQUIRE(sys.activePlagueCount() == 1);
}

TEST_CASE("PlagueSystem totalInfected + totalImmune across regions", "[Game][G35][Plague]") {
    NF::PlagueSystem sys;
    sys.createRegion("Alpha");
    sys.createRegion("Beta");

    NF::PlagueCarrier c1; c1.id = "c1"; c1.status = NF::InfectionStatus::Infected;
    NF::PlagueCarrier c2; c2.id = "c2"; c2.status = NF::InfectionStatus::Immune;
    NF::PlagueCarrier c3; c3.id = "c3"; c3.status = NF::InfectionStatus::Infected;
    sys.regionByName("Alpha")->addCarrier(c1);
    sys.regionByName("Alpha")->addCarrier(c2);
    sys.regionByName("Beta")->addCarrier(c3);

    REQUIRE(sys.totalInfected() == 2);
    REQUIRE(sys.totalImmune()   == 1);
}

TEST_CASE("PlagueSystem tick propagates to regions", "[Game][G35][Plague]") {
    NF::PlagueSystem sys;
    sys.createRegion("East");
    sys.tick(1.f);
    sys.tick(1.f);
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.regionByName("East")->tickCount() == 2);
}

// ============================================================
// G36 — Famine System tests
// ============================================================

TEST_CASE("FamineType names", "[Game][G36][Famine]") {
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Drought))  == "Drought");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Blight))   == "Blight");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Flood))    == "Flood");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Pest))     == "Pest");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::War))      == "War");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Blockade)) == "Blockade");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Economic)) == "Economic");
    REQUIRE(std::string(NF::famineTypeName(NF::FamineType::Climate))  == "Climate");
}

TEST_CASE("FamineEvent lifecycle", "[Game][G36][Famine]") {
    NF::FamineEvent ev;
    ev.id       = "fev1";
    ev.region   = "North";
    ev.type     = NF::FamineType::Drought;
    ev.severity = NF::FamineSeverity::Severe;

    REQUIRE(ev.isActive());
    REQUIRE(ev.isCritical());
    REQUIRE(ev.duration == 0.f);

    ev.advanceDuration(5.f);
    REQUIRE(ev.duration == 5.f);

    ev.resolve();
    REQUIRE_FALSE(ev.isActive());

    ev.advanceDuration(5.f); // no advance after resolved
    REQUIRE(ev.duration == 5.f);
}

TEST_CASE("FamineEvent isCritical threshold", "[Game][G36][Famine]") {
    NF::FamineEvent ev;
    ev.severity = NF::FamineSeverity::Moderate;
    REQUIRE_FALSE(ev.isCritical());
    ev.severity = NF::FamineSeverity::Catastrophic;
    REQUIRE(ev.isCritical());
}

TEST_CASE("FamineRegion severity levels", "[Game][G36][Famine]") {
    // food/pop ratio drives severity
    NF::FamineRegion abundant("Fertile", 100.f, 100.f, 0.f);  // ratio=1.0 → None
    REQUIRE(abundant.severity() == NF::FamineSeverity::None);

    NF::FamineRegion mild("MildZone", 100.f, 40.f, 0.f);       // ratio=0.4 → Mild
    REQUIRE(mild.severity() == NF::FamineSeverity::Mild);

    NF::FamineRegion moderate("ModZone", 100.f, 25.f, 0.f);    // ratio=0.25 → Moderate
    REQUIRE(moderate.severity() == NF::FamineSeverity::Moderate);

    NF::FamineRegion severe("SevZone", 100.f, 8.f, 0.f);       // ratio=0.08 → Severe
    REQUIRE(severe.severity() == NF::FamineSeverity::Severe);

    NF::FamineRegion catastrophic("CatZone", 100.f, 1.f, 0.f); // ratio=0.01 → Catastrophic
    REQUIRE(catastrophic.severity() == NF::FamineSeverity::Catastrophic);
}

TEST_CASE("FamineRegion addAid increases food supply", "[Game][G36][Famine]") {
    NF::FamineRegion r("AidZone", 100.f, 10.f, 0.f);
    REQUIRE(r.foodSupply() == 10.f);
    r.addAid(50.f);
    REQUIRE(r.foodSupply() == 60.f);
    r.addAid(-5.f); // negative aid ignored
    REQUIRE(r.foodSupply() == 60.f);
}

TEST_CASE("FamineRegion tick depletes food", "[Game][G36][Famine]") {
    NF::FamineRegion r("DepletingZone", 100.f, 500.f, 1.f); // consumes 100 per tick (dt=1)
    r.tick(1.f);
    REQUIRE(r.foodSupply() == 400.f);
    REQUIRE(r.tickCount() == 1);
}

TEST_CASE("FamineRegion food floor at zero", "[Game][G36][Famine]") {
    NF::FamineRegion r("EmptyZone", 100.f, 5.f, 1.f);
    r.tick(1.f); // consumes 100, floor at 0
    REQUIRE(r.foodSupply() == 0.f);
}

TEST_CASE("FamineSystem createRegion + duplicate rejection", "[Game][G36][Famine]") {
    NF::FamineSystem sys;
    REQUIRE(sys.createRegion("North") != nullptr);
    REQUIRE(sys.createRegion("South") != nullptr);
    REQUIRE(sys.createRegion("North") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("FamineSystem addEvent + activeEventCount + resolvedEventCount", "[Game][G36][Famine]") {
    NF::FamineSystem sys;
    NF::FamineEvent e1; e1.id = "ev1"; e1.resolved = false;
    NF::FamineEvent e2; e2.id = "ev2"; e2.resolved = true;
    NF::FamineEvent e3; e3.id = "ev1"; // duplicate

    REQUIRE(sys.addEvent(e1));
    REQUIRE(sys.addEvent(e2));
    REQUIRE_FALSE(sys.addEvent(e3));
    REQUIRE(sys.eventCount() == 2);
    REQUIRE(sys.activeEventCount() == 1);
    REQUIRE(sys.resolvedEventCount() == 1);
}

TEST_CASE("FamineSystem findEvent + resolve", "[Game][G36][Famine]") {
    NF::FamineSystem sys;
    NF::FamineEvent ev; ev.id = "drought1"; ev.type = NF::FamineType::Drought;
    sys.addEvent(ev);

    auto* found = sys.findEvent("drought1");
    REQUIRE(found != nullptr);
    REQUIRE(found->type == NF::FamineType::Drought);

    found->resolve();
    REQUIRE(sys.activeEventCount() == 0);
    REQUIRE(sys.resolvedEventCount() == 1);
}

TEST_CASE("FamineSystem tick propagates to regions and events", "[Game][G36][Famine]") {
    NF::FamineSystem sys;
    sys.createRegion("West", 100.f, 500.f, 1.f);
    NF::FamineEvent ev; ev.id = "ev1";
    sys.addEvent(ev);

    sys.tick(1.f);
    sys.tick(1.f);
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.regionByName("West")->tickCount() == 2);
    REQUIRE(sys.findEvent("ev1")->duration == 2.f);
}

TEST_CASE("FamineSystem criticalRegionCount", "[Game][G36][Famine]") {
    NF::FamineSystem sys;
    sys.createRegion("ZoneA", 100.f, 500.f, 0.f); // ratio=5 → None
    sys.createRegion("ZoneB", 100.f, 3.f,   0.f); // ratio=0.03 → Catastrophic
    REQUIRE(sys.criticalRegionCount() == 1);
}

// ============================================================
// G37 — Refugee System tests
// ============================================================

TEST_CASE("RefugeeOrigin names cover all 8 values", "[Game][G37][Refugee]") {
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::War))       == "War");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Famine))    == "Famine");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Plague))    == "Plague");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Disaster))  == "Disaster");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Political)) == "Political");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Economic))  == "Economic");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Religious)) == "Religious");
    REQUIRE(std::string(NF::refugeeOriginName(NF::RefugeeOrigin::Climate))   == "Climate");
}

TEST_CASE("Refugee status predicates", "[Game][G37][Refugee]") {
    NF::Refugee r;
    r.id = "r1"; r.status = NF::RefugeeStatus::InTransit;
    REQUIRE(r.isInTransit());
    REQUIRE_FALSE(r.isSheltered());
    REQUIRE_FALSE(r.isSettled());
    REQUIRE_FALSE(r.isDisplaced());
    REQUIRE_FALSE(r.isReturned());
}

TEST_CASE("Refugee shelter + settle + sendHome transitions", "[Game][G37][Refugee]") {
    NF::Refugee r;
    r.id = "r1"; r.status = NF::RefugeeStatus::InTransit;

    REQUIRE(r.shelter());
    REQUIRE(r.isSheltered());

    REQUIRE(r.settle());
    REQUIRE(r.isSettled());

    REQUIRE(r.sendHome());
    REQUIRE(r.isReturned());

    // cannot transition further from Returned
    REQUIRE_FALSE(r.shelter());
    REQUIRE_FALSE(r.displace());
}

TEST_CASE("Refugee displace blocks from settled/returned", "[Game][G37][Refugee]") {
    NF::Refugee r;
    r.id = "r2"; r.status = NF::RefugeeStatus::Settled;
    REQUIRE_FALSE(r.displace()); // settled can't be displaced

    NF::Refugee r2;
    r2.id = "r3"; r2.status = NF::RefugeeStatus::InTransit;
    REQUIRE(r2.displace()); // in transit can be displaced
    REQUIRE(r2.isDisplaced());

    // can shelter from displaced
    REQUIRE(r2.shelter());
    REQUIRE(r2.isSheltered());
}

TEST_CASE("Refugee settle only from Sheltered", "[Game][G37][Refugee]") {
    NF::Refugee r;
    r.id = "r4"; r.status = NF::RefugeeStatus::InTransit;
    REQUIRE_FALSE(r.settle()); // not sheltered yet
}

TEST_CASE("RefugeeCamp addRefugee + duplicate rejection + capacity", "[Game][G37][Refugee]") {
    NF::RefugeeCamp camp("CampAlpha", 2);
    NF::Refugee r1; r1.id = "r1";
    NF::Refugee r2; r2.id = "r2";
    NF::Refugee r3; r3.id = "r3";
    NF::Refugee r1dup; r1dup.id = "r1"; // duplicate

    REQUIRE(camp.addRefugee(r1));
    REQUIRE_FALSE(camp.addRefugee(r1dup)); // duplicate
    REQUIRE(camp.addRefugee(r2));
    REQUIRE_FALSE(camp.addRefugee(r3)); // over capacity
    REQUIRE(camp.refugeeCount() == 2);
    REQUIRE(camp.isFull());
}

TEST_CASE("RefugeeCamp removeRefugee + findRefugee", "[Game][G37][Refugee]") {
    NF::RefugeeCamp camp("CampBeta", 10);
    NF::Refugee r1; r1.id = "r1"; r1.status = NF::RefugeeStatus::Sheltered;
    camp.addRefugee(r1);

    REQUIRE(camp.findRefugee("r1") != nullptr);
    REQUIRE(camp.findRefugee("r1")->isSheltered());
    REQUIRE(camp.removeRefugee("r1"));
    REQUIRE(camp.findRefugee("r1") == nullptr);
    REQUIRE_FALSE(camp.removeRefugee("r1")); // already gone
}

TEST_CASE("RefugeeCamp shelteredCount + settledCount", "[Game][G37][Refugee]") {
    NF::RefugeeCamp camp("CampGamma", 10);
    NF::Refugee r1; r1.id = "r1"; r1.status = NF::RefugeeStatus::Sheltered;
    NF::Refugee r2; r2.id = "r2"; r2.status = NF::RefugeeStatus::Settled;
    NF::Refugee r3; r3.id = "r3"; r3.status = NF::RefugeeStatus::InTransit;
    camp.addRefugee(r1);
    camp.addRefugee(r2);
    camp.addRefugee(r3);

    REQUIRE(camp.shelteredCount() == 1);
    REQUIRE(camp.settledCount()   == 1);
}

TEST_CASE("RefugeeSystem createCamp + duplicate rejection", "[Game][G37][Refugee]") {
    NF::RefugeeSystem sys;
    REQUIRE(sys.createCamp("Alpha") != nullptr);
    REQUIRE(sys.createCamp("Beta")  != nullptr);
    REQUIRE(sys.createCamp("Alpha") == nullptr); // duplicate
    REQUIRE(sys.campCount() == 2);
}

TEST_CASE("RefugeeSystem totalRefugees + totalSheltered + totalSettled", "[Game][G37][Refugee]") {
    NF::RefugeeSystem sys;
    sys.createCamp("A", 10);
    sys.createCamp("B", 10);

    NF::Refugee r1; r1.id = "r1"; r1.status = NF::RefugeeStatus::Sheltered;
    NF::Refugee r2; r2.id = "r2"; r2.status = NF::RefugeeStatus::Settled;
    NF::Refugee r3; r3.id = "r3"; r3.status = NF::RefugeeStatus::InTransit;
    sys.campByName("A")->addRefugee(r1);
    sys.campByName("A")->addRefugee(r2);
    sys.campByName("B")->addRefugee(r3);

    REQUIRE(sys.totalRefugees()  == 3);
    REQUIRE(sys.totalSheltered() == 1);
    REQUIRE(sys.totalSettled()   == 1);
}

TEST_CASE("RefugeeSystem fullCampCount", "[Game][G37][Refugee]") {
    NF::RefugeeSystem sys;
    sys.createCamp("Full", 1);
    sys.createCamp("Empty", 10);

    NF::Refugee r1; r1.id = "r1";
    sys.campByName("Full")->addRefugee(r1);

    REQUIRE(sys.fullCampCount() == 1);
}

TEST_CASE("RefugeeSystem tick propagates to camps", "[Game][G37][Refugee]") {
    NF::RefugeeSystem sys;
    sys.createCamp("Delta");
    sys.tick(1.f);
    sys.tick(1.f);
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.campByName("Delta")->tickCount() == 2);
}

// ============================================================
// G38 — Storm System tests
// ============================================================

TEST_CASE("StormType names cover all 8 values", "[Game][G38][Storm]") {
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Thunderstorm))  == "Thunderstorm");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Hurricane))     == "Hurricane");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Blizzard))      == "Blizzard");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Sandstorm))     == "Sandstorm");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Firestorm))     == "Firestorm");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Hailstorm))     == "Hailstorm");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::Tornado))       == "Tornado");
    REQUIRE(std::string(NF::stormTypeName(NF::StormType::ElectricStorm)) == "ElectricStorm");
}

TEST_CASE("Storm isActive and isCritical predicates", "[Game][G38][Storm]") {
    NF::Storm s;
    s.id = "s1"; s.active = true; s.duration = 5; s.severity = NF::StormSeverity::Mild;
    REQUIRE(s.isActive());
    REQUIRE_FALSE(s.isCritical());

    s.severity = NF::StormSeverity::Severe;
    REQUIRE(s.isCritical());

    s.duration = 0;
    REQUIRE_FALSE(s.isActive());
}

TEST_CASE("Storm dissipate clears active and duration", "[Game][G38][Storm]") {
    NF::Storm s;
    s.id = "s1"; s.active = true; s.duration = 10;
    s.dissipate();
    REQUIRE_FALSE(s.active);
    REQUIRE(s.duration == 0);
    REQUIRE_FALSE(s.isActive());
}

TEST_CASE("Storm advanceDuration counts down and auto-dissipates", "[Game][G38][Storm]") {
    NF::Storm s;
    s.id = "s1"; s.active = true; s.duration = 2;
    s.advanceDuration();
    REQUIRE(s.duration == 1);
    REQUIRE(s.isActive());
    s.advanceDuration();
    REQUIRE(s.duration == 0);
    REQUIRE_FALSE(s.isActive());
}

TEST_CASE("StormRegion addStorm + duplicate rejection", "[Game][G38][Storm]") {
    NF::StormRegion region("Coast");
    NF::Storm s1; s1.id = "s1"; s1.active = true; s1.duration = 3;
    NF::Storm s1dup; s1dup.id = "s1";

    REQUIRE(region.addStorm(s1));
    REQUIRE_FALSE(region.addStorm(s1dup));
    REQUIRE(region.stormCount() == 1);
}

TEST_CASE("StormRegion activeStormCount + currentSeverity", "[Game][G38][Storm]") {
    NF::StormRegion region("Plains");
    NF::Storm s1; s1.id = "s1"; s1.active = true;  s1.duration = 5; s1.severity = NF::StormSeverity::Moderate;
    NF::Storm s2; s2.id = "s2"; s2.active = false; s2.duration = 0; s2.severity = NF::StormSeverity::Catastrophic;
    region.addStorm(s1);
    region.addStorm(s2);

    REQUIRE(region.activeStormCount() == 1);
    REQUIRE(region.currentSeverity() == NF::StormSeverity::Moderate); // s2 inactive
}

TEST_CASE("StormRegion tick advances storms and region tickCount", "[Game][G38][Storm]") {
    NF::StormRegion region("Valley");
    NF::Storm s; s.id = "s1"; s.active = true; s.duration = 3;
    region.addStorm(s);

    region.tick();
    region.tick();
    REQUIRE(region.tickCount() == 2);
    REQUIRE(region.findStorm("s1")->duration == 1);
}

TEST_CASE("StormSystem createRegion + duplicate rejection", "[Game][G38][Storm]") {
    NF::StormSystem sys;
    REQUIRE(sys.createRegion("North") != nullptr);
    REQUIRE(sys.createRegion("South") != nullptr);
    REQUIRE(sys.createRegion("North") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("StormSystem addStorm + activeStormCount", "[Game][G38][Storm]") {
    NF::StormSystem sys;
    sys.createRegion("East");

    NF::Storm s; s.id = "st1"; s.region = "East"; s.active = true; s.duration = 5;
    REQUIRE(sys.addStorm(s));
    REQUIRE(sys.totalStormCount() == 1);
    REQUIRE(sys.activeStormCount() == 1);

    // bad region returns false
    NF::Storm bad; bad.id = "st2"; bad.region = "Nowhere"; bad.active = true; bad.duration = 1;
    REQUIRE_FALSE(sys.addStorm(bad));
}

TEST_CASE("StormSystem criticalRegionCount", "[Game][G38][Storm]") {
    NF::StormSystem sys;
    sys.createRegion("R1");
    sys.createRegion("R2");

    NF::Storm severe; severe.id = "st1"; severe.region = "R1";
    severe.active = true; severe.duration = 10; severe.severity = NF::StormSeverity::Severe;
    sys.addStorm(severe);

    NF::Storm mild; mild.id = "st2"; mild.region = "R2";
    mild.active = true; mild.duration = 5; mild.severity = NF::StormSeverity::Mild;
    sys.addStorm(mild);

    REQUIRE(sys.criticalRegionCount() == 1);
}

TEST_CASE("StormSystem tick propagates to all regions", "[Game][G38][Storm]") {
    NF::StormSystem sys;
    sys.createRegion("Alpha");
    sys.createRegion("Beta");
    sys.tick();
    sys.tick();
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.regionByName("Alpha")->tickCount() == 2);
    REQUIRE(sys.regionByName("Beta")->tickCount() == 2);
}

// ============================================================
// G39 — Earthquake System tests
// ============================================================

TEST_CASE("EarthquakeScale names cover all 8 values", "[Game][G39][Earthquake]") {
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Micro))        == "Micro");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Minor))        == "Minor");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Light))        == "Light");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Moderate))     == "Moderate");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Strong))       == "Strong");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Major))        == "Major");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Great))        == "Great");
    REQUIRE(std::string(NF::earthquakeScaleName(NF::EarthquakeScale::Catastrophic)) == "Catastrophic");
}

TEST_CASE("Earthquake status predicates", "[Game][G39][Earthquake]") {
    NF::Earthquake eq;
    eq.id = "eq1"; eq.status = NF::EarthquakeStatus::Pending;
    REQUIRE(eq.isPending());
    REQUIRE_FALSE(eq.isActive());

    eq.activate();
    REQUIRE(eq.isActive());
    REQUIRE_FALSE(eq.isPending());
}

TEST_CASE("Earthquake isMajor threshold at Major+", "[Game][G39][Earthquake]") {
    NF::Earthquake eq; eq.id = "eq1";
    eq.scale = NF::EarthquakeScale::Strong;
    REQUIRE_FALSE(eq.isMajor());
    eq.scale = NF::EarthquakeScale::Major;
    REQUIRE(eq.isMajor());
    eq.scale = NF::EarthquakeScale::Catastrophic;
    REQUIRE(eq.isMajor());
}

TEST_CASE("Earthquake toAftershock only from Active", "[Game][G39][Earthquake]") {
    NF::Earthquake eq; eq.id = "eq1"; eq.status = NF::EarthquakeStatus::Pending;
    eq.toAftershock();
    REQUIRE(eq.isPending()); // no change from Pending

    eq.activate();
    eq.toAftershock();
    REQUIRE(eq.isAftershock());
}

TEST_CASE("Earthquake resolve sets Resolved", "[Game][G39][Earthquake]") {
    NF::Earthquake eq; eq.id = "eq1"; eq.status = NF::EarthquakeStatus::Active;
    eq.resolve();
    REQUIRE(eq.isResolved());
}

TEST_CASE("FaultLine addEarthquake + duplicate rejection", "[Game][G39][Earthquake]") {
    NF::FaultLine fault("Pacific");
    NF::Earthquake eq; eq.id = "eq1"; eq.status = NF::EarthquakeStatus::Active;
    NF::Earthquake eqdup; eqdup.id = "eq1";

    REQUIRE(fault.addEarthquake(eq));
    REQUIRE_FALSE(fault.addEarthquake(eqdup));
    REQUIRE(fault.earthquakeCount() == 1);
}

TEST_CASE("FaultLine activeCount + majorCount", "[Game][G39][Earthquake]") {
    NF::FaultLine fault("Alpine");
    NF::Earthquake eq1; eq1.id = "e1"; eq1.status = NF::EarthquakeStatus::Active;     eq1.scale = NF::EarthquakeScale::Major;
    NF::Earthquake eq2; eq2.id = "e2"; eq2.status = NF::EarthquakeStatus::Aftershock; eq2.scale = NF::EarthquakeScale::Minor;
    NF::Earthquake eq3; eq3.id = "e3"; eq3.status = NF::EarthquakeStatus::Resolved;   eq3.scale = NF::EarthquakeScale::Great;
    fault.addEarthquake(eq1);
    fault.addEarthquake(eq2);
    fault.addEarthquake(eq3);

    REQUIRE(fault.activeCount() == 2); // Active + Aftershock
    REQUIRE(fault.majorCount()  == 2); // Major + Great (resolved still counts by scale)
}

TEST_CASE("FaultLine tick increments tickCount", "[Game][G39][Earthquake]") {
    NF::FaultLine fault("Anatolian");
    fault.tick(); fault.tick();
    REQUIRE(fault.tickCount() == 2);
}

TEST_CASE("EarthquakeSystem createFaultLine + duplicate rejection", "[Game][G39][Earthquake]") {
    NF::EarthquakeSystem sys;
    REQUIRE(sys.createFaultLine("SanAndreas") != nullptr);
    REQUIRE(sys.createFaultLine("Cascadia")   != nullptr);
    REQUIRE(sys.createFaultLine("SanAndreas") == nullptr); // duplicate
    REQUIRE(sys.faultCount() == 2);
}

TEST_CASE("EarthquakeSystem addEarthquake + bad-region rejection", "[Game][G39][Earthquake]") {
    NF::EarthquakeSystem sys;
    sys.createFaultLine("Japan");

    NF::Earthquake eq; eq.id = "e1"; eq.region = "Japan"; eq.status = NF::EarthquakeStatus::Active;
    REQUIRE(sys.addEarthquake(eq));
    REQUIRE(sys.earthquakeCount() == 1);

    NF::Earthquake bad; bad.id = "e2"; bad.region = "Nowhere";
    REQUIRE_FALSE(sys.addEarthquake(bad));
}

TEST_CASE("EarthquakeSystem activeEarthquakeCount + majorEarthquakeCount", "[Game][G39][Earthquake]") {
    NF::EarthquakeSystem sys;
    sys.createFaultLine("Fault1");

    NF::Earthquake e1; e1.id = "e1"; e1.region = "Fault1"; e1.status = NF::EarthquakeStatus::Active;     e1.scale = NF::EarthquakeScale::Major;
    NF::Earthquake e2; e2.id = "e2"; e2.region = "Fault1"; e2.status = NF::EarthquakeStatus::Resolved;   e2.scale = NF::EarthquakeScale::Minor;
    sys.addEarthquake(e1);
    sys.addEarthquake(e2);

    REQUIRE(sys.activeEarthquakeCount() == 1);
    REQUIRE(sys.majorEarthquakeCount()  == 1);
}

TEST_CASE("EarthquakeSystem tick propagates to faults and increments tickCount", "[Game][G39][Earthquake]") {
    NF::EarthquakeSystem sys;
    sys.createFaultLine("F1");
    sys.createFaultLine("F2");
    sys.tick();
    sys.tick();
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.faultByName("F1")->tickCount() == 2);
    REQUIRE(sys.faultByName("F2")->tickCount() == 2);
}

// ============================================================
// G40 — Volcano System tests
// ============================================================

TEST_CASE("VolcanoActivity names cover all 8 values", "[Game][G40][Volcano]") {
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Dormant))      == "Dormant");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Restless))     == "Restless");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Elevated))     == "Elevated");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Unrest))       == "Unrest");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Minor))        == "Minor");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Moderate))     == "Moderate");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Major))        == "Major");
    REQUIRE(std::string(NF::volcanoActivityName(NF::VolcanoActivity::Catastrophic)) == "Catastrophic");
}

TEST_CASE("Volcano status transitions", "[Game][G40][Volcano]") {
    NF::Volcano v("Vesuvius");
    REQUIRE(v.isInactive());

    v.monitor();
    REQUIRE(v.status() == NF::VolcanoStatus::Monitoring);

    v.startEruption();
    REQUIRE(v.isErupting());

    v.beginSubsiding();
    REQUIRE(v.isSubsiding());
}

TEST_CASE("Volcano beginSubsiding only from Erupting", "[Game][G40][Volcano]") {
    NF::Volcano v("Etna");
    v.monitor();
    v.beginSubsiding(); // not erupting — should be no-op
    REQUIRE(v.status() == NF::VolcanoStatus::Monitoring);
}

TEST_CASE("Volcano isMajor via setActivity", "[Game][G40][Volcano]") {
    NF::Volcano v("Fuji");
    v.setActivity(NF::VolcanoActivity::Moderate);
    REQUIRE_FALSE(v.activity() >= NF::VolcanoActivity::Major);

    v.setActivity(NF::VolcanoActivity::Major);
    REQUIRE(v.activity() >= NF::VolcanoActivity::Major);
}

TEST_CASE("VolcanicEvent addEvent + duplicate rejection", "[Game][G40][Volcano]") {
    NF::Volcano v("Krakatau");
    NF::VolcanicEvent ev; ev.id = "ev1"; ev.activity = NF::VolcanoActivity::Major;
    NF::VolcanicEvent dup; dup.id = "ev1";

    REQUIRE(v.addEvent(ev));
    REQUIRE_FALSE(v.addEvent(dup));
    REQUIRE(v.eventCount() == 1);
}

TEST_CASE("Volcano majorEvents count", "[Game][G40][Volcano]") {
    NF::Volcano v("Pinatubo");
    NF::VolcanicEvent e1; e1.id = "e1"; e1.activity = NF::VolcanoActivity::Major;
    NF::VolcanicEvent e2; e2.id = "e2"; e2.activity = NF::VolcanoActivity::Minor;
    NF::VolcanicEvent e3; e3.id = "e3"; e3.activity = NF::VolcanoActivity::Catastrophic;
    v.addEvent(e1); v.addEvent(e2); v.addEvent(e3);

    REQUIRE(v.majorEvents() == 2); // Major + Catastrophic
}

TEST_CASE("VolcanicEvent isCatastrophic predicate", "[Game][G40][Volcano]") {
    NF::VolcanicEvent ev; ev.id = "e1";
    ev.activity = NF::VolcanoActivity::Major;
    REQUIRE_FALSE(ev.isCatastrophic());
    ev.activity = NF::VolcanoActivity::Catastrophic;
    REQUIRE(ev.isCatastrophic());
}

TEST_CASE("VolcanoSystem createVolcano + duplicate rejection", "[Game][G40][Volcano]") {
    NF::VolcanoSystem sys;
    REQUIRE(sys.createVolcano("Tambora") != nullptr);
    REQUIRE(sys.createVolcano("Stromboli") != nullptr);
    REQUIRE(sys.createVolcano("Tambora") == nullptr); // duplicate
    REQUIRE(sys.volcanoCount() == 2);
}

TEST_CASE("VolcanoSystem addEvent + bad-volcano rejection", "[Game][G40][Volcano]") {
    NF::VolcanoSystem sys;
    sys.createVolcano("MaunaLoa");

    NF::VolcanicEvent ev; ev.id = "e1"; ev.activity = NF::VolcanoActivity::Minor;
    REQUIRE(sys.addEvent("MaunaLoa", ev));
    REQUIRE(sys.eventCount() == 1);

    NF::VolcanicEvent bad; bad.id = "e2";
    REQUIRE_FALSE(sys.addEvent("Nowhere", bad));
}

TEST_CASE("VolcanoSystem eruptingCount + majorEventCount", "[Game][G40][Volcano]") {
    NF::VolcanoSystem sys;
    sys.createVolcano("V1");
    sys.createVolcano("V2");
    sys.byName("V1")->startEruption();

    NF::VolcanicEvent ev; ev.id = "e1"; ev.activity = NF::VolcanoActivity::Catastrophic;
    sys.addEvent("V2", ev);

    REQUIRE(sys.eruptingCount()   == 1);
    REQUIRE(sys.majorEventCount() == 1);
}

TEST_CASE("VolcanoSystem tick propagates to volcanoes and increments tickCount", "[Game][G40][Volcano]") {
    NF::VolcanoSystem sys;
    sys.createVolcano("A");
    sys.createVolcano("B");
    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
    REQUIRE(sys.byName("A")->tickCount() == 2);
    REQUIRE(sys.byName("B")->tickCount() == 2);
}

// ============================================================
// G41 — Tsunami System tests
// ============================================================

TEST_CASE("TsunamiCause names cover all 8 values", "[Game][G41][Tsunami]") {
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Earthquake))  == "Earthquake");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Landslide))   == "Landslide");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Volcanic))    == "Volcanic");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Meteorite))   == "Meteorite");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Submarine))   == "Submarine");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Glacial))     == "Glacial");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Nuclear))     == "Nuclear");
    REQUIRE(std::string(NF::tsunamiCauseName(NF::TsunamiCause::Unknown))     == "Unknown");
}

TEST_CASE("Tsunami status advance chain", "[Game][G41][Tsunami]") {
    NF::Tsunami t("t1");
    REQUIRE(t.status() == NF::TsunamiStatus::Forming);
    t.advance();
    REQUIRE(t.status() == NF::TsunamiStatus::Traveling);
    t.advance();
    REQUIRE(t.status() == NF::TsunamiStatus::Striking);
    REQUIRE(t.isStriking());
    t.advance();
    REQUIRE(t.status() == NF::TsunamiStatus::Receding);
    REQUIRE(t.isReceding());
}

TEST_CASE("Tsunami advance is no-op from Receding", "[Game][G41][Tsunami]") {
    NF::Tsunami t("t2");
    t.advance(); t.advance(); t.advance(); // reach Receding
    t.advance(); // should stay Receding
    REQUIRE(t.isReceding());
}

TEST_CASE("TsunamiWave isDevastating threshold", "[Game][G41][Tsunami]") {
    NF::TsunamiWave w;
    w.heightMeters = 9.9f;
    REQUIRE_FALSE(w.isDevastating());
    w.heightMeters = 10.f;
    REQUIRE(w.isDevastating());
}

TEST_CASE("TsunamiWave isFast threshold", "[Game][G41][Tsunami]") {
    NF::TsunamiWave w;
    w.speedKmh = 799.f;
    REQUIRE_FALSE(w.isFast());
    w.speedKmh = 800.f;
    REQUIRE(w.isFast());
}

TEST_CASE("Tsunami maxWaveHeight and isDevastating via waves", "[Game][G41][Tsunami]") {
    NF::Tsunami t("t3");
    NF::TsunamiWave w1; w1.heightMeters = 5.f;
    NF::TsunamiWave w2; w2.heightMeters = 15.f;
    t.addWave(w1); t.addWave(w2);
    REQUIRE(t.maxWaveHeight() == 15.f);
    REQUIRE(t.isDevastating());
    REQUIRE(t.waveCount() == 2);
}

TEST_CASE("TsunamiSystem create + duplicate rejection", "[Game][G41][Tsunami]") {
    NF::TsunamiSystem sys;
    REQUIRE(sys.create("pacific-1") != nullptr);
    REQUIRE(sys.create("indian-1")  != nullptr);
    REQUIRE(sys.create("pacific-1") == nullptr); // duplicate
    REQUIRE(sys.count() == 2);
}

TEST_CASE("TsunamiSystem strikingCount and devastatingCount", "[Game][G41][Tsunami]") {
    NF::TsunamiSystem sys;
    sys.create("a");
    sys.create("b");
    // Use find() after both are inserted to avoid dangling pointer on vector realloc
    sys.find("a")->advance(); sys.find("a")->advance(); // Forming → Traveling → Striking

    NF::TsunamiWave big; big.heightMeters = 20.f;
    sys.find("b")->addWave(big);

    REQUIRE(sys.strikingCount()    == 1);
    REQUIRE(sys.devastatingCount() == 1);
}

TEST_CASE("TsunamiSystem tick increments all tsunami tickCounts", "[Game][G41][Tsunami]") {
    NF::TsunamiSystem sys;
    sys.create("x"); sys.create("y");
    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 3);
    REQUIRE(sys.find("x")->tickCount() == 3);
    REQUIRE(sys.find("y")->tickCount() == 3);
}

// ============================================================
// G42 — Wildfire System
// ============================================================

TEST_CASE("WildfireType names cover all 8 values", "[Game][G42][Wildfire]") {
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Forest))       == "Forest");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Grassland))    == "Grassland");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Shrub))        == "Shrub");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Peat))         == "Peat");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Urban))        == "Urban");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Agricultural)) == "Agricultural");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Desert))       == "Desert");
    REQUIRE(std::string(NF::wildfireTypeName(NF::WildfireType::Tropical))     == "Tropical");
}

TEST_CASE("WildfireSeverity names cover all 5 values", "[Game][G42][Wildfire]") {
    REQUIRE(std::string(NF::wildfireSeverityName(NF::WildfireSeverity::Minor))        == "Minor");
    REQUIRE(std::string(NF::wildfireSeverityName(NF::WildfireSeverity::Moderate))     == "Moderate");
    REQUIRE(std::string(NF::wildfireSeverityName(NF::WildfireSeverity::Significant))  == "Significant");
    REQUIRE(std::string(NF::wildfireSeverityName(NF::WildfireSeverity::Major))        == "Major");
    REQUIRE(std::string(NF::wildfireSeverityName(NF::WildfireSeverity::Catastrophic)) == "Catastrophic");
}

TEST_CASE("WildfireFront contain/spread/isContained/isSpreading", "[Game][G42][Wildfire]") {
    NF::WildfireFront f;
    f.id = "front-1";
    f.spread(15.f);
    REQUIRE(f.isSpreading());
    REQUIRE_FALSE(f.isContained());
    f.contain();
    REQUIRE(f.isContained());
    REQUIRE_FALSE(f.isSpreading());
}

TEST_CASE("WildfireFront isCatastrophic threshold", "[Game][G42][Wildfire]") {
    NF::WildfireFront f;
    f.id = "front-2";
    REQUIRE_FALSE(f.isCatastrophic());
    f.severity = NF::WildfireSeverity::Catastrophic;
    REQUIRE(f.isCatastrophic());
}

TEST_CASE("WildfireZone addFront and duplicate rejection", "[Game][G42][Wildfire]") {
    NF::WildfireZone zone("australia-east");
    NF::WildfireFront f1; f1.id = "f1"; f1.spread(10.f);
    NF::WildfireFront f2; f2.id = "f2"; f2.spread(5.f);
    NF::WildfireFront dup; dup.id = "f1";

    REQUIRE(zone.addFront(f1));
    REQUIRE(zone.addFront(f2));
    REQUIRE_FALSE(zone.addFront(dup));
    REQUIRE(zone.frontCount() == 2);
}

TEST_CASE("WildfireZone containAll sets all fronts contained", "[Game][G42][Wildfire]") {
    NF::WildfireZone zone("california");
    NF::WildfireFront f1; f1.id = "f1"; f1.spread(20.f);
    NF::WildfireFront f2; f2.id = "f2"; f2.spread(8.f);
    zone.addFront(f1);
    zone.addFront(f2);
    zone.containAll();
    REQUIRE(zone.containedFronts() == 2);
    REQUIRE_FALSE(zone.isActive());
}

TEST_CASE("WildfireZone isActive requires spreading front", "[Game][G42][Wildfire]") {
    NF::WildfireZone zone("portugal");
    REQUIRE_FALSE(zone.isActive());
    NF::WildfireFront f; f.id = "f1"; f.spread(3.f);
    zone.addFront(f);
    REQUIRE(zone.isActive());
}

TEST_CASE("WildfireZone frontCount and containedFronts count", "[Game][G42][Wildfire]") {
    NF::WildfireZone zone("greece");
    NF::WildfireFront a; a.id = "a"; a.spread(5.f);
    NF::WildfireFront b; b.id = "b"; b.contained = true;
    zone.addFront(a);
    zone.addFront(b);
    REQUIRE(zone.frontCount() == 2);
    REQUIRE(zone.containedFronts() == 1);
}

TEST_CASE("WildfireSystem createZone and duplicate rejection", "[Game][G42][Wildfire]") {
    NF::WildfireSystem sys;
    REQUIRE(sys.createZone("zone-a") != nullptr);
    REQUIRE(sys.createZone("zone-b") != nullptr);
    REQUIRE(sys.createZone("zone-a") == nullptr); // duplicate
    REQUIRE(sys.zoneCount() == 2);
}

TEST_CASE("WildfireSystem activeCount and tick", "[Game][G42][Wildfire]") {
    NF::WildfireSystem sys;
    sys.createZone("z1");
    sys.createZone("z2");

    NF::WildfireFront f; f.id = "f1"; f.spread(12.f);
    sys.byName("z1")->addFront(f);
    sys.byName("z2")->setSeverity(NF::WildfireSeverity::Catastrophic);

    REQUIRE(sys.activeCount()       == 1);
    REQUIRE(sys.catastrophicCount() == 1);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

// ============================================================
// G43 — Flood System tests
// ============================================================

TEST_CASE("FloodType names cover all 8 values", "[Game][G43][Flood]") {
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::River))       == "River");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Coastal))     == "Coastal");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Flash))       == "Flash");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Urban))       == "Urban");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Groundwater)) == "Groundwater");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Dam))         == "Dam");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Snowmelt))    == "Snowmelt");
    REQUIRE(std::string(NF::floodTypeName(NF::FloodType::Tropical))    == "Tropical");
}

TEST_CASE("FloodSeverity names cover all 5 values", "[Game][G43][Flood]") {
    REQUIRE(std::string(NF::floodSeverityName(NF::FloodSeverity::Minor))        == "Minor");
    REQUIRE(std::string(NF::floodSeverityName(NF::FloodSeverity::Moderate))     == "Moderate");
    REQUIRE(std::string(NF::floodSeverityName(NF::FloodSeverity::Significant))  == "Significant");
    REQUIRE(std::string(NF::floodSeverityName(NF::FloodSeverity::Major))        == "Major");
    REQUIRE(std::string(NF::floodSeverityName(NF::FloodSeverity::Catastrophic)) == "Catastrophic");
}

TEST_CASE("FloodWaterLevel isRising and startReceding", "[Game][G43][Flood]") {
    NF::FloodWaterLevel lvl;
    lvl.id = "river-gauge-1";

    REQUIRE_FALSE(lvl.isRising());
    lvl.rise(0.5f);
    REQUIRE(lvl.isRising());
    lvl.startReceding();
    REQUIRE_FALSE(lvl.isRising());
    REQUIRE(lvl.receding);
}

TEST_CASE("FloodWaterLevel isDangerous and isCatastrophic thresholds", "[Game][G43][Flood]") {
    NF::FloodWaterLevel lvl;
    lvl.id = "gauge-2";

    lvl.depthMeters = 0.5f;
    REQUIRE_FALSE(lvl.isDangerous());
    REQUIRE_FALSE(lvl.isCatastrophic());

    lvl.depthMeters = 1.0f;
    REQUIRE(lvl.isDangerous());
    REQUIRE_FALSE(lvl.isCatastrophic());

    lvl.depthMeters = 5.0f;
    REQUIRE(lvl.isDangerous());
    REQUIRE(lvl.isCatastrophic());
}

TEST_CASE("FloodZone addLevel and duplicate rejection", "[Game][G43][Flood]") {
    NF::FloodZone zone("thames-basin");
    NF::FloodWaterLevel l1; l1.id = "l1"; l1.rise(0.3f);
    NF::FloodWaterLevel l2; l2.id = "l2"; l2.rise(0.1f);
    NF::FloodWaterLevel dup; dup.id = "l1";

    REQUIRE(zone.addLevel(l1));
    REQUIRE(zone.addLevel(l2));
    REQUIRE_FALSE(zone.addLevel(dup));
    REQUIRE(zone.levelCount() == 2);
}

TEST_CASE("FloodZone recessAll sets all levels receding", "[Game][G43][Flood]") {
    NF::FloodZone zone("mississippi");
    NF::FloodWaterLevel a; a.id = "a"; a.rise(1.0f);
    NF::FloodWaterLevel b; b.id = "b"; b.rise(0.5f);
    zone.addLevel(a);
    zone.addLevel(b);

    zone.recessAll();
    REQUIRE(zone.recedingLevels() == 2);
    REQUIRE_FALSE(zone.isFlooding());
}

TEST_CASE("FloodZone isFlooding requires rising level", "[Game][G43][Flood]") {
    NF::FloodZone zone("seine");
    REQUIRE_FALSE(zone.isFlooding());

    NF::FloodWaterLevel l; l.id = "g1"; l.rise(0.8f);
    zone.addLevel(l);
    REQUIRE(zone.isFlooding());
}

TEST_CASE("FloodZone levelCount and recedingLevels count", "[Game][G43][Flood]") {
    NF::FloodZone zone("danube");
    NF::FloodWaterLevel a; a.id = "a"; a.rise(0.4f);
    NF::FloodWaterLevel b; b.id = "b"; b.receding = true;
    zone.addLevel(a);
    zone.addLevel(b);

    REQUIRE(zone.levelCount() == 2);
    REQUIRE(zone.recedingLevels() == 1);
}

TEST_CASE("FloodSystem createZone and duplicate rejection", "[Game][G43][Flood]") {
    NF::FloodSystem sys;
    REQUIRE(sys.createZone("zone-a") != nullptr);
    REQUIRE(sys.createZone("zone-b") != nullptr);
    REQUIRE(sys.createZone("zone-a") == nullptr); // duplicate
    REQUIRE(sys.zoneCount() == 2);
}

TEST_CASE("FloodSystem floodingCount and tick", "[Game][G43][Flood]") {
    NF::FloodSystem sys;
    sys.createZone("z1");
    sys.createZone("z2");

    NF::FloodWaterLevel l; l.id = "l1"; l.rise(1.5f);
    sys.byName("z1")->addLevel(l);
    sys.byName("z2")->setSeverity(NF::FloodSeverity::Catastrophic);

    REQUIRE(sys.floodingCount()      == 1);
    REQUIRE(sys.catastrophicCount()  == 1);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

// ============================================================
// G44 — Landslide System
// ============================================================

TEST_CASE("LandslideType names cover all 8 values", "[Game][G44][Landslide]") {
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Debris))    == "Debris");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Rockfall))  == "Rockfall");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Mudflow))   == "Mudflow");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Slump))     == "Slump");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Creep))     == "Creep");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Avalanche)) == "Avalanche");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Earthflow)) == "Earthflow");
    REQUIRE(std::string(NF::landslideTypeName(NF::LandslideType::Topple))    == "Topple");
}

TEST_CASE("LandslideSeverity names cover all 5 values", "[Game][G44][Landslide]") {
    REQUIRE(std::string(NF::landslideSeverityName(NF::LandslideSeverity::Minor))        == "Minor");
    REQUIRE(std::string(NF::landslideSeverityName(NF::LandslideSeverity::Moderate))     == "Moderate");
    REQUIRE(std::string(NF::landslideSeverityName(NF::LandslideSeverity::Significant))  == "Significant");
    REQUIRE(std::string(NF::landslideSeverityName(NF::LandslideSeverity::Major))        == "Major");
    REQUIRE(std::string(NF::landslideSeverityName(NF::LandslideSeverity::Catastrophic)) == "Catastrophic");
}

TEST_CASE("LandslideDebrisFlow isMoving and halt", "[Game][G44][Landslide]") {
    NF::LandslideDebrisFlow flow;
    flow.id = "flow-1";

    REQUIRE_FALSE(flow.isMoving());
    flow.accelerate(5.0f);
    REQUIRE(flow.isMoving());
    flow.halt();
    REQUIRE_FALSE(flow.isMoving());
}

TEST_CASE("LandslideDebrisFlow isDangerous and isCatastrophic thresholds", "[Game][G44][Landslide]") {
    NF::LandslideDebrisFlow flow;
    flow.id = "flow-2";

    flow.volumeCubicMeters = 500.f;
    REQUIRE_FALSE(flow.isDangerous());
    REQUIRE_FALSE(flow.isCatastrophic());

    flow.volumeCubicMeters = 1000.f;
    REQUIRE(flow.isDangerous());
    REQUIRE_FALSE(flow.isCatastrophic());

    flow.volumeCubicMeters = 100000.f;
    REQUIRE(flow.isDangerous());
    REQUIRE(flow.isCatastrophic());
}

TEST_CASE("LandslideZone addFlow and duplicate rejection", "[Game][G44][Landslide]") {
    NF::LandslideZone zone("alps-north");
    NF::LandslideDebrisFlow f1; f1.id = "f1"; f1.accelerate(3.f);
    NF::LandslideDebrisFlow f2; f2.id = "f2"; f2.accelerate(1.f);
    NF::LandslideDebrisFlow dup; dup.id = "f1";

    REQUIRE(zone.addFlow(f1));
    REQUIRE(zone.addFlow(f2));
    REQUIRE_FALSE(zone.addFlow(dup));
    REQUIRE(zone.flowCount() == 2);
}

TEST_CASE("LandslideZone haltAll stops all flows", "[Game][G44][Landslide]") {
    NF::LandslideZone zone("pyrenees");
    NF::LandslideDebrisFlow a; a.id = "a"; a.accelerate(4.f);
    NF::LandslideDebrisFlow b; b.id = "b"; b.accelerate(2.f);
    zone.addFlow(a);
    zone.addFlow(b);

    REQUIRE(zone.movingFlows() == 2);
    zone.haltAll();
    REQUIRE(zone.movingFlows() == 0);
}

TEST_CASE("LandslideZone isActive requires moving flow", "[Game][G44][Landslide]") {
    NF::LandslideZone zone("andes");
    REQUIRE_FALSE(zone.isActive());

    NF::LandslideDebrisFlow f; f.id = "f1"; f.accelerate(1.5f);
    zone.addFlow(f);
    REQUIRE(zone.isActive());

    zone.haltAll();
    REQUIRE_FALSE(zone.isActive());
}

TEST_CASE("LandslideZone movingFlows count", "[Game][G44][Landslide]") {
    NF::LandslideZone zone("rockies");
    NF::LandslideDebrisFlow a; a.id = "a"; a.accelerate(3.f);
    NF::LandslideDebrisFlow b; b.id = "b"; b.accelerate(2.f); b.halt();
    NF::LandslideDebrisFlow c; c.id = "c"; c.accelerate(0.5f);
    zone.addFlow(a);
    zone.addFlow(b);
    zone.addFlow(c);

    REQUIRE(zone.movingFlows() == 2);
}

TEST_CASE("LandslideSystem createZone and duplicate rejection", "[Game][G44][Landslide]") {
    NF::LandslideSystem sys;
    REQUIRE(sys.createZone("zone-a") != nullptr);
    REQUIRE(sys.createZone("zone-b") != nullptr);
    REQUIRE(sys.createZone("zone-a") == nullptr); // duplicate
    REQUIRE(sys.zoneCount() == 2);
}

TEST_CASE("LandslideSystem activeCount and tick", "[Game][G44][Landslide]") {
    NF::LandslideSystem sys;
    sys.createZone("z1");
    sys.createZone("z2");

    NF::LandslideDebrisFlow f; f.id = "f1"; f.accelerate(2.f);
    sys.byName("z1")->addFlow(f);
    sys.byName("z2")->setSeverity(NF::LandslideSeverity::Catastrophic);

    REQUIRE(sys.activeCount()        == 1);
    REQUIRE(sys.catastrophicCount()  == 1);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

// ============================================================
// G45 — Drought System
// ============================================================

TEST_CASE("DroughtType names cover all 8 values", "[Game][G45][Drought]") {
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Agricultural))  == "Agricultural");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Hydrological))  == "Hydrological");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Meteorological))== "Meteorological");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Socioeconomic)) == "Socioeconomic");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Groundwater))   == "Groundwater");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Ecological))    == "Ecological");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Coastal))       == "Coastal");
    REQUIRE(std::string(NF::droughtTypeName(NF::DroughtType::Urban))         == "Urban");
}

TEST_CASE("DroughtIntensity names cover all 5 values", "[Game][G45][Drought]") {
    REQUIRE(std::string(NF::droughtIntensityName(NF::DroughtIntensity::Mild))        == "Mild");
    REQUIRE(std::string(NF::droughtIntensityName(NF::DroughtIntensity::Moderate))    == "Moderate");
    REQUIRE(std::string(NF::droughtIntensityName(NF::DroughtIntensity::Severe))      == "Severe");
    REQUIRE(std::string(NF::droughtIntensityName(NF::DroughtIntensity::Extreme))     == "Extreme");
    REQUIRE(std::string(NF::droughtIntensityName(NF::DroughtIntensity::Exceptional)) == "Exceptional");
}

TEST_CASE("DroughtRegion deplete clamps to 0", "[Game][G45][Drought]") {
    NF::DroughtRegion r;
    r.id = "r1";
    r.waterReservePercent = 20.f;
    r.deplete(30.f);
    REQUIRE(r.waterReservePercent == 0.f);
    r.deplete(10.f);
    REQUIRE(r.waterReservePercent == 0.f);
}

TEST_CASE("DroughtRegion replenish clamps to 100", "[Game][G45][Drought]") {
    NF::DroughtRegion r;
    r.id = "r2";
    r.waterReservePercent = 90.f;
    r.replenish(20.f);
    REQUIRE(r.waterReservePercent == 100.f);
    r.replenish(5.f);
    REQUIRE(r.waterReservePercent == 100.f);
}

TEST_CASE("DroughtRegion isArid / isCritical / isExhausted thresholds", "[Game][G45][Drought]") {
    NF::DroughtRegion r;
    r.id = "r3";

    r.waterReservePercent = 50.f;
    REQUIRE_FALSE(r.isArid());
    REQUIRE_FALSE(r.isCritical());
    REQUIRE_FALSE(r.isExhausted());

    r.waterReservePercent = 20.f;
    REQUIRE(r.isArid());
    REQUIRE_FALSE(r.isCritical());

    r.waterReservePercent = 5.f;
    REQUIRE(r.isArid());
    REQUIRE(r.isCritical());
    REQUIRE_FALSE(r.isExhausted());

    r.waterReservePercent = 0.f;
    REQUIRE(r.isExhausted());
    REQUIRE(r.isCritical());
}

TEST_CASE("DroughtZone addRegion and duplicate rejection", "[Game][G45][Drought]") {
    NF::DroughtZone zone("sahara");

    NF::DroughtRegion a; a.id = "r-a";
    NF::DroughtRegion b; b.id = "r-b";
    NF::DroughtRegion dup; dup.id = "r-a";

    REQUIRE(zone.addRegion(a));
    REQUIRE(zone.addRegion(b));
    REQUIRE_FALSE(zone.addRegion(dup));
    REQUIRE(zone.regionCount() == 2);
}

TEST_CASE("DroughtZone depleteAll reduces all regions", "[Game][G45][Drought]") {
    NF::DroughtZone zone("kalahari");

    NF::DroughtRegion a; a.id = "r1"; a.waterReservePercent = 80.f;
    NF::DroughtRegion b; b.id = "r2"; b.waterReservePercent = 60.f;
    zone.addRegion(a);
    zone.addRegion(b);

    zone.depleteAll(20.f);

    // Both regions should be depleted by 20
    REQUIRE(zone.aridCount() == 0); // 60 and 40 both >= 25
    zone.depleteAll(40.f);
    REQUIRE(zone.aridCount() == 2); // 20 and 0 both < 25
}

TEST_CASE("DroughtZone aridCount counts arid regions", "[Game][G45][Drought]") {
    NF::DroughtZone zone("gobi");

    NF::DroughtRegion a; a.id = "r1"; a.waterReservePercent = 10.f;
    NF::DroughtRegion b; b.id = "r2"; b.waterReservePercent = 50.f;
    NF::DroughtRegion c; c.id = "r3"; c.waterReservePercent = 15.f;
    zone.addRegion(a);
    zone.addRegion(b);
    zone.addRegion(c);

    REQUIRE(zone.aridCount() == 2);
}

TEST_CASE("DroughtSystem createZone and duplicate rejection", "[Game][G45][Drought]") {
    NF::DroughtSystem sys;
    REQUIRE(sys.createZone("zone-1") != nullptr);
    REQUIRE(sys.createZone("zone-2") != nullptr);
    REQUIRE(sys.createZone("zone-1") == nullptr); // duplicate
    REQUIRE(sys.zoneCount() == 2);
}

TEST_CASE("DroughtSystem criticalCount and tick", "[Game][G45][Drought]") {
    NF::DroughtSystem sys;
    sys.createZone("z1");
    sys.createZone("z2");

    NF::DroughtRegion cr; cr.id = "c1"; cr.waterReservePercent = 5.f;
    sys.byName("z1")->addRegion(cr);

    REQUIRE(sys.criticalCount() == 1);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

TEST_CASE("EpidemicType names cover all 8 values", "[Game][G46][Epidemic]") {
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Viral))      == "Viral");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Bacterial))  == "Bacterial");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Fungal))     == "Fungal");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Parasitic))  == "Parasitic");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Prion))      == "Prion");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Zoonotic))   == "Zoonotic");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Waterborne)) == "Waterborne");
    REQUIRE(std::string(NF::epidemicTypeName(NF::EpidemicType::Airborne))   == "Airborne");
}

TEST_CASE("EpidemicPhase names cover all 5 values", "[Game][G46][Epidemic]") {
    REQUIRE(std::string(NF::epidemicPhaseName(NF::EpidemicPhase::Outbreak))  == "Outbreak");
    REQUIRE(std::string(NF::epidemicPhaseName(NF::EpidemicPhase::Epidemic))  == "Epidemic");
    REQUIRE(std::string(NF::epidemicPhaseName(NF::EpidemicPhase::Endemic))   == "Endemic");
    REQUIRE(std::string(NF::epidemicPhaseName(NF::EpidemicPhase::Pandemic))  == "Pandemic");
    REQUIRE(std::string(NF::epidemicPhaseName(NF::EpidemicPhase::Resolved))  == "Resolved");
}

TEST_CASE("EpidemicVector infect clamps to populationSize", "[Game][G46][Epidemic]") {
    NF::EpidemicVector v;
    v.id = "v1"; v.populationSize = 100;

    v.infect(60);
    REQUIRE(v.infectedCount == 60);
    v.infect(60); // would exceed 100
    REQUIRE(v.infectedCount == 100);
}

TEST_CASE("EpidemicVector recover clamps to 0", "[Game][G46][Epidemic]") {
    NF::EpidemicVector v;
    v.id = "v2"; v.populationSize = 100;

    v.infect(40);
    v.recover(20);
    REQUIRE(v.infectedCount == 20);
    v.recover(50); // would go below 0
    REQUIRE(v.infectedCount == 0);
}

TEST_CASE("EpidemicVector infectionRate calculation", "[Game][G46][Epidemic]") {
    NF::EpidemicVector v;
    v.id = "v3"; v.populationSize = 1000;

    v.infect(500);
    REQUIRE(v.infectionRate() == Catch::Approx(0.5f));

    NF::EpidemicVector empty;
    empty.id = "v4"; empty.populationSize = 0;
    REQUIRE(empty.infectionRate() == Catch::Approx(0.f));
}

TEST_CASE("EpidemicVector isContained and isCritical thresholds", "[Game][G46][Epidemic]") {
    NF::EpidemicVector v;
    v.id = "v5"; v.populationSize = 1000;

    REQUIRE(v.isContained());  // 0% < 5%
    REQUIRE_FALSE(v.isCritical());

    v.infect(500);
    REQUIRE_FALSE(v.isContained());
    REQUIRE(v.isCritical()); // 50% >= 50%
}

TEST_CASE("EpidemicZone addVector and duplicate rejection", "[Game][G46][Epidemic]") {
    NF::EpidemicZone zone("test-zone");

    NF::EpidemicVector a; a.id = "vec-a";
    NF::EpidemicVector b; b.id = "vec-b";
    NF::EpidemicVector dup; dup.id = "vec-a";

    REQUIRE(zone.addVector(a));
    REQUIRE(zone.addVector(b));
    REQUIRE_FALSE(zone.addVector(dup));
    REQUIRE(zone.vectorCount() == 2);
}

TEST_CASE("EpidemicZone infectAll and recoverAll", "[Game][G46][Epidemic]") {
    NF::EpidemicZone zone("spread-zone");

    NF::EpidemicVector a; a.id = "a"; a.populationSize = 100;
    NF::EpidemicVector b; b.id = "b"; b.populationSize = 200;
    zone.addVector(a);
    zone.addVector(b);

    // infect all, then verify criticalCount
    zone.infectAll(600); // clamps both to their population size
    REQUIRE(zone.criticalCount() == 2); // both at 100% >= 50% threshold

    zone.recoverAll(1000); // clamp both back to 0
    REQUIRE(zone.criticalCount() == 0);
}

TEST_CASE("EpidemicSystem createZone and duplicate rejection", "[Game][G46][Epidemic]") {
    NF::EpidemicSystem sys;
    REQUIRE(sys.createZone("zone-1") != nullptr);
    REQUIRE(sys.createZone("zone-2") != nullptr);
    REQUIRE(sys.createZone("zone-1") == nullptr); // duplicate
    REQUIRE(sys.zoneCount() == 2);
}

TEST_CASE("EpidemicSystem criticalZoneCount and containedZoneCount", "[Game][G46][Epidemic]") {
    NF::EpidemicSystem sys;
    sys.createZone("z1");
    sys.createZone("z2");

    NF::EpidemicVector cv; cv.id = "cv1"; cv.populationSize = 100;
    cv.infect(80); // 80% — critical
    sys.byName("z1")->addVector(cv);

    NF::EpidemicVector sv; sv.id = "sv1"; sv.populationSize = 100;
    // no infection — contained
    sys.byName("z2")->addVector(sv);

    REQUIRE(sys.criticalZoneCount()   == 1);
    REQUIRE(sys.containedZoneCount()  == 1);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

TEST_CASE("SolarFlareClass names cover all 8 values", "[Game][G47][SolarFlare]") {
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::A)) == "A");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::B)) == "B");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::C)) == "C");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::M)) == "M");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::X)) == "X");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::S)) == "S");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::N)) == "N");
    REQUIRE(std::string(NF::solarFlareClassName(NF::SolarFlareClass::Z)) == "Z");
}

TEST_CASE("SolarFlareEffect names cover all 6 values", "[Game][G47][SolarFlare]") {
    REQUIRE(std::string(NF::solarFlareEffectName(NF::SolarFlareEffect::RadioBlackout))       == "RadioBlackout");
    REQUIRE(std::string(NF::solarFlareEffectName(NF::SolarFlareEffect::RadiationStorm))      == "RadiationStorm");
    REQUIRE(std::string(NF::solarFlareEffectName(NF::SolarFlareEffect::GeomagneticStorm))    == "GeomagneticStorm");
    REQUIRE(std::string(NF::solarFlareEffectName(NF::SolarFlareEffect::PowerGridDisruption)) == "PowerGridDisruption");
    REQUIRE(std::string(NF::solarFlareEffectName(NF::SolarFlareEffect::SatelliteDamage))     == "SatelliteDamage");
    REQUIRE(std::string(NF::solarFlareEffectName(NF::SolarFlareEffect::CommunicationLoss))   == "CommunicationLoss");
}

TEST_CASE("SolarFlareEvent isMajor and isExtreme thresholds", "[Game][G47][SolarFlare]") {
    NF::SolarFlareEvent e;
    e.id = "ev1"; e.flareClass = NF::SolarFlareClass::C;

    REQUIRE_FALSE(e.isMajor());
    REQUIRE_FALSE(e.isExtreme());

    e.flareClass = NF::SolarFlareClass::M;
    REQUIRE(e.isMajor());
    REQUIRE_FALSE(e.isExtreme());

    e.flareClass = NF::SolarFlareClass::X;
    REQUIRE(e.isMajor());
    REQUIRE(e.isExtreme());
}

TEST_CASE("SolarFlareEvent energyOutput calculation", "[Game][G47][SolarFlare]") {
    NF::SolarFlareEvent e;
    e.id = "ev2"; e.intensity = 2.0f; e.duration = 30.f;

    REQUIRE(e.energyOutput() == Catch::Approx(60.f));
}

TEST_CASE("SolarFlareEvent activate and deactivate", "[Game][G47][SolarFlare]") {
    NF::SolarFlareEvent e;
    e.id = "ev3";

    REQUIRE_FALSE(e.active);
    e.activate();
    REQUIRE(e.active);
    e.deactivate();
    REQUIRE_FALSE(e.active);
}

TEST_CASE("SolarFlareRegion addEvent and duplicate rejection", "[Game][G47][SolarFlare]") {
    NF::SolarFlareRegion region("region-1");

    NF::SolarFlareEvent a; a.id = "ev-a"; a.flareClass = NF::SolarFlareClass::M;
    NF::SolarFlareEvent b; b.id = "ev-b"; b.flareClass = NF::SolarFlareClass::X;
    NF::SolarFlareEvent dup; dup.id = "ev-a";

    REQUIRE(region.addEvent(a));
    REQUIRE(region.addEvent(b));
    REQUIRE_FALSE(region.addEvent(dup));
    REQUIRE(region.eventCount() == 2);
}

TEST_CASE("SolarFlareRegion activateAll and deactivateAll, activeCount", "[Game][G47][SolarFlare]") {
    NF::SolarFlareRegion region("region-2");

    NF::SolarFlareEvent a; a.id = "a1";
    NF::SolarFlareEvent b; b.id = "a2";
    region.addEvent(a);
    region.addEvent(b);

    REQUIRE(region.activeCount() == 0);
    region.activateAll();
    REQUIRE(region.activeCount() == 2);
    region.deactivateAll();
    REQUIRE(region.activeCount() == 0);
}

TEST_CASE("SolarFlareRegion majorCount counts M+ class events", "[Game][G47][SolarFlare]") {
    NF::SolarFlareRegion region("region-3");

    NF::SolarFlareEvent minor; minor.id = "c1"; minor.flareClass = NF::SolarFlareClass::C;
    NF::SolarFlareEvent major; major.id = "m1"; major.flareClass = NF::SolarFlareClass::M;
    NF::SolarFlareEvent extreme; extreme.id = "x1"; extreme.flareClass = NF::SolarFlareClass::X;

    region.addEvent(minor);
    region.addEvent(major);
    region.addEvent(extreme);

    REQUIRE(region.majorCount() == 2);
}

TEST_CASE("SolarFlareSystem createRegion and duplicate rejection", "[Game][G47][SolarFlare]") {
    NF::SolarFlareSystem sys;
    REQUIRE(sys.createRegion("solar-a") != nullptr);
    REQUIRE(sys.createRegion("solar-b") != nullptr);
    REQUIRE(sys.createRegion("solar-a") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("SolarFlareSystem activeEventCount, majorEventCount and tick", "[Game][G47][SolarFlare]") {
    NF::SolarFlareSystem sys;
    sys.createRegion("r1");
    sys.createRegion("r2");

    NF::SolarFlareEvent ae; ae.id = "ae1"; ae.flareClass = NF::SolarFlareClass::X; ae.active = true;
    NF::SolarFlareEvent me; me.id = "me1"; me.flareClass = NF::SolarFlareClass::M;
    sys.byName("r1")->addEvent(ae);
    sys.byName("r2")->addEvent(me);

    REQUIRE(sys.activeEventCount()  == 1);
    REQUIRE(sys.majorEventCount()   == 2); // X and M are both major

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

TEST_CASE("MeteorShowerClass names cover all 8 values", "[Game][G48][MeteorShower]") {
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Sporadic))  == "Sporadic");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Minor))     == "Minor");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Moderate))  == "Moderate");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Major))     == "Major");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Annual))    == "Annual");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Periodic))  == "Periodic");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Storm))     == "Storm");
    REQUIRE(std::string(NF::meteorShowerClassName(NF::MeteorShowerClass::Outburst))  == "Outburst");
}

TEST_CASE("MeteorImpactType names cover all 6 values", "[Game][G48][MeteorShower]") {
    REQUIRE(std::string(NF::meteorImpactTypeName(NF::MeteorImpactType::Airburst))        == "Airburst");
    REQUIRE(std::string(NF::meteorImpactTypeName(NF::MeteorImpactType::CraterFormation)) == "CraterFormation");
    REQUIRE(std::string(NF::meteorImpactTypeName(NF::MeteorImpactType::Graze))           == "Graze");
    REQUIRE(std::string(NF::meteorImpactTypeName(NF::MeteorImpactType::Ablation))        == "Ablation");
    REQUIRE(std::string(NF::meteorImpactTypeName(NF::MeteorImpactType::Penetrating))     == "Penetrating");
    REQUIRE(std::string(NF::meteorImpactTypeName(NF::MeteorImpactType::Fragmentation))   == "Fragmentation");
}

TEST_CASE("MeteorEvent isMajor and isMinor thresholds", "[Game][G48][MeteorShower]") {
    NF::MeteorEvent ev;
    ev.id = "ev1"; ev.showerClass = NF::MeteorShowerClass::Minor;

    REQUIRE(ev.isMinor());
    REQUIRE_FALSE(ev.isMajor());

    ev.showerClass = NF::MeteorShowerClass::Major;
    REQUIRE_FALSE(ev.isMinor());
    REQUIRE(ev.isMajor());

    ev.showerClass = NF::MeteorShowerClass::Storm;
    REQUIRE(ev.isMajor());
}

TEST_CASE("MeteorEvent radiationFlux calculation", "[Game][G48][MeteorShower]") {
    NF::MeteorEvent ev;
    ev.id = "ev2"; ev.intensity = 3.0f; ev.duration = 4.0f;

    REQUIRE(ev.radiationFlux() == Catch::Approx(12.0f));
}

TEST_CASE("MeteorEvent activate and deactivate toggle active", "[Game][G48][MeteorShower]") {
    NF::MeteorEvent ev;
    ev.id = "ev3";

    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("MeteorShowerRegion addEvent and duplicate rejection", "[Game][G48][MeteorShower]") {
    NF::MeteorShowerRegion region("region-1");

    NF::MeteorEvent a; a.id = "ev-a";
    NF::MeteorEvent b; b.id = "ev-b";
    NF::MeteorEvent dup; dup.id = "ev-a";

    REQUIRE(region.addEvent(a));
    REQUIRE(region.addEvent(b));
    REQUIRE_FALSE(region.addEvent(dup));
    REQUIRE(region.eventCount() == 2);
}

TEST_CASE("MeteorShowerRegion activateAll and deactivateAll", "[Game][G48][MeteorShower]") {
    NF::MeteorShowerRegion region("region-2");

    NF::MeteorEvent a; a.id = "a";
    NF::MeteorEvent b; b.id = "b";
    region.addEvent(a);
    region.addEvent(b);

    REQUIRE(region.activeCount() == 0);
    region.activateAll();
    REQUIRE(region.activeCount() == 2);
    region.deactivateAll();
    REQUIRE(region.activeCount() == 0);
}

TEST_CASE("MeteorShowerRegion activeCount and majorCount", "[Game][G48][MeteorShower]") {
    NF::MeteorShowerRegion region("region-3");

    NF::MeteorEvent major; major.id = "maj"; major.showerClass = NF::MeteorShowerClass::Major; major.activate();
    NF::MeteorEvent minor; minor.id = "min"; minor.showerClass = NF::MeteorShowerClass::Minor;

    region.addEvent(major);
    region.addEvent(minor);

    REQUIRE(region.activeCount() == 1);
    REQUIRE(region.majorCount()  == 1);
}

TEST_CASE("MeteorShowerSystem createRegion and duplicate rejection", "[Game][G48][MeteorShower]") {
    NF::MeteorShowerSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("MeteorShowerSystem tick propagates to all regions", "[Game][G48][MeteorShower]") {
    NF::MeteorShowerSystem sys;
    sys.createRegion("r1");
    sys.createRegion("r2");

    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 3);
    REQUIRE(sys.byName("r1")->tickCount() == 3);
    REQUIRE(sys.byName("r2")->tickCount() == 3);
}

TEST_CASE("MeteorShowerSystem activeEventCount and majorEventCount", "[Game][G48][MeteorShower]") {
    NF::MeteorShowerSystem sys;
    sys.createRegion("r1");
    sys.createRegion("r2");

    NF::MeteorEvent ae; ae.id = "ae1"; ae.showerClass = NF::MeteorShowerClass::Storm; ae.activate();
    NF::MeteorEvent me; me.id = "me1"; me.showerClass = NF::MeteorShowerClass::Outburst;

    sys.byName("r1")->addEvent(ae);
    sys.byName("r2")->addEvent(me);

    REQUIRE(sys.activeEventCount() == 1);
    REQUIRE(sys.majorEventCount()  == 2); // Storm and Outburst are both Major

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount() == 2);
}

// ── G49 — Aurora System ───────────────────────────────────────────

TEST_CASE("AuroraType names cover all 8 values", "[Game][G49][Aurora]") {
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Borealis))  == "Borealis");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Australis)) == "Australis");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Polar))     == "Polar");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Substorm))  == "Substorm");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Diffuse))   == "Diffuse");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Discrete))  == "Discrete");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Pulsating)) == "Pulsating");
    REQUIRE(std::string(NF::auroraTypeName(NF::AuroraType::Custom))    == "Custom");
}

TEST_CASE("AuroraIntensity names cover all 6 values", "[Game][G49][Aurora]") {
    REQUIRE(std::string(NF::auroraIntensityName(NF::AuroraIntensity::Faint))   == "Faint");
    REQUIRE(std::string(NF::auroraIntensityName(NF::AuroraIntensity::Quiet))   == "Quiet");
    REQUIRE(std::string(NF::auroraIntensityName(NF::AuroraIntensity::Active))  == "Active");
    REQUIRE(std::string(NF::auroraIntensityName(NF::AuroraIntensity::Storm))   == "Storm");
    REQUIRE(std::string(NF::auroraIntensityName(NF::AuroraIntensity::Severe))  == "Severe");
    REQUIRE(std::string(NF::auroraIntensityName(NF::AuroraIntensity::Extreme)) == "Extreme");
}

TEST_CASE("AuroraEvent isVisible and isStorm thresholds", "[Game][G49][Aurora]") {
    NF::AuroraEvent ev;
    ev.id = "ev1"; ev.intensity = NF::AuroraIntensity::Quiet;
    REQUIRE_FALSE(ev.isVisible());
    REQUIRE_FALSE(ev.isStorm());

    ev.intensity = NF::AuroraIntensity::Active;
    REQUIRE(ev.isVisible());
    REQUIRE_FALSE(ev.isStorm());

    ev.intensity = NF::AuroraIntensity::Storm;
    REQUIRE(ev.isVisible());
    REQUIRE(ev.isStorm());

    ev.intensity = NF::AuroraIntensity::Extreme;
    REQUIRE(ev.isVisible());
    REQUIRE(ev.isStorm());
}

TEST_CASE("AuroraEvent radiationIndex calculation", "[Game][G49][Aurora]") {
    NF::AuroraEvent ev;
    ev.intensity = NF::AuroraIntensity::Active; // index 2 → (2+1)*duration
    ev.duration  = 2.0f;
    REQUIRE(ev.radiationIndex() == Catch::Approx(6.0f));
}

TEST_CASE("AuroraEvent activate and deactivate toggle active", "[Game][G49][Aurora]") {
    NF::AuroraEvent ev; ev.id = "aurora-1";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("AuroraRegion addEvent and duplicate rejection", "[Game][G49][Aurora]") {
    NF::AuroraRegion region("north-pole");

    NF::AuroraEvent a; a.id = "ev-a";
    NF::AuroraEvent b; b.id = "ev-b";
    NF::AuroraEvent dup; dup.id = "ev-a";

    REQUIRE(region.addEvent(a));
    REQUIRE(region.addEvent(b));
    REQUIRE_FALSE(region.addEvent(dup));
    REQUIRE(region.eventCount() == 2);
}

TEST_CASE("AuroraRegion activateAll and deactivateAll", "[Game][G49][Aurora]") {
    NF::AuroraRegion region("south-pole");

    NF::AuroraEvent a; a.id = "a";
    NF::AuroraEvent b; b.id = "b";
    region.addEvent(a);
    region.addEvent(b);

    REQUIRE(region.activeCount() == 0);
    region.activateAll();
    REQUIRE(region.activeCount() == 2);
    region.deactivateAll();
    REQUIRE(region.activeCount() == 0);
}

TEST_CASE("AuroraRegion activeCount and stormCount", "[Game][G49][Aurora]") {
    NF::AuroraRegion region("polar-cap");

    NF::AuroraEvent storm; storm.id = "s1"; storm.intensity = NF::AuroraIntensity::Storm; storm.activate();
    NF::AuroraEvent faint; faint.id = "f1"; faint.intensity = NF::AuroraIntensity::Faint; faint.activate();
    NF::AuroraEvent idle;  idle.id  = "i1"; idle.intensity  = NF::AuroraIntensity::Quiet;

    region.addEvent(storm);
    region.addEvent(faint);
    region.addEvent(idle);

    REQUIRE(region.activeCount() == 2);
    REQUIRE(region.stormCount()  == 1);
}

TEST_CASE("AuroraSystem createRegion and duplicate rejection", "[Game][G49][Aurora]") {
    NF::AuroraSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("AuroraSystem tick propagates to all regions", "[Game][G49][Aurora]") {
    NF::AuroraSystem sys;
    sys.createRegion("a");
    sys.createRegion("b");

    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()           == 3);
    REQUIRE(sys.byName("a")->tickCount() == 3);
    REQUIRE(sys.byName("b")->tickCount() == 3);
}

TEST_CASE("AuroraSystem activeEventCount and stormEventCount", "[Game][G49][Aurora]") {
    NF::AuroraSystem sys;
    sys.createRegion("reg1");
    sys.createRegion("reg2");

    NF::AuroraEvent ae; ae.id = "ae1"; ae.intensity = NF::AuroraIntensity::Severe; ae.activate();
    NF::AuroraEvent se; se.id = "se1"; se.intensity = NF::AuroraIntensity::Storm;  se.activate();
    NF::AuroraEvent ie; ie.id = "ie1"; ie.intensity = NF::AuroraIntensity::Quiet;

    sys.byName("reg1")->addEvent(ae);
    sys.byName("reg1")->addEvent(ie);
    sys.byName("reg2")->addEvent(se);

    REQUIRE(sys.activeEventCount() == 2); // ae and se are active
    REQUIRE(sys.stormEventCount()  == 2); // Severe and Storm both >= Storm

    sys.tick();
    REQUIRE(sys.tickCount() == 1);
}

// ── G50 — Heatwave System ─────────────────────────────────────────

TEST_CASE("HeatwaveType names cover all 8 values", "[Game][G50][Heatwave]") {
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Regional))    == "Regional");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Urban))       == "Urban");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Coastal))     == "Coastal");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Continental)) == "Continental");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Desert))      == "Desert");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Tropical))    == "Tropical");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Polar))       == "Polar");
    REQUIRE(std::string(NF::heatwaveTypeName(NF::HeatwaveType::Custom))      == "Custom");
}

TEST_CASE("HeatwaveSeverity names cover all 6 values", "[Game][G50][Heatwave]") {
    REQUIRE(std::string(NF::heatwaveSeverityName(NF::HeatwaveSeverity::Mild))          == "Mild");
    REQUIRE(std::string(NF::heatwaveSeverityName(NF::HeatwaveSeverity::Moderate))      == "Moderate");
    REQUIRE(std::string(NF::heatwaveSeverityName(NF::HeatwaveSeverity::Severe))        == "Severe");
    REQUIRE(std::string(NF::heatwaveSeverityName(NF::HeatwaveSeverity::Extreme))       == "Extreme");
    REQUIRE(std::string(NF::heatwaveSeverityName(NF::HeatwaveSeverity::Catastrophic))  == "Catastrophic");
    REQUIRE(std::string(NF::heatwaveSeverityName(NF::HeatwaveSeverity::Unprecedented)) == "Unprecedented");
}

TEST_CASE("HeatwaveEvent isDangerous and isExtreme thresholds", "[Game][G50][Heatwave]") {
    NF::HeatwaveEvent ev;
    ev.id = "hw1"; ev.severity = NF::HeatwaveSeverity::Mild;
    REQUIRE_FALSE(ev.isDangerous());
    REQUIRE_FALSE(ev.isExtreme());

    ev.severity = NF::HeatwaveSeverity::Severe;
    REQUIRE(ev.isDangerous());
    REQUIRE_FALSE(ev.isExtreme());

    ev.severity = NF::HeatwaveSeverity::Extreme;
    REQUIRE(ev.isDangerous());
    REQUIRE(ev.isExtreme());

    ev.severity = NF::HeatwaveSeverity::Unprecedented;
    REQUIRE(ev.isDangerous());
    REQUIRE(ev.isExtreme());
}

TEST_CASE("HeatwaveEvent heatIndex calculation", "[Game][G50][Heatwave]") {
    NF::HeatwaveEvent ev;
    ev.severity = NF::HeatwaveSeverity::Severe; // index 2 → (2+1)*peakTemp
    ev.peakTemp = 40.0f;
    REQUIRE(ev.heatIndex() == Catch::Approx(120.0f));
}

TEST_CASE("HeatwaveEvent activate and deactivate toggle active", "[Game][G50][Heatwave]") {
    NF::HeatwaveEvent ev; ev.id = "hw-1";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("HeatwaveRegion addEvent and duplicate rejection", "[Game][G50][Heatwave]") {
    NF::HeatwaveRegion region("sahara");

    NF::HeatwaveEvent a; a.id = "ev-a";
    NF::HeatwaveEvent b; b.id = "ev-b";
    NF::HeatwaveEvent dup; dup.id = "ev-a";

    REQUIRE(region.addEvent(a));
    REQUIRE(region.addEvent(b));
    REQUIRE_FALSE(region.addEvent(dup));
    REQUIRE(region.eventCount() == 2);
}

TEST_CASE("HeatwaveRegion activateAll and deactivateAll", "[Game][G50][Heatwave]") {
    NF::HeatwaveRegion region("mediterranean");

    NF::HeatwaveEvent a; a.id = "a";
    NF::HeatwaveEvent b; b.id = "b";
    region.addEvent(a);
    region.addEvent(b);

    REQUIRE(region.activeCount() == 0);
    region.activateAll();
    REQUIRE(region.activeCount() == 2);
    region.deactivateAll();
    REQUIRE(region.activeCount() == 0);
}

TEST_CASE("HeatwaveRegion activeCount and extremeCount", "[Game][G50][Heatwave]") {
    NF::HeatwaveRegion region("asia-pacific");

    NF::HeatwaveEvent extreme; extreme.id = "e1"; extreme.severity = NF::HeatwaveSeverity::Extreme; extreme.activate();
    NF::HeatwaveEvent mild;    mild.id    = "m1"; mild.severity    = NF::HeatwaveSeverity::Mild;    mild.activate();
    NF::HeatwaveEvent idle;    idle.id    = "i1"; idle.severity    = NF::HeatwaveSeverity::Moderate;

    region.addEvent(extreme);
    region.addEvent(mild);
    region.addEvent(idle);

    REQUIRE(region.activeCount()  == 2);
    REQUIRE(region.extremeCount() == 1);
}

TEST_CASE("HeatwaveSystem createRegion and duplicate rejection", "[Game][G50][Heatwave]") {
    NF::HeatwaveSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("HeatwaveSystem tick propagates to all regions", "[Game][G50][Heatwave]") {
    NF::HeatwaveSystem sys;
    sys.createRegion("a");
    sys.createRegion("b");

    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()              == 3);
    REQUIRE(sys.byName("a")->tickCount() == 3);
    REQUIRE(sys.byName("b")->tickCount() == 3);
}

TEST_CASE("HeatwaveSystem activeEventCount and extremeEventCount", "[Game][G50][Heatwave]") {
    NF::HeatwaveSystem sys;
    sys.createRegion("reg1");
    sys.createRegion("reg2");

    NF::HeatwaveEvent ae; ae.id = "ae1"; ae.severity = NF::HeatwaveSeverity::Catastrophic; ae.activate();
    NF::HeatwaveEvent xe; xe.id = "xe1"; xe.severity = NF::HeatwaveSeverity::Extreme;      xe.activate();
    NF::HeatwaveEvent ie; ie.id = "ie1"; ie.severity = NF::HeatwaveSeverity::Mild;

    sys.byName("reg1")->addEvent(ae);
    sys.byName("reg1")->addEvent(ie);
    sys.byName("reg2")->addEvent(xe);

    REQUIRE(sys.activeEventCount()  == 2); // ae and xe are active
    REQUIRE(sys.extremeEventCount() == 2); // Catastrophic and Extreme both >= Extreme

    sys.tick();
    REQUIRE(sys.tickCount() == 1);
}

// ── G51 — Blizzard System ─────────────────────────────────────────

TEST_CASE("BlizzardType names cover all 8 values", "[Game][G51][Blizzard]") {
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Arctic))    == "Arctic");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Alpine))    == "Alpine");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Coastal))   == "Coastal");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Prairie))   == "Prairie");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Lake))      == "Lake");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Polar))     == "Polar");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Nor_easter)) == "Nor'easter");
    REQUIRE(std::string(NF::blizzardTypeName(NF::BlizzardType::Custom))    == "Custom");
}

TEST_CASE("BlizzardIntensity names cover all 6 values", "[Game][G51][Blizzard]") {
    REQUIRE(std::string(NF::blizzardIntensityName(NF::BlizzardIntensity::Light))    == "Light");
    REQUIRE(std::string(NF::blizzardIntensityName(NF::BlizzardIntensity::Moderate)) == "Moderate");
    REQUIRE(std::string(NF::blizzardIntensityName(NF::BlizzardIntensity::Heavy))    == "Heavy");
    REQUIRE(std::string(NF::blizzardIntensityName(NF::BlizzardIntensity::Severe))   == "Severe");
    REQUIRE(std::string(NF::blizzardIntensityName(NF::BlizzardIntensity::Extreme))  == "Extreme");
    REQUIRE(std::string(NF::blizzardIntensityName(NF::BlizzardIntensity::Whiteout)) == "Whiteout");
}

TEST_CASE("BlizzardEvent isSevere and isWhiteout thresholds", "[Game][G51][Blizzard]") {
    NF::BlizzardEvent ev; ev.id = "bz1";

    ev.intensity = NF::BlizzardIntensity::Light;
    REQUIRE_FALSE(ev.isSevere());
    REQUIRE_FALSE(ev.isWhiteout());

    ev.intensity = NF::BlizzardIntensity::Severe;
    REQUIRE(ev.isSevere());
    REQUIRE_FALSE(ev.isWhiteout());

    ev.intensity = NF::BlizzardIntensity::Whiteout;
    REQUIRE(ev.isSevere());
    REQUIRE(ev.isWhiteout());
}

TEST_CASE("BlizzardEvent visibilityIndex decreases with intensity", "[Game][G51][Blizzard]") {
    NF::BlizzardEvent ev; ev.id = "bz2";

    ev.intensity = NF::BlizzardIntensity::Light;    // Whiteout(5) - Light(0) = 5
    REQUIRE(ev.visibilityIndex() == Catch::Approx(5.0f));

    ev.intensity = NF::BlizzardIntensity::Whiteout; // 5 - 5 = 0
    REQUIRE(ev.visibilityIndex() == Catch::Approx(0.0f));
}

TEST_CASE("BlizzardEvent activate and deactivate toggle active", "[Game][G51][Blizzard]") {
    NF::BlizzardEvent ev; ev.id = "bz3";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("BlizzardRegion addEvent and duplicate rejection", "[Game][G51][Blizzard]") {
    NF::BlizzardRegion region("greenland");

    NF::BlizzardEvent a; a.id = "ev-a";
    NF::BlizzardEvent b; b.id = "ev-b";
    NF::BlizzardEvent dup; dup.id = "ev-a";

    REQUIRE(region.addEvent(a));
    REQUIRE(region.addEvent(b));
    REQUIRE_FALSE(region.addEvent(dup));
    REQUIRE(region.eventCount() == 2);
}

TEST_CASE("BlizzardRegion activateAll and deactivateAll", "[Game][G51][Blizzard]") {
    NF::BlizzardRegion region("alaska");

    NF::BlizzardEvent a; a.id = "a";
    NF::BlizzardEvent b; b.id = "b";
    region.addEvent(a);
    region.addEvent(b);

    REQUIRE(region.activeCount() == 0);
    region.activateAll();
    REQUIRE(region.activeCount() == 2);
    region.deactivateAll();
    REQUIRE(region.activeCount() == 0);
}

TEST_CASE("BlizzardRegion severeCount and whiteoutCount", "[Game][G51][Blizzard]") {
    NF::BlizzardRegion region("siberia");

    NF::BlizzardEvent wo; wo.id = "w1"; wo.intensity = NF::BlizzardIntensity::Whiteout; wo.activate();
    NF::BlizzardEvent sv; sv.id = "s1"; sv.intensity = NF::BlizzardIntensity::Severe;   sv.activate();
    NF::BlizzardEvent lt; lt.id = "l1"; lt.intensity = NF::BlizzardIntensity::Light;    lt.activate();

    region.addEvent(wo);
    region.addEvent(sv);
    region.addEvent(lt);

    REQUIRE(region.activeCount()   == 3);
    REQUIRE(region.severeCount()   == 2); // Whiteout and Severe are both >= Severe
    REQUIRE(region.whiteoutCount() == 1);
}

TEST_CASE("BlizzardSystem createRegion and duplicate rejection", "[Game][G51][Blizzard]") {
    NF::BlizzardSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr);
    REQUIRE(sys.regionCount() == 2);
}

TEST_CASE("BlizzardSystem tick propagates to all regions", "[Game][G51][Blizzard]") {
    NF::BlizzardSystem sys;
    sys.createRegion("a");
    sys.createRegion("b");

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()              == 2);
    REQUIRE(sys.byName("a")->tickCount() == 2);
    REQUIRE(sys.byName("b")->tickCount() == 2);
}

TEST_CASE("BlizzardSystem activeEventCount, severeEventCount, whiteoutEventCount", "[Game][G51][Blizzard]") {
    NF::BlizzardSystem sys;
    sys.createRegion("region-a");
    sys.createRegion("region-b");

    NF::BlizzardEvent w1; w1.id = "w1"; w1.intensity = NF::BlizzardIntensity::Whiteout; w1.activate();
    NF::BlizzardEvent s1; s1.id = "s1"; s1.intensity = NF::BlizzardIntensity::Severe;   s1.activate();
    NF::BlizzardEvent l1; l1.id = "l1"; l1.intensity = NF::BlizzardIntensity::Light;

    sys.byName("region-a")->addEvent(w1);
    sys.byName("region-a")->addEvent(l1);
    sys.byName("region-b")->addEvent(s1);

    REQUIRE(sys.activeEventCount()   == 2); // w1 and s1 active
    REQUIRE(sys.severeEventCount()   == 2); // both w1 and s1 >= Severe
    REQUIRE(sys.whiteoutEventCount() == 1); // only w1 is whiteout
}

// ── G52 — Sandstorm System ────────────────────────────────────────

TEST_CASE("SandstormType names cover all 8 values", "[Game][G52][Sandstorm]") {
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Haboob))    == "Haboob");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Simoom))    == "Simoom");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Khamsin))   == "Khamsin");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Shamal))    == "Shamal");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Harmattan)) == "Harmattan");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Sirocco))   == "Sirocco");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Zonda))     == "Zonda");
    REQUIRE(std::string(NF::sandstormTypeName(NF::SandstormType::Custom))    == "Custom");
}

TEST_CASE("SandstormSeverity names cover all 6 values", "[Game][G52][Sandstorm]") {
    REQUIRE(std::string(NF::sandstormSeverityName(NF::SandstormSeverity::Dust))         == "Dust");
    REQUIRE(std::string(NF::sandstormSeverityName(NF::SandstormSeverity::Moderate))     == "Moderate");
    REQUIRE(std::string(NF::sandstormSeverityName(NF::SandstormSeverity::Strong))       == "Strong");
    REQUIRE(std::string(NF::sandstormSeverityName(NF::SandstormSeverity::Severe))       == "Severe");
    REQUIRE(std::string(NF::sandstormSeverityName(NF::SandstormSeverity::Violent))      == "Violent");
    REQUIRE(std::string(NF::sandstormSeverityName(NF::SandstormSeverity::Catastrophic)) == "Catastrophic");
}

TEST_CASE("SandstormEvent isSevere and isCatastrophic thresholds", "[Game][G52][Sandstorm]") {
    NF::SandstormEvent ev; ev.id = "ss1";

    ev.severity = NF::SandstormSeverity::Dust;
    REQUIRE_FALSE(ev.isSevere());
    REQUIRE_FALSE(ev.isCatastrophic());

    ev.severity = NF::SandstormSeverity::Severe;
    REQUIRE(ev.isSevere());
    REQUIRE_FALSE(ev.isCatastrophic());

    ev.severity = NF::SandstormSeverity::Catastrophic;
    REQUIRE(ev.isSevere());
    REQUIRE(ev.isCatastrophic());
}

TEST_CASE("SandstormEvent particleDensity scales with severity and windSpeed", "[Game][G52][Sandstorm]") {
    NF::SandstormEvent ev; ev.id = "ss2";
    ev.severity  = NF::SandstormSeverity::Moderate; // uint8_t = 1, +1 = 2
    ev.windSpeed = 10.0f;
    REQUIRE(ev.particleDensity() == Catch::Approx(20.0f));

    ev.severity  = NF::SandstormSeverity::Dust; // uint8_t = 0, +1 = 1
    ev.windSpeed = 5.0f;
    REQUIRE(ev.particleDensity() == Catch::Approx(5.0f));
}

TEST_CASE("SandstormEvent activate and deactivate toggle active", "[Game][G52][Sandstorm]") {
    NF::SandstormEvent ev; ev.id = "ss3";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("SandstormRegion addEvent and duplicate rejection", "[Game][G52][Sandstorm]") {
    NF::SandstormRegion region("sahara");
    NF::SandstormEvent a; a.id = "a";
    NF::SandstormEvent b; b.id = "b";
    NF::SandstormEvent dup; dup.id = "a";

    REQUIRE(region.addEvent(a));
    REQUIRE(region.addEvent(b));
    REQUIRE_FALSE(region.addEvent(dup));
    REQUIRE(region.eventCount() == 2);
}

TEST_CASE("SandstormRegion severeCount and catastrophicCount", "[Game][G52][Sandstorm]") {
    NF::SandstormRegion region("gobi");

    NF::SandstormEvent cat; cat.id = "c1"; cat.severity = NF::SandstormSeverity::Catastrophic; cat.activate();
    NF::SandstormEvent sev; sev.id = "s1"; sev.severity = NF::SandstormSeverity::Severe;       sev.activate();
    NF::SandstormEvent dus; dus.id = "d1"; dus.severity = NF::SandstormSeverity::Dust;          dus.activate();

    region.addEvent(cat);
    region.addEvent(sev);
    region.addEvent(dus);

    REQUIRE(region.activeCount()        == 3);
    REQUIRE(region.severeCount()        == 2); // Catastrophic + Severe
    REQUIRE(region.catastrophicCount()  == 1);
}

TEST_CASE("SandstormSystem createRegion and tick propagation", "[Game][G52][Sandstorm]") {
    NF::SandstormSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()              == 3);
    REQUIRE(sys.byName("r1")->tickCount() == 3);
    REQUIRE(sys.byName("r2")->tickCount() == 3);
}

TEST_CASE("SandstormSystem activeEventCount, severeEventCount, catastrophicEventCount", "[Game][G52][Sandstorm]") {
    NF::SandstormSystem sys;
    sys.createRegion("ra");
    sys.createRegion("rb");

    NF::SandstormEvent c1; c1.id = "c1"; c1.severity = NF::SandstormSeverity::Catastrophic; c1.activate();
    NF::SandstormEvent s1; s1.id = "s1"; s1.severity = NF::SandstormSeverity::Severe;        s1.activate();
    NF::SandstormEvent d1; d1.id = "d1"; d1.severity = NF::SandstormSeverity::Dust;

    sys.byName("ra")->addEvent(c1);
    sys.byName("ra")->addEvent(d1);
    sys.byName("rb")->addEvent(s1);

    REQUIRE(sys.activeEventCount()        == 2);
    REQUIRE(sys.severeEventCount()        == 2); // c1 and s1
    REQUIRE(sys.catastrophicEventCount()  == 1); // only c1
}

// ── G53 — Cyclone System ──────────────────────────────────────────

TEST_CASE("CycloneCategory names cover all 6 values", "[Game][G53][Cyclone]") {
    REQUIRE(std::string(NF::cycloneCategoryName(NF::CycloneCategory::TropicalDepression)) == "TropicalDepression");
    REQUIRE(std::string(NF::cycloneCategoryName(NF::CycloneCategory::TropicalStorm))      == "TropicalStorm");
    REQUIRE(std::string(NF::cycloneCategoryName(NF::CycloneCategory::Cat1))               == "Cat1");
    REQUIRE(std::string(NF::cycloneCategoryName(NF::CycloneCategory::Cat2))               == "Cat2");
    REQUIRE(std::string(NF::cycloneCategoryName(NF::CycloneCategory::Cat3))               == "Cat3");
    REQUIRE(std::string(NF::cycloneCategoryName(NF::CycloneCategory::Cat4))               == "Cat4");
}

TEST_CASE("CycloneStage names cover all 6 values", "[Game][G53][Cyclone]") {
    REQUIRE(std::string(NF::cycloneStageName(NF::CycloneStage::Forming))      == "Forming");
    REQUIRE(std::string(NF::cycloneStageName(NF::CycloneStage::Intensifying)) == "Intensifying");
    REQUIRE(std::string(NF::cycloneStageName(NF::CycloneStage::Peak))         == "Peak");
    REQUIRE(std::string(NF::cycloneStageName(NF::CycloneStage::Weakening))    == "Weakening");
    REQUIRE(std::string(NF::cycloneStageName(NF::CycloneStage::Dissipating))  == "Dissipating");
    REQUIRE(std::string(NF::cycloneStageName(NF::CycloneStage::Remnant))      == "Remnant");
}

TEST_CASE("CycloneEvent isMajor threshold is Cat3+", "[Game][G53][Cyclone]") {
    NF::CycloneEvent ev; ev.id = "c1";

    ev.category = NF::CycloneCategory::Cat2;
    REQUIRE_FALSE(ev.isMajor());

    ev.category = NF::CycloneCategory::Cat3;
    REQUIRE(ev.isMajor());

    ev.category = NF::CycloneCategory::Cat4;
    REQUIRE(ev.isMajor());
}

TEST_CASE("CycloneEvent isAtPeak only true at Peak stage", "[Game][G53][Cyclone]") {
    NF::CycloneEvent ev; ev.id = "c2";
    ev.stage = NF::CycloneStage::Intensifying;
    REQUIRE_FALSE(ev.isAtPeak());

    ev.stage = NF::CycloneStage::Peak;
    REQUIRE(ev.isAtPeak());
}

TEST_CASE("CycloneEvent stormSurge scales with category and windSpeed", "[Game][G53][Cyclone]") {
    NF::CycloneEvent ev; ev.id = "c3";
    // Cat1 (uint8=2): (2+1) * 0.5 * windSpeed / 100 = 1.5 * windSpeed / 100
    ev.category  = NF::CycloneCategory::Cat1;
    ev.windSpeed = 100.0f;
    REQUIRE(ev.stormSurge() == Catch::Approx(1.5f));
}

TEST_CASE("CycloneEvent activate and deactivate toggle active", "[Game][G53][Cyclone]") {
    NF::CycloneEvent ev; ev.id = "c4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("CycloneRegion majorCount and peakCount", "[Game][G53][Cyclone]") {
    NF::CycloneRegion region("pacific");

    NF::CycloneEvent a; a.id = "a"; a.category = NF::CycloneCategory::Cat4; a.stage = NF::CycloneStage::Peak; a.activate();
    NF::CycloneEvent b; b.id = "b"; b.category = NF::CycloneCategory::Cat3; b.stage = NF::CycloneStage::Intensifying; b.activate();
    NF::CycloneEvent c; c.id = "c"; c.category = NF::CycloneCategory::Cat1; c.stage = NF::CycloneStage::Peak; c.activate();

    region.addEvent(a);
    region.addEvent(b);
    region.addEvent(c);

    REQUIRE(region.activeCount() == 3);
    REQUIRE(region.majorCount()  == 2); // a and b
    REQUIRE(region.peakCount()   == 2); // a and c
}

TEST_CASE("CycloneSystem createRegion and tick propagation", "[Game][G53][Cyclone]") {
    NF::CycloneSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()              == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
    REQUIRE(sys.byName("r2")->tickCount() == 2);
}

TEST_CASE("CycloneSystem activeEventCount, majorEventCount, peakEventCount", "[Game][G53][Cyclone]") {
    NF::CycloneSystem sys;
    sys.createRegion("ra");
    sys.createRegion("rb");

    NF::CycloneEvent x; x.id = "x"; x.category = NF::CycloneCategory::Cat4; x.stage = NF::CycloneStage::Peak; x.activate();
    NF::CycloneEvent y; y.id = "y"; y.category = NF::CycloneCategory::Cat2; y.stage = NF::CycloneStage::Forming; y.activate();
    NF::CycloneEvent z; z.id = "z"; z.category = NF::CycloneCategory::Cat3; z.stage = NF::CycloneStage::Weakening;

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount() == 2);
    REQUIRE(sys.majorEventCount()  == 1); // only x (y is Cat2, z inactive)
    REQUIRE(sys.peakEventCount()   == 1); // only x
}

// ── G54 — Tornado System ──────────────────────────────────────────

TEST_CASE("TornadoScale names cover all 6 values", "[Game][G54][Tornado]") {
    REQUIRE(std::string(NF::tornadoScaleName(NF::TornadoScale::EF0)) == "EF0");
    REQUIRE(std::string(NF::tornadoScaleName(NF::TornadoScale::EF1)) == "EF1");
    REQUIRE(std::string(NF::tornadoScaleName(NF::TornadoScale::EF2)) == "EF2");
    REQUIRE(std::string(NF::tornadoScaleName(NF::TornadoScale::EF3)) == "EF3");
    REQUIRE(std::string(NF::tornadoScaleName(NF::TornadoScale::EF4)) == "EF4");
    REQUIRE(std::string(NF::tornadoScaleName(NF::TornadoScale::EF5)) == "EF5");
}

TEST_CASE("TornadoStage names cover all 6 values", "[Game][G54][Tornado]") {
    REQUIRE(std::string(NF::tornadoStageName(NF::TornadoStage::Organizing))  == "Organizing");
    REQUIRE(std::string(NF::tornadoStageName(NF::TornadoStage::Mature))      == "Mature");
    REQUIRE(std::string(NF::tornadoStageName(NF::TornadoStage::Shrinking))   == "Shrinking");
    REQUIRE(std::string(NF::tornadoStageName(NF::TornadoStage::Rope))        == "Rope");
    REQUIRE(std::string(NF::tornadoStageName(NF::TornadoStage::Dissipating)) == "Dissipating");
    REQUIRE(std::string(NF::tornadoStageName(NF::TornadoStage::Remnant))     == "Remnant");
}

TEST_CASE("TornadoEvent isViolent threshold is EF4+", "[Game][G54][Tornado]") {
    NF::TornadoEvent ev; ev.id = "t1";

    ev.scale = NF::TornadoScale::EF3;
    REQUIRE_FALSE(ev.isViolent());

    ev.scale = NF::TornadoScale::EF4;
    REQUIRE(ev.isViolent());

    ev.scale = NF::TornadoScale::EF5;
    REQUIRE(ev.isViolent());
}

TEST_CASE("TornadoEvent isAtPeak only true at Mature stage", "[Game][G54][Tornado]") {
    NF::TornadoEvent ev; ev.id = "t2";
    ev.stage = NF::TornadoStage::Shrinking;
    REQUIRE_FALSE(ev.isAtPeak());

    ev.stage = NF::TornadoStage::Mature;
    REQUIRE(ev.isAtPeak());
}

TEST_CASE("TornadoEvent destructionIndex scales with scale and windSpeed", "[Game][G54][Tornado]") {
    NF::TornadoEvent ev; ev.id = "t3";
    // EF2 (uint8=2): (2+1) * windSpeed / 100 = 3 * 100/100 = 3.0
    ev.scale     = NF::TornadoScale::EF2;
    ev.windSpeed = 100.0f;
    REQUIRE(ev.destructionIndex() == Catch::Approx(3.0f));
}

TEST_CASE("TornadoEvent activate and deactivate", "[Game][G54][Tornado]") {
    NF::TornadoEvent ev; ev.id = "t4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("TornadoRegion violentCount and peakCount", "[Game][G54][Tornado]") {
    NF::TornadoRegion region("plains");

    NF::TornadoEvent a; a.id = "a"; a.scale = NF::TornadoScale::EF5; a.stage = NF::TornadoStage::Mature; a.activate();
    NF::TornadoEvent b; b.id = "b"; b.scale = NF::TornadoScale::EF4; b.stage = NF::TornadoStage::Shrinking; b.activate();
    NF::TornadoEvent c; c.id = "c"; c.scale = NF::TornadoScale::EF1; c.stage = NF::TornadoStage::Mature; c.activate();

    region.addEvent(a);
    region.addEvent(b);
    region.addEvent(c);

    REQUIRE(region.activeCount()  == 3);
    REQUIRE(region.violentCount() == 2); // a and b (EF4+)
    REQUIRE(region.peakCount()    == 2); // a and c (Mature)
}

TEST_CASE("TornadoSystem createRegion and tick propagation", "[Game][G54][Tornado]") {
    NF::TornadoSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 3);
    REQUIRE(sys.byName("r1")->tickCount() == 3);
    REQUIRE(sys.byName("r2")->tickCount() == 3);
}

TEST_CASE("TornadoSystem activeEventCount, violentEventCount, peakEventCount", "[Game][G54][Tornado]") {
    NF::TornadoSystem sys;
    sys.createRegion("ra");
    sys.createRegion("rb");

    NF::TornadoEvent x; x.id = "x"; x.scale = NF::TornadoScale::EF5; x.stage = NF::TornadoStage::Mature; x.activate();
    NF::TornadoEvent y; y.id = "y"; y.scale = NF::TornadoScale::EF2; y.stage = NF::TornadoStage::Rope; y.activate();
    NF::TornadoEvent z; z.id = "z"; z.scale = NF::TornadoScale::EF4; z.stage = NF::TornadoStage::Mature; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()  == 2);
    REQUIRE(sys.violentEventCount() == 1); // only x (y is EF2, z inactive)
    REQUIRE(sys.peakEventCount()    == 1); // only x (y is Rope, z inactive)
}

// ── G55 — Dust Storm System ───────────────────────────────────────

TEST_CASE("DustDensity names cover all 5 values", "[Game][G55][DustStorm]") {
    REQUIRE(std::string(NF::dustDensityName(NF::DustDensity::Trace))    == "Trace");
    REQUIRE(std::string(NF::dustDensityName(NF::DustDensity::Light))    == "Light");
    REQUIRE(std::string(NF::dustDensityName(NF::DustDensity::Moderate)) == "Moderate");
    REQUIRE(std::string(NF::dustDensityName(NF::DustDensity::Heavy))    == "Heavy");
    REQUIRE(std::string(NF::dustDensityName(NF::DustDensity::Extreme))  == "Extreme");
}

TEST_CASE("DustStormPhase names cover all 5 values", "[Game][G55][DustStorm]") {
    REQUIRE(std::string(NF::dustStormPhaseName(NF::DustStormPhase::Forming))    == "Forming");
    REQUIRE(std::string(NF::dustStormPhaseName(NF::DustStormPhase::Advancing))  == "Advancing");
    REQUIRE(std::string(NF::dustStormPhaseName(NF::DustStormPhase::Peak))       == "Peak");
    REQUIRE(std::string(NF::dustStormPhaseName(NF::DustStormPhase::Retreating)) == "Retreating");
    REQUIRE(std::string(NF::dustStormPhaseName(NF::DustStormPhase::Clearing))   == "Clearing");
}

TEST_CASE("DustStormEvent isHazardous threshold is Heavy+", "[Game][G55][DustStorm]") {
    NF::DustStormEvent ev; ev.id = "d1";
    ev.density = NF::DustDensity::Moderate;
    REQUIRE_FALSE(ev.isHazardous());

    ev.density = NF::DustDensity::Heavy;
    REQUIRE(ev.isHazardous());

    ev.density = NF::DustDensity::Extreme;
    REQUIRE(ev.isHazardous());
}

TEST_CASE("DustStormEvent isAtPeak and isLowVisibility", "[Game][G55][DustStorm]") {
    NF::DustStormEvent ev; ev.id = "d2";
    ev.phase = NF::DustStormPhase::Advancing;
    ev.visibility = 500.0f;
    REQUIRE_FALSE(ev.isAtPeak());
    REQUIRE_FALSE(ev.isLowVisibility());

    ev.phase = NF::DustStormPhase::Peak;
    ev.visibility = 100.0f;
    REQUIRE(ev.isAtPeak());
    REQUIRE(ev.isLowVisibility());
}

TEST_CASE("DustStormEvent hazardScore calculation", "[Game][G55][DustStorm]") {
    NF::DustStormEvent ev; ev.id = "d3";
    // Moderate (uint8=2): (2+1) * 50 / 50 = 3.0
    ev.density   = NF::DustDensity::Moderate;
    ev.windSpeed = 50.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(3.0f));
}

TEST_CASE("DustStormEvent activate and deactivate", "[Game][G55][DustStorm]") {
    NF::DustStormEvent ev; ev.id = "d4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("DustStormRegion hazardousCount and peakCount", "[Game][G55][DustStorm]") {
    NF::DustStormRegion region("sahara");

    NF::DustStormEvent a; a.id = "a"; a.density = NF::DustDensity::Extreme; a.phase = NF::DustStormPhase::Peak;     a.activate();
    NF::DustStormEvent b; b.id = "b"; b.density = NF::DustDensity::Heavy;   b.phase = NF::DustStormPhase::Advancing;a.activate(); b.activate();
    NF::DustStormEvent c; c.id = "c"; c.density = NF::DustDensity::Light;   c.phase = NF::DustStormPhase::Peak;     c.activate();

    region.addEvent(a);
    region.addEvent(b);
    region.addEvent(c);

    REQUIRE(region.hazardousCount() == 2); // a and b (Heavy+)
    REQUIRE(region.peakCount()      == 2); // a and c (Peak phase)
}

TEST_CASE("DustStormSystem createRegion and tick propagation", "[Game][G55][DustStorm]") {
    NF::DustStormSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
    REQUIRE(sys.byName("r2")->tickCount() == 2);
}

TEST_CASE("DustStormSystem aggregate event counts", "[Game][G55][DustStorm]") {
    NF::DustStormSystem sys;
    sys.createRegion("ra");
    sys.createRegion("rb");

    NF::DustStormEvent x; x.id = "x"; x.density = NF::DustDensity::Extreme; x.phase = NF::DustStormPhase::Peak; x.activate();
    NF::DustStormEvent y; y.id = "y"; y.density = NF::DustDensity::Light;   y.phase = NF::DustStormPhase::Advancing; y.activate();
    NF::DustStormEvent z; z.id = "z"; z.density = NF::DustDensity::Heavy;   z.phase = NF::DustStormPhase::Peak; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()    == 2);
    REQUIRE(sys.hazardousEventCount() == 1); // only x (y=Light, z inactive)
    REQUIRE(sys.peakEventCount()      == 1); // only x (y=Advancing, z inactive)
}

// ── G56 — Hail Storm System ───────────────────────────────────────

TEST_CASE("HailSize names cover all 5 values", "[Game][G56][HailStorm]") {
    REQUIRE(std::string(NF::hailSizeName(NF::HailSize::Pea))        == "Pea");
    REQUIRE(std::string(NF::hailSizeName(NF::HailSize::Marble))     == "Marble");
    REQUIRE(std::string(NF::hailSizeName(NF::HailSize::Golf))       == "Golf");
    REQUIRE(std::string(NF::hailSizeName(NF::HailSize::Baseball))   == "Baseball");
    REQUIRE(std::string(NF::hailSizeName(NF::HailSize::Grapefruit)) == "Grapefruit");
}

TEST_CASE("HailStormPhase names cover all 5 values", "[Game][G56][HailStorm]") {
    REQUIRE(std::string(NF::hailStormPhaseName(NF::HailStormPhase::Developing))   == "Developing");
    REQUIRE(std::string(NF::hailStormPhaseName(NF::HailStormPhase::Intensifying)) == "Intensifying");
    REQUIRE(std::string(NF::hailStormPhaseName(NF::HailStormPhase::Peak))         == "Peak");
    REQUIRE(std::string(NF::hailStormPhaseName(NF::HailStormPhase::Weakening))    == "Weakening");
    REQUIRE(std::string(NF::hailStormPhaseName(NF::HailStormPhase::Dissipating))  == "Dissipating");
}

TEST_CASE("HailStormEvent isSevere threshold is Golf+", "[Game][G56][HailStorm]") {
    NF::HailStormEvent ev; ev.id = "h1";
    ev.hailSize = NF::HailSize::Marble;
    REQUIRE_FALSE(ev.isSevere());

    ev.hailSize = NF::HailSize::Golf;
    REQUIRE(ev.isSevere());

    ev.hailSize = NF::HailSize::Grapefruit;
    REQUIRE(ev.isSevere());
}

TEST_CASE("HailStormEvent isAtPeak and isWidespread", "[Game][G56][HailStorm]") {
    NF::HailStormEvent ev; ev.id = "h2";
    ev.phase    = NF::HailStormPhase::Intensifying;
    ev.coverage = 100.0f;
    REQUIRE_FALSE(ev.isAtPeak());
    REQUIRE_FALSE(ev.isWidespread());

    ev.phase    = NF::HailStormPhase::Peak;
    ev.coverage = 600.0f;
    REQUIRE(ev.isAtPeak());
    REQUIRE(ev.isWidespread());
}

TEST_CASE("HailStormEvent damageScore calculation", "[Game][G56][HailStorm]") {
    NF::HailStormEvent ev; ev.id = "h3";
    // Golf (uint8=2): (2+1) * 100 / 100 = 3.0
    ev.hailSize  = NF::HailSize::Golf;
    ev.intensity = 100.0f;
    REQUIRE(ev.damageScore() == Catch::Approx(3.0f));
}

TEST_CASE("HailStormEvent activate and deactivate", "[Game][G56][HailStorm]") {
    NF::HailStormEvent ev; ev.id = "h4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("HailStormRegion severeCount and peakCount", "[Game][G56][HailStorm]") {
    NF::HailStormRegion region("midwest");

    NF::HailStormEvent a; a.id = "a"; a.hailSize = NF::HailSize::Grapefruit; a.phase = NF::HailStormPhase::Peak;     a.activate();
    NF::HailStormEvent b; b.id = "b"; b.hailSize = NF::HailSize::Baseball;   b.phase = NF::HailStormPhase::Weakening; b.activate();
    NF::HailStormEvent c; c.id = "c"; c.hailSize = NF::HailSize::Pea;        c.phase = NF::HailStormPhase::Peak;     c.activate();

    region.addEvent(a);
    region.addEvent(b);
    region.addEvent(c);

    REQUIRE(region.severeCount() == 2); // a (Grapefruit) and b (Baseball)
    REQUIRE(region.peakCount()   == 2); // a and c (Peak phase)
}

TEST_CASE("HailStormSystem createRegion and tick propagation", "[Game][G56][HailStorm]") {
    NF::HailStormSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 3);
    REQUIRE(sys.byName("r1")->tickCount() == 3);
}

TEST_CASE("HailStormSystem aggregate event counts", "[Game][G56][HailStorm]") {
    NF::HailStormSystem sys;
    sys.createRegion("ra");
    sys.createRegion("rb");

    NF::HailStormEvent x; x.id = "x"; x.hailSize = NF::HailSize::Grapefruit; x.phase = NF::HailStormPhase::Peak; x.coverage = 800.0f; x.activate();
    NF::HailStormEvent y; y.id = "y"; y.hailSize = NF::HailSize::Pea;         y.phase = NF::HailStormPhase::Developing; y.activate();
    NF::HailStormEvent z; z.id = "z"; z.hailSize = NF::HailSize::Baseball;    z.phase = NF::HailStormPhase::Peak; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()     == 2);
    REQUIRE(sys.severeEventCount()     == 1); // only x (y=Pea, z inactive)
    REQUIRE(sys.peakEventCount()       == 1); // only x (y=Developing, z inactive)
    REQUIRE(sys.widespreadEventCount() == 1); // only x (coverage >= 500)
}

// ── G57 — Ice Storm System ────────────────────────────────────────

TEST_CASE("IceThickness names cover all 5 values", "[Game][G57][IceStorm]") {
    REQUIRE(std::string(NF::iceThicknessName(NF::IceThickness::Glaze))    == "Glaze");
    REQUIRE(std::string(NF::iceThicknessName(NF::IceThickness::Light))    == "Light");
    REQUIRE(std::string(NF::iceThicknessName(NF::IceThickness::Moderate)) == "Moderate");
    REQUIRE(std::string(NF::iceThicknessName(NF::IceThickness::Heavy))    == "Heavy");
    REQUIRE(std::string(NF::iceThicknessName(NF::IceThickness::Extreme))  == "Extreme");
}

TEST_CASE("IceStormPhase names cover all 5 values", "[Game][G57][IceStorm]") {
    REQUIRE(std::string(NF::iceStormPhaseName(NF::IceStormPhase::Forming))      == "Forming");
    REQUIRE(std::string(NF::iceStormPhaseName(NF::IceStormPhase::Spreading))    == "Spreading");
    REQUIRE(std::string(NF::iceStormPhaseName(NF::IceStormPhase::Intensifying)) == "Intensifying");
    REQUIRE(std::string(NF::iceStormPhaseName(NF::IceStormPhase::Freezing))     == "Freezing");
    REQUIRE(std::string(NF::iceStormPhaseName(NF::IceStormPhase::Dissipating))  == "Dissipating");
}

TEST_CASE("IceStormEvent isSevere threshold is Heavy+", "[Game][G57][IceStorm]") {
    NF::IceStormEvent ev; ev.id = "i1";
    ev.thickness = NF::IceThickness::Moderate;
    REQUIRE_FALSE(ev.isSevere());

    ev.thickness = NF::IceThickness::Heavy;
    REQUIRE(ev.isSevere());

    ev.thickness = NF::IceThickness::Extreme;
    REQUIRE(ev.isSevere());
}

TEST_CASE("IceStormEvent isFreezing and isWidespread", "[Game][G57][IceStorm]") {
    NF::IceStormEvent ev; ev.id = "i2";
    ev.phase    = NF::IceStormPhase::Spreading;
    ev.coverage = 100.0f;
    REQUIRE_FALSE(ev.isFreezing());
    REQUIRE_FALSE(ev.isWidespread());

    ev.phase    = NF::IceStormPhase::Freezing;
    ev.coverage = 500.0f;
    REQUIRE(ev.isFreezing());
    REQUIRE(ev.isWidespread());
}

TEST_CASE("IceStormEvent hazardScore calculation", "[Game][G57][IceStorm]") {
    NF::IceStormEvent ev; ev.id = "i3";
    // Heavy (uint8=3): (3+1) * 100 / 100 = 4.0
    ev.thickness = NF::IceThickness::Heavy;
    ev.intensity = 100.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(4.0f));
}

TEST_CASE("IceStormEvent activate and deactivate", "[Game][G57][IceStorm]") {
    NF::IceStormEvent ev; ev.id = "i4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("IceStormRegion freezingCount and severeCount", "[Game][G57][IceStorm]") {
    NF::IceStormRegion region("north");

    NF::IceStormEvent a; a.id = "a"; a.thickness = NF::IceThickness::Extreme; a.phase = NF::IceStormPhase::Freezing; a.activate();
    NF::IceStormEvent b; b.id = "b"; b.thickness = NF::IceThickness::Heavy;   b.phase = NF::IceStormPhase::Spreading; b.activate();
    NF::IceStormEvent c; c.id = "c"; c.thickness = NF::IceThickness::Light;   c.phase = NF::IceStormPhase::Freezing; c.activate();

    region.addEvent(a); region.addEvent(b); region.addEvent(c);

    REQUIRE(region.severeCount()   == 2); // a (Extreme) and b (Heavy)
    REQUIRE(region.freezingCount() == 2); // a and c (Freezing phase)
}

TEST_CASE("IceStormSystem createRegion and tick propagation", "[Game][G57][IceStorm]") {
    NF::IceStormSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
}

TEST_CASE("IceStormSystem aggregate event counts", "[Game][G57][IceStorm]") {
    NF::IceStormSystem sys;
    sys.createRegion("ra"); sys.createRegion("rb");

    NF::IceStormEvent x; x.id = "x"; x.thickness = NF::IceThickness::Extreme; x.phase = NF::IceStormPhase::Freezing; x.coverage = 600.0f; x.intensity = 80.0f; x.activate();
    NF::IceStormEvent y; y.id = "y"; y.thickness = NF::IceThickness::Glaze;   y.phase = NF::IceStormPhase::Forming;  y.activate();
    NF::IceStormEvent z; z.id = "z"; z.thickness = NF::IceThickness::Heavy;   z.phase = NF::IceStormPhase::Freezing; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()     == 2);
    REQUIRE(sys.severeEventCount()     == 1); // only x (y=Glaze, z inactive)
    REQUIRE(sys.freezingEventCount()   == 1); // only x (y=Forming, z inactive)
    REQUIRE(sys.widespreadEventCount() == 1); // only x (coverage >= 400)
}

// ── G58 — Monsoon System ────────────────────────────────────────

TEST_CASE("MonsoonIntensity names cover all 5 values", "[Game][G58][Monsoon]") {
    REQUIRE(std::string(NF::monsoonIntensityName(NF::MonsoonIntensity::Light))    == "Light");
    REQUIRE(std::string(NF::monsoonIntensityName(NF::MonsoonIntensity::Moderate)) == "Moderate");
    REQUIRE(std::string(NF::monsoonIntensityName(NF::MonsoonIntensity::Heavy))    == "Heavy");
    REQUIRE(std::string(NF::monsoonIntensityName(NF::MonsoonIntensity::Severe))   == "Severe");
    REQUIRE(std::string(NF::monsoonIntensityName(NF::MonsoonIntensity::Extreme))  == "Extreme");
}

TEST_CASE("MonsoonPhase names cover all 5 values", "[Game][G58][Monsoon]") {
    REQUIRE(std::string(NF::monsoonPhaseName(NF::MonsoonPhase::Onset))      == "Onset");
    REQUIRE(std::string(NF::monsoonPhaseName(NF::MonsoonPhase::Active))     == "Active");
    REQUIRE(std::string(NF::monsoonPhaseName(NF::MonsoonPhase::Peak))       == "Peak");
    REQUIRE(std::string(NF::monsoonPhaseName(NF::MonsoonPhase::Retreating)) == "Retreating");
    REQUIRE(std::string(NF::monsoonPhaseName(NF::MonsoonPhase::Withdrawal)) == "Withdrawal");
}

TEST_CASE("MonsoonEvent isSevere threshold is Severe+", "[Game][G58][Monsoon]") {
    NF::MonsoonEvent ev; ev.id = "m1";
    ev.intensity = NF::MonsoonIntensity::Heavy;
    REQUIRE_FALSE(ev.isSevere());

    ev.intensity = NF::MonsoonIntensity::Severe;
    REQUIRE(ev.isSevere());

    ev.intensity = NF::MonsoonIntensity::Extreme;
    REQUIRE(ev.isSevere());
}

TEST_CASE("MonsoonEvent isPeak and isFlooding", "[Game][G58][Monsoon]") {
    NF::MonsoonEvent ev; ev.id = "m2";
    ev.phase      = NF::MonsoonPhase::Active;
    ev.rainfallMm = 100.0f;
    REQUIRE_FALSE(ev.isPeak());
    REQUIRE_FALSE(ev.isFlooding());

    ev.phase      = NF::MonsoonPhase::Peak;
    ev.rainfallMm = 250.0f;
    REQUIRE(ev.isPeak());
    REQUIRE(ev.isFlooding());
}

TEST_CASE("MonsoonEvent hazardScore calculation", "[Game][G58][Monsoon]") {
    NF::MonsoonEvent ev; ev.id = "m3";
    // Severe (uint8=3): (3+1) * 200 / 100 = 8.0
    ev.intensity  = NF::MonsoonIntensity::Severe;
    ev.rainfallMm = 200.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(8.0f));
}

TEST_CASE("MonsoonEvent activate and deactivate", "[Game][G58][Monsoon]") {
    NF::MonsoonEvent ev; ev.id = "m4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("MonsoonRegion severeCount and floodingCount", "[Game][G58][Monsoon]") {
    NF::MonsoonRegion region("south");

    NF::MonsoonEvent a; a.id = "a"; a.intensity = NF::MonsoonIntensity::Extreme; a.phase = NF::MonsoonPhase::Peak; a.rainfallMm = 300.0f; a.activate();
    NF::MonsoonEvent b; b.id = "b"; b.intensity = NF::MonsoonIntensity::Heavy;   b.phase = NF::MonsoonPhase::Active; b.rainfallMm = 50.0f; b.activate();
    NF::MonsoonEvent c; c.id = "c"; c.intensity = NF::MonsoonIntensity::Severe;  c.phase = NF::MonsoonPhase::Peak;  c.rainfallMm = 250.0f; c.activate();

    region.addEvent(a); region.addEvent(b); region.addEvent(c);

    REQUIRE(region.severeCount()   == 2); // a (Extreme) and c (Severe)
    REQUIRE(region.peakCount()     == 2); // a and c
    REQUIRE(region.floodingCount() == 2); // a (300mm) and c (250mm)
}

TEST_CASE("MonsoonSystem createRegion and tick propagation", "[Game][G58][Monsoon]") {
    NF::MonsoonSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
}

TEST_CASE("MonsoonSystem aggregate event counts", "[Game][G58][Monsoon]") {
    NF::MonsoonSystem sys;
    sys.createRegion("ra"); sys.createRegion("rb");

    NF::MonsoonEvent x; x.id = "x"; x.intensity = NF::MonsoonIntensity::Extreme; x.phase = NF::MonsoonPhase::Peak;   x.rainfallMm = 400.0f; x.activate();
    NF::MonsoonEvent y; y.id = "y"; y.intensity = NF::MonsoonIntensity::Light;   y.phase = NF::MonsoonPhase::Onset;  y.rainfallMm = 10.0f;  y.activate();
    NF::MonsoonEvent z; z.id = "z"; z.intensity = NF::MonsoonIntensity::Severe;  z.phase = NF::MonsoonPhase::Peak;   z.rainfallMm = 250.0f; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()   == 2);
    REQUIRE(sys.severeEventCount()   == 1); // only x (y=Light, z inactive)
    REQUIRE(sys.peakEventCount()     == 1); // only x (y=Onset, z inactive)
    REQUIRE(sys.floodingEventCount() == 1); // only x (y=10mm, z inactive)
}

// ── G59 — Lightning Storm System ────────────────────────────────

TEST_CASE("LightningIntensity names cover all 5 values", "[Game][G59][Lightning]") {
    REQUIRE(std::string(NF::lightningIntensityName(NF::LightningIntensity::Scattered)) == "Scattered");
    REQUIRE(std::string(NF::lightningIntensityName(NF::LightningIntensity::Moderate))  == "Moderate");
    REQUIRE(std::string(NF::lightningIntensityName(NF::LightningIntensity::Frequent))  == "Frequent");
    REQUIRE(std::string(NF::lightningIntensityName(NF::LightningIntensity::Severe))    == "Severe");
    REQUIRE(std::string(NF::lightningIntensityName(NF::LightningIntensity::Extreme))   == "Extreme");
}

TEST_CASE("LightningStormPhase names cover all 5 values", "[Game][G59][Lightning]") {
    REQUIRE(std::string(NF::lightningStormPhaseName(NF::LightningStormPhase::Building))    == "Building");
    REQUIRE(std::string(NF::lightningStormPhaseName(NF::LightningStormPhase::Active))      == "Active");
    REQUIRE(std::string(NF::lightningStormPhaseName(NF::LightningStormPhase::Peak))        == "Peak");
    REQUIRE(std::string(NF::lightningStormPhaseName(NF::LightningStormPhase::Weakening))   == "Weakening");
    REQUIRE(std::string(NF::lightningStormPhaseName(NF::LightningStormPhase::Dissipating)) == "Dissipating");
}

TEST_CASE("LightningEvent isSevere threshold is Severe+", "[Game][G59][Lightning]") {
    NF::LightningEvent ev; ev.id = "l1";
    ev.intensity = NF::LightningIntensity::Frequent;
    REQUIRE_FALSE(ev.isSevere());

    ev.intensity = NF::LightningIntensity::Severe;
    REQUIRE(ev.isSevere());

    ev.intensity = NF::LightningIntensity::Extreme;
    REQUIRE(ev.isSevere());
}

TEST_CASE("LightningEvent isPeak and isWidespread", "[Game][G59][Lightning]") {
    NF::LightningEvent ev; ev.id = "l2";
    ev.phase    = NF::LightningStormPhase::Active;
    ev.coverage = 100.0f;
    REQUIRE_FALSE(ev.isPeak());
    REQUIRE_FALSE(ev.isWidespread());

    ev.phase    = NF::LightningStormPhase::Peak;
    ev.coverage = 400.0f;
    REQUIRE(ev.isPeak());
    REQUIRE(ev.isWidespread());
}

TEST_CASE("LightningEvent hazardScore calculation", "[Game][G59][Lightning]") {
    NF::LightningEvent ev; ev.id = "l3";
    // Severe (uint8=3): (3+1) * 50 / 10 = 20.0
    ev.intensity  = NF::LightningIntensity::Severe;
    ev.strikeRate = 50.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(20.0f));
}

TEST_CASE("LightningEvent activate and deactivate", "[Game][G59][Lightning]") {
    NF::LightningEvent ev; ev.id = "l4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("LightningRegion severeCount and widespreadCount", "[Game][G59][Lightning]") {
    NF::LightningRegion region("plains");

    NF::LightningEvent a; a.id = "a"; a.intensity = NF::LightningIntensity::Extreme; a.phase = NF::LightningStormPhase::Peak; a.coverage = 500.0f; a.activate();
    NF::LightningEvent b; b.id = "b"; b.intensity = NF::LightningIntensity::Moderate; b.phase = NF::LightningStormPhase::Active; b.coverage = 50.0f; b.activate();
    NF::LightningEvent c; c.id = "c"; c.intensity = NF::LightningIntensity::Severe; c.coverage = 350.0f; // inactive

    region.addEvent(a); region.addEvent(b); region.addEvent(c);

    REQUIRE(region.severeCount()     == 1); // only a (c is inactive)
    REQUIRE(region.peakCount()       == 1); // only a
    REQUIRE(region.widespreadCount() == 1); // only a (coverage >= 300)
}

TEST_CASE("LightningSystem createRegion and tick propagation", "[Game][G59][Lightning]") {
    NF::LightningSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
}

TEST_CASE("LightningSystem aggregate event counts", "[Game][G59][Lightning]") {
    NF::LightningSystem sys;
    sys.createRegion("ra"); sys.createRegion("rb");

    NF::LightningEvent x; x.id = "x"; x.intensity = NF::LightningIntensity::Extreme; x.phase = NF::LightningStormPhase::Peak; x.coverage = 500.0f; x.strikeRate = 80.0f; x.activate();
    NF::LightningEvent y; y.id = "y"; y.intensity = NF::LightningIntensity::Scattered; y.phase = NF::LightningStormPhase::Building; y.coverage = 10.0f; y.activate();
    NF::LightningEvent z; z.id = "z"; z.intensity = NF::LightningIntensity::Severe; z.phase = NF::LightningStormPhase::Peak; z.coverage = 400.0f; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()     == 2);
    REQUIRE(sys.severeEventCount()     == 1); // only x (y=Scattered, z inactive)
    REQUIRE(sys.peakEventCount()       == 1); // only x (y=Building, z inactive)
    REQUIRE(sys.widespreadEventCount() == 1); // only x (y=10km², z inactive)
}



// ── G60 — Geomagnetic Storm System ──────────────────────────────

TEST_CASE("GeoStormKpIndex names cover all 5 values", "[Game][G60][GeoStorm]") {
    REQUIRE(std::string(NF::geoStormKpIndexName(NF::GeoStormKpIndex::Quiet))     == "Quiet");
    REQUIRE(std::string(NF::geoStormKpIndexName(NF::GeoStormKpIndex::Unsettled)) == "Unsettled");
    REQUIRE(std::string(NF::geoStormKpIndexName(NF::GeoStormKpIndex::Active))    == "Active");
    REQUIRE(std::string(NF::geoStormKpIndexName(NF::GeoStormKpIndex::Storm))     == "Storm");
    REQUIRE(std::string(NF::geoStormKpIndexName(NF::GeoStormKpIndex::Extreme))   == "Extreme");
}

TEST_CASE("GeoStormPhase names cover all 5 values", "[Game][G60][GeoStorm]") {
    REQUIRE(std::string(NF::geoStormPhaseName(NF::GeoStormPhase::Onset))     == "Onset");
    REQUIRE(std::string(NF::geoStormPhaseName(NF::GeoStormPhase::Initial))   == "Initial");
    REQUIRE(std::string(NF::geoStormPhaseName(NF::GeoStormPhase::Main))      == "Main");
    REQUIRE(std::string(NF::geoStormPhaseName(NF::GeoStormPhase::Recovery))  == "Recovery");
    REQUIRE(std::string(NF::geoStormPhaseName(NF::GeoStormPhase::Quiescent)) == "Quiescent");
}

TEST_CASE("GeoStormEvent isSevere threshold is Storm+", "[Game][G60][GeoStorm]") {
    NF::GeoStormEvent ev; ev.id = "g1";
    ev.kp = NF::GeoStormKpIndex::Active;
    REQUIRE_FALSE(ev.isSevere());

    ev.kp = NF::GeoStormKpIndex::Storm;
    REQUIRE(ev.isSevere());

    ev.kp = NF::GeoStormKpIndex::Extreme;
    REQUIRE(ev.isSevere());
}

TEST_CASE("GeoStormEvent isMainPhase and isGlobal", "[Game][G60][GeoStorm]") {
    NF::GeoStormEvent ev; ev.id = "g2";
    ev.phase    = NF::GeoStormPhase::Initial;
    ev.coverage = 20.0f;
    REQUIRE_FALSE(ev.isMainPhase());
    REQUIRE_FALSE(ev.isGlobal());

    ev.phase    = NF::GeoStormPhase::Main;
    ev.coverage = 70.0f;
    REQUIRE(ev.isMainPhase());
    REQUIRE(ev.isGlobal());
}

TEST_CASE("GeoStormEvent hazardScore calculation", "[Game][G60][GeoStorm]") {
    NF::GeoStormEvent ev; ev.id = "g3";
    // Storm (uint8=3): (3+1) * 50 / 10 = 20.0
    ev.kp       = NF::GeoStormKpIndex::Storm;
    ev.coverage = 50.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(20.0f));
}

TEST_CASE("GeoStormEvent activate and deactivate", "[Game][G60][GeoStorm]") {
    NF::GeoStormEvent ev; ev.id = "g4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("GeoStormRegion severeCount, mainPhaseCount, globalCount", "[Game][G60][GeoStorm]") {
    NF::GeoStormRegion region("polar");

    NF::GeoStormEvent a; a.id = "a"; a.kp = NF::GeoStormKpIndex::Extreme; a.phase = NF::GeoStormPhase::Main;     a.coverage = 80.0f; a.activate();
    NF::GeoStormEvent b; b.id = "b"; b.kp = NF::GeoStormKpIndex::Quiet;   b.phase = NF::GeoStormPhase::Initial;  b.coverage = 10.0f; b.activate();
    NF::GeoStormEvent c; c.id = "c"; c.kp = NF::GeoStormKpIndex::Storm;   c.phase = NF::GeoStormPhase::Main;     c.coverage = 60.0f; // inactive

    region.addEvent(a); region.addEvent(b); region.addEvent(c);

    REQUIRE(region.severeCount()    == 1); // only a (b=Quiet, c inactive)
    REQUIRE(region.mainPhaseCount() == 1); // only a (b=Initial, c inactive)
    REQUIRE(region.globalCount()    == 1); // only a (b=10%, c inactive)
}

TEST_CASE("GeoStormSystem createRegion and tick propagation", "[Game][G60][GeoStorm]") {
    NF::GeoStormSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
}

TEST_CASE("GeoStormSystem aggregate event counts", "[Game][G60][GeoStorm]") {
    NF::GeoStormSystem sys;
    sys.createRegion("ra"); sys.createRegion("rb");

    NF::GeoStormEvent x; x.id = "x"; x.kp = NF::GeoStormKpIndex::Extreme; x.phase = NF::GeoStormPhase::Main;    x.coverage = 75.0f; x.activate();
    NF::GeoStormEvent y; y.id = "y"; y.kp = NF::GeoStormKpIndex::Unsettled;y.phase = NF::GeoStormPhase::Onset;   y.coverage = 5.0f;  y.activate();
    NF::GeoStormEvent z; z.id = "z"; z.kp = NF::GeoStormKpIndex::Storm;    z.phase = NF::GeoStormPhase::Main;    z.coverage = 60.0f; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()    == 2);
    REQUIRE(sys.severeEventCount()    == 1); // only x (y=Unsettled, z inactive)
    REQUIRE(sys.mainPhaseEventCount() == 1); // only x (y=Onset, z inactive)
    REQUIRE(sys.globalEventCount()    == 1); // only x (y=5%, z inactive)
}

// ── G61 — Cosmic Ray System ────────────────────────────────────

TEST_CASE("CosmicRayType names cover all 5 values", "[Game][G61][CosmicRay]") {
    REQUIRE(std::string(NF::cosmicRayTypeName(NF::CosmicRayType::Proton))   == "Proton");
    REQUIRE(std::string(NF::cosmicRayTypeName(NF::CosmicRayType::Alpha))    == "Alpha");
    REQUIRE(std::string(NF::cosmicRayTypeName(NF::CosmicRayType::HeavyIon)) == "HeavyIon");
    REQUIRE(std::string(NF::cosmicRayTypeName(NF::CosmicRayType::Electron)) == "Electron");
    REQUIRE(std::string(NF::cosmicRayTypeName(NF::CosmicRayType::Positron)) == "Positron");
}

TEST_CASE("CosmicRayIntensity names cover all 5 values", "[Game][G61][CosmicRay]") {
    REQUIRE(std::string(NF::cosmicRayIntensityName(NF::CosmicRayIntensity::Background)) == "Background");
    REQUIRE(std::string(NF::cosmicRayIntensityName(NF::CosmicRayIntensity::Elevated))   == "Elevated");
    REQUIRE(std::string(NF::cosmicRayIntensityName(NF::CosmicRayIntensity::High))       == "High");
    REQUIRE(std::string(NF::cosmicRayIntensityName(NF::CosmicRayIntensity::Severe))     == "Severe");
    REQUIRE(std::string(NF::cosmicRayIntensityName(NF::CosmicRayIntensity::Extreme))    == "Extreme");
}

TEST_CASE("CosmicRayEvent isSevere threshold is Severe+", "[Game][G61][CosmicRay]") {
    NF::CosmicRayEvent ev; ev.id = "cr1";
    ev.intensity = NF::CosmicRayIntensity::High;
    REQUIRE_FALSE(ev.isSevere());

    ev.intensity = NF::CosmicRayIntensity::Severe;
    REQUIRE(ev.isSevere());

    ev.intensity = NF::CosmicRayIntensity::Extreme;
    REQUIRE(ev.isSevere());
}

TEST_CASE("CosmicRayEvent isHighEnergy and isWidespread", "[Game][G61][CosmicRay]") {
    NF::CosmicRayEvent ev; ev.id = "cr2";
    ev.type     = NF::CosmicRayType::Electron;
    ev.coverage = 20.0f;
    REQUIRE_FALSE(ev.isHighEnergy());
    REQUIRE_FALSE(ev.isWidespread());

    ev.type     = NF::CosmicRayType::Alpha;
    ev.coverage = 70.0f;
    REQUIRE(ev.isHighEnergy());
    REQUIRE(ev.isWidespread());
}

TEST_CASE("CosmicRayEvent hazardScore calculation", "[Game][G61][CosmicRay]") {
    NF::CosmicRayEvent ev; ev.id = "cr3";
    // Severe (uint8=3): (3+1) * 50 / 10 = 20.0
    ev.intensity = NF::CosmicRayIntensity::Severe;
    ev.coverage  = 50.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(20.0f));
}

TEST_CASE("CosmicRayEvent activate and deactivate", "[Game][G61][CosmicRay]") {
    NF::CosmicRayEvent ev; ev.id = "cr4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("CosmicRayRegion severeCount, highEnergyCount, widespreadCount", "[Game][G61][CosmicRay]") {
    NF::CosmicRayRegion region("outerBelt");

    NF::CosmicRayEvent a; a.id = "a"; a.type = NF::CosmicRayType::Proton;   a.intensity = NF::CosmicRayIntensity::Extreme; a.coverage = 80.0f; a.activate();
    NF::CosmicRayEvent b; b.id = "b"; b.type = NF::CosmicRayType::Electron; b.intensity = NF::CosmicRayIntensity::Elevated; b.coverage = 10.0f; b.activate();
    NF::CosmicRayEvent c; c.id = "c"; c.type = NF::CosmicRayType::Alpha;    c.intensity = NF::CosmicRayIntensity::Severe;   c.coverage = 60.0f; // inactive

    region.addEvent(a); region.addEvent(b); region.addEvent(c);

    REQUIRE(region.severeCount()     == 1); // only a (b=Elevated, c inactive)
    REQUIRE(region.highEnergyCount() == 1); // only a (b=Electron, c inactive)
    REQUIRE(region.widespreadCount() == 1); // only a (b=10%, c inactive)
}

TEST_CASE("CosmicRaySystem createRegion and tick propagation", "[Game][G61][CosmicRay]") {
    NF::CosmicRaySystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
}

TEST_CASE("CosmicRaySystem aggregate event counts", "[Game][G61][CosmicRay]") {
    NF::CosmicRaySystem sys;
    sys.createRegion("ra"); sys.createRegion("rb");

    NF::CosmicRayEvent x; x.id = "x"; x.type = NF::CosmicRayType::HeavyIon; x.intensity = NF::CosmicRayIntensity::Extreme; x.coverage = 75.0f; x.activate();
    NF::CosmicRayEvent y; y.id = "y"; y.type = NF::CosmicRayType::Positron;  y.intensity = NF::CosmicRayIntensity::Elevated; y.coverage = 5.0f;  y.activate();
    NF::CosmicRayEvent z; z.id = "z"; z.type = NF::CosmicRayType::Proton;    z.intensity = NF::CosmicRayIntensity::Severe;   z.coverage = 60.0f; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()     == 2);
    REQUIRE(sys.severeEventCount()     == 1); // only x (y=Elevated, z inactive)
    REQUIRE(sys.highEnergyEventCount() == 1); // only x (y=Positron, z inactive)
    REQUIRE(sys.widespreadEventCount() == 1); // only x (y=5%, z inactive)
}

// ── G62 — Solar Wind System ────────────────────────────────────

TEST_CASE("SolarWindType names cover all 5 values", "[Game][G62][SolarWind]") {
    REQUIRE(std::string(NF::solarWindTypeName(NF::SolarWindType::Slow))           == "Slow");
    REQUIRE(std::string(NF::solarWindTypeName(NF::SolarWindType::Fast))           == "Fast");
    REQUIRE(std::string(NF::solarWindTypeName(NF::SolarWindType::Transient))      == "Transient");
    REQUIRE(std::string(NF::solarWindTypeName(NF::SolarWindType::Coronal))        == "Coronal");
    REQUIRE(std::string(NF::solarWindTypeName(NF::SolarWindType::Interplanetary)) == "Interplanetary");
}

TEST_CASE("SolarWindIntensity names cover all 5 values", "[Game][G62][SolarWind]") {
    REQUIRE(std::string(NF::solarWindIntensityName(NF::SolarWindIntensity::Calm))     == "Calm");
    REQUIRE(std::string(NF::solarWindIntensityName(NF::SolarWindIntensity::Moderate)) == "Moderate");
    REQUIRE(std::string(NF::solarWindIntensityName(NF::SolarWindIntensity::Strong))   == "Strong");
    REQUIRE(std::string(NF::solarWindIntensityName(NF::SolarWindIntensity::Violent))  == "Violent");
    REQUIRE(std::string(NF::solarWindIntensityName(NF::SolarWindIntensity::Extreme))  == "Extreme");
}

TEST_CASE("SolarWindEvent isSevere threshold is Violent+", "[Game][G62][SolarWind]") {
    NF::SolarWindEvent ev; ev.id = "sw1";
    ev.intensity = NF::SolarWindIntensity::Strong;
    REQUIRE_FALSE(ev.isSevere());

    ev.intensity = NF::SolarWindIntensity::Violent;
    REQUIRE(ev.isSevere());

    ev.intensity = NF::SolarWindIntensity::Extreme;
    REQUIRE(ev.isSevere());
}

TEST_CASE("SolarWindEvent isFastStream and isWidespread", "[Game][G62][SolarWind]") {
    NF::SolarWindEvent ev; ev.id = "sw2";
    ev.type     = NF::SolarWindType::Slow;
    ev.coverage = 20.0f;
    REQUIRE_FALSE(ev.isFastStream());
    REQUIRE_FALSE(ev.isWidespread());

    ev.type     = NF::SolarWindType::Fast;
    ev.coverage = 70.0f;
    REQUIRE(ev.isFastStream());
    REQUIRE(ev.isWidespread());
}

TEST_CASE("SolarWindEvent hazardScore calculation", "[Game][G62][SolarWind]") {
    NF::SolarWindEvent ev; ev.id = "sw3";
    // Violent (uint8=3): (3+1) * 50 / 10 = 20.0
    ev.intensity = NF::SolarWindIntensity::Violent;
    ev.coverage  = 50.0f;
    REQUIRE(ev.hazardScore() == Catch::Approx(20.0f));
}

TEST_CASE("SolarWindEvent activate and deactivate", "[Game][G62][SolarWind]") {
    NF::SolarWindEvent ev; ev.id = "sw4";
    REQUIRE_FALSE(ev.active);
    ev.activate();
    REQUIRE(ev.active);
    ev.deactivate();
    REQUIRE_FALSE(ev.active);
}

TEST_CASE("SolarWindRegion severeCount, fastStreamCount, widespreadCount", "[Game][G62][SolarWind]") {
    NF::SolarWindRegion region("heliosphere");

    NF::SolarWindEvent a; a.id = "a"; a.type = NF::SolarWindType::Fast;    a.intensity = NF::SolarWindIntensity::Extreme; a.coverage = 80.0f; a.activate();
    NF::SolarWindEvent b; b.id = "b"; b.type = NF::SolarWindType::Slow;    b.intensity = NF::SolarWindIntensity::Moderate; b.coverage = 10.0f; b.activate();
    NF::SolarWindEvent c; c.id = "c"; c.type = NF::SolarWindType::Transient; c.intensity = NF::SolarWindIntensity::Violent; c.coverage = 60.0f; // inactive

    region.addEvent(a); region.addEvent(b); region.addEvent(c);

    REQUIRE(region.severeCount()     == 1); // only a (b=Moderate, c inactive)
    REQUIRE(region.fastStreamCount() == 1); // only a (b=Slow, c inactive)
    REQUIRE(region.widespreadCount() == 1); // only a (b=10%, c inactive)
}

TEST_CASE("SolarWindSystem createRegion and tick propagation", "[Game][G62][SolarWind]") {
    NF::SolarWindSystem sys;
    REQUIRE(sys.createRegion("r1") != nullptr);
    REQUIRE(sys.createRegion("r2") != nullptr);
    REQUIRE(sys.createRegion("r1") == nullptr); // duplicate
    REQUIRE(sys.regionCount() == 2);

    sys.tick(); sys.tick();
    REQUIRE(sys.tickCount()               == 2);
    REQUIRE(sys.byName("r1")->tickCount() == 2);
}

TEST_CASE("SolarWindSystem aggregate event counts", "[Game][G62][SolarWind]") {
    NF::SolarWindSystem sys;
    sys.createRegion("ra"); sys.createRegion("rb");

    NF::SolarWindEvent x; x.id = "x"; x.type = NF::SolarWindType::Transient; x.intensity = NF::SolarWindIntensity::Extreme; x.coverage = 75.0f; x.activate();
    NF::SolarWindEvent y; y.id = "y"; y.type = NF::SolarWindType::Coronal;    y.intensity = NF::SolarWindIntensity::Moderate; y.coverage = 5.0f;  y.activate();
    NF::SolarWindEvent z; z.id = "z"; z.type = NF::SolarWindType::Fast;       z.intensity = NF::SolarWindIntensity::Violent;  z.coverage = 60.0f; // inactive

    sys.byName("ra")->addEvent(x);
    sys.byName("ra")->addEvent(y);
    sys.byName("rb")->addEvent(z);

    REQUIRE(sys.activeEventCount()     == 2);
    REQUIRE(sys.severeEventCount()     == 1); // only x (y=Moderate, z inactive)
    REQUIRE(sys.fastStreamEventCount() == 1); // only x (y=Coronal not fast/transient, z inactive)
    REQUIRE(sys.widespreadEventCount() == 1); // only x (y=5%, z inactive)
}
