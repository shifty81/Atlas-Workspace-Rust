#!/usr/bin/env python3
"""Atlas Replay Inspector — Interactive CLI

Reads an Atlas replay file (.atlasreplay) and prints a summary of its
contents: header info, per-frame input data, state hashes, save-point
markers, and divergence analysis when comparing two replays.

Usage:
    python tools/replay_inspector.py <replay.atlasreplay> [--compare <other.atlasreplay>]
    python tools/replay_inspector.py <replay.atlasreplay> --frames 10 20
    python tools/replay_inspector.py <replay.atlasreplay> --summary

Replay binary format (little-endian):
    Header (20 bytes):
        magic:      uint32 = 0x52504C59 ("RPLY")
        version:    uint32
        tickRate:   uint32
        frameCount: uint32
        seed:       uint32
    Per frame:
        tick:       uint32
        dataLen:    uint32
        inputData:  dataLen bytes
        stateHash:  uint64
        isSavePoint: uint8 (1 = true, 0 = false)
"""

import struct
import sys
import os

HEADER_MAGIC = 0x52504C59
HEADER_SIZE = 20
MAX_DATA_LEN = 100 * 1024 * 1024  # 100 MB safety limit per frame
MAX_FRAME_COUNT = 10_000_000      # 10M frames safety limit


def read_header(f):
    """Read replay header. Returns dict or None on error."""
    data = f.read(HEADER_SIZE)
    if len(data) < HEADER_SIZE:
        return None
    magic, version, tick_rate, frame_count, seed = struct.unpack("<5I", data)
    if magic != HEADER_MAGIC:
        return None
    return {
        "magic": magic,
        "version": version,
        "tickRate": tick_rate,
        "frameCount": frame_count,
        "seed": seed,
    }


def read_frames(f, frame_count):
    """Read all frames after the header."""
    if frame_count > MAX_FRAME_COUNT:
        print(f"Warning: frame_count {frame_count} exceeds limit, truncating to {MAX_FRAME_COUNT}",
              file=sys.stderr)
        frame_count = MAX_FRAME_COUNT

    frames = []
    for _ in range(frame_count):
        hdr = f.read(8)
        if len(hdr) < 8:
            break
        tick, data_len = struct.unpack("<II", hdr)

        if data_len > MAX_DATA_LEN:
            print(f"Warning: frame data_len {data_len} exceeds limit, skipping remaining frames",
                  file=sys.stderr)
            break

        input_data = f.read(data_len) if data_len > 0 else b""
        if len(input_data) < data_len:
            break  # truncated file

        hash_data = f.read(8)
        if len(hash_data) < 8:
            break  # truncated file
        state_hash = struct.unpack("<Q", hash_data)[0]

        sp_data = f.read(1)
        if len(sp_data) < 1:
            break  # truncated file
        is_save_point = sp_data[0] != 0

        frames.append({
            "tick": tick,
            "inputData": input_data,
            "stateHash": state_hash,
            "isSavePoint": is_save_point,
        })
    return frames


def load_replay(path):
    """Load a replay file, returning (header, frames)."""
    with open(path, "rb") as f:
        header = read_header(f)
        if header is None:
            print(f"Error: invalid replay header in {path}", file=sys.stderr)
            sys.exit(1)
        frames = read_frames(f, header["frameCount"])
    return header, frames


def print_summary(path, header, frames):
    """Print replay summary."""
    print("=" * 60)
    print("Atlas Replay Inspector")
    print("=" * 60)
    print(f"  File:       {path}")
    print(f"  Version:    {header['version']}")
    print(f"  Tick Rate:  {header['tickRate']} Hz")
    print(f"  Frames:     {header['frameCount']} (loaded {len(frames)})")
    print(f"  Seed:       {header['seed']}")

    save_points = [f for f in frames if f["isSavePoint"]]
    input_frames = [f for f in frames if len(f["inputData"]) > 0]
    print(f"  Save Points: {len(save_points)}")
    print(f"  Input Frames: {len(input_frames)}")
    if frames:
        print(f"  Tick Range:  {frames[0]['tick']} .. {frames[-1]['tick']}")
    print("=" * 60)


def print_frames(frames, start_tick=None, end_tick=None):
    """Print per-frame details for a tick range."""
    print(f"\n  {'Tick':<8} {'InputLen':<10} {'Hash':<20} {'SavePt'}")
    print(f"  {'-'*8} {'-'*10} {'-'*20} {'-'*6}")
    for f in frames:
        if start_tick is not None and f["tick"] < start_tick:
            continue
        if end_tick is not None and f["tick"] > end_tick:
            break
        sp = "YES" if f["isSavePoint"] else ""
        print(f"  {f['tick']:<8} {len(f['inputData']):<10} 0x{f['stateHash']:016X}  {sp}")


def compare_replays(path_a, header_a, frames_a, path_b, header_b, frames_b):
    """Compare two replays and report divergences."""
    print(f"\n--- Comparison: {os.path.basename(path_a)} vs {os.path.basename(path_b)} ---\n")
    min_len = min(len(frames_a), len(frames_b))
    first_diverge = -1
    match_count = 0
    for i in range(min_len):
        if frames_a[i]["stateHash"] == frames_b[i]["stateHash"]:
            match_count += 1
        elif first_diverge < 0:
            first_diverge = frames_a[i]["tick"]

    total = max(len(frames_a), len(frames_b))
    pct = (match_count / total * 100) if total > 0 else 100.0
    print(f"  Match:      {match_count}/{total} ({pct:.1f}%)")
    if first_diverge >= 0:
        print(f"  First Div:  tick {first_diverge}")
    else:
        if len(frames_a) != len(frames_b):
            print(f"  Length Diff: {len(frames_a)} vs {len(frames_b)}")
        else:
            print("  ✅ Replays are identical")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.isfile(path):
        print(f"Error: file not found: {path}", file=sys.stderr)
        sys.exit(1)

    header, frames = load_replay(path)

    # Parse optional flags
    compare_path = None
    start_tick = None
    end_tick = None
    summary_only = False

    i = 2
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "--compare" and i + 1 < len(sys.argv):
            compare_path = sys.argv[i + 1]
            i += 2
        elif arg == "--frames" and i + 2 < len(sys.argv):
            start_tick = int(sys.argv[i + 1])
            end_tick = int(sys.argv[i + 2])
            i += 3
        elif arg == "--summary":
            summary_only = True
            i += 1
        else:
            i += 1

    print_summary(path, header, frames)

    if not summary_only:
        if start_tick is not None and end_tick is not None:
            print_frames(frames, start_tick, end_tick)
        elif len(frames) <= 50:
            print_frames(frames)
        else:
            print(f"\n  (Showing first 20 of {len(frames)} frames; use --frames START END)")
            print_frames(frames[:20])

    if compare_path:
        if not os.path.isfile(compare_path):
            print(f"Error: file not found: {compare_path}", file=sys.stderr)
            sys.exit(1)
        header_b, frames_b = load_replay(compare_path)
        compare_replays(path, header, frames, compare_path, header_b, frames_b)


if __name__ == "__main__":
    main()
