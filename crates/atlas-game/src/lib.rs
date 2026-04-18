//! # Atlas Game
//!
//! Standalone game runtime.  The `atlas-game` binary loads a [`GameModule`]
//! and drives the game loop via [`GameRunner`].
//!
//! ## Design
//!
//! * [`GameModule`] — implemented by each project's game logic crate.
//!   The editor never depends on this crate; only the game binary does.
//! * [`GameRunner`] — owns the ECS world, tick scheduler, and system
//!   registry, and drives the fixed-step game loop.
//! * [`GameRunConfig`] — tunable parameters passed at start-up.
//!
//! The `atlas-game` binary serves two purposes:
//! 1. **Standalone** — final shipping build, no editor attached.
//! 2. **PIE target** — spawned by `atlas-editor`'s `GameBuildSystem` during
//!    Play-In-Editor sessions.  When launched with `--pie <pipe>` it reads
//!    scene data from the editor over a named pipe / stdin channel.

pub mod module;
pub mod runner;

pub use module::{GameModule, GameInitContext, GameTickContext};
pub use runner::{GameRunner, GameRunConfig, RunResult};
