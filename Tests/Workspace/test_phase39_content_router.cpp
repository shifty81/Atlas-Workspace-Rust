// Tests/Workspace/test_phase39_content_router.cpp
// Phase 39 — Content Router and Drop Target Handler
//
// Tests for:
//   1. ContentRouterPolicy — enum name helpers
//   2. RouteResult         — default state, succeeded()
//   3. RoutingRule         — isValid, matches (type/source filters, enabled flag)
//   4. ContentRouter       — addRule/removeRule/enableRule/route; policy handling
//   5. DropState/DropEffect — enum name helpers
//   6. DropTargetHandler   — onDragEnter/onDragOver/onDragLeave/onDrop; state tracking
//   7. Integration         — full intake→route pipeline, drop→route end-to-end

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/ContentRouter.h"
#include "NF/Workspace/DropTargetHandler.h"
#include <string>
#include <vector>

using namespace NF;

// Helper: build a simple routing rule
static RoutingRule makeRule(const std::string& name, const std::string& toolId,
                            AssetTypeTag tag, int priority = 0) {
    RoutingRule r;
    r.name     = name;
    r.toolId   = toolId;
    r.typeTag  = tag;
    r.priority = priority;
    r.enabled  = true;
    return r;
}

// Helper: build a wildcard rule (typeTag = Unknown = match all)
static RoutingRule makeWildcard(const std::string& name, const std::string& toolId,
                                int priority = 0) {
    return makeRule(name, toolId, AssetTypeTag::Unknown, priority);
}

// ─────────────────────────────────────────────────────────────────
// 1. ContentRouterPolicy enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ContentRouterPolicy name helpers all values", "[ContentRouterPolicy]") {
    CHECK(std::string(contentRouterPolicyName(ContentRouterPolicy::Reject))     == "Reject");
    CHECK(std::string(contentRouterPolicyName(ContentRouterPolicy::UseDefault)) == "UseDefault");
    CHECK(std::string(contentRouterPolicyName(ContentRouterPolicy::Prompt))     == "Prompt");
}

// ─────────────────────────────────────────────────────────────────
// 2. RouteResult
// ─────────────────────────────────────────────────────────────────
TEST_CASE("RouteResult default is not matched", "[RouteResult]") {
    RouteResult r;
    CHECK_FALSE(r.matched);
    CHECK_FALSE(r.succeeded());
    CHECK(r.toolId.empty());
    CHECK_FALSE(r.needsPrompt);
}

// ─────────────────────────────────────────────────────────────────
// 3. RoutingRule
// ─────────────────────────────────────────────────────────────────
TEST_CASE("RoutingRule invalid without name", "[RoutingRule]") {
    RoutingRule r;
    r.toolId = "scene_editor";
    CHECK_FALSE(r.isValid());
}

TEST_CASE("RoutingRule invalid without toolId", "[RoutingRule]") {
    RoutingRule r;
    r.name = "texture_rule";
    CHECK_FALSE(r.isValid());
}

TEST_CASE("RoutingRule valid with name and toolId", "[RoutingRule]") {
    auto r = makeRule("texture_rule", "material_editor", AssetTypeTag::Texture);
    CHECK(r.isValid());
}

