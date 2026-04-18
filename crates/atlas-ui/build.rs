//! Build script for atlas-ui: compile GLSL shaders to SPIR-V if `glslc` is
//! available.  The compiled `.spv` files are written to `$OUT_DIR` so they
//! can be loaded at runtime via a path relative to that directory.
//!
//! If `glslc` is not found the build still succeeds — the GPU renderer will
//! detect missing shaders at runtime and log a warning.

use std::path::Path;
use std::process::Command;

fn main() {
    println!("cargo:rerun-if-changed=shaders/egui.vert");
    println!("cargo:rerun-if-changed=shaders/egui.frag");
    println!("cargo:rerun-if-changed=shaders/scene.vert");
    println!("cargo:rerun-if-changed=shaders/scene.frag");

    let out = std::env::var("OUT_DIR").expect("OUT_DIR must be set");

    let shaders = &[
        ("shaders/egui.vert",  "egui.vert.spv"),
        ("shaders/egui.frag",  "egui.frag.spv"),
        ("shaders/scene.vert", "scene.vert.spv"),
        ("shaders/scene.frag", "scene.frag.spv"),
    ];

    for (src, dst) in shaders {
        let dst_path = Path::new(&out).join(dst);
        match Command::new("glslc")
            .args([src, "-o", dst_path.to_str().unwrap()])
            .output()
        {
            Ok(output) if output.status.success() => {
                eprintln!("cargo:info: compiled {src} → {dst}");
            }
            Ok(output) => {
                let stderr = String::from_utf8_lossy(&output.stderr);
                eprintln!("cargo:warning=glslc error for {src}: {stderr}");
            }
            Err(e) => {
                // glslc not installed — skip; runtime will fall back gracefully
                eprintln!("cargo:warning=glslc not found ({e}), skipping {src}");
            }
        }
    }
}
