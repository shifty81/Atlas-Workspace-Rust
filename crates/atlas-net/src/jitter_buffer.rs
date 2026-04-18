pub const JITTER_ALPHA: f32 = 0.125;
pub const ADAPTIVE_SCALE: f32 = 2.0;
pub const MIN_TARGET_DELAY: f32 = 0.02;
pub const MAX_TARGET_DELAY: f32 = 0.50;

#[derive(Debug, Clone)]
pub struct JitterEntry {
    pub tick: u32,
    pub arrival_time: f32,
    pub payload: Vec<u8>,
}

pub struct JitterBuffer {
    buffer: Vec<JitterEntry>,
    target_delay: f32,
    max_buffer_size: usize,
    adaptive: bool,
    last_released_tick: u32,
    total_pushed: u64,
    total_dropped: u64,
    smoothed_delay: f32,
}

impl JitterBuffer {
    pub fn new(target_delay: f32, max_buffer_size: usize, adaptive: bool) -> Self {
        let clamped = target_delay.clamp(MIN_TARGET_DELAY, MAX_TARGET_DELAY);
        Self {
            buffer: Vec::new(),
            target_delay: clamped,
            max_buffer_size,
            adaptive,
            last_released_tick: 0,
            total_pushed: 0,
            total_dropped: 0,
            smoothed_delay: clamped,
        }
    }

    pub fn push(&mut self, tick: u32, arrival_time: f32, payload: Vec<u8>) {
        // Drop late packets
        if tick <= self.last_released_tick && self.last_released_tick > 0 {
            self.total_dropped += 1;
            return;
        }
        self.total_pushed += 1;

        // Insert sorted by tick
        let pos = self.buffer.partition_point(|e| e.tick < tick);
        self.buffer.insert(pos, JitterEntry { tick, arrival_time, payload });

        // Trim overflow
        while self.buffer.len() > self.max_buffer_size {
            self.buffer.remove(0);
            self.total_dropped += 1;
        }
    }

    pub fn flush(&mut self, current_time: f32) -> Vec<JitterEntry> {
        let mut ready = Vec::new();
        let threshold = current_time - self.target_delay;
        self.buffer.retain(|e| {
            if e.arrival_time <= threshold {
                ready.push(e.clone());
                false
            } else {
                true
            }
        });
        ready.sort_by_key(|e| e.tick);
        if let Some(last) = ready.last() {
            self.last_released_tick = last.tick;
        }
        if self.adaptive && !ready.is_empty() {
            let jitter = (current_time - ready.last().map(|e| e.arrival_time).unwrap_or(current_time)).abs();
            self.smoothed_delay = (1.0 - JITTER_ALPHA) * self.smoothed_delay + JITTER_ALPHA * jitter;
            self.target_delay = (self.smoothed_delay * ADAPTIVE_SCALE)
                .clamp(MIN_TARGET_DELAY, MAX_TARGET_DELAY);
        }
        ready
    }

    pub fn buffered_count(&self) -> usize { self.buffer.len() }
    pub fn target_delay(&self) -> f32 { self.target_delay }
    pub fn max_buffer_size(&self) -> usize { self.max_buffer_size }
    pub fn is_adaptive(&self) -> bool { self.adaptive }
    pub fn total_pushed(&self) -> u64 { self.total_pushed }
    pub fn total_dropped(&self) -> u64 { self.total_dropped }

    pub fn reset(&mut self) {
        self.buffer.clear();
        self.last_released_tick = 0;
        self.total_pushed = 0;
        self.total_dropped = 0;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn push_and_flush_ready() {
        let mut jb = JitterBuffer::new(0.05, 16, false);
        jb.push(1, 0.0, vec![1, 2, 3]);
        jb.push(2, 0.01, vec![4, 5, 6]);
        // time 0.10 > arrival + delay (0.0 + 0.05 = 0.05)
        let ready = jb.flush(0.10);
        assert_eq!(ready.len(), 2);
        assert_eq!(ready[0].tick, 1);
        assert_eq!(ready[1].tick, 2);
    }

    #[test]
    fn not_ready_before_delay() {
        let mut jb = JitterBuffer::new(0.2, 16, false);
        jb.push(1, 0.0, vec![]);
        // Only 0.05s elapsed, target delay is 0.2s
        let ready = jb.flush(0.05);
        assert!(ready.is_empty());
        assert_eq!(jb.buffered_count(), 1);
    }

    #[test]
    fn late_packet_dropped() {
        let mut jb = JitterBuffer::new(0.05, 16, false);
        jb.push(5, 0.0, vec![]);
        let _ = jb.flush(1.0); // release tick 5
        // tick 3 is older than last released (5) → should be dropped
        jb.push(3, 1.1, vec![]);
        assert_eq!(jb.total_dropped(), 1);
        assert_eq!(jb.buffered_count(), 0);
    }

    #[test]
    fn overflow_trims_buffer() {
        let mut jb = JitterBuffer::new(0.05, 3, false);
        for i in 0..5u32 {
            jb.push(i, 0.0, vec![i as u8]);
        }
        assert!(jb.buffered_count() <= 3);
        assert!(jb.total_dropped() > 0);
    }

    #[test]
    fn stats_track_push_count() {
        let mut jb = JitterBuffer::new(0.05, 16, false);
        jb.push(1, 0.0, vec![]);
        jb.push(2, 0.0, vec![]);
        assert_eq!(jb.total_pushed(), 2);
    }

    #[test]
    fn reset_clears_state() {
        let mut jb = JitterBuffer::new(0.05, 16, false);
        jb.push(1, 0.0, vec![]);
        jb.reset();
        assert_eq!(jb.buffered_count(), 0);
        assert_eq!(jb.total_pushed(), 0);
        assert_eq!(jb.total_dropped(), 0);
    }

    #[test]
    fn ordered_flush() {
        let mut jb = JitterBuffer::new(0.05, 16, false);
        // Push out of order
        jb.push(3, 0.0, vec![3]);
        jb.push(1, 0.0, vec![1]);
        jb.push(2, 0.0, vec![2]);
        let ready = jb.flush(1.0);
        assert_eq!(ready.iter().map(|e| e.tick).collect::<Vec<_>>(), vec![1, 2, 3]);
    }
}
