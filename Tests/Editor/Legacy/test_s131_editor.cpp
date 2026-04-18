// S131 editor tests: GraphicsSettingsEditor, DisplayModeEditor, ResolutionEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ResolutionEditor.h"
#include "NF/Editor/DisplayModeEditor.h"
#include "NF/Editor/GraphicsSettingsEditor.h"

using namespace NF;

// ── GraphicsSettingsEditor ────────────────────────────────────────────────────

TEST_CASE("GfxQualityLevel names", "[Editor][S131]") {
    REQUIRE(std::string(gfxQualityLevelName(GfxQualityLevel::Low))    == "Low");
    REQUIRE(std::string(gfxQualityLevelName(GfxQualityLevel::Medium)) == "Medium");
    REQUIRE(std::string(gfxQualityLevelName(GfxQualityLevel::High))   == "High");
    REQUIRE(std::string(gfxQualityLevelName(GfxQualityLevel::Ultra))  == "Ultra");
    REQUIRE(std::string(gfxQualityLevelName(GfxQualityLevel::Custom)) == "Custom");
}

TEST_CASE("GfxFeature names", "[Editor][S131]") {
    REQUIRE(std::string(gfxFeatureName(GfxFeature::Shadows))       == "Shadows");
    REQUIRE(std::string(gfxFeatureName(GfxFeature::AO))            == "AO");
    REQUIRE(std::string(gfxFeatureName(GfxFeature::Reflections))   == "Reflections");
    REQUIRE(std::string(gfxFeatureName(GfxFeature::AntiAliasing))  == "AntiAliasing");
    REQUIRE(std::string(gfxFeatureName(GfxFeature::VolumetricFog)) == "VolumetricFog");
    REQUIRE(std::string(gfxFeatureName(GfxFeature::MotionBlur))    == "MotionBlur");
}

TEST_CASE("GfxPreset defaults", "[Editor][S131]") {
    GfxPreset p(1, "high_preset", GfxQualityLevel::High);
    REQUIRE(p.id()                  == 1u);
    REQUIRE(p.name()                == "high_preset");
    REQUIRE(p.quality()             == GfxQualityLevel::High);
    REQUIRE(p.enabledFeaturesMask() == 0u);
    REQUIRE(p.shadowDistance()      == 100.0f);
    REQUIRE(p.isEnabled());
}

TEST_CASE("GfxPreset mutation", "[Editor][S131]") {
    GfxPreset p(2, "ultra_preset", GfxQualityLevel::Ultra);
    p.setEnabledFeaturesMask(0xFFu);
    p.setShadowDistance(500.0f);
    p.setIsEnabled(false);
    REQUIRE(p.enabledFeaturesMask() == 0xFFu);
    REQUIRE(p.shadowDistance()      == 500.0f);
    REQUIRE(!p.isEnabled());
}

TEST_CASE("GraphicsSettingsEditor defaults", "[Editor][S131]") {
    GraphicsSettingsEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByQuality());
    REQUIRE(ed.defaultShadowDistance() == 50.0f);
    REQUIRE(ed.presetCount()           == 0u);
}

TEST_CASE("GraphicsSettingsEditor add/remove presets", "[Editor][S131]") {
    GraphicsSettingsEditor ed;
    REQUIRE(ed.addPreset(GfxPreset(1, "p_low",    GfxQualityLevel::Low)));
    REQUIRE(ed.addPreset(GfxPreset(2, "p_medium", GfxQualityLevel::Medium)));
    REQUIRE(ed.addPreset(GfxPreset(3, "p_high",   GfxQualityLevel::High)));
    REQUIRE(!ed.addPreset(GfxPreset(1, "p_low",   GfxQualityLevel::Low)));
    REQUIRE(ed.presetCount() == 3u);
    REQUIRE(ed.removePreset(2));
    REQUIRE(ed.presetCount() == 2u);
    REQUIRE(!ed.removePreset(99));
}

