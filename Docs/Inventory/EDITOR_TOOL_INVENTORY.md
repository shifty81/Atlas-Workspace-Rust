# Editor Tool Inventory

323 headers in `Source/Editor/include/NF/Editor/`. Target: up to 20 primary tools + project adapter tools (unlimited).

## Classification Key

| Action | Meaning |
|--------|---------|
| **Keep** | Primary tool or critical workspace component |
| **Panel** | Convert to shared reusable panel |
| **Service** | Convert to shared non-visual service |
| **Adapter** | Move to NovaForge adapter (project-specific) |
| **Archive** | Remove from active registry, defer/archive |
| **Merge** | Fold into a broader existing tool |

---

## Keep as Primary Tools / Core Workspace

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| EditorApp.h | Core | Core | Keep |
| EditorPanel.h | Core | Core | Keep |
| Editor.h | Umbrella | Minimal Core | Edit (reduce) |
| DockLayout.h | Core | Core | Keep |
| LayoutPersistence.h | Core | Core | Keep |
| MenuBar.h | Core | Core | Keep |
| CommandPalette.h | Core | Core | Keep |
| SelectionService.h | Core | Core | Keep |
| EditorTheme.h | Core | Core | Keep |
| WorkspaceAppRegistry.h | Core | Core | Keep |
| WorkspaceLaunchContract.h | Core | Core | Keep |
| WorkspacePanelHost.h | Core | Core | Keep |
| WorkspaceInputBridge.h | Core | Core | Keep |
| WorkspaceShellContract.h | Core | Core | Keep |
| WorkspaceLayout.h | Core | Core | Keep |
| PluginSystem.h | Core | Core | Keep |
| UndoRedoSystem.h | Core | Core | Keep |
| ShortcutManager.h | Core | Core | Keep |
| ProjectServices.h | Core | Core | Keep |
| AtlasAIPanelHost.h | Core | Core | Keep |

## Keep as Primary Editor Tools

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| AnimationEditorTool.h | Tool | Tool | Keep |
| AnimBlueprintEditor.h | Tool | Tool | Keep |
| MaterialEditorTool.h | Tool | Tool | Keep |
| ShaderGraphEditor.h | Tool | Tool | Keep |
| DataEditorTool.h | Tool | Tool | Keep |
| PackagingEditor.h | Tool | Tool | Keep |
| BuildPipelineEditorV1.h | Tool | Tool | Keep |
| UIEditorTool.h | Tool | Tool | Keep (rename from HUDEditor) |
| TimelineEditor.h | Tool | Tool | Keep |
| TextureEditor.h | Tool | Tool | Keep |
| StaticMeshEditor.h | Tool | Tool | Keep |
| PrefabEditor.h | Tool | Tool | Keep |
| GraphEditorPanel.h | Tool | Tool | Keep |
| LogicGraph.h | Tool | Tool | Keep |
| ParticleEditorTool.h | Tool | Tool | Keep |
| AudioEditorTool.h | Tool | Tool | Keep |
| PhysicsEditorTool.h | Tool | Tool | Keep (stub needed) |
| TerrainEditorTool.h | Tool | Tool | Keep (promote from Scene/) |
| CinematicEditorTool.h | Tool | Tool | Keep (promote from Animation/) |
| ProfilerTool.h | Tool | Tool | Keep (stub needed) |
| VersionControlTool.h | Tool | Tool | Keep (promote from service) |
| ScriptingConsoleTool.h | Tool | Tool | Keep (promote from ScriptingConsole.h) |
| SettingsTool.h | Tool | Tool | Keep (stub needed) |
| WorkspaceBrowserTool.h | Tool | Tool | Keep (stub needed) |

## Convert to Shared Panels

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| ContentBrowser.h | Tool | Panel | Panel |
| ContentBrowserPanel.h | Tool | Panel | Panel |
| InspectorPanel.h | Tool | Panel | Panel |
| HierarchyPanel.h | Tool | Panel | Panel |
| ConsolePanel.h | Tool | Panel | Panel |
| NotificationSystem.h | Service | Panel | Panel |
| Notifications.h | Service | Panel | Panel |
| NotificationWorkflow.h | Service | Panel | Panel |
| NotificationCenterEditor.h | Tool | Panel | Panel |
| ViewportPanel.h | Panel | Panel | Panel |
| ComponentInspectorV1.h | Tool | Panel | Panel |
| DiagnosticPanelV1.h | Tool | Panel | Panel |
| MemoryProfilerPanel.h | Tool | Panel | Panel |
| PipelineMonitorPanel.h | Tool | Panel | Panel |
| OnlineServicesPanel.h | Tool | Panel | Panel |
| RenderStatsPanel.h | Panel | Panel | Panel |
| RenderSettingsPanel.h | Panel | Panel | Panel |
| PropertyEditor.h | Panel | Panel | Panel |
| PropertyGridV1.h | Panel | Panel | Panel |
| TreeViewV1.h | Panel | Panel | Panel |
| TableViewV1.h | Panel | Panel | Panel |
| AIAssistantPanel.h | Panel | Panel | Panel |
| SettingsPanel.h | Panel | Panel | Panel |
| SettingsControlPanelV1.h | Panel | Panel | Panel |
| ProfilerViewV1.h | Panel | Panel | Panel |
| MemoryTrackerV1.h | Panel | Panel | Panel |
| LODEditorPanel.h | Panel | Panel | Panel |

