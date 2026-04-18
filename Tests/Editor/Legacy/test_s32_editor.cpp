#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ParticleEditor.h"

using namespace NF;

TEST_CASE("ParticleEmitterShape names cover all 8 values", "[Editor][S32]") {
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Point))     == "Point");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Circle))    == "Circle");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Rectangle)) == "Rectangle");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Cone))      == "Cone");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Sphere))    == "Sphere");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Ring))      == "Ring");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Line))      == "Line");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Custom))    == "Custom");
}

TEST_CASE("ParticleBlendMode names cover all 4 values", "[Editor][S32]") {
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Alpha))    == "Alpha");
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Multiply)) == "Multiply");
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Screen))   == "Screen");
}

TEST_CASE("ParticleEmitterConfig defaults are valid", "[Editor][S32]") {
    ParticleEmitterConfig cfg;
    cfg.id = "fire-1";
    REQUIRE(cfg.isValid());
    REQUIRE(cfg.looping);
    REQUIRE(cfg.shape     == ParticleEmitterShape::Point);
    REQUIRE(cfg.blendMode == ParticleBlendMode::Additive);
}

TEST_CASE("ParticleEmitterConfig setters update fields", "[Editor][S32]") {
    ParticleEmitterConfig cfg;
    cfg.id = "smoke-1";
    cfg.setEmitRate(50.0f);
    cfg.setLifetime(2.5f);
    cfg.setSpeed(3.0f);
    cfg.setSize(0.5f);

    REQUIRE(cfg.emitRate == Catch::Approx(50.0f));
    REQUIRE(cfg.lifetime == Catch::Approx(2.5f));
    REQUIRE(cfg.speed    == Catch::Approx(3.0f));
    REQUIRE(cfg.size     == Catch::Approx(0.5f));
}

TEST_CASE("ParticleEmitterConfig isValid rejects non-positive fields", "[Editor][S32]") {
    ParticleEmitterConfig cfg;
    cfg.id = "invalid";
    cfg.setEmitRate(0.0f);
    REQUIRE_FALSE(cfg.isValid());

    cfg.setEmitRate(10.0f);
    cfg.setLifetime(0.0f);
    REQUIRE_FALSE(cfg.isValid());

    cfg.setLifetime(1.0f);
    cfg.setSize(0.0f);
    REQUIRE_FALSE(cfg.isValid());
}

TEST_CASE("ParticleEffectLayer addEmitter and duplicate rejection", "[Editor][S32]") {
    ParticleEffectLayer layer("sparks");

    ParticleEmitterConfig a; a.id = "e1"; a.setEmitRate(10.0f);
    ParticleEmitterConfig b; b.id = "e2"; b.setEmitRate(20.0f);
    ParticleEmitterConfig dup; dup.id = "e1";

    REQUIRE(layer.addEmitter(a));
    REQUIRE(layer.addEmitter(b));
    REQUIRE_FALSE(layer.addEmitter(dup));
    REQUIRE(layer.emitterCount() == 2);
}

TEST_CASE("ParticleEffectLayer removeEmitter returns correct result", "[Editor][S32]") {
    ParticleEffectLayer layer("dust");

    ParticleEmitterConfig e; e.id = "e1";
    layer.addEmitter(e);

    REQUIRE(layer.removeEmitter("e1"));
    REQUIRE_FALSE(layer.removeEmitter("e1"));
    REQUIRE(layer.emitterCount() == 0);
}

TEST_CASE("ParticleEffectLayer findEmitter returns pointer or nullptr", "[Editor][S32]") {
    ParticleEffectLayer layer("trail");

    ParticleEmitterConfig e; e.id = "ex"; e.setEmitRate(15.0f);
    layer.addEmitter(e);

    REQUIRE(layer.findEmitter("ex") != nullptr);
    REQUIRE(layer.findEmitter("ex")->emitRate == Catch::Approx(15.0f));
    REQUIRE(layer.findEmitter("missing") == nullptr);
}

