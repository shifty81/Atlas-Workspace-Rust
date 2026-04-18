# Command, Shortcut, and Menu Standard

## Command system rule
Development-facing actions should bind through one shared command system.

## Required command concerns
- command ids
- labels
- shortcut chords
- enable/disable evaluation
- scope handling
- toolbar/menu binding
- tooltip integration
- AtlasAI action compatibility

## Shortcut routing rule
Support:
- global shortcuts
- focused-surface shortcuts
- panel-local shortcuts
- reserved shell shortcuts

Text input and editor contexts must have proper precedence.

## Menu system rule
Use one AtlasUI menu/context menu model for:
- top menu bar
- dropdown menus
- context menus
- submenu trees
- command palette integration later

## Minimum expected defaults
- File menu
- Edit menu
- View menu
- Tools menu
- Window menu
- Help menu
