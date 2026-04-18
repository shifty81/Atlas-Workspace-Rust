#include "../engine/physics/PhysicsWorld.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::physics;

void test_physics_create_body() {
    PhysicsWorld world;
    world.Init();

    BodyID id = world.CreateBody(1.0f, false);
    assert(id > 0);
    assert(world.BodyCount() == 1);

    auto* body = world.GetBody(id);
    assert(body != nullptr);
    assert(body->mass == 1.0f);
    assert(!body->isStatic);

    std::cout << "[PASS] test_physics_create_body" << std::endl;
}

void test_physics_destroy_body() {
    PhysicsWorld world;
    world.Init();

    BodyID id = world.CreateBody(1.0f);
    assert(world.BodyCount() == 1);

    world.DestroyBody(id);
    assert(world.BodyCount() == 0);
    assert(world.GetBody(id) == nullptr);

    std::cout << "[PASS] test_physics_destroy_body" << std::endl;
}

void test_physics_gravity() {
    PhysicsWorld world;
    world.Init();

    BodyID id = world.CreateBody(1.0f);
    world.SetPosition(id, 0, 10, 0);

    world.Step(1.0f);

    auto* body = world.GetBody(id);
    assert(body->position.y < 10.0f);
    assert(body->velocity.y < 0.0f);

    std::cout << "[PASS] test_physics_gravity" << std::endl;
}

void test_physics_static_body() {
    PhysicsWorld world;
    world.Init();

    BodyID id = world.CreateBody(1.0f, true);
    world.SetPosition(id, 0, 10, 0);

    world.Step(1.0f);

    auto* body = world.GetBody(id);
    assert(std::abs(body->position.y - 10.0f) < 0.001f);

    std::cout << "[PASS] test_physics_static_body" << std::endl;
}

void test_physics_apply_force() {
    PhysicsWorld world;
    world.Init();
    world.SetGravity(0, 0, 0);

    BodyID id = world.CreateBody(1.0f);
    world.SetPosition(id, 0, 0, 0);
    world.ApplyForce(id, 10.0f, 0, 0);

    world.Step(1.0f);

    auto* body = world.GetBody(id);
    assert(body->position.x > 0.0f);
    assert(body->velocity.x > 0.0f);

    std::cout << "[PASS] test_physics_apply_force" << std::endl;
}

void test_physics_collision_detection() {
    PhysicsWorld world;
    world.Init();
    world.SetGravity(0, 0, 0);

    BodyID a = world.CreateBody(1.0f);
    BodyID b = world.CreateBody(1.0f);
    world.SetPosition(a, 0, 0, 0);
    world.SetPosition(b, 0.1f, 0, 0);

    world.Step(0.01f);

    assert(world.GetCollisions().size() == 1);
    assert(world.GetCollisions()[0].a == a);
    assert(world.GetCollisions()[0].b == b);

    std::cout << "[PASS] test_physics_collision_detection" << std::endl;
}
