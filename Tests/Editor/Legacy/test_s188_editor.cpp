// S188 editor tests: ClothSimEditorV1, PostProcessEditorV1, HeightmapEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ClothSimEditorV1.h"
#include "NF/Editor/PostProcessEditorV1.h"
#include "NF/Editor/HeightmapEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ClothSimEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Csev1ClothLayer validity", "[Editor][S188]") {
    Csev1ClothLayer l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "Cape";
    REQUIRE(l.isValid());
}

TEST_CASE("ClothSimEditorV1 addLayer and layerCount", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    REQUIRE(cse.layerCount() == 0);
    Csev1ClothLayer l; l.id = 1; l.name = "Cloth1";
    REQUIRE(cse.addLayer(l));
    REQUIRE(cse.layerCount() == 1);
}

TEST_CASE("ClothSimEditorV1 addLayer invalid fails", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    REQUIRE(!cse.addLayer(Csev1ClothLayer{}));
}

TEST_CASE("ClothSimEditorV1 addLayer duplicate fails", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1ClothLayer l; l.id = 1; l.name = "A";
    cse.addLayer(l);
    REQUIRE(!cse.addLayer(l));
}

TEST_CASE("ClothSimEditorV1 removeLayer", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1ClothLayer l; l.id = 2; l.name = "B";
    cse.addLayer(l);
    REQUIRE(cse.removeLayer(2));
    REQUIRE(cse.layerCount() == 0);
    REQUIRE(!cse.removeLayer(2));
}

TEST_CASE("ClothSimEditorV1 findLayer", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1ClothLayer l; l.id = 3; l.name = "C";
    cse.addLayer(l);
    REQUIRE(cse.findLayer(3) != nullptr);
    REQUIRE(cse.findLayer(99) == nullptr);
}

TEST_CASE("ClothSimEditorV1 addConstraint and constraintCount", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1Constraint c; c.id = 1; c.layerId = 10; c.name = "Anchor";
    REQUIRE(cse.addConstraint(c));
    REQUIRE(cse.constraintCount() == 1);
}

TEST_CASE("ClothSimEditorV1 removeConstraint", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1Constraint c; c.id = 1; c.layerId = 10; c.name = "Stiff";
    cse.addConstraint(c);
    REQUIRE(cse.removeConstraint(1));
    REQUIRE(cse.constraintCount() == 0);
}

TEST_CASE("ClothSimEditorV1 simulatingCount", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1ClothLayer l1; l1.id = 1; l1.name = "A"; l1.state = Csev1ClothState::Simulating;
    Csev1ClothLayer l2; l2.id = 2; l2.name = "B";
    cse.addLayer(l1); cse.addLayer(l2);
    REQUIRE(cse.simulatingCount() == 1);
}

TEST_CASE("ClothSimEditorV1 bakedCount", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1ClothLayer l1; l1.id = 1; l1.name = "A"; l1.state = Csev1ClothState::Baked;
    Csev1ClothLayer l2; l2.id = 2; l2.name = "B"; l2.state = Csev1ClothState::Simulating;
    cse.addLayer(l1); cse.addLayer(l2);
    REQUIRE(cse.bakedCount() == 1);
}

TEST_CASE("ClothSimEditorV1 countByType", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1ClothLayer l1; l1.id = 1; l1.name = "A"; l1.clothType = Csev1ClothType::Soft;
    Csev1ClothLayer l2; l2.id = 2; l2.name = "B"; l2.clothType = Csev1ClothType::Tearable;
    Csev1ClothLayer l3; l3.id = 3; l3.name = "C"; l3.clothType = Csev1ClothType::Soft;
    cse.addLayer(l1); cse.addLayer(l2); cse.addLayer(l3);
    REQUIRE(cse.countByType(Csev1ClothType::Soft) == 2);
    REQUIRE(cse.countByType(Csev1ClothType::Tearable) == 1);
}

TEST_CASE("ClothSimEditorV1 constraintsForLayer", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    Csev1Constraint c1; c1.id = 1; c1.layerId = 10; c1.name = "A";
    Csev1Constraint c2; c2.id = 2; c2.layerId = 10; c2.name = "B";
    Csev1Constraint c3; c3.id = 3; c3.layerId = 20; c3.name = "C";
    cse.addConstraint(c1); cse.addConstraint(c2); cse.addConstraint(c3);
    REQUIRE(cse.constraintsForLayer(10) == 2);
    REQUIRE(cse.constraintsForLayer(20) == 1);
}

TEST_CASE("ClothSimEditorV1 onChange callback", "[Editor][S188]") {
    ClothSimEditorV1 cse;
    uint64_t notified = 0;
    cse.setOnChange([&](uint64_t id) { notified = id; });
    Csev1ClothLayer l; l.id = 6; l.name = "F";
    cse.addLayer(l);
    REQUIRE(notified == 6);
}

TEST_CASE("Csev1ClothLayer state helpers", "[Editor][S188]") {
    Csev1ClothLayer l; l.id = 1; l.name = "X";
    l.state = Csev1ClothState::Simulating; REQUIRE(l.isSimulating());
    l.state = Csev1ClothState::Baked;      REQUIRE(l.isBaked());
    l.state = Csev1ClothState::Error;      REQUIRE(l.isError());
}

