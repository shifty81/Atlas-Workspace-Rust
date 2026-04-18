#!/usr/bin/env python3
"""
Atlas Crash Reporter

Bundles crash report artifacts (.atlascrash manifests) into a single
portable archive for upload, sharing, or CI artifact collection.

Given an output directory produced by DesyncReproducer (containing
.atlascrash, .asav, .rply, and _report.txt files), this tool:

  1. Validates the crash bundle manifest
  2. Collects all referenced files
  3. Generates a hash ladder summary
  4. Produces a single .tar.gz archive for sharing

Usage:
    python tools/crash_reporter.py --dir /tmp/atlas_repro [--output crash_bundle.tar.gz]

See: docs/NEXT_IMPLEMENTATION_TASKS.md
     engine/sim/DesyncReproducer.h
"""

import argparse
import hashlib
import os
import pathlib
import sys
import tarfile
import tempfile
from typing import Dict, List, Optional


class CrashManifest:
    """Parsed .atlascrash manifest file."""

    def __init__(self) -> None:
        self.version: str = ""
        self.engine_version: str = ""
        self.platform: str = ""
        self.tick: int = 0
        self.tick_rate: int = 0
        self.seed: int = 0
        self.local_hash: str = ""
        self.remote_hash: str = ""
        self.save_path: str = ""
        self.replay_path: str = ""
        self.report_path: str = ""
        self.repro_command: str = ""
        self.manifest_path: str = ""

    @staticmethod
    def parse(filepath: pathlib.Path) -> "CrashManifest":
        """Parse a .atlascrash manifest file."""
        manifest = CrashManifest()
        manifest.manifest_path = str(filepath)

        text = filepath.read_text(errors="ignore")
        for line in text.splitlines():
            line = line.strip()
            if line == "atlas_crash_bundle_v1":
                manifest.version = "v1"
                continue
            if "=" not in line:
                continue
            key, _, value = line.partition("=")
            key = key.strip()
            value = value.strip()

            if key == "engine_version":
                manifest.engine_version = value
            elif key == "platform":
                manifest.platform = value
            elif key == "tick":
                manifest.tick = int(value)
            elif key == "tick_rate":
                manifest.tick_rate = int(value)
            elif key == "seed":
                manifest.seed = int(value)
            elif key == "local_hash":
                manifest.local_hash = value
            elif key == "remote_hash":
                manifest.remote_hash = value
            elif key == "save":
                manifest.save_path = value
            elif key == "replay":
                manifest.replay_path = value
            elif key == "report":
                manifest.report_path = value
            elif key == "repro":
                manifest.repro_command = value

        return manifest

    def validate(self) -> List[str]:
        """Validate manifest and referenced files. Returns list of errors."""
        errors: List[str] = []

        if self.version != "v1":
            errors.append(f"Unsupported manifest version: '{self.version}'")

        if not self.engine_version:
            errors.append("Missing engine_version")

        if self.tick <= 0:
            errors.append(f"Invalid tick: {self.tick}")

        if self.tick_rate <= 0:
            errors.append(f"Invalid tick_rate: {self.tick_rate}")

        # Check referenced files exist
        for desc, path in [
            ("save", self.save_path),
            ("replay", self.replay_path),
            ("report", self.report_path),
        ]:
            if not path:
                errors.append(f"Missing {desc} path")
            elif not pathlib.Path(path).exists():
                errors.append(f"{desc} file not found: {path}")

        return errors


def compute_file_hash(filepath: pathlib.Path) -> str:
    """Compute SHA-256 hash of a file."""
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest()


def find_manifests(directory: pathlib.Path) -> List[pathlib.Path]:
    """Find all .atlascrash manifest files in a directory."""
    manifests = sorted(directory.glob("*.atlascrash"))
    return manifests


def generate_hash_ladder(manifest: CrashManifest) -> str:
    """Generate a hash ladder summary for debugging."""
    lines = [
        "=== Atlas Hash Ladder ===",
        f"Engine:     {manifest.engine_version}",
        f"Platform:   {manifest.platform}",
        f"Tick:       {manifest.tick}",
        f"Tick Rate:  {manifest.tick_rate}",
        f"Seed:       {manifest.seed}",
        "",
        "Hash Comparison:",
        f"  Local:  {manifest.local_hash}",
        f"  Remote: {manifest.remote_hash}",
        f"  Match:  {'YES' if manifest.local_hash == manifest.remote_hash else 'NO — DESYNC'}",
        "",
        "File Integrity:",
    ]

    for desc, path_str in [
        ("Save", manifest.save_path),
        ("Replay", manifest.replay_path),
        ("Report", manifest.report_path),
    ]:
        path = pathlib.Path(path_str) if path_str else None
        if path and path.exists():
            file_hash = compute_file_hash(path)
            size = path.stat().st_size
            lines.append(f"  {desc}: {file_hash[:16]}... ({size} bytes)")
        else:
            lines.append(f"  {desc}: MISSING")

    lines.append("")
    lines.append("Reproduction:")
    lines.append(f"  {manifest.repro_command}")

    return "\n".join(lines)


def _is_safe_path(path: pathlib.Path, directory: pathlib.Path) -> bool:
    """Check that path is inside or a direct artifact of the given directory."""
    try:
        resolved = path.resolve()
        dir_resolved = directory.resolve()
        return resolved == dir_resolved or dir_resolved in resolved.parents
    except (OSError, ValueError):
        return False


