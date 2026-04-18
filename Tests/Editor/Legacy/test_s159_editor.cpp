// S159 editor tests: ConstraintEditorV1, RigidBodyEditorV1, ColliderEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ColliderEditorV1.h"
#include "NF/Editor/RigidBodyEditorV1.h"
#include "NF/Editor/ConstraintEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ConstraintEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Cev1Constraint validity", "[Editor][S159]") {
    Cev1Constraint c;
    REQUIRE(!c.isValid());
    c.id = 1; c.label = "Hinge"; c.bodyA = 10;
    REQUIRE(c.isValid());
}

TEST_CASE("ConstraintEditorV1 addConstraint and constraintCount", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    REQUIRE(ce.constraintCount() == 0);
    Cev1Constraint c; c.id = 1; c.label = "Fixed"; c.bodyA = 1; c.bodyB = 2;
    REQUIRE(ce.addConstraint(c));
    REQUIRE(ce.constraintCount() == 1);
}

TEST_CASE("ConstraintEditorV1 addConstraint invalid fails", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c;
    REQUIRE(!ce.addConstraint(c));
}

TEST_CASE("ConstraintEditorV1 addConstraint duplicate id fails", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c; c.id = 1; c.label = "A"; c.bodyA = 1;
    ce.addConstraint(c);
    REQUIRE(!ce.addConstraint(c));
}

TEST_CASE("ConstraintEditorV1 removeConstraint", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c; c.id = 2; c.label = "Ball"; c.bodyA = 1;
    ce.addConstraint(c);
    REQUIRE(ce.removeConstraint(2));
    REQUIRE(ce.constraintCount() == 0);
    REQUIRE(!ce.removeConstraint(2));
}

TEST_CASE("ConstraintEditorV1 enableConstraint", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c; c.id = 3; c.label = "Slider"; c.bodyA = 1;
    ce.addConstraint(c);
    REQUIRE(ce.enableConstraint(3, false));
    auto* found = ce.findConstraint(3);
    REQUIRE(found != nullptr);
    REQUIRE(!found->enabled);
}

TEST_CASE("ConstraintEditorV1 enableConstraint unknown id fails", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    REQUIRE(!ce.enableConstraint(99, false));
}

TEST_CASE("ConstraintEditorV1 setLimit linear", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c; c.id = 4; c.label = "Spring"; c.bodyA = 1;
    ce.addConstraint(c);
    REQUIRE(ce.setLimit(4, false, -1.f, 1.f));
    auto* found = ce.findConstraint(4);
    REQUIRE(found != nullptr);
    REQUIRE(found->linear.enabled);
    REQUIRE(found->linear.lower == Approx(-1.f));
    REQUIRE(found->linear.upper == Approx(1.f));
}

TEST_CASE("ConstraintEditorV1 setLimit angular", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c; c.id = 5; c.label = "Hinge"; c.bodyA = 2;
    ce.addConstraint(c);
    REQUIRE(ce.setLimit(5, true, -90.f, 90.f));
    auto* found = ce.findConstraint(5);
    REQUIRE(found != nullptr);
    REQUIRE(found->angular.enabled);
}

TEST_CASE("ConstraintEditorV1 setLimit unknown id fails", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    REQUIRE(!ce.setLimit(99, false, 0.f, 1.f));
}

TEST_CASE("ConstraintEditorV1 onChange fires on enableConstraint", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    Cev1Constraint c; c.id = 6; c.label = "Distance"; c.bodyA = 3;
    ce.addConstraint(c);
    uint64_t changedId = 0;
    ce.setOnChange([&](uint64_t id){ changedId = id; });
    ce.enableConstraint(6, false);
    REQUIRE(changedId == 6);
}

TEST_CASE("ConstraintEditorV1 findConstraint unknown returns null", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    REQUIRE(ce.findConstraint(999) == nullptr);
}

// ── RigidBodyEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Rbv1Body validity", "[Editor][S159]") {
    Rbv1Body b;
    REQUIRE(!b.isValid());
    b.id = 1; b.label = "Box"; b.mass = 1.f;
    REQUIRE(b.isValid());
}

TEST_CASE("Rbv1Body invalid zero mass fails", "[Editor][S159]") {
    Rbv1Body b;
    b.id = 1; b.label = "Box"; b.mass = 0.f;
    REQUIRE(!b.isValid());
}

TEST_CASE("RigidBodyEditorV1 addBody and bodyCount", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    REQUIRE(rbe.bodyCount() == 0);
    Rbv1Body b; b.id = 1; b.label = "Crate"; b.mass = 5.f;
    REQUIRE(rbe.addBody(b));
    REQUIRE(rbe.bodyCount() == 1);
}

TEST_CASE("RigidBodyEditorV1 addBody invalid fails", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b;
    REQUIRE(!rbe.addBody(b));
}

TEST_CASE("RigidBodyEditorV1 addBody duplicate id fails", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 1; b.label = "A"; b.mass = 1.f;
    rbe.addBody(b);
    REQUIRE(!rbe.addBody(b));
}

TEST_CASE("RigidBodyEditorV1 removeBody", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 2; b.label = "Barrel"; b.mass = 10.f;
    rbe.addBody(b);
    REQUIRE(rbe.removeBody(2));
    REQUIRE(rbe.bodyCount() == 0);
    REQUIRE(!rbe.removeBody(2));
}

TEST_CASE("RigidBodyEditorV1 setMass", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 3; b.label = "Rock"; b.mass = 1.f;
    rbe.addBody(b);
    REQUIRE(rbe.setMass(3, 20.f));
    auto* found = rbe.findBody(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->mass == Approx(20.f));
}

