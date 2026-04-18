# Wiring Spec v1 — Atlas Workspace ↔ NovaForge Integration

> This document specifies the concrete integration contracts needed to turn Atlas
> Workspace from a shell of contracts and descriptors into a working editor that
> loads NovaForge projects, presents editable content, renders runtime-backed
> previews, runs shared PCG generation, and supports Play-In-Editor.

---

## 1. .atlas Load Pipeline

### Current State

- `AtlasProjectFileLoader` parses `.atlas` JSON into `AtlasProjectManifest`
- `main.cpp` detects `NovaForge.atlas` via filesystem check
- `NovaForgeAdapter` is selected and `initialize()` called
- Adapter returns 6 panel descriptors; panels are stubs

### Target State

Loading `.atlas` should produce real, populated project state across multiple registries.

### Contract

```
User opens .atlas file
    │
    ▼
AtlasProjectFileLoader::load(path)
    → AtlasProjectManifest
    → Validates: name, version, adapter, capabilities, content roots
    → Returns ProjectBootstrapResult (success + manifest OR errors)
    │
    ▼
ProjectRegistry::loadProject(manifest.adapterId)
    → Instantiates adapter via factory
    → Calls adapter.initialize(manifest, projectRoot)
    │
    ▼
NovaForgeAdapter::initialize(manifest, projectRoot)
    → Calls NovaForgeProjectBootstrap::boot(manifest, projectRoot)
        │
        ├─ Validates project directory structure
        │    Content/, Data/, worlds/, Config/
        │
        ├─ AssetCatalogPopulator::scanAndPopulate(contentRoot, assetCatalog)
        │    Recursive scan → classify → register in AssetCatalog
        │
        ├─ NovaForgeDocumentRegistry::loadSchemas(dataRoot)
        │    Discovers .json / .nfd files → registers document types
        │
        ├─ NovaForgeDataRegistry::loadTables(dataRoot)
        │    Loads gameplay data tables into memory
        │    (items, factions, missions, progression, economy, etc.)
        │
        └─ Returns BootstrapResult
             ├─ assetCount
             ├─ documentCount
             ├─ dataTableCount
             ├─ errors[]
             └─ warnings[]
    │
    ▼
ProjectSystemsTool::loadFromAdapter(adapter)
    → Receives panel descriptors with real createPanel factories
    → Factories now close over loaded data registries
    │
    ▼
WorkspaceShell enters Ready state
    → ContentBrowserPanel binds to AssetCatalog
    → InspectorPanel binds to SelectionService
    → ConsolePanel binds to DiagnosticCollector
    → NotificationCenter shows load summary
```

### Required New Types

| Type | Location | Purpose |
|------|----------|---------|
| `ProjectBootstrapResult` | `Source/Workspace/include/NF/Workspace/` | Typed load result with counts and errors |
| `NovaForgeProjectBootstrap` | `NovaForge/Source/EditorAdapter/` | Project structure validation and registry population |
| `NovaForgeDataRegistry` | `NovaForge/Source/EditorAdapter/` | In-memory gameplay data tables |

### Validation Rules

- Missing `Content/` root → error, project unusable
- Missing `Data/` root → warning, panels load empty
- Missing `worlds/` → warning, no default scene
- Invalid adapter ID → error, fallback to generic workspace
- Schema version mismatch → error with upgrade guidance

---

## 2. NovaForge Document System

### Current State

No document model exists. Panels have `projectRoot` and `ready` flag but no data.

### Target State

Every editable artifact in a NovaForge project is represented as a typed document
with schema validation, dirty tracking, undo/redo, and save/apply/revert.

### Document Type Registry

| Document Type | File Pattern | Panel | Schema Source |
|---------------|-------------|-------|---------------|
| `ItemDefinition` | `Data/items/*.json` | InventoryRulesPanel | `Schemas/item.schema.json` |
| `StructureArchetype` | `Data/structures/*.json` | (Scene/Asset Editor) | `Schemas/structure.schema.json` |
| `BiomeDefinition` | `Data/biomes/*.json` | (Scene Editor) | `Schemas/biome.schema.json` |
| `PlanetArchetype` | `Data/planets/*.json` | (Scene Editor) | `Schemas/planet.schema.json` |
| `FactionDefinition` | `Data/factions/*.json` | CharacterRulesPanel | `Schemas/faction.schema.json` |
| `MissionDefinition` | `Data/missions/*.json` | MissionRulesPanel | `Schemas/mission.schema.json` |
| `ProgressionRules` | `Data/progression/*.json` | ProgressionPanel | `Schemas/progression.schema.json` |
| `CharacterRules` | `Data/characters/*.json` | CharacterRulesPanel | `Schemas/character.schema.json` |
| `EconomyRules` | `Data/economy/*.json` | EconomyPanel | `Schemas/economy.schema.json` |
| `CraftingDefinition` | `Data/crafting/*.json` | (Data Editor) | `Schemas/crafting.schema.json` |
| `ShopDefinition` | `Data/shops/*.json` | ShopPanel | `Schemas/shop.schema.json` |
| `PCGRuleSet` | `Data/pcg/*.json` | (ProcGen Editor) | `Schemas/pcg_rules.schema.json` |

