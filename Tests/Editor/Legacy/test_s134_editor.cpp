// S134 editor tests: CameraRigEditor, CameraBlendEditor, CameraShakeEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/CameraShakeEditor.h"
#include "NF/Editor/CameraBlendEditor.h"
#include "NF/Editor/CameraRigEditor.h"

using namespace NF;

// ── CameraRigEditor ───────────────────────────────────────────────────────────

TEST_CASE("CamRigType names", "[Editor][S134]") {
    REQUIRE(std::string(camRigTypeName(CamRigType::Fixed))   == "Fixed");
    REQUIRE(std::string(camRigTypeName(CamRigType::Orbital)) == "Orbital");
    REQUIRE(std::string(camRigTypeName(CamRigType::Follow))  == "Follow");
    REQUIRE(std::string(camRigTypeName(CamRigType::Dolly))   == "Dolly");
    REQUIRE(std::string(camRigTypeName(CamRigType::FreeFly)) == "FreeFly");
}

TEST_CASE("CamRigConstraint names", "[Editor][S134]") {
    REQUIRE(std::string(camRigConstraintName(CamRigConstraint::None))   == "None");
    REQUIRE(std::string(camRigConstraintName(CamRigConstraint::LookAt)) == "LookAt");
    REQUIRE(std::string(camRigConstraintName(CamRigConstraint::Follow)) == "Follow");
    REQUIRE(std::string(camRigConstraintName(CamRigConstraint::Rail))   == "Rail");
    REQUIRE(std::string(camRigConstraintName(CamRigConstraint::Spline)) == "Spline");
}

TEST_CASE("CameraRig defaults", "[Editor][S134]") {
    CameraRig r(1, "main_cam", CamRigType::Follow, CamRigConstraint::LookAt);
    REQUIRE(r.id()         == 1u);
    REQUIRE(r.name()       == "main_cam");
    REQUIRE(r.type()       == CamRigType::Follow);
    REQUIRE(r.constraint() == CamRigConstraint::LookAt);
    REQUIRE(r.fov()        == 60.0f);
    REQUIRE(r.nearClip()   == 0.1f);
    REQUIRE(r.farClip()    == 1000.0f);
    REQUIRE(r.isEnabled());
}

TEST_CASE("CameraRig mutation", "[Editor][S134]") {
    CameraRig r(2, "cinematic", CamRigType::Dolly, CamRigConstraint::Rail);
    r.setFov(90.0f);
    r.setNearClip(0.5f);
    r.setFarClip(500.0f);
    r.setIsEnabled(false);
    REQUIRE(r.fov()      == 90.0f);
    REQUIRE(r.nearClip() == 0.5f);
    REQUIRE(r.farClip()  == 500.0f);
    REQUIRE(!r.isEnabled());
}

TEST_CASE("CameraRigEditor defaults", "[Editor][S134]") {
    CameraRigEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.defaultFov()  == 75.0f);
    REQUIRE(ed.rigCount()    == 0u);
}

TEST_CASE("CameraRigEditor add/remove rigs", "[Editor][S134]") {
    CameraRigEditor ed;
    REQUIRE(ed.addRig(CameraRig(1, "r_a", CamRigType::Fixed,   CamRigConstraint::None)));
    REQUIRE(ed.addRig(CameraRig(2, "r_b", CamRigType::Orbital, CamRigConstraint::LookAt)));
    REQUIRE(ed.addRig(CameraRig(3, "r_c", CamRigType::Follow,  CamRigConstraint::Follow)));
    REQUIRE(!ed.addRig(CameraRig(1, "r_a", CamRigType::Fixed,  CamRigConstraint::None)));
    REQUIRE(ed.rigCount() == 3u);
    REQUIRE(ed.removeRig(2));
    REQUIRE(ed.rigCount() == 2u);
    REQUIRE(!ed.removeRig(99));
}

