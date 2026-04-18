// S189 editor tests: SkeletonEditorV1, MorphTargetEditorV1, IKRigEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SkeletonEditorV1.h"
#include "NF/Editor/MorphTargetEditorV1.h"
#include "NF/Editor/IKRigEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── SkeletonEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Skev1Bone validity", "[Editor][S189]") {
    Skev1Bone b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "Hip";
    REQUIRE(b.isValid());
}

TEST_CASE("SkeletonEditorV1 addBone and boneCount", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    REQUIRE(ske.boneCount() == 0);
    Skev1Bone b; b.id = 1; b.name = "Root";
    REQUIRE(ske.addBone(b));
    REQUIRE(ske.boneCount() == 1);
}

TEST_CASE("SkeletonEditorV1 addBone invalid fails", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    REQUIRE(!ske.addBone(Skev1Bone{}));
}

TEST_CASE("SkeletonEditorV1 addBone duplicate fails", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Bone b; b.id = 1; b.name = "A";
    ske.addBone(b);
    REQUIRE(!ske.addBone(b));
}

TEST_CASE("SkeletonEditorV1 removeBone", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Bone b; b.id = 2; b.name = "B";
    ske.addBone(b);
    REQUIRE(ske.removeBone(2));
    REQUIRE(ske.boneCount() == 0);
    REQUIRE(!ske.removeBone(2));
}

TEST_CASE("SkeletonEditorV1 findBone", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Bone b; b.id = 3; b.name = "C";
    ske.addBone(b);
    REQUIRE(ske.findBone(3) != nullptr);
    REQUIRE(ske.findBone(99) == nullptr);
}

TEST_CASE("SkeletonEditorV1 addJoint and jointCount", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Joint j; j.id = 1; j.boneId = 10; j.name = "HipJoint";
    REQUIRE(ske.addJoint(j));
    REQUIRE(ske.jointCount() == 1);
}

TEST_CASE("SkeletonEditorV1 removeJoint", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Joint j; j.id = 1; j.boneId = 10; j.name = "J";
    ske.addJoint(j);
    REQUIRE(ske.removeJoint(1));
    REQUIRE(ske.jointCount() == 0);
}

TEST_CASE("SkeletonEditorV1 rootBoneCount", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Bone b1; b1.id = 1; b1.name = "Root"; b1.parentId = 0;
    Skev1Bone b2; b2.id = 2; b2.name = "Spine"; b2.parentId = 1;
    ske.addBone(b1); ske.addBone(b2);
    REQUIRE(ske.rootBoneCount() == 1);
}

TEST_CASE("SkeletonEditorV1 selectedCount", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Bone b1; b1.id = 1; b1.name = "A"; b1.state = Skev1BoneState::Selected;
    Skev1Bone b2; b2.id = 2; b2.name = "B";
    ske.addBone(b1); ske.addBone(b2);
    REQUIRE(ske.selectedCount() == 1);
}

TEST_CASE("SkeletonEditorV1 countByType", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Bone b1; b1.id = 1; b1.name = "A"; b1.boneType = Skev1BoneType::Limb;
    Skev1Bone b2; b2.id = 2; b2.name = "B"; b2.boneType = Skev1BoneType::Finger;
    Skev1Bone b3; b3.id = 3; b3.name = "C"; b3.boneType = Skev1BoneType::Limb;
    ske.addBone(b1); ske.addBone(b2); ske.addBone(b3);
    REQUIRE(ske.countByType(Skev1BoneType::Limb) == 2);
    REQUIRE(ske.countByType(Skev1BoneType::Finger) == 1);
}

TEST_CASE("SkeletonEditorV1 jointsForBone", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    Skev1Joint j1; j1.id = 1; j1.boneId = 10; j1.name = "A";
    Skev1Joint j2; j2.id = 2; j2.boneId = 10; j2.name = "B";
    Skev1Joint j3; j3.id = 3; j3.boneId = 20; j3.name = "C";
    ske.addJoint(j1); ske.addJoint(j2); ske.addJoint(j3);
    REQUIRE(ske.jointsForBone(10) == 2);
    REQUIRE(ske.jointsForBone(20) == 1);
}

