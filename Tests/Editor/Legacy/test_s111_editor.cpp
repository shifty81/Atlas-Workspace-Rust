// S111 editor tests: VFXGraphEditor, TrailEditor, ImpactEffectEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ImpactEffectEditor.h"
#include "NF/Editor/TrailEditor.h"
#include "NF/Editor/VFXGraphEditor.h"

using namespace NF;

// ── VFXGraphEditor ───────────────────────────────────────────────────────────

TEST_CASE("VFXNodeType names", "[Editor][S111]") {
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Emitter))      == "Emitter");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Force))        == "Force");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Collision))    == "Collision");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::ColorOverLife))== "ColorOverLife");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::SizeOverLife)) == "SizeOverLife");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Velocity))     == "Velocity");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Spawn))        == "Spawn");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Solver))       == "Solver");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Noise))        == "Noise");
    REQUIRE(std::string(vfxNodeTypeName(VFXNodeType::Output))       == "Output");
}

TEST_CASE("VFXBlendMode names", "[Editor][S111]") {
    REQUIRE(std::string(vfxBlendModeName(VFXBlendMode::Additive))      == "Additive");
    REQUIRE(std::string(vfxBlendModeName(VFXBlendMode::AlphaBlend))    == "AlphaBlend");
    REQUIRE(std::string(vfxBlendModeName(VFXBlendMode::Multiply))      == "Multiply");
    REQUIRE(std::string(vfxBlendModeName(VFXBlendMode::Screen))        == "Screen");
    REQUIRE(std::string(vfxBlendModeName(VFXBlendMode::Premultiplied)) == "Premultiplied");
    REQUIRE(std::string(vfxBlendModeName(VFXBlendMode::Opaque))        == "Opaque");
}

TEST_CASE("VFXSimulationSpace names", "[Editor][S111]") {
    REQUIRE(std::string(vfxSimulationSpaceName(VFXSimulationSpace::World))  == "World");
    REQUIRE(std::string(vfxSimulationSpaceName(VFXSimulationSpace::Local))  == "Local");
    REQUIRE(std::string(vfxSimulationSpaceName(VFXSimulationSpace::Custom)) == "Custom");
}

TEST_CASE("VFXNode defaults", "[Editor][S111]") {
    VFXNode node(1, "fire_emitter", VFXNodeType::Emitter);
    REQUIRE(node.id()        == 1u);
    REQUIRE(node.name()      == "fire_emitter");
    REQUIRE(node.nodeType()  == VFXNodeType::Emitter);
    REQUIRE(node.posX()      == 0.0f);
    REQUIRE(node.posY()      == 0.0f);
    REQUIRE(node.isEnabled());
    REQUIRE(node.blendMode() == VFXBlendMode::AlphaBlend);
}

TEST_CASE("VFXNode mutation", "[Editor][S111]") {
    VFXNode node(2, "gravity", VFXNodeType::Force);
    node.setPosX(100.0f);
    node.setPosY(200.0f);
    node.setEnabled(false);
    node.setBlendMode(VFXBlendMode::Additive);
    REQUIRE(node.posX()      == 100.0f);
    REQUIRE(node.posY()      == 200.0f);
    REQUIRE(!node.isEnabled());
    REQUIRE(node.blendMode() == VFXBlendMode::Additive);
}

TEST_CASE("VFXGraphEditor defaults", "[Editor][S111]") {
    VFXGraphEditor ed;
    REQUIRE(ed.simulationSpace() == VFXSimulationSpace::World);
    REQUIRE(ed.isShowGrid());
    REQUIRE(ed.isShowMinimap());
    REQUIRE(ed.previewDuration() == 5.0f);
    REQUIRE(ed.nodeCount()       == 0u);
}

TEST_CASE("VFXGraphEditor add/remove nodes", "[Editor][S111]") {
    VFXGraphEditor ed;
    REQUIRE(ed.addNode(VFXNode(1, "emitter",  VFXNodeType::Emitter)));
    REQUIRE(ed.addNode(VFXNode(2, "output",   VFXNodeType::Output)));
    REQUIRE(ed.addNode(VFXNode(3, "noise",    VFXNodeType::Noise)));
    REQUIRE(!ed.addNode(VFXNode(1, "emitter", VFXNodeType::Emitter)));
    REQUIRE(ed.nodeCount() == 3u);
    REQUIRE(ed.removeNode(2));
    REQUIRE(ed.nodeCount() == 2u);
    REQUIRE(!ed.removeNode(99));
}

