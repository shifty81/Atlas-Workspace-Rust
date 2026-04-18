// S104 editor tests: MaterialLayerEditor, TextureAtlasEditor, ShaderVariantEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ShaderVariantEditor.h"
#include "NF/Editor/TextureAtlasEditor.h"
#include "NF/Editor/MaterialLayerEditor.h"

using namespace NF;

// ── MaterialLayerEditor ──────────────────────────────────────────────────────

TEST_CASE("MaterialLayerBlendMode names", "[Editor][S104]") {
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Opaque))      == "Opaque");
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Translucent)) == "Translucent");
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Additive))    == "Additive");
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Multiply))    == "Multiply");
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Screen))      == "Screen");
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Overlay))     == "Overlay");
    REQUIRE(std::string(materialLayerBlendModeName(MaterialLayerBlendMode::Masked))      == "Masked");
}

TEST_CASE("MaterialLayerShadingModel names", "[Editor][S104]") {
    REQUIRE(std::string(materialLayerShadingModelName(MaterialLayerShadingModel::Lit))                  == "Lit");
    REQUIRE(std::string(materialLayerShadingModelName(MaterialLayerShadingModel::Unlit))                == "Unlit");
    REQUIRE(std::string(materialLayerShadingModelName(MaterialLayerShadingModel::SubsurfaceScattering)) == "SubsurfaceScattering");
    REQUIRE(std::string(materialLayerShadingModelName(MaterialLayerShadingModel::ClearCoat))            == "ClearCoat");
    REQUIRE(std::string(materialLayerShadingModelName(MaterialLayerShadingModel::TwoSidedFoliage))      == "TwoSidedFoliage");
    REQUIRE(std::string(materialLayerShadingModelName(MaterialLayerShadingModel::Hair))                 == "Hair");
}

TEST_CASE("MaterialLayer defaults", "[Editor][S104]") {
    MaterialLayer layer(1, "base");
    REQUIRE(layer.id()           == 1u);
    REQUIRE(layer.name()         == "base");
    REQUIRE(layer.blendMode()    == MaterialLayerBlendMode::Opaque);
    REQUIRE(layer.shadingModel() == MaterialLayerShadingModel::Lit);
    REQUIRE(layer.opacity()      == 1.0f);
    REQUIRE(layer.tiling()       == 1.0f);
    REQUIRE(layer.isEnabled());
    REQUIRE(!layer.isLocked());
}

TEST_CASE("MaterialLayer mutation", "[Editor][S104]") {
    MaterialLayer layer(2, "overlay_fx");
    layer.setBlendMode(MaterialLayerBlendMode::Additive);
    layer.setShadingModel(MaterialLayerShadingModel::Unlit);
    layer.setOpacity(0.7f);
    layer.setTiling(2.0f);
    layer.setEnabled(false);
    layer.setLocked(true);
    REQUIRE(layer.blendMode()    == MaterialLayerBlendMode::Additive);
    REQUIRE(layer.shadingModel() == MaterialLayerShadingModel::Unlit);
    REQUIRE(layer.opacity()      == 0.7f);
    REQUIRE(layer.tiling()       == 2.0f);
    REQUIRE(!layer.isEnabled());
    REQUIRE(layer.isLocked());
}

TEST_CASE("MaterialLayerEditor add/remove", "[Editor][S104]") {
    MaterialLayerEditor ed;
    REQUIRE(ed.addLayer(MaterialLayer(1, "base")));
    REQUIRE(ed.addLayer(MaterialLayer(2, "detail")));
    REQUIRE(!ed.addLayer(MaterialLayer(1, "dup")));
    REQUIRE(ed.layerCount() == 2u);
    REQUIRE(ed.removeLayer(1));
    REQUIRE(ed.layerCount() == 1u);
    REQUIRE(!ed.removeLayer(1));
}