TEST_CASE("CameraRigEditor counts and find", "[Editor][S134]") {
    CameraRigEditor ed;
    CameraRig r1(1, "a", CamRigType::Fixed,   CamRigConstraint::None);
    CameraRig r2(2, "b", CamRigType::Fixed,   CamRigConstraint::LookAt);
    CameraRig r3(3, "c", CamRigType::Follow,  CamRigConstraint::Spline);
    CameraRig r4(4, "d", CamRigType::FreeFly, CamRigConstraint::None); r4.setIsEnabled(false);
    ed.addRig(r1); ed.addRig(r2); ed.addRig(r3); ed.addRig(r4);
    REQUIRE(ed.countByType(CamRigType::Fixed)                  == 2u);
    REQUIRE(ed.countByType(CamRigType::Follow)                 == 1u);
    REQUIRE(ed.countByType(CamRigType::Orbital)                == 0u);
    REQUIRE(ed.countByConstraint(CamRigConstraint::None)       == 2u);
    REQUIRE(ed.countByConstraint(CamRigConstraint::LookAt)     == 1u);
    REQUIRE(ed.countEnabled()                                  == 3u);
    auto* found = ed.findRig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == CamRigType::Follow);
    REQUIRE(ed.findRig(99) == nullptr);
}

TEST_CASE("CameraRigEditor settings mutation", "[Editor][S134]") {
    CameraRigEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(false);
    ed.setDefaultFov(90.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.defaultFov() == 90.0f);
}

// ── CameraBlendEditor ─────────────────────────────────────────────────────────

TEST_CASE("CamBlendCurve names", "[Editor][S134]") {
    REQUIRE(std::string(camBlendCurveName(CamBlendCurve::Linear))    == "Linear");
    REQUIRE(std::string(camBlendCurveName(CamBlendCurve::EaseIn))    == "EaseIn");
    REQUIRE(std::string(camBlendCurveName(CamBlendCurve::EaseOut))   == "EaseOut");
    REQUIRE(std::string(camBlendCurveName(CamBlendCurve::EaseInOut)) == "EaseInOut");
    REQUIRE(std::string(camBlendCurveName(CamBlendCurve::Custom))    == "Custom");
}

TEST_CASE("CamBlendTrigger names", "[Editor][S134]") {
    REQUIRE(std::string(camBlendTriggerName(CamBlendTrigger::Immediate))  == "Immediate");
    REQUIRE(std::string(camBlendTriggerName(CamBlendTrigger::OnEnter))    == "OnEnter");
    REQUIRE(std::string(camBlendTriggerName(CamBlendTrigger::OnExit))     == "OnExit");
    REQUIRE(std::string(camBlendTriggerName(CamBlendTrigger::OnDistance)) == "OnDistance");
    REQUIRE(std::string(camBlendTriggerName(CamBlendTrigger::Manual))     == "Manual");
}

TEST_CASE("CameraBlend defaults", "[Editor][S134]") {
    CameraBlend b(1, "soft_cut", CamBlendCurve::EaseInOut, CamBlendTrigger::OnEnter);
    REQUIRE(b.id()          == 1u);
    REQUIRE(b.name()        == "soft_cut");
    REQUIRE(b.curve()       == CamBlendCurve::EaseInOut);
    REQUIRE(b.trigger()     == CamBlendTrigger::OnEnter);
    REQUIRE(b.duration()    == 1.0f);
    REQUIRE(b.blendWeight() == 1.0f);
    REQUIRE(b.isEnabled());
}

TEST_CASE("CameraBlend mutation", "[Editor][S134]") {
    CameraBlend b(2, "hard_cut", CamBlendCurve::Linear, CamBlendTrigger::Immediate);
    b.setDuration(0.3f);
    b.setBlendWeight(0.5f);
    b.setIsEnabled(false);
    REQUIRE(b.duration()    == 0.3f);
    REQUIRE(b.blendWeight() == 0.5f);
    REQUIRE(!b.isEnabled());
}

TEST_CASE("CameraBlendEditor defaults", "[Editor][S134]") {
    CameraBlendEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByCurve());
    REQUIRE(ed.defaultDuration() == 0.5f);
    REQUIRE(ed.blendCount()      == 0u);
}

