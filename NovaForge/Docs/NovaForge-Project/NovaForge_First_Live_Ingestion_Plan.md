# NovaForge First Live Ingestion Plan

## Goal
Bring real gameplay-facing data into the new Master Repo runtime safely.

## This pass includes
- item definitions
- recipe definitions
- mission definitions
- faction definitions
- loot table definitions

## This pass excludes
- old engine code
- rendering code
- input code
- networking code
- monolithic gameplay actors

## Integration target folders
- /Data/Definitions/Items/
- /Data/Definitions/Recipes/
- /Data/Definitions/Missions/
- /Data/Definitions/Factions/
- /Data/Definitions/Loot/

## Suggested next runtime tasks
1. extend DataRegistry to scan these new folders
2. log loaded counts for each category
3. add a simple item/recipe lookup API
4. spawn a test mission/faction summary in startup logs
5. wire loot table validation into boot checks