TEST_CASE("csev1ClothTypeName all values", "[Editor][S188]") {
    REQUIRE(std::string(csev1ClothTypeName(Csev1ClothType::Rigid))    == "Rigid");
    REQUIRE(std::string(csev1ClothTypeName(Csev1ClothType::Soft))     == "Soft");
    REQUIRE(std::string(csev1ClothTypeName(Csev1ClothType::Hybrid))   == "Hybrid");
    REQUIRE(std::string(csev1ClothTypeName(Csev1ClothType::Tearable)) == "Tearable");
    REQUIRE(std::string(csev1ClothTypeName(Csev1ClothType::Wind))     == "Wind");
}

// ── PostProcessEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Ppev1Effect validity", "[Editor][S188]") {
    Ppev1Effect e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "BloomEffect";
    REQUIRE(e.isValid());
}

TEST_CASE("PostProcessEditorV1 addEffect and effectCount", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    REQUIRE(ppe.effectCount() == 0);
    Ppev1Effect e; e.id = 1; e.name = "E1";
    REQUIRE(ppe.addEffect(e));
    REQUIRE(ppe.effectCount() == 1);
}

TEST_CASE("PostProcessEditorV1 addEffect invalid fails", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    REQUIRE(!ppe.addEffect(Ppev1Effect{}));
}

TEST_CASE("PostProcessEditorV1 addEffect duplicate fails", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Effect e; e.id = 1; e.name = "A";
    ppe.addEffect(e);
    REQUIRE(!ppe.addEffect(e));
}

TEST_CASE("PostProcessEditorV1 removeEffect", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Effect e; e.id = 2; e.name = "B";
    ppe.addEffect(e);
    REQUIRE(ppe.removeEffect(2));
    REQUIRE(ppe.effectCount() == 0);
    REQUIRE(!ppe.removeEffect(2));
}

TEST_CASE("PostProcessEditorV1 findEffect", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Effect e; e.id = 3; e.name = "C";
    ppe.addEffect(e);
    REQUIRE(ppe.findEffect(3) != nullptr);
    REQUIRE(ppe.findEffect(99) == nullptr);
}

TEST_CASE("PostProcessEditorV1 addProfile and profileCount", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Profile p; p.id = 1; p.name = "Cinematic";
    REQUIRE(ppe.addProfile(p));
    REQUIRE(ppe.profileCount() == 1);
}

TEST_CASE("PostProcessEditorV1 removeProfile", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Profile p; p.id = 1; p.name = "Standard";
    ppe.addProfile(p);
    REQUIRE(ppe.removeProfile(1));
    REQUIRE(ppe.profileCount() == 0);
}

TEST_CASE("PostProcessEditorV1 enabledCount", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Effect e1; e1.id = 1; e1.name = "A"; e1.state = Ppev1EffectState::Enabled;
    Ppev1Effect e2; e2.id = 2; e2.name = "B";
    ppe.addEffect(e1); ppe.addEffect(e2);
    REQUIRE(ppe.enabledCount() == 1);
}

TEST_CASE("PostProcessEditorV1 countByType", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    Ppev1Effect e1; e1.id = 1; e1.name = "A"; e1.effectType = Ppev1EffectType::Bloom;
    Ppev1Effect e2; e2.id = 2; e2.name = "B"; e2.effectType = Ppev1EffectType::SSAO;
    Ppev1Effect e3; e3.id = 3; e3.name = "C"; e3.effectType = Ppev1EffectType::Bloom;
    ppe.addEffect(e1); ppe.addEffect(e2); ppe.addEffect(e3);
    REQUIRE(ppe.countByType(Ppev1EffectType::Bloom) == 2);
    REQUIRE(ppe.countByType(Ppev1EffectType::SSAO) == 1);
}

TEST_CASE("PostProcessEditorV1 onChange callback", "[Editor][S188]") {
    PostProcessEditorV1 ppe;
    uint64_t notified = 0;
    ppe.setOnChange([&](uint64_t id) { notified = id; });
    Ppev1Effect e; e.id = 9; e.name = "I";
    ppe.addEffect(e);
    REQUIRE(notified == 9);
}

TEST_CASE("Ppev1Effect state helpers", "[Editor][S188]") {
    Ppev1Effect e; e.id = 1; e.name = "X";
    e.state = Ppev1EffectState::Enabled;  REQUIRE(e.isEnabled());
    e.state = Ppev1EffectState::Preview;  REQUIRE(e.isPreview());
    e.state = Ppev1EffectState::Override; REQUIRE(e.isOverride());
}

TEST_CASE("ppev1EffectTypeName all values", "[Editor][S188]") {
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::Bloom))               == "Bloom");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::DOF))                 == "DOF");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::SSAO))                == "SSAO");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::MotionBlur))          == "MotionBlur");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::ChromaticAberration)) == "ChromaticAberration");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::Vignette))            == "Vignette");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::ColorGrading))        == "ColorGrading");
    REQUIRE(std::string(ppev1EffectTypeName(Ppev1EffectType::ToneMapping))         == "ToneMapping");
}

// ── HeightmapEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Hmev1Layer validity", "[Editor][S188]") {
    Hmev1Layer l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "BaseLayer";
    REQUIRE(l.isValid());
}

TEST_CASE("HeightmapEditorV1 addLayer and layerCount", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    REQUIRE(hme.layerCount() == 0);
    Hmev1Layer l; l.id = 1; l.name = "L1";
    REQUIRE(hme.addLayer(l));
    REQUIRE(hme.layerCount() == 1);
}

TEST_CASE("HeightmapEditorV1 addLayer invalid fails", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    REQUIRE(!hme.addLayer(Hmev1Layer{}));
}

TEST_CASE("HeightmapEditorV1 addLayer duplicate fails", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Layer l; l.id = 1; l.name = "A";
    hme.addLayer(l);
    REQUIRE(!hme.addLayer(l));
}

TEST_CASE("HeightmapEditorV1 removeLayer", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Layer l; l.id = 2; l.name = "B";
    hme.addLayer(l);
    REQUIRE(hme.removeLayer(2));
    REQUIRE(hme.layerCount() == 0);
    REQUIRE(!hme.removeLayer(2));
}

TEST_CASE("HeightmapEditorV1 findLayer", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Layer l; l.id = 3; l.name = "C";
    hme.addLayer(l);
    REQUIRE(hme.findLayer(3) != nullptr);
    REQUIRE(hme.findLayer(99) == nullptr);
}

TEST_CASE("HeightmapEditorV1 addBrush and brushCount", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Brush b; b.id = 1; b.name = "Raise";
    REQUIRE(hme.addBrush(b));
    REQUIRE(hme.brushCount() == 1);
}

TEST_CASE("HeightmapEditorV1 removeBrush", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Brush b; b.id = 1; b.name = "Smooth";
    hme.addBrush(b);
    REQUIRE(hme.removeBrush(1));
    REQUIRE(hme.brushCount() == 0);
}

TEST_CASE("HeightmapEditorV1 visibleCount", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Layer l1; l1.id = 1; l1.name = "A"; l1.state = Hmev1LayerState::Visible;
    Hmev1Layer l2; l2.id = 2; l2.name = "B"; l2.state = Hmev1LayerState::Hidden;
    hme.addLayer(l1); hme.addLayer(l2);
    REQUIRE(hme.visibleCount() == 1);
}

TEST_CASE("HeightmapEditorV1 lockedCount", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Layer l1; l1.id = 1; l1.name = "A"; l1.state = Hmev1LayerState::Locked;
    Hmev1Layer l2; l2.id = 2; l2.name = "B"; l2.state = Hmev1LayerState::Visible;
    hme.addLayer(l1); hme.addLayer(l2);
    REQUIRE(hme.lockedCount() == 1);
}

TEST_CASE("HeightmapEditorV1 countByType", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    Hmev1Layer l1; l1.id = 1; l1.name = "A"; l1.layerType = Hmev1LayerType::Erosion;
    Hmev1Layer l2; l2.id = 2; l2.name = "B"; l2.layerType = Hmev1LayerType::Noise;
    Hmev1Layer l3; l3.id = 3; l3.name = "C"; l3.layerType = Hmev1LayerType::Erosion;
    hme.addLayer(l1); hme.addLayer(l2); hme.addLayer(l3);
    REQUIRE(hme.countByType(Hmev1LayerType::Erosion) == 2);
    REQUIRE(hme.countByType(Hmev1LayerType::Noise) == 1);
}

TEST_CASE("HeightmapEditorV1 onChange callback", "[Editor][S188]") {
    HeightmapEditorV1 hme;
    uint64_t notified = 0;
    hme.setOnChange([&](uint64_t id) { notified = id; });
    Hmev1Layer l; l.id = 5; l.name = "E";
    hme.addLayer(l);
    REQUIRE(notified == 5);
}

TEST_CASE("Hmev1Layer state helpers", "[Editor][S188]") {
    Hmev1Layer l; l.id = 1; l.name = "X";
    l.state = Hmev1LayerState::Visible; REQUIRE(l.isVisible());
    l.state = Hmev1LayerState::Locked;  REQUIRE(l.isLocked());
    l.state = Hmev1LayerState::Solo;    REQUIRE(l.isSolo());
}

TEST_CASE("hmev1LayerTypeName all values", "[Editor][S188]") {
    REQUIRE(std::string(hmev1LayerTypeName(Hmev1LayerType::Base))    == "Base");
    REQUIRE(std::string(hmev1LayerTypeName(Hmev1LayerType::Erosion)) == "Erosion");
    REQUIRE(std::string(hmev1LayerTypeName(Hmev1LayerType::Noise))   == "Noise");
    REQUIRE(std::string(hmev1LayerTypeName(Hmev1LayerType::Stamp))   == "Stamp");
    REQUIRE(std::string(hmev1LayerTypeName(Hmev1LayerType::Sculpt))  == "Sculpt");
    REQUIRE(std::string(hmev1LayerTypeName(Hmev1LayerType::Import))  == "Import");
}
