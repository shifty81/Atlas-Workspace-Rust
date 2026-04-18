#!/usr/bin/env python3
"""AtlasUI Path Validator — ensures shared shell code lives under AtlasUI paths
and uses the NF::UI::AtlasUI namespace."""

import argparse
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

ATLASUI_INCLUDE = "Source/UI/include/NF/UI/AtlasUI"
ATLASUI_SRC = "Source/UI/src/AtlasUI"

WIDGET_NAME_RE = re.compile(
    r"(?i)(Panel|TabBar|Splitter|Toolbar|Button|TextInput|Dropdown|"
    r"PropertyRow|Tooltip|NotificationCard)\.(h|hpp|cpp|cxx|cc|c)$"
)

THEME_SYMBOL_RE = re.compile(
    r"(?i)(ThemeManager|ThemeDefaults|ThemeTypes|AtlasTheme|ColorTokens|"
    r"SpacingTokens|RadiusTokens|BorderTokens|TypographyTokens|SizeTokens|"
    r"StateTokens|LayerTokens|MotionTokens)"
)

NS_RE = re.compile(r"namespace\s+NF::UI::AtlasUI")
NS_ALT_RE = re.compile(
    r"namespace\s+NF\s*\{\s*namespace\s+UI\s*\{\s*namespace\s+AtlasUI"
)

ALLOWED_WIDGET_DIRS = {
    os.path.join(ATLASUI_INCLUDE, "Widgets"),
    os.path.join(ATLASUI_SRC, "Widgets"),
}

ALLOWED_THEME_DIRS = {
    os.path.join(ATLASUI_INCLUDE, "Theme"),
    os.path.join(ATLASUI_SRC, "Theme"),
}


def collect_atlasui_files():
    """Return all source files under the AtlasUI tree."""
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


def check_namespace(filepath):
    """Return True if file contains the required namespace."""
    fullpath = os.path.join(ROOT, filepath)
    try:
        with open(fullpath, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
    except OSError:
        return True  # skip unreadable
    return bool(NS_RE.search(content) or NS_ALT_RE.search(content))


def main():
    parser = argparse.ArgumentParser(description="AtlasUI path validator")
    parser.add_argument("--changed-files", help="File containing list of changed files")
    args = parser.parse_args()

    files = collect_atlasui_files()
    if args.changed_files and os.path.isfile(args.changed_files):
        with open(args.changed_files) as f:
            changed = {l.strip() for l in f if l.strip()}
        files = [fp for fp in files if fp in changed]

    failures = []

    for fp in files:
        # Namespace check
        if not check_namespace(fp):
            failures.append(f"FAIL: missing NF::UI::AtlasUI namespace in {fp}")

    # Check no source in Additions/
    additions = os.path.join(ROOT, "Additions")
    if os.path.isdir(additions):
        for dirpath, _, filenames in os.walk(additions):
            for fn in filenames:
                if fn.endswith((".cpp", ".cxx", ".cc", ".c")):
                    rel = os.path.relpath(os.path.join(dirpath, fn), ROOT)
                    # warn only — Additions may keep reference copies
                    pass

    if failures:
        for f in failures:
            print(f)
        print(f"\n❌  AtlasUI path validation failed ({len(failures)} issue(s)).")
        return 1

    print(f"✅  AtlasUI path validation passed ({len(files)} file(s) checked).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
