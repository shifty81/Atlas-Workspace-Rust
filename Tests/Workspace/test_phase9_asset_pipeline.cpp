// Tests/Workspace/test_phase9_asset_pipeline.cpp
// Phase 9 — Asset Pipeline and Content Routing
//
// Tests for:
//   1. AssetCatalog      — descriptor CRUD, query, state mutations, metadata
//   2. AssetTransformer  — step chain, skip/error handling, type routing, stats
//   3. ContentRouter     — rule matching, priority ordering, policy, intake routing
//   4. AssetWatcher      — watch management, debounce, event delivery, callbacks
//   5. Integration       — FileIntakePipeline → ContentRouter → AssetCatalog

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/AssetTransformer.h"
#include "NF/Workspace/ContentRouter.h"
#include "NF/Workspace/AssetWatcher.h"
#include "NF/Workspace/FileIntakePipeline.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — AssetCatalog
// ═════════════════════════════════════════════════════════════════

TEST_CASE("assetTypeTagName returns correct strings", "[Phase9][AssetCatalog]") {
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Unknown))   == "Unknown");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Texture))   == "Texture");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Mesh))      == "Mesh");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Audio))     == "Audio");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Script))    == "Script");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Shader))    == "Shader");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Scene))     == "Scene");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Font))      == "Font");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Material))  == "Material");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Animation)) == "Animation");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Prefab))    == "Prefab");
    CHECK(std::string(assetTypeTagName(AssetTypeTag::Custom))    == "Custom");
}

TEST_CASE("assetImportStateName returns correct strings", "[Phase9][AssetCatalog]") {
    CHECK(std::string(assetImportStateName(AssetImportState::Unknown))   == "Unknown");
    CHECK(std::string(assetImportStateName(AssetImportState::Staged))    == "Staged");
    CHECK(std::string(assetImportStateName(AssetImportState::Importing)) == "Importing");
    CHECK(std::string(assetImportStateName(AssetImportState::Imported))  == "Imported");
    CHECK(std::string(assetImportStateName(AssetImportState::Dirty))     == "Dirty");
    CHECK(std::string(assetImportStateName(AssetImportState::Error))     == "Error");
    CHECK(std::string(assetImportStateName(AssetImportState::Excluded))  == "Excluded");
}

TEST_CASE("AssetMetadata set/get/has/remove", "[Phase9][AssetCatalog]") {
    AssetMetadata m;
    CHECK(m.empty());
    m.set("width", "1024");
    m.set("height", "512");
    CHECK(m.count() == 2);
    CHECK(m.has("width"));
    REQUIRE(m.get("width") != nullptr);
    CHECK(*m.get("width") == "1024");
    CHECK(m.getOr("missing", "default") == "default");

    // Overwrite
    m.set("width", "2048");
    CHECK(*m.get("width") == "2048");

    CHECK(m.remove("height"));
    CHECK(m.count() == 1);
    CHECK_FALSE(m.has("height"));

    m.clear();
    CHECK(m.empty());
}

TEST_CASE("AssetDescriptor validity checks", "[Phase9][AssetCatalog]") {
    AssetDescriptor d;
    CHECK_FALSE(d.isValid()); // id=0 and empty paths

    d.id          = 1;
    d.sourcePath  = "/src/rock.png";
    d.catalogPath = "Textures/rock.png";
    d.typeTag     = AssetTypeTag::Texture;
    CHECK(d.isValid());
    CHECK(d.extension() == ".png");
    CHECK_FALSE(d.isImported());
    CHECK(d.needsReimport()); // Unknown state → needs reimport
}

TEST_CASE("AssetDescriptor isImported and needsReimport states", "[Phase9][AssetCatalog]") {
    AssetDescriptor d;
    d.id = 1; d.sourcePath = "/f.fbx"; d.catalogPath = "M/f.fbx";
    d.typeTag = AssetTypeTag::Mesh;

    d.importState = AssetImportState::Imported;
    CHECK(d.isImported());
    CHECK_FALSE(d.needsReimport());

    d.importState = AssetImportState::Dirty;
    CHECK_FALSE(d.isImported());
    CHECK(d.needsReimport());

    d.importState = AssetImportState::Error;
    CHECK(d.needsReimport());

    d.importState = AssetImportState::Excluded;
    CHECK_FALSE(d.needsReimport());
}

TEST_CASE("AssetCatalog add and find", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    CHECK(cat.empty());

    AssetDescriptor d;
    d.sourcePath  = "/src/rock.png";
    d.catalogPath = "Textures/rock.png";
    d.typeTag     = AssetTypeTag::Texture;

    AssetId id = cat.add(d);
    CHECK(id != INVALID_ASSET_ID);
    CHECK(cat.count() == 1);
    CHECK_FALSE(cat.empty());

    const auto* found = cat.find(id);
    REQUIRE(found != nullptr);
    CHECK(found->catalogPath == "Textures/rock.png");
    CHECK(cat.contains(id));
}

