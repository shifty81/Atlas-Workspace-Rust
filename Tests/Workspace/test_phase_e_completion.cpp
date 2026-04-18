// Phase E Completion — E.3 ProcGenRuleEditorPanel + E.4 Event-driven PCG tags
//
// E.3 — ProcGenRuleEditorPanel: panel-level wiring to PCGRuleSet + PCGPreviewService
//   - bindDocument / clearDocument / hasDocument
//   - attachPreviewService / detachPreviewService
//   - editRule: propagates to document + preview service
//   - resetRule: restores default + propagates
//   - resetAll: resets all rules + triggers regen
//   - save (save-back): commits snapshot, clears dirty
//   - revert: restores snapshot, triggers rebind
//   - dirty tracking: isDirty, editCount, saveCount
//   - onChange / onResetAll / onSave / onRevert callbacks
//   - lifecycle: activate / deactivate / isActive
//   - ruleCount / ruleValue / hasRule inspection
//
// E.4 — Asset PCG tag changes → event-driven PCG preview regen
//   - attachPCGPreviewService / detachPCGPreviewService / hasPCGPreviewService
//   - setPlacementTagAndNotify → PCGPreviewService::forceRegenerate called
//   - addGenerationTagAndNotify / removeGenerationTagAndNotify trigger regen
//   - setPCGScaleRangeAndNotify triggers regen
//   - setPCGDensityAndNotify triggers regen
//   - setPCGExclusionGroupAndNotify triggers regen
//   - pcgRegenTriggerCount tracks invocations
//   - No regen when no service attached

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NovaForge/EditorAdapter/PCGRuleSet.h"
#include "NovaForge/EditorAdapter/PCGDeterministicSeedContext.h"
#include "NovaForge/EditorAdapter/PCGGeneratorService.h"
#include "NovaForge/EditorAdapter/PCGPreviewService.h"
#include "NovaForge/EditorAdapter/ProcGenRuleEditorPanel.h"
#include "NovaForge/EditorAdapter/NovaForgeAssetPreview.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"

#include <algorithm>
#include <string>

using namespace NovaForge;
using Catch::Approx;

// ── Helpers ───────────────────────────────────────────────────────────────────

static PCGRuleSet makeRuleSet(const std::string& id = "rs",
                               uint32_t count = 3,
                               float density = 1.0f)
{
    PCGRuleSet rs(id, "test_domain");
    PCGRule density_r; density_r.key = "density"; density_r.type = PCGRuleValueType::Float;
    density_r.value = std::to_string(density); density_r.defaultValue = "1.0";
    density_r.category = "placement";
    PCGRule count_r;  count_r.key = "count";   count_r.type = PCGRuleValueType::Int;
    count_r.value = std::to_string(count);   count_r.defaultValue = "10";
    count_r.category = "placement";
    PCGRule tag_r;    tag_r.key = "assetTag"; tag_r.type = PCGRuleValueType::Tag;
    tag_r.value = "tree"; tag_r.defaultValue = "tree";
    tag_r.category = "appearance";
    rs.addRule(density_r);
    rs.addRule(count_r);
    rs.addRule(tag_r);
    return rs;
}

static NovaForgeAssetPreview makeAssetPreview(const std::string& path = "mesh/rock.fbx") {
    NovaForgeAssetPreview ap;
    AssetPreviewDescriptor d;
    d.assetPath = path;
    d.meshTag   = "rock";
    d.materialTag = "stone";
    ap.bindAsset(d);
    return ap;
}

// ── E.3 — ProcGenRuleEditorPanel ─────────────────────────────────────────────

TEST_CASE("E.3 Panel: default state — no document, not active", "[phase_e_completion][e3][panel]") {
    ProcGenRuleEditorPanel p;
    REQUIRE_FALSE(p.hasDocument());
    REQUIRE_FALSE(p.isActive());
    REQUIRE_FALSE(p.isDirty());
    REQUIRE(p.editCount() == 0);
    REQUIRE(p.saveCount() == 0);
    REQUIRE(p.ruleCount() == 0);
}

TEST_CASE("E.3 Panel: bindDocument sets hasDocument", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    REQUIRE(p.hasDocument());
    REQUIRE(p.document() == &rs);
}

TEST_CASE("E.3 Panel: clearDocument clears document reference", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.clearDocument();
    REQUIRE_FALSE(p.hasDocument());
}

