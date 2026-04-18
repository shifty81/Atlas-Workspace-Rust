// S199 editor tests: FogVolumeEditorV1, OcclusionVolumeEditorV1, SkydomeEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/FogVolumeEditorV1.h"
#include "NF/Editor/OcclusionVolumeEditorV1.h"
#include "NF/Editor/SkydomeEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── FogVolumeEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Fvv1Volume validity", "[Editor][S199]") {
    Fvv1Volume v;
    REQUIRE(!v.isValid());
    v.id = 1; v.name = "TestFog";
    REQUIRE(v.isValid());
}

TEST_CASE("FogVolumeEditorV1 addVolume and volumeCount", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    REQUIRE(fe.volumeCount() == 0);
    Fvv1Volume v; v.id = 1; v.name = "Fog1";
    REQUIRE(fe.addVolume(v));
    REQUIRE(fe.volumeCount() == 1);
}

TEST_CASE("FogVolumeEditorV1 addVolume invalid fails", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    REQUIRE(!fe.addVolume(Fvv1Volume{}));
}

TEST_CASE("FogVolumeEditorV1 addVolume duplicate fails", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v; v.id = 1; v.name = "A";
    fe.addVolume(v);
    REQUIRE(!fe.addVolume(v));
}

TEST_CASE("FogVolumeEditorV1 removeVolume", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v; v.id = 2; v.name = "B";
    fe.addVolume(v);
    REQUIRE(fe.removeVolume(2));
    REQUIRE(fe.volumeCount() == 0);
    REQUIRE(!fe.removeVolume(2));
}

TEST_CASE("FogVolumeEditorV1 findVolume", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v; v.id = 3; v.name = "C";
    fe.addVolume(v);
    REQUIRE(fe.findVolume(3) != nullptr);
    REQUIRE(fe.findVolume(99) == nullptr);
}

TEST_CASE("FogVolumeEditorV1 setState", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v; v.id = 1; v.name = "V";
    fe.addVolume(v);
    REQUIRE(fe.setState(1, Fvv1VolumeState::Preview));
    REQUIRE(fe.findVolume(1)->state == Fvv1VolumeState::Preview);
}

TEST_CASE("FogVolumeEditorV1 setDensity clamped", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v; v.id = 1; v.name = "V";
    fe.addVolume(v);
    fe.setDensity(1, 2.f);
    REQUIRE(fe.findVolume(1)->density == Approx(1.f));
    fe.setDensity(1, -0.5f);
    REQUIRE(fe.findVolume(1)->density == Approx(0.f));
}

TEST_CASE("FogVolumeEditorV1 setPriority", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v; v.id = 1; v.name = "V";
    fe.addVolume(v);
    REQUIRE(fe.setPriority(1, Fvv1Priority::High));
    REQUIRE(fe.findVolume(1)->priority == Fvv1Priority::High);
}

TEST_CASE("FogVolumeEditorV1 activeCount", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v1; v1.id = 1; v1.name = "A";
    Fvv1Volume v2; v2.id = 2; v2.name = "B";
    fe.addVolume(v1); fe.addVolume(v2);
    fe.setState(2, Fvv1VolumeState::Disabled);
    REQUIRE(fe.activeCount() == 1);
}

TEST_CASE("FogVolumeEditorV1 countByType", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    Fvv1Volume v1; v1.id = 1; v1.name = "A"; v1.type = Fvv1FogType::Box;
    Fvv1Volume v2; v2.id = 2; v2.name = "B"; v2.type = Fvv1FogType::Sphere;
    Fvv1Volume v3; v3.id = 3; v3.name = "C"; v3.type = Fvv1FogType::Box;
    fe.addVolume(v1); fe.addVolume(v2); fe.addVolume(v3);
    REQUIRE(fe.countByType(Fvv1FogType::Box) == 2);
    REQUIRE(fe.countByType(Fvv1FogType::Sphere) == 1);
}

