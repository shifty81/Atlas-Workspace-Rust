// S138 editor tests: PhysicsMaterialEditor, PhysicsConstraintEditor, PhysicsTriggerEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PhysicsTriggerEditor.h"
#include "NF/Editor/PhysicsConstraintEditor.h"
#include "NF/Editor/PhysicsMaterialEditor.h"

using namespace NF;

// ── PhysicsMaterialEditor ─────────────────────────────────────────────────────

TEST_CASE("PhysMatSurface names", "[Editor][S138]") {
    REQUIRE(std::string(physMatSurfaceName(PhysMatSurface::Default))  == "Default");
    REQUIRE(std::string(physMatSurfaceName(PhysMatSurface::Metal))    == "Metal");
    REQUIRE(std::string(physMatSurfaceName(PhysMatSurface::Wood))     == "Wood");
    REQUIRE(std::string(physMatSurfaceName(PhysMatSurface::Concrete)) == "Concrete");
    REQUIRE(std::string(physMatSurfaceName(PhysMatSurface::Ice))      == "Ice");
    REQUIRE(std::string(physMatSurfaceName(PhysMatSurface::Rubber))   == "Rubber");
}

TEST_CASE("PhysMatCombine names", "[Editor][S138]") {
    REQUIRE(std::string(physMatCombineName(PhysMatCombine::Average))  == "Average");
    REQUIRE(std::string(physMatCombineName(PhysMatCombine::Minimum))  == "Minimum");
    REQUIRE(std::string(physMatCombineName(PhysMatCombine::Maximum))  == "Maximum");
    REQUIRE(std::string(physMatCombineName(PhysMatCombine::Multiply)) == "Multiply");
}

TEST_CASE("PhysicsMaterial defaults", "[Editor][S138]") {
    PhysicsMaterial m(1, "rubber_surface", PhysMatSurface::Rubber, PhysMatCombine::Average);
    REQUIRE(m.id()          == 1u);
    REQUIRE(m.name()        == "rubber_surface");
    REQUIRE(m.surface()     == PhysMatSurface::Rubber);
    REQUIRE(m.combine()     == PhysMatCombine::Average);
    REQUIRE(m.friction()    == 0.5f);
    REQUIRE(m.restitution() == 0.3f);
    REQUIRE(m.density()     == 1.0f);
    REQUIRE(m.isEnabled());
}

TEST_CASE("PhysicsMaterial mutation", "[Editor][S138]") {
    PhysicsMaterial m(2, "ice_slick", PhysMatSurface::Ice, PhysMatCombine::Minimum);
    m.setFriction(0.05f);
    m.setRestitution(0.1f);
    m.setDensity(0.9f);
    m.setIsEnabled(false);
    REQUIRE(m.friction()    == 0.05f);
    REQUIRE(m.restitution() == 0.1f);
    REQUIRE(m.density()     == 0.9f);
    REQUIRE(!m.isEnabled());
}

TEST_CASE("PhysicsMaterialEditor defaults", "[Editor][S138]") {
    PhysicsMaterialEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupBySurface());
    REQUIRE(ed.defaultFriction() == 0.6f);
    REQUIRE(ed.materialCount()   == 0u);
}

TEST_CASE("PhysicsMaterialEditor add/remove", "[Editor][S138]") {
    PhysicsMaterialEditor ed;
    REQUIRE(ed.addMaterial(PhysicsMaterial(1, "a", PhysMatSurface::Default,  PhysMatCombine::Average)));
    REQUIRE(ed.addMaterial(PhysicsMaterial(2, "b", PhysMatSurface::Metal,    PhysMatCombine::Maximum)));
    REQUIRE(ed.addMaterial(PhysicsMaterial(3, "c", PhysMatSurface::Wood,     PhysMatCombine::Multiply)));
    REQUIRE(!ed.addMaterial(PhysicsMaterial(1, "a", PhysMatSurface::Default, PhysMatCombine::Average)));
    REQUIRE(ed.materialCount() == 3u);
    REQUIRE(ed.removeMaterial(2));
    REQUIRE(ed.materialCount() == 2u);
    REQUIRE(!ed.removeMaterial(99));
}