TEST_CASE("MaterialLayerEditor counts", "[Editor][S104]") {
    MaterialLayerEditor ed;
    MaterialLayer l1(1, "a"); l1.setBlendMode(MaterialLayerBlendMode::Opaque); l1.setShadingModel(MaterialLayerShadingModel::Lit);
    MaterialLayer l2(2, "b"); l2.setBlendMode(MaterialLayerBlendMode::Additive); l2.setShadingModel(MaterialLayerShadingModel::ClearCoat); l2.setEnabled(false);
    MaterialLayer l3(3, "c"); l3.setBlendMode(MaterialLayerBlendMode::Opaque); l3.setLocked(true); l3.setShadingModel(MaterialLayerShadingModel::Unlit);
    ed.addLayer(l1); ed.addLayer(l2); ed.addLayer(l3);
    REQUIRE(ed.layerCount()                                                     == 3u);
    REQUIRE(ed.enabledCount()                                                   == 2u);
    REQUIRE(ed.lockedCount()                                                    == 1u);
    REQUIRE(ed.countByBlendMode(MaterialLayerBlendMode::Opaque)                 == 2u);
    REQUIRE(ed.countByShadingModel(MaterialLayerShadingModel::Lit)              == 1u);
    auto* found = ed.findLayer(2);
    REQUIRE(found != nullptr);
    REQUIRE(found->blendMode() == MaterialLayerBlendMode::Additive);
    REQUIRE(ed.findLayer(99) == nullptr);
}

// ── TextureAtlasEditor ───────────────────────────────────────────────────────

TEST_CASE("AtlasPackAlgorithm names", "[Editor][S104]") {
    REQUIRE(std::string(atlasPackAlgorithmName(AtlasPackAlgorithm::MaxRects))       == "MaxRects");
    REQUIRE(std::string(atlasPackAlgorithmName(AtlasPackAlgorithm::Guillotine))     == "Guillotine");
    REQUIRE(std::string(atlasPackAlgorithmName(AtlasPackAlgorithm::Shelf))          == "Shelf");
    REQUIRE(std::string(atlasPackAlgorithmName(AtlasPackAlgorithm::SkylineSimple))  == "SkylineSimple");
    REQUIRE(std::string(atlasPackAlgorithmName(AtlasPackAlgorithm::OptimalPacking)) == "OptimalPacking");
}

TEST_CASE("AtlasExportFormat names", "[Editor][S104]") {
    REQUIRE(std::string(atlasExportFormatName(AtlasExportFormat::PNG))  == "PNG");
    REQUIRE(std::string(atlasExportFormatName(AtlasExportFormat::JPEG)) == "JPEG");
    REQUIRE(std::string(atlasExportFormatName(AtlasExportFormat::WEBP)) == "WEBP");
    REQUIRE(std::string(atlasExportFormatName(AtlasExportFormat::KTX))  == "KTX");
    REQUIRE(std::string(atlasExportFormatName(AtlasExportFormat::DDS))  == "DDS");
}

TEST_CASE("AtlasSprite defaults", "[Editor][S104]") {
    AtlasSprite sprite("icon_sword", 64, 64);
    REQUIRE(sprite.name()      == "icon_sword");
    REQUIRE(sprite.width()     == 64u);
    REQUIRE(sprite.height()    == 64u);
    REQUIRE(sprite.padding()   == 2u);
    REQUIRE(!sprite.isRotated());
    REQUIRE(!sprite.isTrimmed());
}

TEST_CASE("AtlasSprite mutation", "[Editor][S104]") {
    AtlasSprite sprite("tree_bark", 128, 256);
    sprite.setPadding(4);
    sprite.setRotated(true);
    sprite.setTrimmed(true);
    REQUIRE(sprite.padding()   == 4u);
    REQUIRE(sprite.isRotated());
    REQUIRE(sprite.isTrimmed());
}