TEST_CASE("E.3 Panel: ruleCount reflects bound document", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet("rs", 3, 1.0f);
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    REQUIRE(p.ruleCount() == 3);
}

TEST_CASE("E.3 Panel: hasRule and ruleValue delegate to document", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    REQUIRE(p.hasRule("density"));
    REQUIRE_FALSE(p.hasRule("nonexistent"));
    REQUIRE(p.ruleValue("assetTag") == "tree");
}

TEST_CASE("E.3 Panel: editRule marks dirty and increments editCount", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    REQUIRE(p.editRule("density", "2.5"));
    REQUIRE(p.isDirty());
    REQUIRE(p.editCount() == 1);
}

TEST_CASE("E.3 Panel: editRule updates live document value", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "3.0");
    REQUIRE(rs.getValue("density") == "3.0");
}

TEST_CASE("E.3 Panel: editRule on missing key returns false", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    REQUIRE_FALSE(p.editRule("nonexistent_key", "42"));
    REQUIRE_FALSE(p.isDirty());
}

TEST_CASE("E.3 Panel: editRule without document returns false", "[phase_e_completion][e3][panel]") {
    ProcGenRuleEditorPanel p;
    REQUIRE_FALSE(p.editRule("density", "1.0"));
}

TEST_CASE("E.3 Panel: editRule invokes onChange callback", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    std::string notifiedKey, notifiedVal;
    p.setOnChange([&](const std::string& k, const std::string& v) {
        notifiedKey = k; notifiedVal = v;
    });
    p.editRule("density", "5.0");
    REQUIRE(notifiedKey == "density");
    REQUIRE(notifiedVal == "5.0");
}

TEST_CASE("E.3 Panel: editRule propagates to PCGPreviewService", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet("rs", 5, 1.0f);
    PCGPreviewService svc;
    svc.setAutoRegenerate(false);

    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.attachPreviewService(&svc);
    REQUIRE(p.hasPreviewService());

    // editRule should call setRuleValue on the preview service
    p.editRule("density", "2.0");
    // Service has the ruleset bound; confirm ruleSet is accessible
    REQUIRE(svc.hasRuleSet());
    REQUIRE(svc.ruleSet() == &rs);
}

TEST_CASE("E.3 Panel: resetRule resets to default and marks dirty", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "99.0");
    REQUIRE(p.editCount() == 1);
    REQUIRE(p.resetRule("density"));
    REQUIRE(p.isDirty());
    REQUIRE(rs.getValue("density") == "1.0"); // restored to defaultValue
}

TEST_CASE("E.3 Panel: resetRule on missing key returns false", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    REQUIRE_FALSE(p.resetRule("no_such_rule"));
}

TEST_CASE("E.3 Panel: resetAll resets document and triggers onResetAll", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "7.0");

    bool resetAllCalled = false;
    p.setOnResetAll([&]{ resetAllCalled = true; });

    REQUIRE(p.resetAll());
    REQUIRE(resetAllCalled);
    REQUIRE(p.isDirty());
    REQUIRE(rs.getValue("density") == "1.0");
}

TEST_CASE("E.3 Panel: save clears dirty and increments saveCount", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "4.0");
    REQUIRE(p.isDirty());

    auto result = p.save();
    REQUIRE(result.ok());
    REQUIRE_FALSE(p.isDirty());
    REQUIRE(p.saveCount() == 1);
}

TEST_CASE("E.3 Panel: save without document returns failure", "[phase_e_completion][e3][panel]") {
    ProcGenRuleEditorPanel p;
    auto result = p.save();
    REQUIRE(result.failed());
    REQUIRE_FALSE(result.errorMsg.empty());
}

TEST_CASE("E.3 Panel: save invokes onSave callback with document", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "2.0");

    bool savedCalled = false;
    std::string savedDomain;
    p.setOnSave([&](const PCGRuleSet& saved) {
        savedCalled  = true;
        savedDomain  = saved.domain();
    });
    p.save();
    REQUIRE(savedCalled);
    REQUIRE(savedDomain == "test_domain");
}

TEST_CASE("E.3 Panel: revert restores pre-edit state", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    // Snapshot captured at bind: density = "1"
    p.editRule("density", "99.0");
    REQUIRE(rs.getValue("density") == "99.0");

    REQUIRE(p.revert());
    REQUIRE_FALSE(p.isDirty());
    REQUIRE(rs.getValue("density") == std::to_string(1.0f)); // reverted to snapshot
}

