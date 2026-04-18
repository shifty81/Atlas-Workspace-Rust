// S197 editor tests: PhysicsJointEditorV1, DestructionEditorV1, WindFieldEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PhysicsJointEditorV1.h"
#include "NF/Editor/DestructionEditorV1.h"
#include "NF/Editor/WindFieldEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── PhysicsJointEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Pjv1Joint validity", "[Editor][S197]") {
    Pjv1Joint j;
    REQUIRE(!j.isValid());
    j.id = 1; j.name = "Hinge1"; j.bodyA = 10; j.bodyB = 20;
    REQUIRE(j.isValid());
}

TEST_CASE("Pjv1Joint isBreakable", "[Editor][S197]") {
    Pjv1Joint j;
    j.breakForce = -1.f;
    REQUIRE(!j.isBreakable());
    j.breakForce = 100.f;
    REQUIRE(j.isBreakable());
}

TEST_CASE("PhysicsJointEditorV1 addJoint and jointCount", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    REQUIRE(pj.jointCount() == 0);
    Pjv1Joint j; j.id = 1; j.name = "J1"; j.bodyA = 1; j.bodyB = 2;
    REQUIRE(pj.addJoint(j));
    REQUIRE(pj.jointCount() == 1);
}

TEST_CASE("PhysicsJointEditorV1 addJoint invalid fails", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    REQUIRE(!pj.addJoint(Pjv1Joint{}));
}

TEST_CASE("PhysicsJointEditorV1 addJoint duplicate fails", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j; j.id = 1; j.name = "A"; j.bodyA = 1; j.bodyB = 2;
    pj.addJoint(j);
    REQUIRE(!pj.addJoint(j));
}

TEST_CASE("PhysicsJointEditorV1 removeJoint", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j; j.id = 2; j.name = "B"; j.bodyA = 1; j.bodyB = 2;
    pj.addJoint(j);
    REQUIRE(pj.removeJoint(2));
    REQUIRE(pj.jointCount() == 0);
    REQUIRE(!pj.removeJoint(2));
}

TEST_CASE("PhysicsJointEditorV1 findJoint", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j; j.id = 3; j.name = "C"; j.bodyA = 1; j.bodyB = 2;
    pj.addJoint(j);
    REQUIRE(pj.findJoint(3) != nullptr);
    REQUIRE(pj.findJoint(99) == nullptr);
}

TEST_CASE("PhysicsJointEditorV1 setState", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j; j.id = 1; j.name = "J"; j.bodyA = 1; j.bodyB = 2;
    pj.addJoint(j);
    REQUIRE(pj.setState(1, Pjv1JointState::Broken));
    REQUIRE(pj.findJoint(1)->state == Pjv1JointState::Broken);
}

TEST_CASE("PhysicsJointEditorV1 setBreakForce", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j; j.id = 1; j.name = "J"; j.bodyA = 1; j.bodyB = 2;
    pj.addJoint(j);
    REQUIRE(pj.setBreakForce(1, 50.f));
    REQUIRE(pj.findJoint(1)->breakForce == Approx(50.f));
    REQUIRE(pj.findJoint(1)->isBreakable());
}

TEST_CASE("PhysicsJointEditorV1 activeCount", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j1; j1.id = 1; j1.name = "A"; j1.bodyA = 1; j1.bodyB = 2;
    Pjv1Joint j2; j2.id = 2; j2.name = "B"; j2.bodyA = 3; j2.bodyB = 4;
    pj.addJoint(j1); pj.addJoint(j2);
    pj.setState(2, Pjv1JointState::Disabled);
    REQUIRE(pj.activeCount() == 1);
}

