// S101 editor tests: SplineEditor, RopeSimEditor, ClothSimEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ClothSimEditor.h"
#include "NF/Editor/RopeSimEditor.h"
#include "NF/Editor/SplineEditor.h"

using namespace NF;

// ── SplineEditor ─────────────────────────────────────────────────────────────

TEST_CASE("SplineType names", "[Editor][S101]") {
    REQUIRE(std::string(splineTypeName(SplineType::Linear))     == "Linear");
    REQUIRE(std::string(splineTypeName(SplineType::CatmullRom)) == "CatmullRom");
    REQUIRE(std::string(splineTypeName(SplineType::Bezier))     == "Bezier");
    REQUIRE(std::string(splineTypeName(SplineType::Hermite))    == "Hermite");
    REQUIRE(std::string(splineTypeName(SplineType::NURBS))      == "NURBS");
}

TEST_CASE("SplineTangentMode names", "[Editor][S101]") {
    REQUIRE(std::string(splineTangentModeName(SplineTangentMode::Auto))     == "Auto");
    REQUIRE(std::string(splineTangentModeName(SplineTangentMode::Clamped))  == "Clamped");
    REQUIRE(std::string(splineTangentModeName(SplineTangentMode::Linear))   == "Linear");
    REQUIRE(std::string(splineTangentModeName(SplineTangentMode::Constant)) == "Constant");
    REQUIRE(std::string(splineTangentModeName(SplineTangentMode::Broken))   == "Broken");
}

TEST_CASE("SplineLoopMode names", "[Editor][S101]") {
    REQUIRE(std::string(splineLoopModeName(SplineLoopMode::None))     == "None");
    REQUIRE(std::string(splineLoopModeName(SplineLoopMode::Loop))     == "Loop");
    REQUIRE(std::string(splineLoopModeName(SplineLoopMode::PingPong)) == "PingPong");
}

TEST_CASE("SplineControlPoint defaults", "[Editor][S101]") {
    SplineControlPoint pt(1);
    REQUIRE(pt.id()          == 1u);
    REQUIRE(pt.tangentMode() == SplineTangentMode::Auto);
    REQUIRE(pt.weight()      == 1.0f);
    REQUIRE(!pt.isSelected());
}

TEST_CASE("SplineControlPoint mutation", "[Editor][S101]") {
    SplineControlPoint pt(2);
    pt.setTangentMode(SplineTangentMode::Broken);
    pt.setWeight(0.5f);
    pt.setSelected(true);
    REQUIRE(pt.tangentMode() == SplineTangentMode::Broken);
    REQUIRE(pt.weight()      == 0.5f);
    REQUIRE(pt.isSelected());
}

TEST_CASE("SplineEditor defaults", "[Editor][S101]") {
    SplineEditor ed("road_spline");
    REQUIRE(ed.name()       == "road_spline");
    REQUIRE(ed.type()       == SplineType::CatmullRom);
    REQUIRE(ed.loopMode()   == SplineLoopMode::None);
    REQUIRE(!ed.isClosed());
    REQUIRE(ed.resolution() == 32u);
    REQUIRE(!ed.isDirty());
    REQUIRE(ed.pointCount() == 0u);
}

TEST_CASE("SplineEditor add/remove points", "[Editor][S101]") {
    SplineEditor ed("path");
    REQUIRE(ed.addPoint(SplineControlPoint(1)));
    REQUIRE(ed.addPoint(SplineControlPoint(2)));
    REQUIRE(!ed.addPoint(SplineControlPoint(1)));
    REQUIRE(ed.pointCount() == 2u);
    REQUIRE(ed.isDirty());
    REQUIRE(ed.removePoint(1));
    REQUIRE(ed.pointCount() == 1u);
    REQUIRE(!ed.removePoint(99));
}

TEST_CASE("SplineEditor find and counts", "[Editor][S101]") {
    SplineEditor ed("spline");
    SplineControlPoint p1(1); p1.setSelected(true);
    SplineControlPoint p2(2);
    SplineControlPoint p3(3); p3.setSelected(true);
    ed.addPoint(p1); ed.addPoint(p2); ed.addPoint(p3);
    REQUIRE(ed.pointCount()          == 3u);
    REQUIRE(ed.selectedPointCount()  == 2u);
    auto* fp = ed.findPoint(2);
    REQUIRE(fp != nullptr);
    REQUIRE(fp->id() == 2u);
    REQUIRE(ed.findPoint(99) == nullptr);
}

TEST_CASE("SplineEditor mutation", "[Editor][S101]") {
    SplineEditor ed("curve");
    ed.setType(SplineType::Bezier);
    ed.setLoopMode(SplineLoopMode::Loop);
    ed.setClosed(true);
    ed.setResolution(64);
    REQUIRE(ed.type()       == SplineType::Bezier);
    REQUIRE(ed.loopMode()   == SplineLoopMode::Loop);
    REQUIRE(ed.isClosed());
    REQUIRE(ed.resolution() == 64u);
}

