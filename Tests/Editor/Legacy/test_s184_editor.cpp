// S184 editor tests: CinematicEditorV1, DecalEditorV1, LightingRigEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/CinematicEditorV1.h"
#include "NF/Editor/DecalEditorV1.h"
#include "NF/Editor/LightingRigEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── CinematicEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Cev1Shot validity", "[Editor][S184]") {
    Cev1Shot s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "OpeningShot"; s.duration = 3.0f;
    REQUIRE(s.isValid());
}

TEST_CASE("CinematicEditorV1 addShot and shotCount", "[Editor][S184]") {
    CinematicEditorV1 ce;
    REQUIRE(ce.shotCount() == 0);
    Cev1Shot s; s.id = 1; s.name = "S1"; s.duration = 2.0f;
    REQUIRE(ce.addShot(s));
    REQUIRE(ce.shotCount() == 1);
}

TEST_CASE("CinematicEditorV1 addShot invalid fails", "[Editor][S184]") {
    CinematicEditorV1 ce;
    REQUIRE(!ce.addShot(Cev1Shot{}));
}

TEST_CASE("CinematicEditorV1 addShot duplicate fails", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 1; s.name = "A"; s.duration = 1.0f;
    ce.addShot(s);
    REQUIRE(!ce.addShot(s));
}

TEST_CASE("CinematicEditorV1 removeShot", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 2; s.name = "B"; s.duration = 1.5f;
    ce.addShot(s);
    REQUIRE(ce.removeShot(2));
    REQUIRE(ce.shotCount() == 0);
    REQUIRE(!ce.removeShot(2));
}

TEST_CASE("CinematicEditorV1 findShot", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 3; s.name = "C"; s.duration = 2.0f;
    ce.addShot(s);
    REQUIRE(ce.findShot(3) != nullptr);
    REQUIRE(ce.findShot(99) == nullptr);
}

TEST_CASE("CinematicEditorV1 setActive and activeId", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 1; s.name = "Intro"; s.duration = 2.0f;
    ce.addShot(s);
    REQUIRE(ce.setActive(1));
    REQUIRE(ce.activeId() == 1);
    REQUIRE(!ce.setActive(99));
}

TEST_CASE("CinematicEditorV1 setEditState", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 1; s.name = "A"; s.duration = 1.0f;
    ce.addShot(s);
    REQUIRE(ce.setEditState(1, Cev1EditState::Approved));
    REQUIRE(ce.findShot(1)->isApproved());
    REQUIRE(!ce.setEditState(99, Cev1EditState::Approved));
}

TEST_CASE("CinematicEditorV1 setDuration", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s; s.id = 1; s.name = "A"; s.duration = 1.0f;
    ce.addShot(s);
    REQUIRE(ce.setDuration(1, 5.0f));
    REQUIRE(ce.findShot(1)->duration == Approx(5.0f));
    REQUIRE(!ce.setDuration(1, -1.0f));
}

TEST_CASE("CinematicEditorV1 approvedCount", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s1; s1.id = 1; s1.name = "A"; s1.duration = 1.0f; s1.editState = Cev1EditState::Approved;
    Cev1Shot s2; s2.id = 2; s2.name = "B"; s2.duration = 1.0f;
    ce.addShot(s1); ce.addShot(s2);
    REQUIRE(ce.approvedCount() == 1);
}

TEST_CASE("CinematicEditorV1 countByType", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s1; s1.id = 1; s1.name = "A"; s1.duration = 1.0f; s1.type = Cev1ShotType::CutScene;
    Cev1Shot s2; s2.id = 2; s2.name = "B"; s2.duration = 1.0f; s2.type = Cev1ShotType::Gameplay;
    Cev1Shot s3; s3.id = 3; s3.name = "C"; s3.duration = 1.0f; s3.type = Cev1ShotType::CutScene;
    ce.addShot(s1); ce.addShot(s2); ce.addShot(s3);
    REQUIRE(ce.countByType(Cev1ShotType::CutScene) == 2);
    REQUIRE(ce.countByType(Cev1ShotType::Gameplay) == 1);
}

