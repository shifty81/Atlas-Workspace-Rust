// Phase E — Shared PCG Preview Pipeline
//
// E.1 — PCGRuleSet: typed rule container
//   - identity (id, domain, name, version)
//   - addRule, setValue, removeRule, findRule, getValue, hasRule
//   - resetToDefaults, resetRule, ruleCount, rulesInCategory
//   - dirty tracking, kMaxRules capacity
//
// E.1 — PCGDeterministicSeedContext: reproducible seed management
//   - universeSeed get/set
//   - seedForDomain (deterministic, repeatable)
//   - childContext (derived seed different from parent)
//   - pinDomainSeed / hasPinnedSeed / clearPinnedSeeds
//   - registerDomain / hasDomain / registeredDomains
//
// E.1 — PCGGeneratorService: stateless generator
//   - generate() with empty ruleset → success, 0 placements
//   - generate() with density+count → correct placement count
//   - generate() same inputs → same output (deterministic)
//   - generate() different seeds → different placement positions
//   - validate() warns on missing recommended rules
//   - PCGPlacement fields populated
//
// E.2 — PCGPreviewService: editor-side wrapper
//   - bindRuleSet / clearRuleSet / hasRuleSet
//   - setSeed / currentSeed
//   - forceRegenerate() → hasResult=true
//   - autoRegenerate=true triggers regen on bindRuleSet
//   - autoRegenerate=false does NOT regen on bind
//   - stats updated after generation
//
// E.3 — Rule editing via PCGPreviewService
//   - setRuleValue triggers regen when autoRegenerate=true
//   - setRuleValue does not regen when autoRegenerate=false
//   - resetRules reverts to defaults + triggers regen
//   - change callback invoked after regen
//   - setDomainOverride used in generation
//
// E.4 — PCGPreviewService → NovaForgePreviewWorld population
//   - populatePreviewWorld creates entities from placements
//   - world entity count matches placement count
//   - populatePreviewWorld before regen returns 0
//   - entity mesh/material tags set from placement

#include <catch2/catch_test_macros.hpp>

#include "NovaForge/EditorAdapter/PCGRuleSet.h"
#include "NovaForge/EditorAdapter/PCGDeterministicSeedContext.h"
#include "NovaForge/EditorAdapter/PCGGeneratorService.h"
#include "NovaForge/EditorAdapter/PCGPreviewService.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"

#include <algorithm>
#include <string>

using namespace NovaForge;

// ── Helper to make a minimal valid rule set ───────────────────────────────

static PCGRuleSet makePCGRuleSet(const std::string& id = "test_ruleset",
                                  int count = 10, float density = 1.0f) {
    PCGRuleSet rs(id, "test_domain");
    PCGRule r;
    r.key          = "count";
    r.type         = PCGRuleValueType::Int;
    r.value        = std::to_string(count);
    r.defaultValue = std::to_string(count);
    rs.addRule(r);

    PCGRule rd;
    rd.key          = "density";
    rd.type         = PCGRuleValueType::Float;
    rd.value        = std::to_string(density);
    rd.defaultValue = std::to_string(density);
    rs.addRule(rd);

    PCGRule rp;
    rp.key          = "placementTag";
    rp.type         = PCGRuleValueType::Tag;
    rp.value        = "asteroid.small";
    rp.defaultValue = "asteroid.small";
    rs.addRule(rp);
    return rs;
}

// ═══════════════════════════════════════════════════════════════════════════
//  E.1 — PCGRuleSet
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("E.1 PCGRuleSet: default id is empty", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs;
    REQUIRE(rs.id().empty());
    REQUIRE(rs.ruleCount() == 0);
    REQUIRE_FALSE(rs.isDirty());
}

TEST_CASE("E.1 PCGRuleSet: constructor sets id and domain", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("ship_gen", "ship_domain");
    REQUIRE(rs.id()     == "ship_gen");
    REQUIRE(rs.domain() == "ship_domain");
}