### Document Lifecycle

```
                    ┌─────────┐
                    │  Closed  │
                    └────┬─────┘
                         │ open(path)
                         ▼
                    ┌─────────┐
         ┌──────── │  Clean   │ ◄──── save() / revert()
         │         └────┬─────┘
         │              │ edit()
         │              ▼
         │         ┌─────────┐
         │         │  Dirty   │ ◄──── edit()
         │         └────┬─────┘
         │              │ save()
         │              ▼
         │         ┌─────────┐
         └──────── │  Clean   │
                   └──────────┘
```

### Required Interfaces

```cpp
// Base document contract
class NovaForgeDocument {
public:
    virtual ~NovaForgeDocument() = default;

    virtual std::string documentId() const = 0;
    virtual NovaForgeDocumentType documentType() const = 0;
    virtual std::string displayName() const = 0;
    virtual std::filesystem::path filePath() const = 0;

    // Lifecycle
    virtual bool load(const std::filesystem::path& path) = 0;
    virtual bool save() = 0;
    virtual bool revert() = 0;

    // State
    virtual bool isDirty() const = 0;
    virtual bool isValid() const = 0;
    virtual std::vector<std::string> validationErrors() const = 0;

    // Edit tracking
    virtual void markDirty() = 0;
    virtual void markClean() = 0;
};

// Panel contract for document-backed panels
class IDocumentPanel : public IEditorPanel {
public:
    virtual void bindDocument(std::shared_ptr<NovaForgeDocument> doc) = 0;
    virtual NovaForgeDocument* boundDocument() const = 0;
    virtual bool isDirty() const = 0;
    virtual bool save() = 0;
    virtual bool revert() = 0;
};
```

### Document ↔ UndoStack Binding

Each open document gets its own named `UndoStack` registered with `UndoManager`.
Stack name follows pattern: `"doc.<documentType>.<documentId>"`.

Edit operations push `UndoAction` entries with do/undo handlers that mutate the
document's in-memory data model. `save()` calls `markClean()` and resets dirty flag.

---

## 3. Preview Runtime Bridge

### Current State

- `ViewportPanel` accepts a texture ID via `setColorAttachment()` but shows placeholder grid
- `SceneEditorTool` implements `IViewportSceneProvider` but returns placeholder data
- No actual 3D scene or runtime exists in the editor

### Target State

A lightweight NovaForge runtime instance runs inside the editor, providing real scene
data to viewport panels. The same rendering/simulation code used by the game client
powers editor previews.

### Architecture

