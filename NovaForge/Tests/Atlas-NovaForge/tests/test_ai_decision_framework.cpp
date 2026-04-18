#include <iostream>
#include <cassert>
#include <cstring>
#include <memory>

#include "../engine/ai/ProjectContext.h"
#include "../engine/ai/AIAssetDecisionFramework.h"
#include "../engine/ai/AtlasAICore.h"

using namespace atlas::ai;

// =============================================================
// ProjectContext Tests
// =============================================================

void test_project_context_game_type() {
    ProjectContext ctx;
    
    // Default is unknown
    assert(ctx.GetGameType() == GameType::Unknown);
    
    // Set and verify
    ctx.SetGameType(GameType::RPG);
    assert(ctx.GetGameType() == GameType::RPG);
    
    ctx.SetGameType(GameType::FPS);
    assert(ctx.GetGameType() == GameType::FPS);
    
    std::cout << "[PASS] test_project_context_game_type" << std::endl;
}

void test_project_context_project_info() {
    ProjectContext ctx;
    
    ctx.SetProjectName("TestGame");
    ctx.SetProjectRoot("/path/to/project");
    
    assert(ctx.GetProjectName() == "TestGame");
    assert(ctx.GetProjectRoot() == "/path/to/project");
    
    std::cout << "[PASS] test_project_context_project_info" << std::endl;
}

void test_project_context_asset_inventory() {
    ProjectContext ctx;
    
    // Initially empty
    assert(!ctx.HasAsset("mesh_001"));
    assert(ctx.GetAssetCount(AssetCategory::Mesh3D) == 0);
    
    // Add assets
    ctx.AddAsset("mesh_001", AssetCategory::Mesh3D);
    ctx.AddAsset("mesh_002", AssetCategory::Mesh3D);
    ctx.AddAsset("texture_001", AssetCategory::Texture);
    
    assert(ctx.HasAsset("mesh_001"));
    assert(ctx.HasAsset("mesh_002"));
    assert(ctx.HasAsset("texture_001"));
    assert(!ctx.HasAsset("mesh_003"));
    
    assert(ctx.GetAssetCount(AssetCategory::Mesh3D) == 2);
    assert(ctx.GetAssetCount(AssetCategory::Texture) == 1);
    assert(ctx.GetAssetCount(AssetCategory::Audio) == 0);
    
    assert(ctx.HasAssetCategory(AssetCategory::Mesh3D));
    assert(!ctx.HasAssetCategory(AssetCategory::Audio));
    
    // Verify inventory
    const auto& inventory = ctx.GetInventory();
    assert(inventory.totalAssetCount == 3);
    
    std::cout << "[PASS] test_project_context_asset_inventory" << std::endl;
}

void test_project_context_remove_asset() {
    ProjectContext ctx;
    
    ctx.AddAsset("asset_1", AssetCategory::Character);
    ctx.AddAsset("asset_2", AssetCategory::Character);
    
    assert(ctx.GetAssetCount(AssetCategory::Character) == 2);
    
    ctx.RemoveAsset("asset_1");
    
    assert(!ctx.HasAsset("asset_1"));
    assert(ctx.HasAsset("asset_2"));
    assert(ctx.GetAssetCount(AssetCategory::Character) == 1);
    
    // Removing non-existent asset should be safe
    ctx.RemoveAsset("nonexistent");
    assert(ctx.GetAssetCount(AssetCategory::Character) == 1);
    
    std::cout << "[PASS] test_project_context_remove_asset" << std::endl;
}

void test_project_context_requirements() {
    ProjectContext ctx;
    ctx.SetGameType(GameType::RPG);
    
    // Initially no requirements
    assert(ctx.GetRequirements().empty());
    
    // Add requirement manually
    AssetRequirement req;
    req.category = AssetCategory::Character;
    req.name = "player_char";
    req.description = "Player character model";
    req.priority = 3;
    ctx.AddRequirement(req);
    
    assert(ctx.GetRequirements().size() == 1);
    
    // Clear requirements
    ctx.ClearRequirements();
    assert(ctx.GetRequirements().empty());
    
    // Analyze requirements (auto-generate based on game type)
    ctx.AnalyzeRequirements();
    assert(!ctx.GetRequirements().empty());
    
    std::cout << "[PASS] test_project_context_requirements" << std::endl;
}

