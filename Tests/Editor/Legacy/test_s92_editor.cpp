#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ReflectionProbeEditor.h"
#include "NF/Editor/LightEditor.h"
#include "NF/Editor/DecalEditor.h"

using namespace NF;

// ── S92: DecalEditor + LightEditor + ReflectionProbeEditor ───────

// ── DecalEditor ──────────────────────────────────────────────────

TEST_CASE("DecalBlendMode names are correct", "[Editor][S92]") {
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Translucent)) == "Translucent");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Additive))    == "Additive");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Multiply))    == "Multiply");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Normal))      == "Normal");
    REQUIRE(std::string(decalBlendModeName(DecalBlendMode::Emissive))    == "Emissive");
}

TEST_CASE("DecalProjection names are correct", "[Editor][S92]") {
    REQUIRE(std::string(decalProjectionName(DecalProjection::Box))      == "Box");
    REQUIRE(std::string(decalProjectionName(DecalProjection::Sphere))   == "Sphere");
    REQUIRE(std::string(decalProjectionName(DecalProjection::Cylinder)) == "Cylinder");
    REQUIRE(std::string(decalProjectionName(DecalProjection::Mesh))     == "Mesh");
}

TEST_CASE("DecalSortOrder names are correct", "[Editor][S92]") {
    REQUIRE(std::string(decalSortOrderName(DecalSortOrder::Default)) == "Default");
    REQUIRE(std::string(decalSortOrderName(DecalSortOrder::Front))   == "Front");
    REQUIRE(std::string(decalSortOrderName(DecalSortOrder::Back))    == "Back");
    REQUIRE(std::string(decalSortOrderName(DecalSortOrder::Manual))  == "Manual");
}

TEST_CASE("DecalAsset stores properties", "[Editor][S92]") {
    DecalAsset decal("BloodSplat");
    decal.setBlendMode(DecalBlendMode::Translucent);
    decal.setProjection(DecalProjection::Box);
    decal.setOpacity(0.9f);
    decal.setDepthFade(0.2f);
    decal.setDirty(true);
    REQUIRE(decal.name()       == "BloodSplat");
    REQUIRE(decal.blendMode()  == DecalBlendMode::Translucent);
    REQUIRE(decal.opacity()    == 0.9f);
    REQUIRE(decal.isDirty());
    REQUIRE(decal.isEnabled());
}

TEST_CASE("DecalEditor addDecal setActive removeDecal", "[Editor][S92]") {
    DecalEditor editor;
    DecalAsset d1("Crack");
    DecalAsset d2("Puddle");
    REQUIRE(editor.addDecal(d1));
    REQUIRE(editor.addDecal(d2));
    REQUIRE(editor.decalCount() == 2);
    REQUIRE(editor.setActiveDecal("Crack"));
    REQUIRE(editor.activeDecal() == "Crack");
    editor.removeDecal("Crack");
    REQUIRE(editor.activeDecal().empty());
}

TEST_CASE("DecalEditor rejects duplicate name", "[Editor][S92]") {
    DecalEditor editor;
    DecalAsset d("Graffiti");
    editor.addDecal(d);
    REQUIRE_FALSE(editor.addDecal(d));
}

TEST_CASE("DecalEditor countByBlendMode and dirtyCount", "[Editor][S92]") {
    DecalEditor editor;
    DecalAsset d1("A"); d1.setBlendMode(DecalBlendMode::Additive);  d1.setDirty(true);
    DecalAsset d2("B"); d2.setBlendMode(DecalBlendMode::Translucent);
    DecalAsset d3("C"); d3.setBlendMode(DecalBlendMode::Additive);  d3.setDirty(true);
    editor.addDecal(d1); editor.addDecal(d2); editor.addDecal(d3);
    REQUIRE(editor.countByBlendMode(DecalBlendMode::Additive) == 2);
    REQUIRE(editor.dirtyCount() == 2);
}

TEST_CASE("DecalEditor MAX_DECALS is 256", "[Editor][S92]") {
    REQUIRE(DecalEditor::MAX_DECALS == 256);
}

// ── LightEditor ──────────────────────────────────────────────────