```
┌─────────────────────────────────────┐
│          Atlas Workspace            │
│                                     │
│  ┌───────────┐   ┌───────────────┐  │
│  │ Scene     │   │ Asset         │  │
│  │ Editor    │   │ Editor        │  │
│  │ Tool      │   │ Tool          │  │
│  └─────┬─────┘   └──────┬────────┘  │
│        │                │            │
│        ▼                ▼            │
│  ┌──────────────────────────────┐   │
│  │    NovaForgePreviewRuntime   │   │
│  │                              │   │
│  │  ┌────────────────────────┐  │   │
│  │  │  PreviewWorld          │  │   │
│  │  │  - entities[]          │  │   │
│  │  │  - components[]        │  │   │
│  │  │  - transforms[]        │  │   │
│  │  └────────────────────────┘  │   │
│  │                              │   │
│  │  ┌────────────────────────┐  │   │
│  │  │  PreviewCamera         │  │   │
│  │  │  - position/rotation   │  │   │
│  │  │  - projection          │  │   │
│  │  └────────────────────────┘  │   │
│  │                              │   │
│  │  ┌────────────────────────┐  │   │
│  │  │  PreviewRenderer       │  │   │
│  │  │  - scene → framebuffer │  │   │
│  │  │  - gizmo overlay       │  │   │
│  │  └────────────────────────┘  │   │
│  └──────────────────────────────┘   │
│                 │                    │
│                 ▼                    │
│  ┌──────────────────────────────┐   │
│  │  ViewportHostRegistry        │   │
│  │  slot → texture ID → panel   │   │
│  └──────────────────────────────┘   │
│                 │                    │
│                 ▼                    │
│  ┌──────────────────────────────┐   │
│  │  ViewportPanel (UI)          │   │
│  │  drawImage(textureId)        │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

### Preview Scene Types

| Preview Type | Owner Tool | Scene Content | Update Trigger |
|-------------|-----------|---------------|----------------|
| World/Level | SceneEditorTool | Full level entities + terrain | Entity edit, transform change |
| Single Asset | AssetEditorTool | Selected asset + ground plane + light rig | Asset selection, metadata edit |
| Material | MaterialEditorTool | Test mesh + selected material | Parameter edit |
| PCG Structure | ProcGenRuleEditor | Generated structure fragment | Rule edit, seed change |
| PCG Biome | ProcGenRuleEditor | Generated biome chunk | Rule edit, seed change |
| Animation | AnimationEditorTool | Skinned mesh + playing clip | Clip selection, timeline scrub |

### Required New Types

| Type | Location | Purpose |
|------|----------|---------|
| `NovaForgePreviewRuntime` | `NovaForge/Source/Runtime/` | Lightweight game runtime for editor |
| `PreviewWorld` | `NovaForge/Source/Runtime/` | Entity/component container for preview |
| `PreviewCamera` | `NovaForge/Source/Runtime/` | Editor camera with fly-cam controls |
| `PreviewRenderer` | `NovaForge/Source/Runtime/` | Scene → framebuffer rendering |
| `PreviewSceneFactory` | `NovaForge/Source/Runtime/` | Creates typed preview scenes |
| `AssetPreviewScene` | `NovaForge/Source/Runtime/` | Single-asset preview with environment |
| `MaterialPreviewScene` | `NovaForge/Source/Runtime/` | Test mesh for material preview |

### Camera Controls

| Input | Action | Mode |
|-------|--------|------|
| RMB + WASD | Fly-camera movement | Viewport focused |
| RMB + Mouse | Camera rotation (pitch/yaw) | Viewport focused |
| Scroll wheel | Dolly zoom | Viewport hover |
| MMB + drag | Pan | Viewport focused |
| F | Frame selected object | Any |
| Numpad 0 | Reset camera | Any |

---

## 4. Shared PCG Preview Loop

### Current State

- PCG pipeline exists as file-based event system (`.atlas/pipeline/changes/`)
- `PipelineWatcher` polls for change events
- `ToolAdapter` pattern processes events (BlenderGenAdapter, ContractScannerAdapter)
- No connection between PCG pipeline and editor viewport
- No shared generation code between editor and game client

### Target State

One shared PCG core library is used by both the game client runtime and the editor
preview system. Edits to PCG rules in panels trigger immediate preview regeneration.

### Architecture

```
┌─────────────────────────────────────────────────────┐
│              Shared PCG Core (library)              │
│                                                     │
│  PCGRuleSet          PCGGeneratorService            │
│  ├─ rules[]          ├─ generate(rules, seed, ctx)  │
│  ├─ constraints[]    ├─ validate(rules)             │
│  └─ assetRefs[]      └─ estimateCost(rules)         │
│                                                     │
│  PCGSeedContext       PCGOutput                     │
│  ├─ masterSeed       ├─ placements[]                │
│  ├─ layerSeeds[]     ├─ meshInstances[]             │
│  └─ deterministic()  ├─ materialOverrides[]         │
│                      └─ metadata{}                  │
└─────────────────┬───────────────┬───────────────────┘
                  │               │
        ┌─────────┘               └──────────┐
        ▼                                    ▼
┌───────────────┐                  ┌───────────────────┐
│  Game Client  │                  │  Editor Preview    │
│  Runtime      │                  │  Service           │
│               │                  │                    │
│  Uses PCG     │                  │  PCGPreviewService │
│  core at      │                  │  ├─ generatePreview│
│  world load   │                  │  ├─ cache results  │
│  and runtime  │                  │  ├─ invalidate on  │
│  generation   │                  │  │   rule edit      │
│               │                  │  └─ feed to        │
│               │                  │     PreviewWorld   │
└───────────────┘                  └───────────────────┘
```

### Edit → Preview Loop

```
1. User edits PCG rule in panel (e.g., changes structure density)
     │
     ▼