TEST_CASE("ParticleEffectLayer visible flag set and read", "[Editor][S32]") {
    ParticleEffectLayer layer("glow");

    REQUIRE(layer.visible());
    layer.setVisible(false);
    REQUIRE_FALSE(layer.visible());
    layer.setVisible(true);
    REQUIRE(layer.visible());
}

TEST_CASE("ParticleEffectEditor addLayer and duplicate rejection", "[Editor][S32]") {
    ParticleEffectEditor editor;

    ParticleEffectLayer la("fire");
    ParticleEffectLayer lb("smoke");
    ParticleEffectLayer dup("fire");

    REQUIRE(editor.addLayer(la));
    REQUIRE(editor.addLayer(lb));
    REQUIRE_FALSE(editor.addLayer(dup));
    REQUIRE(editor.layerCount() == 2);
}

TEST_CASE("ParticleEffectEditor removeLayer reduces layerCount", "[Editor][S32]") {
    ParticleEffectEditor editor;

    ParticleEffectLayer l("ember");
    editor.addLayer(l);

    REQUIRE(editor.layerCount() == 1);
    REQUIRE(editor.removeLayer("ember"));
    REQUIRE(editor.layerCount() == 0);
    REQUIRE_FALSE(editor.removeLayer("ember"));
}

TEST_CASE("ParticleEffectEditor setActiveLayer and activeLayer", "[Editor][S32]") {
    ParticleEffectEditor editor;

    ParticleEffectLayer la("spark");
    ParticleEffectLayer lb("burst");
    editor.addLayer(la);
    editor.addLayer(lb);

    REQUIRE(editor.activeLayer().empty());
    REQUIRE(editor.setActiveLayer("spark"));
    REQUIRE(editor.activeLayer() == "spark");
    REQUIRE_FALSE(editor.setActiveLayer("nonexistent"));
    REQUIRE(editor.activeLayer() == "spark");
}

TEST_CASE("ParticleEffectEditor preview and stopPreview", "[Editor][S32]") {
    ParticleEffectEditor editor;

    REQUIRE_FALSE(editor.isPreviewing());
    editor.preview();
    REQUIRE(editor.isPreviewing());
    editor.stopPreview();
    REQUIRE_FALSE(editor.isPreviewing());
}

TEST_CASE("ParticleEffectEditor totalEmitterCount sums across layers", "[Editor][S32]") {
    ParticleEffectEditor editor;

    ParticleEffectLayer la("a");
    ParticleEmitterConfig e1; e1.id = "e1";
    ParticleEmitterConfig e2; e2.id = "e2";
    la.addEmitter(e1);
    la.addEmitter(e2);

    ParticleEffectLayer lb("b");
    ParticleEmitterConfig e3; e3.id = "e3";
    lb.addEmitter(e3);

    editor.addLayer(la);
    editor.addLayer(lb);

    REQUIRE(editor.totalEmitterCount() == 3);
}

TEST_CASE("ParticleEffectEditor MAX_LAYERS limit is enforced", "[Editor][S32]") {
    ParticleEffectEditor editor;

    for (size_t i = 0; i < ParticleEffectEditor::MAX_LAYERS; ++i) {
        ParticleEffectLayer l("layer-" + std::to_string(i));
        REQUIRE(editor.addLayer(l));
    }

    ParticleEffectLayer overflow("overflow");
    REQUIRE_FALSE(editor.addLayer(overflow));
    REQUIRE(editor.layerCount() == ParticleEffectEditor::MAX_LAYERS);
}

TEST_CASE("ParticleEffectEditor removeLayer clears activeLayer if active", "[Editor][S32]") {
    ParticleEffectEditor editor;

    ParticleEffectLayer l("active-layer");
    editor.addLayer(l);
    editor.setActiveLayer("active-layer");
    REQUIRE(editor.activeLayer() == "active-layer");

    editor.removeLayer("active-layer");
    REQUIRE(editor.activeLayer().empty());
}