TEST_CASE("PhysicsMaterialEditor counts and find", "[Editor][S138]") {
    PhysicsMaterialEditor ed;
    PhysicsMaterial m1(1, "a", PhysMatSurface::Metal,    PhysMatCombine::Average);
    PhysicsMaterial m2(2, "b", PhysMatSurface::Metal,    PhysMatCombine::Minimum);
    PhysicsMaterial m3(3, "c", PhysMatSurface::Concrete, PhysMatCombine::Maximum);
    PhysicsMaterial m4(4, "d", PhysMatSurface::Ice,      PhysMatCombine::Multiply); m4.setIsEnabled(false);
    ed.addMaterial(m1); ed.addMaterial(m2); ed.addMaterial(m3); ed.addMaterial(m4);
    REQUIRE(ed.countBySurface(PhysMatSurface::Metal)       == 2u);
    REQUIRE(ed.countBySurface(PhysMatSurface::Concrete)    == 1u);
    REQUIRE(ed.countBySurface(PhysMatSurface::Wood)        == 0u);
    REQUIRE(ed.countByCombine(PhysMatCombine::Average)     == 1u);
    REQUIRE(ed.countByCombine(PhysMatCombine::Minimum)     == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findMaterial(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->surface() == PhysMatSurface::Concrete);
    REQUIRE(ed.findMaterial(99) == nullptr);
}

TEST_CASE("PhysicsMaterialEditor settings mutation", "[Editor][S138]") {
    PhysicsMaterialEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupBySurface(false);
    ed.setDefaultFriction(0.8f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupBySurface());
    REQUIRE(ed.defaultFriction() == 0.8f);
}

// ── PhysicsConstraintEditor ───────────────────────────────────────────────────

TEST_CASE("PhysConstraintType names", "[Editor][S138]") {
    REQUIRE(std::string(physConstraintTypeName(PhysConstraintType::Fixed))  == "Fixed");
    REQUIRE(std::string(physConstraintTypeName(PhysConstraintType::Hinge))  == "Hinge");
    REQUIRE(std::string(physConstraintTypeName(PhysConstraintType::Slider)) == "Slider");
    REQUIRE(std::string(physConstraintTypeName(PhysConstraintType::Ball))   == "Ball");
    REQUIRE(std::string(physConstraintTypeName(PhysConstraintType::Spring)) == "Spring");
}

TEST_CASE("PhysConstraintAxis names", "[Editor][S138]") {
    REQUIRE(std::string(physConstraintAxisName(PhysConstraintAxis::X))   == "X");
    REQUIRE(std::string(physConstraintAxisName(PhysConstraintAxis::Y))   == "Y");
    REQUIRE(std::string(physConstraintAxisName(PhysConstraintAxis::Z))   == "Z");
    REQUIRE(std::string(physConstraintAxisName(PhysConstraintAxis::XY))  == "XY");
    REQUIRE(std::string(physConstraintAxisName(PhysConstraintAxis::XYZ)) == "XYZ");
}

TEST_CASE("PhysicsConstraint defaults", "[Editor][S138]") {
    PhysicsConstraint c(1, "door_hinge", PhysConstraintType::Hinge, PhysConstraintAxis::Y);
    REQUIRE(c.id()        == 1u);
    REQUIRE(c.name()      == "door_hinge");
    REQUIRE(c.type()      == PhysConstraintType::Hinge);
    REQUIRE(c.axis()      == PhysConstraintAxis::Y);
    REQUIRE(c.stiffness() == 100.0f);
    REQUIRE(c.damping()   == 10.0f);
    REQUIRE(c.isEnabled());
}

TEST_CASE("PhysicsConstraint mutation", "[Editor][S138]") {
    PhysicsConstraint c(2, "spring_joint", PhysConstraintType::Spring, PhysConstraintAxis::XYZ);
    c.setStiffness(500.0f);
    c.setDamping(50.0f);
    c.setIsEnabled(false);
    REQUIRE(c.stiffness() == 500.0f);
    REQUIRE(c.damping()   == 50.0f);
    REQUIRE(!c.isEnabled());
}

TEST_CASE("PhysicsConstraintEditor defaults", "[Editor][S138]") {
    PhysicsConstraintEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.defaultStiffness() == 200.0f);
    REQUIRE(ed.constraintCount()  == 0u);
}