TEST_CASE("PhysicsJointEditorV1 countByType", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j1; j1.id = 1; j1.name = "A"; j1.bodyA = 1; j1.bodyB = 2; j1.type = Pjv1JointType::Hinge;
    Pjv1Joint j2; j2.id = 2; j2.name = "B"; j2.bodyA = 3; j2.bodyB = 4; j2.type = Pjv1JointType::Ball;
    Pjv1Joint j3; j3.id = 3; j3.name = "C"; j3.bodyA = 5; j3.bodyB = 6; j3.type = Pjv1JointType::Hinge;
    pj.addJoint(j1); pj.addJoint(j2); pj.addJoint(j3);
    REQUIRE(pj.countByType(Pjv1JointType::Hinge) == 2);
    REQUIRE(pj.countByType(Pjv1JointType::Ball) == 1);
}

TEST_CASE("PhysicsJointEditorV1 breakableCount", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    Pjv1Joint j1; j1.id = 1; j1.name = "A"; j1.bodyA = 1; j1.bodyB = 2; j1.breakForce = 100.f;
    Pjv1Joint j2; j2.id = 2; j2.name = "B"; j2.bodyA = 3; j2.bodyB = 4; // unbreakable
    pj.addJoint(j1); pj.addJoint(j2);
    REQUIRE(pj.breakableCount() == 1);
}

TEST_CASE("PhysicsJointEditorV1 onChange callback", "[Editor][S197]") {
    PhysicsJointEditorV1 pj;
    uint64_t notified = 0;
    pj.setOnChange([&](uint64_t id){ notified = id; });
    Pjv1Joint j; j.id = 5; j.name = "X"; j.bodyA = 1; j.bodyB = 2;
    pj.addJoint(j);
    REQUIRE(notified == 5);
}

TEST_CASE("pjv1JointTypeName all values", "[Editor][S197]") {
    REQUIRE(std::string(pjv1JointTypeName(Pjv1JointType::Hinge))    == "Hinge");
    REQUIRE(std::string(pjv1JointTypeName(Pjv1JointType::Ball))     == "Ball");
    REQUIRE(std::string(pjv1JointTypeName(Pjv1JointType::Slider))   == "Slider");
    REQUIRE(std::string(pjv1JointTypeName(Pjv1JointType::Fixed))    == "Fixed");
    REQUIRE(std::string(pjv1JointTypeName(Pjv1JointType::Spring))   == "Spring");
    REQUIRE(std::string(pjv1JointTypeName(Pjv1JointType::Distance)) == "Distance");
}

// ── DestructionEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Dev1Chunk validity", "[Editor][S197]") {
    Dev1Chunk c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Chunk0";
    REQUIRE(c.isValid());
}

TEST_CASE("Dev1FractureConfig validity", "[Editor][S197]") {
    Dev1FractureConfig fc;
    REQUIRE(!fc.isValid());
    fc.id = 1; fc.name = "Config1"; fc.chunkCount = 8;
    REQUIRE(fc.isValid());
}

TEST_CASE("DestructionEditorV1 addConfig", "[Editor][S197]") {
    DestructionEditorV1 de;
    REQUIRE(de.configCount() == 0);
    Dev1FractureConfig fc; fc.id = 1; fc.name = "C1"; fc.chunkCount = 4;
    REQUIRE(de.addConfig(fc));
    REQUIRE(de.configCount() == 1);
}

TEST_CASE("DestructionEditorV1 addConfig duplicate fails", "[Editor][S197]") {
    DestructionEditorV1 de;
    Dev1FractureConfig fc; fc.id = 1; fc.name = "C"; fc.chunkCount = 4;
    de.addConfig(fc);
    REQUIRE(!de.addConfig(fc));
}

TEST_CASE("DestructionEditorV1 removeConfig", "[Editor][S197]") {
    DestructionEditorV1 de;
    Dev1FractureConfig fc; fc.id = 1; fc.name = "C"; fc.chunkCount = 4;
    de.addConfig(fc);
    REQUIRE(de.removeConfig(1));
    REQUIRE(de.configCount() == 0);
}

