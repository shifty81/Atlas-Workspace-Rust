# Archive: shifty81/AtlasToolingSuite

**Archived:** 2026-04-05
**Source:** https://github.com/shifty81/AtlasToolingSuite
**Merge Phase:** C2 — AtlasToolingSuite
**Status:** ✅ Merged

## What Was Here

AtlasToolingSuite was the comprehensive UI framework, tooling, and spec pack
repository. It provided:

- **AtlasUI Framework** — Full widget kit, theme/token system, command/shortcut
  system, services (focus, popup, tooltip, notification), panel host, and draw
  primitives. All under `NF::UI::AtlasUI` namespace.
- **Spec Rollup Pack** — 12 specification documents defining locked project
  direction, AtlasUI standards, panel rewrite standard, theme/token system,
  command/shortcut/menu standard, docking/floating/chrome, workspace/AI
  integration, roadmap priority, repo audit checklist, feature gap matrix, next
  artifacts queue, and recommended repo structure.
- **Starter Packs** — Pre-built implementation scaffolds for the widget kit,
  command/shortcut system, and theme/token system.
- **Validation Suite** — 4 Python validators (path, symbol, theme, migration-gate)
  plus CI workflow for continuous compliance checking.

## Items Taken → tempnovaforge

| Source | Destination | Status |
|--------|-------------|--------|
| AtlasUI widget framework | `Source/UI/include/NF/UI/AtlasUI/` | ✅ Merged |
| AtlasUI widget implementations | `Source/UI/src/AtlasUI/` | ✅ Merged |
| Theme/token system | `Source/UI/include/NF/UI/AtlasUI/Theme/` | ✅ Merged |
| Command/shortcut system | `Source/UI/include/NF/UI/AtlasUI/Commands/` | ✅ Merged |
| Services (Focus, Popup, Tooltip, Notification) | `Source/UI/src/AtlasUI/Services/` | ✅ Merged |
| Spec rollup documents | `Docs/SpecRollup/` | ✅ Merged |
| Validation scripts | `Tools/Validation/` | ✅ Merged |
| CI validation workflow | `.github/workflows/atlasui_validation.yml` | ✅ Merged |
| Widget kit starter patch | `Additions/atlasui_widget_kit_starter_patch/` | ✅ Integrated → archived |
| Command shortcut starter pack | `Additions/command_shortcut_system_v1_starter_pack/` | ✅ Integrated → archived |
| Theme token starter pack | `Additions/theme_token_system_v1_starter_pack/` | ✅ Integrated → archived |
| Spec rollup starter pack | `Additions/atlas_workspace_novaforge_spec_rollup_pack/` | ✅ Integrated → archived |

## Key Context

- AtlasUI framework added 72+ source files across headers and implementations
- 36 AtlasUI-specific tests added to the test suite (708 total tests at archive time)
- 4 Python validation scripts enforce path, symbol, theme, and migration-gate compliance
- All widget implementations use the `NF::UI::AtlasUI` namespace consistently
- Theme system uses `uint32_t` packed Color internally (`DrawPrimitives.h`) and
  `ColorRGBA` floats in theme tokens (`ThemeTokens.h`)
- The Additions/ directory contained starter packs that have been fully integrated
  into the Source/UI/ tree; originals remain as reference in Archive

## Namespace Convention

- Framework: `NF::UI::AtlasUI`
- Theme tokens: `NF::UI::AtlasUI::Theme`
- Commands: `NF::UI::AtlasUI` (CommandRegistry, CommandManager, ShortcutRouter)
- Services: `NF::UI::AtlasUI` (FocusService, PopupHost, TooltipService, NotificationHost)

## Original Repo

https://github.com/shifty81/AtlasToolingSuite (to be archived once user confirms)