TEST_CASE("VFXGraphEditor counts and find", "[Editor][S111]") {
    VFXGraphEditor ed;
    VFXNode n1(1, "emitter1",  VFXNodeType::Emitter);
    VFXNode n2(2, "emitter2",  VFXNodeType::Emitter);
    VFXNode n3(3, "force",     VFXNodeType::Force);    n3.setEnabled(false);
    VFXNode n4(4, "output",    VFXNodeType::Output);
    ed.addNode(n1); ed.addNode(n2); ed.addNode(n3); ed.addNode(n4);
    REQUIRE(ed.countByType(VFXNodeType::Emitter)  == 2u);
    REQUIRE(ed.countByType(VFXNodeType::Force)    == 1u);
    REQUIRE(ed.countByType(VFXNodeType::Spawn)    == 0u);
    REQUIRE(ed.countEnabled()                     == 3u);
    auto* found = ed.findNode(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->nodeType() == VFXNodeType::Force);
    REQUIRE(ed.findNode(99) == nullptr);
}

TEST_CASE("VFXGraphEditor mutation", "[Editor][S111]") {
    VFXGraphEditor ed;
    ed.setSimulationSpace(VFXSimulationSpace::Local);
    ed.setShowGrid(false);
    ed.setShowMinimap(false);
    ed.setPreviewDuration(10.0f);
    REQUIRE(ed.simulationSpace() == VFXSimulationSpace::Local);
    REQUIRE(!ed.isShowGrid());
    REQUIRE(!ed.isShowMinimap());
    REQUIRE(ed.previewDuration() == 10.0f);
}

// ── TrailEditor ──────────────────────────────────────────────────────────────

TEST_CASE("TrailRendererType names", "[Editor][S111]") {
    REQUIRE(std::string(trailRendererTypeName(TrailRendererType::Line))      == "Line");
    REQUIRE(std::string(trailRendererTypeName(TrailRendererType::Ribbon))    == "Ribbon");
    REQUIRE(std::string(trailRendererTypeName(TrailRendererType::Tube))      == "Tube");
    REQUIRE(std::string(trailRendererTypeName(TrailRendererType::Billboard)) == "Billboard");
    REQUIRE(std::string(trailRendererTypeName(TrailRendererType::Mesh))      == "Mesh");
}

TEST_CASE("TrailColorMode names", "[Editor][S111]") {
    REQUIRE(std::string(trailColorModeName(TrailColorMode::Constant))     == "Constant");
    REQUIRE(std::string(trailColorModeName(TrailColorMode::OverLifetime)) == "OverLifetime");
    REQUIRE(std::string(trailColorModeName(TrailColorMode::OverSpeed))    == "OverSpeed");
    REQUIRE(std::string(trailColorModeName(TrailColorMode::OverAngle))    == "OverAngle");
}

TEST_CASE("TrailWidthMode names", "[Editor][S111]") {
    REQUIRE(std::string(trailWidthModeName(TrailWidthMode::Constant))     == "Constant");
    REQUIRE(std::string(trailWidthModeName(TrailWidthMode::OverLifetime)) == "OverLifetime");
    REQUIRE(std::string(trailWidthModeName(TrailWidthMode::OverSpeed))    == "OverSpeed");
}

TEST_CASE("TrailConfig defaults", "[Editor][S111]") {
    TrailConfig cfg(1, "fire_trail");
    REQUIRE(cfg.id()                == 1u);
    REQUIRE(cfg.name()              == "fire_trail");
    REQUIRE(cfg.rendererType()      == TrailRendererType::Ribbon);
    REQUIRE(cfg.colorMode()         == TrailColorMode::OverLifetime);
    REQUIRE(cfg.widthMode()         == TrailWidthMode::OverLifetime);
    REQUIRE(cfg.lifetime()          == 1.0f);
    REQUIRE(cfg.minVertexDistance() == 0.1f);
    REQUIRE(cfg.width()             == 0.1f);
    REQUIRE(!cfg.castShadows());
}

TEST_CASE("TrailConfig mutation", "[Editor][S111]") {
    TrailConfig cfg(2, "sword_slash");
    cfg.setRendererType(TrailRendererType::Mesh);
    cfg.setColorMode(TrailColorMode::OverSpeed);
    cfg.setWidthMode(TrailWidthMode::Constant);
    cfg.setLifetime(2.0f);
    cfg.setMinVertexDistance(0.05f);
    cfg.setWidth(0.3f);
    cfg.setCastShadows(true);
    REQUIRE(cfg.rendererType()      == TrailRendererType::Mesh);
    REQUIRE(cfg.colorMode()         == TrailColorMode::OverSpeed);
    REQUIRE(cfg.widthMode()         == TrailWidthMode::Constant);
    REQUIRE(cfg.lifetime()          == 2.0f);
    REQUIRE(cfg.minVertexDistance() == 0.05f);
    REQUIRE(cfg.width()             == 0.3f);
    REQUIRE(cfg.castShadows());
}

