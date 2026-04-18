#include "../engine/sim/WorldStateSerializer.h"
#include "../engine/sim/StateHasher.h"
#include <iostream>
#include <cassert>

using namespace atlas::sim;

void test_serializer_default_version() {
    WorldStateSerializer ws;
    assert(ws.CurrentVersion().major == 1);
    assert(ws.CurrentVersion().minor == 0);
    assert(ws.MinimumVersion().major == 1);
    assert(ws.MinimumVersion().minor == 0);

    std::cout << "[PASS] test_serializer_default_version" << std::endl;
}

void test_serializer_set_version() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({2, 3});
    assert(ws.CurrentVersion().major == 2);
    assert(ws.CurrentVersion().minor == 3);

    ws.SetMinimumVersion({1, 5});
    assert(ws.MinimumVersion().major == 1);
    assert(ws.MinimumVersion().minor == 5);

    std::cout << "[PASS] test_serializer_set_version" << std::endl;
}

void test_serializer_serialize_roundtrip() {
    WorldStateSerializer ws;
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};

    SerializedState state = ws.Serialize(data);
    assert(state.version == ws.CurrentVersion());
    assert(state.data == data);
    assert(state.hash != 0);

    SerializerResult res = ws.Deserialize(state);
    assert(res == SerializerResult::Success);
    assert(state.data == data);

    std::cout << "[PASS] test_serializer_serialize_roundtrip" << std::endl;
}

void test_serializer_serialize_empty() {
    WorldStateSerializer ws;
    std::vector<uint8_t> empty;

    SerializedState state = ws.Serialize(empty);
    assert(state.data.empty());
    assert(state.hash == 0);

    SerializerResult res = ws.Deserialize(state);
    assert(res == SerializerResult::Success);

    std::cout << "[PASS] test_serializer_serialize_empty" << std::endl;
}

void test_serializer_hash_mismatch() {
    WorldStateSerializer ws;
    std::vector<uint8_t> data = {1, 2, 3};

    SerializedState state = ws.Serialize(data);
    state.data[0] = 99;  // Corrupt data

    SerializerResult res = ws.Validate(state);
    assert(res == SerializerResult::HashMismatch);

    std::cout << "[PASS] test_serializer_hash_mismatch" << std::endl;
}

void test_serializer_version_too_old() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({3, 0});
    ws.SetMinimumVersion({2, 0});

    SerializedState state;
    state.version = {1, 0};
    state.hash = 0;

    SerializerResult res = ws.Validate(state);
    assert(res == SerializerResult::VersionTooOld);

    std::cout << "[PASS] test_serializer_version_too_old" << std::endl;
}

void test_serializer_version_too_new() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({2, 0});

    SerializedState state;
    state.version = {3, 0};
    state.hash = 0;

    SerializerResult res = ws.Validate(state);
    assert(res == SerializerResult::VersionTooNew);

    std::cout << "[PASS] test_serializer_version_too_new" << std::endl;
}

void test_serializer_migration() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({1, 2});
    ws.SetMinimumVersion({1, 0});

    // Register migration: v1.0 → v1.1 (append byte 0xAA)
    ws.RegisterMigration({{1, 0}, {1, 1}, "Add field A",
        [](std::vector<uint8_t>& d) -> bool {
            d.push_back(0xAA);
            return true;
        }});

    // Register migration: v1.1 → v1.2 (append byte 0xBB)
    ws.RegisterMigration({{1, 1}, {1, 2}, "Add field B",
        [](std::vector<uint8_t>& d) -> bool {
            d.push_back(0xBB);
            return true;
        }});

    assert(ws.MigrationCount() == 2);

    // Serialize data at v1.0 and migrate
    std::vector<uint8_t> data = {1, 2, 3};
    SerializedState state = ws.Serialize(data);
    state.version = {1, 0};
    // Recompute hash for v1.0 data
    state.hash = StateHasher::HashCombine(0, state.data.data(), state.data.size());

    SerializerResult res = ws.Deserialize(state);
    assert(res == SerializerResult::Success);
    assert(state.version == ws.CurrentVersion());
    assert(state.data.size() == 5);  // 3 original + 0xAA + 0xBB
    assert(state.data[3] == 0xAA);
    assert(state.data[4] == 0xBB);

    std::cout << "[PASS] test_serializer_migration" << std::endl;
}

void test_serializer_can_migrate() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({1, 2});
    ws.SetMinimumVersion({1, 0});

    ws.RegisterMigration({{1, 0}, {1, 1}, "Step 1",
        [](std::vector<uint8_t>&) { return true; }});
    ws.RegisterMigration({{1, 1}, {1, 2}, "Step 2",
        [](std::vector<uint8_t>&) { return true; }});

    assert(ws.CanMigrate({1, 0}));
    assert(ws.CanMigrate({1, 1}));
    assert(ws.CanMigrate({1, 2}));

    // No path from {0, 9}
    assert(!ws.CanMigrate({0, 9}));

    std::cout << "[PASS] test_serializer_can_migrate" << std::endl;
}

void test_serializer_migration_path() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({1, 3});

    ws.RegisterMigration({{1, 0}, {1, 1}, "A", [](std::vector<uint8_t>&) { return true; }});
    ws.RegisterMigration({{1, 1}, {1, 2}, "B", [](std::vector<uint8_t>&) { return true; }});
    ws.RegisterMigration({{1, 2}, {1, 3}, "C", [](std::vector<uint8_t>&) { return true; }});

    auto path = ws.MigrationPath({1, 0});
    assert(path.size() == 3);
    assert(path[0].description == "A");
    assert(path[1].description == "B");
    assert(path[2].description == "C");

    auto path2 = ws.MigrationPath({1, 2});
    assert(path2.size() == 1);

    std::cout << "[PASS] test_serializer_migration_path" << std::endl;
}

void test_serializer_migration_failure() {
    WorldStateSerializer ws;
    ws.SetCurrentVersion({1, 1});
    ws.SetMinimumVersion({1, 0});

    ws.RegisterMigration({{1, 0}, {1, 1}, "Fail step",
        [](std::vector<uint8_t>&) -> bool { return false; }});

    std::vector<uint8_t> data = {1, 2, 3};
    SerializedState state = ws.Serialize(data);
    state.version = {1, 0};
    state.hash = StateHasher::HashCombine(0, state.data.data(), state.data.size());

    SerializerResult res = ws.Deserialize(state);
    assert(res == SerializerResult::MigrationFailed);

    std::cout << "[PASS] test_serializer_migration_failure" << std::endl;
}

void test_serializer_schema_version_comparison() {
    SchemaVersion a{1, 0};
    SchemaVersion b{1, 1};
    SchemaVersion c{2, 0};

    assert(a < b);
    assert(b < c);
    assert(a < c);
    assert(!(b < a));
    assert(a <= a);
    assert(a <= b);
    assert(a == a);
    assert(a != b);

    std::cout << "[PASS] test_serializer_schema_version_comparison" << std::endl;
}
