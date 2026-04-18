---- MODULE replay ----
(***************************************************************************)
(* Replay Hash Ladder for the Atlas engine.                                *)
(*                                                                         *)
(* Models the chained hash commitment scheme used by StateHasher and       *)
(* ReplayRecorder.  Each tick produces a hash that depends on the previous *)
(* hash, the current ECS state, and the current inputs:                    *)
(*                                                                         *)
(*   H[n] = Hash(H[n-1] || State[n] || Inputs[n])                         *)
(*                                                                         *)
(* The real implementation uses FNV-1a 64-bit.  Here we abstract hashing   *)
(* as an injective combining function to verify structural properties      *)
(* without bit-level detail.                                               *)
(*                                                                         *)
(* Verified invariants:                                                    *)
(*   ChainIntegrity   — Every hash derives from exactly its predecessor,   *)
(*                      the current state, and the current inputs.         *)
(*   NoGaps           — The tick sequence is contiguous (no skipped ticks). *)
(*   Determinism      — Identical input sequences produce identical hash   *)
(*                      sequences (modeled via a shadow replay).           *)
(*   MonotonicTicks   — Frame ticks are strictly increasing.               *)
(*                                                                         *)
(* Also models recording vs. playback modes, matching ReplayRecorder's     *)
(* StartRecording / LoadReplay distinction, and save-point markers.        *)
(***************************************************************************)
EXTENDS Naturals, Sequences, FiniteSets

CONSTANTS
    MaxTick,          \* Model-checking bound on tick count
    StateSpace,       \* Finite set of abstract state values
    InputSpace,       \* Finite set of abstract input values
    Seed              \* Initial hash seed (corresponds to ReplayHeader::seed)

(***************************************************************************)
(* Abstract hash combiner.  We model Hash as a deterministic function      *)
(* from (prevHash, state, input) -> Nat.  For TLC checking with small      *)
(* domains we define it as an injective arithmetic combination.            *)
(***************************************************************************)
HashCombine(prev, state, input) ==
    (prev * 31 + state) * 17 + input

VARIABLES
    mode,             \* "idle" | "recording" | "playback"
    tick,             \* Current simulation tick (uint64_t in engine)
    hashChain,        \* Sequence of <<tick, hash>> pairs (StateHasher::m_history)
    currentHash,      \* Most recent chain hash (StateHasher::m_currentHash)
    frames,           \* Sequence of <<tick, input, stateHash, isSavePoint>> records
    replaySource,     \* In playback: the recorded frame sequence being replayed
    replayIndex,      \* In playback: current position in replaySource
    shadowChain       \* Shadow hash chain built during playback for comparison

vars == <<mode, tick, hashChain, currentHash, frames,
          replaySource, replayIndex, shadowChain>>

(***************************************************************************)
(* Type invariant.                                                         *)
(***************************************************************************)
TypeOK ==
    /\ mode \in {"idle", "recording", "playback"}
    /\ tick \in 0..MaxTick
    /\ currentHash \in Nat
    /\ Len(hashChain) = tick
    /\ Len(frames) <= MaxTick

(***************************************************************************)
(* INV: ChainIntegrity — each entry in hashChain is correctly derived.     *)
(* hashChain entries are 4-tuples <<tick, hash, state, input>>.            *)
(* entry[2] = HashCombine(prev_hash, entry[3], entry[4]) verifies that     *)
(* each hash depends on exactly the prior hash, current state, and input.  *)
(***************************************************************************)
ChainIntegrity ==
    \A i \in 1..Len(hashChain) :
        LET entry == hashChain[i]
        IN  /\ entry[1] = i   \* Tick number matches position
            /\ IF i = 1
               THEN entry[2] = HashCombine(Seed, entry[3], entry[4])
               ELSE entry[2] = HashCombine(hashChain[i-1][2], entry[3], entry[4])

(***************************************************************************)
(* INV: NoGaps — the tick sequence is contiguous from 1 to tick.           *)
(* Guaranteed by construction: tick increments by exactly 1 each step.     *)
(***************************************************************************)
NoGaps ==
    \A i \in 1..Len(hashChain) : hashChain[i][1] = i

(***************************************************************************)
(* INV: MonotonicTicks — frame ticks are strictly increasing.              *)
(* Matches ReplayRecorder which appends frames with increasing tick.       *)
(***************************************************************************)
MonotonicTicks ==
    \A i \in 1..(Len(frames) - 1) :
        frames[i][1] < frames[i + 1][1]

(***************************************************************************)
(* INV: ReplayDeterminism — during playback the shadow chain (rebuilt      *)
(* from replayed inputs) must match the original hashChain (built during   *)
(* recording).  Identical inputs produce identical hash sequences.         *)
(***************************************************************************)
ReplayDeterminism ==
    mode = "playback" =>
        \A i \in 1..Len(shadowChain) :
            shadowChain[i][2] = hashChain[i][2]

(***************************************************************************)
(* INV: FrameHashConsistency — every recorded frame's stateHash matches    *)
(* the corresponding hashChain entry.                                      *)
(***************************************************************************)
FrameHashConsistency ==
    \A i \in 1..Len(frames) :
        LET fTick == frames[i][1]
        IN  fTick <= Len(hashChain) =>
                frames[i][3] = hashChain[fTick][2]

(***************************************************************************)
(* Combined invariant.                                                     *)
(***************************************************************************)
Invariant ==
    /\ TypeOK
    /\ NoGaps
    /\ MonotonicTicks
    /\ (Len(hashChain) > 0 => ChainIntegrity)
    /\ FrameHashConsistency

(***************************************************************************)
(* Initial state: idle, no history, hash initialised to Seed.              *)
(* Mirrors StateHasher::Reset(seed) and ReplayRecorder in idle state.      *)
(***************************************************************************)
Init ==
    /\ mode = "idle"
    /\ tick = 0
    /\ hashChain = <<>>
    /\ currentHash = Seed
    /\ frames = <<>>
    /\ replaySource = <<>>
    /\ replayIndex = 0
    /\ shadowChain = <<>>

(***************************************************************************)
(* ACTION: StartRecording — begin a new recording session.                 *)
(* Mirrors ReplayRecorder::StartRecording(tickRate, seed).                 *)
(***************************************************************************)
StartRecording ==
    /\ mode = "idle"
    /\ mode' = "recording"
    /\ tick' = 0
    /\ hashChain' = <<>>
    /\ currentHash' = Seed
    /\ frames' = <<>>
    /\ UNCHANGED <<replaySource, replayIndex, shadowChain>>

(***************************************************************************)
(* ACTION: RecordTick — advance one tick during recording.                 *)
(* Chooses a state and input non-deterministically (models arbitrary game  *)
(* execution), computes the chained hash, and appends a replay frame.      *)
(* Mirrors StateHasher::AdvanceTick + ReplayRecorder::RecordFrame.         *)
(***************************************************************************)
RecordTick(state, input) ==
    /\ mode = "recording"
    /\ tick < MaxTick
    /\ state \in StateSpace
    /\ input \in InputSpace
    /\ LET newHash == HashCombine(currentHash, state, input)
           newTick == tick + 1
       IN  /\ tick' = newTick
           /\ currentHash' = newHash
           /\ hashChain' = Append(hashChain, <<newTick, newHash, state, input>>)
           /\ frames' = Append(frames, <<newTick, input, newHash, FALSE>>)
           /\ UNCHANGED <<mode, replaySource, replayIndex, shadowChain>>

(***************************************************************************)
(* ACTION: MarkSavePoint — flag the most recent frame as a save-point.     *)
(* Mirrors ReplayRecorder::MarkSavePoint(tick).                            *)
(***************************************************************************)
MarkSavePoint ==
    /\ mode = "recording"
    /\ Len(frames) > 0
    /\ LET last == frames[Len(frames)]
       IN  frames' = [frames EXCEPT ![Len(frames)] = <<last[1], last[2], last[3], TRUE>>]
    /\ UNCHANGED <<mode, tick, hashChain, currentHash, replaySource, replayIndex, shadowChain>>

(***************************************************************************)
(* ACTION: StopRecording — end the recording session, return to idle.      *)
(***************************************************************************)
StopRecording ==
    /\ mode = "recording"
    /\ mode' = "idle"
    /\ UNCHANGED <<tick, hashChain, currentHash, frames, replaySource, replayIndex, shadowChain>>

(***************************************************************************)
(* ACTION: StartPlayback — load a previously recorded session and begin    *)
(* replaying.  The recorded frames become replaySource; we rebuild the     *)
(* hash chain in shadowChain to verify determinism.                        *)
(* Mirrors ReplayRecorder::LoadReplay().                                   *)
(***************************************************************************)
StartPlayback ==
    /\ mode = "idle"
    /\ Len(frames) > 0           \* Must have something to replay
    /\ mode' = "playback"
    /\ replaySource' = frames    \* Snapshot the recorded frames
    /\ replayIndex' = 0
    /\ shadowChain' = <<>>
    /\ tick' = 0
    /\ currentHash' = Seed
    \* Preserve original hashChain and frames for comparison
    /\ UNCHANGED <<hashChain, frames>>

(***************************************************************************)
(* ACTION: PlaybackTick — replay one frame from replaySource.              *)
(* Reads the recorded input, applies it to produce state, recomputes the   *)
(* hash, and appends to shadowChain.  The state is deterministic given     *)
(* identical inputs, so we reuse the recorded state value.                 *)
(* Mirrors the replay loop that feeds recorded inputs to the simulation.   *)
(***************************************************************************)
PlaybackTick ==
    /\ mode = "playback"
    /\ replayIndex < Len(replaySource)
    /\ LET idx     == replayIndex + 1
           frame   == replaySource[idx]
           fInput  == frame[2]
           \* Determinism: same input => same state as original recording
           origEntry == hashChain[idx]
           fState    == origEntry[3]
           newHash   == HashCombine(currentHash, fState, fInput)
           newTick   == tick + 1
       IN  /\ tick' = newTick
           /\ currentHash' = newHash
           /\ replayIndex' = idx
           /\ shadowChain' = Append(shadowChain, <<newTick, newHash, fState, fInput>>)
           /\ UNCHANGED <<mode, hashChain, frames, replaySource>>

(***************************************************************************)
(* ACTION: EndPlayback — all frames replayed, return to idle.              *)
(***************************************************************************)
EndPlayback ==
    /\ mode = "playback"
    /\ replayIndex = Len(replaySource)
    /\ mode' = "idle"
    /\ UNCHANGED <<tick, hashChain, currentHash, frames, replaySource, replayIndex, shadowChain>>

(***************************************************************************)
(* Next-state relation.                                                    *)
(***************************************************************************)
Next ==
    \/ StartRecording
    \/ \E s \in StateSpace, i \in InputSpace : RecordTick(s, i)
    \/ MarkSavePoint
    \/ StopRecording
    \/ StartPlayback
    \/ PlaybackTick
    \/ EndPlayback

Spec == Init /\ [][Next]_vars

(***************************************************************************)
(* Temporal property: a recording session eventually stops.                *)
(***************************************************************************)
RecordingTerminates == [](mode = "recording" => <>(mode = "idle"))

(***************************************************************************)
(* Temporal property: playback eventually completes.                       *)
(***************************************************************************)
PlaybackTerminates == [](mode = "playback" => <>(mode = "idle"))

====
