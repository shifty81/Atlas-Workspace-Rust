# NovaForge — Phase-by-Phase Roadmap

> MasterRepo (v001) is the structural canonical reference. All phases are locked deliverables.
> tempnovaforge is the consolidation target — it is ahead of MasterRepo on implementation.

---

## Engine & Editor Phases

| Phase | Goal | Status |
|-------|------|--------|
| **Phase 0** — Bootstrap | Project scaffold, build system, directory structure, CMake, presets, vcpkg | ✅ Done |
| **Phase 1** — Core Engine | Core math/memory/reflection/serialization, Engine ECS, Input module | ✅ Done |
| **Phase 2** — Rendering & Physics | OpenGL RHI, physics rigid bodies, audio device, animation skeleton | ✅ Done |
| **Phase 3** — Voxel Runtime | Chunk data, ChunkMesher, WorldState, VoxelPickService, game loop scaffold | ✅ Done |
| **Phase 4** — Editor | Docking layout, 7-panel UI, viewport, inspector, project path service | ✅ Done |
| **Phase 5** — Graph VM | Deterministic bytecode VM, 14 graph types, visual scripting compiler | ✅ Done |
| **Phase 6** — Multiplayer | Server authority, replication, sessions, lockstep/rollback networking | ✅ Done |
| **Phase 7** — AI & Tooling | SwissAgent, ArbiterAI, BlenderGenerator, ContractScanner, ReplayMinimizer | ✅ Done |
| **Phase 8** — Custom IDE | Project-aware IDE: ProjectIndexer, CodeNavigator, BreadcrumbTrail, IDEPanel | ✅ Done |
| **Phase 9** — Pipeline Core | NF::Pipeline: PipelineWatcher, Manifest, WatchLog, ChangeEvent (S0) | ✅ Done |
| **Phase 10** — Polish & CI | Documentation, GitHub Actions, Docker, modding guide, final audit | ✅ Done |
| **Phase 11** — Suite Integration | Full tool suite wired through pipeline; AtlasAI broker enabled | ✅ Done |

---

## Game Phases (Nova Forge)

> Game phases use the G-series prefix. Each phase has a locked deliverable.