TEST_CASE("TextureAtlasEditor defaults", "[Editor][S104]") {
    TextureAtlasEditor ed;
    REQUIRE(ed.packAlgorithm()  == AtlasPackAlgorithm::MaxRects);
    REQUIRE(ed.exportFormat()   == AtlasExportFormat::PNG);
    REQUIRE(ed.maxWidth()       == 4096u);
    REQUIRE(ed.maxHeight()      == 4096u);
    REQUIRE(ed.allowsRotation());
    REQUIRE(ed.allowsTrimming());
    REQUIRE(ed.spriteCount()    == 0u);
}

TEST_CASE("TextureAtlasEditor add/remove sprites", "[Editor][S104]") {
    TextureAtlasEditor ed;
    REQUIRE(ed.addSprite(AtlasSprite("a", 32, 32)));
    REQUIRE(ed.addSprite(AtlasSprite("b", 64, 64)));
    REQUIRE(!ed.addSprite(AtlasSprite("a", 16, 16)));
    REQUIRE(ed.spriteCount() == 2u);
    REQUIRE(ed.removeSprite("a"));
    REQUIRE(ed.spriteCount() == 1u);
    REQUIRE(!ed.removeSprite("a"));
}

TEST_CASE("TextureAtlasEditor counts", "[Editor][S104]") {
    TextureAtlasEditor ed(2048, 2048);
    REQUIRE(ed.maxWidth()  == 2048u);
    REQUIRE(ed.maxHeight() == 2048u);
    AtlasSprite s1("a", 32, 32); s1.setRotated(true); s1.setTrimmed(true);
    AtlasSprite s2("b", 64, 64); s2.setTrimmed(true);
    AtlasSprite s3("c", 128, 128);
    ed.addSprite(s1); ed.addSprite(s2); ed.addSprite(s3);
    REQUIRE(ed.spriteCount()        == 3u);
    REQUIRE(ed.rotatedSpriteCount() == 1u);
    REQUIRE(ed.trimmedSpriteCount() == 2u);
    auto* found = ed.findSprite("b");
    REQUIRE(found != nullptr);
    REQUIRE(found->width() == 64u);
    REQUIRE(ed.findSprite("missing") == nullptr);
}

// ── ShaderVariantEditor ──────────────────────────────────────────────────────

TEST_CASE("ShaderVariantStage names", "[Editor][S104]") {
    REQUIRE(std::string(shaderVariantStageName(ShaderVariantStage::Vertex))      == "Vertex");
    REQUIRE(std::string(shaderVariantStageName(ShaderVariantStage::Fragment))    == "Fragment");
    REQUIRE(std::string(shaderVariantStageName(ShaderVariantStage::Geometry))    == "Geometry");
    REQUIRE(std::string(shaderVariantStageName(ShaderVariantStage::TessControl)) == "TessControl");
    REQUIRE(std::string(shaderVariantStageName(ShaderVariantStage::TessEval))    == "TessEval");
    REQUIRE(std::string(shaderVariantStageName(ShaderVariantStage::Compute))     == "Compute");
}

TEST_CASE("ShaderVariantCompileStatus names", "[Editor][S104]") {
    REQUIRE(std::string(shaderVariantCompileStatusName(ShaderVariantCompileStatus::NotCompiled)) == "NotCompiled");
    REQUIRE(std::string(shaderVariantCompileStatusName(ShaderVariantCompileStatus::Compiling))   == "Compiling");
    REQUIRE(std::string(shaderVariantCompileStatusName(ShaderVariantCompileStatus::Ok))          == "Ok");
    REQUIRE(std::string(shaderVariantCompileStatusName(ShaderVariantCompileStatus::Error))       == "Error");
    REQUIRE(std::string(shaderVariantCompileStatusName(ShaderVariantCompileStatus::Warning))     == "Warning");
}

TEST_CASE("ShaderTargetAPI names", "[Editor][S104]") {
    REQUIRE(std::string(shaderTargetAPIName(ShaderTargetAPI::Vulkan))  == "Vulkan");
    REQUIRE(std::string(shaderTargetAPIName(ShaderTargetAPI::DX12))    == "DX12");
    REQUIRE(std::string(shaderTargetAPIName(ShaderTargetAPI::Metal))   == "Metal");
    REQUIRE(std::string(shaderTargetAPIName(ShaderTargetAPI::OpenGL))  == "OpenGL");
    REQUIRE(std::string(shaderTargetAPIName(ShaderTargetAPI::WebGPU))  == "WebGPU");
}

