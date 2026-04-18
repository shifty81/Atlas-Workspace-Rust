pub trait ISimulation: Send + Sync {
    fn step(&mut self, input: &[u8]);
    fn world_hash(&self) -> u64;
    fn current_tick(&self) -> u64;
}

#[derive(Debug, Clone)]
pub struct MirrorDesyncEvent {
    pub tick: u64,
    pub server_hash: u64,
    pub client_hash: u64,
}

#[derive(Default)]
pub struct SimMirrorController {
    server: Option<Box<dyn ISimulation>>,
    client: Option<Box<dyn ISimulation>>,
    desyncs: Vec<MirrorDesyncEvent>,
    frame_count: u64,
    enabled: bool,
}

impl SimMirrorController {
    pub fn new() -> Self { Self { enabled: true, ..Self::default() } }

    pub fn set_server(&mut self, sim: Box<dyn ISimulation>) { self.server = Some(sim); }
    pub fn set_client(&mut self, sim: Box<dyn ISimulation>) { self.client = Some(sim); }

    pub fn step(&mut self, input: &[u8]) -> bool {
        if !self.enabled { return false; }
        let (Some(server), Some(client)) = (self.server.as_mut(), self.client.as_mut()) else { return false; };
        server.step(input);
        client.step(input);
        self.frame_count += 1;
        let sh = server.world_hash();
        let ch = client.world_hash();
        if sh != ch {
            self.desyncs.push(MirrorDesyncEvent { tick: self.frame_count, server_hash: sh, client_hash: ch });
            return true;
        }
        false
    }

    pub fn run_frames(&mut self, inputs: &[Vec<u8>]) -> u64 {
        let mut desyncs = 0u64;
        for input in inputs {
            if self.step(input) { desyncs += 1; }
        }
        desyncs
    }

    pub fn has_desync(&self) -> bool { !self.desyncs.is_empty() }
    pub fn desyncs(&self) -> &[MirrorDesyncEvent] { &self.desyncs }
    pub fn first_desync(&self) -> Option<&MirrorDesyncEvent> { self.desyncs.first() }
    pub fn frame_count(&self) -> u64 { self.frame_count }

    pub fn reset(&mut self) {
        self.desyncs.clear();
        self.frame_count = 0;
    }

    pub fn is_enabled(&self) -> bool { self.enabled }
    pub fn set_enabled(&mut self, e: bool) { self.enabled = e; }
}

#[cfg(test)]
mod tests {
    use super::*;

    // Minimal deterministic simulation: hash = tick mod N
    struct CounterSim { tick: u64 }
    impl ISimulation for CounterSim {
        fn step(&mut self, _input: &[u8]) { self.tick += 1; }
        fn world_hash(&self) -> u64 { self.tick }
        fn current_tick(&self) -> u64 { self.tick }
    }

    // Simulation that diverges after a configurable tick
    struct DivergeSim { tick: u64, diverge_at: u64 }
    impl ISimulation for DivergeSim {
        fn step(&mut self, _input: &[u8]) { self.tick += 1; }
        fn world_hash(&self) -> u64 {
            if self.tick >= self.diverge_at { self.tick + 0xFFFF } else { self.tick }
        }
        fn current_tick(&self) -> u64 { self.tick }
    }

    fn synced_controller() -> SimMirrorController {
        let mut ctrl = SimMirrorController::new();
        ctrl.set_server(Box::new(CounterSim { tick: 0 }));
        ctrl.set_client(Box::new(CounterSim { tick: 0 }));
        ctrl
    }

    #[test]
    fn no_desync_when_identical() {
        let mut ctrl = synced_controller();
        ctrl.run_frames(&[vec![], vec![], vec![]]);
        assert!(!ctrl.has_desync());
        assert_eq!(ctrl.frame_count(), 3);
    }

    #[test]
    fn desync_detected_on_diverge() {
        let mut ctrl = SimMirrorController::new();
        ctrl.set_server(Box::new(DivergeSim { tick: 0, diverge_at: 2 }));
        ctrl.set_client(Box::new(CounterSim { tick: 0 }));
        let desync_count = ctrl.run_frames(&[vec![], vec![], vec![]]);
        assert!(ctrl.has_desync());
        assert!(desync_count > 0);
        // After step 2 the server hash = tick+0xFFFF (≥ diverge_at=2), client = tick → mismatch at frame 2
        assert_eq!(ctrl.first_desync().unwrap().tick, 2);
    }

    #[test]
    fn reset_clears_desyncs() {
        let mut ctrl = SimMirrorController::new();
        ctrl.set_server(Box::new(DivergeSim { tick: 0, diverge_at: 1 }));
        ctrl.set_client(Box::new(CounterSim { tick: 0 }));
        ctrl.run_frames(&[vec![], vec![]]);
        assert!(ctrl.has_desync());
        ctrl.reset();
        assert!(!ctrl.has_desync());
        assert_eq!(ctrl.frame_count(), 0);
    }

    #[test]
    fn disabled_controller_returns_false() {
        let mut ctrl = synced_controller();
        ctrl.set_enabled(false);
        assert!(!ctrl.is_enabled());
        let result = ctrl.step(&[]);
        assert!(!result); // disabled always returns false
        assert_eq!(ctrl.frame_count(), 0);
    }

    #[test]
    fn step_without_sims_returns_false() {
        let mut ctrl = SimMirrorController::new();
        assert!(!ctrl.step(&[]));
    }

    #[test]
    fn desyncs_slice_contains_all_events() {
        let mut ctrl = SimMirrorController::new();
        ctrl.set_server(Box::new(DivergeSim { tick: 0, diverge_at: 1 }));
        ctrl.set_client(Box::new(CounterSim { tick: 0 }));
        ctrl.run_frames(&[vec![], vec![], vec![]]);
        assert!(!ctrl.desyncs().is_empty());
    }
}