TEST_CASE("E.1 PCGRuleSet: setters work", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs;
    rs.setId("my_id");
    rs.setDomain("planet");
    rs.setName("Planet Generator");
    rs.setVersion("2.0");
    REQUIRE(rs.id()      == "my_id");
    REQUIRE(rs.domain()  == "planet");
    REQUIRE(rs.name()    == "Planet Generator");
    REQUIRE(rs.version() == "2.0");
}

TEST_CASE("E.1 PCGRuleSet: addRule increases ruleCount and marks dirty", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    PCGRule r; r.key = "density"; r.value = "1.0";
    REQUIRE(rs.addRule(r));
    REQUIRE(rs.ruleCount() == 1);
    REQUIRE(rs.isDirty());
}

TEST_CASE("E.1 PCGRuleSet: addRule with empty key returns false", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    PCGRule r; r.key = ""; r.value = "1.0";
    REQUIRE_FALSE(rs.addRule(r));
    REQUIRE(rs.ruleCount() == 0);
}

TEST_CASE("E.1 PCGRuleSet: addRule with duplicate key returns false", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    PCGRule r; r.key = "density"; r.value = "1.0";
    REQUIRE(rs.addRule(r));
    PCGRule r2; r2.key = "density"; r2.value = "2.0";
    REQUIRE_FALSE(rs.addRule(r2));
    REQUIRE(rs.ruleCount() == 1);
}

TEST_CASE("E.1 PCGRuleSet: setValue updates value and marks dirty", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    rs.clearDirty();
    REQUIRE(rs.setValue("density", "0.5"));
    REQUIRE(rs.getValue("density") == "0.5");
    REQUIRE(rs.isDirty());
}

TEST_CASE("E.1 PCGRuleSet: setValue returns false for missing key", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    REQUIRE_FALSE(rs.setValue("ghost", "x"));
}

TEST_CASE("E.1 PCGRuleSet: setValue returns false for readOnly rule", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    PCGRule r; r.key = "seed"; r.value = "1"; r.readOnly = true;
    rs.addRule(r);
    REQUIRE_FALSE(rs.setValue("seed", "2"));
    REQUIRE(rs.getValue("seed") == "1");
}

TEST_CASE("E.1 PCGRuleSet: removeRule by key", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    rs.clearDirty();
    uint32_t before = rs.ruleCount();
    REQUIRE(rs.removeRule("density"));
    REQUIRE(rs.ruleCount() == before - 1);
    REQUIRE_FALSE(rs.hasRule("density"));
    REQUIRE(rs.isDirty());
}

TEST_CASE("E.1 PCGRuleSet: removeRule non-existent returns false", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    REQUIRE_FALSE(rs.removeRule("ghost"));
}

TEST_CASE("E.1 PCGRuleSet: findRule returns pointer", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    const auto* r = rs.findRule("count");
    REQUIRE(r != nullptr);
    REQUIRE(r->key == "count");
}

TEST_CASE("E.1 PCGRuleSet: findRule returns nullptr for missing key", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    REQUIRE(rs.findRule("ghost") == nullptr);
}

TEST_CASE("E.1 PCGRuleSet: getValue with fallback", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    REQUIRE(rs.getValue("missing", "default") == "default");
}

TEST_CASE("E.1 PCGRuleSet: hasRule true/false", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    REQUIRE(rs.hasRule("density"));
    REQUIRE_FALSE(rs.hasRule("ghost"));
}

TEST_CASE("E.1 PCGRuleSet: resetToDefaults restores all values", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet("test", 10, 1.0f);
    rs.setValue("density", "0.25");
    REQUIRE(rs.getValue("density") != "1");
    rs.resetToDefaults();
    // default was set to std::to_string(1.0f) — just check it changed back
    REQUIRE_FALSE(rs.getValue("density") == "0.25");
}