void test_project_context_unfulfilled_requirements() {
    ProjectContext ctx;
    ctx.SetGameType(GameType::RPG);
    ctx.AnalyzeRequirements();
    
    // All requirements should be unfulfilled initially
    auto unfulfilled = ctx.GetUnfulfilledRequirements();
    assert(unfulfilled.size() == ctx.GetRequirements().size());
    
    // Add an asset that fulfills a requirement
    ctx.AddAsset("char_001", AssetCategory::Character);
    ctx.ScanAssets(); // This updates fulfillment status
    
    // Should have fewer unfulfilled now
    auto newUnfulfilled = ctx.GetUnfulfilledRequirements();
    assert(newUnfulfilled.size() < unfulfilled.size());
    
    std::cout << "[PASS] test_project_context_unfulfilled_requirements" << std::endl;
}

void test_project_context_suggestions() {
    ProjectContext ctx;
    ctx.SetGameType(GameType::RPG);
    ctx.AnalyzeRequirements();
    
    // Should generate suggestions for unfulfilled requirements
    auto suggestions = ctx.GenerateSuggestions();
    assert(!suggestions.empty());
    
    // Priority suggestions
    auto topSuggestions = ctx.GetPrioritySuggestions(3);
    assert(topSuggestions.size() <= 3);
    
    // Verify suggestions have sources
    for (const auto& s : suggestions) {
        assert(!s.source.empty());
        assert(s.confidence > 0.0);
    }
    
    std::cout << "[PASS] test_project_context_suggestions" << std::endl;
}

void test_project_context_game_type_conversion() {
    // Test string conversions
    assert(ProjectContext::StringToGameType("rpg") == GameType::RPG);
    assert(ProjectContext::StringToGameType("fps") == GameType::FPS);
    assert(ProjectContext::StringToGameType("platformer") == GameType::Platformer);
    assert(ProjectContext::StringToGameType("invalid") == GameType::Unknown);
    
    assert(strcmp(ProjectContext::GameTypeToString(GameType::RPG), "rpg") == 0);
    assert(strcmp(ProjectContext::GameTypeToString(GameType::FPS), "fps") == 0);
    
    std::cout << "[PASS] test_project_context_game_type_conversion" << std::endl;
}

void test_project_context_category_conversion() {
    assert(ProjectContext::StringToCategory("mesh3d") == AssetCategory::Mesh3D);
    assert(ProjectContext::StringToCategory("character") == AssetCategory::Character);
    assert(ProjectContext::StringToCategory("audio") == AssetCategory::Audio);
    assert(ProjectContext::StringToCategory("invalid") == AssetCategory::Unknown);
    
    assert(strcmp(ProjectContext::CategoryToString(AssetCategory::Mesh3D), "mesh3d") == 0);
    assert(strcmp(ProjectContext::CategoryToString(AssetCategory::Character), "character") == 0);
    
    std::cout << "[PASS] test_project_context_category_conversion" << std::endl;
}

void test_project_context_clear() {
    ProjectContext ctx;
    ctx.SetGameType(GameType::RPG);
    ctx.SetProjectName("TestGame");
    ctx.AddAsset("asset_1", AssetCategory::Mesh3D);
    ctx.AnalyzeRequirements();
    
    ctx.Clear();
    
    assert(ctx.GetGameType() == GameType::Unknown);
    assert(ctx.GetProjectName().empty());
    assert(ctx.GetInventory().totalAssetCount == 0);
    assert(ctx.GetRequirements().empty());
    
    std::cout << "[PASS] test_project_context_clear" << std::endl;
}

void test_project_context_json() {
    ProjectContext ctx;
    ctx.SetGameType(GameType::RPG);
    ctx.SetProjectName("TestProject");
    ctx.AddAsset("asset_1", AssetCategory::Mesh3D);
    
    std::string json = ctx.ToJson();
    assert(!json.empty());
    assert(json.find("rpg") != std::string::npos);
    assert(json.find("TestProject") != std::string::npos);
    
    std::cout << "[PASS] test_project_context_json" << std::endl;
}

// =============================================================
// AIAssetDecisionFramework Tests
// =============================================================

