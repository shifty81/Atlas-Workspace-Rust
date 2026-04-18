"""
Main ship generator module for the NovaForge content pipeline.
Coordinates the generation of ship parts and assembly.

Generation pipeline (spine-first ordered assembly):
  1. Core spine (reactor → capacitor → bridge)
  2. Major structures (wings, structural plates)
  3. Engines (archetype-varied)
  4. Weapons & turrets
  5. Detail modules
  6. Hull taper / deform pass
  7. Bevel + auto-smooth cleanup pass
"""

import bpy
import math
import random
from . import ship_parts
from . import interior_generator
from . import module_system
from . import brick_system
from . import novaforge_importer


# Maximum number of turret hardpoints any ship may have
MAX_TURRET_HARDPOINTS = 10


# Ship class configurations (NovaForge classes only)
SHIP_CONFIGS = {
    'SHUTTLE': {
        'scale': 1.0,
        'hull_segments': 3,
        'engines': 2,
        'weapons': 0,
        'turret_hardpoints': 0,
        'wings': False,
        'crew_capacity': 2,
    },
    'FIGHTER': {
        'scale': 1.5,
        'hull_segments': 4,
        'engines': 2,
        'weapons': 2,
        'turret_hardpoints': 1,
        'wings': True,
        'crew_capacity': 1,
    },
    'CORVETTE': {
        'scale': 3.0,
        'hull_segments': 5,
        'engines': 3,
        'weapons': 4,
        'turret_hardpoints': 2,
        'wings': True,
        'crew_capacity': 4,
    },
    'FRIGATE': {
        'scale': 5.0,
        'hull_segments': 6,
        'engines': 4,
        'weapons': 6,
        'turret_hardpoints': 3,
        'wings': False,
        'crew_capacity': 10,
    },
    'DESTROYER': {
        'scale': 8.0,
        'hull_segments': 7,
        'engines': 4,
        'weapons': 8,
        'turret_hardpoints': 4,
        'wings': False,
        'crew_capacity': 25,
    },
    'CRUISER': {
        'scale': 12.0,
        'hull_segments': 8,
        'engines': 6,
        'weapons': 12,
        'turret_hardpoints': 6,
        'wings': False,
        'crew_capacity': 50,
    },
    'BATTLECRUISER': {
        'scale': 15.0,
        'hull_segments': 9,
        'engines': 6,
        'weapons': 14,
        'turret_hardpoints': 7,
        'wings': False,
        'crew_capacity': 75,
    },
    'BATTLESHIP': {
        'scale': 18.0,
        'hull_segments': 10,
        'engines': 8,
        'weapons': 16,
        'turret_hardpoints': 8,
        'wings': False,
        'crew_capacity': 100,
    },
    'CARRIER': {
        'scale': 25.0,
        'hull_segments': 12,
        'engines': 10,
        'weapons': 10,
        'turret_hardpoints': 6,
        'wings': False,
        'crew_capacity': 200,
    },
    'DREADNOUGHT': {
        'scale': 30.0,
        'hull_segments': 14,
        'engines': 5,
        'weapons': 18,
        'turret_hardpoints': 10,
        'wings': False,
        'crew_capacity': 400,
    },
    'TITAN': {
        'scale': 50.0,
        'hull_segments': 18,
        'engines': 10,
        'weapons': 24,
        'turret_hardpoints': 10,
        'wings': False,
        'crew_capacity': 1000,
    },
    'INDUSTRIAL': {
        'scale': 6.0,
        'hull_segments': 5,
        'engines': 3,
        'weapons': 1,
        'turret_hardpoints': 0,
        'wings': False,
        'crew_capacity': 5,
    },
    'MINING_BARGE': {
        'scale': 4.0,
        'hull_segments': 4,
        'engines': 2,
        'weapons': 0,
        'turret_hardpoints': 0,
        'wings': False,
        'crew_capacity': 3,
    },
    'EXHUMER': {
        'scale': 5.0,
        'hull_segments': 5,
        'engines': 3,
        'weapons': 0,
        'turret_hardpoints': 0,
        'wings': False,
        'crew_capacity': 4,
    },
    'CAPITAL': {
        'scale': 35.0,
        'hull_segments': 15,
        'engines': 12,
        'weapons': 20,
        'turret_hardpoints': 10,
        'wings': False,
        'crew_capacity': 500,
    },
    'EXPLORER': {
        'scale': 2.0,
        'hull_segments': 5,
        'engines': 2,
        'weapons': 1,
        'turret_hardpoints': 1,
        'wings': True,
        'crew_capacity': 1,
    },
    'HAULER': {
        'scale': 5.5,
        'hull_segments': 6,
        'engines': 4,
        'weapons': 0,
        'turret_hardpoints': 0,
        'wings': False,
        'crew_capacity': 2,
    },
    'EXOTIC': {
        'scale': 2.5,
        'hull_segments': 7,
        'engines': 2,
        'weapons': 2,
        'turret_hardpoints': 1,
        'wings': True,
        'crew_capacity': 1,
    },
}


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def generate_spaceship(ship_class='FRIGATE', seed=1, generate_interior=True,
                       module_slots=2, hull_complexity=1.0, symmetry=True,
                       style='SOLARI', naming_prefix='', turret_hardpoints=0,
                       hull_taper=0.85, launcher_hardpoints=0, drone_bays=0,
                       engine_count_override=0, novaforge_scale=None):
    """
    Generate a complete spaceship with all parts using spine-first assembly.

    Generation order:
      1. Core hull (spine)
      2. Cockpit / bridge
      3. Major structures (wings)
      4. Engines (archetype-varied)
      5. Weapons & turrets
      6. Detail modules
      7. Interior (optional)
      8. Hull taper / deform pass
      9. Bevel + auto-smooth cleanup pass

    Args:
        ship_class: Type of ship to generate
        seed: Random seed for generation
        generate_interior: Whether to generate interior
        module_slots: Number of module slots to add
        hull_complexity: Complexity factor for hull geometry
        symmetry: Whether to use symmetrical design
        style: NovaForge faction style (SOLARI, VEYREN, AURELIAN, KELDARI)
        naming_prefix: Project naming prefix applied to all generated elements
        turret_hardpoints: Number of turret hardpoints to generate (0-10)
        hull_taper: Taper factor for hull silhouette shaping (0.5-1.0, 1.0=none)
    """
    random.seed(seed)

    # Get ship configuration
    config = SHIP_CONFIGS.get(ship_class, SHIP_CONFIGS['FRIGATE'])
    scale = config['scale']
    grid_size = brick_system.get_grid_size(ship_class)

    # Track placed bricks for Ship DNA export
    placed_bricks = []

    # Create main collection for the ship
    collection_name = _prefixed_name(naming_prefix, f"Spaceship_{ship_class}_{seed}")
    collection = bpy.data.collections.new(collection_name)
    bpy.context.scene.collection.children.link(collection)

    # ------------------------------------------------------------------
    # Stage 1 – Core hull (spine)
    # ------------------------------------------------------------------
    hull = ship_parts.generate_hull(
        segments=config['hull_segments'],
        scale=scale,
        complexity=hull_complexity,
        symmetry=symmetry,
        style=style,
        naming_prefix=naming_prefix,
        ship_class=ship_class,
        seed=seed,
    )
    collection.objects.link(hull)
    placed_bricks.append({'type': 'STRUCTURAL_SPINE', 'pos': [0, 0, 0]})

    # ------------------------------------------------------------------
    # Stage 2 – Cockpit / bridge
    # ------------------------------------------------------------------
    cockpit = ship_parts.generate_cockpit(
        scale=scale,
        position=(0, scale * 0.8, 0),
        ship_class=ship_class,
        style=style,
        naming_prefix=naming_prefix
    )
    collection.objects.link(cockpit)
    placed_bricks.append({'type': 'REACTOR_CORE', 'pos': [0, scale * 0.8, 0]})

    # ------------------------------------------------------------------
    # Stage 3 – Major structures (wings)
    # ------------------------------------------------------------------
    if config['wings']:
        wings = ship_parts.generate_wings(
            scale=scale,
            symmetry=symmetry,
            style=style,
            naming_prefix=naming_prefix
        )
        for wing in wings:
            collection.objects.link(wing)
            placed_bricks.append({
                'type': 'HULL_PLATE',
                'pos': list(wing.location),
            })

        # Wing root fairings
        wing_roots = ship_parts.generate_wing_roots(
            wings, hull, scale, naming_prefix=naming_prefix)
        for fairing in wing_roots:
            collection.objects.link(fairing)

    # ------------------------------------------------------------------
    # Stage 4 – Engines (archetype-varied)
    # ------------------------------------------------------------------
    engine_count = engine_count_override if engine_count_override > 0 else config['engines']
    engines = ship_parts.generate_engines(
        count=engine_count,
        scale=scale,
        symmetry=symmetry,
        style=style,
        naming_prefix=naming_prefix
    )
    for i, engine in enumerate(engines):
        collection.objects.link(engine)
        archetype = brick_system.select_engine_archetype(i, len(engines))
        placed_bricks.append({
            'type': 'ENGINE_BLOCK',
            'pos': list(engine.location),
            'archetype': archetype,
        })

    # ------------------------------------------------------------------
    # Stage 4b – Connecting geometry (pylons & fairings)
    # ------------------------------------------------------------------
    # Cockpit neck
    neck = ship_parts.generate_cockpit_neck(
        hull, cockpit, scale, naming_prefix=naming_prefix)
    collection.objects.link(neck)

    # Engine pylons
    engine_pylons = ship_parts.generate_engine_pylons(
        engines, hull, scale, naming_prefix=naming_prefix)
    for pylon in engine_pylons:
        collection.objects.link(pylon)

    # ------------------------------------------------------------------
    # Stage 5 – Weapons & turrets
    # ------------------------------------------------------------------
    if config['weapons'] > 0:
        weapons = ship_parts.generate_weapon_hardpoints(
            count=config['weapons'],
            scale=scale,
            symmetry=symmetry,
            naming_prefix=naming_prefix
        )
        for weapon in weapons:
            collection.objects.link(weapon)
            placed_bricks.append({
                'type': 'HARDPOINT_MOUNT',
                'pos': list(weapon.location),
            })

        # Weapon pylons
        weapon_pylons = ship_parts.generate_weapon_pylons(
            weapons, hull, scale, naming_prefix=naming_prefix)
        for pylon in weapon_pylons:
            collection.objects.link(pylon)

    turret_count = turret_hardpoints if turret_hardpoints > 0 else config.get('turret_hardpoints', 0)
    turret_count = min(turret_count, MAX_TURRET_HARDPOINTS)
    if turret_count > 0:
        turrets = ship_parts.generate_turret_hardpoints(
            count=turret_count,
            scale=scale,
            symmetry=symmetry,
            naming_prefix=naming_prefix
        )
        for turret in turrets:
            collection.objects.link(turret)
            placed_bricks.append({
                'type': 'HARDPOINT_MOUNT',
                'pos': list(turret.location),
            })

    # Launcher hardpoints
    if launcher_hardpoints > 0:
        launchers = ship_parts.generate_launcher_hardpoints(
            count=launcher_hardpoints,
            scale=scale,
            symmetry=symmetry,
            naming_prefix=naming_prefix
        )
        for launcher in launchers:
            collection.objects.link(launcher)
            placed_bricks.append({
                'type': 'HARDPOINT_MOUNT',
                'pos': list(launcher.location),
                'subtype': 'launcher',
            })

    # Drone bays
    if drone_bays > 0:
        drone_bay_objs = ship_parts.generate_drone_bays(
            count=drone_bays,
            scale=scale,
            naming_prefix=naming_prefix
        )
        for bay in drone_bay_objs:
            collection.objects.link(bay)
            placed_bricks.append({
                'type': 'DOCKING_CLAMP',
                'pos': list(bay.location),
                'subtype': 'drone_bay',
            })

    # ------------------------------------------------------------------
    # Stage 6 – Detail modules
    # ------------------------------------------------------------------
    if module_slots > 0:
        modules = module_system.generate_modules(
            count=module_slots,
            scale=scale,
            ship_class=ship_class,
            naming_prefix=naming_prefix
        )
        for module in modules:
            collection.objects.link(module)
            placed_bricks.append({
                'type': 'CAPACITOR',
                'pos': list(module.location),
            })

    # ------------------------------------------------------------------
    # Stage 6b – Surface detail (greebles)
    # ------------------------------------------------------------------
    greeble_count = max(2, config['hull_segments'])
    greebles = generate_greeble_details(
        hull, scale=scale, count=greeble_count, seed=seed,
        naming_prefix=naming_prefix,
    )
    for greeble in greebles:
        collection.objects.link(greeble)
        placed_bricks.append({
            'type': greeble.get('brick_type', 'PANEL'),
            'pos': list(greeble.location),
        })

    # ------------------------------------------------------------------
    # Stage 6c – Faction-specific details
    # ------------------------------------------------------------------
    if style in ('SOLARI', 'VEYREN', 'AURELIAN', 'KELDARI'):
        faction_details = ship_parts.add_faction_details(
            hull, scale=scale, style=style, seed=seed,
            naming_prefix=naming_prefix,
        )
        for detail in faction_details:
            collection.objects.link(detail)

    # ------------------------------------------------------------------
    # Stage 7 – Interior (optional)
    # ------------------------------------------------------------------
    if generate_interior:
        interior_objects = interior_generator.generate_interior(
            ship_class=ship_class,
            scale=scale,
            crew_capacity=config['crew_capacity'],
            naming_prefix=naming_prefix
        )
        for obj in interior_objects:
            collection.objects.link(obj)

    # Parent all objects to the hull
    for obj in collection.objects:
        if obj != hull:
            obj.parent = hull

    # ------------------------------------------------------------------
    # Stage 8 – Hull taper / deform pass
    # ------------------------------------------------------------------
    taper_hull(hull, factor=hull_taper)

    # ------------------------------------------------------------------
    # Stage 9 – Bevel + auto-smooth cleanup pass
    # ------------------------------------------------------------------
    apply_cleanup_pass(hull)

    # Center the ship at the 3D cursor
    hull.location = bpy.context.scene.cursor.location

    # Store Ship DNA as a custom property on the hull
    dna = brick_system.generate_ship_dna(
        ship_class=ship_class,
        seed=seed,
        bricks=placed_bricks,
        style=style,
        naming_prefix=naming_prefix,
    )
    hull["ship_dna"] = brick_system.ship_dna_to_json(dna)

    return hull


