#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Physics/Physics.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("AABB intersection", "[Physics]") {
    NF::AABB a{{0, 0, 0}, {2, 2, 2}};
    NF::AABB b{{1, 1, 1}, {3, 3, 3}};
    NF::AABB c{{5, 5, 5}, {6, 6, 6}};

    REQUIRE(a.intersects(b));
    REQUIRE(b.intersects(a));
    REQUIRE_FALSE(a.intersects(c));
    REQUIRE_FALSE(c.intersects(a));
}

TEST_CASE("AABB touching edges intersect", "[Physics]") {
    NF::AABB a{{0, 0, 0}, {1, 1, 1}};
    NF::AABB b{{1, 0, 0}, {2, 1, 1}};

    REQUIRE(a.intersects(b)); // touching at x=1
}

TEST_CASE("AABB contains point", "[Physics]") {
    NF::AABB box{{0, 0, 0}, {10, 10, 10}};

    REQUIRE(box.contains({5, 5, 5}));
    REQUIRE(box.contains({0, 0, 0}));
    REQUIRE(box.contains({10, 10, 10}));
    REQUIRE_FALSE(box.contains({-1, 5, 5}));
    REQUIRE_FALSE(box.contains({5, 11, 5}));
}

TEST_CASE("AABB center and extents", "[Physics]") {
    NF::AABB box{{0, 0, 0}, {10, 20, 30}};

    auto center = box.center();
    REQUIRE_THAT(center.x, WithinAbs(5.f, 1e-5));
    REQUIRE_THAT(center.y, WithinAbs(10.f, 1e-5));
    REQUIRE_THAT(center.z, WithinAbs(15.f, 1e-5));

    auto ext = box.extents();
    REQUIRE_THAT(ext.x, WithinAbs(5.f, 1e-5));
    REQUIRE_THAT(ext.y, WithinAbs(10.f, 1e-5));
    REQUIRE_THAT(ext.z, WithinAbs(15.f, 1e-5));
}

TEST_CASE("AABB merge", "[Physics]") {
    NF::AABB a{{0, 0, 0}, {5, 5, 5}};
    NF::AABB b{{3, 3, 3}, {10, 10, 10}};

    auto merged = a.merged(b);
    REQUIRE(merged.min.x == 0.f);
    REQUIRE(merged.min.y == 0.f);
    REQUIRE(merged.max.x == 10.f);
    REQUIRE(merged.max.y == 10.f);
}

// ── Ray ──────────────────────────────────────────────────────────

TEST_CASE("Ray pointAt", "[Physics][Ray]") {
    NF::Ray ray{{0, 0, 0}, {1, 0, 0}};
    auto p = ray.pointAt(5.f);
    REQUIRE_THAT(p.x, WithinAbs(5.f, 1e-5));
    REQUIRE_THAT(p.y, WithinAbs(0.f, 1e-5));
    REQUIRE_THAT(p.z, WithinAbs(0.f, 1e-5));
}

TEST_CASE("Ray-AABB intersection hit", "[Physics][Ray]") {
    NF::Ray ray{{-5, 0.5f, 0.5f}, {1, 0, 0}};
    NF::AABB box{{0, 0, 0}, {1, 1, 1}};

    auto hit = NF::rayIntersectAABB(ray, box);
    REQUIRE(hit.has_value());
    REQUIRE_THAT(hit->distance, WithinAbs(5.f, 1e-4));
    REQUIRE_THAT(hit->point.x, WithinAbs(0.f, 1e-4));
}

TEST_CASE("Ray-AABB intersection miss", "[Physics][Ray]") {
    NF::Ray ray{{-5, 5, 5}, {1, 0, 0}};
    NF::AABB box{{0, 0, 0}, {1, 1, 1}};

    auto hit = NF::rayIntersectAABB(ray, box);
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Ray-AABB behind ray origin", "[Physics][Ray]") {
    NF::Ray ray{{5, 0.5f, 0.5f}, {1, 0, 0}};  // looking away from box
    NF::AABB box{{0, 0, 0}, {1, 1, 1}};

    auto hit = NF::rayIntersectAABB(ray, box);
    REQUIRE_FALSE(hit.has_value());
}

// ── Sphere ───────────────────────────────────────────────────────

TEST_CASE("Sphere contains point", "[Physics]") {
    NF::Sphere s{{0, 0, 0}, 5.f};

    REQUIRE(s.contains({0, 0, 0}));
    REQUIRE(s.contains({3, 0, 0}));
    REQUIRE(s.contains({0, 5, 0}));
    REQUIRE_FALSE(s.contains({6, 0, 0}));
}

TEST_CASE("Sphere intersection", "[Physics]") {
    NF::Sphere a{{0, 0, 0}, 5.f};
    NF::Sphere b{{8, 0, 0}, 5.f};
    NF::Sphere c{{20, 0, 0}, 2.f};

    REQUIRE(a.intersects(b)); // overlap
    REQUIRE_FALSE(a.intersects(c)); // too far
}

