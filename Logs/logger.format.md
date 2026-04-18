# NovaForge — Logger Format

## File: Logs/build.logger

## Line Format
```
[LEVEL] [MODULE] [MESSAGE]
```

## Levels
- INFO
- WARN
- ERROR
- FATAL

## Example
```
[INFO] [Core] NovaForge Core initialized
[INFO] [Main] Version: 0.1.0
[ERROR] [Game] Failed to load world chunk (0,0,0)
[FATAL] [Editor] Cannot initialize renderer
```

## Routing
Atlas Workspace watches Logs/build.logger.
On ERROR or FATAL, it routes to AtlasAI for diagnosis and fix proposal.
The fix proposal is presented as a diff with accept/reject UI.
