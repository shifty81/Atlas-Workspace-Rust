#include "SaveSystem.h"
#include "StateHasher.h"
#include <fstream>
#include <cstring>

namespace atlas::sim {

SaveResult SaveSystem::Save(const std::string& path,
                            uint64_t tick,
                            uint32_t tickRate,
                            uint32_t seed,
                            const std::vector<uint8_t>& ecsData,
                            const std::vector<uint8_t>& auxData,
                            const std::string& metadata) {
    // Compute deterministic hash over all saved state.
    uint64_t hash = StateHasher::HashCombine(0, ecsData.data(), ecsData.size());
    if (!auxData.empty()) {
        hash = StateHasher::HashCombine(hash, auxData.data(), auxData.size());
    }

    SaveHeader header;
    header.saveTick = tick;
    header.tickRate = tickRate;
    header.seed = seed;
    header.stateHash = hash;
    header.ecsDataSize = static_cast<uint32_t>(ecsData.size());
    header.auxDataSize = static_cast<uint32_t>(auxData.size());
    header.metadataSize = static_cast<uint32_t>(metadata.size());

    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        return SaveResult::IOError;
    }

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    if (!ecsData.empty()) {
        out.write(reinterpret_cast<const char*>(ecsData.data()),
                  static_cast<std::streamsize>(ecsData.size()));
    }
    if (!auxData.empty()) {
        out.write(reinterpret_cast<const char*>(auxData.data()),
                  static_cast<std::streamsize>(auxData.size()));
    }
    if (!metadata.empty()) {
        out.write(metadata.data(),
                  static_cast<std::streamsize>(metadata.size()));
    }

    if (!out.good()) {
        return SaveResult::IOError;
    }

    return SaveResult::Success;
}

SaveResult SaveSystem::Load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return SaveResult::FileNotFound;
    }

    SaveHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!in.good()) {
        return SaveResult::InvalidFormat;
    }

    if (header.magic != 0x41534156) {
        return SaveResult::InvalidFormat;
    }

    if (header.version != 1) {
        return SaveResult::VersionMismatch;
    }

    // Sanity check sizes to prevent malicious allocations.
    static constexpr uint32_t kMaxDataSize = 256 * 1024 * 1024;  // 256 MB
    if (header.ecsDataSize > kMaxDataSize ||
        header.auxDataSize > kMaxDataSize ||
        header.metadataSize > kMaxDataSize) {
        return SaveResult::InvalidFormat;
    }

    std::vector<uint8_t> ecsData(header.ecsDataSize);
    if (header.ecsDataSize > 0) {
        in.read(reinterpret_cast<char*>(ecsData.data()),
                static_cast<std::streamsize>(header.ecsDataSize));
    }

    std::vector<uint8_t> auxData(header.auxDataSize);
    if (header.auxDataSize > 0) {
        in.read(reinterpret_cast<char*>(auxData.data()),
                static_cast<std::streamsize>(header.auxDataSize));
    }

    std::string metadata(header.metadataSize, '\0');
    if (header.metadataSize > 0) {
        in.read(metadata.data(),
                static_cast<std::streamsize>(header.metadataSize));
    }

    if (!in.good() && !in.eof()) {
        return SaveResult::IOError;
    }

    // Verify hash integrity.
    uint64_t hash = StateHasher::HashCombine(0, ecsData.data(), ecsData.size());
    if (!auxData.empty()) {
        hash = StateHasher::HashCombine(hash, auxData.data(), auxData.size());
    }

    if (hash != header.stateHash) {
        return SaveResult::HashMismatch;
    }

    m_header = header;
    m_ecsData = std::move(ecsData);
    m_auxData = std::move(auxData);
    m_metadata = std::move(metadata);

    return SaveResult::Success;
}

SaveResult SaveSystem::Validate(const std::string& path) const {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return SaveResult::FileNotFound;
    }

    SaveHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!in.good()) {
        return SaveResult::InvalidFormat;
    }

    if (header.magic != 0x41534156) {
        return SaveResult::InvalidFormat;
    }

    if (header.version != 1) {
        return SaveResult::VersionMismatch;
    }

    return SaveResult::Success;
}

const SaveHeader& SaveSystem::Header() const {
    return m_header;
}

