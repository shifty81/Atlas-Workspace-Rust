#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LightmapEditor.h"
#include "NF/Editor/PostProcessEditor.h"

using namespace NF;

// ── S90: PostProcessEditor + LightmapEditor + RenderSettingsPanel ─

// ── PostProcessEditor ────────────────────────────────────────────

TEST_CASE("PostProcessEffect names are correct", "[Editor][S90]") {
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::Bloom))               == "Bloom");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::DepthOfField))        == "DepthOfField");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::MotionBlur))          == "MotionBlur");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::AmbientOcclusion))    == "AmbientOcclusion");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::ChromaticAberration)) == "ChromaticAberration");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::Vignette))            == "Vignette");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::LensFlare))           == "LensFlare");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::ColorGrading))        == "ColorGrading");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::ToneMapping))         == "ToneMapping");
    REQUIRE(std::string(postProcessEffectName(PostProcessEffect::FilmGrain))           == "FilmGrain");
}

TEST_CASE("ToneMappingOperator names are correct", "[Editor][S90]") {
    REQUIRE(std::string(toneMappingOperatorName(ToneMappingOperator::Linear))     == "Linear");
    REQUIRE(std::string(toneMappingOperatorName(ToneMappingOperator::Reinhard))   == "Reinhard");
    REQUIRE(std::string(toneMappingOperatorName(ToneMappingOperator::ACES))       == "ACES");
    REQUIRE(std::string(toneMappingOperatorName(ToneMappingOperator::Filmic))     == "Filmic");
    REQUIRE(std::string(toneMappingOperatorName(ToneMappingOperator::Uncharted2)) == "Uncharted2");
    REQUIRE(std::string(toneMappingOperatorName(ToneMappingOperator::Custom))     == "Custom");
}

TEST_CASE("PostProcessPassOrder names are correct", "[Editor][S90]") {
    REQUIRE(std::string(postProcessPassOrderName(PostProcessPassOrder::BeforeTransparency)) == "BeforeTransparency");
    REQUIRE(std::string(postProcessPassOrderName(PostProcessPassOrder::AfterTransparency))  == "AfterTransparency");
    REQUIRE(std::string(postProcessPassOrderName(PostProcessPassOrder::BeforeUI))           == "BeforeUI");
    REQUIRE(std::string(postProcessPassOrderName(PostProcessPassOrder::AfterUI))            == "AfterUI");
    REQUIRE(std::string(postProcessPassOrderName(PostProcessPassOrder::Custom))             == "Custom");
}

TEST_CASE("PostProcessLayer stores properties", "[Editor][S90]") {
    PostProcessLayer layer("BloomLayer", PostProcessEffect::Bloom);
    layer.setEnabled(true);
    layer.setIntensity(0.8f);
    layer.setPassOrder(PostProcessPassOrder::AfterTransparency);
    layer.setPriority(5);

    REQUIRE(layer.name()      == "BloomLayer");
    REQUIRE(layer.effect()    == PostProcessEffect::Bloom);
    REQUIRE(layer.isEnabled());
    REQUIRE(layer.intensity() == 0.8f);
    REQUIRE(layer.priority()  == 5);
}

TEST_CASE("PostProcessEditor addLayer findLayer setActiveLayer", "[Editor][S90]") {
    PostProcessEditor editor;
    PostProcessLayer bloom("Bloom1", PostProcessEffect::Bloom);
    PostProcessLayer dof("DOF1",     PostProcessEffect::DepthOfField);
    REQUIRE(editor.addLayer(bloom));
    REQUIRE(editor.addLayer(dof));
    REQUIRE(editor.layerCount() == 2);
    REQUIRE(editor.setActiveLayer("Bloom1"));
    REQUIRE(editor.activeLayer() == "Bloom1");
}

TEST_CASE("PostProcessEditor rejects duplicate layer name", "[Editor][S90]") {
    PostProcessEditor editor;
    PostProcessLayer layer("Vig", PostProcessEffect::Vignette);
    editor.addLayer(layer);
    REQUIRE_FALSE(editor.addLayer(layer));
}

TEST_CASE("PostProcessEditor remove activeLayer and enabledCount", "[Editor][S90]") {
    PostProcessEditor editor;
    PostProcessLayer l1("L1", PostProcessEffect::Bloom);
    PostProcessLayer l2("L2", PostProcessEffect::FilmGrain); l2.setEnabled(false);
    editor.addLayer(l1); editor.addLayer(l2);
    REQUIRE(editor.enabledCount() == 1);
    editor.removeLayer("L1");
    REQUIRE(editor.layerCount() == 1);
}

TEST_CASE("PostProcessEditor toneMapping and countByEffect", "[Editor][S90]") {
    PostProcessEditor editor;
    editor.setToneMapping(ToneMappingOperator::ACES);
    editor.addLayer(PostProcessLayer("B1", PostProcessEffect::Bloom));
    editor.addLayer(PostProcessLayer("B2", PostProcessEffect::Bloom));
    editor.addLayer(PostProcessLayer("MB", PostProcessEffect::MotionBlur));
    REQUIRE(editor.toneMapping()                                   == ToneMappingOperator::ACES);
    REQUIRE(editor.countByEffect(PostProcessEffect::Bloom)         == 2);
    REQUIRE(editor.countByEffect(PostProcessEffect::MotionBlur)    == 1);
}

