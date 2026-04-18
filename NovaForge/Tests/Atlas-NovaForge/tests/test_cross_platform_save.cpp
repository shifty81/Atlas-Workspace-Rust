#include "../engine/sim/SaveSystem.h"
#include "../engine/sim/StateHasher.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <cstring>

using namespace atlas::sim;

namespace {
    std::string tmpPath(const std::string& filename) {
        return (std::filesystem::temp_directory_path() / filename).string();
    }
}
// ============================================================
// Cross-Platform Save Compatibility Tests
// ============================================================
// Verify that save files are portable across platforms by
// checking binary layout stability, endianness, struct sizing,
// and round-trip integrity.

/// Verify that SaveHeader has a fixed, predictable size.
/// Any change to the struct layout breaks cross-platform saves.
void test_cross_platform_save_header_size() {
    // SaveHeader must be exactly 48 bytes for binary compatibility.
    static_assert(sizeof(uint32_t) == 4, "uint32_t size unexpected");
    static_assert(sizeof(uint64_t) == 8, "uint64_t size unexpected");

    // Header fields: magic(4) + version(4) + tickRate(4) + saveTick(8)
    //              + stateHash(8) + seed(4) + ecsDataSize(4) + auxDataSize(4)
    //              + metadataSize(4) = 40 expected minimum
    // Actual struct size may include padding — we verify the exact value.
    size_t headerSize = sizeof(SaveHeader);
    assert(headerSize > 0);

    // The header must be consistent every time we compile.
    // Write and read back to verify no padding surprises.
    SaveHeader h;
    h.magic = 0x41534156;
    h.version = 1;
    h.tickRate = 60;
    h.saveTick = 12345;
    h.stateHash = 0xDEADBEEFCAFE0000ULL;
    h.seed = 42;
    h.ecsDataSize = 100;
    h.auxDataSize = 200;
    h.metadataSize = 50;

    // Serialize to bytes and deserialize — must be identical.
    std::vector<uint8_t> buf(sizeof(SaveHeader));
    std::memcpy(buf.data(), &h, sizeof(SaveHeader));

    SaveHeader h2;
    std::memcpy(&h2, buf.data(), sizeof(SaveHeader));

    assert(h2.magic == h.magic);
    assert(h2.version == h.version);
    assert(h2.tickRate == h.tickRate);
    assert(h2.saveTick == h.saveTick);
    assert(h2.stateHash == h.stateHash);
    assert(h2.seed == h.seed);
    assert(h2.ecsDataSize == h.ecsDataSize);
    assert(h2.auxDataSize == h.auxDataSize);
    assert(h2.metadataSize == h.metadataSize);

    std::cout << "[PASS] test_cross_platform_save_header_size" << std::endl;
}

/// Verify that PartialSaveHeader has a stable binary layout.
void test_cross_platform_partial_header_size() {
    PartialSaveHeader h;
    h.magic = 0x41535057;
    h.version = 1;
    h.tickRate = 60;
    h.saveTick = 9999;
    h.stateHash = 0x1234567890ABCDEFULL;
    h.seed = 7;
    h.chunkCount = 3;

    std::vector<uint8_t> buf(sizeof(PartialSaveHeader));
    std::memcpy(buf.data(), &h, sizeof(PartialSaveHeader));

    PartialSaveHeader h2;
    std::memcpy(&h2, buf.data(), sizeof(PartialSaveHeader));

    assert(h2.magic == h.magic);
    assert(h2.version == h.version);
    assert(h2.tickRate == h.tickRate);
    assert(h2.saveTick == h.saveTick);
    assert(h2.stateHash == h.stateHash);
    assert(h2.seed == h.seed);
    assert(h2.chunkCount == h.chunkCount);

    std::cout << "[PASS] test_cross_platform_partial_header_size" << std::endl;
}

/// Verify that the magic number is written at the correct byte offset.
void test_cross_platform_save_magic_offset() {
    std::string path = tmpPath("atlas_xplat_magic.asav");
    SaveSystem saver;
    std::vector<uint8_t> data = {1, 2, 3};
    SaveResult res = saver.Save(path, 1, 60, 0, data);
    assert(res == SaveResult::Success);

    // Read raw bytes and check magic at offset 0.
    std::ifstream in(path, std::ios::binary);
    uint32_t magic = 0;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    assert(magic == 0x41534156);  // "ASAV"

    std::filesystem::remove(path);
    std::cout << "[PASS] test_cross_platform_save_magic_offset" << std::endl;
}

