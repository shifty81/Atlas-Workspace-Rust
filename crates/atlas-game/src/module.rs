//! [`GameModule`] — trait implemented by each game project's logic crate.

use atlas_ecs::World;

/// Context passed to [`GameModule::init`].
pub struct GameInitContext<'a> {
    pub world: &'a mut World,
}

/// Context passed to [`GameModule::tick`] every fixed step.
pub struct GameTickContext<'a> {
    pub world:    &'a mut World,
    /// Fixed-step delta time in seconds.
    pub delta_s:  f32,
    /// Total elapsed time in seconds.
    pub elapsed_s: f32,
}

/// Implemented by each game project to provide gameplay logic.
///
/// The editor calls `GameModule` indirectly through the `GameRunner`.
/// The game binary registers one concrete implementation at startup.
pub trait GameModule: Send + Sync + 'static {
    /// Called once before the first tick; use to spawn initial entities.
    fn init(&mut self, ctx: GameInitContext<'_>);

    /// Called every fixed simulation step.
    fn tick(&mut self, ctx: GameTickContext<'_>);

    /// Called when the game is shutting down.  Default: no-op.
    fn shutdown(&mut self) {}

    /// Human-readable name of this game module (used in editor logging).
    fn name(&self) -> &str { "UnnamedGame" }
}

// ── Default no-op module (used in standalone binary before a project loads) ──

/// A no-op game module used as a placeholder until a real module is loaded.
pub struct NullGameModule;

impl GameModule for NullGameModule {
    fn init(&mut self, _ctx: GameInitContext<'_>) {
        log::info!("[GameModule] NullGameModule init — no project loaded");
    }

    fn tick(&mut self, _ctx: GameTickContext<'_>) {}

    fn name(&self) -> &str { "NullGame" }
}

#[cfg(test)]
mod tests {
    use super::*;
    use atlas_ecs::World;

    #[test]
    fn null_module_name() {
        let m = NullGameModule;
        assert_eq!(m.name(), "NullGame");
    }

    #[test]
    fn null_module_init_does_not_panic() {
        let mut m = NullGameModule;
        let mut w = World::new();
        m.init(GameInitContext { world: &mut w });
    }

    #[test]
    fn null_module_tick_does_not_panic() {
        let mut m = NullGameModule;
        let mut w = World::new();
        m.tick(GameTickContext { world: &mut w, delta_s: 0.016, elapsed_s: 0.0 });
    }

    #[test]
    fn null_module_shutdown_does_not_panic() {
        let mut m = NullGameModule;
        m.shutdown();
    }
}