TEST_CASE("PostProcessEditor MAX_LAYERS is 32", "[Editor][S90]") {
    REQUIRE(PostProcessEditor::MAX_LAYERS == 32);
}

// ── LightmapEditor ───────────────────────────────────────────────

TEST_CASE("LightmapQuality names are correct", "[Editor][S90]") {
    REQUIRE(std::string(lightmapQualityName(LightmapQuality::Draft))      == "Draft");
    REQUIRE(std::string(lightmapQualityName(LightmapQuality::Low))        == "Low");
    REQUIRE(std::string(lightmapQualityName(LightmapQuality::Medium))     == "Medium");
    REQUIRE(std::string(lightmapQualityName(LightmapQuality::High))       == "High");
    REQUIRE(std::string(lightmapQualityName(LightmapQuality::Production)) == "Production");
}

TEST_CASE("LightmapBakeMode names are correct", "[Editor][S90]") {
    REQUIRE(std::string(lightmapBakeModeName(LightmapBakeMode::Baked))          == "Baked");
    REQUIRE(std::string(lightmapBakeModeName(LightmapBakeMode::Mixed))          == "Mixed");
    REQUIRE(std::string(lightmapBakeModeName(LightmapBakeMode::Realtime))       == "Realtime");
    REQUIRE(std::string(lightmapBakeModeName(LightmapBakeMode::ShadowMaskOnly)) == "ShadowMaskOnly");
}

TEST_CASE("LightmapBakeStatus names are correct", "[Editor][S90]") {
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Idle))       == "Idle");
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Unwrapping)) == "Unwrapping");
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Sampling))   == "Sampling");
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Denoising))  == "Denoising");
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Done))       == "Done");
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Failed))     == "Failed");
    REQUIRE(std::string(lightmapBakeStatusName(LightmapBakeStatus::Cancelled))  == "Cancelled");
}

TEST_CASE("LightmapBakeConfig stores properties and isDone", "[Editor][S90]") {
    LightmapBakeConfig cfg("FloorMesh");
    cfg.setQuality(LightmapQuality::High);
    cfg.setBakeMode(LightmapBakeMode::Baked);
    cfg.setTexelDensity(2.0f);
    cfg.setTextureSizePx(1024);
    cfg.setStatus(LightmapBakeStatus::Done);
    cfg.setProgress(1.0f);

    REQUIRE(cfg.meshName()      == "FloorMesh");
    REQUIRE(cfg.quality()       == LightmapQuality::High);
    REQUIRE(cfg.textureSizePx() == 1024);
    REQUIRE(cfg.isDone());
    REQUIRE_FALSE(cfg.isBaking());
}

TEST_CASE("LightmapBakeConfig isBaking for active statuses", "[Editor][S90]") {
    LightmapBakeConfig cfg("WallMesh");
    cfg.setStatus(LightmapBakeStatus::Sampling);
    REQUIRE(cfg.isBaking());
    cfg.setStatus(LightmapBakeStatus::Denoising);
    REQUIRE(cfg.isBaking());
}

TEST_CASE("LightmapEditorPanel addConfig findConfig remove", "[Editor][S90]") {
    LightmapEditorPanel panel;
    LightmapBakeConfig c1("Mesh1"); c1.setStatus(LightmapBakeStatus::Done);
    LightmapBakeConfig c2("Mesh2");
    REQUIRE(panel.addConfig(c1));
    REQUIRE(panel.addConfig(c2));
    REQUIRE(panel.configCount() == 2);
    REQUIRE(panel.doneCount()   == 1);
    panel.removeConfig("Mesh1");
    REQUIRE(panel.configCount() == 1);
}

TEST_CASE("LightmapEditorPanel rejects duplicate meshName", "[Editor][S90]") {
    LightmapEditorPanel panel;
    LightmapBakeConfig cfg("DupMesh");
    panel.addConfig(cfg);
    REQUIRE_FALSE(panel.addConfig(cfg));
}

TEST_CASE("LightmapEditorPanel countByQuality and countByMode", "[Editor][S90]") {
    LightmapEditorPanel panel;
    LightmapBakeConfig c1("A"); c1.setQuality(LightmapQuality::High); c1.setBakeMode(LightmapBakeMode::Baked);
    LightmapBakeConfig c2("B"); c2.setQuality(LightmapQuality::High); c2.setBakeMode(LightmapBakeMode::Mixed);
    LightmapBakeConfig c3("C"); c3.setQuality(LightmapQuality::Low);  c3.setBakeMode(LightmapBakeMode::Baked);
    panel.addConfig(c1); panel.addConfig(c2); panel.addConfig(c3);
    REQUIRE(panel.countByQuality(LightmapQuality::High)    == 2);
    REQUIRE(panel.countByMode(LightmapBakeMode::Baked)     == 2);
}

