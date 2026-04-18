#pragma once
// NF::World — World gen: cube-sphere, voxel, terrain, galaxy, streaming
#include "NF/Core/Core.h"
#include "NF/Game/Game.h"
#include <cmath>

namespace NF {

class WorldGenerator {
public:
    void init(uint32_t seed) {
        m_seed = seed;
        NF_LOG_INFO("World", "World generator initialized with seed " + std::to_string(seed));
    }

    void shutdown() { NF_LOG_INFO("World", "World generator shutdown"); }

    Chunk generateChunk(int cx, int cy, int cz) const {
        Chunk chunk;
        chunk.cx = cx;
        chunk.cy = cy;
        chunk.cz = cz;

        // Simple flat terrain with hills
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                int worldX = cx * CHUNK_SIZE + x;
                int worldZ = cz * CHUNK_SIZE + z;
                int height = 8 + static_cast<int>(2.0f *
                    std::sin(worldX * 0.05f) * std::cos(worldZ * 0.05f));

                for (int y = 0; y < CHUNK_SIZE; ++y) {
                    int worldY = cy * CHUNK_SIZE + y;
                    if (worldY < height - 3)
                        chunk.voxels[x][y][z] = VoxelType::Stone;
                    else if (worldY < height)
                        chunk.voxels[x][y][z] = VoxelType::Dirt;
                    else if (worldY == height)
                        chunk.voxels[x][y][z] = VoxelType::Grass;
                    else
                        chunk.voxels[x][y][z] = VoxelType::Air;
                }
            }
        }

        chunk.meshDirty = true;
        chunk.collisionDirty = true;
        return chunk;
    }

private:
    uint32_t m_seed = 42;
};

} // namespace NF