TEST_CASE("SkeletonEditorV1 onChange callback", "[Editor][S189]") {
    SkeletonEditorV1 ske;
    uint64_t notified = 0;
    ske.setOnChange([&](uint64_t id) { notified = id; });
    Skev1Bone b; b.id = 4; b.name = "D";
    ske.addBone(b);
    REQUIRE(notified == 4);
}

TEST_CASE("Skev1Bone state helpers", "[Editor][S189]") {
    Skev1Bone b; b.id = 1; b.name = "X";
    b.state = Skev1BoneState::Selected;  REQUIRE(b.isSelected());
    b.state = Skev1BoneState::Hidden;    REQUIRE(b.isHidden());
    b.state = Skev1BoneState::Deforming; REQUIRE(b.isDeforming());
}

TEST_CASE("skev1BoneTypeName all values", "[Editor][S189]") {
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Root))   == "Root");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Spine))  == "Spine");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Limb))   == "Limb");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Finger)) == "Finger");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Toe))    == "Toe");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Head))   == "Head");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Tail))   == "Tail");
    REQUIRE(std::string(skev1BoneTypeName(Skev1BoneType::Helper)) == "Helper");
}

// ── MorphTargetEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Mtev1Target validity", "[Editor][S189]") {
    Mtev1Target t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Smile";
    REQUIRE(t.isValid());
}

TEST_CASE("MorphTargetEditorV1 addTarget and targetCount", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    REQUIRE(mte.targetCount() == 0);
    Mtev1Target t; t.id = 1; t.name = "T1";
    REQUIRE(mte.addTarget(t));
    REQUIRE(mte.targetCount() == 1);
}

TEST_CASE("MorphTargetEditorV1 addTarget invalid fails", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    REQUIRE(!mte.addTarget(Mtev1Target{}));
}

TEST_CASE("MorphTargetEditorV1 addTarget duplicate fails", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1Target t; t.id = 1; t.name = "A";
    mte.addTarget(t);
    REQUIRE(!mte.addTarget(t));
}

TEST_CASE("MorphTargetEditorV1 removeTarget", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1Target t; t.id = 2; t.name = "B";
    mte.addTarget(t);
    REQUIRE(mte.removeTarget(2));
    REQUIRE(mte.targetCount() == 0);
    REQUIRE(!mte.removeTarget(2));
}

TEST_CASE("MorphTargetEditorV1 findTarget", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1Target t; t.id = 3; t.name = "C";
    mte.addTarget(t);
    REQUIRE(mte.findTarget(3) != nullptr);
    REQUIRE(mte.findTarget(99) == nullptr);
}

TEST_CASE("MorphTargetEditorV1 addBlendShape and blendShapeCount", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1BlendShape s; s.id = 1; s.name = "MouthOpen";
    REQUIRE(mte.addBlendShape(s));
    REQUIRE(mte.blendShapeCount() == 1);
}

TEST_CASE("MorphTargetEditorV1 removeBlendShape", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1BlendShape s; s.id = 1; s.name = "EyeBlink";
    mte.addBlendShape(s);
    REQUIRE(mte.removeBlendShape(1));
    REQUIRE(mte.blendShapeCount() == 0);
}

TEST_CASE("MorphTargetEditorV1 activeCount", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1Target t1; t1.id = 1; t1.name = "A"; t1.state = Mtev1TargetState::Active;
    Mtev1Target t2; t2.id = 2; t2.name = "B";
    mte.addTarget(t1); mte.addTarget(t2);
    REQUIRE(mte.activeCount() == 1);
}

TEST_CASE("MorphTargetEditorV1 lockedCount", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1Target t1; t1.id = 1; t1.name = "A"; t1.state = Mtev1TargetState::Locked;
    Mtev1Target t2; t2.id = 2; t2.name = "B"; t2.state = Mtev1TargetState::Active;
    mte.addTarget(t1); mte.addTarget(t2);
    REQUIRE(mte.lockedCount() == 1);
}

