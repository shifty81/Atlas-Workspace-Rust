//! `atlas-game` binary — standalone game runtime entry point.
//!
//! ## Usage
//!
//! | Mode | Command |
//! |------|---------|
//! | Standalone | `cargo run --bin atlas-game` |
//! | PIE (editor spawns) | `cargo run --bin atlas-game -- --pie` |
//! | Demo / CI smoke-test | `cargo run --bin atlas-game -- --demo` |

use atlas_core::Logger;
use atlas_game::{
    runner::{GameRunConfig, GameRunner},
    module::NullGameModule,
};

fn main() -> anyhow::Result<()> {
    Logger::init();

    let args: Vec<String> = std::env::args().collect();
    let pie_mode  = args.iter().any(|a| a == "--pie");
    let demo_mode = args.iter().any(|a| a == "--demo");

    if demo_mode {
        run_demo();
        Logger::shutdown();
        return Ok(());
    }

    log::info!("=== Atlas Game v{} ===", atlas_core::VERSION);
    if pie_mode {
        log::info!("[Game] Running in PIE mode");
    }

    let config = GameRunConfig {
        tick_rate:       60,
        max_ticks_frame: 8,
        max_ticks_total: if pie_mode { 300 } else { 0 }, // PIE auto-stops after 5 s in demo
        pie_mode,
        ..Default::default()
    };

    let mut runner = GameRunner::new(config);
    let mut module = NullGameModule;

    let result = runner.run(&mut module);
    log::info!("[Game] Exited with result: {:?}", result);

    Logger::shutdown();
    Ok(())
}

fn run_demo() {
    use atlas_game::{runner::{GameRunConfig, GameRunner, RunResult}, module::NullGameModule};

    log::info!("── Atlas Game Demo ─────────────────────────────────────────");

    let config = GameRunConfig {
        tick_rate:       60,
        max_ticks_total: 120, // 2 seconds at 60 Hz
        ..Default::default()
    };
    let mut runner = GameRunner::new(config);
    let mut module = NullGameModule;

    let result = runner.run(&mut module);
    assert_eq!(result, RunResult::Ok);
    log::info!(
        "[Demo] Ran {} ticks in {:.2}s",
        runner.tick_count(), runner.elapsed_s()
    );
}
