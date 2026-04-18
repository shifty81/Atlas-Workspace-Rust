# Atlas Marketing & Presentation Assets

Consolidated guidance for the public-facing landing page, conference talk, slide deck script, and a live demo runbook that demonstrates Atlas’ determinism story.

---

## Landing Page — Visual Direction

- **Goal:** “Infrastructure, not a toy.” Calm, precise, and technical. Minimal color, clean typography, slow intentional motion.
- **Palette:** Deep charcoal background (`#0B0E14`), thin grid/lattice accent lines, high-contrast text. Avoid gradients except for subtle depth. Accent color only for links/CTAs.
- **Type:** Monospace + geometric sans pairing. Tight letterspacing in headings, generous line height in body. No gamer-style fonts.
- **Motion:** Slow parallax on background grid; staggered fades for section reveals. No bounce or overshoot animations.

### Sections
- **Hero:** Headline “Deterministic Game Simulation, Enforced by Design.” Subtext: “Replays survive engine upgrades. Multiplayer without guesswork. CI-enforced correctness.” Primary CTAs: “Download Editor” and “Read the Constitution.”
- **Comparison (“Why Atlas Is Different”):** Split screen. Left: Traditional engines (“best-effort determinism,” “hidden globals,” “replays break”). Right: Atlas (“declared contracts,” “fixed-point core,” “replay hashes,” “editor enforcement”). Use checkmarks and hard dividers.
- **Constitution:** Numbered clauses styled like an engineering/legal doc. Hover/click to expand rules (deterministic, replay-safe, migration-safe). Message: “If it isn’t declared, it doesn’t ship.”
- **Editor & Replay:** Mock UI panels showing replay timeline, divergence frame highlight, ghost entities (prediction vs truth), replay hash panel. Labels such as “Frame 12,442: divergence” and “Auto repro generated.”
- **Audience Fit:** Two columns. “For” (simulation-heavy, multiplayer/MMO, long-lived projects). “Not for” (game jams, prototype churn, “ship now, fix later”).
- **CTA Footer:** Copy: “Atlas is not fast to start. It’s fast to finish.” Buttons: “Get the SDK,” “Clone Example Project,” “Read CONTRIBUTING.md.”

---

## Conference Talk Assets

### Talk Outline — “Making Bugs Illegal: Determinism as an Engine Feature”
- **Act I — The Problem (10 min):** Multiplayer bug that can’t be reproduced; “mostly deterministic” myth; cost of drift (time, trust, frozen refactors).
- **Act II — The Atlas Thesis (15 min):** Determinism is a property, not a style. Contracts over conventions (deterministic, replay-safe, migration-safe). Replays as first-class artifacts. Editor as enforcement.
- **Act III — How Atlas Does It (15 min):** Fixed-point core; simulation spine (snapshot store, rollback manager, prediction smoothing); versioned SDK/ABI; CI gates determinism and contracts.
- **Act IV — Payoff (5 min):** Fearless refactors, stable multiplayer, long-lived projects. Honest fit: who Atlas is for vs not for. Close with “Atlas doesn’t make games faster. It makes them survivable.”
- **Q&A Prep:** Anticipate “Isn’t this slow?” → debugging is slower, determinism isn’t. “Why not Unity/Unreal?” → they optimize iteration speed; Atlas optimizes correctness. “Can I cheat?” → yes, but it’s explicit and auditable.

