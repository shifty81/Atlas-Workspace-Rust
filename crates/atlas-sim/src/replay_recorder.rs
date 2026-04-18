use std::fs::File;
use std::io::{Read, Write};

#[derive(Debug, Clone, Default)]
pub struct ReplayFrame {
    pub tick: u32,
    pub input_data: Vec<u8>,
    pub state_hash: u64,
    pub is_save_point: bool,
}

#[derive(Debug, Clone)]
pub struct ReplayHeader {
    pub magic: u32,
    pub version: u32,
    pub tick_rate: u32,
    pub frame_count: u32,
    pub seed: u32,
}

impl Default for ReplayHeader {
    fn default() -> Self {
        Self { magic: 0x52504C59, version: 3, tick_rate: 0, frame_count: 0, seed: 0 }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum ReplayState { Idle, Recording, Playing }

impl Default for ReplayState { fn default() -> Self { ReplayState::Idle } }

#[derive(Debug, Default)]
pub struct ReplayRecorder {
    state: ReplayState,
    header: ReplayHeader,
    frames: Vec<ReplayFrame>,
}

impl ReplayRecorder {
    pub fn new() -> Self { Self::default() }

    pub fn start_recording(&mut self, tick_rate: u32, seed: u32) {
        self.clear();
        self.header.tick_rate = tick_rate;
        self.header.seed = seed;
        self.state = ReplayState::Recording;
    }

    pub fn start_from_save(&mut self, _save_tick: u32, tick_rate: u32, seed: u32) {
        self.start_recording(tick_rate, seed);
    }

    pub fn record_frame(&mut self, tick: u32, input_data: Vec<u8>) {
        self.record_frame_with_hash(tick, input_data, 0);
    }

    pub fn record_frame_with_hash(&mut self, tick: u32, input_data: Vec<u8>, state_hash: u64) {
        self.frames.push(ReplayFrame { tick, input_data, state_hash, is_save_point: false });
        self.header.frame_count = self.frames.len() as u32;
    }

    pub fn stop_recording(&mut self) {
        self.state = ReplayState::Idle;
        self.header.frame_count = self.frames.len() as u32;
    }

    pub fn load_replay(&mut self, path: &str) -> bool {
        let Ok(mut f) = File::open(path) else { return false; };
        let mut buf = Vec::new();
        if f.read_to_end(&mut buf).is_err() { return false; }
        let mut cursor = 0usize;
        macro_rules! read_u32 {
            () => {{
                if cursor + 4 > buf.len() { return false; }
                let v = u32::from_le_bytes(buf[cursor..cursor+4].try_into().unwrap());
                cursor += 4; v
            }};
        }
        macro_rules! read_u64 {
            () => {{
                if cursor + 8 > buf.len() { return false; }
                let v = u64::from_le_bytes(buf[cursor..cursor+8].try_into().unwrap());
                cursor += 8; v
            }};
        }
        let magic = read_u32!();
        if magic != 0x52504C59 { return false; }
        let version = read_u32!();
        let tick_rate = read_u32!();
        let frame_count = read_u32!();
        let seed = read_u32!();
        self.header = ReplayHeader { magic, version, tick_rate, frame_count, seed };
        self.frames.clear();
        for _ in 0..frame_count {
            let tick = read_u32!();
            let hash = read_u64!();
            if cursor + 1 > buf.len() { return false; }
            let is_save_point = buf[cursor] != 0;
            cursor += 1;
            let input_len = read_u32!() as usize;
            if cursor + input_len > buf.len() { return false; }
            let input_data = buf[cursor..cursor+input_len].to_vec();
            cursor += input_len;
            self.frames.push(ReplayFrame { tick, input_data, state_hash: hash, is_save_point });
        }
        self.state = ReplayState::Playing;
        true
    }

    pub fn save_replay(&self, path: &str) -> bool {
        let Ok(mut f) = File::create(path) else { return false; };
        macro_rules! write_u32 {
            ($v:expr) => { if f.write_all(&$v.to_le_bytes()).is_err() { return false; } };
        }
        macro_rules! write_u64 {
            ($v:expr) => { if f.write_all(&$v.to_le_bytes()).is_err() { return false; } };
        }
        write_u32!(self.header.magic);
        write_u32!(self.header.version);
        write_u32!(self.header.tick_rate);
        write_u32!(self.frames.len() as u32);
        write_u32!(self.header.seed);
        for frame in &self.frames {
            write_u32!(frame.tick);
            write_u64!(frame.state_hash);
            if f.write_all(&[frame.is_save_point as u8]).is_err() { return false; }
            write_u32!(frame.input_data.len() as u32);
            if f.write_all(&frame.input_data).is_err() { return false; }
        }
        true
    }

    pub fn frame_at_tick(&self, tick: u32) -> Option<&ReplayFrame> {
        self.frames.iter().find(|f| f.tick == tick)
    }

    pub fn mark_save_point(&mut self, tick: u32) {
        if let Some(f) = self.frames.iter_mut().find(|f| f.tick == tick) {
            f.is_save_point = true;
        }
    }

    pub fn save_points(&self) -> Vec<u32> {
        self.frames.iter().filter(|f| f.is_save_point).map(|f| f.tick).collect()
    }

    pub fn state(&self) -> ReplayState { self.state.clone() }
    pub fn header(&self) -> &ReplayHeader { &self.header }
    pub fn frames(&self) -> &[ReplayFrame] { &self.frames }
    pub fn frame_count(&self) -> usize { self.frames.len() }
    pub fn duration_ticks(&self) -> u32 { self.frames.iter().map(|f| f.tick).max().unwrap_or(0) }

    pub fn clear(&mut self) {
        self.frames.clear();
        self.header = ReplayHeader::default();
        self.state = ReplayState::Idle;
    }
}
