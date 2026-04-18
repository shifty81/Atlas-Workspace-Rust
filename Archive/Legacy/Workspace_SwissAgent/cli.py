#!/usr/bin/env python3
"""Atlas SwissAgent — Conversational AI query tool CLI.

Deployable wrapper around the AtlasAI/Atlas_SwissAgent source.
Provides workspace-aware code generation, analysis, and queries.

Usage:
    python cli.py run "your prompt here" [--llm-backend ollama]
    python cli.py session list
    python cli.py watch [--pipeline ../../.novaforge/pipeline/]

See AtlasAI/Atlas_SwissAgent/README.md for full documentation.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path


def run_prompt(prompt: str, llm_backend: str = "ollama") -> int:
    """Run a single prompt through the agent."""
    print(f"[SwissAgent] Backend: {llm_backend}")
    print(f"[SwissAgent] Prompt: {prompt}")
    print("[SwissAgent] (stub — connect AtlasAI/Atlas_SwissAgent core for execution)")
    return 0


def list_sessions() -> int:
    """List active sessions."""
    sessions_dir = Path("../Sessions")
    if not sessions_dir.is_dir():
        print("No sessions directory found.")
        return 0
    sessions = [d.name for d in sessions_dir.iterdir() if d.is_dir()]
    if not sessions:
        print("No active sessions.")
    else:
        for s in sessions:
            print(f"  {s}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Atlas SwissAgent CLI")
    sub = parser.add_subparsers(dest="command")

    rn = sub.add_parser("run", help="Run a prompt")
    rn.add_argument("prompt", help="Natural language prompt")
    rn.add_argument("--llm-backend", default="ollama",
                    choices=["ollama", "local", "api", "openwebui", "localai"],
                    help="LLM backend")

    sub.add_parser("session", help="Session management").add_subparsers(dest="session_cmd")

    sub.add_parser("watch", help="Watch pipeline for events (stub)")

    args = parser.parse_args()
    if args.command == "run":
        return run_prompt(args.prompt, args.llm_backend)
    elif args.command == "session":
        return list_sessions()
    elif args.command == "watch":
        print("Watching .novaforge/pipeline/changes/ for events... (stub)")
        return 0
    else:
        parser.print_help()
        return 0


if __name__ == "__main__":
    sys.exit(main())
