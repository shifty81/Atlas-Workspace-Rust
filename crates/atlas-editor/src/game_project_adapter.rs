//! [`GameProjectAdapter`] — Play-In-Editor (PIE) integration.
//!
//! The adapter trait decouples the editor from any specific game project.
//! An [`EditorSession`] wraps the active adapter and tracks PIE state.

use std::{
    process::{Child, Command, Stdio},
    sync::{Arc, Mutex},
};

use atlas_ecs::World;

// ── PIE state ────────────────────────────────────────────────────────────────

/// Lifecycle state of a Play-In-Editor session.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PieState {
    /// No PIE session is active.
    Idle,
    /// `atlas-game --pie` process is starting.
    Starting,
    /// Game is running in PIE mode.
    Running,
    /// PIE is being stopped; waiting for process exit.
    Stopping,
    /// Last PIE run ended with an error.
    Error(String),
}

// ── GameProjectAdapter ───────────────────────────────────────────────────────

/// Adapter trait between the editor and a game project.
///
/// Implement this to integrate project-specific PIE behavior.
/// The default implementation (`StandaloneGameAdapter`) launches the
/// `atlas-game` binary as a subprocess.
pub trait GameProjectAdapter: Send + Sync {
    /// Return the display name of the project.
    fn project_name(&self) -> &str;

    /// Called when the user starts PIE (F5).  Receives a snapshot of the
    /// current editor world so the game process can pre-load the scene.
    fn start_pie(&mut self, world: &World) -> Result<(), String>;

    /// Called when the user stops PIE (Shift-F5 or PIE toolbar button).
    fn stop_pie(&mut self);

    /// Called once per frame to poll child-process status.
    fn poll(&mut self) -> PieState;
}

// ── StandaloneGameAdapter ────────────────────────────────────────────────────

/// Default adapter: launches the `atlas-game` binary as a subprocess.
pub struct StandaloneGameAdapter {
    child: Option<Child>,
    state: PieState,
}

impl StandaloneGameAdapter {
    pub fn new() -> Self {
        Self { child: None, state: PieState::Idle }
    }
}

impl Default for StandaloneGameAdapter {
    fn default() -> Self { Self::new() }
}

impl GameProjectAdapter for StandaloneGameAdapter {
    fn project_name(&self) -> &str { "AtlasGame (standalone)" }

    fn start_pie(&mut self, _world: &World) -> Result<(), String> {
        if self.state == PieState::Running || self.state == PieState::Starting {
            return Ok(()); // already running
        }

        log::info!("[PIE] Starting atlas-game subprocess …");
        self.state = PieState::Starting;

        // Remove any leftover stop signal from a previous session
        std::env::remove_var("ATLAS_PIE_STOP");

        match Command::new("cargo")
            .args(["run", "--bin", "atlas-game", "--", "--pie"])
            .stdout(Stdio::inherit())
            .stderr(Stdio::inherit())
            .spawn()
        {
            Ok(child) => {
                self.child = Some(child);
                self.state = PieState::Running;
                log::info!("[PIE] Game process started (PID {:?})", self.child.as_ref().map(|c| c.id()));
                Ok(())
            }
            Err(e) => {
                let msg = format!("Failed to spawn atlas-game: {e}");
                log::error!("[PIE] {msg}");
                self.state = PieState::Error(msg.clone());
                Err(msg)
            }
        }
    }

    fn stop_pie(&mut self) {
        if self.state == PieState::Idle { return; }
        log::info!("[PIE] Stopping PIE session");
        self.state = PieState::Stopping;
        // Signal the child to exit via the env-var protocol
        std::env::set_var("ATLAS_PIE_STOP", "1");
        if let Some(ref mut child) = self.child {
            let _ = child.kill();
        }
    }

    fn poll(&mut self) -> PieState {
        if let Some(ref mut child) = self.child {
            match child.try_wait() {
                Ok(Some(status)) => {
                    log::info!("[PIE] Game process exited: {status}");
                    self.child = None;
                    std::env::remove_var("ATLAS_PIE_STOP");
                    if status.success() {
                        self.state = PieState::Idle;
                    } else {
                        self.state = PieState::Error(format!("exit status: {status}"));
                    }
                }
                Ok(None) => {
                    // Still running
                    if self.state == PieState::Starting {
                        self.state = PieState::Running;
                    }
                }
                Err(e) => {
                    log::error!("[PIE] poll error: {e}");
                    self.child = None;
                    self.state = PieState::Error(e.to_string());
                }
            }
        }
        self.state.clone()
    }
}

// ── EditorSession ─────────────────────────────────────────────────────────────

/// Holds per-project editor state including the active PIE adapter.
pub struct EditorSession {
    /// The current project adapter.
    adapter:       Box<dyn GameProjectAdapter>,
    /// Snapshot of the last PIE state (updated each frame by polling).
    pie_state:     PieState,
    /// Shared build log for display in the console.
    pub build_log: Arc<Mutex<String>>,
}

impl EditorSession {
    /// Create a session with the default standalone adapter.
    pub fn new() -> Self {
        Self {
            adapter:   Box::new(StandaloneGameAdapter::new()),
            pie_state: PieState::Idle,
            build_log: Arc::new(Mutex::new(String::new())),
        }
    }

    /// Start a PIE session.
    pub fn start_pie(&mut self, world: &World) {
        if let Err(e) = self.adapter.start_pie(world) {
            log::error!("[EditorSession] PIE failed to start: {e}");
        }
    }

    /// Stop the active PIE session.
    pub fn stop_pie(&mut self) {
        self.adapter.stop_pie();
    }

    /// Poll adapter — call once per frame.
    pub fn poll(&mut self) -> &PieState {
        self.pie_state = self.adapter.poll();
        &self.pie_state
    }

    /// Current PIE state (without polling).
    pub fn pie_state(&self) -> &PieState { &self.pie_state }

    /// True if PIE is active (Starting or Running).
    pub fn is_playing(&self) -> bool {
        matches!(self.pie_state, PieState::Running | PieState::Starting)
    }

    /// Replace the adapter (e.g. when a different project is loaded).
    pub fn set_adapter(&mut self, adapter: Box<dyn GameProjectAdapter>) {
        self.adapter = adapter;
    }

    pub fn project_name(&self) -> &str { self.adapter.project_name() }
}

impl Default for EditorSession {
    fn default() -> Self { Self::new() }
}