## Convert to Shared Services

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| EditorEventBus.h | Service | Service | Service |
| ConsoleCommandBus.h | Service | Service | Service |
| GraphHostContract.h | Contract | Service | Service |
| GraphHostContractV1.h | Contract | Service | Service |
| FileIntakePipeline.h | Service | Service | Service |
| CodexSnippetMirror.h | Service | Service | Service |
| AssetDatabase.h | Service | Service | Service |
| AssetImporters.h | Service | Service | Service |
| BlenderImporter.h | Service | Service | Service |
| ViewportHostContract.h | Contract | Service | Service |
| EditorCamera.h | Service | Service | Service |
| ToolWindowManager.h | Service | Service | Service |
| EditorToolbar.h | Service | Service | Service |
| EditorSettings.h | Service | Service | Service |
| ThemeManager.h | Service | Service | Service |
| ToolEcosystem.h | Service | Service | Service |
| ScrollVirtualization.h | Service | Service | Service |
| ScrollVirtualizerV1.h | Service | Service | Service |
| PanelStateSerializer.h | Service | Service | Service |
| DockTreeSerializer.h | Service | Service | Service |
| LayoutManagerV1.h | Service | Service | Service |
| DropTargetHandler.h | Service | Service | Service |
| AssetImportQueue.h | Service | Service | Service |
| HotReload.h | Service | Service | Service |
| VersionControl.h | Service | Service | Service |
| Scripting.h | Service | Service | Service |
| ScriptingConsole.h | Service | Service | Service |
| Collaboration.h | Service | Service | Service |
| FrameStatsTracker.h | Service | Service | Service |
| Profiling.h | Service | Service | Service |
| ResourceMonitor.h | Service | Service | Service |
| BuildConfiguration.h | Service | Service | Service |
| BuildReport.h | Service | Service | Service |
| PlatformProfile.h | Service | Service | Service |
| Localization.h | Service | Service | Service |
| WidgetKitV1.h | Service | Service | Service |
| TooltipSystemV1.h | Service | Service | Service |
| TabBarV1.h | Service | Service | Service |
| CommandPaletteV1.h | Service | Service | Service |
| HotkeyRegistryV1.h | Service | Service | Service |
| GestureRecognizerV1.h | Service | Service | Service |
| LoggingRouteV1.h | Service | Service | Service |
| AIDebugPathV1.h | Service | Service | Service |
| AIPanelSession.h | Service | Service | Service |
| AIActionSurface.h | Service | Service | Service |
| AIIntegration.h | Service | Service | Service |
| ProjectSurfaceV1.h | Service | Service | Service |
| RepoSurfaceV1.h | Service | Service | Service |
| TypographySystem.h | Service | Service | Service |

## Move to Project Adapter (NovaForge or other project)

> These headers belong in `NovaForge/Source/Editor/include/NovaForge/Editor/` (or equivalent per-project path), **not** in workspace core. They load into the workspace shell through `ProjectSystemsTool` + `IGameProjectAdapter` and do not count against the 20-tool roster cap.

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| GameEconomyEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| InventoryEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| ItemShopEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| DailyQuestEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| AchievementEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| CharacterCreatorEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| QuestEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| ProgressionEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| CostumeEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| VirtualCurrencyEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| TrophyEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| SeasonPassEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| RewardSystemEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| BiomeEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| EcosystemEditor.h | Tool | NovaForge/Source/Editor/ | Adapter |
| DungeonGenerator.h | Tool | NovaForge/Source/Editor/ | Adapter |

