// S103 editor tests: ProceduralMeshEditor, MeshDeformerEditor, SkeletonEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SkeletonEditor.h"
#include "NF/Editor/MeshDeformerEditor.h"
#include "NF/Editor/ProceduralMeshEditor.h"

using namespace NF;

// ── ProceduralMeshEditor ─────────────────────────────────────────────────────

TEST_CASE("ProceduralMeshPrimitive names", "[Editor][S103]") {
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Plane))    == "Plane");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Box))      == "Box");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Sphere))   == "Sphere");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Cylinder)) == "Cylinder");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Cone))     == "Cone");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Torus))    == "Torus");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Capsule))  == "Capsule");
    REQUIRE(std::string(proceduralMeshPrimitiveName(ProceduralMeshPrimitive::Custom))   == "Custom");
}

TEST_CASE("ProceduralMeshNormalMode names", "[Editor][S103]") {
    REQUIRE(std::string(proceduralMeshNormalModeName(ProceduralMeshNormalMode::Smooth))         == "Smooth");
    REQUIRE(std::string(proceduralMeshNormalModeName(ProceduralMeshNormalMode::Flat))           == "Flat");
    REQUIRE(std::string(proceduralMeshNormalModeName(ProceduralMeshNormalMode::WeightedSmooth)) == "WeightedSmooth");
}

TEST_CASE("ProceduralMeshUVMode names", "[Editor][S103]") {
    REQUIRE(std::string(proceduralMeshUVModeName(ProceduralMeshUVMode::Planar))      == "Planar");
    REQUIRE(std::string(proceduralMeshUVModeName(ProceduralMeshUVMode::Box))         == "Box");
    REQUIRE(std::string(proceduralMeshUVModeName(ProceduralMeshUVMode::Spherical))   == "Spherical");
    REQUIRE(std::string(proceduralMeshUVModeName(ProceduralMeshUVMode::Cylindrical)) == "Cylindrical");
    REQUIRE(std::string(proceduralMeshUVModeName(ProceduralMeshUVMode::Custom))      == "Custom");
}

TEST_CASE("ProceduralMeshConfig defaults", "[Editor][S103]") {
    ProceduralMeshConfig cfg("floor_tile", ProceduralMeshPrimitive::Plane);
    REQUIRE(cfg.name()               == "floor_tile");
    REQUIRE(cfg.primitive()          == ProceduralMeshPrimitive::Plane);
    REQUIRE(cfg.normalMode()         == ProceduralMeshNormalMode::Smooth);
    REQUIRE(cfg.uvMode()             == ProceduralMeshUVMode::Box);
    REQUIRE(cfg.subdivisions()       == 1u);
    REQUIRE(cfg.generatesTangents());
    REQUIRE(!cfg.generatesCollision());
    REQUIRE(!cfg.isDirty());
}

TEST_CASE("ProceduralMeshConfig mutation", "[Editor][S103]") {
    ProceduralMeshConfig cfg("column", ProceduralMeshPrimitive::Cylinder);
    cfg.setNormalMode(ProceduralMeshNormalMode::Flat);
    cfg.setUVMode(ProceduralMeshUVMode::Cylindrical);
    cfg.setSubdivisions(4);
    cfg.setGenerateTangents(false);
    cfg.setGenerateCollision(true);
    cfg.setDirty(true);
    REQUIRE(cfg.normalMode()         == ProceduralMeshNormalMode::Flat);
    REQUIRE(cfg.uvMode()             == ProceduralMeshUVMode::Cylindrical);
    REQUIRE(cfg.subdivisions()       == 4u);
    REQUIRE(!cfg.generatesTangents());
    REQUIRE(cfg.generatesCollision());
    REQUIRE(cfg.isDirty());
}

TEST_CASE("ProceduralMeshEditor add/remove", "[Editor][S103]") {
    ProceduralMeshEditor ed;
    REQUIRE(ed.addMesh(ProceduralMeshConfig("m1", ProceduralMeshPrimitive::Box)));
    REQUIRE(ed.addMesh(ProceduralMeshConfig("m2", ProceduralMeshPrimitive::Sphere)));
    REQUIRE(!ed.addMesh(ProceduralMeshConfig("m1", ProceduralMeshPrimitive::Torus)));
    REQUIRE(ed.meshCount() == 2u);
    REQUIRE(ed.removeMesh("m1"));
    REQUIRE(ed.meshCount() == 1u);
    REQUIRE(!ed.removeMesh("m1"));
}