TEST_CASE("CinematicEditorV1 totalDuration", "[Editor][S184]") {
    CinematicEditorV1 ce;
    Cev1Shot s1; s1.id = 1; s1.name = "A"; s1.duration = 3.5f;
    Cev1Shot s2; s2.id = 2; s2.name = "B"; s2.duration = 2.5f;
    ce.addShot(s1); ce.addShot(s2);
    REQUIRE(ce.totalDuration() == Approx(6.0f));
}

TEST_CASE("CinematicEditorV1 onChange callback on setEditState", "[Editor][S184]") {
    CinematicEditorV1 ce;
    uint64_t notified = 0;
    ce.setOnChange([&](uint64_t id) { notified = id; });
    Cev1Shot s; s.id = 5; s.name = "F"; s.duration = 1.0f;
    ce.addShot(s);
    ce.setEditState(5, Cev1EditState::Locked);
    REQUIRE(notified == 5);
}

TEST_CASE("Cev1Shot endTime helper", "[Editor][S184]") {
    Cev1Shot s; s.id = 1; s.name = "X"; s.startTime = 2.0f; s.duration = 3.0f;
    REQUIRE(s.endTime() == Approx(5.0f));
}

TEST_CASE("cev1ShotTypeName all values", "[Editor][S184]") {
    REQUIRE(std::string(cev1ShotTypeName(Cev1ShotType::CutScene))   == "CutScene");
    REQUIRE(std::string(cev1ShotTypeName(Cev1ShotType::Gameplay))   == "Gameplay");
    REQUIRE(std::string(cev1ShotTypeName(Cev1ShotType::Transition)) == "Transition");
    REQUIRE(std::string(cev1ShotTypeName(Cev1ShotType::StingerCut)) == "StingerCut");
}

TEST_CASE("cev1PlaybackModeName all values", "[Editor][S184]") {
    REQUIRE(std::string(cev1PlaybackModeName(Cev1PlaybackMode::Once))     == "Once");
    REQUIRE(std::string(cev1PlaybackModeName(Cev1PlaybackMode::Loop))     == "Loop");
    REQUIRE(std::string(cev1PlaybackModeName(Cev1PlaybackMode::PingPong)) == "PingPong");
    REQUIRE(std::string(cev1PlaybackModeName(Cev1PlaybackMode::Hold))     == "Hold");
}

// ── DecalEditorV1 ────────────────────────────────────────────────────────────

TEST_CASE("Dev1Decal validity", "[Editor][S184]") {
    Dev1Decal d;
    REQUIRE(!d.isValid());
    d.id = 1; d.name = "BulletHole";
    REQUIRE(d.isValid());
}

TEST_CASE("DecalEditorV1 addDecal and decalCount", "[Editor][S184]") {
    DecalEditorV1 de;
    REQUIRE(de.decalCount() == 0);
    Dev1Decal d; d.id = 1; d.name = "D1";
    REQUIRE(de.addDecal(d));
    REQUIRE(de.decalCount() == 1);
}

TEST_CASE("DecalEditorV1 addDecal invalid fails", "[Editor][S184]") {
    DecalEditorV1 de;
    REQUIRE(!de.addDecal(Dev1Decal{}));
}

TEST_CASE("DecalEditorV1 addDecal duplicate fails", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1Decal d; d.id = 1; d.name = "A";
    de.addDecal(d);
    REQUIRE(!de.addDecal(d));
}

TEST_CASE("DecalEditorV1 removeDecal", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1Decal d; d.id = 2; d.name = "B";
    de.addDecal(d);
    REQUIRE(de.removeDecal(2));
    REQUIRE(de.decalCount() == 0);
    REQUIRE(!de.removeDecal(2));
}