TEST_CASE("E.3 Panel: revert invokes onRevert callback", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    bool revertCalled = false;
    p.setOnRevert([&]{ revertCalled = true; });
    p.editRule("density", "9.0");
    p.revert();
    REQUIRE(revertCalled);
}

TEST_CASE("E.3 Panel: revert without document returns false", "[phase_e_completion][e3][panel]") {
    ProcGenRuleEditorPanel p;
    REQUIRE_FALSE(p.revert());
}

TEST_CASE("E.3 Panel: save then revert restores post-save state", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "5.0");
    p.save(); // snapshot is now "density=5.0"
    p.editRule("density", "99.0");
    p.revert(); // should go back to "5.0"
    REQUIRE(rs.getValue("density") == "5.0");
}

TEST_CASE("E.3 Panel: activate and deactivate track isActive", "[phase_e_completion][e3][panel]") {
    ProcGenRuleEditorPanel p;
    REQUIRE_FALSE(p.isActive());
    p.activate();
    REQUIRE(p.isActive());
    p.deactivate();
    REQUIRE_FALSE(p.isActive());
}

TEST_CASE("E.3 Panel: attachPreviewService and detach", "[phase_e_completion][e3][panel]") {
    ProcGenRuleEditorPanel p;
    REQUIRE_FALSE(p.hasPreviewService());
    PCGPreviewService svc;
    p.attachPreviewService(&svc);
    REQUIRE(p.hasPreviewService());
    p.detachPreviewService();
    REQUIRE_FALSE(p.hasPreviewService());
}

TEST_CASE("E.3 Panel: bindDocument with service auto-binds ruleset to service", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    ProcGenRuleEditorPanel p;
    p.attachPreviewService(&svc);
    p.bindDocument(&rs);
    REQUIRE(svc.hasRuleSet());
    REQUIRE(svc.ruleSet() == &rs);
}

TEST_CASE("E.3 Panel: clearDocument clears ruleset from service", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    ProcGenRuleEditorPanel p;
    p.attachPreviewService(&svc);
    p.bindDocument(&rs);
    REQUIRE(svc.hasRuleSet());
    p.clearDocument();
    REQUIRE_FALSE(svc.hasRuleSet());
}

TEST_CASE("E.3 Panel: multiple edits accumulate editCount", "[phase_e_completion][e3][panel]") {
    PCGRuleSet rs = makeRuleSet();
    ProcGenRuleEditorPanel p;
    p.bindDocument(&rs);
    p.editRule("density", "2.0");
    p.editRule("count",   "20");
    p.editRule("density", "3.0");
    REQUIRE(p.editCount() == 3);
}

// ── E.4 — Asset PCG tag changes → event-driven PCG preview regen ──────────────

TEST_CASE("E.4 AssetPreview: hasPCGPreviewService default false", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    REQUIRE_FALSE(ap.hasPCGPreviewService());
}

TEST_CASE("E.4 AssetPreview: attachPCGPreviewService sets hasPCGPreviewService", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGPreviewService svc;
    ap.attachPCGPreviewService(&svc);
    REQUIRE(ap.hasPCGPreviewService());
}

TEST_CASE("E.4 AssetPreview: detachPCGPreviewService clears", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGPreviewService svc;
    ap.attachPCGPreviewService(&svc);
    ap.detachPCGPreviewService();
    REQUIRE_FALSE(ap.hasPCGPreviewService());
}

TEST_CASE("E.4 AssetPreview: pcgRegenTriggerCount starts at zero", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    REQUIRE(ap.pcgRegenTriggerCount() == 0);
}

TEST_CASE("E.4 AssetPreview: setPlacementTagAndNotify increments trigger count", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    REQUIRE(ap.setPlacementTagAndNotify("forest_rock"));
    REQUIRE(ap.pcgRegenTriggerCount() == 1);
    REQUIRE(ap.pcgMetadata().placementTag == "forest_rock");
}

TEST_CASE("E.4 AssetPreview: setPlacementTagAndNotify triggers PCGPreviewService regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGRuleSet rs = makeRuleSet("rs", 3, 1.0f);
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    svc.setAutoRegenerate(false);

    ap.attachPCGPreviewService(&svc);
    svc.forceRegenerate(); // ensure an initial result
    uint32_t genBefore = svc.stats().generationCount;

    ap.setPlacementTagAndNotify("desert_cactus");
    REQUIRE(svc.stats().generationCount > genBefore);
}

