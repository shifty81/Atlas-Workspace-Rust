// S102 editor tests: WaterSimEditor, FluidSimEditor, ParticleSystemEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ParticleSystemEditor.h"
#include "NF/Editor/FluidSimEditor.h"
#include "NF/Editor/WaterSimEditor.h"

using namespace NF;

// ── WaterSimEditor ───────────────────────────────────────────────────────────

TEST_CASE("WaterSimMethod names", "[Editor][S102]") {
    REQUIRE(std::string(waterSimMethodName(WaterSimMethod::FFT))         == "FFT");
    REQUIRE(std::string(waterSimMethodName(WaterSimMethod::Gerstner))    == "Gerstner");
    REQUIRE(std::string(waterSimMethodName(WaterSimMethod::SPH))         == "SPH");
    REQUIRE(std::string(waterSimMethodName(WaterSimMethod::HeightField)) == "HeightField");
    REQUIRE(std::string(waterSimMethodName(WaterSimMethod::Hybrid))      == "Hybrid");
}

TEST_CASE("WaterSimBodyType names", "[Editor][S102]") {
    REQUIRE(std::string(waterSimBodyTypeName(WaterSimBodyType::Ocean))     == "Ocean");
    REQUIRE(std::string(waterSimBodyTypeName(WaterSimBodyType::Lake))      == "Lake");
    REQUIRE(std::string(waterSimBodyTypeName(WaterSimBodyType::River))     == "River");
    REQUIRE(std::string(waterSimBodyTypeName(WaterSimBodyType::Pool))      == "Pool");
    REQUIRE(std::string(waterSimBodyTypeName(WaterSimBodyType::Waterfall)) == "Waterfall");
}

TEST_CASE("WaterFoamMode names", "[Editor][S102]") {
    REQUIRE(std::string(waterFoamModeName(WaterFoamMode::None))       == "None");
    REQUIRE(std::string(waterFoamModeName(WaterFoamMode::Simple))     == "Simple");
    REQUIRE(std::string(waterFoamModeName(WaterFoamMode::Detailed))   == "Detailed");
    REQUIRE(std::string(waterFoamModeName(WaterFoamMode::Volumetric)) == "Volumetric");
}

TEST_CASE("WaterBodyConfig defaults", "[Editor][S102]") {
    WaterBodyConfig cfg("main_ocean", WaterSimBodyType::Ocean);
    REQUIRE(cfg.name()         == "main_ocean");
    REQUIRE(cfg.bodyType()     == WaterSimBodyType::Ocean);
    REQUIRE(cfg.method()       == WaterSimMethod::Gerstner);
    REQUIRE(cfg.foamMode()     == WaterFoamMode::Simple);
    REQUIRE(cfg.amplitude()    == 1.0f);
    REQUIRE(cfg.windSpeed()    == 5.0f);
    REQUIRE(cfg.turbulence()   == 0.2f);
    REQUIRE(cfg.tessellation() == 64u);
    REQUIRE(cfg.isEnabled());
    REQUIRE(!cfg.isInteractive());
}

TEST_CASE("WaterBodyConfig mutation", "[Editor][S102]") {
    WaterBodyConfig cfg("river1", WaterSimBodyType::River);
    cfg.setMethod(WaterSimMethod::FFT);
    cfg.setFoamMode(WaterFoamMode::Volumetric);
    cfg.setAmplitude(2.5f);
    cfg.setWindSpeed(10.0f);
    cfg.setTurbulence(0.5f);
    cfg.setTessellation(128);
    cfg.setInteractive(true);
    cfg.setEnabled(false);
    REQUIRE(cfg.method()       == WaterSimMethod::FFT);
    REQUIRE(cfg.foamMode()     == WaterFoamMode::Volumetric);
    REQUIRE(cfg.amplitude()    == 2.5f);
    REQUIRE(cfg.windSpeed()    == 10.0f);
    REQUIRE(cfg.turbulence()   == 0.5f);
    REQUIRE(cfg.tessellation() == 128u);
    REQUIRE(cfg.isInteractive());
    REQUIRE(!cfg.isEnabled());
}