TEST_CASE("DecalEditorV1 findDecal", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1Decal d; d.id = 3; d.name = "C";
    de.addDecal(d);
    REQUIRE(de.findDecal(3) != nullptr);
    REQUIRE(de.findDecal(99) == nullptr);
}

TEST_CASE("DecalEditorV1 addZone and zoneCount", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1PlacementZone z; z.id = 1; z.name = "Floor";
    REQUIRE(de.addZone(z));
    REQUIRE(de.zoneCount() == 1);
}

TEST_CASE("DecalEditorV1 removeZone", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1PlacementZone z; z.id = 1; z.name = "Wall";
    de.addZone(z);
    REQUIRE(de.removeZone(1));
    REQUIRE(de.zoneCount() == 0);
}

TEST_CASE("DecalEditorV1 activeCount", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1Decal d1; d1.id = 1; d1.name = "A"; d1.state = Dev1DecalState::Active;
    Dev1Decal d2; d2.id = 2; d2.name = "B";
    de.addDecal(d1); de.addDecal(d2);
    REQUIRE(de.activeCount() == 1);
}

TEST_CASE("DecalEditorV1 countByType", "[Editor][S184]") {
    DecalEditorV1 de;
    Dev1Decal d1; d1.id = 1; d1.name = "A"; d1.decalType = Dev1DecalType::Damage;
    Dev1Decal d2; d2.id = 2; d2.name = "B"; d2.decalType = Dev1DecalType::Blood;
    Dev1Decal d3; d3.id = 3; d3.name = "C"; d3.decalType = Dev1DecalType::Damage;
    de.addDecal(d1); de.addDecal(d2); de.addDecal(d3);
    REQUIRE(de.countByType(Dev1DecalType::Damage) == 2);
    REQUIRE(de.countByType(Dev1DecalType::Blood) == 1);
}

TEST_CASE("DecalEditorV1 onChange callback", "[Editor][S184]") {
    DecalEditorV1 de;
    uint64_t notified = 0;
    de.setOnChange([&](uint64_t id) { notified = id; });
    Dev1Decal d; d.id = 7; d.name = "G";
    de.addDecal(d);
    REQUIRE(notified == 7);
}

TEST_CASE("Dev1Decal state helpers", "[Editor][S184]") {
    Dev1Decal d; d.id = 1; d.name = "X";
    d.state = Dev1DecalState::Active;   REQUIRE(d.isActive());
    d.state = Dev1DecalState::Fading;   REQUIRE(d.isFading());
    d.state = Dev1DecalState::Removed;  REQUIRE(d.isRemoved());
}

TEST_CASE("dev1DecalTypeName all values", "[Editor][S184]") {
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Damage))   == "Damage");
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Blood))    == "Blood");
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Dirt))     == "Dirt");
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Graffiti)) == "Graffiti");
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Burn))     == "Burn");
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Wet))      == "Wet");
    REQUIRE(std::string(dev1DecalTypeName(Dev1DecalType::Custom))   == "Custom");
}

// ── LightingRigEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Lrev1Light validity", "[Editor][S184]") {
    Lrev1Light l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "Sun";
    REQUIRE(l.isValid());
}

TEST_CASE("LightingRigEditorV1 addLight and lightCount", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    REQUIRE(lre.lightCount() == 0);
    Lrev1Light l; l.id = 1; l.name = "L1";
    REQUIRE(lre.addLight(l));
    REQUIRE(lre.lightCount() == 1);
}

TEST_CASE("LightingRigEditorV1 addLight invalid fails", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    REQUIRE(!lre.addLight(Lrev1Light{}));
}

TEST_CASE("LightingRigEditorV1 addLight duplicate fails", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Light l; l.id = 1; l.name = "A";
    lre.addLight(l);
    REQUIRE(!lre.addLight(l));
}