2. Panel mutates NovaForgeDocument (PCGRuleSet)
     │ Document becomes dirty
     │ UndoAction pushed to stack
     │
     ▼
3. Panel fires document-changed event on EventBus
     │ Topic: "document.changed"
     │ Payload: documentId, documentType
     │
     ▼
4. PCGPreviewService receives event
     │ Checks if document is PCG-related
     │
     ▼
5. PCGPreviewService calls shared PCGGeneratorService
     │ Inputs: updated rules, current seed, generation context
     │ Output: PCGOutput (placements, meshes, materials)
     │
     ▼
6. PreviewWorld updates from PCGOutput
     │ Removes old generated entities
     │ Creates new entities from placements
     │
     ▼
7. PreviewRenderer re-renders scene
     │ Viewport texture updated
     │
     ▼
8. ViewportPanel displays updated preview
```

### PCG Rule Types

| Rule Type | Governs | Example Parameters |
|-----------|---------|-------------------|
| `StructurePlacement` | Building/structure spawn | density, min/max spacing, biome affinity |
| `BiomeGeneration` | Terrain biome layout | noise octaves, scale, blend, elevation range |
| `VegetationScatter` | Flora placement | density, slope range, cluster size, exclusion zones |
| `ResourceDistribution` | Ore/resource placement | rarity, vein size, depth range, biome constraints |
| `LootTable` | Item drop generation | weight, min/max count, level scaling, rarity tiers |
| `EnemySpawn` | NPC/enemy placement | density, patrol radius, level range, faction |
| `DungeonLayout` | Interior generation | room count, corridor width, branch factor |

### Save-Back Flow

Edits made in preview panels save back to the project's `Data/pcg/` directory.

```
Panel edit → Document dirty → User saves (Ctrl+S)
    → NovaForgeDocument::save()
    → Writes updated JSON to Data/pcg/<ruleset>.json
    → Document marked clean
    → Pipeline change event emitted (ChangeEventType::ScriptUpdated)
    → PipelineWatcher notifies other tools if needed
```

---

## 5. PIE (Play-In-Editor) Lifecycle

### Current State

No PIE implementation exists. The NovaForge game server (`NovaForgeServer`) can run
as a standalone process but is not integrated with the editor.

### Target State

Two play modes:
1. **Embedded PIE** — lightweight runtime in viewport for fast iteration
2. **External Launch** — full game executable for validation

### Embedded PIE Lifecycle

```
┌──────────────────────────────────────────────────────────────┐
│                    PIE State Machine                         │
│                                                              │
│  ┌────────┐  enter()  ┌─────────┐  pause()  ┌────────────┐  │
│  │ Editor │ ────────► │ Playing │ ────────► │   Paused   │  │
│  │  Mode  │ ◄──────── │         │ ◄──────── │            │  │
│  └────────┘   exit()  └─────────┘  resume() └────────────┘  │
│       ▲                     │                     │          │
│       │                     │ step()              │ step()   │
│       │                     ▼                     ▼          │
│       │               ┌──────────┐          ┌──────────┐    │
│       │               │ Stepped  │          │ Stepped  │    │
│       │               └──────────┘          └──────────┘    │
│       │                                                      │
│       └──────────────── reset() ◄────────────────────────    │
└──────────────────────────────────────────────────────────────┘
```

### PIEService Contract

```cpp
class PIEService {
public:
    enum class State { Editor, Entering, Playing, Paused, Exiting };

    // Lifecycle
    bool enter();      // Snapshot editor state, create runtime instance
    bool exit();       // Destroy runtime, restore editor state
    bool pause();      // Freeze runtime tick
    bool resume();     // Resume runtime tick
    bool step();       // Advance one frame while paused
    bool reset();      // Restart from last snapshot

    // State
    State state() const;
    bool isPlaying() const;
    bool isPaused() const;

    // Runtime access
    NovaForgePreviewRuntime* runtime();

    // Configuration
    void setEntryWorld(const std::string& worldPath);
    void setStartPosition(float x, float y, float z);
};
```

### PIE Enter Flow

```
1. User clicks Play button or presses F5
     │
     ▼
