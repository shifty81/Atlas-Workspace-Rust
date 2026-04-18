//! [`GameBuildSystem`] — invokes `cargo build --bin atlas-game` in a
//! subprocess and reports the result back to the editor.

use std::{
    process::{Command, Stdio},
    sync::{Arc, Mutex},
    thread,
};

// ── Build status ─────────────────────────────────────────────────────────────

/// Status of the most recent build.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum BuildStatus {
    /// No build has been started yet.
    Idle,
    /// Cargo is currently compiling.
    Building,
    /// Compilation succeeded.
    Success,
    /// Compilation failed; contains compiler output.
    Failed(String),
}

// ── GameBuildSystem ──────────────────────────────────────────────────────────

/// Manages building the `atlas-game` binary from within the editor.
///
/// Invokes `cargo build --bin atlas-game` in a background thread so the
/// editor UI stays responsive during compilation.
pub struct GameBuildSystem {
    /// Shared status; updated by the background thread.
    status:      Arc<Mutex<BuildStatus>>,
    /// Captured stdout+stderr from the last build.
    log_output:  Arc<Mutex<String>>,
    /// True while a build is in progress.
    building:    bool,
}

impl GameBuildSystem {
    pub fn new() -> Self {
        Self {
            status:     Arc::new(Mutex::new(BuildStatus::Idle)),
            log_output: Arc::new(Mutex::new(String::new())),
            building:   false,
        }
    }

    /// Start a background build.  Does nothing if a build is already running.
    pub fn start_build(&mut self, release: bool) {
        if self.building { return; }

        // Resolve the workspace root relative to the editor's working directory.
        let workspace_root = std::env::current_dir()
            .unwrap_or_else(|_| std::path::PathBuf::from("."));

        let status_shared = Arc::clone(&self.status);
        let log_output = Arc::clone(&self.log_output);

        *status_shared.lock().unwrap() = BuildStatus::Building;
        *log_output.lock().unwrap()    = String::new();
        self.building = true;

        thread::spawn(move || {
            log::info!("[BuildSystem] Starting {} build of atlas-game …",
                if release { "release" } else { "debug" });

            let mut cmd = Command::new("cargo");
            cmd.arg("build").arg("--bin").arg("atlas-game");
            if release { cmd.arg("--release"); }
            cmd.current_dir(&workspace_root)
                .stdout(Stdio::piped())
                .stderr(Stdio::piped());

            match cmd.output() {
                Ok(out) => {
                    let combined = format!(
                        "{}\n{}",
                        String::from_utf8_lossy(&out.stdout),
                        String::from_utf8_lossy(&out.stderr),
                    );
                    *log_output.lock().unwrap() = combined.clone();
                    if out.status.success() {
                        log::info!("[BuildSystem] Build succeeded");
                        *status_shared.lock().unwrap() = BuildStatus::Success;
                    } else {
                        log::error!("[BuildSystem] Build failed:\n{combined}");
                        *status_shared.lock().unwrap() = BuildStatus::Failed(combined);
                    }
                }
                Err(e) => {
                    let msg = format!("Failed to invoke cargo: {e}");
                    log::error!("[BuildSystem] {msg}");
                    *status_shared.lock().unwrap() = BuildStatus::Failed(msg);
                }
            }
        });
    }

    /// Poll the build status.  Updates internal `building` flag.
    pub fn poll_status(&mut self) -> BuildStatus {
        let s = self.status.lock().unwrap().clone();
        if s != BuildStatus::Building { self.building = false; }
        s
    }

    /// Retrieve the captured build log.
    pub fn log_output(&self) -> String {
        self.log_output.lock().unwrap().clone()
    }

    pub fn is_building(&self) -> bool { self.building }
}

impl Default for GameBuildSystem {
    fn default() -> Self { Self::new() }
}