TEST_CASE("GraphicsSettingsEditor counts and find", "[Editor][S131]") {
    GraphicsSettingsEditor ed;
    GfxPreset p1(1, "p_a", GfxQualityLevel::High);
    GfxPreset p2(2, "p_b", GfxQualityLevel::High);
    GfxPreset p3(3, "p_c", GfxQualityLevel::Ultra);
    GfxPreset p4(4, "p_d", GfxQualityLevel::Low); p4.setIsEnabled(false);
    ed.addPreset(p1); ed.addPreset(p2); ed.addPreset(p3); ed.addPreset(p4);
    REQUIRE(ed.countByQuality(GfxQualityLevel::High)   == 2u);
    REQUIRE(ed.countByQuality(GfxQualityLevel::Ultra)  == 1u);
    REQUIRE(ed.countByQuality(GfxQualityLevel::Medium) == 0u);
    REQUIRE(ed.countEnabled()                          == 3u);
    auto* found = ed.findPreset(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->quality() == GfxQualityLevel::Ultra);
    REQUIRE(ed.findPreset(99) == nullptr);
}

TEST_CASE("GraphicsSettingsEditor settings mutation", "[Editor][S131]") {
    GraphicsSettingsEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByQuality(false);
    ed.setDefaultShadowDistance(200.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByQuality());
    REQUIRE(ed.defaultShadowDistance() == 200.0f);
}

// ── DisplayModeEditor ─────────────────────────────────────────────────────────

TEST_CASE("DisplayApi names", "[Editor][S131]") {
    REQUIRE(std::string(displayApiName(DisplayApi::DirectX11)) == "DirectX11");
    REQUIRE(std::string(displayApiName(DisplayApi::DirectX12)) == "DirectX12");
    REQUIRE(std::string(displayApiName(DisplayApi::Vulkan))    == "Vulkan");
    REQUIRE(std::string(displayApiName(DisplayApi::Metal))     == "Metal");
    REQUIRE(std::string(displayApiName(DisplayApi::OpenGL))    == "OpenGL");
}

TEST_CASE("DisplaySync names", "[Editor][S131]") {
    REQUIRE(std::string(displaySyncName(DisplaySync::Off))          == "Off");
    REQUIRE(std::string(displaySyncName(DisplaySync::VSync))        == "VSync");
    REQUIRE(std::string(displaySyncName(DisplaySync::AdaptiveSync)) == "AdaptiveSync");
    REQUIRE(std::string(displaySyncName(DisplaySync::FreeSync))     == "FreeSync");
    REQUIRE(std::string(displaySyncName(DisplaySync::GSync))        == "GSync");
}

TEST_CASE("DisplayMode defaults", "[Editor][S131]") {
    DisplayMode m(1, "dx12_vsync", DisplayApi::DirectX12, DisplaySync::VSync);
    REQUIRE(m.id()          == 1u);
    REQUIRE(m.name()        == "dx12_vsync");
    REQUIRE(m.api()         == DisplayApi::DirectX12);
    REQUIRE(m.sync()        == DisplaySync::VSync);
    REQUIRE(m.refreshRate() == 60u);
    REQUIRE(!m.isFullscreen());
    REQUIRE(m.isEnabled());
}

TEST_CASE("DisplayMode mutation", "[Editor][S131]") {
    DisplayMode m(2, "vk_gsync", DisplayApi::Vulkan, DisplaySync::GSync);
    m.setRefreshRate(144u);
    m.setIsFullscreen(true);
    m.setIsEnabled(false);
    REQUIRE(m.refreshRate() == 144u);
    REQUIRE(m.isFullscreen());
    REQUIRE(!m.isEnabled());
}

TEST_CASE("DisplayModeEditor defaults", "[Editor][S131]") {
    DisplayModeEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByApi());
    REQUIRE(ed.defaultRefreshRate() == 60u);
    REQUIRE(ed.modeCount()          == 0u);
}

