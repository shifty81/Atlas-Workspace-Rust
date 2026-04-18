#!/usr/bin/env python3
"""AtlasUI Theme Validator — enforces token-based styling in AtlasUI widgets."""

import argparse
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

ATLASUI_INCLUDE = "Source/UI/include/NF/UI/AtlasUI"
ATLASUI_SRC = "Source/UI/src/AtlasUI"

# Raw Color(...) float construction — banned outside theme defaults
RAW_COLOR_RE = re.compile(
    r"\bColor\s*\(\s*[-+]?\d*\.?\d+f?\s*,"
    r"\s*[-+]?\d*\.?\d+f?\s*,"
    r"\s*[-+]?\d*\.?\d+f?"
    r"(?:\s*,\s*[-+]?\d*\.?\d+f?)?\s*\)"
)

# Paint-like methods
PAINT_RE = re.compile(r"\b(Paint|paint|Draw|draw|Render|render)\s*\(")

# ThemeManager or Theme:: access
THEME_ACCESS_RE = re.compile(r"ThemeManager::Get|Theme::")

ALLOW_MARKER = "ATLASUI_VALIDATE_ALLOW"

DEFAULT_ALLOWED_FILES = [
    "Source/UI/src/AtlasUI/Theme/ThemeDefaults.cpp",
    "Source/UI/include/NF/UI/AtlasUI/Theme/ThemeTypes.h",
    "Source/UI/include/NF/UI/AtlasUI/Theme/ThemeTokens.h",
    "Source/UI/include/NF/UI/AtlasUI/DrawPrimitives.h",
    "Source/UI/include/NF/UI/AtlasUI/WidgetTheme.h",
]


def load_config(config_path):
    allowed = set(DEFAULT_ALLOWED_FILES)
    if config_path and os.path.isfile(config_path):
        with open(config_path) as f:
            cfg = json.load(f)
        for fp in cfg.get("allowed_theme_default_files", []):
            allowed.add(fp)
    return allowed


def collect_atlasui_files():
    files = []
    for base in (ATLASUI_INCLUDE, ATLASUI_SRC):
        full = os.path.join(ROOT, base)
        if not os.path.isdir(full):
            continue
        for dirpath, _, filenames in os.walk(full):
            for fn in filenames:
                if fn.endswith((".h", ".hpp", ".cpp", ".cxx", ".cc", ".c")):
                    rel = os.path.relpath(os.path.join(dirpath, fn), ROOT)
                    files.append(rel)
    return files


def main():
    parser = argparse.ArgumentParser(description="AtlasUI theme validator")
    parser.add_argument("--changed-files", help="File with changed file list")
    parser.add_argument("--phase", type=int, default=1)
    parser.add_argument("--config", default=None)
    args = parser.parse_args()

    config_path = args.config or os.path.join(
        ROOT, "Tools", "Validation", "atlasui_validation_config.json"
    )
    allowed_files = load_config(config_path)

    files = collect_atlasui_files()
    if args.changed_files and os.path.isfile(args.changed_files):
        with open(args.changed_files) as f:
            changed = {l.strip() for l in f if l.strip()}
        files = [fp for fp in files if fp in changed]

    failures = []
    warnings = []

    for fp in files:
        if fp in allowed_files:
            continue

        fullpath = os.path.join(ROOT, fp)
        try:
            with open(fullpath, "r", encoding="utf-8", errors="replace") as f:
                content = f.read()
                lines = content.splitlines()
        except OSError:
            continue

        is_cpp = fp.endswith((".cpp", ".cxx", ".cc", ".c"))
        has_paint = bool(PAINT_RE.search(content))
        has_theme_access = bool(THEME_ACCESS_RE.search(content))

        # Check raw Color(...) in non-allowed files
        for i, line in enumerate(lines, 1):
            if ALLOW_MARKER in line:
                continue
            if RAW_COLOR_RE.search(line):
                warnings.append(f"WARN: {fp}:{i} — raw Color() construction")

    if failures:
        for msg in failures:
            print(msg)
        print(f"\n❌  Theme validation failed ({len(failures)} issue(s)).")
        return 1

    for msg in warnings:
        print(msg)
    warn_count = len(warnings)
    print(f"✅  Theme validation passed ({len(files)} file(s) checked, {warn_count} warning(s)).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
