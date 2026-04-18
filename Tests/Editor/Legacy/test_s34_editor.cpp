#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/MaterialEditor.h"

using namespace NF;

TEST_CASE("MaterialShadingModel names cover all 8 values", "[Editor][S34]") {
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::Unlit))      == "Unlit");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::Lambert))    == "Lambert");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::Phong))      == "Phong");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::BlinnPhong)) == "BlinnPhong");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::PBR))        == "PBR");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::Toon))       == "Toon");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::Subsurface)) == "Subsurface");
    REQUIRE(std::string(materialShadingModelName(MaterialShadingModel::Custom))     == "Custom");
}

TEST_CASE("MaterialBlendMode names cover all 4 values", "[Editor][S34]") {
    REQUIRE(std::string(materialBlendModeName(MaterialBlendMode::Opaque))      == "Opaque");
    REQUIRE(std::string(materialBlendModeName(MaterialBlendMode::Masked))      == "Masked");
    REQUIRE(std::string(materialBlendModeName(MaterialBlendMode::Translucent)) == "Translucent");
    REQUIRE(std::string(materialBlendModeName(MaterialBlendMode::Additive))    == "Additive");
}

TEST_CASE("MaterialParameter expose and hide toggle exposed flag", "[Editor][S34]") {
    MaterialParameter p; p.name = "roughness"; p.value = 0.5f;
    REQUIRE_FALSE(p.exposed);
    p.expose();
    REQUIRE(p.exposed);
    p.hide();
    REQUIRE_FALSE(p.exposed);
}

TEST_CASE("MaterialAsset default shading model is PBR and blend mode is Opaque", "[Editor][S34]") {
    MaterialAsset asset("mat1");
    REQUIRE(asset.shadingModel() == MaterialShadingModel::PBR);
    REQUIRE(asset.blendMode()    == MaterialBlendMode::Opaque);
    REQUIRE_FALSE(asset.isDirty());
}

TEST_CASE("MaterialAsset addParameter and duplicate rejection", "[Editor][S34]") {
    MaterialAsset asset("mat2");
    MaterialParameter a; a.name = "metallic"; a.value = 0.0f;
    MaterialParameter b; b.name = "roughness"; b.value = 0.5f;
    MaterialParameter dup; dup.name = "metallic";

    REQUIRE(asset.addParameter(a));
    REQUIRE(asset.addParameter(b));
    REQUIRE_FALSE(asset.addParameter(dup));
    REQUIRE(asset.paramCount() == 2);
}

TEST_CASE("MaterialAsset removeParameter reduces paramCount", "[Editor][S34]") {
    MaterialAsset asset("mat3");
    MaterialParameter p; p.name = "opacity"; p.value = 1.0f;
    asset.addParameter(p);

    REQUIRE(asset.removeParameter("opacity"));
    REQUIRE_FALSE(asset.removeParameter("opacity"));
    REQUIRE(asset.paramCount() == 0);
}

TEST_CASE("MaterialAsset findParameter returns pointer or nullptr", "[Editor][S34]") {
    MaterialAsset asset("mat4");
    MaterialParameter p; p.name = "emissive"; p.value = 2.0f;
    asset.addParameter(p);

    REQUIRE(asset.findParameter("emissive") != nullptr);
    REQUIRE(asset.findParameter("emissive")->value == Catch::Approx(2.0f));
    REQUIRE(asset.findParameter("missing") == nullptr);
}

TEST_CASE("MaterialAsset exposedParamCount counts only exposed params", "[Editor][S34]") {
    MaterialAsset asset("mat5");
    MaterialParameter a; a.name = "a"; a.exposed = true;
    MaterialParameter b; b.name = "b"; b.exposed = false;
    MaterialParameter c; c.name = "c"; c.exposed = true;
    asset.addParameter(a);
    asset.addParameter(b);
    asset.addParameter(c);

    REQUIRE(asset.exposedParamCount() == 2);
}

TEST_CASE("MaterialAsset setShadingModel and setBlendMode", "[Editor][S34]") {
    MaterialAsset asset("mat6");
    asset.setShadingModel(MaterialShadingModel::Toon);
    asset.setBlendMode(MaterialBlendMode::Translucent);
    REQUIRE(asset.shadingModel() == MaterialShadingModel::Toon);
    REQUIRE(asset.blendMode()    == MaterialBlendMode::Translucent);
}

TEST_CASE("MaterialAsset setDirty and isDirty", "[Editor][S34]") {
    MaterialAsset asset("mat7");
    REQUIRE_FALSE(asset.isDirty());
    asset.setDirty(true);
    REQUIRE(asset.isDirty());
    asset.setDirty(false);
    REQUIRE_FALSE(asset.isDirty());
}

TEST_CASE("MaterialEditor addAsset and duplicate rejection", "[Editor][S34]") {
    MaterialEditor editor;
    MaterialAsset a("A");
    MaterialAsset b("B");
    MaterialAsset dup("A");

    REQUIRE(editor.addAsset(a));
    REQUIRE(editor.addAsset(b));
    REQUIRE_FALSE(editor.addAsset(dup));
    REQUIRE(editor.assetCount() == 2);
}

TEST_CASE("MaterialEditor removeAsset clears activeAsset when active", "[Editor][S34]") {
    MaterialEditor editor;
    MaterialAsset a("active");
    editor.addAsset(a);
    editor.setActiveAsset("active");
    REQUIRE(editor.activeAsset() == "active");

    editor.removeAsset("active");
    REQUIRE(editor.assetCount()   == 0);
    REQUIRE(editor.activeAsset().empty());
}

TEST_CASE("MaterialEditor setActiveAsset returns false for missing asset", "[Editor][S34]") {
    MaterialEditor editor;
    REQUIRE_FALSE(editor.setActiveAsset("nonexistent"));
    REQUIRE(editor.activeAsset().empty());
}

TEST_CASE("MaterialEditor findAsset returns pointer or nullptr", "[Editor][S34]") {
    MaterialEditor editor;
    MaterialAsset a("findme");
    editor.addAsset(a);

    REQUIRE(editor.findAsset("findme") != nullptr);
    REQUIRE(editor.findAsset("findme")->name() == "findme");
    REQUIRE(editor.findAsset("unknown") == nullptr);
}

TEST_CASE("MaterialEditor dirtyCount reflects dirty assets", "[Editor][S34]") {
    MaterialEditor editor;
    MaterialAsset a("a");
    MaterialAsset b("b");
    MaterialAsset c("c");
    a.setDirty(true);
    c.setDirty(true);
    editor.addAsset(a);
    editor.addAsset(b);
    editor.addAsset(c);

    REQUIRE(editor.dirtyCount() == 2);
}

TEST_CASE("MaterialEditor MAX_ASSETS limit enforced", "[Editor][S34]") {
    MaterialEditor editor;
    for (size_t i = 0; i < MaterialEditor::MAX_ASSETS; ++i) {
        MaterialAsset a("m" + std::to_string(i));
        REQUIRE(editor.addAsset(a));
    }
    MaterialAsset overflow("overflow");
    REQUIRE_FALSE(editor.addAsset(overflow));
    REQUIRE(editor.assetCount() == MaterialEditor::MAX_ASSETS);
}
