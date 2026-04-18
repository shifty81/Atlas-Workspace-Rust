#!/usr/bin/env python3
"""AtlasUI Symbol Validator — detects banned/deprecated symbol usage
in AtlasUI and migrated files."""

import argparse
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

ATLASUI_INCLUDE = "Source/UI/include/NF/UI/AtlasUI"
ATLASUI_SRC = "Source/UI/src/AtlasUI"

# Deprecated symbols — hard fail in AtlasUI / migrated files
BANNED_SYMBOLS = [
    (re.compile(r"\bUITheme\b"), "UITheme"),
    (re.compile(r"\bEditorTheme\b"), "EditorTheme"),
    (re.compile(r"\bEditorCommandRegistry\b"), "EditorCommandRegistry"),
    (re.compile(r"\bHotkeyDispatcher\b"), "HotkeyDispatcher"),
]

# Banned includes
BANNED_INCLUDES = [
    re.compile(r'^\s*#\s*include\s*["<].*EditorCommandRegistry.*[">]'),
    re.compile(r'^\s*#\s*include\s*["<].*HotkeyDispatcher.*[">]'),
    re.compile(r'^\s*#\s*include\s*["<].*UITheme.*[">]'),
    re.compile(r'^\s*#\s*include\s*["<].*EditorTheme.*[">]'),
]

ALLOW_MARKER = "ATLASUI_VALIDATE_ALLOW"


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


def scan_file(filepath):
    fullpath = os.path.join(ROOT, filepath)
    issues = []
    try:
        with open(fullpath, "r", encoding="utf-8", errors="replace") as f:
            lines = f.readlines()
    except OSError:
        return issues

    for i, line in enumerate(lines, 1):
        if ALLOW_MARKER in line:
            continue

        for pattern, name in BANNED_SYMBOLS:
            if pattern.search(line):
                issues.append((filepath, i, f"banned symbol '{name}'"))

        for inc_re in BANNED_INCLUDES:
            if inc_re.search(line):
                issues.append((filepath, i, "banned include"))

    return issues


def main():
    parser = argparse.ArgumentParser(description="AtlasUI symbol validator")
    parser.add_argument("--changed-files", help="File with changed file list")
    parser.add_argument("--phase", type=int, default=1, help="Migration phase")
    args = parser.parse_args()

    files = collect_atlasui_files()
    if args.changed_files and os.path.isfile(args.changed_files):
        with open(args.changed_files) as f:
            changed = {l.strip() for l in f if l.strip()}
        files = [fp for fp in files if fp in changed]

    failures = []
    warnings = []

    for fp in files:
        for issue in scan_file(fp):
            failures.append(issue)

    if failures:
        for fp, line, msg in failures:
            print(f"FAIL: {fp}:{line} — {msg}")
        print(f"\n❌  Symbol validation failed ({len(failures)} issue(s)).")
        return 1

    warn_count = len(warnings)
    print(f"✅  Symbol validation passed ({len(files)} file(s) checked, {warn_count} warning(s)).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