TEST_CASE("PhysicsConstraintEditor add/remove", "[Editor][S138]") {
    PhysicsConstraintEditor ed;
    REQUIRE(ed.addConstraint(PhysicsConstraint(1, "a", PhysConstraintType::Fixed,  PhysConstraintAxis::X)));
    REQUIRE(ed.addConstraint(PhysicsConstraint(2, "b", PhysConstraintType::Hinge,  PhysConstraintAxis::Y)));
    REQUIRE(ed.addConstraint(PhysicsConstraint(3, "c", PhysConstraintType::Spring, PhysConstraintAxis::XYZ)));
    REQUIRE(!ed.addConstraint(PhysicsConstraint(1, "a", PhysConstraintType::Fixed, PhysConstraintAxis::X)));
    REQUIRE(ed.constraintCount() == 3u);
    REQUIRE(ed.removeConstraint(2));
    REQUIRE(ed.constraintCount() == 2u);
    REQUIRE(!ed.removeConstraint(99));
}

TEST_CASE("PhysicsConstraintEditor counts and find", "[Editor][S138]") {
    PhysicsConstraintEditor ed;
    PhysicsConstraint c1(1, "a", PhysConstraintType::Hinge,  PhysConstraintAxis::Y);
    PhysicsConstraint c2(2, "b", PhysConstraintType::Hinge,  PhysConstraintAxis::X);
    PhysicsConstraint c3(3, "c", PhysConstraintType::Slider, PhysConstraintAxis::Z);
    PhysicsConstraint c4(4, "d", PhysConstraintType::Ball,   PhysConstraintAxis::XYZ); c4.setIsEnabled(false);
    ed.addConstraint(c1); ed.addConstraint(c2); ed.addConstraint(c3); ed.addConstraint(c4);
    REQUIRE(ed.countByType(PhysConstraintType::Hinge)   == 2u);
    REQUIRE(ed.countByType(PhysConstraintType::Slider)  == 1u);
    REQUIRE(ed.countByType(PhysConstraintType::Fixed)   == 0u);
    REQUIRE(ed.countByAxis(PhysConstraintAxis::Y)       == 1u);
    REQUIRE(ed.countByAxis(PhysConstraintAxis::X)       == 1u);
    REQUIRE(ed.countEnabled()                           == 3u);
    auto* found = ed.findConstraint(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == PhysConstraintType::Slider);
    REQUIRE(ed.findConstraint(99) == nullptr);
}

TEST_CASE("PhysicsConstraintEditor settings mutation", "[Editor][S138]") {
    PhysicsConstraintEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(false);
    ed.setDefaultStiffness(400.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.defaultStiffness() == 400.0f);
}

// ── PhysicsTriggerEditor ──────────────────────────────────────────────────────

TEST_CASE("PhysTriggerShape names", "[Editor][S138]") {
    REQUIRE(std::string(physTriggerShapeName(PhysTriggerShape::Box))     == "Box");
    REQUIRE(std::string(physTriggerShapeName(PhysTriggerShape::Sphere))  == "Sphere");
    REQUIRE(std::string(physTriggerShapeName(PhysTriggerShape::Capsule)) == "Capsule");
    REQUIRE(std::string(physTriggerShapeName(PhysTriggerShape::Mesh))    == "Mesh");
    REQUIRE(std::string(physTriggerShapeName(PhysTriggerShape::Convex))  == "Convex");
}

TEST_CASE("PhysTriggerEvent names", "[Editor][S138]") {
    REQUIRE(std::string(physTriggerEventName(PhysTriggerEvent::OnEnter))   == "OnEnter");
    REQUIRE(std::string(physTriggerEventName(PhysTriggerEvent::OnExit))    == "OnExit");
    REQUIRE(std::string(physTriggerEventName(PhysTriggerEvent::OnStay))    == "OnStay");
    REQUIRE(std::string(physTriggerEventName(PhysTriggerEvent::OnOverlap)) == "OnOverlap");
}