TEST_CASE("MorphTargetEditorV1 countByCategory", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    Mtev1Target t1; t1.id = 1; t1.name = "A"; t1.category = Mtev1TargetCategory::Facial;
    Mtev1Target t2; t2.id = 2; t2.name = "B"; t2.category = Mtev1TargetCategory::Body;
    Mtev1Target t3; t3.id = 3; t3.name = "C"; t3.category = Mtev1TargetCategory::Facial;
    mte.addTarget(t1); mte.addTarget(t2); mte.addTarget(t3);
    REQUIRE(mte.countByCategory(Mtev1TargetCategory::Facial) == 2);
    REQUIRE(mte.countByCategory(Mtev1TargetCategory::Body) == 1);
}

TEST_CASE("MorphTargetEditorV1 onChange callback", "[Editor][S189]") {
    MorphTargetEditorV1 mte;
    uint64_t notified = 0;
    mte.setOnChange([&](uint64_t id) { notified = id; });
    Mtev1Target t; t.id = 6; t.name = "F";
    mte.addTarget(t);
    REQUIRE(notified == 6);
}

TEST_CASE("Mtev1Target state helpers", "[Editor][S189]") {
    Mtev1Target t; t.id = 1; t.name = "X";
    t.state = Mtev1TargetState::Active;  REQUIRE(t.isActive());
    t.state = Mtev1TargetState::Preview; REQUIRE(t.isPreview());
    t.state = Mtev1TargetState::Locked;  REQUIRE(t.isLocked());
}

TEST_CASE("mtev1TargetCategoryName all values", "[Editor][S189]") {
    REQUIRE(std::string(mtev1TargetCategoryName(Mtev1TargetCategory::Facial))     == "Facial");
    REQUIRE(std::string(mtev1TargetCategoryName(Mtev1TargetCategory::Body))       == "Body");
    REQUIRE(std::string(mtev1TargetCategoryName(Mtev1TargetCategory::Corrective)) == "Corrective");
    REQUIRE(std::string(mtev1TargetCategoryName(Mtev1TargetCategory::Custom))     == "Custom");
}

// ── IKRigEditorV1 ────────────────────────────────────────────────────────────

TEST_CASE("Ikev1Chain validity", "[Editor][S189]") {
    Ikev1Chain c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "ArmChain";
    REQUIRE(c.isValid());
}

TEST_CASE("IKRigEditorV1 addChain and chainCount", "[Editor][S189]") {
    IKRigEditorV1 ike;
    REQUIRE(ike.chainCount() == 0);
    Ikev1Chain c; c.id = 1; c.name = "LegIK";
    REQUIRE(ike.addChain(c));
    REQUIRE(ike.chainCount() == 1);
}

TEST_CASE("IKRigEditorV1 addChain invalid fails", "[Editor][S189]") {
    IKRigEditorV1 ike;
    REQUIRE(!ike.addChain(Ikev1Chain{}));
}

TEST_CASE("IKRigEditorV1 addChain duplicate fails", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Chain c; c.id = 1; c.name = "A";
    ike.addChain(c);
    REQUIRE(!ike.addChain(c));
}

TEST_CASE("IKRigEditorV1 removeChain", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Chain c; c.id = 2; c.name = "B";
    ike.addChain(c);
    REQUIRE(ike.removeChain(2));
    REQUIRE(ike.chainCount() == 0);
    REQUIRE(!ike.removeChain(2));
}

TEST_CASE("IKRigEditorV1 findChain", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Chain c; c.id = 3; c.name = "C";
    ike.addChain(c);
    REQUIRE(ike.findChain(3) != nullptr);
    REQUIRE(ike.findChain(99) == nullptr);
}

TEST_CASE("IKRigEditorV1 addEffector and effectorCount", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Effector e; e.id = 1; e.chainId = 10; e.name = "FootTarget";
    REQUIRE(ike.addEffector(e));
    REQUIRE(ike.effectorCount() == 1);
}