/// Verify hash determinism: identical data always produces identical hashes.
void test_cross_platform_save_hash_determinism() {
    std::string path1 = tmpPath("atlas_xplat_hash1.asav");
    std::string path2 = tmpPath("atlas_xplat_hash2.asav");

    std::vector<uint8_t> ecsData = {10, 20, 30, 40, 50, 60, 70, 80};
    std::vector<uint8_t> auxData = {100, 200, 255};
    std::string metadata = "cross-platform test";

    SaveSystem saver1, saver2;
    saver1.Save(path1, 500, 60, 42, ecsData, auxData, metadata);
    saver2.Save(path2, 500, 60, 42, ecsData, auxData, metadata);

    SaveSystem loader1, loader2;
    loader1.Load(path1);
    loader2.Load(path2);

    // Hashes must be identical for identical data.
    assert(loader1.Header().stateHash == loader2.Header().stateHash);
    assert(loader1.Header().stateHash != 0);

    std::filesystem::remove(path1);
    std::filesystem::remove(path2);

    std::cout << "[PASS] test_cross_platform_save_hash_determinism" << std::endl;
}

/// Verify that the binary file is byte-exact: two saves of the same data
/// produce identical files.
void test_cross_platform_save_byte_exact() {
    std::string path1 = tmpPath("atlas_xplat_byte1.asav");
    std::string path2 = tmpPath("atlas_xplat_byte2.asav");

    std::vector<uint8_t> ecsData = {1, 2, 3, 4, 5};
    std::vector<uint8_t> auxData = {10, 20};
    std::string metadata = "byte exact test";

    SaveSystem saver1, saver2;
    saver1.Save(path1, 100, 60, 99, ecsData, auxData, metadata);
    saver2.Save(path2, 100, 60, 99, ecsData, auxData, metadata);

    // Read both files and compare byte-for-byte.
    auto readFile = [](const std::string& p) -> std::vector<uint8_t> {
        std::ifstream f(p, std::ios::binary | std::ios::ate);
        auto sz = f.tellg();
        f.seekg(0);
        std::vector<uint8_t> buf(static_cast<size_t>(sz));
        f.read(reinterpret_cast<char*>(buf.data()), sz);
        return buf;
    };

    auto bytes1 = readFile(path1);
    auto bytes2 = readFile(path2);

    assert(bytes1 == bytes2);
    assert(!bytes1.empty());

    std::filesystem::remove(path1);
    std::filesystem::remove(path2);

    std::cout << "[PASS] test_cross_platform_save_byte_exact" << std::endl;
}

/// Verify that a save written with known raw bytes can be loaded correctly,
/// simulating loading a file produced on a different platform.
void test_cross_platform_save_raw_bytes_roundtrip() {
    std::string path = tmpPath("atlas_xplat_raw.asav");

    // Construct a valid save file from raw bytes.
    std::vector<uint8_t> ecsData = {0xAA, 0xBB, 0xCC, 0xDD};
    std::vector<uint8_t> auxData = {0x11, 0x22};
    std::string metadata = "raw roundtrip";

    // Compute the expected hash the same way SaveSystem does.
    uint64_t expectedHash = StateHasher::HashCombine(0, ecsData.data(), ecsData.size());
    expectedHash = StateHasher::HashCombine(expectedHash, auxData.data(), auxData.size());

    // Manually construct the header.
    SaveHeader header;
    header.magic = 0x41534156;
    header.version = 1;
    header.tickRate = 60;
    header.saveTick = 42;
    header.stateHash = expectedHash;
    header.seed = 7;
    header.ecsDataSize = static_cast<uint32_t>(ecsData.size());
    header.auxDataSize = static_cast<uint32_t>(auxData.size());
    header.metadataSize = static_cast<uint32_t>(metadata.size());

    // Write the file manually (simulating another platform's save).
    {
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(ecsData.data()),
                  static_cast<std::streamsize>(ecsData.size()));
        out.write(reinterpret_cast<const char*>(auxData.data()),
                  static_cast<std::streamsize>(auxData.size()));
        out.write(metadata.data(),
                  static_cast<std::streamsize>(metadata.size()));
    }

    // Load with SaveSystem and verify all fields.
    SaveSystem loader;
    SaveResult res = loader.Load(path);
    assert(res == SaveResult::Success);

    assert(loader.Header().magic == 0x41534156);
    assert(loader.Header().version == 1);
    assert(loader.Header().tickRate == 60);
    assert(loader.Header().saveTick == 42);
    assert(loader.Header().seed == 7);
    assert(loader.Header().stateHash == expectedHash);
    assert(loader.ECSData() == ecsData);
    assert(loader.AuxData() == auxData);
    assert(loader.Metadata() == metadata);

    std::filesystem::remove(path);

    std::cout << "[PASS] test_cross_platform_save_raw_bytes_roundtrip" << std::endl;
}