TEST_CASE("ProceduralMeshEditor counts", "[Editor][S103]") {
    ProceduralMeshEditor ed;
    ProceduralMeshConfig c1("a", ProceduralMeshPrimitive::Box);    c1.setDirty(true); c1.setGenerateCollision(true);
    ProceduralMeshConfig c2("b", ProceduralMeshPrimitive::Sphere); c2.setDirty(true);
    ProceduralMeshConfig c3("c", ProceduralMeshPrimitive::Box);
    ed.addMesh(c1); ed.addMesh(c2); ed.addMesh(c3);
    REQUIRE(ed.meshCount()                                          == 3u);
    REQUIRE(ed.dirtyCount()                                         == 2u);
    REQUIRE(ed.collisionCount()                                     == 1u);
    REQUIRE(ed.countByPrimitive(ProceduralMeshPrimitive::Box)       == 2u);
    auto* found = ed.findMesh("b");
    REQUIRE(found != nullptr);
    REQUIRE(found->primitive() == ProceduralMeshPrimitive::Sphere);
    REQUIRE(ed.findMesh("missing") == nullptr);
}

// ── MeshDeformerEditor ───────────────────────────────────────────────────────

TEST_CASE("DeformerType names", "[Editor][S103]") {
    REQUIRE(std::string(deformerTypeName(DeformerType::Blend))    == "Blend");
    REQUIRE(std::string(deformerTypeName(DeformerType::Lattice))  == "Lattice");
    REQUIRE(std::string(deformerTypeName(DeformerType::FFD))      == "FFD");
    REQUIRE(std::string(deformerTypeName(DeformerType::Morph))    == "Morph");
    REQUIRE(std::string(deformerTypeName(DeformerType::SkinWrap)) == "SkinWrap");
    REQUIRE(std::string(deformerTypeName(DeformerType::Wave))     == "Wave");
    REQUIRE(std::string(deformerTypeName(DeformerType::Twist))    == "Twist");
    REQUIRE(std::string(deformerTypeName(DeformerType::Bend))     == "Bend");
    REQUIRE(std::string(deformerTypeName(DeformerType::Noise))    == "Noise");
}

TEST_CASE("DeformerEvalOrder names", "[Editor][S103]") {
    REQUIRE(std::string(deformerEvalOrderName(DeformerEvalOrder::Sequential))   == "Sequential");
    REQUIRE(std::string(deformerEvalOrderName(DeformerEvalOrder::Parallel))     == "Parallel");
    REQUIRE(std::string(deformerEvalOrderName(DeformerEvalOrder::LayeredBlend)) == "LayeredBlend");
}

TEST_CASE("DeformerLayer defaults", "[Editor][S103]") {
    DeformerLayer layer(1, DeformerType::Morph);
    REQUIRE(layer.id()        == 1u);
    REQUIRE(layer.type()      == DeformerType::Morph);
    REQUIRE(layer.weight()    == 1.0f);
    REQUIRE(layer.isEnabled());
    REQUIRE(layer.name()      == "");
}

TEST_CASE("DeformerLayer mutation", "[Editor][S103]") {
    DeformerLayer layer(2, DeformerType::Blend);
    layer.setWeight(0.7f);
    layer.setEnabled(false);
    layer.setName("smile_blend");
    REQUIRE(layer.weight()    == 0.7f);
    REQUIRE(!layer.isEnabled());
    REQUIRE(layer.name()      == "smile_blend");
}

TEST_CASE("MeshDeformerEditor add/remove", "[Editor][S103]") {
    MeshDeformerEditor ed;
    REQUIRE(ed.addLayer(DeformerLayer(1, DeformerType::Morph)));
    REQUIRE(ed.addLayer(DeformerLayer(2, DeformerType::Lattice)));
    REQUIRE(!ed.addLayer(DeformerLayer(1, DeformerType::FFD)));
    REQUIRE(ed.layerCount() == 2u);
    REQUIRE(ed.removeLayer(1));
    REQUIRE(ed.layerCount() == 1u);
    REQUIRE(!ed.removeLayer(1));
}

TEST_CASE("MeshDeformerEditor counts", "[Editor][S103]") {
    MeshDeformerEditor ed;
    ed.setTargetMesh("character_body");
    ed.setEvalOrder(DeformerEvalOrder::LayeredBlend);
    DeformerLayer l1(1, DeformerType::Morph);
    DeformerLayer l2(2, DeformerType::Morph); l2.setEnabled(false);
    DeformerLayer l3(3, DeformerType::Lattice);
    ed.addLayer(l1); ed.addLayer(l2); ed.addLayer(l3);
    REQUIRE(ed.targetMesh()                        == "character_body");
    REQUIRE(ed.evalOrder()                         == DeformerEvalOrder::LayeredBlend);
    REQUIRE(ed.layerCount()                        == 3u);
    REQUIRE(ed.enabledLayerCount()                 == 2u);
    REQUIRE(ed.countByType(DeformerType::Morph)    == 2u);
    auto* found = ed.findLayer(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == DeformerType::Lattice);
    REQUIRE(ed.findLayer(99) == nullptr);
}

// ── SkeletonEditor ───────────────────────────────────────────────────────────