TEST_CASE("WaterSimEditor add/remove", "[Editor][S102]") {
    WaterSimEditor ed;
    REQUIRE(ed.addBody(WaterBodyConfig("ocean1", WaterSimBodyType::Ocean)));
    REQUIRE(ed.addBody(WaterBodyConfig("lake1",  WaterSimBodyType::Lake)));
    REQUIRE(!ed.addBody(WaterBodyConfig("ocean1", WaterSimBodyType::Ocean)));
    REQUIRE(ed.bodyCount() == 2u);
    REQUIRE(ed.removeBody("ocean1"));
    REQUIRE(ed.bodyCount() == 1u);
    REQUIRE(!ed.removeBody("ocean1"));
}

TEST_CASE("WaterSimEditor find and counts", "[Editor][S102]") {
    WaterSimEditor ed;
    WaterBodyConfig b1("o1", WaterSimBodyType::Ocean); b1.setMethod(WaterSimMethod::FFT); b1.setInteractive(true);
    WaterBodyConfig b2("l1", WaterSimBodyType::Lake);  b2.setMethod(WaterSimMethod::Gerstner);
    WaterBodyConfig b3("o2", WaterSimBodyType::Ocean); b3.setMethod(WaterSimMethod::FFT); b3.setEnabled(false);
    ed.addBody(b1); ed.addBody(b2); ed.addBody(b3);
    REQUIRE(ed.bodyCount()                                == 3u);
    REQUIRE(ed.enabledCount()                             == 2u);
    REQUIRE(ed.interactiveCount()                         == 1u);
    REQUIRE(ed.countByType(WaterSimBodyType::Ocean)          == 2u);
    REQUIRE(ed.countByMethod(WaterSimMethod::FFT)         == 2u);
    auto* found = ed.findBody("l1");
    REQUIRE(found != nullptr);
    REQUIRE(found->bodyType() == WaterSimBodyType::Lake);
    REQUIRE(ed.findBody("missing") == nullptr);
}

// ── FluidSimEditor ───────────────────────────────────────────────────────────

TEST_CASE("FluidSimType names", "[Editor][S102]") {
    REQUIRE(std::string(fluidSimTypeName(FluidSimType::SPH))      == "SPH");
    REQUIRE(std::string(fluidSimTypeName(FluidSimType::FLIP))     == "FLIP");
    REQUIRE(std::string(fluidSimTypeName(FluidSimType::APIC))     == "APIC");
    REQUIRE(std::string(fluidSimTypeName(FluidSimType::MPM))      == "MPM");
    REQUIRE(std::string(fluidSimTypeName(FluidSimType::Eulerian)) == "Eulerian");
}

TEST_CASE("FluidRenderMode names", "[Editor][S102]") {
    REQUIRE(std::string(fluidRenderModeName(FluidRenderMode::Particles))   == "Particles");
    REQUIRE(std::string(fluidRenderModeName(FluidRenderMode::SurfaceMesh)) == "SurfaceMesh");
    REQUIRE(std::string(fluidRenderModeName(FluidRenderMode::Volume))      == "Volume");
    REQUIRE(std::string(fluidRenderModeName(FluidRenderMode::Hybrid))      == "Hybrid");
}

TEST_CASE("FluidBoundaryMode names", "[Editor][S102]") {
    REQUIRE(std::string(fluidBoundaryModeName(FluidBoundaryMode::Box))      == "Box");
    REQUIRE(std::string(fluidBoundaryModeName(FluidBoundaryMode::Sphere))   == "Sphere");
    REQUIRE(std::string(fluidBoundaryModeName(FluidBoundaryMode::Mesh))     == "Mesh");
    REQUIRE(std::string(fluidBoundaryModeName(FluidBoundaryMode::Infinite)) == "Infinite");
}

TEST_CASE("FluidSimConfig defaults", "[Editor][S102]") {
    FluidSimConfig cfg("waterfall_sim", FluidSimType::FLIP);
    REQUIRE(cfg.name()           == "waterfall_sim");
    REQUIRE(cfg.simType()        == FluidSimType::FLIP);
    REQUIRE(cfg.renderMode()     == FluidRenderMode::SurfaceMesh);
    REQUIRE(cfg.boundaryMode()   == FluidBoundaryMode::Box);
    REQUIRE(cfg.viscosity()      == 0.01f);
    REQUIRE(cfg.surfaceTension() == 0.0f);
    REQUIRE(cfg.maxParticles()   == 100000u);
    REQUIRE(cfg.isEnabled());
    REQUIRE(!cfg.isGpuAccelerated());
}

