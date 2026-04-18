#[derive(Debug, Clone, Default)]
pub struct SimulationTime {
    pub tick: u64,
    pub fixed_delta_time: f32,
    pub tick_rate: u32,
}

impl SimulationTime {
    pub fn elapsed_seconds(&self) -> f64 {
        self.tick as f64 * self.fixed_delta_time as f64
    }
}

#[derive(Debug, Clone)]
pub struct WorldTime {
    pub elapsed: f64,
    pub dilation: f32,
    pub paused: bool,
}

impl Default for WorldTime {
    fn default() -> Self {
        Self { elapsed: 0.0, dilation: 1.0, paused: false }
    }
}

impl WorldTime {
    pub fn advance(&mut self, fixed_delta_time: f32) {
        if !self.paused {
            self.elapsed += (fixed_delta_time * self.dilation) as f64;
        }
    }
}

#[derive(Debug, Clone, Default)]
pub struct PresentationTime {
    pub wall_elapsed: f64,
    pub interp_alpha: f32,
}

#[derive(Debug, Clone, Default)]
pub struct TimeContext {
    pub sim: SimulationTime,
    pub world: WorldTime,
    pub presentation: PresentationTime,
}

#[derive(Debug, Default)]
pub struct TimeModel {
    context: TimeContext,
}

impl TimeModel {
    pub fn new() -> Self { Self::default() }

    pub fn set_tick_rate(&mut self, rate: u32) {
        self.context.sim.tick_rate = rate;
        if rate > 0 {
            self.context.sim.fixed_delta_time = 1.0 / rate as f32;
        }
    }

    pub fn tick_rate(&self) -> u32 { self.context.sim.tick_rate }

    pub fn advance_tick(&mut self) {
        self.context.sim.tick += 1;
        let fdt = self.context.sim.fixed_delta_time;
        self.context.world.advance(fdt);
    }

    pub fn set_world_dilation(&mut self, d: f32) { self.context.world.dilation = d; }
    pub fn world_dilation(&self) -> f32 { self.context.world.dilation }

    pub fn set_world_paused(&mut self, p: bool) { self.context.world.paused = p; }
    pub fn is_world_paused(&self) -> bool { self.context.world.paused }

    pub fn set_presentation_alpha(&mut self, a: f32) { self.context.presentation.interp_alpha = a; }
    pub fn set_wall_elapsed(&mut self, e: f64) { self.context.presentation.wall_elapsed = e; }

    pub fn context(&self) -> &TimeContext { &self.context }

    pub fn set_tick(&mut self, tick: u64) { self.context.sim.tick = tick; }

    pub fn reset(&mut self) {
        self.context = TimeContext::default();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn tick_rate_and_fixed_delta() {
        let mut m = TimeModel::new();
        m.set_tick_rate(60);
        assert_eq!(m.tick_rate(), 60);
        let dt = m.context().sim.fixed_delta_time;
        assert!((dt - 1.0 / 60.0).abs() < 1e-5);
    }

    #[test]
    fn advance_tick_increments_counter() {
        let mut m = TimeModel::new();
        m.set_tick_rate(10);
        assert_eq!(m.context().sim.tick, 0);
        m.advance_tick();
        m.advance_tick();
        assert_eq!(m.context().sim.tick, 2);
    }

    #[test]
    fn world_time_advances() {
        let mut m = TimeModel::new();
        m.set_tick_rate(10); // dt = 0.1s
        m.advance_tick();
        let elapsed = m.context().world.elapsed;
        assert!((elapsed - 0.1).abs() < 1e-5, "elapsed={elapsed}");
    }

    #[test]
    fn world_paused_stops_elapsed() {
        let mut m = TimeModel::new();
        m.set_tick_rate(10);
        m.set_world_paused(true);
        m.advance_tick();
        assert!((m.context().world.elapsed - 0.0).abs() < 1e-10);
    }

    #[test]
    fn time_dilation() {
        let mut m = TimeModel::new();
        m.set_tick_rate(10); // dt = 0.1s
        m.set_world_dilation(2.0);
        m.advance_tick();
        let elapsed = m.context().world.elapsed;
        // dilation 2x → 0.2s elapsed per tick
        assert!((elapsed - 0.2).abs() < 1e-5, "elapsed={elapsed}");
    }

    #[test]
    fn sim_elapsed_seconds() {
        let mut m = TimeModel::new();
        m.set_tick_rate(20); // dt = 0.05s
        m.advance_tick(); // tick=1
        let s = m.context().sim.elapsed_seconds();
        assert!((s - 0.05).abs() < 1e-6, "elapsed_seconds={s}");
    }

    #[test]
    fn presentation_alpha_set() {
        let mut m = TimeModel::new();
        m.set_presentation_alpha(0.75);
        assert!((m.context().presentation.interp_alpha - 0.75).abs() < f32::EPSILON);
    }
}