TEST_CASE("BoneRotationOrder names", "[Editor][S103]") {
    REQUIRE(std::string(boneRotationOrderName(BoneRotationOrder::XYZ)) == "XYZ");
    REQUIRE(std::string(boneRotationOrderName(BoneRotationOrder::XZY)) == "XZY");
    REQUIRE(std::string(boneRotationOrderName(BoneRotationOrder::YXZ)) == "YXZ");
    REQUIRE(std::string(boneRotationOrderName(BoneRotationOrder::YZX)) == "YZX");
    REQUIRE(std::string(boneRotationOrderName(BoneRotationOrder::ZXY)) == "ZXY");
    REQUIRE(std::string(boneRotationOrderName(BoneRotationOrder::ZYX)) == "ZYX");
}

TEST_CASE("BoneConstraintType names", "[Editor][S103]") {
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::None))     == "None");
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::PointAt))  == "PointAt");
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::Orient))   == "Orient");
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::Position)) == "Position");
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::Scale))    == "Scale");
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::IK))       == "IK");
    REQUIRE(std::string(boneConstraintTypeName(BoneConstraintType::LookAt))   == "LookAt");
}

TEST_CASE("SkeletonRetargetMode names", "[Editor][S103]") {
    REQUIRE(std::string(skeletonRetargetModeName(SkeletonRetargetMode::None))     == "None");
    REQUIRE(std::string(skeletonRetargetModeName(SkeletonRetargetMode::Humanoid)) == "Humanoid");
    REQUIRE(std::string(skeletonRetargetModeName(SkeletonRetargetMode::Generic))  == "Generic");
    REQUIRE(std::string(skeletonRetargetModeName(SkeletonRetargetMode::Custom))   == "Custom");
}

TEST_CASE("BoneDefinition defaults", "[Editor][S103]") {
    BoneDefinition bone(1, "root");
    REQUIRE(bone.id()            == 1u);
    REQUIRE(bone.name()          == "root");
    REQUIRE(!bone.hasParent());
    REQUIRE(bone.rotationOrder() == BoneRotationOrder::XYZ);
    REQUIRE(bone.constraint()    == BoneConstraintType::None);
    REQUIRE(bone.length()        == 1.0f);
    REQUIRE(!bone.isSelected());
}

TEST_CASE("BoneDefinition mutation", "[Editor][S103]") {
    BoneDefinition bone(2, "arm_l");
    bone.setParentId(1);
    bone.setRotationOrder(BoneRotationOrder::ZXY);
    bone.setConstraint(BoneConstraintType::IK);
    bone.setLength(2.5f);
    bone.setSelected(true);
    REQUIRE(bone.hasParent());
    REQUIRE(bone.parentId()      == 1u);
    REQUIRE(bone.rotationOrder() == BoneRotationOrder::ZXY);
    REQUIRE(bone.constraint()    == BoneConstraintType::IK);
    REQUIRE(bone.length()        == 2.5f);
    REQUIRE(bone.isSelected());
}

TEST_CASE("SkeletonEditor add/remove", "[Editor][S103]") {
    SkeletonEditor ed;
    ed.setName("humanoid_rig");
    ed.setRetargetMode(SkeletonRetargetMode::Humanoid);
    REQUIRE(ed.addBone(BoneDefinition(1, "root")));
    REQUIRE(ed.addBone(BoneDefinition(2, "spine")));
    REQUIRE(!ed.addBone(BoneDefinition(1, "dup")));
    REQUIRE(ed.boneCount() == 2u);
    REQUIRE(ed.name()          == "humanoid_rig");
    REQUIRE(ed.retargetMode()  == SkeletonRetargetMode::Humanoid);
    REQUIRE(ed.removeBone(1));
    REQUIRE(ed.boneCount() == 1u);
    REQUIRE(!ed.removeBone(1));
}

TEST_CASE("SkeletonEditor counts", "[Editor][S103]") {
    SkeletonEditor ed;
    BoneDefinition b1(1, "root");   // no parent => root bone
    BoneDefinition b2(2, "spine");  b2.setParentId(1); b2.setSelected(true);
    BoneDefinition b3(3, "arm_l");  b3.setParentId(2); b3.setConstraint(BoneConstraintType::IK); b3.setSelected(true);
    BoneDefinition b4(4, "arm_r");  b4.setParentId(2); b4.setConstraint(BoneConstraintType::Orient);
    ed.addBone(b1); ed.addBone(b2); ed.addBone(b3); ed.addBone(b4);
    REQUIRE(ed.boneCount()                                    == 4u);
    REQUIRE(ed.rootBoneCount()                                == 1u);
    REQUIRE(ed.selectedBoneCount()                            == 2u);
    REQUIRE(ed.constrainedBoneCount()                         == 2u);
    REQUIRE(ed.countByConstraint(BoneConstraintType::IK)      == 1u);
    auto* found = ed.findBone(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->name() == "arm_l");
    REQUIRE(ed.findBone(99) == nullptr);
}
