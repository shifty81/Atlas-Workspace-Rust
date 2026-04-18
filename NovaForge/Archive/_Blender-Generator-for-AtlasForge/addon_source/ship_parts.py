"""
Ship parts generation module for the NovaForge content pipeline.
Generates individual ship components (hull, cockpit, engines, wings, weapons, turrets)

Engine generation supports three archetypes (MAIN_THRUST, MANEUVERING,
UTILITY_EXHAUST) for visual variety defined in :mod:`brick_system`.

Hull generation uses per-class shape profiles so that different ship classes
produce distinctive silhouettes (e.g. wide/flat carriers vs long/narrow
frigates).  A seed-driven vertex noise pass adds organic uniqueness.
"""

import bpy
import bmesh
import random
import math
from . import brick_system


# Maximum number of turret hardpoints any ship may have
MAX_TURRET_HARDPOINTS = 10

# ---------------------------------------------------------------------------
# Per-class hull shape profiles (NovaForge classes only)
# ---------------------------------------------------------------------------
# Each profile defines (width, length, height) multipliers relative to the
# ship scale so every class gets a distinctive silhouette.

HULL_PROFILES = {
    'SHUTTLE':       (0.30, 0.90, 0.25),
    'FIGHTER':       (0.30, 1.40, 0.22),
    'CORVETTE':      (0.35, 1.30, 0.26),
    'FRIGATE':       (0.40, 1.20, 0.28),
    'DESTROYER':     (0.45, 1.30, 0.30),
    'CRUISER':       (0.50, 1.20, 0.32),
    'BATTLECRUISER': (0.55, 1.25, 0.35),
    'BATTLESHIP':    (0.60, 1.10, 0.38),
    'CARRIER':       (0.80, 1.00, 0.25),
    'DREADNOUGHT':   (0.55, 1.30, 0.40),
    'CAPITAL':       (0.65, 1.20, 0.38),
    'TITAN':         (0.60, 1.40, 0.35),
    'INDUSTRIAL':    (0.60, 0.90, 0.50),
    'MINING_BARGE':  (0.70, 0.80, 0.45),
    'EXHUMER':       (0.65, 0.90, 0.40),
    'EXPLORER':      (0.30, 1.30, 0.24),
    'HAULER':        (0.65, 0.85, 0.48),
    'EXOTIC':        (0.35, 1.50, 0.22),
}

# Default profile when ship_class is unknown
_DEFAULT_PROFILE = (0.50, 1.00, 0.30)


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def create_mesh_object(name, verts, edges, faces):
    """Helper function to create a mesh object from vertices, edges, and faces"""
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, edges, faces)
    mesh.update()
    
    obj = bpy.data.objects.new(name, mesh)
    return obj


def generate_hull(segments=5, scale=1.0, complexity=1.0, symmetry=True, style='SOLARI',
                  naming_prefix='', ship_class='FRIGATE', seed=0):
    """
    Generate the main hull of the spaceship

    Uses per-class hull profiles for distinctive silhouettes and applies
    seed-driven vertex noise for organic uniqueness.

    Args:
        segments: Number of hull segments
        scale: Overall scale factor
        complexity: Geometry complexity (0.1-3.0)
        symmetry: Use symmetrical design
        style: NovaForge faction style
        naming_prefix: Project naming prefix
        ship_class: Ship class key used to select hull profile
        seed: Random seed used for vertex noise
    """
    # Create base hull mesh
    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, 0, 0))
    hull = bpy.context.active_object
    hull.name = _prefixed_name(naming_prefix, "Hull")

    # Use per-class profile for distinctive silhouettes
    profile = HULL_PROFILES.get(ship_class, _DEFAULT_PROFILE)
    hull.scale = (scale * profile[0], scale * profile[1], scale * profile[2])
    bpy.ops.object.transform_apply(scale=True)
    
    # Enter edit mode to modify geometry
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    
    # Add subdivisions based on complexity
    subdiv_levels = max(1, int(complexity * 2))
    bpy.ops.mesh.subdivide(number_cuts=subdiv_levels)
    
    # Add some variation to vertices
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Apply NovaForge faction style modifications
    if style == 'SOLARI':
        apply_solari_style(hull, scale)
    elif style == 'VEYREN':
        apply_veyren_style(hull, scale)
    elif style == 'AURELIAN':
        apply_aurelian_style(hull, scale)
    elif style == 'KELDARI':
        apply_keldari_style(hull, scale)
    else:
        apply_solari_style(hull, scale)
    
    # Apply seed-driven vertex noise for organic uniqueness
    _apply_hull_vertex_noise(hull, scale, seed, complexity)

    # Add smooth shading
    bpy.ops.object.shade_smooth()

    # Add subdivision surface modifier for smoother look
    modifier = hull.modifiers.new(name="Subdivision", type='SUBSURF')
    modifier.levels = 1
    modifier.render_levels = 2

    # Add edge split for hard edges
    edge_split = hull.modifiers.new(name="EdgeSplit", type='EDGE_SPLIT')
    edge_split.split_angle = math.radians(30)

    return hull


