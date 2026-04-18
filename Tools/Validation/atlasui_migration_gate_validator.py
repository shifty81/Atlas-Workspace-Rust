#!/usr/bin/env python3
"""AtlasUI Migration Gate Validator — checks phase-specific requirements."""

import argparse
import json
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

# Phase gates: each phase requires certain files/dirs to exist
GATES = {
    0: {
        "description": "AtlasUI directories exist",
        "dirs": [
            "Source/UI/include/NF/UI/AtlasUI/",
            "Source/UI/src/AtlasUI/",
        ],
        "files": [],
    },
    1: {
        "description": "Theme authority installed",
        "dirs": [],
        "files": [
            "Source/UI/include/NF/UI/AtlasUI/Theme/ThemeManager.h",
            "Source/UI/include/NF/UI/AtlasUI/Theme/ThemeTypes.h",
            "Source/UI/include/NF/UI/AtlasUI/Theme/ThemeTokens.h",
            "Source/UI/src/AtlasUI/Theme/ThemeDefaults.cpp",
            "Source/UI/src/AtlasUI/Theme/ThemeManager.cpp",
        ],
    },
    2: {
        "description": "Text and clipping fixed",
        "dirs": [],
        "files": [],
    },
    3: {
        "description": "Services installed",
        "dirs": [],
        "files": [
            "Source/UI/include/NF/UI/AtlasUI/Services/FocusService.h",
            "Source/UI/include/NF/UI/AtlasUI/Services/PopupHost.h",
            "Source/UI/include/NF/UI/AtlasUI/Services/TooltipService.h",
            "Source/UI/include/NF/UI/AtlasUI/Services/NotificationHost.h",
        ],
    },
    4: {
        "description": "Command infrastructure installed",
        "dirs": [],
        "files": [
            "Source/UI/include/NF/UI/AtlasUI/Commands/CommandRegistry.h",
            "Source/UI/include/NF/UI/AtlasUI/Commands/CommandManager.h",
            "Source/UI/include/NF/UI/AtlasUI/Commands/ShortcutRouter.h",
        ],
    },
    5: {
        "description": "Core components live",
        "dirs": [],
        "files": [
            "Source/UI/include/NF/UI/AtlasUI/Widgets/Button.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/Panel.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/TabBar.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/Splitter.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/Toolbar.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/Tooltip.h",
        ],
    },
    6: {
        "description": "Form components live",
        "dirs": [],
        "files": [
            "Source/UI/include/NF/UI/AtlasUI/Widgets/TextInput.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/Dropdown.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/PropertyRow.h",
            "Source/UI/include/NF/UI/AtlasUI/Widgets/NotificationCard.h",
        ],
    },
}


def main():
    parser = argparse.ArgumentParser(description="AtlasUI migration gate validator")
    parser.add_argument(
        "--config",
        default=os.path.join(ROOT, "Tools", "Validation", "atlasui_validation_config.json"),
    )
    args = parser.parse_args()

    current_phase = 1
    if os.path.isfile(args.config):
        with open(args.config) as f:
            cfg = json.load(f)
        current_phase = cfg.get("current_phase", 1)

    print(f"AtlasUI Migration Gate — validating through phase {current_phase}\n")

    passed = 0
    failed = 0
    phase_descriptions = []

    for phase_num in range(current_phase + 1):
        gate = GATES.get(phase_num)
        if not gate:
            continue
        phase_descriptions.append(f"  Phase {phase_num}: {gate['description']}")

    for desc in phase_descriptions:
        print(desc)
    print()

    for phase_num in range(current_phase + 1):
        gate = GATES.get(phase_num)
        if not gate:
            continue

        for d in gate["dirs"]:
            full = os.path.join(ROOT, d)
            if os.path.isdir(full):
                print(f"  ✅  Phase {phase_num}: dir  {d}")
                passed += 1
            else:
                print(f"  ❌  Phase {phase_num}: dir  {d} — MISSING")
                failed += 1

        for fp in gate["files"]:
            full = os.path.join(ROOT, fp)
            if os.path.isfile(full):
                print(f"  ✅  Phase {phase_num}: file {fp}")
                passed += 1
            else:
                print(f"  ❌  Phase {phase_num}: file {fp} — MISSING")
                failed += 1

    print(f"\n  {passed} passed, {failed} failed\n")

    if failed > 0:
        print("❌  Migration gate validation failed.")
        return 1

    print("✅  Migration gate validation passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