| Phase | Goal | Status |
|-------|------|--------|
| **G1** — First Interaction Loop | R.I.G. state, mining tool, resources, inventory, HUD, GameSession | ✅ Done |
| **G2** — Voxel Mesh Rendering | ChunkRenderer, VoxelShader, Frustum, ChunkRenderCache, lit terrain | ✅ Done |
| **G3** — Movement & FPS Camera | FPSCamera, PlayerMovement, VoxelCollider, PlayerController | ✅ Done |
| **G4** — Ship Systems | ShipClass, ShipModule, Ship, FlightController, CombatSystem | ✅ Done |
| **G5** — Fleet AI | Formation, CaptainPersonality, AICaptain, Fleet | ✅ Done |
| **G6** — Economy | Mining, refining, manufacturing, market pricing | ✅ Done |
| **G7** — Exploration | ProbeScanner, WormholeNetwork, AncientTechRegistry | ✅ Done |
| **G8** — FPS Interiors | ShipRoom, ShipInterior, EVAState, SurvivalStatus | ✅ Done |
| **G9** — Legend System | PlayerReputation, WorldBiasMap, NPCMemory, LegendStatus | ✅ Done |
| **G10** — Quest & Missions | MissionObjective, ActiveMission, MissionLog, QuestChain | ✅ Done |
| **G11** — Dialogue System | DialogueCondition, DialogueNode, DialogueGraph, DialogueRunner | ✅ Done |
| **G12** — Save/Load System | SaveSlot, SaveData, GameSaveSerializer, SaveSystem with auto-save | ✅ Done |
| **G13** — World Events | WorldEventType×7, EventEffect, WorldEvent, WorldEventSystem | ✅ Done |
| **G14** — Tech Tree | TechCategory×7, TechNode, TechTree with prereqs, tier bonuses | ✅ Done |
| **G15** — Player Progression | PlayerLevel (cap 50), SkillNode, SkillTree, ProgressionSystem | ✅ Done |
| **G16** — Crafting System | CraftingRecipe, CraftingQueue FIFO, CraftingSystem level-gated | ✅ Done |
| **G17** — Inventory & Equipment | ItemRarity×5, PlayerInventory (stacking), EquipmentLoadout | ✅ Done |
| **G18** — Status Effects | StatusEffectType×8, AilmentStack, StatusEffectSystem | ✅ Done |
| **G19** — Contracts & Bounties | ContractType×6, Contract lifecycle, BountyTarget, ContractBoard | ✅ Done |
| **G20** — Companion System | CompanionRole×6, CompanionPersonality, CompanionManager (max 4) | ✅ Done |
| **G21** — Faction System | FactionType×8, Faction, FactionTerritory, FactionRelation, FactionManager | ✅ Done |
| **G22** — Weather System | WeatherType×8, WeatherCondition, WeatherForecast, WeatherSystem | ✅ Done |
| **G23** — Trading System | TradeGoodCategory×8, TradingPost, TradeRoute, TradingSystem | ✅ Done |
| **G24** — Base Building | BasePartCategory×8, BaseLayout, BaseDefense, BaseSystem | ✅ Done |
| **G25** — Habitat System | HabitatZoneType×8, HabitatZone, LifeSupportModule, HabitatSystem | ✅ Done |
| **G26** — Power Grid System | PowerSourceType×8, PowerNode, PowerConduit, PowerGrid, PowerGridSystem | ✅ Done |
| **G27** — Vehicle System | VehicleType×8, VehicleSeat, VehicleComponent, Vehicle, VehicleSystem | ✅ Done |
| **G28** — Research System | ResearchCategory×8, ResearchProject, ResearchLab, ResearchTree, ResearchSystem | ✅ Done |
| **G29** — Diplomacy System | DiplomacyAction×8, DiplomaticStance×5, DiplomaticRelation, Treaty, DiplomaticChannel, DiplomacySystem | ✅ Done |
| **G30** — Espionage System | EspionageMissionType×8, SpyAgent, EspionageMission, IntelligenceNetwork, EspionageSystem | ✅ Done |
| **G31** — Colony Management | ColonyRole×8, Colonist, ColonyBuilding, Colony, ColonySystem | ✅ Done |
| **G32** — Archaeology System | ArtifactRarity×8, Artifact, ExcavationSite, ArtifactCollection, ArchaeologySystem | ✅ Done |
| **G33** — Migration System | MigrationTrigger×8, Migrant, MigrationWave, MigrationRoute, MigrationSystem | ✅ Done |
| **G34** — Insurgency System | InsurgencyType×8, InsurgentStatus×4, Insurgent, InsurgencyCell, InsurgencyMovement, InsurgencySystem | ✅ Done |
| **G35** — Plague System | PlagueType×8, InfectionStatus×5, PlagueCarrier, PlagueStat, PlagueRegion, PlagueSystem | ✅ Done |
| **G36** — Famine System | FamineType×8, FamineSeverity×5, FamineEvent, FamineRegion, FamineSystem | ✅ Done |
| **G37** — Refugee System | RefugeeOrigin×8, RefugeeStatus×5, Refugee, RefugeeCamp, RefugeeSystem | ✅ Done |
| **G38** — Storm System | StormType×8, StormSeverity×5, Storm, StormRegion, StormSystem | ✅ Done |
| **G39** — Earthquake System | EarthquakeScale×8, EarthquakeStatus×4, Earthquake, FaultLine, EarthquakeSystem | ✅ Done |
| **G40** — Volcano System | VolcanoActivity×8, VolcanoStatus×4, VolcanicEvent, Volcano, VolcanoSystem | ✅ Done |
| **G41** — Tsunami System | TsunamiCause×8, TsunamiStatus×4, TsunamiWave, Tsunami, TsunamiSystem | ✅ Done |
| **G42** — Wildfire System | WildfireType×8, WildfireSeverity×5, WildfireFront, WildfireZone, WildfireSystem | ✅ Done |
| **G43** — Flood System | FloodType×8, FloodSeverity×5, FloodWaterLevel, FloodZone, FloodSystem | ✅ Done |
| **G44** — Landslide System | LandslideType×8, LandslideSeverity×5, LandslideDebrisFlow, LandslideZone, LandslideSystem | ✅ Done |
| **G45** — Drought System | DroughtType×8, DroughtIntensity×5, DroughtRegion, DroughtZone, DroughtSystem | ✅ Done |
| **G46** — Epidemic System | EpidemicType×8, EpidemicPhase×5, EpidemicVector, EpidemicZone, EpidemicSystem | ✅ Done |
| **G47** — Solar Flare System | SolarFlareClass×8, SolarFlareEffect×6, SolarFlareEvent, SolarFlareRegion, SolarFlareSystem | ✅ Done |
| **G48** — Meteor Shower System | MeteorShowerClass×8, MeteorImpactType×6, MeteorEvent, MeteorShowerRegion, MeteorShowerSystem | ✅ Done |
| **G49** — Aurora System | AuroraType×8, AuroraIntensity×6, AuroraEvent, AuroraRegion, AuroraSystem | ✅ Done |
| **G50** — Heatwave System | HeatwaveType×8, HeatwaveSeverity×6, HeatwaveEvent, HeatwaveRegion, HeatwaveSystem | ✅ Done |
| **G51** — Blizzard System | BlizzardType×8, BlizzardIntensity×6, BlizzardEvent, BlizzardRegion, BlizzardSystem | ✅ Done |
| **G52** — Sandstorm System | SandstormType×8, SandstormSeverity×6, SandstormEvent, SandstormRegion, SandstormSystem | ✅ Done |
| **G53** — Cyclone System | CycloneCategory×6, CycloneStage×6, CycloneEvent, CycloneRegion, CycloneSystem | ✅ Done |
| **G54** — Tornado System | TornadoScale×6, TornadoStage×6, TornadoEvent, TornadoRegion, TornadoSystem | ✅ Done |
| **G55** — Dust Storm System | DustDensity×5, DustStormPhase×5, DustStormEvent, DustStormRegion, DustStormSystem | ✅ Done |
| **G56** — Hail Storm System | HailSize×5, HailStormPhase×5, HailStormEvent, HailStormRegion, HailStormSystem | ✅ Done |
| **G57** — Ice Storm System | IceThickness×5, IceStormPhase×5, IceStormEvent, IceStormRegion, IceStormSystem | ✅ Done |
| **G58** — Monsoon System | MonsoonIntensity×5, MonsoonPhase×5, MonsoonEvent, MonsoonRegion, MonsoonSystem | ✅ Done |
| **G59** — Lightning Storm System | LightningIntensity×5, LightningStormPhase×5, LightningEvent, LightningRegion, LightningSystem | ✅ Done |
| **G60** — Geomagnetic Storm System | GeoStormKpIndex×5, GeoStormPhase×5, GeoStormEvent, GeoStormRegion, GeoStormSystem | ✅ Done |
| **G61** — Cosmic Ray System | CosmicRayType×5, CosmicRayIntensity×5, CosmicRayEvent, CosmicRayRegion, CosmicRaySystem | ✅ Done |
| **G62** — Solar Wind System | SolarWindType×5, SolarWindIntensity×5, SolarWindEvent, SolarWindRegion, SolarWindSystem | ✅ Done |