def apply_solari_style(hull, scale):
    """Apply Solari faction style - golden, elegant, armor-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.08 * scale, segments=3)
    bpy.ops.object.mode_set(mode='OBJECT')

    # Add smooth curves for elegant look
    cast_mod = hull.modifiers.new(name="Cast", type='CAST')
    cast_mod.factor = 0.15
    cast_mod.cast_type = 'SPHERE'


def apply_veyren_style(hull, scale):
    """Apply Veyren faction style - angular, utilitarian, shield-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.12 * scale, segments=1)
    bpy.ops.object.mode_set(mode='OBJECT')


def apply_aurelian_style(hull, scale):
    """Apply Aurelian faction style - sleek, organic, drone-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.subdivide(number_cuts=1)
    bpy.ops.object.mode_set(mode='OBJECT')

    cast_mod = hull.modifiers.new(name="Cast", type='CAST')
    cast_mod.factor = 0.25
    cast_mod.cast_type = 'SPHERE'


def apply_keldari_style(hull, scale):
    """Apply Keldari faction style - rugged, industrial, missile-focused"""
    bpy.context.view_layer.objects.active = hull
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.bevel(offset=0.15 * scale, segments=1)
    bpy.ops.object.mode_set(mode='OBJECT')


# ---------------------------------------------------------------------------
# Faction-specific detail generators for NovaForge
# ---------------------------------------------------------------------------

def generate_launcher_hardpoints(count=2, scale=1.0, symmetry=True, naming_prefix=''):
    """Generate missile/torpedo launcher bay visual indicators.

    Launchers use recessed box geometry for missile bays, placed along the
    dorsal-forward hull.  This contrasts with turret hardpoints which use
    cylindrical geometry for beam/projectile weapons.

    Args:
        count: Number of launcher hardpoints
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        naming_prefix: Project naming prefix

    Returns:
        List of launcher hardpoint objects.
    """
    launchers = []
    launcher_size = scale * 0.08

    positions = []
    if symmetry and count % 2 == 0:
        for i in range(count // 2):
            y_pos = scale * 0.2 + (i * scale * 0.18)
            x_offset = scale * 0.15 + (i * scale * 0.08)
            positions.append((x_offset, y_pos, scale * 0.08))
            positions.append((-x_offset, y_pos, scale * 0.08))
    else:
        for i in range(count):
            y_pos = scale * 0.15 + (i * scale * 0.18)
            x_pos = (i - count / 2) * scale * 0.15
            positions.append((x_pos, y_pos, scale * 0.08))

    for i, pos in enumerate(positions):
        bpy.ops.mesh.primitive_cube_add(size=launcher_size, location=pos)
        launcher = bpy.context.active_object
        launcher.name = _prefixed_name(naming_prefix, f"Launcher_{i+1}")
        launcher.scale = (0.6, 1.4, 0.5)
        bpy.ops.object.transform_apply(scale=True)

        # Recessed bay opening (smaller cube subtracted visually)
        bay_pos = (pos[0], pos[1] + launcher_size * 0.4, pos[2])
        bpy.ops.mesh.primitive_cube_add(
            size=launcher_size * 0.5,
            location=bay_pos,
        )
        bay = bpy.context.active_object
        bay.name = _prefixed_name(naming_prefix, f"Launcher_Bay_{i+1}")
        bay.parent = launcher

        launcher["hardpoint_type"] = "launcher"
        launcher["launcher_index"] = i + 1
        launchers.append(launcher)

    return launchers


def generate_drone_bays(count=1, scale=1.0, naming_prefix=''):
    """Generate drone bay visual indicators on the ventral hull.

    Drone bays are flat, wide openings on the underside of the ship.

    Args:
        count: Number of drone bays
        scale: Ship scale factor
        naming_prefix: Project naming prefix

    Returns:
        List of drone bay objects.
    """
    bays = []
    bay_size = scale * 0.12

    for i in range(count):
        y_pos = -scale * 0.1 + (i * scale * 0.15)
        pos = (0, y_pos, -scale * 0.12)

        bpy.ops.mesh.primitive_cube_add(size=bay_size, location=pos)
        bay = bpy.context.active_object
        bay.name = _prefixed_name(naming_prefix, f"Drone_Bay_{i+1}")
        bay.scale = (1.8, 1.2, 0.3)
        bpy.ops.object.transform_apply(scale=True)

        # Bay door lines (two thin cubes)
        for side in (-1, 1):
            door_pos = (pos[0] + side * bay_size * 0.5, pos[1], pos[2])
            bpy.ops.mesh.primitive_cube_add(
                size=bay_size * 0.1,
                location=door_pos,
            )
            door = bpy.context.active_object
            door.name = _prefixed_name(
                naming_prefix, f"Drone_Bay_{i+1}_Door_{'L' if side < 0 else 'R'}")
            door.scale = (0.2, 1.0, 0.5)
            bpy.ops.object.transform_apply(scale=True)
            door.parent = bay

        bay["hardpoint_type"] = "drone_bay"
        bay["drone_bay_index"] = i + 1
        bays.append(bay)

    return bays


def add_faction_details(hull, scale, style, seed, naming_prefix=''):
    """Add faction-specific surface details to a hull.

    Dispatches to per-faction helpers that add distinctive geometry
    matching NovaForge's design language for each race.

    Args:
        hull: The hull mesh object.
        scale: Ship scale factor.
        style: Faction style string (SOLARI, VEYREN, AURELIAN, KELDARI).
        seed: Random seed.
        naming_prefix: Project naming prefix.

    Returns:
        List of created detail objects.
    """
    rng = random.Random(seed + 200)
    details = []

    if style == 'SOLARI':
        details.extend(_add_solari_spires(hull, scale, rng, naming_prefix))
    elif style == 'VEYREN':
        details.extend(_add_veyren_panels(hull, scale, rng, naming_prefix))
    elif style == 'AURELIAN':
        details.extend(_add_aurelian_curves(hull, scale, rng, naming_prefix))
    elif style == 'KELDARI':
        details.extend(_add_keldari_framework(hull, scale, rng, naming_prefix))

    return details


def _add_solari_spires(hull, scale, rng, naming_prefix):
    """Add Solari cathedral spires and golden trim."""
    details = []
    spire_count = max(2, int(scale * 0.3))

    for i in range(spire_count):
        y_pos = rng.uniform(-scale * 0.3, scale * 0.4)
        x_pos = rng.uniform(-scale * 0.08, scale * 0.08)
        height = scale * rng.uniform(0.12, 0.22)

        bpy.ops.mesh.primitive_cone_add(
            radius1=scale * 0.02,
            radius2=0,
            depth=height,
            location=(x_pos, y_pos, scale * 0.15 + height * 0.5),
        )
        spire = bpy.context.active_object
        spire.name = _prefixed_name(naming_prefix, f"Solari_Spire_{i+1}")
        spire.parent = hull

        # Gold material
        mat = bpy.data.materials.new(name=f"Solari_Gold_{i}")
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get('Principled BSDF')
        if bsdf:
            bsdf.inputs['Base Color'].default_value = (0.8, 0.65, 0.2, 1.0)
            bsdf.inputs['Metallic'].default_value = 0.95
            bsdf.inputs['Roughness'].default_value = 0.2
        spire.data.materials.append(mat)
        details.append(spire)

    return details


def _add_veyren_panels(hull, scale, rng, naming_prefix):
    """Add Veyren blocky armor panels and antenna arrays."""
    details = []
    panel_count = max(3, int(scale * 0.4))

    for i in range(panel_count):
        y_pos = rng.uniform(-scale * 0.5, scale * 0.4)
        side = rng.choice([-1, 1])
        x_pos = side * scale * rng.uniform(0.18, 0.28)

        bpy.ops.mesh.primitive_cube_add(
            size=scale * 0.06,
            location=(x_pos, y_pos, scale * rng.uniform(0.05, 0.14)),
        )
        panel = bpy.context.active_object
        panel.name = _prefixed_name(naming_prefix, f"Veyren_Panel_{i+1}")
        panel.scale = (1.0, rng.uniform(1.2, 2.0), 0.4)
        bpy.ops.object.transform_apply(scale=True)
        panel.parent = hull

        mat = bpy.data.materials.new(name=f"Veyren_Steel_{i}")
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get('Principled BSDF')
        if bsdf:
            bsdf.inputs['Base Color'].default_value = (0.35, 0.45, 0.55, 1.0)
            bsdf.inputs['Metallic'].default_value = 0.85
            bsdf.inputs['Roughness'].default_value = 0.35
        panel.data.materials.append(mat)
        details.append(panel)

    return details


def _add_aurelian_curves(hull, scale, rng, naming_prefix):
    """Add Aurelian organic curved pods and drone recesses."""
    details = []
    pod_count = max(2, int(scale * 0.25))

    for i in range(pod_count):
        y_pos = rng.uniform(-scale * 0.3, scale * 0.3)
        side = rng.choice([-1, 1])
        x_pos = side * scale * rng.uniform(0.2, 0.3)

        bpy.ops.mesh.primitive_uv_sphere_add(
            radius=scale * 0.04,
            location=(x_pos, y_pos, 0),
        )
        pod = bpy.context.active_object
        pod.name = _prefixed_name(naming_prefix, f"Aurelian_Pod_{i+1}")
        pod.scale = (0.8, 1.5, 0.6)
        bpy.ops.object.transform_apply(scale=True)
        bpy.ops.object.shade_smooth()
        pod.parent = hull

        mat = bpy.data.materials.new(name=f"Aurelian_Organic_{i}")
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get('Principled BSDF')
        if bsdf:
            bsdf.inputs['Base Color'].default_value = (0.2, 0.45, 0.35, 1.0)
            bsdf.inputs['Metallic'].default_value = 0.6
            bsdf.inputs['Roughness'].default_value = 0.35
        pod.data.materials.append(mat)
        details.append(pod)

    return details


def _add_keldari_framework(hull, scale, rng, naming_prefix):
    """Add Keldari exposed structural framework and asymmetric plating."""
    details = []
    strut_count = max(3, int(scale * 0.35))

    for i in range(strut_count):
        y_pos = rng.uniform(-scale * 0.5, scale * 0.4)
        side = rng.choice([-1, 1])
        x_pos = side * scale * rng.uniform(0.15, 0.3)
        z_pos = rng.uniform(-scale * 0.05, scale * 0.12)

        bpy.ops.mesh.primitive_cylinder_add(
            radius=scale * 0.01,
            depth=scale * rng.uniform(0.08, 0.18),
            location=(x_pos, y_pos, z_pos),
        )
        strut = bpy.context.active_object
        strut.name = _prefixed_name(naming_prefix, f"Keldari_Strut_{i+1}")
        angle = rng.uniform(-0.4, 0.4)
        strut.rotation_euler = (angle, rng.uniform(-0.3, 0.3), 0)
        strut.parent = hull

        mat = bpy.data.materials.new(name=f"Keldari_Rust_{i}")
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get('Principled BSDF')
        if bsdf:
            bsdf.inputs['Base Color'].default_value = (0.45, 0.3, 0.2, 1.0)
            bsdf.inputs['Metallic'].default_value = 0.7
            bsdf.inputs['Roughness'].default_value = 0.65
        strut.data.materials.append(mat)
        details.append(strut)

    # Add asymmetric armor plates
    plate_count = max(2, int(scale * 0.2))
    for i in range(plate_count):
        y_pos = rng.uniform(-scale * 0.4, scale * 0.3)
        x_pos = rng.uniform(-scale * 0.25, scale * 0.25)

        bpy.ops.mesh.primitive_cube_add(
            size=scale * 0.05,
            location=(x_pos, y_pos, scale * rng.uniform(0.08, 0.15)),
        )
        plate = bpy.context.active_object
        plate.name = _prefixed_name(naming_prefix, f"Keldari_Plate_{i+1}")
        plate.scale = (rng.uniform(0.8, 1.5), rng.uniform(0.8, 1.5), 0.3)
        plate.rotation_euler = (0, 0, rng.uniform(-0.2, 0.2))
        bpy.ops.object.transform_apply(scale=True)
        plate.parent = hull
        details.append(plate)

    return details


# Base magnitude for seed-driven hull vertex noise (fraction of scale)
_VERTEX_NOISE_BASE_MAGNITUDE = 0.012


def _apply_hull_vertex_noise(hull, scale, seed, complexity):
    """Displace hull vertices slightly using seed-driven noise.

    Each vertex is offset along its normal by a small random amount so
    that every seed produces a visually unique hull surface.  The
    displacement magnitude scales with *complexity* and *scale* to keep
    the effect proportional.
    """
    rng = random.Random(seed)
    magnitude = scale * _VERTEX_NOISE_BASE_MAGNITUDE * complexity
    mesh = hull.data
    for v in mesh.vertices:
        offset = rng.uniform(-magnitude, magnitude)
        v.co.x += v.normal.x * offset
        v.co.y += v.normal.y * offset
        v.co.z += v.normal.z * offset
    mesh.update()


def generate_cockpit(scale=1.0, position=(0, 0, 0), ship_class='FRIGATE', style='SOLARI',
                     naming_prefix=''):
    """
    Generate cockpit/bridge for the ship
    
    Args:
        scale: Ship scale factor
        position: Position relative to hull
        ship_class: Type of ship
        style: Design style
        naming_prefix: Project naming prefix
    """
    # Create cockpit as a modified cube
    bpy.ops.mesh.primitive_cube_add(size=scale * 0.3, location=position)
    cockpit = bpy.context.active_object
    cockpit.name = _prefixed_name(naming_prefix, "Cockpit")
    
    # Scale to appropriate proportions
    cockpit.scale = (0.8, 1.2, 0.6)
    bpy.ops.object.transform_apply(scale=True)
    
    # Taper the front for viewing angle
    bpy.context.view_layer.objects.active = cockpit
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.subdivide(number_cuts=2)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Add smooth shading
    bpy.ops.object.shade_smooth()
    
    return cockpit


def generate_engines(count=2, scale=1.0, symmetry=True, style='SOLARI', naming_prefix=''):
    """
    Generate engine units with archetype-based variation.

    Engines are assigned one of three archetypes (MAIN_THRUST, MANEUVERING,
    UTILITY_EXHAUST) based on their index.  Main engines are larger with
    nozzle flares; maneuvering thrusters are small; utility vents are flat.

    Args:
        count: Number of engines
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        style: Design style
        naming_prefix: Project naming prefix
    """
    engines = []
    base_engine_size = scale * 0.2

    # Position engines at the rear of the ship
    rear_position = -scale * 0.9

    # Pre-compute archetype for each logical engine slot
    archetypes = [
        brick_system.select_engine_archetype(i, count)
        for i in range(count)
    ]

    if symmetry and count % 2 == 0:
        # Symmetric engine placement
        spacing = scale * 0.3
        for i in range(count // 2):
            offset = spacing * (i + 0.5)

            arch_name = archetypes[i * 2]
            arch = brick_system.get_engine_archetype(arch_name)
            engine_size = base_engine_size * arch['radius_factor']
            depth = engine_size * 2 * random.uniform(*arch['depth_range'])

            # Left engine
            bpy.ops.mesh.primitive_cylinder_add(
                radius=engine_size,
                depth=depth,
                location=(offset, rear_position, 0)
            )
            left_engine = bpy.context.active_object
            left_engine.name = _prefixed_name(naming_prefix, f"Engine_L{i+1}")
            left_engine.rotation_euler = (math.radians(90), 0, 0)
            left_engine["engine_archetype"] = arch_name
            engines.append(left_engine)

            # Add nozzle flare for main thrust engines
            if arch['has_nozzle_flare']:
                _add_nozzle_flare(left_engine, engine_size, naming_prefix)
            if arch.get('inner_cone'):
                _add_inner_cone(left_engine, engine_size, naming_prefix)
            if arch.get('exhaust_rings', 0) > 0:
                _add_exhaust_rings(left_engine, engine_size,
                                   arch['exhaust_rings'], naming_prefix)

            # Right engine
            bpy.ops.mesh.primitive_cylinder_add(
                radius=engine_size,
                depth=depth,
                location=(-offset, rear_position, 0)
            )
            right_engine = bpy.context.active_object
            right_engine.name = _prefixed_name(naming_prefix, f"Engine_R{i+1}")
            right_engine.rotation_euler = (math.radians(90), 0, 0)
            right_engine["engine_archetype"] = arch_name
            engines.append(right_engine)

            if arch['has_nozzle_flare']:
                _add_nozzle_flare(right_engine, engine_size, naming_prefix)
            if arch.get('inner_cone'):
                _add_inner_cone(right_engine, engine_size, naming_prefix)
            if arch.get('exhaust_rings', 0) > 0:
                _add_exhaust_rings(right_engine, engine_size,
                                   arch['exhaust_rings'], naming_prefix)
    else:
        # Non-symmetric or odd count
        for i in range(count):
            x_offset = (i - count / 2) * scale * 0.3

            arch_name = archetypes[i]
            arch = brick_system.get_engine_archetype(arch_name)
            engine_size = base_engine_size * arch['radius_factor']
            depth = engine_size * 2 * random.uniform(*arch['depth_range'])

            bpy.ops.mesh.primitive_cylinder_add(
                radius=engine_size,
                depth=depth,
                location=(x_offset, rear_position, 0)
            )
            engine = bpy.context.active_object
            engine.name = _prefixed_name(naming_prefix, f"Engine_{i+1}")
            engine.rotation_euler = (math.radians(90), 0, 0)
            engine["engine_archetype"] = arch_name
            engines.append(engine)

            if arch['has_nozzle_flare']:
                _add_nozzle_flare(engine, engine_size, naming_prefix)
            if arch.get('inner_cone'):
                _add_inner_cone(engine, engine_size, naming_prefix)
            if arch.get('exhaust_rings', 0) > 0:
                _add_exhaust_rings(engine, engine_size,
                                   arch['exhaust_rings'], naming_prefix)

    # Add glow material to engines (strength varies by archetype)
    for engine in engines:
        arch_name = engine.get("engine_archetype", "MAIN_THRUST")
        arch = brick_system.get_engine_archetype(arch_name) or brick_system.ENGINE_ARCHETYPES['MAIN_THRUST']

        mat = bpy.data.materials.new(name="Engine_Glow")
        mat.use_nodes = True
        nodes = mat.node_tree.nodes

        # Add emission shader
        emission = nodes.new(type='ShaderNodeEmission')
        emission.inputs['Color'].default_value = (0.2, 0.5, 1.0, 1.0)  # Blue glow
        emission.inputs['Strength'].default_value = arch['glow_strength']

        output = nodes.get('Material Output')
        mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])

        engine.data.materials.append(mat)

    return engines


def _add_nozzle_flare(engine_obj, engine_size, naming_prefix=''):
    """Add a cone-shaped nozzle flare to a main-thrust engine."""
    loc = engine_obj.location
    bpy.ops.mesh.primitive_cone_add(
        radius1=engine_size * 1.3,
        radius2=engine_size * 0.9,
        depth=engine_size * 0.5,
        location=(loc.x, loc.y - engine_size * 1.2, loc.z)
    )
    flare = bpy.context.active_object
    flare.name = _prefixed_name(naming_prefix, f"{engine_obj.name}_Flare")
    flare.rotation_euler = (math.radians(90), 0, 0)
    flare.parent = engine_obj


def _add_exhaust_rings(engine_obj, engine_size, ring_count, naming_prefix=''):
    """Add torus exhaust rings around an engine for visual detail."""
    loc = engine_obj.location
    base_offset = 0.4   # first ring distance (fraction of engine_size)
    ring_spacing = 0.5   # spacing between successive rings
    for r in range(ring_count):
        ring_offset = -engine_size * (base_offset + r * ring_spacing)
        bpy.ops.mesh.primitive_torus_add(
            major_radius=engine_size * (1.05 + r * 0.05),
            minor_radius=engine_size * 0.04,
            location=(loc.x, loc.y + ring_offset, loc.z)
        )
        ring = bpy.context.active_object
        ring.name = _prefixed_name(
            naming_prefix, f"{engine_obj.name}_ExhaustRing_{r+1}")
        ring.rotation_euler = (math.radians(90), 0, 0)
        ring.parent = engine_obj


def _add_inner_cone(engine_obj, engine_size, naming_prefix=''):
    """Add an inner cone detail inside main thrust engines."""
    loc = engine_obj.location
    bpy.ops.mesh.primitive_cone_add(
        radius1=engine_size * 0.5,
        radius2=engine_size * 0.15,
        depth=engine_size * 0.8,
        location=(loc.x, loc.y - engine_size * 0.3, loc.z)
    )
    cone = bpy.context.active_object
    cone.name = _prefixed_name(naming_prefix, f"{engine_obj.name}_InnerCone")
    cone.rotation_euler = (math.radians(90), 0, 0)
    cone.parent = engine_obj


def generate_wings(scale=1.0, symmetry=True, style='SOLARI', naming_prefix=''):
    """
    Generate wing structures
    
    Args:
        scale: Ship scale factor
        symmetry: Use symmetrical design
        style: Design style
        naming_prefix: Project naming prefix
    """
    wings = []
    wing_length = scale * 0.8
    wing_width = scale * 0.15
    
    # Left wing
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(wing_length * 0.5, 0, 0)
    )
    left_wing = bpy.context.active_object
    left_wing.name = _prefixed_name(naming_prefix, "Wing_Left")
    left_wing.scale = (wing_length, wing_width, wing_width * 0.3)
    bpy.ops.object.transform_apply(scale=True)
    wings.append(left_wing)
    
    if symmetry:
        # Right wing
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(-wing_length * 0.5, 0, 0)
        )
        right_wing = bpy.context.active_object
        right_wing.name = _prefixed_name(naming_prefix, "Wing_Right")
        right_wing.scale = (wing_length, wing_width, wing_width * 0.3)
        bpy.ops.object.transform_apply(scale=True)
        wings.append(right_wing)
    
    return wings


def generate_weapon_hardpoints(count=2, scale=1.0, symmetry=True, naming_prefix=''):
    """
    Generate weapon hardpoint markers
    
    Args:
        count: Number of weapon hardpoints
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        naming_prefix: Project naming prefix
    """
    hardpoints = []
    hardpoint_size = scale * 0.1
    
    positions = []
    if symmetry and count % 2 == 0:
        # Symmetric placement
        for i in range(count // 2):
            y_pos = scale * 0.3 + (i * scale * 0.2)
            x_offset = scale * 0.3 + (i * scale * 0.1)
            positions.append((x_offset, y_pos, -scale * 0.1))
            positions.append((-x_offset, y_pos, -scale * 0.1))
    else:
        # Non-symmetric
        for i in range(count):
            y_pos = scale * 0.3 + (i * scale * 0.2)
            x_pos = (i - count / 2) * scale * 0.2
            positions.append((x_pos, y_pos, -scale * 0.1))
    
    for i, pos in enumerate(positions):
        bpy.ops.mesh.primitive_cylinder_add(
            radius=hardpoint_size,
            depth=hardpoint_size * 2,
            location=pos
        )
        hardpoint = bpy.context.active_object
        hardpoint.name = _prefixed_name(naming_prefix, f"Weapon_Hardpoint_{i+1}")
        hardpoint.rotation_euler = (math.radians(90), 0, 0)
        hardpoints.append(hardpoint)
    
    return hardpoints


def generate_turret_hardpoints(count=2, scale=1.0, symmetry=True, naming_prefix=''):
    """
    Generate turret hardpoint fittings with visual turret geometry.

    Each turret consists of a cylindrical base, a rotation ring (torus),
    and a barrel.  Custom properties are added to each turret for engine
    mapping: ``turret_index``, ``turret_type``, ``tracking_speed`` and
    ``rotation_limits``.

    Ships may have up to 10 turret hardpoints.

    Args:
        count: Number of turret hardpoints (max 10)
        scale: Ship scale factor
        symmetry: Use symmetrical placement
        naming_prefix: Project naming prefix

    Returns:
        List of turret hardpoint root objects
    """
    count = min(count, MAX_TURRET_HARDPOINTS)
    turrets = []
    turret_size = scale * 0.12

    # Calculate positions along the dorsal (top) surface of the hull
    positions = []
    if symmetry and count % 2 == 0:
        for i in range(count // 2):
            y_pos = scale * 0.2 - (i * scale * 0.25)
            x_offset = scale * 0.2 + (i * scale * 0.08)
            positions.append((x_offset, y_pos, scale * 0.15))
            positions.append((-x_offset, y_pos, scale * 0.15))
    else:
        for i in range(count):
            y_pos = scale * 0.3 - (i * scale * 0.15)
            x_pos = (i - count / 2) * scale * 0.15
            positions.append((x_pos, y_pos, scale * 0.15))

    for i, pos in enumerate(positions):
        turret_name = _prefixed_name(naming_prefix, f"Turret_Hardpoint_{i+1}")

        # --- Turret base (flat cylinder) ---
        bpy.ops.mesh.primitive_cylinder_add(
            radius=turret_size,
            depth=turret_size * 0.4,
            location=pos
        )
        base = bpy.context.active_object
        base.name = turret_name

        # --- Rotation ring (torus around the base) ---
        ring_pos = (pos[0], pos[1], pos[2] + turret_size * 0.25)
        bpy.ops.mesh.primitive_torus_add(
            major_radius=turret_size * 0.8,
            minor_radius=turret_size * 0.08,
            location=ring_pos
        )
        ring = bpy.context.active_object
        ring.name = _prefixed_name(naming_prefix, f"Turret_Ring_{i+1}")
        ring.parent = base

        # --- Barrel ---
        barrel_pos = (pos[0], pos[1] + turret_size * 0.9, pos[2] + turret_size * 0.25)
        bpy.ops.mesh.primitive_cylinder_add(
            radius=turret_size * 0.1,
            depth=turret_size * 1.6,
            location=barrel_pos
        )
        barrel = bpy.context.active_object
        barrel.name = _prefixed_name(naming_prefix, f"Turret_Barrel_{i+1}")
        barrel.rotation_euler = (math.radians(90), 0, 0)
        barrel.parent = base

        # --- Engine mapping custom properties ---
        base["turret_index"] = i + 1
        base["turret_type"] = "projectile"
        base["tracking_speed"] = 30.0
        base["rotation_limits"] = "yaw:360,pitch:90"
        base["hardpoint_size"] = turret_size

        # Apply turret material
        mat = bpy.data.materials.new(
            name=_prefixed_name(naming_prefix, f"Turret_Mat_{i+1}"))
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        bsdf = nodes.get('Principled BSDF')
        if bsdf:
            bsdf.inputs['Base Color'].default_value = (0.35, 0.35, 0.4, 1.0)
            bsdf.inputs['Metallic'].default_value = 0.9
            bsdf.inputs['Roughness'].default_value = 0.3
        base.data.materials.append(mat)
        ring.data.materials.append(mat)
        barrel.data.materials.append(mat)

        turrets.append(base)

    return turrets


def generate_cockpit_neck(hull, cockpit, scale, naming_prefix=''):
    """Create a tapered fairing connecting the hull front face to the cockpit.

    Places a stretched cube between the hull's forward extent and the
    cockpit position so the two meshes appear joined rather than floating.

    Args:
        hull: The hull mesh object.
        cockpit: The cockpit mesh object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        The neck fairing object.
    """
    hull_front_y = hull.dimensions.y * 0.5
    cockpit_y = cockpit.location.y
    mid_y = (hull_front_y + cockpit_y) * 0.5
    length = abs(cockpit_y - hull_front_y) + scale * 0.05
    width = cockpit.dimensions.x * 0.9
    height = cockpit.dimensions.z * 0.85

    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, mid_y, 0))
    neck = bpy.context.active_object
    neck.name = _prefixed_name(naming_prefix, "Cockpit_Neck")
    neck.scale = (width, length, height)
    bpy.ops.object.transform_apply(scale=True)
    bpy.ops.object.shade_smooth()
    return neck


def generate_engine_pylons(engines, hull, scale, naming_prefix=''):
    """Create tapered pylons connecting each engine to the hull rear.

    A cylinder is placed between the hull's aft extent and each engine,
    producing a visible structural link.

    Args:
        engines: List of engine objects.
        hull: The hull mesh object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of pylon objects.
    """
    pylons = []
    hull_rear_y = -hull.dimensions.y * 0.5

    for i, engine in enumerate(engines):
        ex, ey, ez = engine.location
        mid_x = ex * 0.5
        mid_y = (hull_rear_y + ey) * 0.5
        length = abs(ey - hull_rear_y) + scale * 0.02
        radius = scale * 0.03

        bpy.ops.mesh.primitive_cylinder_add(
            radius=radius,
            depth=length,
            location=(mid_x, mid_y, ez),
        )
        pylon = bpy.context.active_object
        pylon.name = _prefixed_name(naming_prefix, f"Engine_Pylon_{i+1}")
        # Angle the pylon toward the engine offset
        pylon_angle = math.atan2(ex - mid_x, abs(ey - hull_rear_y)) if abs(ey - hull_rear_y) > 0 else 0
        pylon.rotation_euler = (math.radians(90), 0, pylon_angle)

        pylons.append(pylon)

    return pylons


def generate_wing_roots(wings, hull, scale, naming_prefix=''):
    """Create fairing blocks connecting each wing to the hull side.

    A scaled cube is placed at the junction of the hull and each wing
    to visually bridge the gap.

    Args:
        wings: List of wing objects.
        hull: The hull mesh object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of wing root fairing objects.
    """
    fairings = []
    hull_half_w = hull.dimensions.x * 0.5

    for i, wing in enumerate(wings):
        wx = wing.location.x
        side_x = hull_half_w if wx > 0 else -hull_half_w
        mid_x = (side_x + wx) * 0.5
        width = abs(wx - side_x) + scale * 0.05
        depth = wing.dimensions.y * 0.8
        height = wing.dimensions.z * 1.2

        bpy.ops.mesh.primitive_cube_add(size=1, location=(mid_x, 0, 0))
        fairing = bpy.context.active_object
        fairing.name = _prefixed_name(naming_prefix, f"Wing_Root_{i+1}")
        fairing.scale = (width, depth, height)
        bpy.ops.object.transform_apply(scale=True)
        bpy.ops.object.shade_smooth()
        fairings.append(fairing)

    return fairings


def generate_weapon_pylons(weapons, hull, scale, naming_prefix=''):
    """Create thin struts connecting weapon hardpoints to the hull.

    A small cylinder is placed between each weapon and the hull
    underside to make weapons look structurally attached.

    Args:
        weapons: List of weapon hardpoint objects.
        hull: The hull mesh object.
        scale: Ship scale factor.
        naming_prefix: Project naming prefix.

    Returns:
        List of weapon pylon objects.
    """
    pylons = []
    hull_bottom_z = -hull.dimensions.z * 0.5

    for i, weapon in enumerate(weapons):
        wx, wy, wz = weapon.location
        mid_z = (hull_bottom_z + wz) * 0.5
        length = abs(wz - hull_bottom_z) + scale * 0.01
        radius = scale * 0.015

        bpy.ops.mesh.primitive_cylinder_add(
            radius=radius,
            depth=length,
            location=(wx, wy, mid_z),
        )
        pylon = bpy.context.active_object
        pylon.name = _prefixed_name(naming_prefix, f"Weapon_Pylon_{i+1}")
        pylons.append(pylon)

    return pylons


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