void test_decision_framework_config() {
    AIAssetDecisionFramework framework;
    
    AssetDecisionConfig config;
    config.autoApprove = true;
    config.autoApproveThreshold = 0.95;
    config.maxSuggestionsPerRun = 10;
    
    framework.SetConfig(config);
    
    const auto& retrieved = framework.GetConfig();
    assert(retrieved.autoApprove == true);
    assert(retrieved.autoApproveThreshold == 0.95);
    assert(retrieved.maxSuggestionsPerRun == 10);
    
    std::cout << "[PASS] test_decision_framework_config" << std::endl;
}

void test_decision_framework_context() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::Strategy);
    context->SetProjectName("StrategyGame");
    
    framework.SetProjectContext(context);
    
    auto retrieved = framework.GetProjectContext();
    assert(retrieved != nullptr);
    assert(retrieved->GetGameType() == GameType::Strategy);
    assert(retrieved->GetProjectName() == "StrategyGame");
    
    std::cout << "[PASS] test_decision_framework_context" << std::endl;
}

void test_decision_framework_analyze_project() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::RPG);
    context->AnalyzeRequirements();
    
    framework.SetProjectContext(context);
    
    auto decisions = framework.AnalyzeProject();
    assert(!decisions.empty());
    
    // Verify decisions have proper structure
    for (const auto& d : decisions) {
        // Either generates or fetches
        bool hasAction = d.shouldGenerate || d.shouldFetch;
        // Has basic suggestion info
        bool hasInfo = !d.suggestion.name.empty() && d.confidence > 0;
        assert(hasAction || hasInfo);
        assert(!d.suggestion.name.empty());
    }
    
    std::cout << "[PASS] test_decision_framework_analyze_project" << std::endl;
}

void test_decision_framework_analyze_category() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::RPG);
    context->AnalyzeRequirements();
    
    framework.SetProjectContext(context);
    
    auto characterDecisions = framework.AnalyzeCategory(AssetCategory::Character);
    
    for (const auto& d : characterDecisions) {
        assert(d.suggestion.category == AssetCategory::Character);
    }
    
    std::cout << "[PASS] test_decision_framework_analyze_category" << std::endl;
}

void test_decision_framework_is_asset_needed() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::FPS);
    
    framework.SetProjectContext(context);
    
    // Initially all assets are needed
    assert(framework.IsAssetNeeded(AssetCategory::Character));
    assert(framework.IsAssetNeeded(AssetCategory::Mesh3D));
    
    // Add an asset
    context->AddAsset("char_1", AssetCategory::Character);
    
    // Now character is not needed
    assert(!framework.IsAssetNeeded(AssetCategory::Character));
    assert(framework.IsAssetNeeded(AssetCategory::Mesh3D));
    
    std::cout << "[PASS] test_decision_framework_is_asset_needed" << std::endl;
}

void test_decision_framework_recommended_sources() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    framework.SetProjectContext(context);
    
    // Terrain can be generated procedurally
    auto terrainSources = framework.GetRecommendedSources(AssetCategory::Terrain);
    bool hasProcedural = false;
    for (const auto& s : terrainSources) {
        if (s == "procedural") hasProcedural = true;
    }
    assert(hasProcedural);
    
    // 3D meshes should recommend marketplace
    auto meshSources = framework.GetRecommendedSources(AssetCategory::Mesh3D);
    bool hasMarketplace = false;
    for (const auto& s : meshSources) {
        if (s.find("marketplace:") == 0) hasMarketplace = true;
    }
    assert(hasMarketplace);
    
    std::cout << "[PASS] test_decision_framework_recommended_sources" << std::endl;
}

void test_decision_framework_statistics() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::Platformer);
    context->AnalyzeRequirements();
    
    framework.SetProjectContext(context);
    
    // Initially zero
    assert(framework.GetDecisionCount() == 0);
    assert(framework.GetExecutedCount() == 0);
    
    // Analyze creates decisions
    auto decisions = framework.AnalyzeProject();
    assert(framework.GetDecisionCount() > 0);
    
    // Clear statistics
    framework.ClearStatistics();
    assert(framework.GetDecisionCount() == 0);
    
    std::cout << "[PASS] test_decision_framework_statistics" << std::endl;
}

