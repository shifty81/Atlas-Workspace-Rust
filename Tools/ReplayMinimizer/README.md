# ReplayMinimizer — Debug Trace Reduction Tool

`Tools/ReplayMinimizer/` — standalone C++ binary.

ReplayMinimizer reads a frame-by-frame `.replay.json` file produced by the
game engine, removes redundant frames, delta-compresses the remainder, and
writes a minimized replay back to the pipeline.  The editor can play back
minimized replays in the viewport, and the regression harness can assert
expected final state from them.

---

## Tool Roadmap

| ID   | Milestone | Goal |
|------|-----------|------|
| RM-1 | Read replay | Parse a `.replay.json` frame log from `pipeline/` |
| RM-2 | Delta compression | Store only changed fields per frame; unchanged fields omitted |
| RM-3 | Semantic minimization | Drop frames where no observable game state changed |
| RM-4 | Pipeline export | Write minimized replay to `pipeline/`; editor plays it back in viewport |
| RM-5 | Regression harness | Run minimized replay headlessly; assert expected final state |

---

## Pipeline integration

- Reads: `pipeline/replays/<session>.replay.json`
- Writes: `pipeline/replays/<session>.min.replay.json`
- Writes: `pipeline/changes/<timestamp>_replayminimizer_ReplayExported.change.json`

### Replay JSON schema (frame entry)

```json
{
  "frame": 42,
  "timestamp_ms": 1400,
  "delta": {
    "player.position": [10.5, 1.0, -3.2],
    "player.health": 85
  }
}
```

---

## Usage (RM-1 / RM-2 target)

```
replayminimizer minimize pipeline/replays/session_001.replay.json
replayminimizer assert pipeline/replays/session_001.min.replay.json expected.json
```
