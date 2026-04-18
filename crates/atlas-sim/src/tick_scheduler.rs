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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_tick_rate_is_60() {
        let s = TickScheduler::new();
        assert_eq!(s.tick_rate(), 60);
        assert!((s.fixed_delta_time() - 1.0 / 60.0).abs() < 1e-6);
    }

    #[test]
    fn set_tick_rate() {
        let mut s = TickScheduler::new();
        s.set_tick_rate(30);
        assert_eq!(s.tick_rate(), 30);
        assert!((s.fixed_delta_time() - 1.0 / 30.0).abs() < 1e-5);
    }

    #[test]
    fn tick_zero_hz_clamped_to_one() {
        let mut s = TickScheduler::new();
        s.set_tick_rate(0); // should clamp to 1
        assert_eq!(s.tick_rate(), 1);
    }

    #[test]
    fn tick_callback_receives_fixed_dt() {
        let mut s = TickScheduler::new();
        s.set_tick_rate(10);
        let mut received = 0.0f32;
        s.tick(|dt| received = dt);
        assert!((received - 0.1).abs() < 1e-5);
        assert_eq!(s.current_tick(), 1);
    }

    #[test]
    fn multiple_ticks_increment_counter() {
        let mut s = TickScheduler::new();
        for _ in 0..5 {
            s.tick(|_| {});
        }
        assert_eq!(s.current_tick(), 5);
    }

    #[test]
    fn frame_pacing() {
        let mut s = TickScheduler::new();
        assert!(!s.frame_pacing_enabled());
        s.set_frame_pacing(true);
        assert!(s.frame_pacing_enabled());
    }

    #[test]
    fn lock_tick_rate_prevents_change() {
        let mut s = TickScheduler::new();
        s.lock_tick_rate();
        assert!(s.is_tick_rate_locked());
        s.set_tick_rate(120); // locked — must not change
        assert_eq!(s.tick_rate(), 60);
    }
}
