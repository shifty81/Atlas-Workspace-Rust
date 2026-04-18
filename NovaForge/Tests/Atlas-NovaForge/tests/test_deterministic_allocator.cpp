#include "../engine/core/DeterministicAllocator.h"
#include <iostream>
#include <cassert>

using namespace atlas;

void test_allocator_create() {
    DeterministicAllocator alloc(1024);
    assert(alloc.ArenaSize() == 1024);
    assert(alloc.UsedBytes() == 0);
    assert(alloc.FreeBytes() == 1024);
    assert(alloc.AllocationCount() == 0);
    assert(alloc.NextID() == 1);
    std::cout << "[PASS] test_allocator_create" << std::endl;
}

void test_allocator_single_alloc() {
    DeterministicAllocator alloc(1024);
    AllocHandle h = alloc.Allocate(64);
    assert(h.id == 1);
    assert(h.size == 64);
    assert(alloc.IsValid(h));
    assert(alloc.GetPointer(h) != nullptr);
    assert(alloc.UsedBytes() == 64);
    assert(alloc.AllocationCount() == 1);
    std::cout << "[PASS] test_allocator_single_alloc" << std::endl;
}

void test_allocator_multiple_allocs() {
    DeterministicAllocator alloc(1024);
    AllocHandle h1 = alloc.Allocate(64);
    AllocHandle h2 = alloc.Allocate(128);
    AllocHandle h3 = alloc.Allocate(32);
    assert(alloc.IsValid(h1));
    assert(alloc.IsValid(h2));
    assert(alloc.IsValid(h3));
    assert(alloc.AllocationCount() == 3);
    assert(alloc.UsedBytes() == 64 + 128 + 32);
    // Pointers must not overlap
    assert(h1.offset + h1.size <= h2.offset);
    assert(h2.offset + h2.size <= h3.offset);
    std::cout << "[PASS] test_allocator_multiple_allocs" << std::endl;
}

void test_allocator_dealloc() {
    DeterministicAllocator alloc(1024);
    AllocHandle h = alloc.Allocate(128);
    assert(alloc.IsValid(h));
    alloc.Deallocate(h);
    assert(!alloc.IsValid(h));
    assert(alloc.GetPointer(h) == nullptr);
    assert(alloc.AllocationCount() == 0);
    assert(alloc.UsedBytes() == 0);
    std::cout << "[PASS] test_allocator_dealloc" << std::endl;
}

void test_allocator_reuse_space() {
    DeterministicAllocator alloc(256);
    AllocHandle h1 = alloc.Allocate(64);
    AllocHandle h2 = alloc.Allocate(64);
    alloc.Deallocate(h1);
    // Next allocation should reuse the freed space at offset 0
    AllocHandle h3 = alloc.Allocate(32);
    assert(h3.offset == 0);
    assert(alloc.IsValid(h3));
    assert(alloc.IsValid(h2));
    std::cout << "[PASS] test_allocator_reuse_space" << std::endl;
}

void test_allocator_stable_ids() {
    DeterministicAllocator alloc(1024);
    AllocHandle h1 = alloc.Allocate(32);
    AllocHandle h2 = alloc.Allocate(32);
    alloc.Deallocate(h1);
    AllocHandle h3 = alloc.Allocate(32);
    // IDs must always increase, never reuse
    assert(h1.id == 1);
    assert(h2.id == 2);
    assert(h3.id == 3);
    assert(alloc.NextID() == 4);
    std::cout << "[PASS] test_allocator_stable_ids" << std::endl;
}

void test_allocator_reset() {
    DeterministicAllocator alloc(512);
    alloc.Allocate(64);
    alloc.Allocate(128);
    alloc.Reset();
    assert(alloc.UsedBytes() == 0);
    assert(alloc.FreeBytes() == 512);
    assert(alloc.AllocationCount() == 0);
    assert(alloc.NextID() == 1);
    std::cout << "[PASS] test_allocator_reset" << std::endl;
}

void test_allocator_deterministic() {
    // Two allocators with the same operations must produce identical state
    auto run_sequence = [](DeterministicAllocator& a) {
        AllocHandle h1 = a.Allocate(64);
        AllocHandle h2 = a.Allocate(128);
        a.Deallocate(h1);
        AllocHandle h3 = a.Allocate(32);
        AllocHandle h4 = a.Allocate(64);
        return std::vector<AllocHandle>{h1, h2, h3, h4};
    };

    DeterministicAllocator a(1024);
    DeterministicAllocator b(1024);
    auto ha = run_sequence(a);
    auto hb = run_sequence(b);

    assert(ha.size() == hb.size());
    for (size_t i = 0; i < ha.size(); ++i) {
        assert(ha[i].id == hb[i].id);
        assert(ha[i].offset == hb[i].offset);
        assert(ha[i].size == hb[i].size);
    }
    assert(a.UsedBytes() == b.UsedBytes());
    assert(a.AllocationCount() == b.AllocationCount());
    assert(a.NextID() == b.NextID());
    std::cout << "[PASS] test_allocator_deterministic" << std::endl;
}