---

## Workspace Suite Milestones (S-series)

> All tools communicate only through the shared `.novaforge/pipeline/` directory.
> The editor is the sole user-facing entry point; all tools run headlessly.

| Milestone | Goal | Status |
|-----------|------|--------|
| **S0** — Pipeline Core | NF::Pipeline: PipelineWatcher, Manifest, WatchLog, ChangeEvent. 17 tests. | ✅ Done |
| **S1** — Tool Wiring | All 5 tools respond to pipeline ChangeEvents end-to-end. 17 tests. | ✅ Done |
| **S2** — BlenderGen Bridge | BG-1→5 fully wired through pipeline into editor asset pipeline | ✅ Done |
| **S3** — SwissAgent Integration | SA-1→5 workspace broker functional in editor | ✅ Done |
| **S4** — ArbiterAI Integration | AB-1→5 AI reasoning broker routes through AtlasAI/ | ✅ Done |
| **S5** — Full Suite Validation | All tools active simultaneously, CI passes with suite tests | ✅ Done |

---

## Repo Consolidation Milestones

| Milestone | Source Repo | Status |
|-----------|-------------|--------|
| **C0** — MasterRepo seed | MasterRepo (v001) | ✅ Done |
| **C1** — MasterRepoRefactor | Structure + Atlas dirs | ✅ Done |
| **C2** — AtlasToolingSuite | Full tool suite | ✅ Done |
| **C3** — Nova-Forge-Expeditions | Richest game codebase | ⬜ Queued |
| **C4–C11** — Remaining repos | See `Docs/CONSOLIDATION_PLAN.md` | ⬜ Queued |
| **M1** — Usable Editor | Real GLFW/ImGui window, viewport clear, basic panels | ✅ Done |
| **SP** — Spec Pack | 6 system contracts, 6 schemas, 25 docs, audit + roadmap | ✅ Done |
| **M2 / S1** — Dev World Editing | PCG tuning panel, entity placement, voxel paint, undo/redo | ✅ Done |
| **M3 / S2** — Play-in-Editor | EditorWorldSession, Play/Pause/Stop toolbar, PIE snapshot | ✅ Done |
| **M4 / S3** — Asset Pipeline | AssetDatabase (GUID), mesh/texture importers, content browser hot-reload | ✅ Done |
| **S4** — Blender Bridge | novaforge_bridge.py add-on (BG-1→5), engine-side auto-import | ✅ Done |
| **S5** — Character & Animation Suite | FPSHandRig, TwoJointIK, AnimationBlendGraph, CharacterGroundingSystem | ✅ Done |
| **S6** — PCG World Tuning | BiomePainter, structure seed overrides, ore-seam editor, PCG preview | ✅ Done |
| **S7** — Logic Wiring UI | Entity logic graphs (GraphVM), event pins, graph templates | ✅ Done |
| **S8** — Tool Ecosystem | SwissAgent, ArbiterAI, ContractScanner, ReplayMinimizer as real standalones | ✅ Done |
| **S9** — AtlasAI Integration | AIInsightType×8, AIAnalysisEngine, AIProactiveSuggester, AIPipelineBridge, AtlasAIIntegration | ✅ Done |
| **S10** — Performance Profiler | ProfileMetricType×8, ProfileSample, ProfileSession, FrameProfiler, MemoryProfiler, ProfilerTimeline, PerformanceProfiler | ✅ Done |
| **S11** — Live Collaboration | CollabUserRole×8, CollabEditType×8, CollabUser, CollabEditAction, CollabSession, CollabConflictResolver, LiveCollaborationSystem | ✅ Done |
| **S12** — Version Control Integration | VCSProviderType×8, VCSFileStatus×8, VCSCommitInfo, VCSBranchInfo, VCSDiffEntry, VCSRepository, VersionControlSystem | ✅ Done |
| **S13** — Localization System | LocaleId×8, LocalizedString, TranslationEntry, TranslationTable, LocaleManager, LocalizationSystem | ✅ Done |
| **S14** — Plugin System | PluginState×8, PluginManifest, PluginInstance, PluginRegistry, PluginLoader, PluginSystem | ✅ Done |
| **S15** — Scripting Console | ScriptLanguage×8, ScriptVariable, ScriptResult, ScriptContext, ScriptConsole | ✅ Done |
| **S16** — Hot-Reload System | HotReloadAssetType×8, HotReloadStatus×5, HotReloadEntry, HotReloadWatcher, HotReloadDispatcher, HotReloadSystem | ✅ Done |
| **S17** — Asset Dependency Tracker | AssetDepType×8, AssetDepStatus×4, AssetDepNode, AssetDepGraph, AssetDependencyTracker | ✅ Done |
| **S18** — Build Configuration System | BuildTarget×8, BuildPlatform×5, BuildConfig, BuildProfile, BuildConfigurationSystem | ✅ Done |
| **S19** — Scene Snapshot System | SceneSnapshotType×8, SceneSnapshotState×4, SceneSnapshotFrame, SceneSnapshotHistory, SceneSnapshotSystem | ✅ Done |
| **S20** — Resource Monitor System | ResourceMonitorMetric×8, ResourceMonitorLevel×4, ResourceMonitorSample, ResourceMonitorChannel, ResourceMonitorSystem | ✅ Done |
| **S21** — Editor Event Bus System | EditorEventPriority×8, EditorBusState×4, EditorBusEvent, EditorEventSubscription, EditorEventBus | ✅ Done |
| **S22** — Workspace Layout Manager | LayoutPanelType×8, LayoutDockZone×4, LayoutPanel, LayoutSplit, WorkspaceLayout, WorkspaceLayoutManager | ✅ Done |
| **S23** — Shortcut Manager | ShortcutCategory×8, ShortcutState×4, ShortcutBinding, ShortcutContext, ShortcutManager | ✅ Done |
| **S24** — Notification System | NotificationSeverity×8, NotificationState×4, Notification, NotificationChannel, NotificationSystem | ✅ Done |
| **S25** — Undo/Redo System | UndoActionType×8, UndoActionState×4, UndoAction, UndoGroup, UndoRedoSystem | ✅ Done |
| **S26** — Command Palette | CommandPaletteCategory×8, CommandPaletteState×4, PaletteCommand, PaletteCommandGroup, CommandPalette | ✅ Done |
| **S27** — Theme Manager | ThemeMode×4, ThemeColor×8, ThemeToken, Theme, ThemeManager | ✅ Done |
| **S28** — Keyframe Animation Editor | KeyframeInterpolation×8, AnimationTrackType×8, Keyframe, AnimationTrack, KeyframeAnimationEditor | ✅ Done |
| **S29** — Curve Editor | CurveType×8, CurveHandleMode×4, CurveControlPoint, Curve, CurveEditorPanel | ✅ Done |
| **S30** — Gradient Editor | GradientType×8, GradientInterpolation×4, GradientColorStop, GradientRamp, GradientEditorPanel | ✅ Done |
| **S31** — Timeline Editor | TimelineEventType×8, TimelineTrackKind×4, TimelineEvent, TimelineTrack, TimelineEditorPanel | ✅ Done |
| **S32** — Particle Effect Editor | ParticleEmitterShape×8, ParticleBlendMode×4, ParticleEmitterConfig, ParticleEffectLayer, ParticleEffectEditor | ✅ Done |
| **S33** — Shader Graph Editor | ShaderNodeType×8, ShaderPortKind×4, ShaderNode, ShaderGraphEdge, ShaderGraphEditor | ✅ Done |
| **S34** — Material Editor | MaterialShadingModel×8, MaterialBlendMode×4, MaterialParameter, MaterialAsset, MaterialEditor | ✅ Done |
| **S35** — Texture Editor | TextureFormat×8, TextureFilter×4, TextureWrapMode×4, TextureAsset, TextureEditor | ✅ Done |
| **S36** — Font Editor | FontStyle×4, FontWeight×6, FontVariant×4, FontAsset, FontEditor | ✅ Done |
| **S37** — Icon Editor | IconSize×5, IconTheme×4, IconState×5, IconAsset, IconEditor | ✅ Done |
| **S38** — Sprite Editor | SpriteOrigin×5, SpriteBlendMode×5, SpriteAnimState×5, SpriteAsset, SpriteEditor | ✅ Done |
| **S39** — Tilemap Editor | TileFlipMode×5, TileLayerType×5, TileAnimMode×5, TileAsset, TilemapEditor | ✅ Done |
| **S40** — Audio Clip Editor | AudioClipFormat×5, AudioClipState×5, AudioLoopMode×5, AudioClipAsset, AudioClipEditor | ✅ Done |
| **S41** — Video Clip Editor | VideoClipCodec×5, VideoClipState×5, VideoAspectRatio×5, VideoClipAsset, VideoClipEditor | ✅ Done |
| **S42** — Scene Prefab Editor | PrefabCategory×5, PrefabState×5, PrefabLOD×5, PrefabAsset, ScenePrefabEditor | ✅ Done |
| **S43** — Animation Blueprint Editor | AnimBPNodeType×5, AnimBPState×5, AnimBPBlendMode×5, AnimBlueprintAsset, AnimBlueprintEditor | ✅ Done |

## Status Key

| Icon | Meaning |
|------|---------|
| ✅ | Done — implemented and tested |
| 🔧 | Active — in progress |
| 🔜 | Next — queued for implementation |
| 📋 | Planned — designed, not yet started |
