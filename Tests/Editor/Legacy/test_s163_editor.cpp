// S163 editor tests: LightmapEditorV1, ReflectionProbeEditorV1, ShadowCasterEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ShadowCasterEditorV1.h"
#include "NF/Editor/ReflectionProbeEditorV1.h"
#include "NF/Editor/LightmapEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── LightmapEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Lmv1BakeJob validity", "[Editor][S163]") {
    Lmv1BakeJob j;
    REQUIRE(!j.isValid());
    j.id = 1; j.meshName = "Terrain"; j.textureSizePx = 512; j.texelDensity = 1.f;
    REQUIRE(j.isValid());
}

TEST_CASE("Lmv1BakeJob zero textureSizePx invalid", "[Editor][S163]") {
    Lmv1BakeJob j; j.id = 1; j.meshName = "M"; j.textureSizePx = 0; j.texelDensity = 1.f;
    REQUIRE(!j.isValid());
}

TEST_CASE("Lmv1BakeJob zero texelDensity invalid", "[Editor][S163]") {
    Lmv1BakeJob j; j.id = 1; j.meshName = "M"; j.textureSizePx = 512; j.texelDensity = 0.f;
    REQUIRE(!j.isValid());
}

TEST_CASE("Lmv1BakeJob isDone and isActive", "[Editor][S163]") {
    Lmv1BakeJob j; j.id = 1; j.meshName = "M"; j.textureSizePx = 256; j.texelDensity = 1.f;
    j.status = Lmv1BakeStatus::Done;
    REQUIRE(j.isDone());
    REQUIRE(!j.isActive());
    j.status = Lmv1BakeStatus::Baking;
    REQUIRE(j.isActive());
    REQUIRE(!j.isDone());
}

TEST_CASE("LightmapEditorV1 addJob and jobCount", "[Editor][S163]") {
    LightmapEditorV1 lm;
    REQUIRE(lm.jobCount() == 0);
    Lmv1BakeJob j; j.id = 1; j.meshName = "House"; j.textureSizePx = 512; j.texelDensity = 1.f;
    REQUIRE(lm.addJob(j));
    REQUIRE(lm.jobCount() == 1);
}

TEST_CASE("LightmapEditorV1 addJob invalid fails", "[Editor][S163]") {
    LightmapEditorV1 lm;
    REQUIRE(!lm.addJob(Lmv1BakeJob{}));
}

TEST_CASE("LightmapEditorV1 addJob duplicate id fails", "[Editor][S163]") {
    LightmapEditorV1 lm;
    Lmv1BakeJob j; j.id = 1; j.meshName = "A"; j.textureSizePx = 256; j.texelDensity = 1.f;
    lm.addJob(j);
    REQUIRE(!lm.addJob(j));
}

TEST_CASE("LightmapEditorV1 removeJob", "[Editor][S163]") {
    LightmapEditorV1 lm;
    Lmv1BakeJob j; j.id = 2; j.meshName = "B"; j.textureSizePx = 128; j.texelDensity = 1.f;
    lm.addJob(j);
    REQUIRE(lm.removeJob(2));
    REQUIRE(lm.jobCount() == 0);
    REQUIRE(!lm.removeJob(2));
}

TEST_CASE("LightmapEditorV1 findJob", "[Editor][S163]") {
    LightmapEditorV1 lm;
    Lmv1BakeJob j; j.id = 3; j.meshName = "C"; j.textureSizePx = 512; j.texelDensity = 2.f;
    lm.addJob(j);
    REQUIRE(lm.findJob(3) != nullptr);
    REQUIRE(lm.findJob(99) == nullptr);
}

TEST_CASE("LightmapEditorV1 updateStatus", "[Editor][S163]") {
    LightmapEditorV1 lm;
    Lmv1BakeJob j; j.id = 4; j.meshName = "D"; j.textureSizePx = 512; j.texelDensity = 1.f;
    lm.addJob(j);
    REQUIRE(lm.updateStatus(4, Lmv1BakeStatus::Done, 1.f));
    REQUIRE(lm.findJob(4)->isDone());
}

