#include "DeterministicAllocator.h"
#include <algorithm>
#include <cstring>

namespace atlas {

DeterministicAllocator::DeterministicAllocator(size_t arenaSize)
    : arena_(arenaSize, 0), nextId_(1) {}

AllocHandle DeterministicAllocator::Allocate(size_t size) {
    if (size == 0) {
        return {0, 0, 0};
    }

    // First-fit: scan for a gap large enough in offset-sorted order
    // Build a sorted view of alive blocks by offset
    std::vector<const Block*> sorted;
    for (const auto& b : blocks_) {
        if (b.alive) {
            sorted.push_back(&b);
        }
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const Block* a, const Block* b) { return a->offset < b->offset; });

    size_t candidate = 0;
    for (const auto* blk : sorted) {
        if (candidate + size <= blk->offset) {
            break; // gap found before this block
        }
        candidate = blk->offset + blk->size;
    }

    if (candidate + size > arena_.size()) {
        return {0, 0, 0}; // out of memory
    }

    uint64_t id = nextId_++;
    blocks_.push_back({candidate, size, id, true});

    return {id, candidate, size};
}

void DeterministicAllocator::Deallocate(const AllocHandle& handle) {
    for (auto& b : blocks_) {
        if (b.id == handle.id && b.alive) {
            b.alive = false;
            return;
        }
    }
}

void* DeterministicAllocator::GetPointer(const AllocHandle& handle) {
    for (const auto& b : blocks_) {
        if (b.id == handle.id && b.alive) {
            return arena_.data() + b.offset;
        }
    }
    return nullptr;
}

const void* DeterministicAllocator::GetPointer(const AllocHandle& handle) const {
    for (const auto& b : blocks_) {
        if (b.id == handle.id && b.alive) {
            return arena_.data() + b.offset;
        }
    }
    return nullptr;
}

bool DeterministicAllocator::IsValid(const AllocHandle& handle) const {
    for (const auto& b : blocks_) {
        if (b.id == handle.id && b.alive) {
            return true;
        }
    }
    return false;
}

size_t DeterministicAllocator::ArenaSize() const {
    return arena_.size();
}

size_t DeterministicAllocator::UsedBytes() const {
    size_t used = 0;
    for (const auto& b : blocks_) {
        if (b.alive) {
            used += b.size;
        }
    }
    return used;
}

size_t DeterministicAllocator::FreeBytes() const {
    return arena_.size() - UsedBytes();
}

size_t DeterministicAllocator::AllocationCount() const {
    size_t count = 0;
    for (const auto& b : blocks_) {
        if (b.alive) {
            ++count;
        }
    }
    return count;
}

uint64_t DeterministicAllocator::NextID() const {
    return nextId_;
}

void DeterministicAllocator::Reset() {
    blocks_.clear();
    nextId_ = 1;
    std::fill(arena_.begin(), arena_.end(), 0);
}

size_t DeterministicAllocator::Defragment() {
    // Remove dead blocks
    blocks_.erase(
        std::remove_if(blocks_.begin(), blocks_.end(),
                       [](const Block& b) { return !b.alive; }),
        blocks_.end());

    // Sort alive blocks by offset
    std::sort(blocks_.begin(), blocks_.end(),
              [](const Block& a, const Block& b) { return a.offset < b.offset; });

    size_t recovered = 0;
    size_t writePos = 0;

    for (auto& b : blocks_) {
        if (b.offset != writePos) {
            size_t gap = b.offset - writePos;
            recovered += gap;
            std::memmove(arena_.data() + writePos, arena_.data() + b.offset, b.size);
            b.offset = writePos;
        }
        writePos = b.offset + b.size;
    }

    // Zero out the reclaimed tail
    if (writePos < arena_.size()) {
        std::memset(arena_.data() + writePos, 0, arena_.size() - writePos);
    }

    return recovered;
}

} // namespace atlas