const std::vector<uint8_t>& SaveSystem::ECSData() const {
    return m_ecsData;
}

const std::vector<uint8_t>& SaveSystem::AuxData() const {
    return m_auxData;
}

const std::string& SaveSystem::Metadata() const {
    return m_metadata;
}

void SaveSystem::Clear() {
    m_header = SaveHeader{};
    m_ecsData.clear();
    m_auxData.clear();
    m_metadata.clear();
}

SaveResult SaveSystem::SavePartial(const std::string& path,
                                   uint64_t tick,
                                   uint32_t tickRate,
                                   uint32_t seed,
                                   const std::vector<ChunkSaveEntry>& chunks) {
    // Compute hash over all chunk data.
    uint64_t hash = 0;
    for (const auto& chunk : chunks) {
        if (!chunk.data.empty()) {
            hash = StateHasher::HashCombine(hash, chunk.data.data(), chunk.data.size());
        }
    }

    PartialSaveHeader header;
    header.saveTick = tick;
    header.tickRate = tickRate;
    header.seed = seed;
    header.stateHash = hash;
    header.chunkCount = static_cast<uint32_t>(chunks.size());

    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        return SaveResult::IOError;
    }

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    for (const auto& chunk : chunks) {
        out.write(reinterpret_cast<const char*>(&chunk.x), sizeof(int32_t));
        out.write(reinterpret_cast<const char*>(&chunk.y), sizeof(int32_t));
        out.write(reinterpret_cast<const char*>(&chunk.z), sizeof(int32_t));
        uint32_t dataSize = static_cast<uint32_t>(chunk.data.size());
        out.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));
        if (dataSize > 0) {
            out.write(reinterpret_cast<const char*>(chunk.data.data()),
                      static_cast<std::streamsize>(dataSize));
        }
    }

    if (!out.good()) {
        return SaveResult::IOError;
    }

    return SaveResult::Success;
}

SaveResult SaveSystem::LoadPartial(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return SaveResult::FileNotFound;
    }

    PartialSaveHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!in.good()) {
        return SaveResult::InvalidFormat;
    }

    if (header.magic != 0x41535057) {
        return SaveResult::InvalidFormat;
    }

    if (header.version != 1) {
        return SaveResult::VersionMismatch;
    }

    static constexpr uint32_t kMaxChunkDataSize = 256 * 1024 * 1024;  // 256 MB
    static constexpr uint32_t kMaxChunkCount = 1000000;  // sanity limit

    if (header.chunkCount > kMaxChunkCount) {
        return SaveResult::InvalidFormat;
    }

    std::vector<ChunkSaveEntry> chunks(header.chunkCount);
    for (uint32_t i = 0; i < header.chunkCount; ++i) {
        in.read(reinterpret_cast<char*>(&chunks[i].x), sizeof(int32_t));
        in.read(reinterpret_cast<char*>(&chunks[i].y), sizeof(int32_t));
        in.read(reinterpret_cast<char*>(&chunks[i].z), sizeof(int32_t));
        uint32_t dataSize = 0;
        in.read(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));
        if (dataSize > kMaxChunkDataSize) {
            return SaveResult::InvalidFormat;
        }
        chunks[i].data.resize(dataSize);
        if (dataSize > 0) {
            in.read(reinterpret_cast<char*>(chunks[i].data.data()),
                    static_cast<std::streamsize>(dataSize));
        }
    }

    if (!in.good() && !in.eof()) {
        return SaveResult::IOError;
    }

    // Verify hash integrity.
    uint64_t hash = 0;
    for (const auto& chunk : chunks) {
        if (!chunk.data.empty()) {
            hash = StateHasher::HashCombine(hash, chunk.data.data(), chunk.data.size());
        }
    }

    if (hash != header.stateHash) {
        return SaveResult::HashMismatch;
    }

    m_partialHeader = header;
    m_chunks = std::move(chunks);

    return SaveResult::Success;
}

const PartialSaveHeader& SaveSystem::PartialHeader() const {
    return m_partialHeader;
}

const std::vector<ChunkSaveEntry>& SaveSystem::Chunks() const {
    return m_chunks;
}

}  // namespace atlas::sim