TEST_CASE("LightmapEditorV1 cancelJob", "[Editor][S163]") {
    LightmapEditorV1 lm;
    Lmv1BakeJob j; j.id = 5; j.meshName = "E"; j.textureSizePx = 512; j.texelDensity = 1.f;
    j.status = Lmv1BakeStatus::Baking;
    lm.addJob(j);
    REQUIRE(lm.cancelJob(5));
    REQUIRE(lm.findJob(5)->isCancelled());
}

TEST_CASE("LightmapEditorV1 doneCount and activeCount", "[Editor][S163]") {
    LightmapEditorV1 lm;
    auto makeJob = [](uint64_t id, const char* name) {
        Lmv1BakeJob j; j.id = id; j.meshName = name; j.textureSizePx = 512; j.texelDensity = 1.f;
        return j;
    };
    auto j1 = makeJob(1, "A"); j1.status = Lmv1BakeStatus::Done;
    auto j2 = makeJob(2, "B"); j2.status = Lmv1BakeStatus::Baking;
    auto j3 = makeJob(3, "C"); j3.status = Lmv1BakeStatus::Denoising;
    lm.addJob(j1); lm.addJob(j2); lm.addJob(j3);
    REQUIRE(lm.doneCount()   == 1);
    REQUIRE(lm.activeCount() == 2);
}

TEST_CASE("LightmapEditorV1 countByQuality", "[Editor][S163]") {
    LightmapEditorV1 lm;
    Lmv1BakeJob j1; j1.id = 1; j1.meshName = "A"; j1.textureSizePx = 512; j1.texelDensity = 1.f; j1.quality = Lmv1BakeQuality::High;
    Lmv1BakeJob j2; j2.id = 2; j2.meshName = "B"; j2.textureSizePx = 512; j2.texelDensity = 1.f; j2.quality = Lmv1BakeQuality::Medium;
    lm.addJob(j1); lm.addJob(j2);
    REQUIRE(lm.countByQuality(Lmv1BakeQuality::High) == 1);
    REQUIRE(lm.countByQuality(Lmv1BakeQuality::Medium) == 1);
}

TEST_CASE("LightmapEditorV1 onChange callback", "[Editor][S163]") {
    LightmapEditorV1 lm;
    uint64_t notified = 0;
    lm.setOnChange([&](uint64_t id) { notified = id; });
    Lmv1BakeJob j; j.id = 7; j.meshName = "G"; j.textureSizePx = 512; j.texelDensity = 1.f;
    lm.addJob(j);
    lm.updateStatus(7, Lmv1BakeStatus::Done);
    REQUIRE(notified == 7);
}

TEST_CASE("lmv1BakeQualityName covers all values", "[Editor][S163]") {
    REQUIRE(std::string(lmv1BakeQualityName(Lmv1BakeQuality::Draft))      == "Draft");
    REQUIRE(std::string(lmv1BakeQualityName(Lmv1BakeQuality::Production)) == "Production");
}

// ── ReflectionProbeEditorV1 ──────────────────────────────────────────────────

TEST_CASE("Rpv1Probe validity", "[Editor][S163]") {
    Rpv1Probe p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "MainProbe"; p.resolution = 256; p.intensity = 1.f;
    REQUIRE(p.isValid());
}

TEST_CASE("Rpv1Probe zero resolution invalid", "[Editor][S163]") {
    Rpv1Probe p; p.id = 1; p.name = "P"; p.resolution = 0; p.intensity = 1.f;
    REQUIRE(!p.isValid());
}

TEST_CASE("Rpv1Probe isDone and isStale", "[Editor][S163]") {
    Rpv1Probe p; p.id = 1; p.name = "P"; p.resolution = 128; p.intensity = 1.f;
    p.status = Rpv1BakeStatus::Done;
    REQUIRE(p.isDone());
    REQUIRE(!p.isStale());
    p.status = Rpv1BakeStatus::Stale;
    REQUIRE(p.isStale());
}

TEST_CASE("ReflectionProbeEditorV1 addProbe and probeCount", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    REQUIRE(rp.probeCount() == 0);
    Rpv1Probe p; p.id = 1; p.name = "A"; p.resolution = 256; p.intensity = 1.f;
    REQUIRE(rp.addProbe(p));
    REQUIRE(rp.probeCount() == 1);
}