TEST_CASE("RoutingRule matches correct type tag", "[RoutingRule]") {
    auto r = makeRule("tex", "material_editor", AssetTypeTag::Texture);
    CHECK(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
    CHECK_FALSE(r.matches(AssetTypeTag::Mesh, IntakeSource::FileDrop));
}

TEST_CASE("RoutingRule wildcard Unknown matches any type", "[RoutingRule]") {
    auto r = makeWildcard("catch_all", "asset_editor");
    CHECK(r.matches(AssetTypeTag::Texture,   IntakeSource::FileDrop));
    CHECK(r.matches(AssetTypeTag::Mesh,      IntakeSource::FileDrop));
    CHECK(r.matches(AssetTypeTag::Unknown,   IntakeSource::FileDrop));
}

TEST_CASE("RoutingRule disabled rule never matches", "[RoutingRule]") {
    auto r = makeRule("tex", "material_editor", AssetTypeTag::Texture);
    r.enabled = false;
    CHECK_FALSE(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
}

TEST_CASE("RoutingRule source filter blocks non-matching source", "[RoutingRule]") {
    RoutingRule r = makeRule("tex_drop", "material_editor", AssetTypeTag::Texture);
    r.filterBySource = true;
    r.sourceFilter   = IntakeSource::FileDrop;
    CHECK(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
    CHECK_FALSE(r.matches(AssetTypeTag::Texture, IntakeSource::CLI));
}

// ─────────────────────────────────────────────────────────────────
// 4. ContentRouter
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ContentRouter default policy is Reject", "[ContentRouter]") {
    ContentRouter router;
    CHECK(router.policy() == ContentRouterPolicy::Reject);
}

TEST_CASE("ContentRouter addRule succeeds for valid rule", "[ContentRouter]") {
    ContentRouter router;
    CHECK(router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture)));
    CHECK(router.ruleCount() == 1u);
}

TEST_CASE("ContentRouter addRule rejects invalid rule (no name)", "[ContentRouter]") {
    ContentRouter router;
    RoutingRule bad;
    bad.toolId = "editor";
    CHECK_FALSE(router.addRule(bad));
    CHECK(router.ruleCount() == 0u);
}

TEST_CASE("ContentRouter addRule rejects duplicate name", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    CHECK_FALSE(router.addRule(makeRule("tex", "other_editor", AssetTypeTag::Mesh)));
    CHECK(router.ruleCount() == 1u);
}

TEST_CASE("ContentRouter removeRule removes existing rule", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    CHECK(router.removeRule("tex"));
    CHECK(router.ruleCount() == 0u);
}

TEST_CASE("ContentRouter removeRule fails for unknown name", "[ContentRouter]") {
    ContentRouter router;
    CHECK_FALSE(router.removeRule("nope"));
}

TEST_CASE("ContentRouter hasRule returns correct result", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    CHECK(router.hasRule("tex"));
    CHECK_FALSE(router.hasRule("missing"));
}

TEST_CASE("ContentRouter enableRule toggles rule enabled state", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    CHECK(router.enableRule("tex", false));
    // Route should now miss (rule disabled)
    auto r = router.route(AssetTypeTag::Texture);
    CHECK_FALSE(r.succeeded());
}

TEST_CASE("ContentRouter route returns matched result for known type", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    auto r = router.route(AssetTypeTag::Texture);
    CHECK(r.succeeded());
    CHECK(r.toolId   == "material_editor");
    CHECK(r.ruleName == "tex");
}

TEST_CASE("ContentRouter route fails with Reject policy on no match", "[ContentRouter]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::Reject);
    auto r = router.route(AssetTypeTag::Mesh);
    CHECK_FALSE(r.succeeded());
    CHECK_FALSE(r.needsPrompt);
}

TEST_CASE("ContentRouter route uses default tool on UseDefault policy", "[ContentRouter]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::UseDefault);
    router.setDefaultToolId("asset_editor");
    auto r = router.route(AssetTypeTag::Mesh);
    CHECK(r.succeeded());
    CHECK(r.toolId   == "asset_editor");
    CHECK(r.ruleName == "default");
}

TEST_CASE("ContentRouter route signals prompt on Prompt policy", "[ContentRouter]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::Prompt);
    auto r = router.route(AssetTypeTag::Video);
    CHECK_FALSE(r.succeeded());
    CHECK(r.needsPrompt);
}

TEST_CASE("ContentRouter route respects priority ordering", "[ContentRouter]") {
    ContentRouter router;
    // Lower-priority wildcard registered first
    router.addRule(makeWildcard("catch_all", "asset_editor", 0));
    // Higher-priority specific rule registered second
    router.addRule(makeRule("tex_high", "material_editor", AssetTypeTag::Texture, 10));

    auto r = router.route(AssetTypeTag::Texture);
    CHECK(r.toolId   == "material_editor");
    CHECK(r.ruleName == "tex_high");
}