## Archive / Defer

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| ArcadeGameEditor.h | Tool | Archive | Archive |
| MiniGameEditor.h | Tool | Archive | Archive |
| PuzzleEditor.h | Tool | Archive | Archive |
| LobbyEditor.h | Tool | Archive | Archive |
| MatchmakingEditor.h | Tool | Archive | Archive |
| MatchReplayEditor.h | Tool | Archive | Archive |
| LiveOpsEditor.h | Tool | Archive | Archive |
| CloudStorageEditor.h | Tool | Archive | Archive |
| CloudSyncEditor.h | Tool | Archive | Archive |
| BroadcastEditor.h | Tool | Archive | Archive |
| ExperimentEditor.h | Tool | Archive | Archive |
| BenchmarkSuiteEditor.h | Tool | Archive | Archive |
| LoadTestEditor.h | Tool | Archive | Archive |
| StressTestEditor.h | Tool | Archive | Archive |
| BandwidthProfileEditor.h | Tool | Archive | Archive |
| LatencySimEditor.h | Tool | Archive | Archive |
| PacketCapture.h | Tool | Archive | Archive |
| NetworkTopologyEditor.h | Tool | Archive | Archive |
| MultiplayerPreview.h | Tool | Archive | Archive |
| AccessibilityEditor.h | Tool | Archive | Archive |
| ColorblindSimulator.h | Tool | Archive | Archive |
| MenuLayoutEditor.h | Tool | Archive | Archive |
| DisplayModeEditor.h | Tool | Archive | Archive |
| GamepadConfigurator.h | Tool | Archive | Archive |
| ControlSchemeEditor.h | Tool | Archive | Archive |
| AxisMappingEditor.h | Tool | Archive | Archive |
| InputActionEditor.h | Tool | Archive | Archive |
| InputBindingsEditor.h | Tool | Archive | Archive |
| TelemetryEditor.h | Tool | Archive | Archive |
| AnalyticsDashboard.h | Tool | Archive | Archive |
| FeatureFlagEditor.h | Tool | Archive | Archive |
| RuntimeConfigEditor.h | Tool | Archive | Archive |
| MusicSequencer.h | Tool | Archive | Archive |
| SpectatorEditor.h | Tool | Archive | Archive |
| TouchInputMapper.h | Tool | Archive | Archive |
| TournamentEditor.h | Tool | Archive | Archive |
| ModdingToolkit.h | Tool | Archive | Archive |
| PlaytestRecorder.h | Tool | Archive | Archive |
| DeploymentEditor.h | Tool | Archive | Archive |
| SandboxEditor.h | Tool | Archive | Archive |

## Merge into Broader Tools