TEST_CASE("ReflectionProbeEditorV1 addProbe invalid fails", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    REQUIRE(!rp.addProbe(Rpv1Probe{}));
}

TEST_CASE("ReflectionProbeEditorV1 addProbe duplicate fails", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    Rpv1Probe p; p.id = 1; p.name = "A"; p.resolution = 256; p.intensity = 1.f;
    rp.addProbe(p);
    REQUIRE(!rp.addProbe(p));
}

TEST_CASE("ReflectionProbeEditorV1 removeProbe", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    Rpv1Probe p; p.id = 2; p.name = "B"; p.resolution = 256; p.intensity = 1.f;
    rp.addProbe(p);
    REQUIRE(rp.removeProbe(2));
    REQUIRE(rp.probeCount() == 0);
}

TEST_CASE("ReflectionProbeEditorV1 setActive and activeId", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    Rpv1Probe p; p.id = 3; p.name = "C"; p.resolution = 256; p.intensity = 1.f;
    rp.addProbe(p);
    REQUIRE(rp.setActive(3));
    REQUIRE(rp.activeId() == 3);
    REQUIRE(!rp.setActive(99));
}

TEST_CASE("ReflectionProbeEditorV1 activeId clears on remove", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    Rpv1Probe p; p.id = 4; p.name = "D"; p.resolution = 256; p.intensity = 1.f;
    rp.addProbe(p); rp.setActive(4); rp.removeProbe(4);
    REQUIRE(rp.activeId() == 0);
}

TEST_CASE("ReflectionProbeEditorV1 updateStatus doneCount staleCount", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    Rpv1Probe p1; p1.id = 1; p1.name = "A"; p1.resolution = 256; p1.intensity = 1.f;
    Rpv1Probe p2; p2.id = 2; p2.name = "B"; p2.resolution = 256; p2.intensity = 1.f;
    rp.addProbe(p1); rp.addProbe(p2);
    rp.updateStatus(1, Rpv1BakeStatus::Done);
    rp.updateStatus(2, Rpv1BakeStatus::Stale);
    REQUIRE(rp.doneCount()  == 1);
    REQUIRE(rp.staleCount() == 1);
}

TEST_CASE("ReflectionProbeEditorV1 countByType", "[Editor][S163]") {
    ReflectionProbeEditorV1 rp;
    Rpv1Probe p1; p1.id = 1; p1.name = "A"; p1.resolution = 256; p1.intensity = 1.f; p1.type = Rpv1ProbeType::Box;
    Rpv1Probe p2; p2.id = 2; p2.name = "B"; p2.resolution = 256; p2.intensity = 1.f; p2.type = Rpv1ProbeType::Sphere;
    rp.addProbe(p1); rp.addProbe(p2);
    REQUIRE(rp.countByType(Rpv1ProbeType::Box)    == 1);
    REQUIRE(rp.countByType(Rpv1ProbeType::Sphere) == 1);
}

TEST_CASE("rpv1ProbeTypeName covers all values", "[Editor][S163]") {
    REQUIRE(std::string(rpv1ProbeTypeName(Rpv1ProbeType::Box))         == "Box");
    REQUIRE(std::string(rpv1ProbeTypeName(Rpv1ProbeType::ScreenSpace)) == "ScreenSpace");
}

// ── ShadowCasterEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Scv1CasterEntry validity", "[Editor][S163]") {
    Scv1CasterEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.meshName = "Wall"; e.resolution = 1024;
    REQUIRE(e.isValid());
}

TEST_CASE("Scv1CasterEntry zero resolution invalid", "[Editor][S163]") {
    Scv1CasterEntry e; e.id = 1; e.meshName = "W"; e.resolution = 0;
    REQUIRE(!e.isValid());
}

TEST_CASE("Scv1CasterEntry castsShadow", "[Editor][S163]") {
    Scv1CasterEntry e; e.id = 1; e.meshName = "W"; e.resolution = 512;
    e.shadowMode = Scv1ShadowMode::Off;
    REQUIRE(!e.castsShadow());
    e.shadowMode = Scv1ShadowMode::TwoSided;
    REQUIRE(e.castsShadow());
}

