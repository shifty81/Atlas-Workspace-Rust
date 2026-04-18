# AtlasToolingSuite — Usable Snippets

This directory contains extracted reusable code patterns from the AtlasToolingSuite
repository that may serve as reference for future development.

## Widget Pattern

All AtlasUI widgets follow this pattern:

```cpp
class MyWidget final : public WidgetBase {
public:
    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;
};
```

## Panel Pattern

All panels derive from PanelBase:

```cpp
class MyPanel final : public PanelBase {
public:
    MyPanel() : PanelBase("my_panel", "My Panel") {}
    void paint(IPaintContext& context) override;
};
```

## Theme Token Usage

Widgets reference theme tokens instead of hardcoded colors:

```cpp
context.fillRect(rect, Theme::ColorToken::Surface);
context.drawText(rect, text, 0, Theme::ColorToken::Text);
float padding = Theme::Spacing::Small;
```

## Command Registration

Commands are registered in the CommandRegistry:

```cpp
CommandRegistry registry;
registry.registerCommand({
    "file.save", "Save File", "File",
    MakeChord('S', true)  // Ctrl+S
});
```

## Validation Scripts

Run validators from project root:

```bash
python3 Tools/Validation/atlasui_path_validator.py
python3 Tools/Validation/atlasui_symbol_validator.py
python3 Tools/Validation/atlasui_theme_validator.py
python3 Tools/Validation/atlasui_migration_gate_validator.py
```