TEST_CASE("ContentRouter route increments routeCount on match", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    (void)router.route(AssetTypeTag::Texture);
    (void)router.route(AssetTypeTag::Texture);
    CHECK(router.routeCount() == 2u);
}

TEST_CASE("ContentRouter route increments missCount on no match", "[ContentRouter]") {
    ContentRouter router;
    (void)router.route(AssetTypeTag::Mesh);
    CHECK(router.missCount() == 1u);
}

TEST_CASE("ContentRouter clearRules empties rule list", "[ContentRouter]") {
    ContentRouter router;
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture));
    router.clearRules();
    CHECK(router.ruleCount() == 0u);
}

// ─────────────────────────────────────────────────────────────────
// 5. DropState / DropEffect enums
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DropState name helpers all values", "[DropState]") {
    CHECK(std::string(dropStateName(DropState::Idle))      == "Idle");
    CHECK(std::string(dropStateName(DropState::DragOver))  == "DragOver");
    CHECK(std::string(dropStateName(DropState::DragLeave)) == "DragLeave");
    CHECK(std::string(dropStateName(DropState::Dropped))   == "Dropped");
    CHECK(std::string(dropStateName(DropState::Rejected))  == "Rejected");
}

TEST_CASE("DropEffect name helpers all values", "[DropEffect]") {
    CHECK(std::string(dropEffectName(DropEffect::None)) == "None");
    CHECK(std::string(dropEffectName(DropEffect::Copy)) == "Copy");
    CHECK(std::string(dropEffectName(DropEffect::Move)) == "Move");
    CHECK(std::string(dropEffectName(DropEffect::Link)) == "Link");
}

// ─────────────────────────────────────────────────────────────────
// 6. DropTargetHandler
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DropTargetHandler default state is Idle", "[DropTargetHandler]") {
    DropTargetHandler h;
    CHECK(h.state() == DropState::Idle);
    CHECK_FALSE(h.isDragActive());
    CHECK(h.enterCount() == 0u);
    CHECK(h.dropCount()  == 0u);
}

TEST_CASE("DropTargetHandler default effect is Copy", "[DropTargetHandler]") {
    DropTargetHandler h;
    CHECK(h.defaultEffect() == DropEffect::Copy);
}

TEST_CASE("DropTargetHandler onDragEnter accepts known extension", "[DropTargetHandler]") {
    DropTargetHandler h;
    auto eff = h.onDragEnter({"/assets/hero.png"});
    CHECK(eff == DropEffect::Copy);
    CHECK(h.state()      == DropState::DragOver);
    CHECK(h.isDragActive());
    CHECK(h.enterCount() == 1u);
}

TEST_CASE("DropTargetHandler onDragEnter rejects unknown extension by default", "[DropTargetHandler]") {
    DropTargetHandler h;
    auto eff = h.onDragEnter({"/assets/weird.xyz"});
    CHECK(eff == DropEffect::None);
    CHECK(h.state() == DropState::Rejected);
}

TEST_CASE("DropTargetHandler onDragEnter accepts unknown extension when acceptUnknown set", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.setAcceptUnknown(true);
    auto eff = h.onDragEnter({"/assets/weird.xyz"});
    CHECK(eff == DropEffect::Copy);
    CHECK(h.state() == DropState::DragOver);
}

TEST_CASE("DropTargetHandler onDragOver returns Copy when not rejected", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.onDragEnter({"/assets/hero.png"});
    CHECK(h.onDragOver(100.f, 200.f) == DropEffect::Copy);
}

TEST_CASE("DropTargetHandler onDragOver returns None when rejected", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.onDragEnter({"/assets/weird.xyz"}); // sets Rejected
    CHECK(h.onDragOver(0.f, 0.f) == DropEffect::None);
}