TEST_CASE("ShadowCasterEditorV1 addEntry and entryCount", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    REQUIRE(sc.entryCount() == 0);
    Scv1CasterEntry e; e.id = 1; e.meshName = "A"; e.resolution = 1024;
    REQUIRE(sc.addEntry(e));
    REQUIRE(sc.entryCount() == 1);
}

TEST_CASE("ShadowCasterEditorV1 addEntry invalid fails", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    REQUIRE(!sc.addEntry(Scv1CasterEntry{}));
}

TEST_CASE("ShadowCasterEditorV1 addEntry duplicate fails", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e; e.id = 1; e.meshName = "A"; e.resolution = 1024;
    sc.addEntry(e);
    REQUIRE(!sc.addEntry(e));
}

TEST_CASE("ShadowCasterEditorV1 removeEntry", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e; e.id = 2; e.meshName = "B"; e.resolution = 512;
    sc.addEntry(e);
    REQUIRE(sc.removeEntry(2));
    REQUIRE(sc.entryCount() == 0);
    REQUIRE(!sc.removeEntry(2));
}

TEST_CASE("ShadowCasterEditorV1 setShadowMode", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e; e.id = 3; e.meshName = "C"; e.resolution = 1024;
    sc.addEntry(e);
    REQUIRE(sc.setShadowMode(3, Scv1ShadowMode::Off));
    REQUIRE(!sc.findEntry(3)->castsShadow());
}

TEST_CASE("ShadowCasterEditorV1 setCascadeMode", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e; e.id = 4; e.meshName = "D"; e.resolution = 1024;
    sc.addEntry(e);
    REQUIRE(sc.setCascadeMode(4, Scv1CascadeMode::TwoCascade));
    REQUIRE(sc.findEntry(4)->cascadeMode == Scv1CascadeMode::TwoCascade);
}

TEST_CASE("ShadowCasterEditorV1 setFilterMode", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e; e.id = 5; e.meshName = "E"; e.resolution = 1024;
    sc.addEntry(e);
    REQUIRE(sc.setFilterMode(5, Scv1FilterMode::PCSS));
    REQUIRE(sc.findEntry(5)->filterMode == Scv1FilterMode::PCSS);
}

TEST_CASE("ShadowCasterEditorV1 castingCount", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e1; e1.id = 1; e1.meshName = "A"; e1.resolution = 512; e1.shadowMode = Scv1ShadowMode::TwoSided;
    Scv1CasterEntry e2; e2.id = 2; e2.meshName = "B"; e2.resolution = 512; e2.shadowMode = Scv1ShadowMode::Off;
    sc.addEntry(e1); sc.addEntry(e2);
    REQUIRE(sc.castingCount() == 1);
}

TEST_CASE("ShadowCasterEditorV1 countByMode and countByFilter", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    Scv1CasterEntry e1; e1.id = 1; e1.meshName = "A"; e1.resolution = 512;
    e1.shadowMode = Scv1ShadowMode::TwoSided; e1.filterMode = Scv1FilterMode::PCF;
    Scv1CasterEntry e2; e2.id = 2; e2.meshName = "B"; e2.resolution = 512;
    e2.shadowMode = Scv1ShadowMode::OneSided; e2.filterMode = Scv1FilterMode::Hard;
    sc.addEntry(e1); sc.addEntry(e2);
    REQUIRE(sc.countByMode(Scv1ShadowMode::TwoSided) == 1);
    REQUIRE(sc.countByFilter(Scv1FilterMode::PCF)    == 1);
}

TEST_CASE("scv1FilterModeName covers all values", "[Editor][S163]") {
    REQUIRE(std::string(scv1FilterModeName(Scv1FilterMode::Hard)) == "Hard");
    REQUIRE(std::string(scv1FilterModeName(Scv1FilterMode::VSM))  == "VSM");
}

TEST_CASE("ShadowCasterEditorV1 onChange callback", "[Editor][S163]") {
    ShadowCasterEditorV1 sc;
    uint64_t notified = 0;
    sc.setOnChange([&](uint64_t id) { notified = id; });
    Scv1CasterEntry e; e.id = 9; e.meshName = "I"; e.resolution = 512;
    sc.addEntry(e);
    sc.setShadowMode(9, Scv1ShadowMode::Off);
    REQUIRE(notified == 9);
}
