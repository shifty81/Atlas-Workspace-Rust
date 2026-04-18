#pragma once
// ============================================================
// Atlas Save System — Deterministic Save/Load
// ============================================================
//
// Provides first-class save and load functionality that preserves
// determinism guarantees across the save/load boundary.
//
// Design principles:
//   - A save file is a WorldSnapshot plus replay metadata.
//   - Loading a save restores the exact simulation state.
//   - After load, all subsequent ticks must match a reference
//     replay (if one exists) to verify correctness.
//   - Save format is versioned for forward compatibility.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/14_REPLAY_AND_PROOFS.md

#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/// Magic number and version for save files.
struct SaveHeader {
    uint32_t magic = 0x41534156;  // "ASAV"
    uint32_t version = 1;
    uint32_t tickRate = 30;
    uint64_t saveTick = 0;        ///< Tick at which the save was taken.
    uint64_t stateHash = 0;       ///< Hash of the saved state.
    uint32_t seed = 0;            ///< World seed for generation.
    uint32_t ecsDataSize = 0;     ///< Size of serialized ECS data.
    uint32_t auxDataSize = 0;     ///< Size of auxiliary data.
    uint32_t metadataSize = 0;    ///< Size of optional metadata string.
};

/// Result of a save or load operation.
enum class SaveResult {
    Success,
    FileNotFound,
    InvalidFormat,
    VersionMismatch,
    HashMismatch,
    IOError
};

/// Header for a partial (chunk-based) save file.
struct PartialSaveHeader {
    uint32_t magic = 0x41535057;  // "ASPW" (Atlas Save Partial World)
    uint32_t version = 1;
    uint32_t tickRate = 30;
    uint64_t saveTick = 0;
    uint64_t stateHash = 0;
    uint32_t seed = 0;
    uint32_t chunkCount = 0;      ///< Number of chunks in this save.
};

/// A single chunk entry in a partial save.
struct ChunkSaveEntry {
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    std::vector<uint8_t> data;
};

/// Deterministic save/load system.
class SaveSystem {
public:
    /// Save the current world state to a file.
    /// @param path      Destination file path.
    /// @param tick      Current simulation tick.
    /// @param tickRate  Simulation tick rate.
    /// @param seed      World generation seed.
    /// @param ecsData   Serialized ECS world state.
    /// @param auxData   Additional simulated state.
    /// @param metadata  Optional human-readable metadata (e.g. save name).
    SaveResult Save(const std::string& path,
                    uint64_t tick,
                    uint32_t tickRate,
                    uint32_t seed,
                    const std::vector<uint8_t>& ecsData,
                    const std::vector<uint8_t>& auxData = {},
                    const std::string& metadata = "");

    /// Load world state from a save file.
    /// After a successful load, use the accessors below to read data.
    SaveResult Load(const std::string& path);

    /// Verify that a save file is valid without fully loading it.
    SaveResult Validate(const std::string& path) const;

    /// Get the header from the last successful Load().
    const SaveHeader& Header() const;

    /// Get the ECS data from the last successful Load().
    const std::vector<uint8_t>& ECSData() const;

    /// Get the auxiliary data from the last successful Load().
    const std::vector<uint8_t>& AuxData() const;

    /// Get the metadata string from the last successful Load().
    const std::string& Metadata() const;

    /// Clear loaded data.
    void Clear();

    /// Save a partial world (subset of chunks) to a file.
    SaveResult SavePartial(const std::string& path,
                           uint64_t tick,
                           uint32_t tickRate,
                           uint32_t seed,
                           const std::vector<ChunkSaveEntry>& chunks);

    /// Load a partial world save.
    SaveResult LoadPartial(const std::string& path);

    /// Get the partial save header from the last successful LoadPartial().
    const PartialSaveHeader& PartialHeader() const;

    /// Get the chunk entries from the last successful LoadPartial().
    const std::vector<ChunkSaveEntry>& Chunks() const;

private:
    SaveHeader m_header;
    std::vector<uint8_t> m_ecsData;
    std::vector<uint8_t> m_auxData;
    std::string m_metadata;
    PartialSaveHeader m_partialHeader;
    std::vector<ChunkSaveEntry> m_chunks;
};

}  // namespace atlas::sim