| Header | Current | Merge Into | Action |
|--------|---------|------------|--------|
| LevelDesignToolkit.h | Tool | Scene Editor | Merge |
| WorldGeneration.h | Tool | Scene Editor | Merge |
| WorldPartition.h | Tool | Scene Editor | Merge |
| WorldPreview.h | Tool | Scene Editor | Merge |
| EntityPlacement.h | Tool | Scene Editor | Merge |
| SceneSnapshot.h | Tool | Scene Editor | Merge |
| SceneStreaming.h | Tool | Scene Editor | Merge |
| VoxelPaint.h | Tool | Scene Editor | Merge |
| RoomEditor.h | Tool | Scene Editor | Merge |
| TilemapEditor.h | Tool | Scene Editor | Merge |
| PCGTuning.h | Tool | Scene Editor | Merge |
| FoliagePainter.h | Tool | Scene Editor | Merge |
| FoliagePainterV1.h | Tool | Scene Editor | Merge |
| TerrainBrushV1.h | Tool | Scene Editor | Merge |
| RoadToolV1.h | Tool | Scene Editor | Merge |
| DecalEditor.h | Tool | Scene Editor | Merge |
| LightEditor.h | Tool | Scene Editor | Merge |
| WaterEditor.h | Tool | Scene Editor | Merge |
| WaterSimEditor.h | Tool | Scene Editor | Merge |
| WeatherSystemEditor.h | Tool | Scene Editor | Merge |
| MaterialLayerEditor.h | Tool | Material Editor | Merge |
| MaterialNodeEditorV1.h | Tool | Material Editor | Merge |
| PostProcessEditor.h | Tool | Material Editor | Merge |
| TextureAtlasEditor.h | Tool | Asset Editor | Merge |
| TextureViewerV1.h | Tool | Asset Editor | Merge |
| SpriteEditor.h | Tool | Asset Editor | Merge |
| MeshInspectorV1.h | Tool | Asset Editor | Merge |
| MeshOptimizer.h | Tool | Asset Editor | Merge |
| MeshDeformerEditor.h | Tool | Asset Editor | Merge |
| ProceduralMeshEditor.h | Tool | Asset Editor | Merge |
| LODEditor.h | Tool | Asset Editor | Merge |
| FontEditor.h | Tool | Asset Editor | Merge |
| FontEditorV1.h | Tool | Asset Editor | Merge |
| AssetBundleEditor.h | Tool | Asset Editor | Merge |
| AssetPackager.h | Tool | Build Tool | Merge |
| AssetTagEditorV1.h | Tool | Asset Editor | Merge |
| AssetDependencyV1.h | Tool | Asset Editor | Merge |
| AssetDependencies.h | Tool | Asset Editor | Merge |
| AssetMigratorV1.h | Tool | Asset Editor | Merge |
| ContentPackEditor.h | Tool | Asset Editor | Merge |
| SkeletonEditor.h | Tool | Animation Editor | Merge |
| MorphTargetEditor.h | Tool | Animation Editor | Merge |
| CurveEditor.h | Tool | Animation Editor | Merge |
| CurveLibrary.h | Tool | Animation Editor | Merge |
| CurveLibraryV1.h | Tool | Animation Editor | Merge |
| AnimationCurveEditorV1.h | Tool | Animation Editor | Merge |
| CinematicsEditor.h | Tool | Animation Editor | Merge |
| CinematicEditorV1.h | Tool | Animation Editor | Merge |
| SequencerTrackV1.h | Tool | Animation Editor | Merge |
| TimelineMarkerV1.h | Tool | Animation Editor | Merge |
| TimelineSequencer.h | Tool | Animation Editor | Merge |
| CutsceneDirector.h | Tool | Animation Editor | Merge |
| CutsceneScriptEditor.h | Tool | Animation Editor | Merge |
| SequenceRecorder.h | Tool | Animation Editor | Merge |
| GradientEditor.h | Tool | Material Editor | Merge |
| GradientEditorV1.h | Tool | Material Editor | Merge |
| SplineEditor.h | Tool | Scene Editor | Merge |
| SplineEditorV1.h | Tool | Scene Editor | Merge |
| AudioClipEditor.h | Tool | Audio Mixer | Merge |
| AudioGraphEditorV1.h | Tool | Audio Mixer | Merge |
| SoundEffectEditor.h | Tool | Audio Mixer | Merge |
| SoundMixerEditorV1.h | Tool | Audio Mixer | Merge |
| SoundscapeEditor.h | Tool | Audio Mixer | Merge |
| VideoClipEditor.h | Tool | Asset Editor | Merge |
| ParticleSystemEditor.h | Tool | Particle Editor | Merge |
| ParticleSystemEditorV1.h | Tool | Particle Editor | Merge |
| VFXGraphEditor.h | Tool | Particle Editor | Merge |
| TrailEditor.h | Tool | Particle Editor | Merge |
| ImpactEffectEditor.h | Tool | Particle Editor | Merge |
| ClothSimEditor.h | Tool | Physics (Merge) | Merge |
| FluidSimEditor.h | Tool | Physics (Merge) | Merge |
| RopeSimEditor.h | Tool | Physics (Merge) | Merge |
| ShaderEditorV1.h | Tool | Material Editor | Merge |
| ShaderVariantEditor.h | Tool | Material Editor | Merge |
| CameraPathEditor.h | Tool | Animation Editor | Merge |
| CameraRigEditor.h | Tool | Scene Editor | Merge |
| CameraBlendEditor.h | Tool | Animation Editor | Merge |
| CameraShakeEditor.h | Tool | Animation Editor | Merge |
| AdvancedViewports.h | Tool | Scene Editor | Merge |
| DialogueEditor.h | Tool | Data Editor | Merge |
| SubtitleEditor.h | Tool | Data Editor | Merge |
| ActorDirector.h | Tool | Scene Editor | Merge |
| GameFlowGraph.h | Tool | Logic Editor | Merge |
| EventGraph.h | Tool | Logic Editor | Merge |
| StateGraphEditor.h | Tool | Logic Editor | Merge |
| StateGraphV1.h | Tool | Logic Editor | Merge |
| EventTimelineV1.h | Tool | Logic Editor | Merge |
| TriggerEditorV1.h | Tool | Logic Editor | Merge |
| TriggerVolumeEditor.h | Tool | Scene Editor | Merge |
| NavMeshEditor.h | Tool | Scene Editor | Merge |
| NetworkDebugger.h | Tool | Diagnostics | Merge |
| NetworkSyncEditorV1.h | Tool | Diagnostics | Merge |
| RpcInspectorV1.h | Tool | Diagnostics | Merge |
| LagCompEditorV1.h | Tool | Diagnostics | Merge |

