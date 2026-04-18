# NovaForge — Project-Specific Editor Headers

These headers define game-specific authoring tools for the NovaForge project. They are
**not** part of workspace core and do **not** count against the 20-tool primary roster cap.

## Loading Model

These tools surface into the Atlas Workspace shell through:
1. `ProjectSystemsTool` (workspace.project_systems) — the adapter host tool
2. `IGameProjectAdapter` — the interface contract that bridges project tools into the workspace
3. `WorkspaceShell::setGameProjectAdapter()` — registration point at workspace boot

## Headers

| Header | Panel Name | Purpose |
|--------|------------|---------|
| `GameEconomyEditor.h` | Economy Editor | In-game economy authoring, currency balance, drop rates |
| `InventoryEditor.h` | Inventory Editor | Item definitions, slots, capacity rules |
| `ItemShopEditor.h` | Item Shop Editor | Shop catalog, pricing, bundles |
| `DailyQuestEditor.h` | Daily Quest Editor | Daily/weekly quest definitions and rotations |
| `AchievementEditor.h` | Achievement Editor | Achievement definitions, unlock conditions |
| `CharacterCreatorEditor.h` | Character Creator | Character customization options and rules |
| `QuestEditor.h` | Quest Editor | Main quest/mission authoring |
| `ProgressionEditor.h` | Progression Editor | XP tables, level-up rules, unlock gates |
| `CostumeEditor.h` | Costume Editor | Cosmetic outfit definitions |
| `VirtualCurrencyEditor.h` | Virtual Currency Editor | Currency types, conversion rates |
| `TrophyEditor.h` | Trophy Editor | Trophy/achievement definitions |
| `SeasonPassEditor.h` | Season Pass Editor | Season pass tiers, rewards, milestones |
| `RewardSystemEditor.h` | Reward System Editor | Reward pools, drop tables, grant rules |
| `BiomeEditor.h` | Biome Editor | Biome definitions, terrain blending rules |
| `EcosystemEditor.h` | Ecosystem Editor | Wildlife, NPC spawn rules, ecosystem simulation |
| `DungeonGenerator.h` | Dungeon Generator | Procedural dungeon layout rules and templates |

## Rules

- These headers must **not** be included by any `Source/` (workspace core) build target
- Add new project-specific tools here, not in `Source/Editor/include/NF/Editor/`
- Workspace core contracts (`IGameProjectAdapter.h`, `IHostedTool.h`, `ProjectSystemsTool.h`) remain in `Source/Editor/include/NF/Editor/`