// ── RopeSimEditor ────────────────────────────────────────────────────────────

TEST_CASE("RopeSimSolver names", "[Editor][S101]") {
    REQUIRE(std::string(ropeSimSolverName(RopeSimSolver::Verlet))        == "Verlet");
    REQUIRE(std::string(ropeSimSolverName(RopeSimSolver::PBD))           == "PBD");
    REQUIRE(std::string(ropeSimSolverName(RopeSimSolver::XPBD))          == "XPBD");
    REQUIRE(std::string(ropeSimSolverName(RopeSimSolver::PositionBased)) == "PositionBased");
    REQUIRE(std::string(ropeSimSolverName(RopeSimSolver::RigidBody))     == "RigidBody");
}

TEST_CASE("RopeAttachMode names", "[Editor][S101]") {
    REQUIRE(std::string(ropeAttachModeName(RopeAttachMode::Both))      == "Both");
    REQUIRE(std::string(ropeAttachModeName(RopeAttachMode::StartOnly)) == "StartOnly");
    REQUIRE(std::string(ropeAttachModeName(RopeAttachMode::EndOnly))   == "EndOnly");
    REQUIRE(std::string(ropeAttachModeName(RopeAttachMode::None))      == "None");
}

TEST_CASE("RopeCollisionMode names", "[Editor][S101]") {
    REQUIRE(std::string(ropeCollisionModeName(RopeCollisionMode::None))    == "None");
    REQUIRE(std::string(ropeCollisionModeName(RopeCollisionMode::Sphere))  == "Sphere");
    REQUIRE(std::string(ropeCollisionModeName(RopeCollisionMode::Capsule)) == "Capsule");
    REQUIRE(std::string(ropeCollisionModeName(RopeCollisionMode::Full))    == "Full");
}

TEST_CASE("RopeSimConfig defaults", "[Editor][S101]") {
    RopeSimConfig cfg("chain");
    REQUIRE(cfg.name()          == "chain");
    REQUIRE(cfg.solver()        == RopeSimSolver::XPBD);
    REQUIRE(cfg.attachMode()    == RopeAttachMode::Both);
    REQUIRE(cfg.collisionMode() == RopeCollisionMode::Capsule);
    REQUIRE(cfg.segments()      == 16u);
    REQUIRE(cfg.stiffness()     == 1.0f);
    REQUIRE(cfg.damping()       == 0.1f);
    REQUIRE(cfg.gravityScale()  == 1.0f);
    REQUIRE(cfg.isEnabled());
}

TEST_CASE("RopeSimConfig mutation", "[Editor][S101]") {
    RopeSimConfig cfg("bridge_cable");
    cfg.setSolver(RopeSimSolver::Verlet);
    cfg.setAttachMode(RopeAttachMode::StartOnly);
    cfg.setCollisionMode(RopeCollisionMode::Full);
    cfg.setSegments(32);
    cfg.setStiffness(0.9f);
    cfg.setDamping(0.05f);
    cfg.setGravityScale(2.0f);
    cfg.setEnabled(false);
    REQUIRE(cfg.solver()        == RopeSimSolver::Verlet);
    REQUIRE(cfg.attachMode()    == RopeAttachMode::StartOnly);
    REQUIRE(cfg.collisionMode() == RopeCollisionMode::Full);
    REQUIRE(cfg.segments()      == 32u);
    REQUIRE(cfg.stiffness()     == 0.9f);
    REQUIRE(cfg.damping()       == 0.05f);
    REQUIRE(cfg.gravityScale()  == 2.0f);
    REQUIRE(!cfg.isEnabled());
}

TEST_CASE("RopeSimEditor add/remove", "[Editor][S101]") {
    RopeSimEditor ed;
    REQUIRE(ed.addRope(RopeSimConfig("rope1")));
    REQUIRE(ed.addRope(RopeSimConfig("rope2")));
    REQUIRE(!ed.addRope(RopeSimConfig("rope1")));
    REQUIRE(ed.ropeCount() == 2u);
    REQUIRE(ed.removeRope("rope1"));
    REQUIRE(ed.ropeCount() == 1u);
    REQUIRE(!ed.removeRope("rope1"));
}

TEST_CASE("RopeSimEditor counts", "[Editor][S101]") {
    RopeSimEditor ed;
    RopeSimConfig r1("r1"); r1.setSolver(RopeSimSolver::XPBD);
    RopeSimConfig r2("r2"); r2.setSolver(RopeSimSolver::Verlet); r2.setEnabled(false);
    RopeSimConfig r3("r3"); r3.setSolver(RopeSimSolver::XPBD); r3.setAttachMode(RopeAttachMode::StartOnly);
    ed.addRope(r1); ed.addRope(r2); ed.addRope(r3);
    REQUIRE(ed.ropeCount()                              == 3u);
    REQUIRE(ed.enabledCount()                           == 2u);
    REQUIRE(ed.countBySolver(RopeSimSolver::XPBD)       == 2u);
    REQUIRE(ed.countByAttach(RopeAttachMode::StartOnly)  == 1u);
}

