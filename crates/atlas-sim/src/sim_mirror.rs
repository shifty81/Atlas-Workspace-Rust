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