TEST_CASE("DropTargetHandler onDragLeave sets DragLeave state", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.onDragEnter({"/assets/hero.png"});
    h.onDragLeave();
    CHECK(h.state()      == DropState::DragLeave);
    CHECK(h.leaveCount() == 1u);
    CHECK(h.hoveredPaths().empty());
}

TEST_CASE("DropTargetHandler onDrop without pipeline returns 0", "[DropTargetHandler]") {
    DropTargetHandler h; // no pipeline
    size_t count = h.onDrop({"/assets/hero.png"});
    CHECK(count == 0u);
    CHECK(h.state()     == DropState::Dropped);
    CHECK(h.dropCount() == 1u);
}

TEST_CASE("DropTargetHandler lastDroppedPaths stores drop paths", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.onDrop({"/a.png", "/b.png"});
    REQUIRE(h.lastDroppedPaths().size() == 2u);
    CHECK(h.lastDroppedPaths()[0] == "/a.png");
}

TEST_CASE("DropTargetHandler reset returns to Idle", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.onDragEnter({"/assets/hero.png"});
    h.reset();
    CHECK(h.state() == DropState::Idle);
    CHECK(h.hoveredPaths().empty());
}

TEST_CASE("DropTargetHandler setDefaultEffect changes effect", "[DropTargetHandler]") {
    DropTargetHandler h;
    h.setDefaultEffect(DropEffect::Link);
    CHECK(h.defaultEffect() == DropEffect::Link);
    auto eff = h.onDragEnter({"/assets/hero.png"});
    CHECK(eff == DropEffect::Link);
}

// ─────────────────────────────────────────────────────────────────
// 7. Integration
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ContentRouter integration: multi-type routing pipeline", "[ContentRouterIntegration]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::UseDefault);
    router.setDefaultToolId("asset_editor");

    router.addRule(makeRule("texture_rule", "material_editor", AssetTypeTag::Texture, 5));
    router.addRule(makeRule("mesh_rule",    "scene_editor",    AssetTypeTag::Mesh,    5));
    router.addRule(makeRule("script_rule",  "code_editor",     AssetTypeTag::Script,  5));

    CHECK(router.route(AssetTypeTag::Texture).toolId == "material_editor");
    CHECK(router.route(AssetTypeTag::Mesh).toolId    == "scene_editor");
    CHECK(router.route(AssetTypeTag::Script).toolId  == "code_editor");
    // Unknown → falls to default
    CHECK(router.route(AssetTypeTag::Unknown).toolId == "asset_editor");

    CHECK(router.routeCount() == 4u);
}

TEST_CASE("ContentRouter integration: disable and fallback to default", "[ContentRouterIntegration]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::UseDefault);
    router.setDefaultToolId("asset_editor");
    router.addRule(makeRule("tex", "material_editor", AssetTypeTag::Texture, 5));

    // Disable the specific rule
    router.enableRule("tex", false);

    // Now should fall to default
    auto r = router.route(AssetTypeTag::Texture);
    CHECK(r.toolId == "asset_editor");
}

TEST_CASE("DropTargetHandler integration: enter → hover → drop sequence", "[DropTargetHandlerIntegration]") {
    DropTargetHandler h;
    h.setAcceptUnknown(true); // accept all files for simplicity

    // Drag enter
    auto enterEff = h.onDragEnter({"/assets/character.fbx"});
    CHECK(enterEff != DropEffect::None);
    CHECK(h.state() == DropState::DragOver);

    // Hover
    h.onDragOver(150.f, 300.f);
    CHECK(h.state() == DropState::DragOver);

    // Drop (no pipeline → 0 ingested, state still changes)
    h.onDrop({"/assets/character.fbx"});
    CHECK(h.state()     == DropState::Dropped);
    CHECK(h.dropCount() == 1u);
    CHECK(h.enterCount() == 1u);
}