TEST_CASE("AssetCatalog findByPath works", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "/src/rock.png"; d.catalogPath = "Textures/rock.png";
    d.typeTag = AssetTypeTag::Texture;
    cat.add(d);

    const auto* found = cat.findByPath("Textures/rock.png");
    REQUIRE(found != nullptr);
    CHECK(found->typeTag == AssetTypeTag::Texture);

    CHECK(cat.findByPath("missing/path") == nullptr);
}

TEST_CASE("AssetCatalog rejects duplicate catalogPath", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "/src/a.png"; d.catalogPath = "T/a.png"; d.typeTag = AssetTypeTag::Texture;
    AssetId id1 = cat.add(d);
    CHECK(id1 != INVALID_ASSET_ID);

    // Same catalog path
    d.sourcePath = "/src/b.png";
    AssetId id2 = cat.add(d);
    CHECK(id2 == INVALID_ASSET_ID);
    CHECK(cat.count() == 1);
}

TEST_CASE("AssetCatalog rejects invalid descriptor", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor bad; // empty sourcePath + Unknown tag
    CHECK(cat.add(bad) == INVALID_ASSET_ID);

    AssetDescriptor noType;
    noType.sourcePath = "/x.png"; noType.catalogPath = "x.png";
    noType.typeTag = AssetTypeTag::Unknown;
    CHECK(cat.add(noType) == INVALID_ASSET_ID);
}

TEST_CASE("AssetCatalog remove works", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "/x.png"; d.catalogPath = "x.png"; d.typeTag = AssetTypeTag::Texture;
    AssetId id = cat.add(d);

    CHECK(cat.remove(id));
    CHECK(cat.count() == 0);
    CHECK(cat.find(id) == nullptr);
    CHECK_FALSE(cat.remove(id)); // already removed
}

TEST_CASE("AssetCatalog setImportState and setImportError", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "/x.png"; d.catalogPath = "x.png"; d.typeTag = AssetTypeTag::Texture;
    AssetId id = cat.add(d);

    CHECK(cat.setImportState(id, AssetImportState::Imported));
    CHECK(cat.find(id)->importState == AssetImportState::Imported);

    CHECK(cat.setImportError(id, "decode failed"));
    const auto* desc = cat.find(id);
    CHECK(desc->importState == AssetImportState::Error);
    CHECK(desc->importError == "decode failed");

    CHECK_FALSE(cat.setImportState(99999, AssetImportState::Imported));
}

TEST_CASE("AssetCatalog markDirty", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "/x.png"; d.catalogPath = "x.png"; d.typeTag = AssetTypeTag::Texture;
    d.importState = AssetImportState::Imported;
    AssetId id = cat.add(d);

    CHECK(cat.markDirty(id));
    CHECK(cat.find(id)->importState == AssetImportState::Dirty);
}

TEST_CASE("AssetCatalog setMetadata via catalog", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "/x.png"; d.catalogPath = "x.png"; d.typeTag = AssetTypeTag::Texture;
    AssetId id = cat.add(d);

    CHECK(cat.setMetadata(id, "width", "2048"));
    const auto* desc = cat.find(id);
    REQUIRE(desc != nullptr);
    CHECK(desc->metadata.has("width"));
    CHECK(*desc->metadata.get("width") == "2048");
}

TEST_CASE("AssetCatalog countByState", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    auto addAsset = [&](const std::string& path, AssetImportState state) {
        AssetDescriptor d;
        d.sourcePath = path; d.catalogPath = path; d.typeTag = AssetTypeTag::Texture;
        d.importState = state;
        cat.add(d);
    };
    addAsset("a.png", AssetImportState::Imported);
    addAsset("b.png", AssetImportState::Imported);
    addAsset("c.png", AssetImportState::Dirty);

    CHECK(cat.countByState(AssetImportState::Imported) == 2);
    CHECK(cat.countByState(AssetImportState::Dirty)    == 1);
    CHECK(cat.countByState(AssetImportState::Error)    == 0);
}

TEST_CASE("AssetCatalog countByType", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    auto addAsset = [&](const std::string& path, AssetTypeTag tag) {
        AssetDescriptor d;
        d.sourcePath = path; d.catalogPath = path; d.typeTag = tag;
        cat.add(d);
    };
    addAsset("a.png", AssetTypeTag::Texture);
    addAsset("b.png", AssetTypeTag::Texture);
    addAsset("c.fbx", AssetTypeTag::Mesh);

    CHECK(cat.countByType(AssetTypeTag::Texture) == 2);
    CHECK(cat.countByType(AssetTypeTag::Mesh)    == 1);
    CHECK(cat.countByType(AssetTypeTag::Audio)   == 0);
}

