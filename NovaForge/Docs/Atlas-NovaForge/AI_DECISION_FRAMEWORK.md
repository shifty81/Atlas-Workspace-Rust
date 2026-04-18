# AI Asset Decision Framework

This document describes the AI Asset Decision Framework integrated into Atlas, which provides intelligent, context-aware asset suggestions for game projects.

## Overview

The AI Asset Decision Framework analyzes your project state and generates intelligent suggestions for:
- Missing assets based on game type
- Asset sources (procedural generation vs. marketplace)
- Priority ordering for asset creation/acquisition

## Core Components

### 1. ProjectContext

The `ProjectContext` class provides a comprehensive view of your project for AI decision-making.

```cpp
#include "engine/ai/ProjectContext.h"

using namespace atlas::ai;

ProjectContext ctx;
ctx.SetProjectName("MyRPG");
ctx.SetProjectRoot("/path/to/project");
ctx.SetGameType(GameType::RPG);

// Scan existing assets
ctx.AddAsset("player_model", AssetCategory::Character);
ctx.AddAsset("sword_mesh", AssetCategory::Mesh3D);

// Analyze requirements based on game type
ctx.AnalyzeRequirements();

// Get unfulfilled requirements
auto missing = ctx.GetUnfulfilledRequirements();
for (const auto& req : missing) {
    std::cout << "Missing: " << req.name << std::endl;
}
```

### 2. GameType Classification

The framework supports multiple game type classifications:

| GameType | Description | Typical Asset Needs |
|----------|-------------|---------------------|
| `RPG` | Role-playing game | Characters, NPCs, terrain, animations |
| `FPS` | First-person shooter | Weapons, VFX, level geometry |
| `TPS` | Third-person shooter | Characters, weapons, VFX |
| `Strategy` | Strategy game | Units, terrain tiles, UI |
| `Platformer` | 2D/3D platformer | Sprites, tilesets, animations |
| `Puzzle` | Puzzle game | Puzzle elements, VFX |
| `Simulation` | Simulation/sandbox | 3D models, terrain |
| `Racing` | Racing game | Vehicles, tracks |
| `Adventure` | Adventure game | Characters, environments |
| `Survival` | Survival game | Resources, environments |

### 3. Asset Categories

Assets are classified into categories:

- `Mesh3D` - 3D models
- `Mesh2D` - 2D sprites
- `Texture` - Textures and materials
- `Terrain` - Terrain data
- `Character` - Player characters
- `NPC` - Non-player characters
- `Environment` - Environment objects
- `Prop` - Props and decorations
- `VFX` - Visual effects
- `Audio` - Sound effects
- `Music` - Background music
- `UI` - User interface elements
- `Animation` - Animation data
- `Script` - Gameplay scripts
- `Prefab` - Prefab compositions
- `World` - World/level data

## AIAssetDecisionFramework

The main framework class for generating and executing asset decisions.

### Basic Usage

```cpp
#include "engine/ai/AIAssetDecisionFramework.h"

using namespace atlas::ai;

// Create framework and context
AIAssetDecisionFramework framework;
auto context = std::make_shared<ProjectContext>();

// Configure project
context->SetGameType(GameType::RPG);
context->SetProjectName("MyGame");
context->AnalyzeRequirements();

// Set context
framework.SetProjectContext(context);

// Configure framework
AssetDecisionConfig config;
config.autoApprove = false;
config.autoApproveThreshold = 0.9;
config.maxSuggestionsPerRun = 5;
config.preferProcedural = true;
framework.SetConfig(config);

// Analyze and get decisions
auto decisions = framework.AnalyzeProject();

for (const auto& decision : decisions) {
    std::cout << "Suggestion: " << decision.suggestion.name << std::endl;
    std::cout << "Source: " << decision.suggestion.source << std::endl;
    std::cout << "Confidence: " << (decision.confidence * 100) << "%" << std::endl;
    std::cout << "Reasoning: " << decision.reasoning << std::endl;
}
```

### AI Core Integration

The framework integrates with `AtlasAICore` for intent-based processing:

```cpp
#include "engine/ai/AtlasAICore.h"

AtlasAICore core;
core.SetPermissions(static_cast<uint8_t>(AIPermission::ReadState) |
                    static_cast<uint8_t>(AIPermission::ExecuteCommands));

AIAssetDecisionFramework framework;
framework.SetProjectContext(context);
framework.RegisterWithAICore(core);

// Process analyze intent
AIRequest request;
request.intentName = "asset.analyze";
auto response = core.ProcessRequest(request);

std::cout << response.result << std::endl;
```

### Available AI Intents

| Intent | Description | Required Permission |
|--------|-------------|---------------------|
| `asset.analyze` | Analyze project and suggest missing assets | `ReadState` |
| `asset.suggest` | Get suggestions for a specific category | `ReadState` |
| `asset.fetch` | Fetch an asset from marketplace | `ExecuteCommands` |

## Configuration Options

```cpp
struct AssetDecisionConfig {
    bool autoApprove = false;           // Auto-approve decisions above threshold
    double autoApproveThreshold = 0.9;  // Confidence threshold for auto-approval
    uint32_t maxSuggestionsPerRun = 5;  // Max suggestions per analysis
    bool preferProcedural = true;       // Prefer procedural generation
    std::string cacheDirectory;         // Cache directory for fetched assets
};
```

## Approval Workflow

The framework supports an approval workflow for decisions:

```cpp
auto decisions = framework.AnalyzeProject();

// Execute with approval callback
uint32_t executed = framework.ExecuteApprovedDecisions(decisions,
    [](const AssetDecision& decision) -> DecisionAction {
        std::cout << "Approve " << decision.suggestion.name << "? (y/n): ";
        char c;
        std::cin >> c;
        
        if (c == 'y') {
            if (decision.shouldGenerate) {
                return DecisionAction::Generate;
            } else if (decision.shouldFetch) {
                return DecisionAction::Fetch;
            }
        }
        return DecisionAction::Skip;
    });

std::cout << "Executed " << executed << " decisions" << std::endl;
```

## Marketplace Integration

Connect to the marketplace import system for external assets:

```cpp
#include "engine/assets/MarketplaceImporter.h"

asset::MarketplaceImportRegistry registry;
registry.RegisterImporter(std::make_unique<asset::ItchIOImporter>());
registry.RegisterImporter(std::make_unique<asset::UnrealMarketplaceImporter>());

framework.SetMarketplaceRegistry(&registry);
```

## Recommended Sources

Get recommended asset sources for a category:

```cpp
auto sources = framework.GetRecommendedSources(AssetCategory::Terrain);
// Returns: ["procedural", "marketplace:itch.io"]

auto meshSources = framework.GetRecommendedSources(AssetCategory::Mesh3D);
// Returns: ["marketplace:sketchfab"]
```

## Best Practices

1. **Set Game Type Early**: Call `SetGameType()` before analyzing requirements
2. **Track Assets**: Use `AddAsset()` to maintain accurate inventory
3. **Review Suggestions**: Consider AI confidence levels before executing
4. **Prefer Procedural**: When possible, use procedural generation for determinism
5. **Validate Imports**: All marketplace assets are validated for Atlas compatibility

## Related Documentation

- [ATLAS_AI.md](16_ATLAS_AI.md) - Overall AI system architecture
- [MARKETPLACE_IMPORTING.md](MARKETPLACE_IMPORTING.md) - External asset importing
- [ASSET_SYSTEM.md](03_ASSET_SYSTEM.md) - Atlas asset system overview