TEST_CASE("LightingRigEditorV1 removeLight", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Light l; l.id = 2; l.name = "B";
    lre.addLight(l);
    REQUIRE(lre.removeLight(2));
    REQUIRE(lre.lightCount() == 0);
    REQUIRE(!lre.removeLight(2));
}

TEST_CASE("LightingRigEditorV1 findLight", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Light l; l.id = 3; l.name = "C";
    lre.addLight(l);
    REQUIRE(lre.findLight(3) != nullptr);
    REQUIRE(lre.findLight(99) == nullptr);
}

TEST_CASE("LightingRigEditorV1 addRig and rigCount", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Rig r; r.id = 1; r.name = "DayRig";
    REQUIRE(lre.addRig(r));
    REQUIRE(lre.rigCount() == 1);
}

TEST_CASE("LightingRigEditorV1 removeRig", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Rig r; r.id = 1; r.name = "NightRig";
    lre.addRig(r);
    REQUIRE(lre.removeRig(1));
    REQUIRE(lre.rigCount() == 0);
}

TEST_CASE("LightingRigEditorV1 onCount", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Light l1; l1.id = 1; l1.name = "A"; l1.state = Lrev1LightState::On;
    Lrev1Light l2; l2.id = 2; l2.name = "B";
    lre.addLight(l1); lre.addLight(l2);
    REQUIRE(lre.onCount() == 1);
}

TEST_CASE("LightingRigEditorV1 countByType", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Light l1; l1.id = 1; l1.name = "A"; l1.lightType = Lrev1LightType::Point;
    Lrev1Light l2; l2.id = 2; l2.name = "B"; l2.lightType = Lrev1LightType::Spot;
    Lrev1Light l3; l3.id = 3; l3.name = "C"; l3.lightType = Lrev1LightType::Point;
    lre.addLight(l1); lre.addLight(l2); lre.addLight(l3);
    REQUIRE(lre.countByType(Lrev1LightType::Point) == 2);
    REQUIRE(lre.countByType(Lrev1LightType::Spot) == 1);
}

TEST_CASE("LightingRigEditorV1 totalIntensity", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    Lrev1Light l1; l1.id = 1; l1.name = "A"; l1.intensity = 2.0f;
    Lrev1Light l2; l2.id = 2; l2.name = "B"; l2.intensity = 3.0f;
    lre.addLight(l1); lre.addLight(l2);
    REQUIRE(lre.totalIntensity() == Approx(5.0f));
}

TEST_CASE("LightingRigEditorV1 onChange callback", "[Editor][S184]") {
    LightingRigEditorV1 lre;
    uint64_t notified = 0;
    lre.setOnChange([&](uint64_t id) { notified = id; });
    Lrev1Light l; l.id = 9; l.name = "I";
    lre.addLight(l);
    REQUIRE(notified == 9);
}

TEST_CASE("Lrev1Light state helpers", "[Editor][S184]") {
    Lrev1Light l; l.id = 1; l.name = "X";
    l.state = Lrev1LightState::Baking;  REQUIRE(l.isBaking());
    l.state = Lrev1LightState::Preview; REQUIRE(l.isPreview());
    l.state = Lrev1LightState::On;      REQUIRE(l.isOn());
}

TEST_CASE("lrev1LightTypeName all values", "[Editor][S184]") {
    REQUIRE(std::string(lrev1LightTypeName(Lrev1LightType::Point))       == "Point");
    REQUIRE(std::string(lrev1LightTypeName(Lrev1LightType::Spot))        == "Spot");
    REQUIRE(std::string(lrev1LightTypeName(Lrev1LightType::Directional)) == "Directional");
    REQUIRE(std::string(lrev1LightTypeName(Lrev1LightType::Area))        == "Area");
    REQUIRE(std::string(lrev1LightTypeName(Lrev1LightType::Emissive))    == "Emissive");
    REQUIRE(std::string(lrev1LightTypeName(Lrev1LightType::Ambient))     == "Ambient");
}