TEST_CASE("AssetCatalog query with predicate", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    for (int i = 0; i < 4; ++i) {
        AssetDescriptor d;
        d.sourcePath = "path" + std::to_string(i); d.catalogPath = d.sourcePath;
        d.typeTag = (i < 2) ? AssetTypeTag::Texture : AssetTypeTag::Mesh;
        cat.add(d);
    }
    auto textures = cat.query([](const AssetDescriptor& d) {
        return d.typeTag == AssetTypeTag::Texture;
    });
    CHECK(textures.size() == 2);
}

TEST_CASE("AssetCatalog all() returns all assets", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    for (int i = 0; i < 5; ++i) {
        AssetDescriptor d;
        d.sourcePath = "p" + std::to_string(i); d.catalogPath = d.sourcePath;
        d.typeTag = AssetTypeTag::Script;
        cat.add(d);
    }
    CHECK(cat.all().size() == 5);
}

TEST_CASE("AssetCatalog clear() empties catalog", "[Phase9][AssetCatalog]") {
    AssetCatalog cat;
    AssetDescriptor d;
    d.sourcePath = "x"; d.catalogPath = "x"; d.typeTag = AssetTypeTag::Texture;
    cat.add(d);
    cat.clear();
    CHECK(cat.empty());
    CHECK(cat.count() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — AssetTransformer
// ═════════════════════════════════════════════════════════════════

TEST_CASE("transformStepStatusName returns correct strings", "[Phase9][AssetTransformer]") {
    CHECK(std::string(transformStepStatusName(TransformStepStatus::Ok))    == "Ok");
    CHECK(std::string(transformStepStatusName(TransformStepStatus::Skip))  == "Skip");
    CHECK(std::string(transformStepStatusName(TransformStepStatus::Error)) == "Error");
}

TEST_CASE("TransformStepResult factory methods", "[Phase9][AssetTransformer]") {
    auto ok   = TransformStepResult::ok("done");
    auto skip = TransformStepResult::skip("not applicable");
    auto err  = TransformStepResult::error("bad format");

    CHECK(ok.succeeded());
    CHECK_FALSE(ok.isError());
    CHECK_FALSE(ok.wasSkipped());

    CHECK(skip.succeeded());
    CHECK(skip.wasSkipped());

    CHECK_FALSE(err.succeeded());
    CHECK(err.isError());
    CHECK(err.message == "bad format");
}

TEST_CASE("TransformContext validity", "[Phase9][AssetTransformer]") {
    TransformContext ctx;
    CHECK_FALSE(ctx.isValid());

    ctx.assetId    = 1;
    ctx.sourcePath = "/src/rock.png";
    ctx.typeTag    = AssetTypeTag::Texture;
    CHECK(ctx.isValid());
}

TEST_CASE("TransformContext scratch data set/get", "[Phase9][AssetTransformer]") {
    TransformContext ctx;
    ctx.setScratch("decoded", "true");
    REQUIRE(ctx.getScratch("decoded") != nullptr);
    CHECK(*ctx.getScratch("decoded") == "true");

    // Overwrite
    ctx.setScratch("decoded", "false");
    CHECK(*ctx.getScratch("decoded") == "false");

    CHECK(ctx.getScratch("missing") == nullptr);
}

TEST_CASE("TransformStep validity", "[Phase9][AssetTransformer]") {
    TransformStep s;
    CHECK_FALSE(s.isValid()); // empty name and null fn

    s.name = "decode";
    CHECK_FALSE(s.isValid()); // still no fn

    s.fn = [](TransformContext&) { return TransformStepResult::ok(); };
    CHECK(s.isValid());
}

TEST_CASE("TransformChain addStep and hasStep", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    TransformStep s;
    s.name = "decode";
    s.fn   = [](TransformContext&) { return TransformStepResult::ok(); };

    CHECK(chain.addStep(s));
    CHECK(chain.stepCount() == 1);
    CHECK(chain.hasStep("decode"));
    CHECK_FALSE(chain.addStep(s)); // duplicate
}

TEST_CASE("TransformChain rejects invalid step", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    TransformStep bad; // no name or fn
    CHECK_FALSE(chain.addStep(bad));
    CHECK(chain.stepCount() == 0);
}

TEST_CASE("TransformChain removeStep works", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    TransformStep s;
    s.name = "step1";
    s.fn   = [](TransformContext&) { return TransformStepResult::ok(); };
    chain.addStep(s);
    CHECK(chain.removeStep("step1"));
    CHECK(chain.stepCount() == 0);
    CHECK_FALSE(chain.removeStep("step1"));
}

TEST_CASE("TransformChain run executes all steps in order", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    std::vector<std::string> log;

    auto makeStep = [&](const std::string& name) -> TransformStep {
        TransformStep s;
        s.name = name;
        s.fn   = [&log, name](TransformContext& ctx) {
            log.push_back(name);
            ctx.progress += 0.25f;
            return TransformStepResult::ok();
        };
        return s;
    };

    chain.addStep(makeStep("a"));
    chain.addStep(makeStep("b"));
    chain.addStep(makeStep("c"));

    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.png"; ctx.typeTag = AssetTypeTag::Texture;
    auto result = chain.run(ctx);

    CHECK(result.succeeded);
    CHECK(result.stepsRun == 3);
    CHECK(log == std::vector<std::string>{"a", "b", "c"});
    CHECK(ctx.progress == Catch::Approx(0.75f));
}

TEST_CASE("TransformChain skip step does not abort chain", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    int afterCount = 0;

    TransformStep skip;
    skip.name = "optional";
    skip.fn   = [](TransformContext&) { return TransformStepResult::skip("not needed"); };
    chain.addStep(skip);

    TransformStep after;
    after.name = "final";
    after.fn   = [&](TransformContext&) { ++afterCount; return TransformStepResult::ok(); };
    chain.addStep(after);

    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.png"; ctx.typeTag = AssetTypeTag::Texture;
    auto result = chain.run(ctx);

    CHECK(result.succeeded);
    CHECK(result.stepsSkipped == 1);
    CHECK(afterCount == 1);
}

TEST_CASE("TransformChain aborts on error step", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    bool afterRan = false;

    TransformStep fail;
    fail.name = "fail_step";
    fail.fn   = [](TransformContext&) { return TransformStepResult::error("format unsupported"); };
    chain.addStep(fail);

    TransformStep after;
    after.name = "after";
    after.fn   = [&](TransformContext&) { afterRan = true; return TransformStepResult::ok(); };
    chain.addStep(after);

    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.png"; ctx.typeTag = AssetTypeTag::Texture;
    auto result = chain.run(ctx);

    CHECK_FALSE(result.succeeded);
    CHECK(result.errorStep    == "fail_step");
    CHECK(result.errorMessage == "format unsupported");
    CHECK_FALSE(afterRan);
}

TEST_CASE("TransformChain disabled step is skipped", "[Phase9][AssetTransformer]") {
    TransformChain chain;
    bool ranDisabled = false;

    TransformStep s;
    s.name    = "disabled";
    s.enabled = false;
    s.fn      = [&](TransformContext&) { ranDisabled = true; return TransformStepResult::ok(); };
    chain.addStep(s);

    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.png"; ctx.typeTag = AssetTypeTag::Texture;
    auto result = chain.run(ctx);

    CHECK(result.succeeded);
    CHECK_FALSE(ranDisabled);
    CHECK(result.stepsRun == 0); // disabled steps not counted as "run"
}

TEST_CASE("AssetTransformer routes by type and tracks stats", "[Phase9][AssetTransformer]") {
    AssetTransformer xfm;
    int textureCalls = 0;

    TransformChain texChain;
    TransformStep s;
    s.name = "tex_decode";
    s.fn   = [&](TransformContext&) { ++textureCalls; return TransformStepResult::ok(); };
    texChain.addStep(s);
    xfm.registerChain(AssetTypeTag::Texture, std::move(texChain));

    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.png"; ctx.typeTag = AssetTypeTag::Texture;
    auto result = xfm.transform(ctx);

    CHECK(result.succeeded);
    CHECK(textureCalls == 1);
    CHECK(xfm.totalTransforms() == 1);
    CHECK(xfm.totalSucceeded()  == 1);
    CHECK(xfm.totalFailed()     == 0);
}

TEST_CASE("AssetTransformer uses default chain when no type-specific chain",
          "[Phase9][AssetTransformer]") {
    AssetTransformer xfm;
    bool defaultCalled = false;

    TransformChain def;
    TransformStep s;
    s.name = "default_step";
    s.fn   = [&](TransformContext&) { defaultCalled = true; return TransformStepResult::ok(); };
    def.addStep(s);
    xfm.setDefaultChain(std::move(def));

    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.fbx"; ctx.typeTag = AssetTypeTag::Mesh;
    auto result = xfm.transform(ctx);

    CHECK(result.succeeded);
    CHECK(defaultCalled);
}

TEST_CASE("AssetTransformer fails when no chain and no default", "[Phase9][AssetTransformer]") {
    AssetTransformer xfm;
    TransformContext ctx;
    ctx.assetId = 1; ctx.sourcePath = "/x.fbx"; ctx.typeTag = AssetTypeTag::Mesh;
    auto result = xfm.transform(ctx);
    CHECK_FALSE(result.succeeded);
    CHECK(result.errorStep == "(router)");
}

TEST_CASE("AssetTransformer fails on invalid context", "[Phase9][AssetTransformer]") {
    AssetTransformer xfm;
    TransformContext bad; // assetId=0
    auto result = xfm.transform(bad);
    CHECK_FALSE(result.succeeded);
    CHECK(result.errorStep == "(validate)");
}

TEST_CASE("AssetTransformer hasChainFor", "[Phase9][AssetTransformer]") {
    AssetTransformer xfm;
    CHECK_FALSE(xfm.hasChainFor(AssetTypeTag::Texture));
    xfm.registerChain(AssetTypeTag::Texture, TransformChain{});
    CHECK(xfm.hasChainFor(AssetTypeTag::Texture));
    CHECK(xfm.chainCount() == 1);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — ContentRouter
// ═════════════════════════════════════════════════════════════════

TEST_CASE("contentRouterPolicyName returns correct strings", "[Phase9][ContentRouter]") {
    CHECK(std::string(contentRouterPolicyName(ContentRouterPolicy::Reject))     == "Reject");
    CHECK(std::string(contentRouterPolicyName(ContentRouterPolicy::UseDefault)) == "UseDefault");
    CHECK(std::string(contentRouterPolicyName(ContentRouterPolicy::Prompt))     == "Prompt");
}

TEST_CASE("RoutingRule validity and matching", "[Phase9][ContentRouter]") {
    RoutingRule r;
    r.name   = "tex_rule";
    r.toolId = "texture.editor";
    r.typeTag = AssetTypeTag::Texture;
    CHECK(r.isValid());
    CHECK(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
    CHECK_FALSE(r.matches(AssetTypeTag::Mesh, IntakeSource::FileDrop));

    // Disabled rule never matches
    r.enabled = false;
    CHECK_FALSE(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
}

TEST_CASE("RoutingRule wildcard type (Unknown) matches any type", "[Phase9][ContentRouter]") {
    RoutingRule r;
    r.name = "wildcard"; r.toolId = "generic";
    r.typeTag = AssetTypeTag::Unknown; // wildcard
    CHECK(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
    CHECK(r.matches(AssetTypeTag::Mesh,    IntakeSource::FileDrop));
    CHECK(r.matches(AssetTypeTag::Audio,   IntakeSource::CLI));
}

TEST_CASE("RoutingRule source filter restricts matching", "[Phase9][ContentRouter]") {
    RoutingRule r;
    r.name = "drop_only"; r.toolId = "importer";
    r.typeTag = AssetTypeTag::Texture;
    r.filterBySource = true;
    r.sourceFilter   = IntakeSource::FileDrop;
    CHECK(r.matches(AssetTypeTag::Texture, IntakeSource::FileDrop));
    CHECK_FALSE(r.matches(AssetTypeTag::Texture, IntakeSource::FileDialog));
}

TEST_CASE("ContentRouter addRule and route basic", "[Phase9][ContentRouter]") {
    ContentRouter router;
    RoutingRule r;
    r.name = "tex"; r.toolId = "texture.editor"; r.typeTag = AssetTypeTag::Texture;
    CHECK(router.addRule(r));
    CHECK(router.ruleCount() == 1);

    auto result = router.route(AssetTypeTag::Texture);
    CHECK(result.matched);
    CHECK(result.toolId   == "texture.editor");
    CHECK(result.ruleName == "tex");
    CHECK(router.routeCount() == 1);
}

TEST_CASE("ContentRouter miss with Reject policy", "[Phase9][ContentRouter]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::Reject);
    auto result = router.route(AssetTypeTag::Texture);
    CHECK_FALSE(result.matched);
    CHECK_FALSE(result.needsPrompt);
    CHECK(router.missCount() == 1);
}

TEST_CASE("ContentRouter miss with UseDefault policy routes to default tool",
          "[Phase9][ContentRouter]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::UseDefault);
    router.setDefaultToolId("generic.importer");

    auto result = router.route(AssetTypeTag::Mesh); // no rule for Mesh
    CHECK(result.matched);
    CHECK(result.toolId   == "generic.importer");
    CHECK(result.ruleName == "default");
}

TEST_CASE("ContentRouter miss with Prompt policy sets needsPrompt",
          "[Phase9][ContentRouter]") {
    ContentRouter router;
    router.setPolicy(ContentRouterPolicy::Prompt);

    auto result = router.route(AssetTypeTag::Unknown);
    CHECK_FALSE(result.matched);
    CHECK(result.needsPrompt);
}

TEST_CASE("ContentRouter priority ordering — higher priority wins", "[Phase9][ContentRouter]") {
    ContentRouter router;

    RoutingRule low;
    low.name = "low"; low.toolId = "tool.low"; low.typeTag = AssetTypeTag::Texture;
    low.priority = 0;

    RoutingRule high;
    high.name = "high"; high.toolId = "tool.high"; high.typeTag = AssetTypeTag::Texture;
    high.priority = 10;

    // Add low first, then high — high should win
    router.addRule(low);
    router.addRule(high);

    auto result = router.route(AssetTypeTag::Texture);
    CHECK(result.toolId == "tool.high");
}

TEST_CASE("ContentRouter removeRule works", "[Phase9][ContentRouter]") {
    ContentRouter router;
    RoutingRule r;
    r.name = "r1"; r.toolId = "t"; r.typeTag = AssetTypeTag::Texture;
    router.addRule(r);
    CHECK(router.removeRule("r1"));
    CHECK(router.ruleCount() == 0);
    CHECK_FALSE(router.removeRule("r1"));
}

TEST_CASE("ContentRouter enableRule disables matching", "[Phase9][ContentRouter]") {
    ContentRouter router;
    RoutingRule r;
    r.name = "tex"; r.toolId = "t"; r.typeTag = AssetTypeTag::Texture;
    router.addRule(r);

    router.enableRule("tex", false);
    router.setPolicy(ContentRouterPolicy::Reject);
    auto result = router.route(AssetTypeTag::Texture);
    CHECK_FALSE(result.matched); // rule disabled
}

TEST_CASE("ContentRouter hasRule", "[Phase9][ContentRouter]") {
    ContentRouter router;
    CHECK_FALSE(router.hasRule("tex"));
    RoutingRule r; r.name = "tex"; r.toolId = "t"; r.typeTag = AssetTypeTag::Texture;
    router.addRule(r);
    CHECK(router.hasRule("tex"));
}

TEST_CASE("ContentRouter route by AssetDescriptor", "[Phase9][ContentRouter]") {
    ContentRouter router;
    RoutingRule r; r.name = "mesh"; r.toolId = "mesh.editor"; r.typeTag = AssetTypeTag::Mesh;
    router.addRule(r);

    AssetDescriptor d;
    d.id = 1; d.sourcePath = "/x.fbx"; d.catalogPath = "x.fbx";
    d.typeTag = AssetTypeTag::Mesh;
    auto result = router.route(d);
    CHECK(result.matched);
    CHECK(result.toolId == "mesh.editor");
}

TEST_CASE("ContentRouter route by IntakeItem maps fileType to typeTag",
          "[Phase9][ContentRouter]") {
    ContentRouter router;
    RoutingRule r; r.name = "audio"; r.toolId = "audio.editor"; r.typeTag = AssetTypeTag::Audio;
    router.addRule(r);

    IntakeItem item;
    item.id       = 1;
    item.path     = "/sound.wav";
    item.source   = IntakeSource::FileDrop;
    item.fileType = IntakeFileType::Audio;
    auto result = router.route(item);
    CHECK(result.matched);
    CHECK(result.toolId == "audio.editor");
}

TEST_CASE("ContentRouter clearRules empties the routing table", "[Phase9][ContentRouter]") {
    ContentRouter router;
    RoutingRule r; r.name = "r"; r.toolId = "t"; r.typeTag = AssetTypeTag::Texture;
    router.addRule(r);
    router.clearRules();
    CHECK(router.ruleCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — AssetWatcher
// ═════════════════════════════════════════════════════════════════

TEST_CASE("changeTypeName returns correct strings", "[Phase9][AssetWatcher]") {
    CHECK(std::string(changeTypeName(ChangeType::Created))  == "Created");
    CHECK(std::string(changeTypeName(ChangeType::Modified)) == "Modified");
    CHECK(std::string(changeTypeName(ChangeType::Deleted))  == "Deleted");
    CHECK(std::string(changeTypeName(ChangeType::Renamed))  == "Renamed");
}

TEST_CASE("ChangeEvent validity", "[Phase9][AssetWatcher]") {
    ChangeEvent ev;
    CHECK_FALSE(ev.isValid()); // watchId=0

    ev.watchId = 1; ev.path = "/x.png";
    CHECK(ev.isValid());
}

TEST_CASE("AssetWatcher addWatch and isWatching", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    uint32_t id = w.addWatch("/assets/textures");
    CHECK(id != 0);
    CHECK(w.watchCount() == 1);
    CHECK(w.isWatching("/assets/textures"));
}

TEST_CASE("AssetWatcher addWatch duplicate returns existing id", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    uint32_t id1 = w.addWatch("/path");
    uint32_t id2 = w.addWatch("/path"); // same path
    CHECK(id1 == id2);
    CHECK(w.watchCount() == 1);
}

TEST_CASE("AssetWatcher addWatch empty path returns 0", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    CHECK(w.addWatch("") == 0);
    CHECK(w.watchCount() == 0);
}

TEST_CASE("AssetWatcher removeWatch works", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    uint32_t id = w.addWatch("/path");
    CHECK(w.removeWatch(id));
    CHECK(w.watchCount() == 0);
    CHECK_FALSE(w.removeWatch(id));
}

TEST_CASE("AssetWatcher removeWatchByPath works", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path/a");
    w.addWatch("/path/b");
    CHECK(w.removeWatchByPath("/path/a"));
    CHECK(w.watchCount() == 1);
    CHECK_FALSE(w.isWatching("/path/a"));
}

TEST_CASE("AssetWatcher enableWatch disables event delivery", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    uint32_t id = w.addWatch("/path");
    w.enableWatch(id, false);

    bool called = false;
    w.subscribe([&](const ChangeEvent&) { called = true; });

    CHECK_FALSE(w.notifyChanged("/path", ChangeType::Modified, 0));
    w.tick(200); // past debounce
    CHECK_FALSE(called);
}

TEST_CASE("AssetWatcher notifyChanged queues pending event", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path");
    CHECK(w.notifyChanged("/path", ChangeType::Modified, 100));
    CHECK(w.pendingCount() == 1);
}

TEST_CASE("AssetWatcher notifyChanged ignores unregistered path", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    CHECK_FALSE(w.notifyChanged("/not/watched", ChangeType::Modified, 100));
    CHECK(w.pendingCount() == 0);
}

TEST_CASE("AssetWatcher tick delivers event after debounce window", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path");

    std::vector<ChangeEvent> received;
    w.subscribe([&](const ChangeEvent& ev) { received.push_back(ev); });

    w.notifyChanged("/path", ChangeType::Modified, 0);
    CHECK(w.pendingCount() == 1);

    // Tick before debounce — should not deliver
    w.tick(50, 100); // 50ms < 100ms debounce
    CHECK(received.empty());
    CHECK(w.pendingCount() == 1);

    // Tick after debounce — should deliver
    w.tick(150, 100); // 150 >= 0 + 100
    CHECK(received.size() == 1);
    CHECK(received[0].path == "/path");
    CHECK(received[0].type == ChangeType::Modified);
    CHECK(w.pendingCount() == 0);
    CHECK(w.totalDelivered() == 1);
}

TEST_CASE("AssetWatcher deduplicates same path+type in pending queue",
          "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path");
    w.notifyChanged("/path", ChangeType::Modified, 0);
    w.notifyChanged("/path", ChangeType::Modified, 10); // same type — should reset debounce
    CHECK(w.pendingCount() == 1); // still only one entry
}

TEST_CASE("AssetWatcher recursive watch matches sub-paths", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/assets", /*recursive=*/true);

    bool called = false;
    w.subscribe([&](const ChangeEvent&) { called = true; });

    CHECK(w.notifyChanged("/assets/textures/rock.png", ChangeType::Modified, 0));
    w.tick(200, 100);
    CHECK(called);
}

TEST_CASE("AssetWatcher non-recursive watch does not match sub-paths",
          "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/assets", /*recursive=*/false);
    // Sub-path should NOT match for non-recursive watch
    bool called = false;
    w.subscribe([&](const ChangeEvent&) { called = true; });
    CHECK_FALSE(w.notifyChanged("/assets/textures/rock.png", ChangeType::Modified, 0));
    CHECK_FALSE(called);
}

TEST_CASE("AssetWatcher subscribe multiple callbacks", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path");
    int count1 = 0, count2 = 0;
    w.subscribe([&](const ChangeEvent&) { ++count1; });
    w.subscribe([&](const ChangeEvent&) { ++count2; });

    w.notifyChanged("/path", ChangeType::Modified, 0);
    w.tick(200, 100);
    CHECK(count1 == 1);
    CHECK(count2 == 1);
}

TEST_CASE("AssetWatcher clearCallbacks removes all subscribers", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path");
    bool called = false;
    w.subscribe([&](const ChangeEvent&) { called = true; });
    w.clearCallbacks();

    w.notifyChanged("/path", ChangeType::Modified, 0);
    w.tick(200, 100);
    CHECK_FALSE(called);
}

TEST_CASE("AssetWatcher clearPending removes pending events", "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    w.addWatch("/path");
    w.notifyChanged("/path", ChangeType::Modified, 0);
    CHECK(w.pendingCount() == 1);
    w.clearPending();
    CHECK(w.pendingCount() == 0);
}

TEST_CASE("AssetWatcher WatchEntry tracks eventCount after delivery",
          "[Phase9][AssetWatcher]") {
    AssetWatcher w;
    uint32_t id = w.addWatch("/path");
    w.subscribe([](const ChangeEvent&){});

    w.notifyChanged("/path", ChangeType::Modified, 0);
    w.tick(200, 100);

    const auto* entry = w.findWatch(id);
    REQUIRE(entry != nullptr);
    CHECK(entry->eventCount == 1);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration: FileIntakePipeline → ContentRouter → AssetCatalog
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: intake → route → catalog add", "[Phase9][Integration]") {
    FileIntakePipeline pipeline;
    ContentRouter router;
    AssetCatalog catalog;

    // Route textures to a texture editor tool
    RoutingRule r;
    r.name = "tex"; r.toolId = "texture.editor"; r.typeTag = AssetTypeTag::Texture;
    router.addRule(r);
    router.setPolicy(ContentRouterPolicy::Reject);

    // Ingest a texture file
    bool ingested = pipeline.ingest("/src/grass.png", IntakeSource::FileDrop);
    REQUIRE(ingested);
    REQUIRE(pipeline.pendingCount() == 1);

    // Route the item and register in catalog
    const auto& item = pipeline.pendingItems()[0];
    auto routeResult = router.route(item);
    REQUIRE(routeResult.matched);
    CHECK(routeResult.toolId == "texture.editor");

    // Add to catalog
    AssetDescriptor d;
    d.sourcePath  = item.path;
    d.catalogPath = "Textures/grass.png";
    d.typeTag     = AssetTypeTag::Texture;
    d.importState = AssetImportState::Staged;
    d.metadata.set("routedTo", routeResult.toolId);
    AssetId id = catalog.add(d);
    REQUIRE(id != INVALID_ASSET_ID);
    CHECK(catalog.countByState(AssetImportState::Staged) == 1);

    // Mark as imported after transform
    catalog.setImportState(id, AssetImportState::Imported);
    CHECK(catalog.countByState(AssetImportState::Imported) == 1);
}

TEST_CASE("Integration: transform chain updates catalog metadata", "[Phase9][Integration]") {
    AssetCatalog catalog;
    AssetTransformer xfm;

    // Register a texture chain that writes width/height into context metadata
    TransformChain chain;
    TransformStep decode;
    decode.name = "decode";
    decode.fn   = [](TransformContext& ctx) {
        ctx.metadata.set("width",  "2048");
        ctx.metadata.set("height", "2048");
        ctx.progress = 1.0f;
        return TransformStepResult::ok("decoded");
    };
    chain.addStep(decode);
    xfm.registerChain(AssetTypeTag::Texture, std::move(chain));

    // Add asset to catalog
    AssetDescriptor d;
    d.sourcePath = "/src/grass.png"; d.catalogPath = "T/grass.png";
    d.typeTag = AssetTypeTag::Texture;
    AssetId id = catalog.add(d);

    // Run transform
    TransformContext ctx;
    ctx.assetId    = id;
    ctx.sourcePath = d.sourcePath;
    ctx.typeTag    = d.typeTag;
    auto result    = xfm.transform(ctx);
    REQUIRE(result.succeeded);

    // Commit metadata to catalog
    catalog.setImportState(id, AssetImportState::Imported);
    for (const auto& [k, v] : ctx.metadata.entries()) {
        catalog.setMetadata(id, k, v);
    }

    const auto* desc = catalog.find(id);
    REQUIRE(desc != nullptr);
    CHECK(desc->importState == AssetImportState::Imported);
    CHECK(*desc->metadata.get("width") == "2048");
}

TEST_CASE("Integration: watcher dirties catalog entry on file change",
          "[Phase9][Integration]") {
    AssetCatalog catalog;
    AssetWatcher watcher;

    // Register an asset
    AssetDescriptor d;
    d.sourcePath = "/src/rock.png"; d.catalogPath = "T/rock.png";
    d.typeTag = AssetTypeTag::Texture; d.importState = AssetImportState::Imported;
    AssetId id = catalog.add(d);

    // Watch the source path
    watcher.addWatch("/src/rock.png");
    watcher.subscribe([&](const ChangeEvent& ev) {
        // Mark corresponding catalog entry dirty
        catalog.markDirty(id);
    });

    // Simulate file change
    watcher.notifyChanged("/src/rock.png", ChangeType::Modified, 0);
    watcher.tick(200, 100); // deliver after debounce

    // Catalog entry should now be Dirty
    const auto* desc = catalog.find(id);
    REQUIRE(desc != nullptr);
    CHECK(desc->importState == AssetImportState::Dirty);
    CHECK(desc->needsReimport());
}