TEST_CASE("FluidSimConfig mutation", "[Editor][S102]") {
    FluidSimConfig cfg("lava", FluidSimType::SPH);
    cfg.setRenderMode(FluidRenderMode::Volume);
    cfg.setBoundaryMode(FluidBoundaryMode::Mesh);
    cfg.setParticleRadius(0.1f);
    cfg.setViscosity(0.5f);
    cfg.setSurfaceTension(0.2f);
    cfg.setMaxParticles(50000);
    cfg.setGpuAccelerated(true);
    cfg.setEnabled(false);
    REQUIRE(cfg.renderMode()       == FluidRenderMode::Volume);
    REQUIRE(cfg.boundaryMode()     == FluidBoundaryMode::Mesh);
    REQUIRE(cfg.particleRadius()   == 0.1f);
    REQUIRE(cfg.viscosity()        == 0.5f);
    REQUIRE(cfg.surfaceTension()   == 0.2f);
    REQUIRE(cfg.maxParticles()     == 50000u);
    REQUIRE(cfg.isGpuAccelerated());
    REQUIRE(!cfg.isEnabled());
}

TEST_CASE("FluidSimEditor add/remove", "[Editor][S102]") {
    FluidSimEditor ed;
    REQUIRE(ed.addSim(FluidSimConfig("sim1", FluidSimType::FLIP)));
    REQUIRE(ed.addSim(FluidSimConfig("sim2", FluidSimType::SPH)));
    REQUIRE(!ed.addSim(FluidSimConfig("sim1", FluidSimType::APIC)));
    REQUIRE(ed.simCount() == 2u);
    REQUIRE(ed.removeSim("sim1"));
    REQUIRE(ed.simCount() == 1u);
    REQUIRE(!ed.removeSim("sim1"));
}

TEST_CASE("FluidSimEditor counts", "[Editor][S102]") {
    FluidSimEditor ed;
    FluidSimConfig s1("a", FluidSimType::FLIP); s1.setGpuAccelerated(true);
    FluidSimConfig s2("b", FluidSimType::SPH);  s2.setEnabled(false);
    FluidSimConfig s3("c", FluidSimType::FLIP); s3.setRenderMode(FluidRenderMode::Particles);
    ed.addSim(s1); ed.addSim(s2); ed.addSim(s3);
    REQUIRE(ed.simCount()                                     == 3u);
    REQUIRE(ed.enabledCount()                                 == 2u);
    REQUIRE(ed.gpuAccelCount()                                == 1u);
    REQUIRE(ed.countByType(FluidSimType::FLIP)                == 2u);
    REQUIRE(ed.countByRenderMode(FluidRenderMode::Particles)  == 1u);
}

// ── ParticleSystemEditor ─────────────────────────────────────────────────────

TEST_CASE("PsEmitterShape names", "[Editor][S102]") {
    REQUIRE(std::string(psEmitterShapeName(PsEmitterShape::Point))    == "Point");
    REQUIRE(std::string(psEmitterShapeName(PsEmitterShape::Sphere))   == "Sphere");
    REQUIRE(std::string(psEmitterShapeName(PsEmitterShape::Box))      == "Box");
    REQUIRE(std::string(psEmitterShapeName(PsEmitterShape::Cone))     == "Cone");
    REQUIRE(std::string(psEmitterShapeName(PsEmitterShape::Cylinder)) == "Cylinder");
    REQUIRE(std::string(psEmitterShapeName(PsEmitterShape::Edge))     == "Edge");
}

TEST_CASE("ParticleRenderMode names", "[Editor][S102]") {
    REQUIRE(std::string(particleRenderModeName(ParticleRenderMode::Billboard)) == "Billboard");
    REQUIRE(std::string(particleRenderModeName(ParticleRenderMode::Mesh))      == "Mesh");
    REQUIRE(std::string(particleRenderModeName(ParticleRenderMode::Trail))     == "Trail");
    REQUIRE(std::string(particleRenderModeName(ParticleRenderMode::Ribbon))    == "Ribbon");
    REQUIRE(std::string(particleRenderModeName(ParticleRenderMode::Beam))      == "Beam");
}