TEST_CASE("E.1 PCGRuleSet: resetRule resets single rule", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    rs.setValue("count", "99");
    REQUIRE(rs.resetRule("count"));
    REQUIRE(rs.getValue("count") == "10");
}

TEST_CASE("E.1 PCGRuleSet: resetRule non-existent returns false", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    REQUIRE_FALSE(rs.resetRule("ghost"));
}

TEST_CASE("E.1 PCGRuleSet: rulesInCategory filters by category", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    PCGRule r1; r1.key = "density"; r1.value = "1.0"; r1.category = "placement";
    PCGRule r2; r2.key = "count";   r2.value = "10";  r2.category = "placement";
    PCGRule r3; r3.key = "seed";    r3.value = "42";  r3.category = "random";
    rs.addRule(r1); rs.addRule(r2); rs.addRule(r3);
    auto placement = rs.rulesInCategory("placement");
    REQUIRE(placement.size() == 2);
    auto random = rs.rulesInCategory("random");
    REQUIRE(random.size() == 1);
}

TEST_CASE("E.1 PCGRuleSet: clearDirty resets flag", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs = makePCGRuleSet();
    REQUIRE(rs.isDirty());
    rs.clearDirty();
    REQUIRE_FALSE(rs.isDirty());
}

TEST_CASE("E.1 PCGRuleSet: default version is 1.0", "[phase_e][e1][ruleset]") {
    PCGRuleSet rs("id", "domain");
    REQUIRE(rs.version() == "1.0");
}

TEST_CASE("E.1 PCGRuleValueType: pcgRuleValueTypeName covers all types", "[phase_e][e1][ruleset]") {
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Float))  == "Float");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Int))    == "Int");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Bool))   == "Bool");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::String)) == "String");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Vec2))   == "Vec2");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Vec3))   == "Vec3");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Range))  == "Range");
    REQUIRE(std::string(pcgRuleValueTypeName(PCGRuleValueType::Tag))    == "Tag");
}

// ═══════════════════════════════════════════════════════════════════════════
//  E.1 — PCGDeterministicSeedContext
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("E.1 SeedCtx: default universe seed is 42424242", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    REQUIRE(ctx.universeSeed() == PCGDeterministicSeedContext::kDefaultUniverseSeed);
}

TEST_CASE("E.1 SeedCtx: constructor accepts custom seed", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx(12345678ULL);
    REQUIRE(ctx.universeSeed() == 12345678ULL);
}

TEST_CASE("E.1 SeedCtx: setUniverseSeed updates seed", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    ctx.setUniverseSeed(999ULL);
    REQUIRE(ctx.universeSeed() == 999ULL);
}

TEST_CASE("E.1 SeedCtx: setUniverseSeed(0) remapped to 1", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    ctx.setUniverseSeed(0);
    REQUIRE(ctx.universeSeed() == 1);
}

TEST_CASE("E.1 SeedCtx: seedForDomain returns non-zero value", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx(100ULL);
    REQUIRE(ctx.seedForDomain("ship") != 0);
}

TEST_CASE("E.1 SeedCtx: seedForDomain is deterministic (same call → same result)", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx(100ULL);
    uint64_t a = ctx.seedForDomain("ship");
    uint64_t b = ctx.seedForDomain("ship");
    REQUIRE(a == b);
}

TEST_CASE("E.1 SeedCtx: different domains yield different seeds", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx(100ULL);
    REQUIRE(ctx.seedForDomain("ship")   != ctx.seedForDomain("planet"));
    REQUIRE(ctx.seedForDomain("planet") != ctx.seedForDomain("asteroid"));
}

TEST_CASE("E.1 SeedCtx: same seed + domain → same result across two contexts", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx1(42ULL);
    PCGDeterministicSeedContext ctx2(42ULL);
    REQUIRE(ctx1.seedForDomain("galaxy") == ctx2.seedForDomain("galaxy"));
}

