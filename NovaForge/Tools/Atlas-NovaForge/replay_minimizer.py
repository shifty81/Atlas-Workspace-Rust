#!/usr/bin/env python3
"""
Atlas Replay Minimizer

Automatically reduces a failing replay to its minimal reproduction case.
Uses binary search to find the shortest prefix that still reproduces the failure.

Usage:
    python tools/replay_minimizer.py --replay PATH --test-command CMD [--output PATH]

Example:
    python tools/replay_minimizer.py \
        --replay tests/replays/failing.atlasreplay \
        --test-command "./build/tests/AtlasTests --replay {replay}" \
        --output tests/replays/minimal.atlasreplay

The test command should return non-zero exit code on failure.
Use {replay} as a placeholder for the replay file path.

See: docs/NEXT_IMPLEMENTATION_TASKS.md
     docs/14_REPLAY_AND_PROOFS.md
"""

import argparse
import pathlib
import subprocess
import sys
import shutil
import tempfile
import struct
from typing import Optional, List, Tuple


class ReplayFrame:
    """Represents a single frame in an Atlas replay."""
    
    def __init__(self, tick: int, data: bytes):
        self.tick = tick
        self.data = data
    
    def to_bytes(self) -> bytes:
        """Serialize frame to bytes: [tick:u64][data_len:u32][data:bytes]"""
        header = struct.pack('<QI', self.tick, len(self.data))
        return header + self.data
    
    @staticmethod
    def from_bytes(data: bytes, offset: int = 0) -> Tuple['ReplayFrame', int]:
        """Deserialize frame from bytes. Returns (frame, next_offset)."""
        if len(data) - offset < 12:  # Minimum header size
            raise ValueError("Insufficient data for frame header")
        
        tick, data_len = struct.unpack('<QI', data[offset:offset+12])
        offset += 12
        
        if len(data) - offset < data_len:
            raise ValueError("Insufficient data for frame payload")
        
        frame_data = data[offset:offset+data_len]
        offset += data_len
        
        return ReplayFrame(tick, frame_data), offset


class AtlasReplay:
    """Represents an Atlas replay file."""
    
    MAGIC = b'ATLSRPLY'
    VERSION = 1
    
    def __init__(self, frames: List[ReplayFrame], metadata: bytes = b''):
        self.frames = frames
        self.metadata = metadata
    
    def to_bytes(self) -> bytes:
        """Serialize replay to .atlasreplay binary format."""
        # Header: [magic:8][version:u32][metadata_len:u32][frame_count:u32]
        header = self.MAGIC
        header += struct.pack('<III', self.VERSION, len(self.metadata), len(self.frames))
        
        # Metadata
        result = header + self.metadata
        
        # Frames
        for frame in self.frames:
            result += frame.to_bytes()
        
        return result
    
    @staticmethod
    def from_file(filepath: pathlib.Path) -> 'AtlasReplay':
        """Load replay from .atlasreplay file."""
        data = filepath.read_bytes()
        
        # Parse header
        if len(data) < 20:
            raise ValueError("Invalid replay file: too short")
        
        magic = data[0:8]
        if magic != AtlasReplay.MAGIC:
            raise ValueError(f"Invalid replay file: bad magic {magic}")
        
        version, metadata_len, frame_count = struct.unpack('<III', data[8:20])
        if version != AtlasReplay.VERSION:
            raise ValueError(f"Unsupported replay version: {version}")
        
        offset = 20
        
        # Parse metadata
        if len(data) - offset < metadata_len:
            raise ValueError("Invalid replay file: metadata too short")
        metadata = data[offset:offset+metadata_len]
        offset += metadata_len
        
        # Parse frames
        frames = []
        for _ in range(frame_count):
            frame, offset = ReplayFrame.from_bytes(data, offset)
            frames.append(frame)
        
        return AtlasReplay(frames, metadata)
    
    def save(self, filepath: pathlib.Path) -> None:
        """Save replay to .atlasreplay file."""
        filepath.write_bytes(self.to_bytes())
    
    def slice(self, end_frame: int) -> 'AtlasReplay':
        """Create a new replay with only frames [0, end_frame)."""
        return AtlasReplay(self.frames[:end_frame], self.metadata)


def run_test(replay_path: pathlib.Path, test_command: str, verbose: bool = False) -> bool:
    """
    Run test command with the given replay.
    Returns True if the test fails (reproduces the issue).
    """
    cmd = test_command.replace('{replay}', str(replay_path))
    
    try:
        result = subprocess.run(
            cmd,
            shell=True,
            stdout=subprocess.PIPE if not verbose else None,
            stderr=subprocess.PIPE if not verbose else None,
            timeout=60,
        )
        return result.returncode != 0
    except subprocess.TimeoutExpired:
        if verbose:
            print("  ⏱ Test timed out (considered a failure)")
        return True
    except Exception as e:
        if verbose:
            print(f"  ❌ Error running test: {e}")
        return False


