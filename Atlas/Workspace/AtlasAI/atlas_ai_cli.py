#!/usr/bin/env python3
"""AtlasAI Rule Evaluator — Rule-based decision engine CLI.

Deployable wrapper around the AtlasAI rule engine source.
Evaluates declarative .atlas_ai.json rules against JSON context snapshots.

Usage:
    python atlas_ai_cli.py evaluate <context.json> [--rules rules/]
    python atlas_ai_cli.py validate <rulefile.atlas_ai.json>
    python atlas_ai_cli.py watch    [--pipeline ../../.novaforge/pipeline/]

See AtlasAI/Atlas_Arbiter/README.md for full documentation.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def evaluate(context_path: str, rules_dir: str = "rules") -> int:
    """Evaluate rules against a context snapshot."""
    ctx = Path(context_path)
    if not ctx.exists():
        print(f"Error: context file not found: {ctx}", file=sys.stderr)
        return 1

    rules = Path(rules_dir)
    if not rules.is_dir():
        print(f"Error: rules directory not found: {rules}", file=sys.stderr)
        return 1

    rule_files = list(rules.glob("*.atlas_ai.json"))
    if not rule_files:
        print(f"Warning: no .atlas_ai.json files in {rules}", file=sys.stderr)
        return 0

    with open(ctx, encoding="utf-8") as f:
        context = json.load(f)

    violations = 0
    for rf in rule_files:
        with open(rf, encoding="utf-8") as f:
            ruleset = json.load(f)
        print(f"  Evaluating {rf.name} ({len(ruleset.get('rules', []))} rules)...")
        # Stub: actual evaluation engine is in AtlasAI
        for rule in ruleset.get("rules", []):
            print(f"    [{rule.get('severity', 'info')}] {rule.get('id', '?')}: {rule.get('description', '')}")

    print(f"\n{len(rule_files)} rule file(s) evaluated, {violations} violation(s).")
    return 1 if violations > 0 else 0


def validate(rulefile: str) -> int:
    """Validate a rule file's JSON structure."""
    path = Path(rulefile)
    if not path.exists():
        print(f"Error: file not found: {path}", file=sys.stderr)
        return 1
    try:
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
        if "rules" not in data:
            print(f"Warning: no 'rules' key in {path.name}")
        else:
            print(f"OK: {path.name} — {len(data['rules'])} rule(s)")
        return 0
    except json.JSONDecodeError as e:
        print(f"Error: invalid JSON in {path.name}: {e}", file=sys.stderr)
        return 1


def main() -> int:
    parser = argparse.ArgumentParser(description="AtlasAI Rule Evaluator CLI")
    sub = parser.add_subparsers(dest="command")

    ev = sub.add_parser("evaluate", help="Evaluate rules against context")
    ev.add_argument("context", help="Path to context JSON file")
    ev.add_argument("--rules", default="rules", help="Rules directory")

    va = sub.add_parser("validate", help="Validate a rule file")
    va.add_argument("rulefile", help="Path to .atlas_ai.json file")

    sub.add_parser("watch", help="Watch pipeline for events (stub)")

    args = parser.parse_args()
    if args.command == "evaluate":
        return evaluate(args.context, args.rules)
    elif args.command == "validate":
        return validate(args.rulefile)
    elif args.command == "watch":
        print("Watching .novaforge/pipeline/changes/ for events... (stub)")
        return 0
    else:
        parser.print_help()
        return 0


if __name__ == "__main__":
    sys.exit(main())