TEST_CASE("FogVolumeEditorV1 onChange callback", "[Editor][S199]") {
    FogVolumeEditorV1 fe;
    uint64_t notified = 0;
    fe.setOnChange([&](uint64_t id){ notified = id; });
    Fvv1Volume v; v.id = 5; v.name = "X";
    fe.addVolume(v);
    REQUIRE(notified == 5);
}

TEST_CASE("fvv1FogTypeName all values", "[Editor][S199]") {
    REQUIRE(std::string(fvv1FogTypeName(Fvv1FogType::Box))    == "Box");
    REQUIRE(std::string(fvv1FogTypeName(Fvv1FogType::Sphere)) == "Sphere");
    REQUIRE(std::string(fvv1FogTypeName(Fvv1FogType::Height)) == "Height");
    REQUIRE(std::string(fvv1FogTypeName(Fvv1FogType::Global)) == "Global");
}

TEST_CASE("fvv1VolumeStateName all values", "[Editor][S199]") {
    REQUIRE(std::string(fvv1VolumeStateName(Fvv1VolumeState::Active))   == "Active");
    REQUIRE(std::string(fvv1VolumeStateName(Fvv1VolumeState::Disabled)) == "Disabled");
    REQUIRE(std::string(fvv1VolumeStateName(Fvv1VolumeState::Preview))  == "Preview");
}

TEST_CASE("fvv1PriorityName all values", "[Editor][S199]") {
    REQUIRE(std::string(fvv1PriorityName(Fvv1Priority::High))   == "High");
    REQUIRE(std::string(fvv1PriorityName(Fvv1Priority::Normal)) == "Normal");
    REQUIRE(std::string(fvv1PriorityName(Fvv1Priority::Low))    == "Low");
}

// ── OcclusionVolumeEditorV1 ─────────────────────────────────────────────────

TEST_CASE("Ovv1Volume validity", "[Editor][S199]") {
    Ovv1Volume v;
    REQUIRE(!v.isValid());
    v.id = 1; v.name = "Occ1";
    REQUIRE(v.isValid());
}

TEST_CASE("OcclusionVolumeEditorV1 addVolume and volumeCount", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    REQUIRE(oe.volumeCount() == 0);
    Ovv1Volume v; v.id = 1; v.name = "O1";
    REQUIRE(oe.addVolume(v));
    REQUIRE(oe.volumeCount() == 1);
}

TEST_CASE("OcclusionVolumeEditorV1 addVolume invalid fails", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    REQUIRE(!oe.addVolume(Ovv1Volume{}));
}

TEST_CASE("OcclusionVolumeEditorV1 addVolume duplicate fails", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v; v.id = 1; v.name = "A";
    oe.addVolume(v);
    REQUIRE(!oe.addVolume(v));
}

TEST_CASE("OcclusionVolumeEditorV1 removeVolume", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v; v.id = 2; v.name = "B";
    oe.addVolume(v);
    REQUIRE(oe.removeVolume(2));
    REQUIRE(oe.volumeCount() == 0);
    REQUIRE(!oe.removeVolume(2));
}

TEST_CASE("OcclusionVolumeEditorV1 findVolume", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v; v.id = 3; v.name = "C";
    oe.addVolume(v);
    REQUIRE(oe.findVolume(3) != nullptr);
    REQUIRE(oe.findVolume(99) == nullptr);
}

TEST_CASE("OcclusionVolumeEditorV1 setState", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v; v.id = 1; v.name = "V";
    oe.addVolume(v);
    REQUIRE(oe.setState(1, Ovv1OccState::Baked));
    REQUIRE(oe.findVolume(1)->state == Ovv1OccState::Baked);
}

TEST_CASE("OcclusionVolumeEditorV1 setBounds", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v; v.id = 1; v.name = "V";
    oe.addVolume(v);
    REQUIRE(oe.setBounds(1, 2.f, 3.f, 4.f));
    REQUIRE(oe.findVolume(1)->boundX == Approx(2.f));
    REQUIRE(oe.findVolume(1)->boundY == Approx(3.f));
    REQUIRE(oe.findVolume(1)->boundZ == Approx(4.f));
}