TEST_CASE("IKRigEditorV1 removeEffector", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Effector e; e.id = 1; e.chainId = 10; e.name = "HandTarget";
    ike.addEffector(e);
    REQUIRE(ike.removeEffector(1));
    REQUIRE(ike.effectorCount() == 0);
}

TEST_CASE("IKRigEditorV1 activeCount", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Chain c1; c1.id = 1; c1.name = "A"; c1.state = Ikev1ChainState::Active;
    Ikev1Chain c2; c2.id = 2; c2.name = "B";
    ike.addChain(c1); ike.addChain(c2);
    REQUIRE(ike.activeCount() == 1);
}

TEST_CASE("IKRigEditorV1 disabledCount", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Chain c1; c1.id = 1; c1.name = "A"; c1.state = Ikev1ChainState::Disabled;
    Ikev1Chain c2; c2.id = 2; c2.name = "B"; c2.state = Ikev1ChainState::Active;
    ike.addChain(c1); ike.addChain(c2);
    REQUIRE(ike.disabledCount() == 1);
}

TEST_CASE("IKRigEditorV1 countByType", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Chain c1; c1.id = 1; c1.name = "A"; c1.chainType = Ikev1ChainType::FABRIK;
    Ikev1Chain c2; c2.id = 2; c2.name = "B"; c2.chainType = Ikev1ChainType::TwoBone;
    Ikev1Chain c3; c3.id = 3; c3.name = "C"; c3.chainType = Ikev1ChainType::FABRIK;
    ike.addChain(c1); ike.addChain(c2); ike.addChain(c3);
    REQUIRE(ike.countByType(Ikev1ChainType::FABRIK) == 2);
    REQUIRE(ike.countByType(Ikev1ChainType::TwoBone) == 1);
}

TEST_CASE("IKRigEditorV1 effectorsForChain", "[Editor][S189]") {
    IKRigEditorV1 ike;
    Ikev1Effector e1; e1.id = 1; e1.chainId = 10; e1.name = "A";
    Ikev1Effector e2; e2.id = 2; e2.chainId = 10; e2.name = "B";
    Ikev1Effector e3; e3.id = 3; e3.chainId = 20; e3.name = "C";
    ike.addEffector(e1); ike.addEffector(e2); ike.addEffector(e3);
    REQUIRE(ike.effectorsForChain(10) == 2);
    REQUIRE(ike.effectorsForChain(20) == 1);
}

TEST_CASE("IKRigEditorV1 onChange callback", "[Editor][S189]") {
    IKRigEditorV1 ike;
    uint64_t notified = 0;
    ike.setOnChange([&](uint64_t id) { notified = id; });
    Ikev1Chain c; c.id = 7; c.name = "G";
    ike.addChain(c);
    REQUIRE(notified == 7);
}

TEST_CASE("Ikev1Chain state helpers", "[Editor][S189]") {
    Ikev1Chain c; c.id = 1; c.name = "X";
    c.state = Ikev1ChainState::Active;   REQUIRE(c.isActive());
    c.state = Ikev1ChainState::Blending; REQUIRE(c.isBlending());
    c.state = Ikev1ChainState::Disabled; REQUIRE(c.isDisabled());
}

TEST_CASE("ikev1ChainTypeName all values", "[Editor][S189]") {
    REQUIRE(std::string(ikev1ChainTypeName(Ikev1ChainType::TwoBone)) == "TwoBone");
    REQUIRE(std::string(ikev1ChainTypeName(Ikev1ChainType::FABRIK))  == "FABRIK");
    REQUIRE(std::string(ikev1ChainTypeName(Ikev1ChainType::CCD))     == "CCD");
    REQUIRE(std::string(ikev1ChainTypeName(Ikev1ChainType::Spline))  == "Spline");
    REQUIRE(std::string(ikev1ChainTypeName(Ikev1ChainType::Aim))     == "Aim");
    REQUIRE(std::string(ikev1ChainTypeName(Ikev1ChainType::LookAt))  == "LookAt");
}