void test_decision_framework_ai_integration() {
    AIAssetDecisionFramework framework;
    AtlasAICore core;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::RPG);
    context->AnalyzeRequirements();
    
    framework.SetProjectContext(context);
    
    // Register with AI core
    framework.RegisterWithAICore(core);
    
    // Should have registered intents
    assert(core.GetIntent("asset.analyze") != nullptr);
    assert(core.GetIntent("asset.suggest") != nullptr);
    assert(core.GetIntent("asset.fetch") != nullptr);
    
    // Set permissions
    core.SetPermissions(static_cast<uint8_t>(AIPermission::ReadState));
    
    // Test analyze intent
    AIRequest analyzeReq;
    analyzeReq.intentName = "asset.analyze";
    auto response = core.ProcessRequest(analyzeReq);
    assert(response.success);
    assert(!response.result.empty());
    
    // Unregister
    framework.UnregisterFromAICore(core);
    assert(core.GetIntent("asset.analyze") == nullptr);
    
    std::cout << "[PASS] test_decision_framework_ai_integration" << std::endl;
}

void test_decision_framework_suggest_intent() {
    AIAssetDecisionFramework framework;
    AtlasAICore core;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::Strategy);
    context->AnalyzeRequirements();
    
    framework.SetProjectContext(context);
    framework.RegisterWithAICore(core);
    
    core.SetPermissions(static_cast<uint8_t>(AIPermission::ReadState));
    
    // Test suggest intent with category
    AIRequest suggestReq;
    suggestReq.intentName = "asset.suggest";
    suggestReq.parameters["category"] = "mesh3d";
    
    auto response = core.ProcessRequest(suggestReq);
    assert(response.success);
    
    // Test suggest without category (should fail)
    AIRequest badReq;
    badReq.intentName = "asset.suggest";
    auto badResponse = core.ProcessRequest(badReq);
    assert(!badResponse.success);
    
    framework.UnregisterFromAICore(core);
    
    std::cout << "[PASS] test_decision_framework_suggest_intent" << std::endl;
}

void test_decision_framework_execute_decision() {
    AIAssetDecisionFramework framework;
    
    auto context = std::make_shared<ProjectContext>();
    context->SetGameType(GameType::Simulation);
    
    framework.SetProjectContext(context);
    
    // Create a procedural generation decision
    AssetDecision decision;
    decision.shouldGenerate = true;
    decision.suggestion.category = AssetCategory::Terrain;
    decision.suggestion.name = "test_terrain";
    
    // Should succeed for procedural generation
    bool result = framework.ExecuteDecision(decision);
    assert(result);
    assert(framework.GetExecutedCount() == 1);
    
    // Empty decision should not execute
    AssetDecision emptyDecision;
    emptyDecision.shouldGenerate = false;
    emptyDecision.shouldFetch = false;
    result = framework.ExecuteDecision(emptyDecision);
    assert(!result);
    
    std::cout << "[PASS] test_decision_framework_execute_decision" << std::endl;
}

void test_decision_framework_no_context() {
    AIAssetDecisionFramework framework;
    
    // Without context, should return empty results
    auto decisions = framework.AnalyzeProject();
    assert(decisions.empty());
    
    assert(!framework.IsAssetNeeded(AssetCategory::Mesh3D));
    
    auto suggestions = framework.GetTopMissingAssets(5);
    assert(suggestions.empty());
    
    std::cout << "[PASS] test_decision_framework_no_context" << std::endl;
}

// =============================================================
// Run All Tests
// =============================================================

void run_ai_decision_framework_tests() {
    std::cout << "\n--- Project Context Tests ---" << std::endl;
    test_project_context_game_type();
    test_project_context_project_info();
    test_project_context_asset_inventory();
    test_project_context_remove_asset();
    test_project_context_requirements();
    test_project_context_unfulfilled_requirements();
    test_project_context_suggestions();
    test_project_context_game_type_conversion();
    test_project_context_category_conversion();
    test_project_context_clear();
    test_project_context_json();
    
    std::cout << "\n--- AI Asset Decision Framework Tests ---" << std::endl;
    test_decision_framework_config();
    test_decision_framework_context();
    test_decision_framework_analyze_project();
    test_decision_framework_analyze_category();
    test_decision_framework_is_asset_needed();
    test_decision_framework_recommended_sources();
    test_decision_framework_statistics();
    test_decision_framework_ai_integration();
    test_decision_framework_suggest_intent();
    test_decision_framework_execute_decision();
    test_decision_framework_no_context();
}