# ------------------------------------------------------------------
# Post-processing helpers
# ------------------------------------------------------------------


# Greeble detail brick types and their associated shapes
_GREEBLE_TYPES = ['PANEL', 'VENT', 'PIPE']


def generate_greeble_details(hull, scale, count, seed, naming_prefix=''):
    """Place small surface detail objects along the hull.

    Distributes *count* greeble pieces (panels, vents, pipes) across the
    hull surface to break up large flat areas and add visual complexity.
    Uses the hull's bounding box to position details on the top and sides.

    Args:
        hull: The hull mesh object.
        scale: Ship scale factor.
        count: Number of greeble details to place.
        seed: Random seed for reproducible placement.
        naming_prefix: Project naming prefix.

    Returns:
        List of created greeble objects.
    """
    rng = random.Random(seed + 99)  # offset to decouple from hull/engine seeds
    greebles = []
    detail_scale = scale * 0.04

    half_w = scale * 0.4
    half_l = scale * 0.8

    for i in range(count):
        brick_type = rng.choice(_GREEBLE_TYPES)
        x_pos = rng.uniform(-half_w, half_w)
        y_pos = rng.uniform(-half_l * 0.6, half_l * 0.5)
        # Place on dorsal surface
        z_pos = scale * 0.15

        if brick_type == 'PANEL':
            bpy.ops.mesh.primitive_cube_add(
                size=detail_scale,
                location=(x_pos, y_pos, z_pos),
            )
        elif brick_type == 'VENT':
            bpy.ops.mesh.primitive_cube_add(
                size=detail_scale * 0.8,
                location=(x_pos, y_pos, z_pos),
            )
        else:  # PIPE
            bpy.ops.mesh.primitive_cylinder_add(
                radius=detail_scale * 0.15,
                depth=detail_scale * 2,
                location=(x_pos, y_pos, z_pos),
            )

        obj = bpy.context.active_object
        obj.name = _prefixed_name(naming_prefix, f"Greeble_{brick_type}_{i+1}")
        obj["brick_type"] = brick_type
        greebles.append(obj)

    return greebles


