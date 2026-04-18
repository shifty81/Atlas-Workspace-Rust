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

    // Demo: new systems
    run_new_systems_demo();

    // Demo: new crates
    run_new_crates_demo();

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
        let total_asteroids: usize = galaxy.systems.iter()
            .flat_map(|s| s.asteroid_belts.iter())
            .map(|b| b.count())
            .sum();
        log::info!("    Total planets: {}  Total asteroids: {}", total_planets, total_asteroids);
    }
}

/// Demo the new systems added in this iteration.
fn run_new_systems_demo() {
    log::info!("── New Systems Demo ────────────────────────────────────────");

    // EventBus
    {
        use atlas_core::{EventBus, Event};
        let mut bus = EventBus::new();
        bus.subscribe("test", |e| log::info!("EventBus received: {}", e.event_type));
        bus.publish(Event::new("test"));
        log::info!("EventBus: {} published", bus.total_published());
    }

    // InputManager
    {
        use atlas_input::{InputManager, InputAction, InputDevice};
        let mut input = InputManager::new();
        input.init();
        input.bind_action(InputAction::Jump, InputDevice::Keyboard, 32, "Jump");
        input.inject_press(InputAction::Jump);
        log::info!("InputManager: jump held={}", input.is_held(InputAction::Jump));
        input.shutdown();
    }

    // PhysicsWorld
    {
        use atlas_physics::PhysicsWorld;
        let mut world = PhysicsWorld::new();
        world.init();
        let b = world.create_body(1.0, false);
        world.step(0.016);
        log::info!("PhysicsWorld: body_count={}, body={:?}", world.body_count(), world.get_body(b).map(|b| b.position));
        world.shutdown();
    }

    // TickScheduler
    {
        use atlas_sim::TickScheduler;
        let mut sched = TickScheduler::new();
        sched.set_tick_rate(60);
        sched.tick(|dt| log::info!("TickScheduler: tick dt={:.4}", dt));
        log::info!("TickScheduler: tick={}", sched.current_tick());
    }

    // ScriptVM
    {
        use atlas_script::{ScriptVM, CompiledScript};
        let mut vm = ScriptVM::new();
        let script = CompiledScript::new("demo");
        let _ = vm.execute(&script);
        log::info!("ScriptVM: steps={}", vm.step_count());
    }

    // AnimationGraph
    {
        use atlas_animation::AnimationGraph;
        let mut graph = AnimationGraph::new();
        graph.compile();
        log::info!("AnimationGraph: nodes={}, compiled={}", graph.node_count(), graph.is_compiled());
    }

    // BehaviorGraph / AIMemory
    {
        use atlas_ai::{BehaviorGraph, AIMemory};
        let mut graph = BehaviorGraph::new();
        graph.compile();
        let mut mem = AIMemory::new();
        mem.store("target", "enemy_01", 1.0, 0.01, 0);
        log::info!("BehaviorGraph: compiled={}, AIMemory count={}", graph.is_compiled(), mem.count());
    }

    // SoundGraph
    {
        use atlas_sound::SoundGraph;
        let mut graph = SoundGraph::new();
        graph.compile();
        log::info!("SoundGraph: compiled={}", graph.is_compiled());
    }

    // GraphVM
    {
        use atlas_graphvm::{GraphVM, Bytecode, VmContext};
        let mut vm = GraphVM::new();
        let bytecode = Bytecode::default();
        let mut ctx = VmContext::default();
        let _ = vm.execute(&bytecode, &mut ctx);
        log::info!("GraphVM: emitted_events={}", vm.emitted_events().len());
    }

    // JitterBuffer / ReplicationManager
    {
        use atlas_net::{JitterBuffer, ReplicationManager};
        let mut jb = JitterBuffer::new(0.05, 64, true);
        jb.push(1, 0.0, vec![1, 2, 3]);
        log::info!("JitterBuffer: buffered={}", jb.buffered_count());
        let rep = ReplicationManager::new();
        log::info!("ReplicationManager: rules={}", rep.rule_count());
    }

    // ModLoader
    {
        use atlas_world::{ModLoader, ModDescriptor, ModLoadResult};
        let mut loader = ModLoader::new();
        let result = loader.register_mod(ModDescriptor {
            id: "core-mod".into(),
            name: "Core Mod".into(),
            version: "1.0.0".into(),
            author: "Atlas".into(),
            description: "Demo mod".into(),
            dependencies: Vec::new(),
            entry_path: "mods/core/main.lua".into(),
            enabled: false,
        });
        assert_eq!(result, ModLoadResult::Success);
        loader.load_mod("core-mod");
        log::info!("ModLoader: mods={}, loaded={}", loader.mod_count(), loader.loaded_mod_count());
    }
}

fn run_new_crates_demo() {
    log::info!("── New Crates Demo ─────────────────────────────────────────");
    // atlas-schema
    {
        use atlas_schema::{SchemaValidator, SchemaDefinition};
        let mut validator = SchemaValidator::new();
        let schema = SchemaDefinition { id: "test".into(), version: 1, inputs: vec![], outputs: vec![], nodes: vec![] };
        let ok = validator.validate(&schema);
        log::info!("SchemaValidator: valid={}", ok);
    }
    // atlas-abi
    {
        use atlas_abi::{AbiRegistry, AbiCapsule, AbiVersion, ProjectAbiTarget};
        let mut registry = AbiRegistry::new();
        let mut cap = AbiCapsule::new(AbiVersion::new(1, 0), "v1.0".into());
        cap.set_complete(true);
        registry.register_capsule(cap);
        let target = ProjectAbiTarget { project_name: "demo".into(), target_abi: AbiVersion::new(1, 0), determinism_profile: "strict".into() };
        let bound = registry.bind_project(&target);
        log::info!("AbiRegistry: bound={}", bound);
    }
    // atlas-asset
    {
        use atlas_asset::{AssetRegistry, AssetMeta};
        let mut reg = AssetRegistry::new();
        let meta = AssetMeta::new("cube", "mesh", "meshes/cube.obj");
        reg.register(meta);
        log::info!("AssetRegistry: count={}", reg.count());
    }
}