TEST_CASE("ShaderVariant defaults", "[Editor][S104]") {
    ShaderVariant v(1, ShaderVariantStage::Vertex, ShaderTargetAPI::Vulkan);
    REQUIRE(v.id()         == 1u);
    REQUIRE(v.stage()      == ShaderVariantStage::Vertex);
    REQUIRE(v.api()        == ShaderTargetAPI::Vulkan);
    REQUIRE(v.status()     == ShaderVariantCompileStatus::NotCompiled);
    REQUIRE(v.entryPoint() == "main");
    REQUIRE(v.defines()    == "");
    REQUIRE(v.isEnabled());
    REQUIRE(!v.isCompiled());
}

TEST_CASE("ShaderVariant mutation", "[Editor][S104]") {
    ShaderVariant v(2, ShaderVariantStage::Fragment, ShaderTargetAPI::DX12);
    v.setStatus(ShaderVariantCompileStatus::Ok);
    v.setDefines("SHADOWS=1;MSAA=4");
    v.setEntryPoint("PSMain");
    v.setEnabled(false);
    REQUIRE(v.status()     == ShaderVariantCompileStatus::Ok);
    REQUIRE(v.isCompiled());
    REQUIRE(v.defines()    == "SHADOWS=1;MSAA=4");
    REQUIRE(v.entryPoint() == "PSMain");
    REQUIRE(!v.isEnabled());
}

TEST_CASE("ShaderVariantEditor add/remove", "[Editor][S104]") {
    ShaderVariantEditor ed;
    REQUIRE(ed.addVariant(ShaderVariant(1, ShaderVariantStage::Vertex,   ShaderTargetAPI::Vulkan)));
    REQUIRE(ed.addVariant(ShaderVariant(2, ShaderVariantStage::Fragment, ShaderTargetAPI::Vulkan)));
    REQUIRE(!ed.addVariant(ShaderVariant(1, ShaderVariantStage::Compute, ShaderTargetAPI::DX12)));
    REQUIRE(ed.variantCount() == 2u);
    REQUIRE(ed.removeVariant(1));
    REQUIRE(ed.variantCount() == 1u);
    REQUIRE(!ed.removeVariant(1));
}

TEST_CASE("ShaderVariantEditor counts", "[Editor][S104]") {
    ShaderVariantEditor ed;
    ShaderVariant v1(1, ShaderVariantStage::Vertex,   ShaderTargetAPI::Vulkan);
    v1.setStatus(ShaderVariantCompileStatus::Ok);
    ShaderVariant v2(2, ShaderVariantStage::Fragment, ShaderTargetAPI::Vulkan);
    v2.setStatus(ShaderVariantCompileStatus::Warning); v2.setEnabled(false);
    ShaderVariant v3(3, ShaderVariantStage::Vertex,   ShaderTargetAPI::DX12);
    v3.setStatus(ShaderVariantCompileStatus::Error);
    ed.addVariant(v1); ed.addVariant(v2); ed.addVariant(v3);
    REQUIRE(ed.variantCount()                                                  == 3u);
    REQUIRE(ed.enabledCount()                                                  == 2u);
    REQUIRE(ed.compiledCount()                                                 == 1u);
    REQUIRE(ed.countByStage(ShaderVariantStage::Vertex)                        == 2u);
    REQUIRE(ed.countByAPI(ShaderTargetAPI::Vulkan)                             == 2u);
    REQUIRE(ed.countByStatus(ShaderVariantCompileStatus::Error)                == 1u);
    auto* found = ed.findVariant(2);
    REQUIRE(found != nullptr);
    REQUIRE(found->stage() == ShaderVariantStage::Fragment);
    REQUIRE(ed.findVariant(99) == nullptr);
}
