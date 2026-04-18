#[derive(Debug)]
pub struct TickScheduler {
    tick_rate: u32,
    current_tick: u64,
    frame_pacing: bool,
    tick_rate_locked: bool,
}

impl Default for TickScheduler {
    fn default() -> Self {
        Self::new()
    }
}

impl TickScheduler {
    pub fn new() -> Self {
        Self {
            tick_rate: 60,
            current_tick: 0,
            frame_pacing: false,
            tick_rate_locked: false,
        }
    }

    pub fn set_tick_rate(&mut self, hz: u32) {
        if !self.tick_rate_locked {
            self.tick_rate = hz.max(1);
        }
    }

    pub fn tick_rate(&self) -> u32 {
        self.tick_rate
    }

    pub fn fixed_delta_time(&self) -> f32 {
        1.0 / self.tick_rate as f32
    }

    pub fn tick<F: FnMut(f32)>(&mut self, mut callback: F) {
        callback(self.fixed_delta_time());
        self.current_tick += 1;
    }

    pub fn current_tick(&self) -> u64 {
        self.current_tick
    }

    pub fn set_frame_pacing(&mut self, enabled: bool) {
        self.frame_pacing = enabled;
    }

    pub fn frame_pacing_enabled(&self) -> bool {
        self.frame_pacing
    }

    pub fn lock_tick_rate(&mut self) {
        self.tick_rate_locked = true;
    }

    pub fn is_tick_rate_locked(&self) -> bool {
        self.tick_rate_locked
    }
}
