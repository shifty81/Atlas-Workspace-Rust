# Docking, Floating, Chrome, and Tooltip Standard

## Docking
Docking is shell-owned, not panel-owned.

Required:
- dock tree
- split nodes
- tab stacks
- active tab selection
- closable tabs where allowed
- floatable panels where allowed
- serializable layout state

## Floating
Floating surfaces should be rehostable without changing panel code.

## Chrome
All docked and floating panels should use shared chrome:
- panel shell
- tab strip
- panel header behavior
- close/utility buttons
- rounded corners
- active/inactive visuals
- hover/press states

## Button behavior
Buttons should support:
- normal
- hover
- pressed
- disabled
- keyboard focus where relevant

Pressed state should include a visible depress behavior.

## Tooltip
Use a shared tooltip service.
Recommended behavior:
- slight hover delay
- concise tooltip content
- hide on drag/click/focus shift
- theme-consistent styling
- one active tooltip per host window