TEST_CASE("TrailEditor defaults", "[Editor][S111]") {
    TrailEditor ed;
    REQUIRE(ed.isShowGizmos());
    REQUIRE(ed.isLoopPreview());
    REQUIRE(ed.previewSpeed()  == 1.0f);
    REQUIRE(ed.configCount()   == 0u);
}

TEST_CASE("TrailEditor add/remove configs", "[Editor][S111]") {
    TrailEditor ed;
    REQUIRE(ed.addConfig(TrailConfig(1, "fire_trail")));
    REQUIRE(ed.addConfig(TrailConfig(2, "ice_trail")));
    REQUIRE(ed.addConfig(TrailConfig(3, "slash_trail")));
    REQUIRE(!ed.addConfig(TrailConfig(1, "fire_trail")));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("TrailEditor counts and find", "[Editor][S111]") {
    TrailEditor ed;
    TrailConfig c1(1, "fire");
    TrailConfig c2(2, "ice");    c2.setRendererType(TrailRendererType::Tube);
    TrailConfig c3(3, "slash");  c3.setColorMode(TrailColorMode::OverSpeed); c3.setCastShadows(true);
    TrailConfig c4(4, "magic");  c4.setRendererType(TrailRendererType::Tube); c4.setCastShadows(true);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countByRendererType(TrailRendererType::Ribbon)      == 2u);
    REQUIRE(ed.countByRendererType(TrailRendererType::Tube)        == 2u);
    REQUIRE(ed.countByRendererType(TrailRendererType::Mesh)        == 0u);
    REQUIRE(ed.countByColorMode(TrailColorMode::OverLifetime)      == 3u);
    REQUIRE(ed.countByColorMode(TrailColorMode::OverSpeed)         == 1u);
    REQUIRE(ed.countCastingShadows()                               == 2u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->colorMode() == TrailColorMode::OverSpeed);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("TrailEditor mutation", "[Editor][S111]") {
    TrailEditor ed;
    ed.setShowGizmos(false);
    ed.setLoopPreview(false);
    ed.setPreviewSpeed(2.0f);
    REQUIRE(!ed.isShowGizmos());
    REQUIRE(!ed.isLoopPreview());
    REQUIRE(ed.previewSpeed() == 2.0f);
}

// ── ImpactEffectEditor ───────────────────────────────────────────────────────

TEST_CASE("ImpactSurface names", "[Editor][S111]") {
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Concrete)) == "Concrete");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Metal))    == "Metal");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Wood))     == "Wood");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Dirt))     == "Dirt");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Water))    == "Water");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Glass))    == "Glass");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Flesh))    == "Flesh");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Sand))     == "Sand");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Stone))    == "Stone");
    REQUIRE(std::string(impactSurfaceName(ImpactSurface::Fabric))   == "Fabric");
}

TEST_CASE("ImpactEffectLayer names", "[Editor][S111]") {
    REQUIRE(std::string(impactEffectLayerName(ImpactEffectLayer::Decal))    == "Decal");
    REQUIRE(std::string(impactEffectLayerName(ImpactEffectLayer::Particles))== "Particles");
    REQUIRE(std::string(impactEffectLayerName(ImpactEffectLayer::Audio))    == "Audio");
    REQUIRE(std::string(impactEffectLayerName(ImpactEffectLayer::Light))    == "Light");
    REQUIRE(std::string(impactEffectLayerName(ImpactEffectLayer::Physics))  == "Physics");
    REQUIRE(std::string(impactEffectLayerName(ImpactEffectLayer::Screen))   == "Screen");
}

TEST_CASE("ImpactResponse names", "[Editor][S111]") {
    REQUIRE(std::string(impactResponseName(ImpactResponse::None))      == "None");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Ricochet))  == "Ricochet");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Penetrate)) == "Penetrate");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Shatter))   == "Shatter");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Splash))    == "Splash");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Burn))      == "Burn");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Crumble))   == "Crumble");
    REQUIRE(std::string(impactResponseName(ImpactResponse::Bounce))    == "Bounce");
}