TEST_CASE("CameraBlendEditor add/remove blends", "[Editor][S134]") {
    CameraBlendEditor ed;
    REQUIRE(ed.addBlend(CameraBlend(1, "b_a", CamBlendCurve::Linear,    CamBlendTrigger::Immediate)));
    REQUIRE(ed.addBlend(CameraBlend(2, "b_b", CamBlendCurve::EaseIn,    CamBlendTrigger::OnEnter)));
    REQUIRE(ed.addBlend(CameraBlend(3, "b_c", CamBlendCurve::EaseInOut, CamBlendTrigger::Manual)));
    REQUIRE(!ed.addBlend(CameraBlend(1, "b_a", CamBlendCurve::Linear,   CamBlendTrigger::Immediate)));
    REQUIRE(ed.blendCount() == 3u);
    REQUIRE(ed.removeBlend(2));
    REQUIRE(ed.blendCount() == 2u);
    REQUIRE(!ed.removeBlend(99));
}

TEST_CASE("CameraBlendEditor counts and find", "[Editor][S134]") {
    CameraBlendEditor ed;
    CameraBlend b1(1, "a", CamBlendCurve::Linear,    CamBlendTrigger::Immediate);
    CameraBlend b2(2, "b", CamBlendCurve::Linear,    CamBlendTrigger::OnEnter);
    CameraBlend b3(3, "c", CamBlendCurve::EaseOut,   CamBlendTrigger::OnExit);
    CameraBlend b4(4, "d", CamBlendCurve::Custom,    CamBlendTrigger::Manual); b4.setIsEnabled(false);
    ed.addBlend(b1); ed.addBlend(b2); ed.addBlend(b3); ed.addBlend(b4);
    REQUIRE(ed.countByCurve(CamBlendCurve::Linear)             == 2u);
    REQUIRE(ed.countByCurve(CamBlendCurve::EaseOut)            == 1u);
    REQUIRE(ed.countByCurve(CamBlendCurve::EaseIn)             == 0u);
    REQUIRE(ed.countByTrigger(CamBlendTrigger::Immediate)      == 1u);
    REQUIRE(ed.countByTrigger(CamBlendTrigger::OnEnter)        == 1u);
    REQUIRE(ed.countEnabled()                                  == 3u);
    auto* found = ed.findBlend(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->curve() == CamBlendCurve::EaseOut);
    REQUIRE(ed.findBlend(99) == nullptr);
}

TEST_CASE("CameraBlendEditor settings mutation", "[Editor][S134]") {
    CameraBlendEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByCurve(true);
    ed.setDefaultDuration(2.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByCurve());
    REQUIRE(ed.defaultDuration() == 2.0f);
}

// ── CameraShakeEditor ─────────────────────────────────────────────────────────

TEST_CASE("CamShakeProfile names", "[Editor][S134]") {
    REQUIRE(std::string(camShakeProfileName(CamShakeProfile::Explosion))  == "Explosion");
    REQUIRE(std::string(camShakeProfileName(CamShakeProfile::Earthquake)) == "Earthquake");
    REQUIRE(std::string(camShakeProfileName(CamShakeProfile::Impact))     == "Impact");
    REQUIRE(std::string(camShakeProfileName(CamShakeProfile::Rumble))     == "Rumble");
    REQUIRE(std::string(camShakeProfileName(CamShakeProfile::Custom))     == "Custom");
}

TEST_CASE("CamShakeAxis names", "[Editor][S134]") {
    REQUIRE(std::string(camShakeAxisName(CamShakeAxis::X))   == "X");
    REQUIRE(std::string(camShakeAxisName(CamShakeAxis::Y))   == "Y");
    REQUIRE(std::string(camShakeAxisName(CamShakeAxis::Z))   == "Z");
    REQUIRE(std::string(camShakeAxisName(CamShakeAxis::XY))  == "XY");
    REQUIRE(std::string(camShakeAxisName(CamShakeAxis::XYZ)) == "XYZ");
}