TEST_CASE("RigidBodyEditorV1 setMass zero or negative fails", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 4; b.label = "Plank"; b.mass = 2.f;
    rbe.addBody(b);
    REQUIRE(!rbe.setMass(4, 0.f));
    REQUIRE(!rbe.setMass(4, -5.f));
}

TEST_CASE("RigidBodyEditorV1 setMass unknown id fails", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    REQUIRE(!rbe.setMass(99, 1.f));
}

TEST_CASE("RigidBodyEditorV1 setBodyType", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 5; b.label = "Floor"; b.mass = 1.f;
    rbe.addBody(b);
    REQUIRE(rbe.setBodyType(5, Rbv1BodyType::Static));
    auto* found = rbe.findBody(5);
    REQUIRE(found != nullptr);
    REQUIRE(found->type == Rbv1BodyType::Static);
}

TEST_CASE("RigidBodyEditorV1 setBodyType unknown id fails", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    REQUIRE(!rbe.setBodyType(99, Rbv1BodyType::Kinematic));
}

TEST_CASE("RigidBodyEditorV1 setMaterial", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 6; b.label = "Ball"; b.mass = 1.f;
    rbe.addBody(b);
    Rbv1PhysicsMat mat; mat.friction = 0.9f; mat.restitution = 0.1f;
    REQUIRE(rbe.setMaterial(6, mat));
    auto* found = rbe.findBody(6);
    REQUIRE(found != nullptr);
    REQUIRE(found->material.friction == Approx(0.9f));
}

TEST_CASE("RigidBodyEditorV1 onChange fires on setMass", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    Rbv1Body b; b.id = 7; b.label = "Box2"; b.mass = 1.f;
    rbe.addBody(b);
    uint64_t changedId = 0;
    rbe.setOnChange([&](uint64_t id){ changedId = id; });
    rbe.setMass(7, 3.f);
    REQUIRE(changedId == 7);
}

TEST_CASE("RigidBodyEditorV1 findBody unknown returns null", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    REQUIRE(rbe.findBody(999) == nullptr);
}

// ── ColliderEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Cov1Collider validity", "[Editor][S159]") {
    Cov1Collider c;
    REQUIRE(!c.isValid());
    c.id = 1; c.label = "BoxCollider";
    REQUIRE(c.isValid());
}

TEST_CASE("ColliderEditorV1 addCollider and colliderCount", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(ce.colliderCount() == 0);
    Cov1Collider c; c.id = 1; c.label = "Box";
    REQUIRE(ce.addCollider(c));
    REQUIRE(ce.colliderCount() == 1);
}

TEST_CASE("ColliderEditorV1 addCollider invalid fails", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c;
    REQUIRE(!ce.addCollider(c));
}

TEST_CASE("ColliderEditorV1 addCollider duplicate id fails", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 1; c.label = "A";
    ce.addCollider(c);
    REQUIRE(!ce.addCollider(c));
}

TEST_CASE("ColliderEditorV1 removeCollider", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 2; c.label = "Sphere";
    ce.addCollider(c);
    REQUIRE(ce.removeCollider(2));
    REQUIRE(ce.colliderCount() == 0);
    REQUIRE(!ce.removeCollider(2));
}

TEST_CASE("ColliderEditorV1 setShape", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 3; c.label = "Capsule";
    ce.addCollider(c);
    REQUIRE(ce.setShape(3, Cov1ColliderShape::Capsule));
    auto* found = ce.findCollider(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->shape == Cov1ColliderShape::Capsule);
}

TEST_CASE("ColliderEditorV1 setShape unknown id fails", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(!ce.setShape(99, Cov1ColliderShape::Sphere));
}

TEST_CASE("ColliderEditorV1 setMode trigger", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 4; c.label = "Zone";
    ce.addCollider(c);
    REQUIRE(ce.setMode(4, Cov1ColliderMode::Trigger));
    auto* found = ce.findCollider(4);
    REQUIRE(found != nullptr);
    REQUIRE(found->mode == Cov1ColliderMode::Trigger);
}

TEST_CASE("ColliderEditorV1 setMode unknown id fails", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(!ce.setMode(99, Cov1ColliderMode::Solid));
}

TEST_CASE("ColliderEditorV1 setLayer", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 5; c.label = "Ground";
    ce.addCollider(c);
    REQUIRE(ce.setLayer(5, 2, 0xFF));
    auto* found = ce.findCollider(5);
    REQUIRE(found != nullptr);
    REQUIRE(found->layer == 2);
    REQUIRE(found->mask == 0xFF);
}

TEST_CASE("ColliderEditorV1 setLayer unknown id fails", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(!ce.setLayer(99, 1, 0xFF));
}

TEST_CASE("ColliderEditorV1 enableCollider", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 6; c.label = "Wall";
    ce.addCollider(c);
    REQUIRE(ce.enableCollider(6, false));
    auto* found = ce.findCollider(6);
    REQUIRE(found != nullptr);
    REQUIRE(!found->enabled);
}

TEST_CASE("ColliderEditorV1 enableCollider unknown id fails", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(!ce.enableCollider(99, false));
}

TEST_CASE("ColliderEditorV1 onChange fires on setShape", "[Editor][S159]") {
    ColliderEditorV1 ce;
    Cov1Collider c; c.id = 7; c.label = "Mesh";
    ce.addCollider(c);
    uint64_t changedId = 0;
    ce.setOnChange([&](uint64_t id){ changedId = id; });
    ce.setShape(7, Cov1ColliderShape::Mesh);
    REQUIRE(changedId == 7);
}

TEST_CASE("ColliderEditorV1 findCollider unknown returns null", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(ce.findCollider(999) == nullptr);
}