def minimize_replay(
    replay_path: pathlib.Path,
    test_command: str,
    output_path: Optional[pathlib.Path] = None,
    verbose: bool = False,
) -> Optional[pathlib.Path]:
    """
    Minimize a replay using binary search.
    Returns the path to the minimized replay, or None if minimization failed.
    """
    if verbose:
        print(f"Loading replay: {replay_path}")
    
    # Load original replay
    try:
        replay = AtlasReplay.from_file(replay_path)
    except Exception as e:
        print(f"❌ Failed to load replay: {e}", file=sys.stderr)
        return None
    
    total_frames = len(replay.frames)
    if verbose:
        print(f"Original replay: {total_frames} frames")
    
    # Verify the original replay actually fails
    if verbose:
        print("Verifying original replay fails...")
    if not run_test(replay_path, test_command, verbose):
        print("❌ Original replay does not fail — nothing to minimize", file=sys.stderr)
        return None
    
    if verbose:
        print("✓ Original replay fails — proceeding with minimization")
    
    # Binary search for minimum failing frame count
    with tempfile.TemporaryDirectory() as tmpdir:
        temp_replay_path = pathlib.Path(tmpdir) / "test.atlasreplay"
        
        # Start with the full replay as the best known failing case
        min_failing_frames = total_frames
        
        # Binary search bounds: looking for smallest N where replay[0:N] fails
        left = 1
        right = total_frames  # Full replay already verified to fail
        
        iteration = 0
        while left < right:
            iteration += 1
            mid = (left + right) // 2
            
            if verbose:
                print(f"\n[Iteration {iteration}] Testing frames [0, {mid}) of {total_frames}")
            
            # Create sliced replay
            sliced_replay = replay.slice(mid)
            sliced_replay.save(temp_replay_path)
            
            # Test if this slice still fails
            if run_test(temp_replay_path, test_command, verbose):
                if verbose:
                    print(f"  ✓ Slice [0, {mid}) still fails")
                # This slice fails, try smaller
                right = mid
                min_failing_frames = mid
            else:
                if verbose:
                    print(f"  ✗ Slice [0, {mid}) passes")
                # This slice passes, need more frames
                left = mid + 1
    
    # Create final minimized replay
    minimized_replay = replay.slice(min_failing_frames)
    
    if output_path is None:
        stem = replay_path.stem
        output_path = replay_path.parent / f"{stem}_minimal.atlasreplay"
    
    minimized_replay.save(output_path)
    
    reduction_pct = 100.0 * (1.0 - min_failing_frames / total_frames)
    
    if verbose:
        print(f"\n✅ Minimization complete!")
        print(f"   Original: {total_frames} frames")
        print(f"   Minimal:  {min_failing_frames} frames")
        print(f"   Reduction: {reduction_pct:.1f}%")
        print(f"   Output: {output_path}")
    
    return output_path


def main():
    parser = argparse.ArgumentParser(
        description="Atlas Replay Minimizer — Reduce failing replays to minimal reproduction",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example:
  python tools/replay_minimizer.py \\
      --replay tests/replays/failing.atlasreplay \\
      --test-command "./build/tests/AtlasTests --replay {replay}" \\
      --output tests/replays/minimal.atlasreplay
        """,
    )
    parser.add_argument(
        '--replay',
        type=pathlib.Path,
        required=True,
        help='Path to the failing .atlasreplay file',
    )
    parser.add_argument(
        '--test-command',
        required=True,
        help='Command to test replay (use {replay} as placeholder)',
    )
    parser.add_argument(
        '--output',
        type=pathlib.Path,
        help='Output path for minimized replay (default: <input>_minimal.atlasreplay)',
    )
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Verbose output',
    )
    
    args = parser.parse_args()
    
    if not args.replay.exists():
        print(f"❌ Replay file not found: {args.replay}", file=sys.stderr)
        sys.exit(2)
    
    if '{replay}' not in args.test_command:
        print("❌ Test command must contain {replay} placeholder", file=sys.stderr)
        sys.exit(2)
    
    result = minimize_replay(
        args.replay,
        args.test_command,
        args.output,
        args.verbose,
    )
    
    if result is None:
        sys.exit(1)
    
    print(f"✅ Minimized replay saved: {result}")
    sys.exit(0)


if __name__ == '__main__':
    main()