def bundle_crash_report(
    directory: pathlib.Path,
    output_path: Optional[pathlib.Path] = None,
    verbose: bool = False,
) -> Optional[pathlib.Path]:
    """
    Bundle all crash report artifacts into a single .tar.gz archive.
    Returns the path to the archive, or None on failure.
    """
    manifests = find_manifests(directory)
    if not manifests:
        print(f"❌ No .atlascrash manifests found in {directory}", file=sys.stderr)
        return None

    if verbose:
        print(f"Found {len(manifests)} crash manifest(s)")

    # Parse and validate all manifests
    parsed: List[CrashManifest] = []
    for mpath in manifests:
        if verbose:
            print(f"  Parsing: {mpath.name}")
        manifest = CrashManifest.parse(mpath)
        errors = manifest.validate()
        if errors:
            print(f"⚠ Manifest {mpath.name} has issues:", file=sys.stderr)
            for err in errors:
                print(f"    • {err}", file=sys.stderr)
        parsed.append(manifest)

    if not parsed:
        print("❌ No valid crash manifests found", file=sys.stderr)
        return None

    # Generate default output path
    if output_path is None:
        output_path = directory / "atlas_crash_bundle.tar.gz"

    # Generate hash ladder summaries
    hash_ladder_dir = pathlib.Path(tempfile.mkdtemp())
    for manifest in parsed:
        ladder = generate_hash_ladder(manifest)
        ladder_file = hash_ladder_dir / f"hash_ladder_tick_{manifest.tick}.txt"
        ladder_file.write_text(ladder)
        if verbose:
            print(f"  Generated hash ladder for tick {manifest.tick}")

    # Build archive
    files_added = 0
    with tarfile.open(str(output_path), "w:gz") as tar:
        # Add manifests
        for mpath in manifests:
            tar.add(str(mpath), arcname=mpath.name)
            files_added += 1

        # Add referenced files from each manifest
        for manifest in parsed:
            for path_str in [
                manifest.save_path,
                manifest.replay_path,
                manifest.report_path,
            ]:
                if path_str:
                    path = pathlib.Path(path_str)
                    if path.exists() and _is_safe_path(path, directory):
                        tar.add(str(path), arcname=path.name)
                        files_added += 1
                    elif path.exists() and verbose:
                        print(f"  ⚠ Skipping file outside bundle directory: {path}")

        # Add hash ladders
        for ladder_file in sorted(hash_ladder_dir.glob("*.txt")):
            tar.add(str(ladder_file), arcname=ladder_file.name)
            files_added += 1

        # Add a top-level summary
        summary = generate_bundle_summary(parsed)
        summary_path = hash_ladder_dir / "CRASH_SUMMARY.txt"
        summary_path.write_text(summary)
        tar.add(str(summary_path), arcname="CRASH_SUMMARY.txt")
        files_added += 1

    if verbose:
        size = output_path.stat().st_size
        print(f"\n✅ Bundle created: {output_path}")
        print(f"   Files: {files_added}")
        print(f"   Size: {size} bytes")

    return output_path


def generate_bundle_summary(manifests: List[CrashManifest]) -> str:
    """Generate a human-readable summary of all crash reports in the bundle."""
    lines = [
        "=" * 60,
        "ATLAS CRASH REPORT BUNDLE",
        "=" * 60,
        "",
        f"Total crash reports: {len(manifests)}",
        "",
    ]

    for i, m in enumerate(manifests):
        lines.append(f"--- Crash Report #{i + 1} ---")
        lines.append(f"  Engine Version: {m.engine_version}")
        lines.append(f"  Platform:       {m.platform}")
        lines.append(f"  Tick:           {m.tick}")
        lines.append(f"  Tick Rate:      {m.tick_rate}")
        lines.append(f"  Seed:           {m.seed}")
        lines.append(f"  Local Hash:     {m.local_hash}")
        lines.append(f"  Remote Hash:    {m.remote_hash}")
        lines.append(f"  Desync:         {'YES' if m.local_hash != m.remote_hash else 'NO'}")
        lines.append(f"  Reproduce:      {m.repro_command}")
        lines.append("")

    lines.append("=" * 60)
    lines.append("To reproduce, extract the archive and run the repro command.")
    lines.append("=" * 60)

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Atlas Crash Reporter — Bundle crash report artifacts",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example:
  python tools/crash_reporter.py \\
      --dir /tmp/atlas_repro \\
      --output crash_bundle.tar.gz \\
      --verbose
        """,
    )
    parser.add_argument(
        "--dir",
        type=pathlib.Path,
        required=True,
        help="Directory containing .atlascrash manifests and artifacts",
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        help="Output path for the crash bundle archive (default: <dir>/atlas_crash_bundle.tar.gz)",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Verbose output",
    )
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="Only validate manifests without creating a bundle",
    )

    args = parser.parse_args()

    if not args.dir.exists():
        print(f"❌ Directory not found: {args.dir}", file=sys.stderr)
        sys.exit(2)

    if args.validate_only:
        manifests = find_manifests(args.dir)
        if not manifests:
            print(f"❌ No .atlascrash manifests found in {args.dir}")
            sys.exit(1)

        all_valid = True
        for mpath in manifests:
            manifest = CrashManifest.parse(mpath)
            errors = manifest.validate()
            if errors:
                print(f"❌ {mpath.name}:")
                for err in errors:
                    print(f"    • {err}")
                all_valid = False
            else:
                print(f"✅ {mpath.name}: valid")

        sys.exit(0 if all_valid else 1)

    result = bundle_crash_report(args.dir, args.output, args.verbose)

    if result is None:
        sys.exit(1)

    print(f"✅ Crash bundle saved: {result}")
    sys.exit(0)


if __name__ == "__main__":
    main()