TEST_CASE("ImpactEffect defaults", "[Editor][S111]") {
    ImpactEffect fx(1, "bullet_hit_concrete", ImpactSurface::Concrete);
    REQUIRE(fx.id()                 == 1u);
    REQUIRE(fx.name()               == "bullet_hit_concrete");
    REQUIRE(fx.surface()            == ImpactSurface::Concrete);
    REQUIRE(fx.response()           == ImpactResponse::None);
    REQUIRE(fx.isDecalEnabled());
    REQUIRE(fx.isParticlesEnabled());
    REQUIRE(fx.isAudioEnabled());
    REQUIRE(fx.intensity()          == 1.0f);
}

TEST_CASE("ImpactEffect mutation", "[Editor][S111]") {
    ImpactEffect fx(2, "blade_hit_glass", ImpactSurface::Glass);
    fx.setResponse(ImpactResponse::Shatter);
    fx.setDecalEnabled(false);
    fx.setParticlesEnabled(true);
    fx.setAudioEnabled(true);
    fx.setIntensity(2.5f);
    REQUIRE(fx.response()           == ImpactResponse::Shatter);
    REQUIRE(!fx.isDecalEnabled());
    REQUIRE(fx.isParticlesEnabled());
    REQUIRE(fx.isAudioEnabled());
    REQUIRE(fx.intensity()          == 2.5f);
}

TEST_CASE("ImpactEffectEditor defaults", "[Editor][S111]") {
    ImpactEffectEditor ed;
    REQUIRE(!ed.isPreviewEnabled());
    REQUIRE(!ed.isShowDecalBounds());
    REQUIRE(ed.impactForce()   == 1.0f);
    REQUIRE(ed.effectCount()   == 0u);
}

TEST_CASE("ImpactEffectEditor add/remove effects", "[Editor][S111]") {
    ImpactEffectEditor ed;
    REQUIRE(ed.addEffect(ImpactEffect(1, "concrete_hit", ImpactSurface::Concrete)));
    REQUIRE(ed.addEffect(ImpactEffect(2, "metal_hit",    ImpactSurface::Metal)));
    REQUIRE(ed.addEffect(ImpactEffect(3, "water_splash", ImpactSurface::Water)));
    REQUIRE(!ed.addEffect(ImpactEffect(1, "concrete_hit",ImpactSurface::Concrete)));
    REQUIRE(ed.effectCount() == 3u);
    REQUIRE(ed.removeEffect(2));
    REQUIRE(ed.effectCount() == 2u);
    REQUIRE(!ed.removeEffect(99));
}

TEST_CASE("ImpactEffectEditor counts and find", "[Editor][S111]") {
    ImpactEffectEditor ed;
    ImpactEffect e1(1, "bullet_concrete",  ImpactSurface::Concrete);
    ImpactEffect e2(2, "blade_glass",      ImpactSurface::Glass);    e2.setResponse(ImpactResponse::Shatter);
    ImpactEffect e3(3, "bullet_metal",     ImpactSurface::Metal);    e3.setResponse(ImpactResponse::Ricochet);
    ImpactEffect e4(4, "arrow_concrete",   ImpactSurface::Concrete); e4.setResponse(ImpactResponse::Penetrate);
    ed.addEffect(e1); ed.addEffect(e2); ed.addEffect(e3); ed.addEffect(e4);
    REQUIRE(ed.countBySurface(ImpactSurface::Concrete)         == 2u);
    REQUIRE(ed.countBySurface(ImpactSurface::Glass)            == 1u);
    REQUIRE(ed.countBySurface(ImpactSurface::Wood)             == 0u);
    REQUIRE(ed.countByResponse(ImpactResponse::None)           == 1u);
    REQUIRE(ed.countByResponse(ImpactResponse::Ricochet)       == 1u);
    REQUIRE(ed.countByResponse(ImpactResponse::Shatter)        == 1u);
    auto* found = ed.findEffect(2);
    REQUIRE(found != nullptr);
    REQUIRE(found->surface() == ImpactSurface::Glass);
    REQUIRE(ed.findEffect(99) == nullptr);
}

TEST_CASE("ImpactEffectEditor mutation", "[Editor][S111]") {
    ImpactEffectEditor ed;
    ed.setPreviewEnabled(true);
    ed.setShowDecalBounds(true);
    ed.setImpactForce(3.0f);
    REQUIRE(ed.isPreviewEnabled());
    REQUIRE(ed.isShowDecalBounds());
    REQUIRE(ed.impactForce() == 3.0f);
}