TEST_CASE("DestructionEditorV1 findConfig", "[Editor][S197]") {
    DestructionEditorV1 de;
    Dev1FractureConfig fc; fc.id = 3; fc.name = "C"; fc.chunkCount = 4;
    de.addConfig(fc);
    REQUIRE(de.findConfig(3) != nullptr);
    REQUIRE(de.findConfig(99) == nullptr);
}

TEST_CASE("DestructionEditorV1 addChunk and chunkCount", "[Editor][S197]") {
    DestructionEditorV1 de;
    REQUIRE(de.chunkCount() == 0);
    Dev1Chunk c; c.id = 1; c.name = "CH1";
    REQUIRE(de.addChunk(c));
    REQUIRE(de.chunkCount() == 1);
}

TEST_CASE("DestructionEditorV1 addChunk invalid fails", "[Editor][S197]") {
    DestructionEditorV1 de;
    REQUIRE(!de.addChunk(Dev1Chunk{}));
}

TEST_CASE("DestructionEditorV1 removeChunk", "[Editor][S197]") {
    DestructionEditorV1 de;
    Dev1Chunk c; c.id = 2; c.name = "CH";
    de.addChunk(c);
    REQUIRE(de.removeChunk(2));
    REQUIRE(de.chunkCount() == 0);
}

TEST_CASE("DestructionEditorV1 setChunkState", "[Editor][S197]") {
    DestructionEditorV1 de;
    Dev1Chunk c; c.id = 1; c.name = "CH";
    de.addChunk(c);
    REQUIRE(de.setChunkState(1, Dev1ChunkState::Destroyed));
    REQUIRE(de.findChunk(1)->isDestroyed());
}

TEST_CASE("DestructionEditorV1 intactCount and destroyedCount", "[Editor][S197]") {
    DestructionEditorV1 de;
    Dev1Chunk c1; c1.id = 1; c1.name = "A";
    Dev1Chunk c2; c2.id = 2; c2.name = "B";
    de.addChunk(c1); de.addChunk(c2);
    de.setChunkState(2, Dev1ChunkState::Destroyed);
    REQUIRE(de.intactCount() == 1);
    REQUIRE(de.destroyedCount() == 1);
}

TEST_CASE("DestructionEditorV1 onChange callback", "[Editor][S197]") {
    DestructionEditorV1 de;
    uint64_t notified = 0;
    de.setOnChange([&](uint64_t id){ notified = id; });
    Dev1Chunk c; c.id = 7; c.name = "X";
    de.addChunk(c);
    REQUIRE(notified == 7);
}

TEST_CASE("dev1FractureModeName all values", "[Editor][S197]") {
    REQUIRE(std::string(dev1FractureModeName(Dev1FractureMode::Voronoi)) == "Voronoi");
    REQUIRE(std::string(dev1FractureModeName(Dev1FractureMode::Uniform)) == "Uniform");
    REQUIRE(std::string(dev1FractureModeName(Dev1FractureMode::Radial))  == "Radial");
    REQUIRE(std::string(dev1FractureModeName(Dev1FractureMode::Custom))  == "Custom");
}

// ── WindFieldEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Wfv1Source validity", "[Editor][S197]") {
    Wfv1Source s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Wind1"; s.strength = 10.f;
    REQUIRE(s.isValid());
}

TEST_CASE("WindFieldEditorV1 addSource and sourceCount", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    REQUIRE(wf.sourceCount() == 0);
    Wfv1Source s; s.id = 1; s.name = "S1"; s.strength = 5.f;
    REQUIRE(wf.addSource(s));
    REQUIRE(wf.sourceCount() == 1);
}

TEST_CASE("WindFieldEditorV1 addSource invalid fails", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    REQUIRE(!wf.addSource(Wfv1Source{}));
}

TEST_CASE("WindFieldEditorV1 addSource duplicate fails", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s; s.id = 1; s.name = "A"; s.strength = 5.f;
    wf.addSource(s);
    REQUIRE(!wf.addSource(s));
}