TEST_CASE("E.1 SeedCtx: different universe seeds → different domain seeds", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx1(10ULL);
    PCGDeterministicSeedContext ctx2(99ULL);
    REQUIRE(ctx1.seedForDomain("ship") != ctx2.seedForDomain("ship"));
}

TEST_CASE("E.1 SeedCtx: setUniverseSeed clears domain cache", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx(10ULL);
    uint64_t before = ctx.seedForDomain("ship");
    ctx.setUniverseSeed(99ULL);
    uint64_t after = ctx.seedForDomain("ship");
    REQUIRE(before != after);
}

TEST_CASE("E.1 SeedCtx: childContext has different seed than parent for same domain", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext parent(42ULL);
    auto child = parent.childContext("sector_001");
    REQUIRE(child.universeSeed() != parent.universeSeed());
}

TEST_CASE("E.1 SeedCtx: childContext is deterministic", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext parent(42ULL);
    auto child1 = parent.childContext("sector_001");
    auto child2 = parent.childContext("sector_001");
    REQUIRE(child1.universeSeed() == child2.universeSeed());
}

TEST_CASE("E.1 SeedCtx: pinDomainSeed overrides derived seed", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx(42ULL);
    ctx.pinDomainSeed("ship", 1234567890ULL);
    REQUIRE(ctx.seedForDomain("ship") == 1234567890ULL);
}

TEST_CASE("E.1 SeedCtx: hasPinnedSeed true after pin", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    REQUIRE_FALSE(ctx.hasPinnedSeed("ship"));
    ctx.pinDomainSeed("ship", 42ULL);
    REQUIRE(ctx.hasPinnedSeed("ship"));
}

TEST_CASE("E.1 SeedCtx: clearPinnedSeeds removes all pins", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    ctx.pinDomainSeed("ship",   42ULL);
    ctx.pinDomainSeed("planet", 99ULL);
    ctx.clearPinnedSeeds();
    REQUIRE_FALSE(ctx.hasPinnedSeed("ship"));
    REQUIRE_FALSE(ctx.hasPinnedSeed("planet"));
}

TEST_CASE("E.1 SeedCtx: registerDomain adds domain", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    ctx.registerDomain("ship");
    REQUIRE(ctx.hasDomain("ship"));
    REQUIRE(ctx.registeredDomains().size() == 1);
}

TEST_CASE("E.1 SeedCtx: registerDomain duplicate does not double-add", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    ctx.registerDomain("ship");
    ctx.registerDomain("ship");
    REQUIRE(ctx.registeredDomains().size() == 1);
}

TEST_CASE("E.1 SeedCtx: hasDomain false for unregistered", "[phase_e][e1][seed]") {
    PCGDeterministicSeedContext ctx;
    REQUIRE_FALSE(ctx.hasDomain("ship"));
}

// ═══════════════════════════════════════════════════════════════════════════
//  E.1 — PCGGeneratorService
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("E.1 Generator: empty ruleset → success with 0 placements", "[phase_e][e1][generator]") {
    PCGRuleSet rs("empty_rs", "domain");
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto result = gen.generate(rs, ctx);
    REQUIRE(result.success);
    REQUIRE(result.placements.empty());
    REQUIRE(result.ruleSetId == "empty_rs");
}

TEST_CASE("E.1 Generator: density=1, count=10 → 10 placements", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 10, 1.0f);
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto result = gen.generate(rs, ctx);
    REQUIRE(result.success);
    REQUIRE(result.placements.size() == 10);
}

TEST_CASE("E.1 Generator: density=0.5, count=10 → 5 placements", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 10, 0.5f);
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto result = gen.generate(rs, ctx);
    REQUIRE(result.success);
    REQUIRE(result.placements.size() == 5);
}

TEST_CASE("E.1 Generator: same inputs → same placement count", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 10, 1.0f);
    PCGDeterministicSeedContext ctx(42ULL);
    PCGGeneratorService gen;
    auto r1 = gen.generate(rs, ctx);
    auto r2 = gen.generate(rs, ctx);
    REQUIRE(r1.placements.size() == r2.placements.size());
}