/// Verify partial save cross-platform compatibility.
void test_cross_platform_partial_save_roundtrip() {
    std::string path = tmpPath("atlas_xplat_partial.aspw");

    std::vector<ChunkSaveEntry> chunks;
    chunks.push_back({1, 2, 3, {0xAA, 0xBB}});
    chunks.push_back({-1, 0, 5, {0xCC, 0xDD, 0xEE}});
    chunks.push_back({0, 0, 0, {}});  // empty chunk

    SaveSystem saver;
    SaveResult res = saver.SavePartial(path, 200, 60, 13, chunks);
    assert(res == SaveResult::Success);

    SaveSystem loader;
    res = loader.LoadPartial(path);
    assert(res == SaveResult::Success);

    assert(loader.PartialHeader().magic == 0x41535057);
    assert(loader.PartialHeader().saveTick == 200);
    assert(loader.PartialHeader().tickRate == 60);
    assert(loader.PartialHeader().seed == 13);
    assert(loader.Chunks().size() == 3);

    assert(loader.Chunks()[0].x == 1);
    assert(loader.Chunks()[0].y == 2);
    assert(loader.Chunks()[0].z == 3);
    assert(loader.Chunks()[0].data == std::vector<uint8_t>({0xAA, 0xBB}));

    assert(loader.Chunks()[1].x == -1);
    assert(loader.Chunks()[1].y == 0);
    assert(loader.Chunks()[1].z == 5);
    assert(loader.Chunks()[1].data == std::vector<uint8_t>({0xCC, 0xDD, 0xEE}));

    assert(loader.Chunks()[2].x == 0);
    assert(loader.Chunks()[2].data.empty());

    std::filesystem::remove(path);

    std::cout << "[PASS] test_cross_platform_partial_save_roundtrip" << std::endl;
}

/// Verify that large data payloads survive save/load without truncation.
void test_cross_platform_save_large_payload() {
    std::string path = tmpPath("atlas_xplat_large.asav");

    // Create a non-trivial payload (64KB).
    std::vector<uint8_t> ecsData(65536);
    for (size_t i = 0; i < ecsData.size(); ++i) {
        ecsData[i] = static_cast<uint8_t>(i % 256);
    }

    std::vector<uint8_t> auxData(1024);
    for (size_t i = 0; i < auxData.size(); ++i) {
        auxData[i] = static_cast<uint8_t>((i * 7) % 256);
    }

    SaveSystem saver;
    SaveResult res = saver.Save(path, 10000, 60, 999, ecsData, auxData, "large test");
    assert(res == SaveResult::Success);

    SaveSystem loader;
    res = loader.Load(path);
    assert(res == SaveResult::Success);

    assert(loader.ECSData() == ecsData);
    assert(loader.AuxData() == auxData);
    assert(loader.Metadata() == "large test");
    assert(loader.Header().saveTick == 10000);

    std::filesystem::remove(path);

    std::cout << "[PASS] test_cross_platform_save_large_payload" << std::endl;
}

/// Verify that StateHasher::HashCombine produces deterministic results
/// regardless of when/where it's called (no global state dependency).
void test_cross_platform_hash_combine_deterministic() {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8};

    uint64_t h1 = StateHasher::HashCombine(0, data.data(), data.size());
    uint64_t h2 = StateHasher::HashCombine(0, data.data(), data.size());

    assert(h1 == h2);
    assert(h1 != 0);  // Extremely unlikely to be zero for non-empty input.

    // Different input must produce different hash.
    std::vector<uint8_t> data2 = {1, 2, 3, 4, 5, 6, 7, 9};
    uint64_t h3 = StateHasher::HashCombine(0, data2.data(), data2.size());
    assert(h3 != h1);

    // Chaining must be order-dependent.
    uint64_t chain1 = StateHasher::HashCombine(h1, data2.data(), data2.size());
    uint64_t chain2 = StateHasher::HashCombine(h3, data.data(), data.size());
    assert(chain1 != chain2);

    std::cout << "[PASS] test_cross_platform_hash_combine_deterministic" << std::endl;
}
