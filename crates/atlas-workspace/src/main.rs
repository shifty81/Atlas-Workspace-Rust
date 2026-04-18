//! Atlas Workspace — main executable entry-point.
//!
//! Boots the Rust/Vulkan workspace, runs a demo PCG world generation pass,
//! and prints a summary to stdout.

use atlas_core::Logger;
use atlas_pcg::{PcgManager, PcgDomain, SeedLevel};
use atlas_renderer::{RenderConfig, VulkanContext};
use atlas_world::{Universe, UniverseConfig};

fn main() -> anyhow::Result<()> {
    // Initialise logging
    Logger::init();

    log::info!("=== Atlas Workspace v{} ===", atlas_core::VERSION);
    log::info!("Renderer backend: Vulkan (ash)");

    // Initialise Vulkan context
    let render_cfg = RenderConfig {
        title:  "Atlas Workspace".into(),
        width:  1920,
        height: 1080,
        validation_layers: cfg!(debug_assertions),
        ..RenderConfig::default()
    };
    let ctx = VulkanContext::new(render_cfg)?;
    log::info!("Vulkan context: {}", ctx.backend_description());

    // Demo: PCG world generation
    run_pcg_demo();

    // Demo: full universe generation
    run_universe_demo();

    Logger::shutdown();
    Ok(())
}

/// Exercise all 16 PCG domains.
fn run_pcg_demo() {
    log::info!("── PCG Domain Demo ────────────────────────────────────────");
    let mgr = PcgManager::new(0xDEADBEEF_CAFEF00D);
    log::info!("Universe seed: {:#018x}", mgr.universe_seed());

    for domain in PcgDomain::all() {
        let mut ctx = mgr.create_context(domain, SeedLevel::Object, 42);
        let sample = ctx.rng.next_float();
        log::info!("  {:12} seed={:#018x}  sample={:.6}", domain.name(), mgr.domain_seed(domain), sample);
    }

    // PCG terrain generation
    {
        use atlas_pcg::{TerrainConfig, TerrainGenerator};
        let gen = TerrainGenerator::new(TerrainConfig {
            width: 512, height: 512, seed: 0xDEADCAFE, max_height: 4000.0,
            octaves: 8, frequency: 1.5, persistence: 0.55, lacunarity: 2.1,
            cell_size: 10.0,
        });
        let hm = gen.generate();
        let min = hm.data.iter().cloned().fold(f32::INFINITY, f32::min);
        let max = hm.data.iter().cloned().fold(f32::NEG_INFINITY, f32::max);
        log::info!("Terrain 512×512: min={:.1}m  max={:.1}m", min, max);
    }

    // Mesh graph
    {
        use atlas_pcg::{MeshGraph, MeshNodeType, MeshEdge};
        let mut g = MeshGraph::new();
        let sphere = g.add_node(MeshNodeType::Primitive);
        g.set_node_property(sphere, "shape", "sphere");
        g.set_node_property(sphere, "radius", "50.0");
        g.set_node_property(sphere, "stacks", "32");
        g.set_node_property(sphere, "slices", "32");
        let noise = g.add_node(MeshNodeType::Noise);
        g.set_node_property(noise, "scale", "5.0");
        let out = g.add_node(MeshNodeType::Output);
        g.add_edge(MeshEdge { from_node: sphere, from_port: 0, to_node: noise, to_port: 0 });
        g.add_edge(MeshEdge { from_node: noise,  from_port: 0, to_node: out,   to_port: 0 });
        assert!(g.compile());
        assert!(g.execute());
        if let Some(mesh) = g.get_output() {
            log::info!("Mesh graph output: {} verts, {} tris", mesh.vertex_count(), mesh.triangle_count());
        }
    }

    // Planetary base
    {
        use atlas_pcg::PlanetaryBase;
        let mut base = PlanetaryBase::new();
        base.generate(0xDEADBEEF);
        log::info!("Planetary base: {} zones, operational={}", base.zone_count(), base.operational_zone_count());
    }
}

/// Generate a full universe and print the hierarchy.
fn run_universe_demo() {
    log::info!("── Universe Generation Demo ────────────────────────────────");
    let u = Universe::generate(UniverseConfig {
        seed:         0xC0FFEE42,
        galaxy_count: 1,
        version:      1,
    });
    log::info!("Universe seed: {:#018x}", u.seed());
    for (gi, galaxy) in u.galaxies.iter().enumerate() {
        log::info!("  Galaxy {}: {} systems, {} clusters",
            gi,
            galaxy.systems.len(),
            galaxy.clusters.len(),
        );
        let total_planets: usize = galaxy.systems.iter().map(|s| s.planets.len()).sum();
        let total_asteroids: usize = galaxy.systems.iter().map(|s| s.asteroid_belts.iter().map(|b| b.count()).sum::<usize>()).sum();
        log::info!("    Total planets: {}  Total asteroids: {}", total_planets, total_asteroids);
    }
}