// ── ClothSimEditor ───────────────────────────────────────────────────────────

TEST_CASE("ClothSolverType names", "[Editor][S101]") {
    REQUIRE(std::string(clothSolverTypeName(ClothSolverType::PBD))          == "PBD");
    REQUIRE(std::string(clothSolverTypeName(ClothSolverType::XPBD))         == "XPBD");
    REQUIRE(std::string(clothSolverTypeName(ClothSolverType::Verlet))       == "Verlet");
    REQUIRE(std::string(clothSolverTypeName(ClothSolverType::FEM))          == "FEM");
    REQUIRE(std::string(clothSolverTypeName(ClothSolverType::CoRotational)) == "CoRotational");
}

TEST_CASE("ClothCollisionResponse names", "[Editor][S101]") {
    REQUIRE(std::string(clothCollisionResponseName(ClothCollisionResponse::None))       == "None");
    REQUIRE(std::string(clothCollisionResponseName(ClothCollisionResponse::PointCloud)) == "PointCloud");
    REQUIRE(std::string(clothCollisionResponseName(ClothCollisionResponse::Mesh))       == "Mesh");
    REQUIRE(std::string(clothCollisionResponseName(ClothCollisionResponse::SDF))        == "SDF");
}

TEST_CASE("ClothPaintMode names", "[Editor][S101]") {
    REQUIRE(std::string(clothPaintModeName(ClothPaintMode::MaxDistance))    == "MaxDistance");
    REQUIRE(std::string(clothPaintModeName(ClothPaintMode::Stiffness))      == "Stiffness");
    REQUIRE(std::string(clothPaintModeName(ClothPaintMode::Damping))        == "Damping");
    REQUIRE(std::string(clothPaintModeName(ClothPaintMode::BackstopRadius)) == "BackstopRadius");
    REQUIRE(std::string(clothPaintModeName(ClothPaintMode::None))           == "None");
}

TEST_CASE("ClothSimConfig defaults", "[Editor][S101]") {
    ClothSimConfig cfg("tunic");
    REQUIRE(cfg.name()              == "tunic");
    REQUIRE(cfg.solver()            == ClothSolverType::XPBD);
    REQUIRE(cfg.collisionResponse() == ClothCollisionResponse::Mesh);
    REQUIRE(cfg.paintMode()         == ClothPaintMode::None);
    REQUIRE(cfg.iterations()        == 8u);
    REQUIRE(cfg.stiffness()         == 0.8f);
    REQUIRE(cfg.damping()           == 0.05f);
    REQUIRE(cfg.gravityScale()      == 1.0f);
    REQUIRE(!cfg.hasSelfCollision());
    REQUIRE(cfg.isEnabled());
}

TEST_CASE("ClothSimConfig mutation", "[Editor][S101]") {
    ClothSimConfig cfg("cape");
    cfg.setSolver(ClothSolverType::FEM);
    cfg.setCollisionResponse(ClothCollisionResponse::SDF);
    cfg.setPaintMode(ClothPaintMode::Stiffness);
    cfg.setIterations(16);
    cfg.setSelfCollision(true);
    cfg.setEnabled(false);
    REQUIRE(cfg.solver()            == ClothSolverType::FEM);
    REQUIRE(cfg.collisionResponse() == ClothCollisionResponse::SDF);
    REQUIRE(cfg.paintMode()         == ClothPaintMode::Stiffness);
    REQUIRE(cfg.iterations()        == 16u);
    REQUIRE(cfg.hasSelfCollision());
    REQUIRE(!cfg.isEnabled());
}

TEST_CASE("ClothSimEditor add/remove", "[Editor][S101]") {
    ClothSimEditor ed;
    REQUIRE(ed.addConfig(ClothSimConfig("cloth1")));
    REQUIRE(ed.addConfig(ClothSimConfig("cloth2")));
    REQUIRE(!ed.addConfig(ClothSimConfig("cloth1")));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(ed.removeConfig("cloth1"));
    REQUIRE(ed.configCount() == 1u);
    REQUIRE(!ed.removeConfig("cloth1"));
}

TEST_CASE("ClothSimEditor counts", "[Editor][S101]") {
    ClothSimEditor ed;
    ClothSimConfig c1("a"); c1.setSolver(ClothSolverType::XPBD); c1.setSelfCollision(true);
    ClothSimConfig c2("b"); c2.setSolver(ClothSolverType::FEM);   c2.setEnabled(false);
    ClothSimConfig c3("c"); c3.setSolver(ClothSolverType::XPBD);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3);
    REQUIRE(ed.configCount()                             == 3u);
    REQUIRE(ed.enabledCount()                            == 2u);
    REQUIRE(ed.selfCollisionCount()                      == 1u);
    REQUIRE(ed.countBySolver(ClothSolverType::XPBD)      == 2u);
}
