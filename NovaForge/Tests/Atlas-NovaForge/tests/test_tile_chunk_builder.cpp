#include "../engine/tile/TileChunkBuilder.h"
#include <iostream>
#include <cassert>
#include <set>

using namespace atlas::tile;
using namespace atlas::editor;

void test_chunk_builder_empty_layer() {
    TileMap map;
    map.gridCellSize = 32;
    TileLayer layer;
    layer.name = "Default";

    TileChunk chunk;
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk);

    assert(chunk.VertexCount() == 0);
    assert(chunk.IndexCount() == 0);
    assert(!chunk.dirty);
    std::cout << "[PASS] test_chunk_builder_empty_layer" << std::endl;
}

void test_chunk_builder_single_tile() {
    TileMap map;
    map.gridCellSize = 32;
    TileLayer layer;
    layer.name = "Default";

    TileInstance inst;
    inst.tileAssetId = 1;
    layer.tiles[{0, 0}] = inst;

    TileChunk chunk;
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk);

    // 1 tile = 4 vertices, 6 indices
    assert(chunk.VertexCount() == 4);
    assert(chunk.IndexCount() == 6);
    std::cout << "[PASS] test_chunk_builder_single_tile" << std::endl;
}

void test_chunk_builder_multiple_tiles() {
    TileMap map;
    map.gridCellSize = 32;
    TileLayer layer;
    layer.name = "Default";

    for (int i = 0; i < 4; ++i) {
        TileInstance inst;
        inst.tileAssetId = 1;
        layer.tiles[{i, 0}] = inst;
    }

    TileChunk chunk;
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk);

    assert(chunk.VertexCount() == 16);  // 4 tiles * 4 verts
    assert(chunk.IndexCount() == 24);   // 4 tiles * 6 indices
    std::cout << "[PASS] test_chunk_builder_multiple_tiles" << std::endl;
}

void test_chunk_builder_world_to_chunk() {
    auto c = TileChunkBuilder::WorldToChunk({0, 0});
    assert(c.cx == 0 && c.cy == 0);

    c = TileChunkBuilder::WorldToChunk({7, 7});
    assert(c.cx == 0 && c.cy == 0);

    c = TileChunkBuilder::WorldToChunk({8, 0});
    assert(c.cx == 1 && c.cy == 0);

    c = TileChunkBuilder::WorldToChunk({16, 16});
    assert(c.cx == 2 && c.cy == 2);
    std::cout << "[PASS] test_chunk_builder_world_to_chunk" << std::endl;
}

void test_chunk_builder_world_to_chunk_negative() {
    auto c = TileChunkBuilder::WorldToChunk({-1, -1});
    assert(c.cx == -1 && c.cy == -1);

    c = TileChunkBuilder::WorldToChunk({-8, 0});
    assert(c.cx == -1 && c.cy == 0);

    c = TileChunkBuilder::WorldToChunk({-9, -9});
    assert(c.cx == -2 && c.cy == -2);
    std::cout << "[PASS] test_chunk_builder_world_to_chunk_negative" << std::endl;
}

void test_chunk_builder_is_inside_chunk() {
    ChunkCoord origin{0, 0};
    assert(TileChunkBuilder::IsInsideChunk({0, 0}, origin));
    assert(TileChunkBuilder::IsInsideChunk({7, 7}, origin));
    assert(!TileChunkBuilder::IsInsideChunk({8, 0}, origin));
    assert(!TileChunkBuilder::IsInsideChunk({-1, 0}, origin));
    std::cout << "[PASS] test_chunk_builder_is_inside_chunk" << std::endl;
}

void test_chunk_builder_mark_dirty() {
    std::set<ChunkCoord> dirty;
    TileChunkBuilder::MarkDirty(dirty, {0, 0});
    assert(dirty.size() == 1);
    assert(dirty.begin()->cx == 0);
    assert(dirty.begin()->cy == 0);

    TileChunkBuilder::MarkDirty(dirty, {8, 8});
    assert(dirty.size() == 2);
    std::cout << "[PASS] test_chunk_builder_mark_dirty" << std::endl;
}

void test_chunk_builder_tiles_outside_chunk_ignored() {
    TileMap map;
    map.gridCellSize = 32;
    TileLayer layer;
    layer.name = "Default";

    // Place tile at (0,0) - inside chunk (0,0)
    layer.tiles[{0, 0}] = TileInstance{1};
    // Place tile at (10,0) - inside chunk (1,0) not chunk (0,0)
    layer.tiles[{10, 0}] = TileInstance{2};

    TileChunk chunk;
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk);

    // Only the tile at (0,0) should be included
    assert(chunk.VertexCount() == 4);
    assert(chunk.IndexCount() == 6);
    std::cout << "[PASS] test_chunk_builder_tiles_outside_chunk_ignored" << std::endl;
}

void test_chunk_builder_deterministic() {
    TileMap map;
    map.gridCellSize = 32;
    TileLayer layer;
    layer.name = "Default";

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            TileInstance inst;
            inst.tileAssetId = static_cast<uint32_t>(i * 8 + j + 1);
            layer.tiles[{i, j}] = inst;
        }
    }

    TileChunk chunk1, chunk2;
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk1);
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk2);

    assert(chunk1.VertexCount() == chunk2.VertexCount());
    assert(chunk1.IndexCount() == chunk2.IndexCount());

    // Vertex data must be bit-exact
    for (size_t i = 0; i < chunk1.vertices.size(); ++i) {
        assert(chunk1.vertices[i].x == chunk2.vertices[i].x);
        assert(chunk1.vertices[i].y == chunk2.vertices[i].y);
        assert(chunk1.vertices[i].u == chunk2.vertices[i].u);
        assert(chunk1.vertices[i].v == chunk2.vertices[i].v);
    }
    std::cout << "[PASS] test_chunk_builder_deterministic" << std::endl;
}

void test_chunk_builder_flip_flags() {
    TileMap map;
    map.gridCellSize = 32;
    TileLayer layer;
    layer.name = "Default";

    TileInstance inst;
    inst.tileAssetId = 1;
    inst.flippedX = true;
    inst.flippedY = true;
    layer.tiles[{0, 0}] = inst;

    TileChunk chunk;
    TileChunkBuilder::Build(map, layer, {0, 0}, chunk);

    assert(chunk.VertexCount() == 4);
    // When flipped, U coords should be swapped
    assert(chunk.vertices[0].u == 1.0f);
    assert(chunk.vertices[1].u == 0.0f);
    // When flippedY, V coords should be swapped
    assert(chunk.vertices[0].v == 1.0f);
    assert(chunk.vertices[3].v == 0.0f);
    std::cout << "[PASS] test_chunk_builder_flip_flags" << std::endl;
}
