#!/usr/bin/env python3
"""Atlas State Diff Viewer — Interactive CLI

Reads two state hash ladder files and shows a side-by-side diff,
highlighting ticks where hashes diverge.

Usage:
    python tools/state_diff_viewer.py <local_hashes.txt> <remote_hashes.txt>

Hash file format (one entry per line):
    tick:hash
Example:
    1:0xABCD1234
    2:0xDEADBEEF
"""

import sys
import os


def load_hash_ladder(path):
    """Load a hash ladder from a text file.

    Returns a dict mapping tick (int) -> hash (str).
    """
    entries = {}
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split(":")
            if len(parts) >= 2:
                try:
                    tick = int(parts[0])
                    hash_val = parts[1].strip()
                    entries[tick] = hash_val
                except ValueError:
                    continue
    return entries


def diff_ladders(local, remote):
    """Compare two hash ladders and return (matches, divergences).

    Each element is (tick, local_hash, remote_hash).
    """
    all_ticks = sorted(set(local.keys()) | set(remote.keys()))
    matches = []
    divergences = []
    for tick in all_ticks:
        lh = local.get(tick, "<missing>")
        rh = remote.get(tick, "<missing>")
        if lh == rh:
            matches.append((tick, lh, rh))
        else:
            divergences.append((tick, lh, rh))
    return matches, divergences


def print_report(local_path, remote_path, matches, divergences):
    """Print a human-readable diff report."""
    total = len(matches) + len(divergences)
    print("=" * 70)
    print("Atlas State Diff Viewer")
    print("=" * 70)
    print(f"  Local:   {local_path}")
    print(f"  Remote:  {remote_path}")
    print(f"  Ticks:   {total}")
    print(f"  Match:   {len(matches)}")
    print(f"  Diverge: {len(divergences)}")
    print("-" * 70)

    if not divergences:
        print("  ✅ No divergences detected — state hashes are identical.")
    else:
        print("  ❌ Divergences found:\n")
        print(f"  {'Tick':<10} {'Local Hash':<22} {'Remote Hash':<22}")
        print(f"  {'-'*10} {'-'*22} {'-'*22}")
        for tick, lh, rh in divergences:
            print(f"  {tick:<10} {lh:<22} {rh:<22}")

    print("=" * 70)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    local_path = sys.argv[1]
    remote_path = sys.argv[2]

    if not os.path.isfile(local_path):
        print(f"Error: file not found: {local_path}", file=sys.stderr)
        sys.exit(1)
    if not os.path.isfile(remote_path):
        print(f"Error: file not found: {remote_path}", file=sys.stderr)
        sys.exit(1)

    local = load_hash_ladder(local_path)
    remote = load_hash_ladder(remote_path)

    matches, divergences = diff_ladders(local, remote)
    print_report(local_path, remote_path, matches, divergences)

    sys.exit(0 if not divergences else 1)


if __name__ == "__main__":
    main()