TEST_CASE("ParticleSimSpace names", "[Editor][S102]") {
    REQUIRE(std::string(particleSimSpaceName(ParticleSimSpace::Local)) == "Local");
    REQUIRE(std::string(particleSimSpaceName(ParticleSimSpace::World)) == "World");
}

TEST_CASE("ParticleSystemConfig defaults", "[Editor][S102]") {
    ParticleSystemConfig cfg("fire_fx");
    REQUIRE(cfg.name()            == "fire_fx");
    REQUIRE(cfg.emitterShape()    == PsEmitterShape::Cone);
    REQUIRE(cfg.renderMode()      == ParticleRenderMode::Billboard);
    REQUIRE(cfg.simSpace()        == ParticleSimSpace::World);
    REQUIRE(cfg.maxParticles()    == 1000u);
    REQUIRE(cfg.emitRate()        == 10.0f);
    REQUIRE(cfg.lifetime()        == 5.0f);
    REQUIRE(cfg.startSpeed()      == 1.0f);
    REQUIRE(cfg.gravityModifier() == 1.0f);
    REQUIRE(cfg.isLooping());
    REQUIRE(cfg.isEnabled());
}

TEST_CASE("ParticleSystemConfig mutation", "[Editor][S102]") {
    ParticleSystemConfig cfg("smoke");
    cfg.setEmitterShape(PsEmitterShape::Box);
    cfg.setRenderMode(ParticleRenderMode::Trail);
    cfg.setSimSpace(ParticleSimSpace::Local);
    cfg.setMaxParticles(500);
    cfg.setEmitRate(50.0f);
    cfg.setLifetime(3.0f);
    cfg.setStartSpeed(2.0f);
    cfg.setGravityModifier(0.0f);
    cfg.setLooping(false);
    cfg.setEnabled(false);
    REQUIRE(cfg.emitterShape()    == PsEmitterShape::Box);
    REQUIRE(cfg.renderMode()      == ParticleRenderMode::Trail);
    REQUIRE(cfg.simSpace()        == ParticleSimSpace::Local);
    REQUIRE(cfg.maxParticles()    == 500u);
    REQUIRE(cfg.emitRate()        == 50.0f);
    REQUIRE(cfg.lifetime()        == 3.0f);
    REQUIRE(cfg.startSpeed()      == 2.0f);
    REQUIRE(cfg.gravityModifier() == 0.0f);
    REQUIRE(!cfg.isLooping());
    REQUIRE(!cfg.isEnabled());
}

TEST_CASE("ParticleSystemEditor add/remove", "[Editor][S102]") {
    ParticleSystemEditor ed;
    REQUIRE(ed.addSystem(ParticleSystemConfig("fx1")));
    REQUIRE(ed.addSystem(ParticleSystemConfig("fx2")));
    REQUIRE(!ed.addSystem(ParticleSystemConfig("fx1")));
    REQUIRE(ed.systemCount() == 2u);
    REQUIRE(ed.removeSystem("fx1"));
    REQUIRE(ed.systemCount() == 1u);
    REQUIRE(!ed.removeSystem("fx1"));
}

TEST_CASE("ParticleSystemEditor counts", "[Editor][S102]") {
    ParticleSystemEditor ed;
    ParticleSystemConfig p1("a"); p1.setEmitterShape(PsEmitterShape::Cone);
    p1.setRenderMode(ParticleRenderMode::Billboard);
    ParticleSystemConfig p2("b"); p2.setEmitterShape(PsEmitterShape::Box);
    p2.setRenderMode(ParticleRenderMode::Mesh); p2.setLooping(false); p2.setEnabled(false);
    ParticleSystemConfig p3("c"); p3.setEmitterShape(PsEmitterShape::Cone);
    p3.setRenderMode(ParticleRenderMode::Trail);
    ed.addSystem(p1); ed.addSystem(p2); ed.addSystem(p3);
    REQUIRE(ed.systemCount()                                         == 3u);
    REQUIRE(ed.enabledCount()                                        == 2u);
    REQUIRE(ed.loopingCount()                                        == 2u);
    REQUIRE(ed.countByShape(PsEmitterShape::Cone)              == 2u);
    REQUIRE(ed.countByRenderMode(ParticleRenderMode::Billboard)      == 1u);
}
