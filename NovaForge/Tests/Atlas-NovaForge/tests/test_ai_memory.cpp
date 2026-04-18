#include <cassert>
#include <iostream>
#include <memory>
#include "../engine/ai/AIMemory.h"

using namespace atlas::ai;

void test_ai_memory_store_recall() {
    AIMemory mem;
    mem.Store("quest.started", "true", 1.0f, 0.0f, 10);
    const MemoryEntry* entry = mem.Recall("quest.started");
    assert(entry != nullptr);
    assert(entry->value == "true");
    assert(entry->strength == 1.0f);
    std::cout << "[PASS] test_ai_memory_store_recall" << std::endl;
}

void test_ai_memory_has() {
    AIMemory mem;
    assert(!mem.Has("key"));
    mem.Store("key", "val", 1.0f, 0.0f, 0);
    assert(mem.Has("key"));
    std::cout << "[PASS] test_ai_memory_has" << std::endl;
}

void test_ai_memory_forget() {
    AIMemory mem;
    mem.Store("key", "val", 1.0f, 0.0f, 0);
    assert(mem.Has("key"));
    mem.Forget("key");
    assert(!mem.Has("key"));
    std::cout << "[PASS] test_ai_memory_forget" << std::endl;
}

void test_ai_memory_decay() {
    AIMemory mem;
    // Store with decay rate of 0.1 per tick
    mem.Store("temp", "data", 1.0f, 0.1f, 0);
    assert(mem.Has("temp"));

    // After 5 ticks: strength = 1.0 - 0.1 * 5 = 0.5
    mem.Tick(5);
    const MemoryEntry* entry = mem.Recall("temp");
    assert(entry != nullptr);
    assert(entry->strength > 0.49f && entry->strength < 0.51f);

    // After 10 more ticks (total 15): strength = 0.5 - 0.1 * 10 = -0.5 -> removed
    mem.Tick(15);
    assert(!mem.Has("temp"));

    std::cout << "[PASS] test_ai_memory_decay" << std::endl;
}

void test_ai_memory_permanent() {
    AIMemory mem;
    // Store with decay rate 0 (permanent)
    mem.Store("perm", "forever", 1.0f, 0.0f, 0);

    mem.Tick(1000);
    assert(mem.Has("perm"));
    const MemoryEntry* entry = mem.Recall("perm");
    assert(entry != nullptr);
    assert(entry->strength == 1.0f);

    std::cout << "[PASS] test_ai_memory_permanent" << std::endl;
}

void test_ai_memory_clear() {
    AIMemory mem;
    mem.Store("a", "1", 1.0f, 0.0f, 0);
    mem.Store("b", "2", 1.0f, 0.0f, 0);
    assert(mem.Count() == 2);
    mem.Clear();
    assert(mem.Count() == 0);
    std::cout << "[PASS] test_ai_memory_clear" << std::endl;
}