TEST_CASE("PhysicsTrigger defaults", "[Editor][S138]") {
    PhysicsTrigger t(1, "zone_enter", PhysTriggerShape::Box, PhysTriggerEvent::OnEnter);
    REQUIRE(t.id()           == 1u);
    REQUIRE(t.name()         == "zone_enter");
    REQUIRE(t.shape()        == PhysTriggerShape::Box);
    REQUIRE(t.event()        == PhysTriggerEvent::OnEnter);
    REQUIRE(!t.isContinuous());
    REQUIRE(t.filterMask()   == 0xFFFFFFFFu);
    REQUIRE(t.isEnabled());
}

TEST_CASE("PhysicsTrigger mutation", "[Editor][S138]") {
    PhysicsTrigger t(2, "stay_sphere", PhysTriggerShape::Sphere, PhysTriggerEvent::OnStay);
    t.setIsContinuous(true);
    t.setFilterMask(0x0000000Fu);
    t.setIsEnabled(false);
    REQUIRE(t.isContinuous());
    REQUIRE(t.filterMask()  == 0x0000000Fu);
    REQUIRE(!t.isEnabled());
}

TEST_CASE("PhysicsTriggerEditor defaults", "[Editor][S138]") {
    PhysicsTriggerEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByShape());
    REQUIRE(ed.defaultFilterMask() == 0x00000001u);
    REQUIRE(ed.triggerCount()      == 0u);
}

TEST_CASE("PhysicsTriggerEditor add/remove", "[Editor][S138]") {
    PhysicsTriggerEditor ed;
    REQUIRE(ed.addTrigger(PhysicsTrigger(1, "a", PhysTriggerShape::Box,     PhysTriggerEvent::OnEnter)));
    REQUIRE(ed.addTrigger(PhysicsTrigger(2, "b", PhysTriggerShape::Sphere,  PhysTriggerEvent::OnExit)));
    REQUIRE(ed.addTrigger(PhysicsTrigger(3, "c", PhysTriggerShape::Capsule, PhysTriggerEvent::OnStay)));
    REQUIRE(!ed.addTrigger(PhysicsTrigger(1, "a", PhysTriggerShape::Box,    PhysTriggerEvent::OnEnter)));
    REQUIRE(ed.triggerCount() == 3u);
    REQUIRE(ed.removeTrigger(2));
    REQUIRE(ed.triggerCount() == 2u);
    REQUIRE(!ed.removeTrigger(99));
}

TEST_CASE("PhysicsTriggerEditor counts and find", "[Editor][S138]") {
    PhysicsTriggerEditor ed;
    PhysicsTrigger t1(1, "a", PhysTriggerShape::Box,    PhysTriggerEvent::OnEnter);
    PhysicsTrigger t2(2, "b", PhysTriggerShape::Box,    PhysTriggerEvent::OnExit);
    PhysicsTrigger t3(3, "c", PhysTriggerShape::Sphere, PhysTriggerEvent::OnStay);
    PhysicsTrigger t4(4, "d", PhysTriggerShape::Mesh,   PhysTriggerEvent::OnOverlap); t4.setIsEnabled(false);
    ed.addTrigger(t1); ed.addTrigger(t2); ed.addTrigger(t3); ed.addTrigger(t4);
    REQUIRE(ed.countByShape(PhysTriggerShape::Box)         == 2u);
    REQUIRE(ed.countByShape(PhysTriggerShape::Sphere)      == 1u);
    REQUIRE(ed.countByShape(PhysTriggerShape::Capsule)     == 0u);
    REQUIRE(ed.countByEvent(PhysTriggerEvent::OnEnter)     == 1u);
    REQUIRE(ed.countByEvent(PhysTriggerEvent::OnExit)      == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findTrigger(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->shape() == PhysTriggerShape::Sphere);
    REQUIRE(ed.findTrigger(99) == nullptr);
}

TEST_CASE("PhysicsTriggerEditor settings mutation", "[Editor][S138]") {
    PhysicsTriggerEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByShape(true);
    ed.setDefaultFilterMask(0x000000FFu);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByShape());
    REQUIRE(ed.defaultFilterMask() == 0x000000FFu);
}
