# Archive — Editor Overspray

These headers were removed from `Source/Editor/include/NF/Editor/` because they have no workspace-native authoring role and violate the 20-tool primary roster cap.

## Categories

### LiveOps / Multiplayer Simulation
No workspace authoring role. Liveops and multiplayer sim tooling are runtime/ops concerns.
- `ArcadeGameEditor.h`
- `MiniGameEditor.h`
- `PuzzleEditor.h`
- `LobbyEditor.h`
- `MatchmakingEditor.h`
- `MatchReplayEditor.h`
- `LiveOpsEditor.h`
- `SpectatorEditor.h`
- `TournamentEditor.h`

### Cloud / Infrastructure Ops
Not workspace authoring — runtime ops and cloud infrastructure concerns.
- `CloudStorageEditor.h`
- `CloudSyncEditor.h`
- `BroadcastEditor.h`
- `ExperimentEditor.h`
- `DeploymentEditor.h`
- `SandboxEditor.h`

### Load / Stress / Benchmark Testing Infrastructure
QA and CI infrastructure, not workspace authoring tools.
- `BenchmarkSuiteEditor.h`
- `LoadTestEditor.h`
- `StressTestEditor.h`

### Network Simulation
No workspace authoring role.
- `BandwidthProfileEditor.h`
- `LatencySimEditor.h`
- `PacketCapture.h`
- `NetworkTopologyEditor.h`
- `MultiplayerPreview.h`

### Accessibility / UI Testing
No workspace authoring role — testing/QA tooling.
- `AccessibilityEditor.h`
- `ColorblindSimulator.h`
- `MenuLayoutEditor.h`
- `DisplayModeEditor.h`

### Input Configuration
Folded into Settings Tool (`SettingsTool.h`). Standalone headers archived.
- `GamepadConfigurator.h`
- `ControlSchemeEditor.h`
- `AxisMappingEditor.h`
- `InputActionEditor.h`
- `InputBindingsEditor.h`
- `TouchInputMapper.h`

### Analytics / Telemetry Infrastructure
Runtime telemetry and analytics — not workspace authoring.
- `TelemetryEditor.h`
- `AnalyticsDashboard.h`
- `FeatureFlagEditor.h`
- `RuntimeConfigEditor.h`

### Game-Specific / Project Tools
No workspace-core role. Project-level concerns belong in per-project adapters.
- `MusicSequencer.h` — game-specific; if needed, belongs in project adapter
- `ModdingToolkit.h` — project-level concern
- `PlaytestRecorder.h` — QA tooling; project-level
- `LeaderboardEditor.h` — game-specific; NovaForge adapter if needed

### Duplicate / Superseded
- `NotificationCenterEditor.h` — superseded by `NotificationSystem.h` panel

### AI Sub-Panels (Archived)
These were standalone AI editor headers. AI behavior tooling belongs as sub-panels of
the Scene Editor or AtlasAI Tool, not as standalone workspace tools.
- `AIBehaviorEditor.h`
- `AIDecisionEditor.h`
- `AIGoalEditorV1.h`
- `AINavMeshEditorV1.h`
- `AIPathfindEditor.h`
- `AIPerceptionEditor.h`
- `AISpawnEditorV1.h`

## Notes

- Files here are preserved for historical reference only
- Do not include these headers in any active build targets
- If a tool is needed again, it should be reimplemented as a sub-panel of an appropriate primary tool or project adapter