TEST_CASE("DisplayModeEditor add/remove modes", "[Editor][S131]") {
    DisplayModeEditor ed;
    REQUIRE(ed.addMode(DisplayMode(1, "m_a", DisplayApi::DirectX11, DisplaySync::Off)));
    REQUIRE(ed.addMode(DisplayMode(2, "m_b", DisplayApi::DirectX12, DisplaySync::VSync)));
    REQUIRE(ed.addMode(DisplayMode(3, "m_c", DisplayApi::Vulkan,    DisplaySync::FreeSync)));
    REQUIRE(!ed.addMode(DisplayMode(1, "m_a", DisplayApi::DirectX11, DisplaySync::Off)));
    REQUIRE(ed.modeCount() == 3u);
    REQUIRE(ed.removeMode(2));
    REQUIRE(ed.modeCount() == 2u);
    REQUIRE(!ed.removeMode(99));
}

TEST_CASE("DisplayModeEditor counts and find", "[Editor][S131]") {
    DisplayModeEditor ed;
    DisplayMode m1(1, "m_a", DisplayApi::Vulkan, DisplaySync::Off);
    DisplayMode m2(2, "m_b", DisplayApi::Vulkan, DisplaySync::FreeSync);
    DisplayMode m3(3, "m_c", DisplayApi::Metal,  DisplaySync::GSync);
    DisplayMode m4(4, "m_d", DisplayApi::OpenGL, DisplaySync::VSync); m4.setIsEnabled(false);
    ed.addMode(m1); ed.addMode(m2); ed.addMode(m3); ed.addMode(m4);
    REQUIRE(ed.countByApi(DisplayApi::Vulkan)          == 2u);
    REQUIRE(ed.countByApi(DisplayApi::Metal)           == 1u);
    REQUIRE(ed.countByApi(DisplayApi::DirectX11)       == 0u);
    REQUIRE(ed.countBySync(DisplaySync::Off)           == 1u);
    REQUIRE(ed.countBySync(DisplaySync::FreeSync)      == 1u);
    REQUIRE(ed.countEnabled()                          == 3u);
    auto* found = ed.findMode(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->api() == DisplayApi::Metal);
    REQUIRE(ed.findMode(99) == nullptr);
}

TEST_CASE("DisplayModeEditor settings mutation", "[Editor][S131]") {
    DisplayModeEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByApi(true);
    ed.setDefaultRefreshRate(120u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByApi());
    REQUIRE(ed.defaultRefreshRate() == 120u);
}

// ── ResolutionEditor ──────────────────────────────────────────────────────────

TEST_CASE("ResolutionScale names", "[Editor][S131]") {
    REQUIRE(std::string(resolutionScaleName(ResolutionScale::Native))  == "Native");
    REQUIRE(std::string(resolutionScaleName(ResolutionScale::Half))    == "Half");
    REQUIRE(std::string(resolutionScaleName(ResolutionScale::Quarter)) == "Quarter");
    REQUIRE(std::string(resolutionScaleName(ResolutionScale::Dynamic)) == "Dynamic");
    REQUIRE(std::string(resolutionScaleName(ResolutionScale::Custom))  == "Custom");
}

TEST_CASE("ResolutionAspect names", "[Editor][S131]") {
    REQUIRE(std::string(resolutionAspectName(ResolutionAspect::Wide16x9))  == "Wide16x9");
    REQUIRE(std::string(resolutionAspectName(ResolutionAspect::Ultra21x9)) == "Ultra21x9");
    REQUIRE(std::string(resolutionAspectName(ResolutionAspect::Square4x3)) == "Square4x3");
    REQUIRE(std::string(resolutionAspectName(ResolutionAspect::Tall9x16))  == "Tall9x16");
    REQUIRE(std::string(resolutionAspectName(ResolutionAspect::Custom))    == "Custom");
}