TEST_CASE("OcclusionVolumeEditorV1 toggleTwoSided", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v; v.id = 1; v.name = "V";
    oe.addVolume(v);
    REQUIRE(!oe.findVolume(1)->isTwoSided);
    REQUIRE(oe.toggleTwoSided(1));
    REQUIRE(oe.findVolume(1)->isTwoSided);
    oe.toggleTwoSided(1);
    REQUIRE(!oe.findVolume(1)->isTwoSided);
}

TEST_CASE("OcclusionVolumeEditorV1 activeCount", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v1; v1.id = 1; v1.name = "A";
    Ovv1Volume v2; v2.id = 2; v2.name = "B";
    oe.addVolume(v1); oe.addVolume(v2);
    oe.setState(2, Ovv1OccState::Disabled);
    REQUIRE(oe.activeCount() == 1);
}

TEST_CASE("OcclusionVolumeEditorV1 countByType", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v1; v1.id = 1; v1.name = "A"; v1.type = Ovv1OccType::Static;
    Ovv1Volume v2; v2.id = 2; v2.name = "B"; v2.type = Ovv1OccType::Portal;
    Ovv1Volume v3; v3.id = 3; v3.name = "C"; v3.type = Ovv1OccType::Static;
    oe.addVolume(v1); oe.addVolume(v2); oe.addVolume(v3);
    REQUIRE(oe.countByType(Ovv1OccType::Static) == 2);
    REQUIRE(oe.countByType(Ovv1OccType::Portal) == 1);
}

TEST_CASE("OcclusionVolumeEditorV1 totalBoundingVolume", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    Ovv1Volume v1; v1.id = 1; v1.name = "A"; v1.boundX = 2.f; v1.boundY = 3.f; v1.boundZ = 4.f;
    Ovv1Volume v2; v2.id = 2; v2.name = "B"; v2.boundX = 1.f; v2.boundY = 1.f; v2.boundZ = 1.f;
    oe.addVolume(v1); oe.addVolume(v2);
    REQUIRE(oe.totalBoundingVolume() == Approx(25.f)); // 24 + 1
}

TEST_CASE("OcclusionVolumeEditorV1 onChange callback", "[Editor][S199]") {
    OcclusionVolumeEditorV1 oe;
    uint64_t notified = 0;
    oe.setOnChange([&](uint64_t id){ notified = id; });
    Ovv1Volume v; v.id = 7; v.name = "X";
    oe.addVolume(v);
    REQUIRE(notified == 7);
}

TEST_CASE("ovv1OccTypeName all values", "[Editor][S199]") {
    REQUIRE(std::string(ovv1OccTypeName(Ovv1OccType::Static))  == "Static");
    REQUIRE(std::string(ovv1OccTypeName(Ovv1OccType::Dynamic)) == "Dynamic");
    REQUIRE(std::string(ovv1OccTypeName(Ovv1OccType::Portal))  == "Portal");
    REQUIRE(std::string(ovv1OccTypeName(Ovv1OccType::Blocker)) == "Blocker");
}

TEST_CASE("ovv1OccStateName all values", "[Editor][S199]") {
    REQUIRE(std::string(ovv1OccStateName(Ovv1OccState::Active))   == "Active");
    REQUIRE(std::string(ovv1OccStateName(Ovv1OccState::Disabled)) == "Disabled");
    REQUIRE(std::string(ovv1OccStateName(Ovv1OccState::Baked))    == "Baked");
}

// ── SkydomeEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Sdv1SkyPreset validity", "[Editor][S199]") {
    Sdv1SkyPreset p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "Sky1";
    REQUIRE(p.isValid());
}

TEST_CASE("SkydomeEditorV1 addPreset and presetCount", "[Editor][S199]") {
    SkydomeEditorV1 se;
    REQUIRE(se.presetCount() == 0);
    Sdv1SkyPreset p; p.id = 1; p.name = "P1";
    REQUIRE(se.addPreset(p));
    REQUIRE(se.presetCount() == 1);
}