TEST_CASE("LightmapEditorPanel MAX_CONFIGS is 256", "[Editor][S90]") {
    REQUIRE(LightmapEditorPanel::MAX_CONFIGS == 256);
}

// ── RenderSettingsPanel ──────────────────────────────────────────

TEST_CASE("RenderPipeline names are correct", "[Editor][S90]") {
    REQUIRE(std::string(renderPipelineName(RenderPipeline::Forward))       == "Forward");
    REQUIRE(std::string(renderPipelineName(RenderPipeline::Deferred))      == "Deferred");
    REQUIRE(std::string(renderPipelineName(RenderPipeline::ForwardPlus))   == "ForwardPlus");
    REQUIRE(std::string(renderPipelineName(RenderPipeline::TiledDeferred)) == "TiledDeferred");
    REQUIRE(std::string(renderPipelineName(RenderPipeline::Custom))        == "Custom");
}

TEST_CASE("AntiAliasingMode names are correct", "[Editor][S90]") {
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::None))   == "None");
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::MSAA2x)) == "MSAA2x");
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::MSAA4x)) == "MSAA4x");
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::FXAA))   == "FXAA");
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::TAA))    == "TAA");
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::DLSS))   == "DLSS");
    REQUIRE(std::string(antiAliasingModeName(AntiAliasingMode::FSR))    == "FSR");
}

TEST_CASE("ShadowQuality names are correct", "[Editor][S90]") {
    REQUIRE(std::string(shadowQualityName(ShadowQuality::Disabled)) == "Disabled");
    REQUIRE(std::string(shadowQualityName(ShadowQuality::Low))      == "Low");
    REQUIRE(std::string(shadowQualityName(ShadowQuality::Medium))   == "Medium");
    REQUIRE(std::string(shadowQualityName(ShadowQuality::High))     == "High");
    REQUIRE(std::string(shadowQualityName(ShadowQuality::Ultra))    == "Ultra");
}

TEST_CASE("TextureQualityLevel names are correct", "[Editor][S90]") {
    REQUIRE(std::string(textureQualityLevelName(TextureQualityLevel::Low))       == "Low");
    REQUIRE(std::string(textureQualityLevelName(TextureQualityLevel::Medium))    == "Medium");
    REQUIRE(std::string(textureQualityLevelName(TextureQualityLevel::High))      == "High");
    REQUIRE(std::string(textureQualityLevelName(TextureQualityLevel::Ultra))     == "Ultra");
    REQUIRE(std::string(textureQualityLevelName(TextureQualityLevel::Streaming)) == "Streaming");
}

TEST_CASE("RenderSettingsPanel default values", "[Editor][S90]") {
    RenderSettingsPanel panel;
    REQUIRE(panel.pipeline()      == RenderPipeline::Deferred);
    REQUIRE(panel.antiAliasing()  == AntiAliasingMode::TAA);
    REQUIRE(panel.shadowQuality() == ShadowQuality::High);
    REQUIRE(panel.vsyncEnabled());
    REQUIRE_FALSE(panel.hdrEnabled());
    REQUIRE_FALSE(panel.rayTracingEnabled());
    REQUIRE(panel.targetFPS()    == 60);
    REQUIRE(panel.renderScale()  == 1.0f);
}

TEST_CASE("RenderSettingsPanel setters and getters", "[Editor][S90]") {
    RenderSettingsPanel panel;
    panel.setPipeline(RenderPipeline::ForwardPlus);
    panel.setAntiAliasing(AntiAliasingMode::DLSS);
    panel.setShadowQuality(ShadowQuality::Ultra);
    panel.setHDREnabled(true);
    panel.setRayTracingEnabled(true);
    panel.setTargetFPS(120);
    panel.setRenderScale(0.75f);

    REQUIRE(panel.pipeline()          == RenderPipeline::ForwardPlus);
    REQUIRE(panel.antiAliasing()      == AntiAliasingMode::DLSS);
    REQUIRE(panel.shadowQuality()     == ShadowQuality::Ultra);
    REQUIRE(panel.hdrEnabled());
    REQUIRE(panel.rayTracingEnabled());
    REQUIRE(panel.targetFPS()         == 120);
    REQUIRE(panel.renderScale()       == 0.75f);
}

TEST_CASE("RenderSettingsPanel isHighFidelity", "[Editor][S90]") {
    RenderSettingsPanel panel;
    panel.setShadowQuality(ShadowQuality::High);
    panel.setTextureQuality(TextureQualityLevel::High);
    REQUIRE(panel.isHighFidelity());
    panel.setShadowQuality(ShadowQuality::Low);
    REQUIRE_FALSE(panel.isHighFidelity());
}

TEST_CASE("RenderSettingsPanel maxDrawDistance", "[Editor][S90]") {
    RenderSettingsPanel panel;
    panel.setMaxDrawDistance(5000.0f);
    REQUIRE(panel.maxDrawDistance() == 5000.0f);
}
