#include "../engine/ecs/ECS.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace atlas::ecs;

struct TestPosition {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct TestVelocity {
    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;
};

struct TestHealth {
    int current = 100;
    int max = 100;
};

void test_ecs_serialize_empty_world() {
    World world;
    world.RegisterComponent<TestPosition>(1);

    auto data = world.Serialize();
    assert(!data.empty());

    World world2;
    world2.RegisterComponent<TestPosition>(1);
    assert(world2.Deserialize(data));
    assert(world2.EntityCount() == 0);

    std::cout << "[PASS] test_ecs_serialize_empty_world" << std::endl;
}

void test_ecs_serialize_entities_no_components() {
    World world;
    world.RegisterComponent<TestPosition>(1);
    world.CreateEntity();
    world.CreateEntity();

    auto data = world.Serialize();

    World world2;
    world2.RegisterComponent<TestPosition>(1);
    assert(world2.Deserialize(data));
    assert(world2.EntityCount() == 2);

    std::cout << "[PASS] test_ecs_serialize_entities_no_components" << std::endl;
}

void test_ecs_serialize_roundtrip() {
    World world;
    world.RegisterComponent<TestPosition>(1);
    world.RegisterComponent<TestVelocity>(2);

    EntityID e1 = world.CreateEntity();
    EntityID e2 = world.CreateEntity();

    world.AddComponent<TestPosition>(e1, {1.0f, 2.0f, 3.0f});
    world.AddComponent<TestVelocity>(e1, {0.1f, 0.2f, 0.3f});
    world.AddComponent<TestPosition>(e2, {4.0f, 5.0f, 6.0f});

    auto data = world.Serialize();

    World world2;
    world2.RegisterComponent<TestPosition>(1);
    world2.RegisterComponent<TestVelocity>(2);
    assert(world2.Deserialize(data));

    assert(world2.EntityCount() == 2);
    assert(world2.IsAlive(e1));
    assert(world2.IsAlive(e2));

    auto* p1 = world2.GetComponent<TestPosition>(e1);
    assert(p1 != nullptr);
    assert(p1->x == 1.0f);
    assert(p1->y == 2.0f);
    assert(p1->z == 3.0f);

    auto* v1 = world2.GetComponent<TestVelocity>(e1);
    assert(v1 != nullptr);
    assert(v1->dx == 0.1f);

    auto* p2 = world2.GetComponent<TestPosition>(e2);
    assert(p2 != nullptr);
    assert(p2->x == 4.0f);

    assert(!world2.HasComponent<TestVelocity>(e2));

    std::cout << "[PASS] test_ecs_serialize_roundtrip" << std::endl;
}

void test_ecs_serialize_preserves_next_id() {
    World world;
    world.RegisterComponent<TestPosition>(1);
    world.CreateEntity(); // id 1
    world.CreateEntity(); // id 2
    world.CreateEntity(); // id 3

    auto data = world.Serialize();

    World world2;
    world2.RegisterComponent<TestPosition>(1);
    assert(world2.Deserialize(data));

    // Next created entity should have id 4
    EntityID e = world2.CreateEntity();
    assert(e == 4);

    std::cout << "[PASS] test_ecs_serialize_preserves_next_id" << std::endl;
}

void test_ecs_deserialize_clears_existing() {
    World world;
    world.RegisterComponent<TestPosition>(1);
    world.CreateEntity();

    auto data = world.Serialize();

    World world2;
    world2.RegisterComponent<TestPosition>(1);
    // Pre-populate with different data
    world2.CreateEntity();
    world2.CreateEntity();
    world2.CreateEntity();
    assert(world2.EntityCount() == 3);

    assert(world2.Deserialize(data));
    assert(world2.EntityCount() == 1);

    std::cout << "[PASS] test_ecs_deserialize_clears_existing" << std::endl;
}

void test_ecs_deserialize_invalid_data() {
    World world;
    world.RegisterComponent<TestPosition>(1);

    // Too small
    std::vector<uint8_t> empty;
    assert(!world.Deserialize(empty));

    std::vector<uint8_t> small = {0, 1, 2};
    assert(!world.Deserialize(small));

    std::cout << "[PASS] test_ecs_deserialize_invalid_data" << std::endl;
}

void test_ecs_has_serializer() {
    World world;
    auto key = std::type_index(typeid(TestPosition));
    assert(!world.HasSerializer(key));

    world.RegisterComponent<TestPosition>(42);
    assert(world.HasSerializer(key));
    assert(world.GetTypeTag(key) == 42);

    std::cout << "[PASS] test_ecs_has_serializer" << std::endl;
}