def taper_hull(obj, axis='Y', factor=0.85):
    """Apply a silhouette taper along *axis* to break the box look.

    Vertices further from centre along *axis* get their cross-section
    scaled by *factor*, giving the hull a tapered nose/tail.
    A factor of 1.0 means no taper (identity).
    """
    if factor >= 1.0:
        return

    mesh = obj.data
    if not mesh.vertices:
        return

    # Determine the extent along the taper axis
    axis_idx = {'X': 0, 'Y': 1, 'Z': 2}.get(axis, 1)
    coords = [v.co[axis_idx] for v in mesh.vertices]
    min_val = min(coords)
    max_val = max(coords)
    span = max_val - min_val
    if span == 0:
        return

    for v in mesh.vertices:
        t = abs(v.co[axis_idx] - (min_val + max_val) / 2) / (span / 2)
        scale = 1.0 - (1.0 - factor) * t
        if axis_idx != 0:
            v.co.x *= scale
        if axis_idx != 2:
            v.co.z *= scale

    mesh.update()


def apply_cleanup_pass(obj):
    """Add bevel and auto-smooth modifiers for a manufactured look."""
    # Add bevel modifier if not already present
    if not any(m.type == 'BEVEL' for m in obj.modifiers):
        bevel = obj.modifiers.new(name="Cleanup_Bevel", type='BEVEL')
        bevel.width = 0.03 * max(obj.dimensions)
        bevel.segments = 2
        bevel.limit_method = 'ANGLE'
        bevel.angle_limit = math.radians(30)

    # Enable auto-smooth via an edge-split modifier (compatible with
    # Blender 2.80+).  The existing EdgeSplit added during hull generation
    # already handles this, so we only act if one is missing.
    if not any(m.type == 'EDGE_SPLIT' for m in obj.modifiers):
        es = obj.modifiers.new(name="Cleanup_EdgeSplit", type='EDGE_SPLIT')
        es.split_angle = math.radians(30)


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