## AI Editors (Merge into AtlasAI Tool)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| AIBehaviorEditor.h | Tool | AtlasAI | Merge |
| AIDecisionEditor.h | Tool | AtlasAI | Merge |
| AIGoalEditorV1.h | Tool | AtlasAI | Merge |
| AINavMeshEditorV1.h | Tool | AtlasAI | Merge |
| AIPathfindEditor.h | Tool | AtlasAI | Merge |
| AIPerceptionEditor.h | Tool | AtlasAI | Merge |
| AISpawnEditorV1.h | Tool | AtlasAI | Merge |

## Physics Editors (Merge into Scene/Physics Tool)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| PhysicsMaterialEditor.h | Tool | Scene Editor | Merge |
| PhysicsConstraintEditor.h | Tool | Scene Editor | Merge |
| PhysicsTriggerEditor.h | Tool | Scene Editor | Merge |
| ConstraintEditorV1.h | Tool | Scene Editor | Merge |
| RigidBodyEditorV1.h | Tool | Scene Editor | Merge |
| ColliderEditorV1.h | Tool | Scene Editor | Merge |

## Rendering Editors (Merge into Material/Scene)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| LightmapEditor.h | Tool | Scene Editor | Merge |
| LightmapEditorV1.h | Tool | Scene Editor | Merge |
| ReflectionProbeEditor.h | Tool | Scene Editor | Merge |
| ReflectionProbeEditorV1.h | Tool | Scene Editor | Merge |
| ShadowCasterEditorV1.h | Tool | Scene Editor | Merge |
| GraphicsSettingsEditor.h | Tool | Settings | Merge |
| ResolutionEditor.h | Tool | Settings | Merge |
| PipelineStateEditorV1.h | Tool | Material Editor | Merge |
| RenderPassEditorV1.h | Tool | Material Editor | Merge |

## Build/Packaging (Merge into Build Tool)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| CompilerSettingsV1.h | Tool | Build Tool | Merge |
| LinkerSettingsV1.h | Tool | Build Tool | Merge |
| PackageManagerV1.h | Tool | Build Tool | Merge |
| DependencyGraphV1.h | Tool | Build Tool | Merge |
| VersionResolverV1.h | Tool | Build Tool | Merge |
| PerformanceBudgetEditor.h | Tool | Build Tool | Merge |
| ProfilingSessionEditor.h | Tool | Diagnostics | Merge |
| CpuProfilerEditor.h | Tool | Diagnostics | Merge |
| GpuProfilerEditor.h | Tool | Diagnostics | Merge |
| DiagnosticProfiler.h | Tool | Diagnostics | Merge |
| DebugDrawEditor.h | Tool | Diagnostics | Merge |
| PlaymodeEditor.h | Tool | Scene Editor | Merge |
| SceneIsolationEditor.h | Tool | Scene Editor | Merge |
| SceneTreeEditorV1.h | Tool | Scene Editor | Merge |
| EntityQueryV1.h | Tool | Scene Editor | Merge |

## Data/Config Editors (Merge into Data Editor)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| EventBusEditor.h | Tool | Data Editor | Merge |
| MessageQueueEditor.h | Tool | Data Editor | Merge |
| PubSubEditor.h | Tool | Data Editor | Merge |
| TagSystemEditor.h | Tool | Data Editor | Merge |
| FilterEditor.h | Tool | Data Editor | Merge |
| SearchIndexEditor.h | Tool | Data Editor | Merge |
| AlertRuleEditor.h | Tool | Data Editor | Merge |
| SaveDataEditor.h | Tool | Data Editor | Merge |
| SyncConflictEditor.h | Tool | Data Editor | Merge |
| AbilitySystemEditor.h | Tool | Data Editor | Merge |

## Localization (Merge into Data/Asset Editor)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| LocalizationEditorV1.h | Tool | Data Editor | Merge |
| LocalizationKeyEditor.h | Tool | Data Editor | Merge |
| TranslationEditor.h | Tool | Data Editor | Merge |
| LanguagePackEditor.h | Tool | Data Editor | Merge |
| FontRegistryEditor.h | Tool | Asset Editor | Merge |
| IconEditor.h | Tool | Asset Editor | Merge |
| IconographySpec.h | Tool | Asset Editor | Merge |

## IDE/Development (Keep or Merge)

| Header | Current | Target | Action |
|--------|---------|--------|--------|
| IDEIntegration.h | Tool | Service | Service |
| Collaboration.h | Tool | Service | Service |