TEST_CASE("CameraShake defaults", "[Editor][S134]") {
    CameraShake s(1, "explosion_hit", CamShakeProfile::Explosion, CamShakeAxis::XYZ);
    REQUIRE(s.id()        == 1u);
    REQUIRE(s.name()      == "explosion_hit");
    REQUIRE(s.profile()   == CamShakeProfile::Explosion);
    REQUIRE(s.axis()      == CamShakeAxis::XYZ);
    REQUIRE(s.amplitude() == 1.0f);
    REQUIRE(s.frequency() == 10.0f);
    REQUIRE(s.decayTime() == 0.5f);
    REQUIRE(s.isEnabled());
}

TEST_CASE("CameraShake mutation", "[Editor][S134]") {
    CameraShake s(2, "rumble", CamShakeProfile::Rumble, CamShakeAxis::XY);
    s.setAmplitude(0.5f);
    s.setFrequency(20.0f);
    s.setDecayTime(1.0f);
    s.setIsEnabled(false);
    REQUIRE(s.amplitude() == 0.5f);
    REQUIRE(s.frequency() == 20.0f);
    REQUIRE(s.decayTime() == 1.0f);
    REQUIRE(!s.isEnabled());
}

TEST_CASE("CameraShakeEditor defaults", "[Editor][S134]") {
    CameraShakeEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByProfile());
    REQUIRE(ed.defaultAmplitude() == 0.5f);
    REQUIRE(ed.shakeCount()       == 0u);
}

TEST_CASE("CameraShakeEditor add/remove shakes", "[Editor][S134]") {
    CameraShakeEditor ed;
    REQUIRE(ed.addShake(CameraShake(1, "s_a", CamShakeProfile::Explosion, CamShakeAxis::XYZ)));
    REQUIRE(ed.addShake(CameraShake(2, "s_b", CamShakeProfile::Impact,    CamShakeAxis::Y)));
    REQUIRE(ed.addShake(CameraShake(3, "s_c", CamShakeProfile::Rumble,    CamShakeAxis::XY)));
    REQUIRE(!ed.addShake(CameraShake(1, "s_a", CamShakeProfile::Explosion, CamShakeAxis::XYZ)));
    REQUIRE(ed.shakeCount() == 3u);
    REQUIRE(ed.removeShake(2));
    REQUIRE(ed.shakeCount() == 2u);
    REQUIRE(!ed.removeShake(99));
}

TEST_CASE("CameraShakeEditor counts and find", "[Editor][S134]") {
    CameraShakeEditor ed;
    CameraShake s1(1, "a", CamShakeProfile::Explosion,  CamShakeAxis::XYZ);
    CameraShake s2(2, "b", CamShakeProfile::Explosion,  CamShakeAxis::X);
    CameraShake s3(3, "c", CamShakeProfile::Earthquake, CamShakeAxis::XY);
    CameraShake s4(4, "d", CamShakeProfile::Custom,     CamShakeAxis::Z); s4.setIsEnabled(false);
    ed.addShake(s1); ed.addShake(s2); ed.addShake(s3); ed.addShake(s4);
    REQUIRE(ed.countByProfile(CamShakeProfile::Explosion)  == 2u);
    REQUIRE(ed.countByProfile(CamShakeProfile::Earthquake) == 1u);
    REQUIRE(ed.countByProfile(CamShakeProfile::Impact)     == 0u);
    REQUIRE(ed.countByAxis(CamShakeAxis::XYZ)              == 1u);
    REQUIRE(ed.countByAxis(CamShakeAxis::X)                == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findShake(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->profile() == CamShakeProfile::Earthquake);
    REQUIRE(ed.findShake(99) == nullptr);
}

TEST_CASE("CameraShakeEditor settings mutation", "[Editor][S134]") {
    CameraShakeEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByProfile(false);
    ed.setDefaultAmplitude(2.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByProfile());
    REQUIRE(ed.defaultAmplitude() == 2.0f);
}