TEST_CASE("EditorLightType names are correct", "[Editor][S92]") {
    REQUIRE(std::string(editorLightTypeName(EditorLightType::Directional)) == "Directional");
    REQUIRE(std::string(editorLightTypeName(EditorLightType::Point))       == "Point");
    REQUIRE(std::string(editorLightTypeName(EditorLightType::Spot))        == "Spot");
    REQUIRE(std::string(editorLightTypeName(EditorLightType::Rect))        == "Rect");
    REQUIRE(std::string(editorLightTypeName(EditorLightType::Sky))         == "Sky");
}

TEST_CASE("EditorLightMobility names are correct", "[Editor][S92]") {
    REQUIRE(std::string(editorLightMobilityName(EditorLightMobility::Static))      == "Static");
    REQUIRE(std::string(editorLightMobilityName(EditorLightMobility::Stationary))  == "Stationary");
    REQUIRE(std::string(editorLightMobilityName(EditorLightMobility::Dynamic))     == "Dynamic");
}

TEST_CASE("EditorLightShadowMode names are correct", "[Editor][S92]") {
    REQUIRE(std::string(editorLightShadowModeName(EditorLightShadowMode::None))      == "None");
    REQUIRE(std::string(editorLightShadowModeName(EditorLightShadowMode::Hard))      == "Hard");
    REQUIRE(std::string(editorLightShadowModeName(EditorLightShadowMode::Soft))      == "Soft");
    REQUIRE(std::string(editorLightShadowModeName(EditorLightShadowMode::RayTraced)) == "RayTraced");
    REQUIRE(std::string(editorLightShadowModeName(EditorLightShadowMode::Contact))   == "Contact");
}

TEST_CASE("LightInstance stores properties", "[Editor][S92]") {
    LightInstance light("Sun", EditorLightType::Directional);
    light.setMobility(EditorLightMobility::Static);
    light.setShadowMode(EditorLightShadowMode::Soft);
    light.setIntensity(10.0f);
    light.setRange(1000.0f);
    REQUIRE(light.name()       == "Sun");
    REQUIRE(light.type()       == EditorLightType::Directional);
    REQUIRE(light.mobility()   == EditorLightMobility::Static);
    REQUIRE(light.intensity()  == 10.0f);
    REQUIRE(light.isEnabled());
    REQUIRE(light.castsShadow());
}

TEST_CASE("LightEditor addLight setActive removeLight", "[Editor][S92]") {
    LightEditor editor;
    LightInstance l1("Sun",   EditorLightType::Directional);
    LightInstance l2("Torch", EditorLightType::Point);
    REQUIRE(editor.addLight(l1));
    REQUIRE(editor.addLight(l2));
    REQUIRE(editor.lightCount() == 2);
    REQUIRE(editor.setActiveLight("Sun"));
    REQUIRE(editor.activeLight() == "Sun");
    editor.removeLight("Sun");
    REQUIRE(editor.activeLight().empty());
}

TEST_CASE("LightEditor rejects duplicate name", "[Editor][S92]") {
    LightEditor editor;
    LightInstance l("Candle", EditorLightType::Point);
    editor.addLight(l);
    REQUIRE_FALSE(editor.addLight(l));
}

TEST_CASE("LightEditor countByType and countByMobility", "[Editor][S92]") {
    LightEditor editor;
    LightInstance l1("A", EditorLightType::Point); l1.setMobility(EditorLightMobility::Dynamic);
    LightInstance l2("B", EditorLightType::Spot);  l2.setMobility(EditorLightMobility::Static);
    LightInstance l3("C", EditorLightType::Point); l3.setMobility(EditorLightMobility::Dynamic);
    editor.addLight(l1); editor.addLight(l2); editor.addLight(l3);
    REQUIRE(editor.countByType(EditorLightType::Point)                == 2);
    REQUIRE(editor.countByMobility(EditorLightMobility::Dynamic)      == 2);
}

TEST_CASE("LightEditor MAX_LIGHTS is 512", "[Editor][S92]") {
    REQUIRE(LightEditor::MAX_LIGHTS == 512);
}

// ── ReflectionProbeEditor ────────────────────────────────────────

TEST_CASE("ReflectionProbeType names are correct", "[Editor][S92]") {
    REQUIRE(std::string(reflectionProbeTypeName(ReflectionProbeType::Box))         == "Box");
    REQUIRE(std::string(reflectionProbeTypeName(ReflectionProbeType::Sphere))      == "Sphere");
    REQUIRE(std::string(reflectionProbeTypeName(ReflectionProbeType::Planar))      == "Planar");
    REQUIRE(std::string(reflectionProbeTypeName(ReflectionProbeType::ScreenSpace)) == "ScreenSpace");
}