### Slide Deck Script (titles + speaking beats)
- **Slide 1 — Title:** “Making Bugs Illegal.” Speaker: set stakes; Atlas is a determinism platform. Note: promise a live replay demo.
- **Slide 2 — The Pain:** Bullet list of unreproducible multiplayer bugs. Note: ask audience for a show of hands; frame shared pain.
- **Slide 3 — The Lie of ‘Mostly Deterministic’:** Call out floating drift, hidden globals, timing chaos. Note: “Linting won’t save you.”
- **Slide 4 — Cost Curve:** Diagram of compounding debug time over project age. Note: “Tech debt is time-based, not team-based.”
- **Slide 5 — Atlas Thesis:** Contracts: deterministic, replay-safe, migration-safe. Note: “If it isn’t declared, it doesn’t ship.”
- **Slide 6 — Replays as Tests:** Show replay hash + divergence frame. Note: “Every bug becomes a replay. Replays outlive code.”
- **Slide 7 — Editor as Enforcer:** UI mock of blocked build without contracts. Note: “The editor refuses unsafe systems.”
- **Slide 8 — Fixed-Point Core:** One diagram: same inputs → same outputs. Note: “Determinism is deliberate, not accidental.”
- **Slide 9 — Simulation Spine:** Snapshot → rollback → prediction smoothing. Note: keep code off slides; narrate data flow.
- **Slide 10 — Versioned SDK/ABI:** Table of SDK versions, supported ABI, determinism profiles. Note: “No silent breaks; upgrades are explicit.”
- **Slide 11 — CI Gate:** Red CI screen for hash mismatch. Note: “CI fails on undocumented behavior, not formatting.”
- **Slide 12 — Payoff:** Fearless refactors, stable multiplayer, long-lived replays. Note: “Atlas optimizes for truth over speed.”
- **Slide 13 — Live Demo Setup:** “Replay desync → fix → replay passes.” Note: set expectations for the demo steps.
- **Slide 14 — Audience Fit:** Two columns (For / Not For). Note: candor builds trust.
- **Slide 15 — CTA:** Links to SDK, example project, constitution. Note: invite questions.

### Speaker Notes (high level)
- Keep slides sparse; speak to stories (pain → principle → proof).
- Emphasize enforcement over advice; avoid buzzwords.
- In Q&A, redirect to the enforcement mechanisms (replay hashes, contracts, CI).
- Time-box demo to <5 minutes; rehearse to avoid tool-switch lag.

---

## Live Demo Runbook — Replay Desync → Fix

Goal: Prove Atlas enforces determinism. Show a replay hash mismatch after an intentional bug, fix it, and rerun to green.

### Prerequisites
- Built binaries: `./build.sh client server runtime` (already covered by `./build.sh all`).
- Example project loaded (any deterministic sample; OrbitSim-style minimal project is ideal).
- Clean workspace with no uncommitted changes before demo.

### Baseline (good state)
1. Start server headless: `./dist/AtlasServer --project projects/Example/Example.atlas --headless --record demo_baseline.atlasreplay`
2. Start client and connect: `./dist/AtlasClient --connect localhost:7777 --project projects/Example/Example.atlas`
3. Play for ~30 seconds, then exit cleanly to finalize replay.
4. Validate replay hash: `./dist/AtlasRuntime --replay demo_baseline.atlasreplay --verify` (expect “hash OK”).

### Introduce a deterministic violation (controlled)
- In a throwaway branch, edit a gameplay system to use `std::chrono::steady_clock::now()` for a random tick (or toggle a non-deterministic RNG seed). Rebuild runtime targets only.

### Observe the failure
1. Re-run client/server with the same steps, recording `demo_regression.atlasreplay`.
2. Verify: `./dist/AtlasRuntime --replay demo_regression.atlasreplay --verify` (expect hash mismatch).
3. Show the divergence frame reported by runtime; note frame number and component diff.

### Fix and prove it
1. Replace the nondeterministic call with deterministic input (e.g., seeded RNG passed via context).
2. Rebuild: `./build.sh runtime client server -b Release`.
3. Record `demo_fixed.atlasreplay` with the same 30s flow.
4. Verify both replays:
   - `./dist/AtlasRuntime --replay demo_baseline.atlasreplay --verify` (still OK)
   - `./dist/AtlasRuntime --replay demo_fixed.atlasreplay --verify` (now OK)
5. Optional: show CI-style output where the regression replay fails before the fix and passes after.

### Demo Tips
- Script terminal panes in advance (server, client, runtime verify).
- Keep code diff tiny and readable for the audience (one-line bug, one-line fix).
- If networking is flaky on-site, run client + server locally with loopback mode.
- Have the “fixed” binary ready as a fallback to avoid live-build risk.

