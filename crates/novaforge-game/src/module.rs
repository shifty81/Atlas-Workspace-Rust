//! [`NovaForgeGameModule`] — implements [`atlas_game::GameModule`] for the
//! NovaForge game project.
//!
//! This is the primary entry point for NovaForge game logic inside the
//! Atlas Workspace runtime.  Register it with [`atlas_game::GameRunner`] to
//! boot the game.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_game::{GameModule, GameInitContext, GameTickContext};

/// Configuration for the NovaForge game module.
#[derive(Debug, Clone)]
pub struct NovaForgeConfig {
    /// Path to the `novaforge-assets/` directory.
    /// If empty, falls back to the `NOVAFORGE_ASSETS_DIR` environment variable,
    /// then to `./novaforge-assets` relative to the current working directory.
    pub assets_dir: String,

    /// Server address for multiplayer mode (empty = singleplayer / LAN host).
    pub server_addr: String,

    /// Enable LAN co-op hosting (spawn embedded server in the game process).
    pub lan_host: bool,

    /// Fixed tick rate for the game simulation (default: 30 Hz).
    pub tick_rate: u32,
}

impl Default for NovaForgeConfig {
    fn default() -> Self {
        Self {
            assets_dir:  String::new(),
            server_addr: String::new(),
            lan_host:    false,
            tick_rate:   30,
        }
    }
}

// ── NovaForgeGameModule ──────────────────────────────────────────────────────

/// The NovaForge game module.
///
/// Implements [`GameModule`] to integrate with `atlas-game`'s [`GameRunner`].
///
/// ## Roadmap
///
/// The following systems will be registered during `init()` as the port matures:
///
/// - `CharacterSystem`  — class stats, appearance config
/// - `EconomySystem`    — currency transactions
/// - `InventorySystem`  — item management, slot rules
/// - `MissionSystem`    — objective tracking, chain progression
/// - `ProgressionSystem` — XP, level threshold, skill unlock
/// - `ShopSystem`       — store listings, purchase flow
/// - `PCGWorldSystem`   — atlas-pcg world generation integration
/// - `LanServerSystem`  — embedded LAN server (optional)
///
/// [`GameRunner`]: atlas_game::GameRunner
pub struct NovaForgeGameModule {
    config:    NovaForgeConfig,
    tick_num:  u64,
    assets_ok: bool,
}

impl NovaForgeGameModule {
    /// Create the module with the given configuration.
    pub fn new(config: NovaForgeConfig) -> Self {
        Self { config, tick_num: 0, assets_ok: false }
    }

    /// Create the module with default configuration.
    pub fn with_defaults() -> Self {
        Self::new(NovaForgeConfig::default())
    }

    /// Resolve the assets directory: config > env var > default path.
    fn resolve_assets_dir(&self) -> String {
        if !self.config.assets_dir.is_empty() {
            return self.config.assets_dir.clone();
        }
        if let Ok(env_dir) = std::env::var("NOVAFORGE_ASSETS_DIR") {
            if !env_dir.is_empty() {
                return env_dir;
            }
        }
        "novaforge-assets".to_string()
    }

    /// Check that the assets directory exists and is readable.
    fn verify_assets(&self) -> bool {
        let dir = self.resolve_assets_dir();
        std::path::Path::new(&dir).is_dir()
    }

    pub fn tick_num(&self)  -> u64  { self.tick_num }
    pub fn assets_ok(&self) -> bool { self.assets_ok }
}

impl GameModule for NovaForgeGameModule {
    fn name(&self) -> &str { "NovaForge" }

    fn init(&mut self, ctx: GameInitContext<'_>) {
        log::info!("[NovaForge] Initialising game module");

        // Resolve and verify assets
        let assets_dir = self.resolve_assets_dir();
        self.assets_ok = self.verify_assets();

        if self.assets_ok {
            log::info!("[NovaForge] Assets found at: {}", assets_dir);
        } else {
            log::warn!(
                "[NovaForge] Asset directory not found: '{}'\n\
                 Run `bash Scripts/fetch_novaforge_assets.sh` to download assets,\n\
                 or set NOVAFORGE_ASSETS_DIR to the correct path.",
                assets_dir
            );
        }

        if self.config.lan_host {
            log::info!("[NovaForge] LAN hosting enabled (tick_rate={}Hz)", self.config.tick_rate);
        }

        // Spawn a root scene entity to anchor the NovaForge world.
        let _world_entity = ctx.world.spawn();
        log::info!("[NovaForge] World entity spawned");

        // TODO (Phase 4): register game systems into SystemRegistry
        // TODO (Phase 4): load initial scene from novaforge-assets/
        // TODO (Phase 4): connect atlas-pcg PCGWorldSystem
    }

    fn tick(&mut self, _ctx: GameTickContext<'_>) {
        self.tick_num += 1;
        // TODO (Phase 4): drive EconomySystem, InventorySystem, MissionSystem, …
    }

    fn shutdown(&mut self) {
        log::info!("[NovaForge] Shutting down after {} ticks", self.tick_num);
        // TODO (Phase 4): flush save data, disconnect LAN server
    }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use atlas_ecs::World;
    use atlas_game::{GameInitContext, GameTickContext};

    #[test]
    fn module_name() {
        let m = NovaForgeGameModule::with_defaults();
        assert_eq!(m.name(), "NovaForge");
    }

    #[test]
    fn config_default_lan_off() {
        let cfg = NovaForgeConfig::default();
        assert!(!cfg.lan_host);
        assert_eq!(cfg.tick_rate, 30);
    }

    #[test]
    fn init_does_not_panic() {
        let mut m = NovaForgeGameModule::with_defaults();
        let mut w = World::new();
        m.init(GameInitContext { world: &mut w });
    }

    #[test]
    fn tick_increments_counter() {
        let mut m = NovaForgeGameModule::with_defaults();
        let mut w = World::new();
        m.init(GameInitContext { world: &mut w });
        m.tick(GameTickContext { world: &mut w, delta_s: 0.033, elapsed_s: 0.0 });
        m.tick(GameTickContext { world: &mut w, delta_s: 0.033, elapsed_s: 0.033 });
        assert_eq!(m.tick_num(), 2);
    }

    #[test]
    fn shutdown_does_not_panic() {
        let mut m = NovaForgeGameModule::with_defaults();
        m.shutdown();
    }

    #[test]
    fn resolve_assets_dir_uses_config() {
        let m = NovaForgeGameModule::new(NovaForgeConfig {
            assets_dir: "/custom/assets".into(),
            ..Default::default()
        });
        assert_eq!(m.resolve_assets_dir(), "/custom/assets");
    }

    #[test]
    fn resolve_assets_dir_falls_back_to_default() {
        // Ensure env var is unset for this test
        std::env::remove_var("NOVAFORGE_ASSETS_DIR");
        let m = NovaForgeGameModule::with_defaults();
        assert_eq!(m.resolve_assets_dir(), "novaforge-assets");
    }
}
