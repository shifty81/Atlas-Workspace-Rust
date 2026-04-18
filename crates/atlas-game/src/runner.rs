//! [`GameRunner`] — drives the fixed-step game loop.

use std::time::{Duration, Instant};

use atlas_ecs::World;

use crate::module::{GameModule, GameInitContext, GameTickContext};

/// Configuration for a [`GameRunner`] session.
#[derive(Debug, Clone)]
pub struct GameRunConfig {
    /// Fixed ticks per second (default 60).
    pub tick_rate:      u32,
    /// Maximum number of ticks to process per frame (prevent spiral of death).
    pub max_ticks_frame: u32,
    /// Exit automatically after this many ticks (0 = run until window closed / Ctrl-C).
    pub max_ticks_total: u64,
    /// True when launched in PIE (Play-In-Editor) mode.
    pub pie_mode:       bool,
}

impl Default for GameRunConfig {
    fn default() -> Self {
        Self {
            tick_rate:       60,
            max_ticks_frame: 8,
            max_ticks_total: 0,
            pie_mode:        false,
        }
    }
}

/// Outcome of a completed [`GameRunner::run`] call.
#[derive(Debug, PartialEq, Eq)]
pub enum RunResult {
    /// Normal exit (window closed, escape key, or max ticks reached).
    Ok,
    /// PIE session was stopped by the editor.
    PieStopped,
    /// The game module panicked or returned a fatal error.
    Error(String),
}

// ── GameRunner ───────────────────────────────────────────────────────────────

/// Owns the ECS world and drives the fixed-step game loop.
pub struct GameRunner {
    config:    GameRunConfig,
    world:     World,
    elapsed_s: f32,
    tick_count: u64,
}

impl GameRunner {
    /// Create a new runner with the given config.
    pub fn new(config: GameRunConfig) -> Self {
        Self {
            config,
            world:      World::new(),
            elapsed_s:  0.0,
            tick_count: 0,
        }
    }

    /// Access the ECS world (for pre-loading scene data before running).
    pub fn world_mut(&mut self) -> &mut World { &mut self.world }

    /// Run the game loop.  Returns when the game exits or `max_ticks_total`
    /// is reached.
    pub fn run(&mut self, module: &mut dyn GameModule) -> RunResult {
        let tick_dt = Duration::from_secs_f64(1.0 / self.config.tick_rate as f64);
        let dt_s    = tick_dt.as_secs_f32();

        log::info!(
            "[GameRunner] Starting '{}' @ {}Hz{}",
            module.name(),
            self.config.tick_rate,
            if self.config.pie_mode { " (PIE)" } else { "" },
        );

        module.init(GameInitContext { world: &mut self.world });

        let start  = Instant::now();
        let mut accumulator = Duration::ZERO;
        let mut last_frame  = Instant::now();

        loop {
            // Check PIE abort via env var (editor sets ATLAS_PIE_STOP=1)
            if self.config.pie_mode && std::env::var("ATLAS_PIE_STOP").is_ok() {
                log::info!("[GameRunner] PIE stop signal received");
                module.shutdown();
                return RunResult::PieStopped;
            }

            let now = Instant::now();
            let frame_time = now.duration_since(last_frame).min(Duration::from_millis(250));
            last_frame = now;
            accumulator += frame_time;

            let mut ticks_this_frame = 0u32;
            while accumulator >= tick_dt {
                accumulator -= tick_dt;
                self.elapsed_s += dt_s;
                self.tick_count += 1;

                module.tick(GameTickContext {
                    world:     &mut self.world,
                    delta_s:   dt_s,
                    elapsed_s: self.elapsed_s,
                });

                ticks_this_frame += 1;
                if ticks_this_frame >= self.config.max_ticks_frame {
                    accumulator = Duration::ZERO; // discard excess
                    break;
                }

                if self.config.max_ticks_total > 0 && self.tick_count >= self.config.max_ticks_total {
                    log::info!(
                        "[GameRunner] Reached max ticks ({}) after {:.2}s",
                        self.tick_count,
                        start.elapsed().as_secs_f32()
                    );
                    module.shutdown();
                    return RunResult::Ok;
                }
            }

            // Yield to avoid burning 100% CPU. Only sleep if there is a
            // meaningful amount of time remaining to avoid excessive wake-ups.
            let remaining = tick_dt.saturating_sub(accumulator);
            if remaining > std::time::Duration::from_millis(1) {
                std::thread::sleep(remaining);
            } else {
                std::thread::yield_now();
            }
        }
    }

    pub fn tick_count(&self) -> u64 { self.tick_count }
    pub fn elapsed_s(&self)  -> f32  { self.elapsed_s }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::module::{GameModule, GameInitContext, GameTickContext, NullGameModule};

    #[test]
    fn new_runner_starts_at_zero() {
        let runner = GameRunner::new(GameRunConfig::default());
        assert_eq!(runner.tick_count(), 0);
        assert_eq!(runner.elapsed_s(), 0.0);
    }

    #[test]
    fn run_exits_after_max_ticks() {
        let cfg = GameRunConfig {
            tick_rate: 60,
            max_ticks_frame: 60,
            max_ticks_total: 10,
            pie_mode: false,
        };
        let mut runner = GameRunner::new(cfg);
        let mut module = NullGameModule;
        let result = runner.run(&mut module);
        assert_eq!(result, RunResult::Ok);
        assert_eq!(runner.tick_count(), 10);
    }

    #[test]
    fn elapsed_increases_proportional_to_ticks() {
        let tick_rate = 60u32;
        let ticks = 60u64;
        let cfg = GameRunConfig {
            tick_rate,
            max_ticks_frame: 120,
            max_ticks_total: ticks,
            pie_mode: false,
        };
        let mut runner = GameRunner::new(cfg);
        let mut module = NullGameModule;
        runner.run(&mut module);
        let expected_s = ticks as f32 / tick_rate as f32;
        assert!((runner.elapsed_s() - expected_s).abs() < 1e-4);
    }

    #[test]
    fn world_mut_returns_world() {
        let mut runner = GameRunner::new(GameRunConfig::default());
        let e = runner.world_mut().spawn();
        assert!(runner.world_mut().entities.is_alive(e));
    }

    #[test]
    fn default_config_tick_rate_60() {
        let cfg = GameRunConfig::default();
        assert_eq!(cfg.tick_rate, 60);
        assert_eq!(cfg.max_ticks_frame, 8);
        assert_eq!(cfg.max_ticks_total, 0);
        assert!(!cfg.pie_mode);
    }
}