TEST_CASE("WindFieldEditorV1 removeSource", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s; s.id = 2; s.name = "B"; s.strength = 5.f;
    wf.addSource(s);
    REQUIRE(wf.removeSource(2));
    REQUIRE(wf.sourceCount() == 0);
    REQUIRE(!wf.removeSource(2));
}

TEST_CASE("WindFieldEditorV1 findSource", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s; s.id = 3; s.name = "C"; s.strength = 5.f;
    wf.addSource(s);
    REQUIRE(wf.findSource(3) != nullptr);
    REQUIRE(wf.findSource(99) == nullptr);
}

TEST_CASE("WindFieldEditorV1 setState", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s; s.id = 1; s.name = "W"; s.strength = 5.f;
    wf.addSource(s);
    REQUIRE(wf.setState(1, Wfv1WindState::Disabled));
    REQUIRE(wf.findSource(1)->state == Wfv1WindState::Disabled);
}

TEST_CASE("WindFieldEditorV1 setStrength", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s; s.id = 1; s.name = "W"; s.strength = 5.f;
    wf.addSource(s);
    REQUIRE(wf.setStrength(1, 25.f));
    REQUIRE(wf.findSource(1)->strength == Approx(25.f));
}

TEST_CASE("WindFieldEditorV1 setStrength clamped to zero", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s; s.id = 1; s.name = "W"; s.strength = 5.f;
    wf.addSource(s);
    wf.setStrength(1, -10.f);
    REQUIRE(wf.findSource(1)->strength == Approx(0.f));
}

TEST_CASE("WindFieldEditorV1 activeCount", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s1; s1.id = 1; s1.name = "A"; s1.strength = 5.f;
    Wfv1Source s2; s2.id = 2; s2.name = "B"; s2.strength = 5.f;
    wf.addSource(s1); wf.addSource(s2);
    wf.setState(2, Wfv1WindState::Disabled);
    REQUIRE(wf.activeCount() == 1);
}

TEST_CASE("WindFieldEditorV1 countByType", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s1; s1.id = 1; s1.name = "A"; s1.strength = 5.f; s1.type = Wfv1WindType::Directional;
    Wfv1Source s2; s2.id = 2; s2.name = "B"; s2.strength = 5.f; s2.type = Wfv1WindType::Vortex;
    Wfv1Source s3; s3.id = 3; s3.name = "C"; s3.strength = 5.f; s3.type = Wfv1WindType::Directional;
    wf.addSource(s1); wf.addSource(s2); wf.addSource(s3);
    REQUIRE(wf.countByType(Wfv1WindType::Directional) == 2);
    REQUIRE(wf.countByType(Wfv1WindType::Vortex) == 1);
}

TEST_CASE("WindFieldEditorV1 totalStrength", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    Wfv1Source s1; s1.id = 1; s1.name = "A"; s1.strength = 10.f;
    Wfv1Source s2; s2.id = 2; s2.name = "B"; s2.strength = 20.f;
    wf.addSource(s1); wf.addSource(s2);
    REQUIRE(wf.totalStrength() == Approx(30.f));
}

TEST_CASE("WindFieldEditorV1 onChange callback", "[Editor][S197]") {
    WindFieldEditorV1 wf;
    uint64_t notified = 0;
    wf.setOnChange([&](uint64_t id){ notified = id; });
    Wfv1Source s; s.id = 5; s.name = "X"; s.strength = 5.f;
    wf.addSource(s);
    REQUIRE(notified == 5);
}

TEST_CASE("wfv1WindTypeName all values", "[Editor][S197]") {
    REQUIRE(std::string(wfv1WindTypeName(Wfv1WindType::Directional)) == "Directional");
    REQUIRE(std::string(wfv1WindTypeName(Wfv1WindType::Point))       == "Point");
    REQUIRE(std::string(wfv1WindTypeName(Wfv1WindType::Vortex))      == "Vortex");
    REQUIRE(std::string(wfv1WindTypeName(Wfv1WindType::Turbulence))  == "Turbulence");
}