TEST_CASE("E.1 Generator: same inputs → first placement at same position (deterministic)", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);
    PCGDeterministicSeedContext ctx(42ULL);
    PCGGeneratorService gen;
    auto r1 = gen.generate(rs, ctx);
    auto r2 = gen.generate(rs, ctx);
    REQUIRE(r1.placements[0].x == r2.placements[0].x);
    REQUIRE(r1.placements[0].z == r2.placements[0].z);
}

TEST_CASE("E.1 Generator: different seeds → different first placement positions", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);
    PCGDeterministicSeedContext ctx1(100ULL);
    PCGDeterministicSeedContext ctx2(200ULL);
    PCGGeneratorService gen;
    auto r1 = gen.generate(rs, ctx1);
    auto r2 = gen.generate(rs, ctx2);
    // At least one coordinate should differ (extremely unlikely to match)
    bool differs = (r1.placements[0].x != r2.placements[0].x) ||
                   (r1.placements[0].z != r2.placements[0].z);
    REQUIRE(differs);
}

TEST_CASE("E.1 Generator: domain in result from ruleset domain", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto r = gen.generate(rs, ctx);
    REQUIRE(r.domain == "test_domain");
}

TEST_CASE("E.1 Generator: domainOverride takes priority", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto r = gen.generate(rs, ctx, "custom_domain");
    REQUIRE(r.domain == "custom_domain");
}

TEST_CASE("E.1 Generator: placements have non-empty assetTag from placementTag rule", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 3, 1.0f);
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto r = gen.generate(rs, ctx);
    REQUIRE_FALSE(r.placements.empty());
    REQUIRE(r.placements[0].assetTag == "asteroid.small");
}

TEST_CASE("E.1 Generator: placements scale within minScale/maxScale", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet("rs", 20, 1.0f);
    PCGRule rMin; rMin.key = "minScale"; rMin.value = "0.5"; rMin.defaultValue = "0.5"; rs.addRule(rMin);
    PCGRule rMax; rMax.key = "maxScale"; rMax.value = "2.0"; rMax.defaultValue = "2.0"; rs.addRule(rMax);
    PCGDeterministicSeedContext ctx;
    PCGGeneratorService gen;
    auto r = gen.generate(rs, ctx);
    for (const auto& p : r.placements) {
        REQUIRE(p.scale >= 0.5f);
        REQUIRE(p.scale <= 2.0f);
    }
}

TEST_CASE("E.1 Generator: validate warns on missing recommended rules", "[phase_e][e1][generator]") {
    PCGRuleSet rs("id", "domain"); // no rules at all
    auto v = PCGGeneratorService::validate(rs);
    REQUIRE_FALSE(v.warnings.empty());
}

TEST_CASE("E.1 Generator: validate valid=false for empty id", "[phase_e][e1][generator]") {
    PCGRuleSet rs; // empty id
    auto v = PCGGeneratorService::validate(rs);
    REQUIRE_FALSE(v.valid);
}

