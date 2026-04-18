# Workspace and AtlasAI Integration

## Workspace shell direction
Workspace should act like a cohesive development shell that hosts tools and panels consistently.

## Expected workspace-facing systems
- start bar / launcher behavior
- auto-hide shell bar behavior
- notification center
- AtlasAI slideout/chat surface
- tool launch surfaces
- settings/control panel
- project/repo flows
- file intake surfaces

## AtlasAI role
AtlasAI is the only visible broker AI layer.

Expected AtlasAI tooling behavior:
- chat interface
- markdown-style responses by default
- diff/change summary display
- apply/reject action surfaces
- debug workflow entry from notifications and logs
- integration with file intake and bug-fix flow

## Notification flow
Recommended direction:
- logger/build system emits events
- notification center shows actionable items
- relevant errors escalate into AtlasAI workflow
- user can open chat/diff/action flow from the notification

## Future integration expectations
- GitHub account linking
- Google account linking
- broader file/project intake
- codex/snippet mirroring
