#!/usr/bin/env python3
"""
Test suite for replay_minimizer.py

Verifies that the replay minimizer correctly reduces replays to minimal
reproduction cases using binary search.
"""

import sys
import pathlib
import tempfile
import subprocess
import struct

# Add tools to path for importing
sys.path.insert(0, str(pathlib.Path(__file__).parent.parent / 'tools'))
import replay_minimizer


def create_test_replay(frame_count: int, failing_after: int) -> pathlib.Path:
    """
    Create a test replay file with the specified number of frames.
    The replay will "fail" if it has more than `failing_after` frames.
    """
    frames = []
    for i in range(frame_count):
        # Each frame has tick number and dummy data
        data = f"frame_{i}".encode('utf-8')
        frames.append(replay_minimizer.ReplayFrame(i, data))
    
    replay = replay_minimizer.AtlasReplay(frames, b'test_metadata')
    
    temp_file = tempfile.NamedTemporaryFile(suffix='.atlasreplay', delete=False)
    temp_path = pathlib.Path(temp_file.name)
    replay.save(temp_path)
    
    return temp_path


def create_test_script(failing_after: int) -> pathlib.Path:
    """
    Create a test script that fails if the replay has more than
    `failing_after` frames.
    """
    # Get absolute path to tools directory
    tools_dir = pathlib.Path(__file__).parent.parent / 'tools'
    tools_dir = tools_dir.resolve()
    
    script_content = f"""#!/usr/bin/env python3
import sys
import pathlib

# Add tools directory to path
sys.path.insert(0, '{tools_dir}')
import replay_minimizer

replay_path = pathlib.Path(sys.argv[1])
replay = replay_minimizer.AtlasReplay.from_file(replay_path)
frame_count = len(replay.frames)

# Fail if frame count > {failing_after}
if frame_count > {failing_after}:
    sys.exit(1)
else:
    sys.exit(0)
"""
    
    temp_file = tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False)
    temp_file.write(script_content)
    temp_file.close()
    
    temp_path = pathlib.Path(temp_file.name)
    temp_path.chmod(0o755)
    
    return temp_path


def test_replay_serialization():
    """Test replay serialization/deserialization."""
    print("Testing replay serialization...")
    
    frames = [
        replay_minimizer.ReplayFrame(0, b'data0'),
        replay_minimizer.ReplayFrame(1, b'data1'),
        replay_minimizer.ReplayFrame(2, b'data2'),
    ]
    
    replay = replay_minimizer.AtlasReplay(frames, b'metadata')
    
    with tempfile.NamedTemporaryFile(suffix='.atlasreplay', delete=False) as f:
        temp_path = pathlib.Path(f.name)
    
    try:
        replay.save(temp_path)
        loaded = replay_minimizer.AtlasReplay.from_file(temp_path)
        
        assert len(loaded.frames) == len(frames), "Frame count mismatch"
        assert loaded.metadata == b'metadata', "Metadata mismatch"
        
        for i, (orig, loaded_frame) in enumerate(zip(frames, loaded.frames)):
            assert orig.tick == loaded_frame.tick, f"Tick mismatch at frame {i}"
            assert orig.data == loaded_frame.data, f"Data mismatch at frame {i}"
        
        print("  ✓ Serialization test passed")
    finally:
        temp_path.unlink(missing_ok=True)


def test_replay_slicing():
    """Test replay slicing."""
    print("Testing replay slicing...")
    
    frames = [
        replay_minimizer.ReplayFrame(i, f'data{i}'.encode())
        for i in range(10)
    ]
    
    replay = replay_minimizer.AtlasReplay(frames, b'metadata')
    
    # Slice to first 5 frames
    sliced = replay.slice(5)
    
    assert len(sliced.frames) == 5, "Slice length mismatch"
    assert sliced.metadata == b'metadata', "Metadata not preserved"
    
    for i in range(5):
        assert sliced.frames[i].tick == i, f"Tick mismatch at frame {i}"
    
    print("  ✓ Slicing test passed")


def test_minimization():
    """Test replay minimization."""
    print("Testing replay minimization...")
    
    # Create a replay with 100 frames that fails when it has > 25 frames
    failing_after = 25
    original_frame_count = 100
    
    replay_path = create_test_replay(original_frame_count, failing_after)
    test_script = create_test_script(failing_after)
    
    try:
        # Run minimization
        test_command = f"python3 {test_script} {{replay}}"
        output_path = replay_path.parent / "minimized_test.atlasreplay"
        
        result = replay_minimizer.minimize_replay(
            replay_path,
            test_command,
            output_path,
            verbose=False,
        )
        
        assert result is not None, "Minimization failed"
        assert output_path.exists(), "Output file not created"
        
        # Verify the minimized replay
        minimized = replay_minimizer.AtlasReplay.from_file(output_path)
        minimized_count = len(minimized.frames)
        
        # The minimized replay should:
        # 1. Still fail (have > failing_after frames)
        # 2. Be significantly smaller than the original
        # 3. Be close to optimal (within a few frames of failing_after + 1)
        assert minimized_count > failing_after, \
            f"Minimized replay doesn't fail: {minimized_count} frames (threshold: {failing_after})"
        assert minimized_count <= failing_after + 5, \
            f"Minimization not optimal: {minimized_count} frames (expected ~{failing_after + 1})"
        assert minimized_count < original_frame_count * 0.5, \
            f"Minimization didn't reduce size enough: {minimized_count}/{original_frame_count}"
        
        reduction_pct = 100.0 * (1.0 - minimized_count / original_frame_count)
        print(f"  ✓ Minimization test passed")
        print(f"    Original: {original_frame_count} frames")
        print(f"    Minimized: {minimized_count} frames")
        print(f"    Reduction: {reduction_pct:.1f}%")
        
    finally:
        replay_path.unlink(missing_ok=True)
        test_script.unlink(missing_ok=True)
        if output_path.exists():
            output_path.unlink(missing_ok=True)


def main():
    print("Running replay_minimizer tests...\n")
    
    try:
        test_replay_serialization()
        test_replay_slicing()
        test_minimization()
        
        print("\n✅ All tests passed!")
        return 0
    except AssertionError as e:
        print(f"\n❌ Test failed: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