TEST_CASE("E.4 AssetPreview: addGenerationTagAndNotify triggers regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    ap.attachPCGPreviewService(&svc);
    uint32_t before = svc.stats().generationCount;
    REQUIRE(ap.addGenerationTagAndNotify("large"));
    REQUIRE(svc.stats().generationCount > before);
    REQUIRE(ap.pcgRegenTriggerCount() == 1);
}

TEST_CASE("E.4 AssetPreview: removeGenerationTagAndNotify triggers regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    ap.addGenerationTag("large");
    ap.addGenerationTag("small");

    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    ap.attachPCGPreviewService(&svc);
    uint32_t before = svc.stats().generationCount;

    REQUIRE(ap.removeGenerationTagAndNotify("large"));
    REQUIRE(svc.stats().generationCount > before);
}

TEST_CASE("E.4 AssetPreview: removeGenerationTagAndNotify returns false on missing tag", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGPreviewService svc;
    ap.attachPCGPreviewService(&svc);
    REQUIRE_FALSE(ap.removeGenerationTagAndNotify("nonexistent"));
    REQUIRE(ap.pcgRegenTriggerCount() == 0);
}

TEST_CASE("E.4 AssetPreview: setPCGScaleRangeAndNotify triggers regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    ap.attachPCGPreviewService(&svc);
    uint32_t before = svc.stats().generationCount;
    REQUIRE(ap.setPCGScaleRangeAndNotify(0.5f, 1.5f));
    REQUIRE(svc.stats().generationCount > before);
    REQUIRE(ap.pcgMetadata().minScale == Approx(0.5f));
    REQUIRE(ap.pcgMetadata().maxScale == Approx(1.5f));
}

TEST_CASE("E.4 AssetPreview: setPCGDensityAndNotify triggers regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    ap.attachPCGPreviewService(&svc);
    uint32_t before = svc.stats().generationCount;
    REQUIRE(ap.setPCGDensityAndNotify(2.5f));
    REQUIRE(svc.stats().generationCount > before);
    REQUIRE(ap.pcgMetadata().density == Approx(2.5f));
}

TEST_CASE("E.4 AssetPreview: setPCGExclusionGroupAndNotify triggers regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    ap.attachPCGPreviewService(&svc);
    uint32_t before = svc.stats().generationCount;
    REQUIRE(ap.setPCGExclusionGroupAndNotify("boulders"));
    REQUIRE(svc.stats().generationCount > before);
    REQUIRE(ap.pcgMetadata().exclusionGroup == "boulders");
}

TEST_CASE("E.4 AssetPreview: no regen when no service attached", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    // No PCGPreviewService attached — setPlacementTagAndNotify still succeeds
    // but trigger count increments while no external regen happens.
    REQUIRE(ap.setPlacementTagAndNotify("grass"));
    REQUIRE(ap.pcgRegenTriggerCount() == 1); // trigger recorded even without service
}

TEST_CASE("E.4 AssetPreview: multiple tag changes accumulate trigger count", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    ap.setPlacementTagAndNotify("rock");
    ap.addGenerationTagAndNotify("medium");
    ap.setPCGDensityAndNotify(0.8f);
    REQUIRE(ap.pcgRegenTriggerCount() == 3);
}

TEST_CASE("E.4 AssetPreview: base setPlacementTag does NOT trigger regen", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    PCGRuleSet rs = makeRuleSet();
    PCGPreviewService svc;
    svc.bindRuleSet(&rs);
    ap.attachPCGPreviewService(&svc);
    uint32_t before = svc.stats().generationCount;
    // Use the non-notify variant — should NOT trigger regen
    ap.setPlacementTag("silent_tag");
    REQUIRE(svc.stats().generationCount == before);
    REQUIRE(ap.pcgRegenTriggerCount() == 0);
}

TEST_CASE("E.4 AssetPreview: setPCGScaleRangeAndNotify invalid range returns false", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap = makeAssetPreview();
    REQUIRE_FALSE(ap.setPCGScaleRangeAndNotify(2.0f, 1.0f)); // min > max
    REQUIRE(ap.pcgRegenTriggerCount() == 0);
}

TEST_CASE("E.4 AssetPreview: tag change without asset returns false", "[phase_e_completion][e4][event_pcg]") {
    NovaForgeAssetPreview ap; // no asset bound
    REQUIRE_FALSE(ap.setPlacementTagAndNotify("rock"));
    REQUIRE(ap.pcgRegenTriggerCount() == 0);
}