TEST_CASE("ResolutionProfile defaults", "[Editor][S131]") {
    ResolutionProfile p(1, "1080p", ResolutionScale::Native, ResolutionAspect::Wide16x9);
    REQUIRE(p.id()        == 1u);
    REQUIRE(p.name()      == "1080p");
    REQUIRE(p.scale()     == ResolutionScale::Native);
    REQUIRE(p.aspect()    == ResolutionAspect::Wide16x9);
    REQUIRE(p.width()     == 1920u);
    REQUIRE(p.height()    == 1080u);
    REQUIRE(p.isEnabled());
}

TEST_CASE("ResolutionProfile mutation", "[Editor][S131]") {
    ResolutionProfile p(2, "720p_half", ResolutionScale::Half, ResolutionAspect::Wide16x9);
    p.setWidth(1280u);
    p.setHeight(720u);
    p.setIsEnabled(false);
    REQUIRE(p.width()     == 1280u);
    REQUIRE(p.height()    == 720u);
    REQUIRE(!p.isEnabled());
}

TEST_CASE("ResolutionEditor defaults", "[Editor][S131]") {
    ResolutionEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByAspect());
    REQUIRE(ed.defaultWidth()  == 1280u);
    REQUIRE(ed.profileCount()  == 0u);
}

TEST_CASE("ResolutionEditor add/remove profiles", "[Editor][S131]") {
    ResolutionEditor ed;
    REQUIRE(ed.addProfile(ResolutionProfile(1, "p_a", ResolutionScale::Native,  ResolutionAspect::Wide16x9)));
    REQUIRE(ed.addProfile(ResolutionProfile(2, "p_b", ResolutionScale::Half,    ResolutionAspect::Wide16x9)));
    REQUIRE(ed.addProfile(ResolutionProfile(3, "p_c", ResolutionScale::Quarter, ResolutionAspect::Ultra21x9)));
    REQUIRE(!ed.addProfile(ResolutionProfile(1, "p_a", ResolutionScale::Native, ResolutionAspect::Wide16x9)));
    REQUIRE(ed.profileCount() == 3u);
    REQUIRE(ed.removeProfile(2));
    REQUIRE(ed.profileCount() == 2u);
    REQUIRE(!ed.removeProfile(99));
}

TEST_CASE("ResolutionEditor counts and find", "[Editor][S131]") {
    ResolutionEditor ed;
    ResolutionProfile p1(1, "p_a", ResolutionScale::Native,  ResolutionAspect::Wide16x9);
    ResolutionProfile p2(2, "p_b", ResolutionScale::Native,  ResolutionAspect::Ultra21x9);
    ResolutionProfile p3(3, "p_c", ResolutionScale::Dynamic, ResolutionAspect::Wide16x9);
    ResolutionProfile p4(4, "p_d", ResolutionScale::Custom,  ResolutionAspect::Square4x3); p4.setIsEnabled(false);
    ed.addProfile(p1); ed.addProfile(p2); ed.addProfile(p3); ed.addProfile(p4);
    REQUIRE(ed.countByScale(ResolutionScale::Native)          == 2u);
    REQUIRE(ed.countByScale(ResolutionScale::Dynamic)         == 1u);
    REQUIRE(ed.countByScale(ResolutionScale::Half)            == 0u);
    REQUIRE(ed.countByAspect(ResolutionAspect::Wide16x9)      == 2u);
    REQUIRE(ed.countByAspect(ResolutionAspect::Ultra21x9)     == 1u);
    REQUIRE(ed.countEnabled()                                 == 3u);
    auto* found = ed.findProfile(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->scale() == ResolutionScale::Dynamic);
    REQUIRE(ed.findProfile(99) == nullptr);
}

TEST_CASE("ResolutionEditor settings mutation", "[Editor][S131]") {
    ResolutionEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByAspect(true);
    ed.setDefaultWidth(1920u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByAspect());
    REQUIRE(ed.defaultWidth() == 1920u);
}
