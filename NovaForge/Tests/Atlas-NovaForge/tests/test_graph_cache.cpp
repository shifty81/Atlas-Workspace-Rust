#include <iostream>
#include <cassert>
#include "graphvm/GraphCache.h"

void test_cache_store_and_get() {
    atlas::graphvm::GraphCache cache;
    atlas::graphvm::CacheEntry entry{42, {1.0f, 2.0f, 3.0f}, 10};
    cache.Store(100, entry);

    const auto* result = cache.Get(100);
    assert(result != nullptr);
    assert(result->hash == 42);
    assert(result->data.size() == 3);
    assert(result->data[0] == 1.0f);
    assert(result->tick == 10);
    std::cout << "  PASS: test_cache_store_and_get" << std::endl;
}

void test_cache_has() {
    atlas::graphvm::GraphCache cache;
    assert(!cache.Has(1));
    cache.Store(1, {0, {}, 0});
    assert(cache.Has(1));
    assert(!cache.Has(2));
    std::cout << "  PASS: test_cache_has" << std::endl;
}

void test_cache_invalidate() {
    atlas::graphvm::GraphCache cache;
    cache.Store(5, {0, {1.0f}, 1});
    assert(cache.Has(5));
    cache.Invalidate(5);
    assert(!cache.Has(5));
    assert(cache.Get(5) == nullptr);
    std::cout << "  PASS: test_cache_invalidate" << std::endl;
}

void test_cache_invalidate_all() {
    atlas::graphvm::GraphCache cache;
    cache.Store(1, {0, {}, 0});
    cache.Store(2, {0, {}, 0});
    cache.Store(3, {0, {}, 0});
    assert(cache.Size() == 3);
    cache.InvalidateAll();
    assert(cache.Size() == 0);
    std::cout << "  PASS: test_cache_invalidate_all" << std::endl;
}

void test_cache_evict_before() {
    atlas::graphvm::GraphCache cache;
    cache.Store(1, {0, {}, 5});
    cache.Store(2, {0, {}, 10});
    cache.Store(3, {0, {}, 15});
    cache.EvictBefore(10);
    assert(!cache.Has(1));
    assert(cache.Has(2));
    assert(cache.Has(3));
    assert(cache.Size() == 2);
    std::cout << "  PASS: test_cache_evict_before" << std::endl;
}

void test_cache_hash_key() {
    uint64_t a = atlas::graphvm::GraphCache::HashKey(1, 100, 0);
    uint64_t b = atlas::graphvm::GraphCache::HashKey(1, 100, 0);
    assert(a == b);

    uint64_t c = atlas::graphvm::GraphCache::HashKey(2, 100, 0);
    assert(a != c);

    uint64_t d = atlas::graphvm::GraphCache::HashKey(1, 200, 0);
    assert(a != d);

    uint64_t e = atlas::graphvm::GraphCache::HashKey(1, 100, 1);
    assert(a != e);
    std::cout << "  PASS: test_cache_hash_key" << std::endl;
}