2. PIEService::enter()
     │
     ├─ Snapshot current editor world state
     │   (entity positions, component data, camera)
     │
     ├─ Create PIE runtime instance (NovaForgePreviewRuntime)
     │   Duplicate world state into runtime
     │
     ├─ Switch input routing: editor → game input mode
     │   WorkspaceInputBridge routes to runtime input handler
     │
     ├─ Switch viewport: editor scene → PIE runtime scene
     │   ViewportPanel receives PIE runtime texture
     │
     ├─ Set editor panels to read-only mode
     │   Dirty edits blocked; inspection still works
     │
     └─ Notify workspace: PIEEntered event on EventBus
```

### PIE Exit Flow

```
1. User clicks Stop or presses Escape/F5
     │
     ▼
2. PIEService::exit()
     │
     ├─ Destroy PIE runtime instance
     │
     ├─ Restore editor world state from snapshot
     │
     ├─ Switch input routing: game → editor input mode
     │
     ├─ Switch viewport: PIE runtime → editor scene
     │
     ├─ Restore editor panels to read-write mode
     │
     └─ Notify workspace: PIEExited event on EventBus
```

### PIE Controls

| Input | Action | Context |
|-------|--------|---------|
| F5 | Toggle Play/Stop | Global |
| F6 | Pause/Resume | PIE active |
| F7 | Step one frame | PIE paused |
| F8 | Reset to start | PIE active |
| Escape | Exit PIE | PIE active |

### External Launch

External launch uses the existing `WorkspaceLaunchContract` and `WorkspaceAppRegistry`:

```
1. User selects Build → Run (or Ctrl+F5)
     │
     ▼
2. BuildTool executes build task graph
     │ Output → ConsolePanel
     │ Errors → DiagnosticCollector → NotificationBus
     │
     ▼
3. On success: WorkspaceAppRegistry::launch(launchTarget)
     │ Target from .atlas buildProfiles (Client, Server, etc.)
     │ Process launched as child
     │
     ▼
4. Console output from child process → ConsolePanel
     │ Exit code → NotificationBus
