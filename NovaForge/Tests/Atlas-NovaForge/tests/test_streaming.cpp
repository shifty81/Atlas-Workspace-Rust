#include "../engine/world/WorldStreamer.h"
#include "../engine/world/VoxelGridLayout.h"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace atlas::world;

void test_streamer_request_load() {
    VoxelGridLayout layout;
    layout.chunkSize = 16;
    WorldStreamer streamer(layout);

    ChunkCoord chunk = {0, 0, 0, 0};
    bool requested = streamer.RequestLoad(chunk);
    assert(requested);

    assert(streamer.GetChunkState(chunk) == ChunkState::Loading);

    std::cout << "[PASS] test_streamer_request_load" << std::endl;
}

void test_streamer_set_chunk_data() {
    VoxelGridLayout layout;
    layout.chunkSize = 16;
    WorldStreamer streamer(layout);

    ChunkCoord chunk = {1, 0, 1, 0};
    streamer.RequestLoad(chunk);

    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    streamer.SetChunkData(chunk, data);

    assert(streamer.GetChunkState(chunk) == ChunkState::Loaded);
    assert(streamer.LoadedCount() == 1);

    std::cout << "[PASS] test_streamer_set_chunk_data" << std::endl;
}

void test_streamer_unload_chunk() {
    VoxelGridLayout layout;
    layout.chunkSize = 16;
    WorldStreamer streamer(layout);

    ChunkCoord chunk = {2, 0, 2, 0};
    std::vector<uint8_t> data = {10, 20, 30};
    streamer.SetChunkData(chunk, data);
    assert(streamer.LoadedCount() == 1);

    streamer.UnloadChunk(chunk);
    assert(streamer.GetChunkState(chunk) == ChunkState::Unloaded);
    assert(streamer.LoadedCount() == 0);

    std::cout << "[PASS] test_streamer_unload_chunk" << std::endl;
}

void test_streamer_get_loaded_chunks() {
    VoxelGridLayout layout;
    layout.chunkSize = 16;
    WorldStreamer streamer(layout);

    ChunkCoord c1 = {0, 0, 0, 0};
    ChunkCoord c2 = {1, 0, 0, 0};
    ChunkCoord c3 = {0, 0, 1, 0};

    streamer.SetChunkData(c1, {1});
    streamer.SetChunkData(c2, {2});
    streamer.SetChunkData(c3, {3});

    auto loaded = streamer.GetLoadedChunks();
    assert(loaded.size() == 3);

    std::cout << "[PASS] test_streamer_get_loaded_chunks" << std::endl;
}

void test_streamer_disk_cache() {
    std::string cacheDir = std::filesystem::temp_directory_path().string() + "/atlas_test_cache";
    // Clean up any previous test artifacts
    std::filesystem::remove_all(cacheDir);

    VoxelGridLayout layout;
    layout.chunkSize = 16;
    WorldStreamer streamer(layout, cacheDir);

    ChunkCoord chunk = {5, 0, 5, 0};
    std::vector<uint8_t> data = {100, 200, 42, 99};
    streamer.SetChunkData(chunk, data);

    // Save to cache
    bool saved = streamer.SaveChunkToCache(chunk);
    assert(saved);

    // Unload
    streamer.UnloadChunk(chunk);
    assert(streamer.GetChunkState(chunk) == ChunkState::Cached);

    // Load from cache
    bool loaded = streamer.LoadChunkFromCache(chunk);
    assert(loaded);
    assert(streamer.GetChunkState(chunk) == ChunkState::Loaded);

    // Clean up
    std::filesystem::remove_all(cacheDir);

    std::cout << "[PASS] test_streamer_disk_cache" << std::endl;
}

void test_streamer_duplicate_request() {
    VoxelGridLayout layout;
    layout.chunkSize = 16;
    WorldStreamer streamer(layout);

    ChunkCoord chunk = {0, 0, 0, 0};
    std::vector<uint8_t> data = {1, 2, 3};
    streamer.SetChunkData(chunk, data);

    // Requesting load on already loaded chunk should return false
    bool requested = streamer.RequestLoad(chunk);
    assert(!requested);

    std::cout << "[PASS] test_streamer_duplicate_request" << std::endl;
}
