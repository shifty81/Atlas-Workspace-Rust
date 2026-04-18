#pragma once
// ============================================================
// Atlas Deterministic Allocator
// ============================================================
//
// Arena-style allocator providing deterministic allocation with
// stable IDs and no pointer-order dependence. Guarantees that
// the same sequence of allocate/deallocate operations always
// produces the same memory layout, which is critical for
// deterministic simulation and replay.
//
// Uses monotonically increasing IDs (never reused) and a
// first-fit allocation strategy for full determinism.
//
// See: docs/ATLAS_CORE_CONTRACT.md

#include <cstdint>
#include <vector>

namespace atlas {

// Handle returned by Allocate(). An id of 0 indicates a failed or
// zero-size allocation and should be treated as invalid.
struct AllocHandle {
    uint64_t id;
    size_t offset;
    size_t size;
};

class DeterministicAllocator {
public:
    explicit DeterministicAllocator(size_t arenaSize);

    AllocHandle Allocate(size_t size);
    void Deallocate(const AllocHandle& handle);

    void* GetPointer(const AllocHandle& handle);
    const void* GetPointer(const AllocHandle& handle) const;
    bool IsValid(const AllocHandle& handle) const;

    size_t ArenaSize() const;
    size_t UsedBytes() const;
    size_t FreeBytes() const;
    size_t AllocationCount() const;
    uint64_t NextID() const;

    void Reset();
    size_t Defragment();

private:
    struct Block {
        size_t offset;
        size_t size;
        uint64_t id;
        bool alive;
    };

    std::vector<uint8_t> arena_;
    std::vector<Block> blocks_;
    uint64_t nextId_;
};

} // namespace atlas