```

---

## 6. Panel-to-Document Binding Matrix

### Core Tool Panels

| Tool | Primary Panel | Document Type | Preview Scene | Shared Panels Used |
|------|--------------|---------------|---------------|-------------------|
| **SceneEditorTool** | Hierarchy + Viewport | World/Level document | Full world scene | Inspector, Outliner, Console |
| **AssetEditorTool** | Content Browser + Preview | Asset document + metadata | Single asset scene | Inspector, Console |
| **MaterialEditorTool** | Material Graph + Preview | Material graph document | Test mesh scene | Inspector |
| **AnimationEditorTool** | Timeline + Skeleton | Animation clip document | Skinned mesh scene | Inspector |
| **DataEditorTool** | Data Grid + Inspector | Any NovaForge data table | None | Inspector |
| **VisualLogicEditorTool** | Graph Canvas + Node Palette | Graph/script document | Test scene (optional) | Inspector, Console |
| **BuildTool** | Build Log + Config | Build task graph | None | Console, Notifications |
| **AtlasAITool** | Chat + Context | AI session document | None | Console, Notifications |

### NovaForge Gameplay Panels

| Panel | Document Type | Data Source | Editable Fields | Preview |
|-------|--------------|-------------|-----------------|---------|
| **EconomyPanel** | `EconomyRules` | `Data/economy/*.json` | Currencies, exchange rates, inflation, sinks/faucets | Chart/graph preview |
| **InventoryRulesPanel** | `ItemDefinition` | `Data/items/*.json` | Slot layouts, stack sizes, weight limits, storage rules | Grid layout preview |
| **ShopPanel** | `ShopDefinition` | `Data/shops/*.json` | Store listings, prices, availability, purchase conditions | Storefront preview |
| **MissionRulesPanel** | `MissionDefinition` | `Data/missions/*.json` | Objectives, chains, rewards, prerequisites, XP grants | Flow graph preview |
| **ProgressionPanel** | `ProgressionRules` | `Data/progression/*.json` | XP curves, level thresholds, skill trees, unlock gates | Curve/tree preview |
| **CharacterRulesPanel** | `CharacterRules` | `Data/characters/*.json` | Creation presets, classes, stat caps, faction membership | Stat radar preview |

### Shared Panel Bindings

| Shared Panel | Data Source | Updates On |
|-------------|-------------|-----------|
| **InspectorPanel** | Selected entity/asset/document properties | SelectionService change |
| **HierarchyPanel** | Active world/scene entity tree | Entity create/delete/reparent |
| **ContentBrowserPanel** | AssetCatalog entries | Asset import/delete/rename |
| **ConsolePanel** | DiagnosticCollector messages | Build output, runtime errors, warnings |
| **GraphEditorPanel** | Active graph document (material, visual logic) | Node add/remove/connect |
| **IDEPanel** | SearchEngine results (symbols, files) | User search query |
| **PipelineMonitorPanel** | PipelineWatcher events | Pipeline change events |

### Document ↔ Panel Wiring Pattern

Every document-backed panel follows this pattern:

```
Panel Construction:
    panel = new FooPanel()
    panel.bindDocument(document)     // Sets data source
    panel.setUndoStack(undoStack)    // Connects undo/redo
    panel.setEventBus(eventBus)      // For change notifications
    panel.setSaveCommand(saveCmd)    // Ctrl+S handler

Panel Update Loop:
    if (document.isDirty())
        panel.setTabDirty(true)      // Show dirty indicator
    panel.renderFromDocument()       // Display current document state

User Edit:
    panel.onFieldEdit(field, newValue)
        → undoStack.push(editAction)
        → document.setField(field, newValue)
        → document.markDirty()
        → eventBus.publish("document.changed", documentId)

User Save (Ctrl+S):
    panel.save()
        → document.validate()
        → if valid: document.save()
        →           document.markClean()
        →           panel.setTabDirty(false)
        →           notificationBus.info("Saved: " + document.displayName())
        → if invalid: notificationBus.error(document.validationErrors())

User Revert:
    panel.revert()
        → document.revert()
        → undoStack.clear()
        → panel.refreshFromDocument()
        → panel.setTabDirty(false)
```

### Selection Flow Across Panels

```
User clicks entity in HierarchyPanel
    │
    ├─ SelectionService::select(entityId)
    │
    ├─ EventBus::publish("selection.changed", entityId)
    │
    ├─ InspectorPanel receives event
    │   → Loads entity component properties
    │   → Displays editable property grid
    │
    ├─ ViewportPanel receives event
    │   → Highlights selected entity
    │   → Shows transform gizmo
    │
    └─ Other panels receive event
        → Update context-sensitive displays
```

---

## Appendix: File Locations for New Types

| New Type | Proposed Location |
|----------|-------------------|
| `NovaForgeProjectBootstrap` | `NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/NovaForgeProjectBootstrap.h` |
| `NovaForgeDocumentRegistry` | `NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/NovaForgeDocumentRegistry.h` |
| `NovaForgeDocument` | `NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/NovaForgeDocument.h` |
| `NovaForgeDataRegistry` | `NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/NovaForgeDataRegistry.h` |
| `IDocumentPanel` | `Source/Workspace/include/NF/Workspace/IDocumentPanel.h` |
| `ProjectBootstrapResult` | `Source/Workspace/include/NF/Workspace/ProjectBootstrapResult.h` |
| `NovaForgePreviewRuntime` | `NovaForge/Source/Runtime/include/NovaForge/Runtime/NovaForgePreviewRuntime.h` |
| `PreviewWorld` | `NovaForge/Source/Runtime/include/NovaForge/Runtime/PreviewWorld.h` |
| `PreviewCamera` | `NovaForge/Source/Runtime/include/NovaForge/Runtime/PreviewCamera.h` |
| `PreviewRenderer` | `NovaForge/Source/Runtime/include/NovaForge/Runtime/PreviewRenderer.h` |
| `PreviewSceneFactory` | `NovaForge/Source/Runtime/include/NovaForge/Runtime/PreviewSceneFactory.h` |
| `PCGRuleSet` | `NovaForge/Source/PCG/include/NovaForge/PCG/PCGRuleSet.h` |
| `PCGGeneratorService` | `NovaForge/Source/PCG/include/NovaForge/PCG/PCGGeneratorService.h` |
| `PCGSeedContext` | `NovaForge/Source/PCG/include/NovaForge/PCG/PCGSeedContext.h` |
| `PCGOutput` | `NovaForge/Source/PCG/include/NovaForge/PCG/PCGOutput.h` |
| `PCGPreviewService` | `NovaForge/Source/PCG/include/NovaForge/PCG/PCGPreviewService.h` |
| `PIEService` | `Source/Workspace/include/NF/Workspace/PIEService.h` |
