use crate::state_hasher::StateHasher;

#[derive(Debug, Clone)]
pub struct TickBreakpoint {
    pub id: u32,
    pub tick: u64,
    pub hash_mismatch: u64,
    pub enabled: bool,
    pub label: String,
}

#[derive(Debug, Default)]
pub struct TickStepDebugger {
    hasher: Option<*const StateHasher>,
    current_tick: u64,
    breakpoints: Vec<TickBreakpoint>,
    next_bp_id: u32,
    paused: bool,
    triggered_bp_id: u32,
}

// Safety: we only store raw pointer for optional reference, not used across threads in this demo context
unsafe impl Send for TickStepDebugger {}
unsafe impl Sync for TickStepDebugger {}

impl TickStepDebugger {
    pub fn new() -> Self { Self::default() }

    pub fn set_hasher(&mut self, hasher: &StateHasher) {
        self.hasher = Some(hasher as *const StateHasher);
    }

    pub fn current_tick(&self) -> u64 { self.current_tick }
    pub fn set_current_tick(&mut self, tick: u64) { self.current_tick = tick; }

    pub fn step_forward(&mut self, count: u64) {
        self.current_tick = self.current_tick.saturating_add(count);
    }

    pub fn step_backward(&mut self, count: u64) {
        self.current_tick = self.current_tick.saturating_sub(count);
    }

    pub fn jump_to_tick(&mut self, tick: u64) { self.current_tick = tick; }

    pub fn add_breakpoint(&mut self, mut bp: TickBreakpoint) -> u32 {
        let id = self.next_bp_id;
        self.next_bp_id += 1;
        bp.id = id;
        self.breakpoints.push(bp);
        id
    }

    pub fn remove_breakpoint(&mut self, id: u32) -> bool {
        let before = self.breakpoints.len();
        self.breakpoints.retain(|b| b.id != id);
        self.breakpoints.len() < before
    }

    pub fn enable_breakpoint(&mut self, id: u32, enabled: bool) -> bool {
        if let Some(bp) = self.breakpoints.iter_mut().find(|b| b.id == id) {
            bp.enabled = enabled;
            true
        } else { false }
    }

    pub fn breakpoints(&self) -> &[TickBreakpoint] { &self.breakpoints }
    pub fn breakpoint_count(&self) -> u32 { self.breakpoints.len() as u32 }
    pub fn clear_breakpoints(&mut self) { self.breakpoints.clear(); }

    pub fn check_breakpoints(&mut self) -> bool {
        for bp in &self.breakpoints {
            if !bp.enabled { continue; }
            if bp.tick == self.current_tick || (bp.hash_mismatch != 0) {
                self.triggered_bp_id = bp.id;
                self.paused = true;
                return true;
            }
        }
        false
    }

    pub fn triggered_breakpoint_id(&self) -> u32 { self.triggered_bp_id }

    pub fn add_hash_mismatch_breakpoint(&mut self, expected_hash: u64, label: String) -> u32 {
        let bp = TickBreakpoint {
            id: 0,
            tick: u64::MAX,
            hash_mismatch: expected_hash,
            enabled: true,
            label,
        };
        self.add_breakpoint(bp)
    }

    pub fn is_paused(&self) -> bool { self.paused }
    pub fn set_paused(&mut self, p: bool) { self.paused = p; }
}