TEST_CASE("SkydomeEditorV1 addPreset invalid fails", "[Editor][S199]") {
    SkydomeEditorV1 se;
    REQUIRE(!se.addPreset(Sdv1SkyPreset{}));
}

TEST_CASE("SkydomeEditorV1 addPreset duplicate fails", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p; p.id = 1; p.name = "A";
    se.addPreset(p);
    REQUIRE(!se.addPreset(p));
}

TEST_CASE("SkydomeEditorV1 removePreset", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p; p.id = 2; p.name = "B";
    se.addPreset(p);
    REQUIRE(se.removePreset(2));
    REQUIRE(se.presetCount() == 0);
    REQUIRE(!se.removePreset(2));
}

TEST_CASE("SkydomeEditorV1 findPreset", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p; p.id = 3; p.name = "C";
    se.addPreset(p);
    REQUIRE(se.findPreset(3) != nullptr);
    REQUIRE(se.findPreset(99) == nullptr);
}

TEST_CASE("SkydomeEditorV1 selectPreset", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p; p.id = 1; p.name = "P";
    se.addPreset(p);
    REQUIRE(se.selectPreset(1));
    REQUIRE(se.activePresetId() == 1);
    REQUIRE(!se.selectPreset(99));
}

TEST_CASE("SkydomeEditorV1 setRotation clamped", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p; p.id = 1; p.name = "P";
    se.addPreset(p);
    se.setRotation(1, 400.f);
    REQUIRE(se.findPreset(1)->rotationDeg == Approx(360.f));
    se.setRotation(1, -10.f);
    REQUIRE(se.findPreset(1)->rotationDeg == Approx(0.f));
}

TEST_CASE("SkydomeEditorV1 setExposure clamped", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p; p.id = 1; p.name = "P";
    se.addPreset(p);
    se.setExposure(1, 10.f);
    REQUIRE(se.findPreset(1)->exposure == Approx(5.f));
    se.setExposure(1, -10.f);
    REQUIRE(se.findPreset(1)->exposure == Approx(-5.f));
}

TEST_CASE("SkydomeEditorV1 countByType", "[Editor][S199]") {
    SkydomeEditorV1 se;
    Sdv1SkyPreset p1; p1.id = 1; p1.name = "A"; p1.type = Sdv1SkyType::HDRI;
    Sdv1SkyPreset p2; p2.id = 2; p2.name = "B"; p2.type = Sdv1SkyType::Procedural;
    Sdv1SkyPreset p3; p3.id = 3; p3.name = "C"; p3.type = Sdv1SkyType::HDRI;
    se.addPreset(p1); se.addPreset(p2); se.addPreset(p3);
    REQUIRE(se.countByType(Sdv1SkyType::HDRI) == 2);
    REQUIRE(se.countByType(Sdv1SkyType::Procedural) == 1);
}

TEST_CASE("SkydomeEditorV1 onChange callback", "[Editor][S199]") {
    SkydomeEditorV1 se;
    uint64_t notified = 0;
    se.setOnChange([&](uint64_t id){ notified = id; });
    Sdv1SkyPreset p; p.id = 5; p.name = "X";
    se.addPreset(p);
    REQUIRE(notified == 5);
}

TEST_CASE("sdv1SkyTypeName all values", "[Editor][S199]") {
    REQUIRE(std::string(sdv1SkyTypeName(Sdv1SkyType::Procedural)) == "Procedural");
    REQUIRE(std::string(sdv1SkyTypeName(Sdv1SkyType::HDRI))       == "HDRI");
    REQUIRE(std::string(sdv1SkyTypeName(Sdv1SkyType::Gradient))   == "Gradient");
    REQUIRE(std::string(sdv1SkyTypeName(Sdv1SkyType::Cubemap))    == "Cubemap");
}