// ── RigidBody ────────────────────────────────────────────────────

TEST_CASE("RigidBody defaults", "[Physics][RigidBody]") {
    NF::RigidBody body;
    REQUIRE(body.type == NF::BodyType::Dynamic);
    REQUIRE(body.mass == 1.f);
    REQUIRE_THAT(body.inverseMass(), WithinAbs(1.f, 1e-5));
    REQUIRE(body.useGravity == true);
}

TEST_CASE("RigidBody static has zero inverse mass", "[Physics][RigidBody]") {
    NF::RigidBody body;
    body.type = NF::BodyType::Static;
    REQUIRE(body.inverseMass() == 0.f);
}

TEST_CASE("RigidBody integrate applies force", "[Physics][RigidBody]") {
    NF::RigidBody body;
    body.mass = 2.f;
    body.applyForce({10.f, 0.f, 0.f});

    body.integrate(1.f);
    // acceleration = force / mass = 5
    REQUIRE_THAT(body.velocity.x, WithinAbs(5.f, 1e-5));
    // Force should be cleared after integration
    REQUIRE(body.force.x == 0.f);
}

// ── Physics World ────────────────────────────────────────────────

TEST_CASE("PhysicsWorld step with gravity", "[Physics][World]") {
    NF::PhysicsWorld world;
    world.init();

    NF::RigidBody body;
    body.mass = 1.f;
    body.useGravity = true;
    size_t id = world.addBody(body);
    world.setPosition(id, {0.f, 10.f, 0.f});

    REQUIRE(world.bodyCount() == 1);

    // Step 1 second
    world.step(1.f);

    auto pos = world.getPosition(id);
    REQUIRE(pos.y < 10.f); // should have fallen

    world.shutdown();
}

TEST_CASE("PhysicsWorld static body doesn't move", "[Physics][World]") {
    NF::PhysicsWorld world;
    world.init();

    NF::RigidBody body;
    body.type = NF::BodyType::Static;
    size_t id = world.addBody(body);
    world.setPosition(id, {0.f, 5.f, 0.f});

    world.step(1.f);

    auto pos = world.getPosition(id);
    REQUIRE_THAT(pos.y, WithinAbs(5.f, 1e-5));

    world.shutdown();
}

// ── Collision Shapes ─────────────────────────────────────────────

TEST_CASE("BoxShape bounding box", "[Physics][Collision]") {
    NF::BoxShape box({2.f, 3.f, 4.f});

    REQUIRE(box.shapeType() == NF::CollisionShapeType::Box);

    auto bb = box.boundingBox();
    REQUIRE_THAT(bb.min.x, WithinAbs(-2.f, 1e-5));
    REQUIRE_THAT(bb.max.x, WithinAbs(2.f, 1e-5));
    REQUIRE_THAT(bb.min.y, WithinAbs(-3.f, 1e-5));
    REQUIRE_THAT(bb.max.y, WithinAbs(3.f, 1e-5));

    REQUIRE_THAT(box.halfExtents().x, WithinAbs(2.f, 1e-5));
}

TEST_CASE("SphereShape bounding box", "[Physics][Collision]") {
    NF::SphereShape sphere(5.f);

    REQUIRE(sphere.shapeType() == NF::CollisionShapeType::Sphere);
    REQUIRE_THAT(sphere.radius(), WithinAbs(5.f, 1e-5));

    auto bb = sphere.boundingBox();
    REQUIRE_THAT(bb.min.x, WithinAbs(-5.f, 1e-5));
    REQUIRE_THAT(bb.max.x, WithinAbs(5.f, 1e-5));
}

TEST_CASE("MeshCollisionShape add triangles and bounding box", "[Physics][Collision]") {
    NF::MeshCollisionShape mesh;

    REQUIRE(mesh.triangleCount() == 0);
    REQUIRE(mesh.isDirty());

    mesh.addTriangle({0, 0, 0}, {1, 0, 0}, {0, 1, 0});
    mesh.addTriangle({0, 0, 0}, {0, 0, 1}, {1, 0, 0});

    REQUIRE(mesh.triangleCount() == 2);
    REQUIRE(mesh.shapeType() == NF::CollisionShapeType::Mesh);

    auto bb = mesh.boundingBox();
    REQUIRE_THAT(bb.min.x, WithinAbs(0.f, 1e-5));
    REQUIRE_THAT(bb.max.x, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(bb.max.y, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(bb.max.z, WithinAbs(1.f, 1e-5));

    mesh.clearDirty();
    REQUIRE_FALSE(mesh.isDirty());

    mesh.clear();
    REQUIRE(mesh.triangleCount() == 0);
    REQUIRE(mesh.isDirty());
}