TEST_CASE("E.1 Generator: validate valid=true for properly formed ruleset", "[phase_e][e1][generator]") {
    PCGRuleSet rs = makePCGRuleSet();
    auto v = PCGGeneratorService::validate(rs);
    REQUIRE(v.valid);
    REQUIRE(v.missingRules.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
//  E.2 — PCGPreviewService
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("E.2 PreviewService: starts with no ruleset", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    REQUIRE_FALSE(svc.hasRuleSet());
    REQUIRE_FALSE(svc.hasResult());
}

TEST_CASE("E.2 PreviewService: bindRuleSet sets hasRuleSet=true", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE(svc.hasRuleSet());
    REQUIRE(svc.ruleSet() == &rs);
}

TEST_CASE("E.2 PreviewService: clearRuleSet removes ruleset", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    svc.clearRuleSet();
    REQUIRE_FALSE(svc.hasRuleSet());
}

TEST_CASE("E.2 PreviewService: autoRegenerate=true → hasResult after bindRuleSet", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(true);
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE(svc.hasResult());
}

TEST_CASE("E.2 PreviewService: autoRegenerate=false → no result after bindRuleSet", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(false);
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE_FALSE(svc.hasResult());
}

TEST_CASE("E.2 PreviewService: forceRegenerate → hasResult=true", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(false);
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE_FALSE(svc.hasResult());
    REQUIRE(svc.forceRegenerate());
    REQUIRE(svc.hasResult());
}

TEST_CASE("E.2 PreviewService: forceRegenerate returns false with no ruleset", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    REQUIRE_FALSE(svc.forceRegenerate());
    REQUIRE_FALSE(svc.hasResult());
}

TEST_CASE("E.2 PreviewService: setSeed changes seed", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    svc.setSeed(12345ULL);
    REQUIRE(svc.currentSeed() == 12345ULL);
}

TEST_CASE("E.2 PreviewService: default seed is 42424242", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    REQUIRE(svc.currentSeed() == PCGDeterministicSeedContext::kDefaultUniverseSeed);
}

TEST_CASE("E.2 PreviewService: stats.generationCount increments on each regen", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(false);
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE(svc.stats().generationCount == 0);
    svc.forceRegenerate();
    REQUIRE(svc.stats().generationCount == 1);
    svc.forceRegenerate();
    REQUIRE(svc.stats().generationCount == 2);
}

TEST_CASE("E.2 PreviewService: stats.lastPlacementCount matches result", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 7, 1.0f);
    svc.bindRuleSet(&rs);
    REQUIRE(svc.stats().lastPlacementCount == 7);
}

TEST_CASE("E.2 PreviewService: stats.hasResult true after regen", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE(svc.stats().hasResult);
}

TEST_CASE("E.2 PreviewService: lastResult has correct ruleSetId", "[phase_e][e2][preview_service]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("my_ruleset");
    svc.bindRuleSet(&rs);
    REQUIRE(svc.lastResult().ruleSetId == "my_ruleset");
}

// ═══════════════════════════════════════════════════════════════════════════
//  E.3 — Rule editing via PCGPreviewService
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("E.3 Rule edit: setRuleValue triggers regen when autoRegenerate=true", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(true);
    PCGRuleSet rs = makePCGRuleSet("rs", 10, 1.0f);
    svc.bindRuleSet(&rs);
    uint32_t countBefore = svc.stats().generationCount;
    REQUIRE(svc.setRuleValue("count", "5"));
    REQUIRE(svc.stats().generationCount == countBefore + 1);
    REQUIRE(svc.lastResult().placements.size() == 5);
}

TEST_CASE("E.3 Rule edit: setRuleValue does NOT regen when autoRegenerate=false", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(false);
    PCGRuleSet rs = makePCGRuleSet("rs", 10, 1.0f);
    svc.bindRuleSet(&rs);
    uint32_t countBefore = svc.stats().generationCount;
    svc.setRuleValue("count", "5");
    REQUIRE(svc.stats().generationCount == countBefore);
}

TEST_CASE("E.3 Rule edit: setRuleValue returns false for missing key", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE_FALSE(svc.setRuleValue("ghost", "x"));
}

TEST_CASE("E.3 Rule edit: setRuleValue returns false with no ruleset", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    REQUIRE_FALSE(svc.setRuleValue("density", "0.5"));
}

TEST_CASE("E.3 Rule edit: resetRules reverts to defaults and triggers regen", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(true);
    PCGRuleSet rs = makePCGRuleSet("rs", 10, 1.0f);
    svc.bindRuleSet(&rs);
    svc.setRuleValue("count", "3");
    REQUIRE(svc.lastResult().placements.size() == 3);

    svc.resetRules();
    REQUIRE(svc.lastResult().placements.size() == 10);
}

TEST_CASE("E.3 Rule edit: change callback invoked after regen", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);

    bool callbackFired = false;
    svc.setOnRegenerateCallback([&](const PCGGenerationResult& r) {
        callbackFired = true;
    });
    svc.bindRuleSet(&rs);
    REQUIRE(callbackFired);
}