TEST_CASE("ReflectionBakeMode names are correct", "[Editor][S92]") {
    REQUIRE(std::string(reflectionBakeModeName(ReflectionBakeMode::Realtime)) == "Realtime");
    REQUIRE(std::string(reflectionBakeModeName(ReflectionBakeMode::Baked))    == "Baked");
    REQUIRE(std::string(reflectionBakeModeName(ReflectionBakeMode::Custom))   == "Custom");
    REQUIRE(std::string(reflectionBakeModeName(ReflectionBakeMode::Once))     == "Once");
}

TEST_CASE("ReflectionBakeStatus names are correct", "[Editor][S92]") {
    REQUIRE(std::string(reflectionBakeStatusName(ReflectionBakeStatus::Idle))   == "Idle");
    REQUIRE(std::string(reflectionBakeStatusName(ReflectionBakeStatus::Baking)) == "Baking");
    REQUIRE(std::string(reflectionBakeStatusName(ReflectionBakeStatus::Done))   == "Done");
    REQUIRE(std::string(reflectionBakeStatusName(ReflectionBakeStatus::Stale))  == "Stale");
    REQUIRE(std::string(reflectionBakeStatusName(ReflectionBakeStatus::Failed)) == "Failed");
}

TEST_CASE("ReflectionProbe stores properties", "[Editor][S92]") {
    ReflectionProbe probe("LobbyProbe", ReflectionProbeType::Box);
    probe.setBakeMode(ReflectionBakeMode::Baked);
    probe.setStatus(ReflectionBakeStatus::Done);
    probe.setResolution(512);
    probe.setBlendDistance(2.0f);
    probe.setBoxProjection(true);
    REQUIRE(probe.name()          == "LobbyProbe");
    REQUIRE(probe.type()          == ReflectionProbeType::Box);
    REQUIRE(probe.isDone());
    REQUIRE_FALSE(probe.isStale());
    REQUIRE(probe.resolution()    == 512);
    REQUIRE(probe.boxProjection());
}

TEST_CASE("ReflectionProbeEditor add setActive remove", "[Editor][S92]") {
    ReflectionProbeEditor editor;
    ReflectionProbe p1("ProbeA", ReflectionProbeType::Box);
    ReflectionProbe p2("ProbeB", ReflectionProbeType::Sphere);
    REQUIRE(editor.addProbe(p1));
    REQUIRE(editor.addProbe(p2));
    REQUIRE(editor.probeCount() == 2);
    REQUIRE(editor.setActiveProbe("ProbeA"));
    REQUIRE(editor.activeProbe() == "ProbeA");
    editor.removeProbe("ProbeA");
    REQUIRE(editor.activeProbe().empty());
}

TEST_CASE("ReflectionProbeEditor rejects duplicate name", "[Editor][S92]") {
    ReflectionProbeEditor editor;
    ReflectionProbe p("Dup", ReflectionProbeType::Planar);
    editor.addProbe(p);
    REQUIRE_FALSE(editor.addProbe(p));
}

TEST_CASE("ReflectionProbeEditor doneCount staleCount and countByType", "[Editor][S92]") {
    ReflectionProbeEditor editor;
    ReflectionProbe p1("A", ReflectionProbeType::Box);    p1.setStatus(ReflectionBakeStatus::Done);
    ReflectionProbe p2("B", ReflectionProbeType::Box);    p2.setStatus(ReflectionBakeStatus::Stale);
    ReflectionProbe p3("C", ReflectionProbeType::Sphere); p3.setStatus(ReflectionBakeStatus::Done);
    editor.addProbe(p1); editor.addProbe(p2); editor.addProbe(p3);
    REQUIRE(editor.doneCount()                           == 2);
    REQUIRE(editor.staleCount()                          == 1);
    REQUIRE(editor.countByType(ReflectionProbeType::Box) == 2);
}

TEST_CASE("ReflectionProbeEditor MAX_PROBES is 128", "[Editor][S92]") {
    REQUIRE(ReflectionProbeEditor::MAX_PROBES == 128);
}