TEST_CASE("E.3 Rule edit: callback receives correct result", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 7, 1.0f);

    std::string receivedId;
    svc.setOnRegenerateCallback([&](const PCGGenerationResult& r) {
        receivedId = r.ruleSetId;
    });
    svc.bindRuleSet(&rs);
    REQUIRE(receivedId == "rs");
}

TEST_CASE("E.3 Rule edit: domainOverride used in generation", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    svc.setDomainOverride("custom_override");
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    REQUIRE(svc.lastResult().domain == "custom_override");
}

TEST_CASE("E.3 Rule edit: domainOverride getter", "[phase_e][e3][rule_edit]") {
    PCGPreviewService svc;
    svc.setDomainOverride("planet.ring");
    REQUIRE(svc.domainOverride() == "planet.ring");
}

// ═══════════════════════════════════════════════════════════════════════════
//  E.4 — PCGPreviewService → NovaForgePreviewWorld population
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("E.4 WorldPop: populatePreviewWorld returns 0 before regen", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    svc.setAutoRegenerate(false);
    PCGRuleSet rs = makePCGRuleSet();
    svc.bindRuleSet(&rs);
    NovaForgePreviewWorld world;
    REQUIRE(svc.populatePreviewWorld(world) == 0);
    REQUIRE(world.entityCount() == 0);
}

TEST_CASE("E.4 WorldPop: populatePreviewWorld creates correct entity count", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);
    svc.bindRuleSet(&rs);
    NovaForgePreviewWorld world;
    uint32_t created = svc.populatePreviewWorld(world);
    REQUIRE(created == 5);
    REQUIRE(world.entityCount() == 5);
}

TEST_CASE("E.4 WorldPop: populatePreviewWorld clears world first", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 3, 1.0f);
    svc.bindRuleSet(&rs);
    NovaForgePreviewWorld world;
    svc.populatePreviewWorld(world);
    // Run again — should still have 3 (old ones cleared)
    svc.populatePreviewWorld(world);
    REQUIRE(world.entityCount() == 3);
}

TEST_CASE("E.4 WorldPop: entities have meshTag set from placementTag", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 2, 1.0f);
    svc.bindRuleSet(&rs);
    NovaForgePreviewWorld world;
    svc.populatePreviewWorld(world);
    const auto& entities = world.entities();
    REQUIRE_FALSE(entities.empty());
    REQUIRE_FALSE(entities[0].meshTag.empty());
}

TEST_CASE("E.4 WorldPop: regeneration with different seed changes positions", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 5, 1.0f);

    svc.setSeed(100ULL);
    svc.bindRuleSet(&rs);
    NovaForgePreviewWorld world1;
    svc.populatePreviewWorld(world1);
    float x1 = world1.entities()[0].transform.position.x;

    svc.setSeed(999ULL);
    svc.forceRegenerate();
    NovaForgePreviewWorld world2;
    svc.populatePreviewWorld(world2);
    float x2 = world2.entities()[0].transform.position.x;

    REQUIRE(x1 != x2);
}

TEST_CASE("E.4 WorldPop: populatePreviewWorld world not dirty after populate", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    PCGRuleSet rs = makePCGRuleSet("rs", 3, 1.0f);
    svc.bindRuleSet(&rs);
    NovaForgePreviewWorld world;
    svc.populatePreviewWorld(world);
    REQUIRE_FALSE(world.isDirty());
}

TEST_CASE("E.4 WorldPop: autoRegenerate seedCtx accessible and mutable", "[phase_e][e4][world_pop]") {
    PCGPreviewService svc;
    svc.seedContext().registerDomain("ship");
    REQUIRE(svc.seedContext().hasDomain("ship"));
}
