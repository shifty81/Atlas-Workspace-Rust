# Atlas Blender Spaceship Generator
#
# Procedural spaceship generator addon for Blender 3.6+.
# Designed for use with the Atlas Engine asset pipeline.
#
# Install: Edit → Preferences → Add-ons → Install → select this file
# Usage:   Sidebar (N) → Atlas tab → Generate Ship

bl_info = {
    "name": "Atlas: Spaceship Generator",
    "author": "Atlas Engine Team",
    "version": (1, 0, 0),
    "blender": (3, 6, 0),
    "location": "View3D > Sidebar > Atlas",
    "description": "Procedural spaceship generator for the Atlas Engine",
    "category": "Object",
}

import bpy
import bmesh
import json
import math
import os
import random
from mathutils import Vector, Matrix


# ── Deterministic RNG (mirrors engine/procedural/DeterministicRNG) ──────

class DeterministicRNG:
    """xorshift64* RNG matching the engine's DeterministicRNG."""

    MASK = (1 << 64) - 1

    def __init__(self, seed):
        self.state = seed if seed != 0 else 1

    def _next(self):
        s = self.state
        s ^= (s >> 12) & self.MASK
        s ^= (s << 25) & self.MASK
        s ^= (s >> 27) & self.MASK
        self.state = s & self.MASK
        return ((s * 0x2545F4914F6CDD1D) & self.MASK) >> 32

    def next_float(self):
        return (self._next() & 0xFFFFFF) / float(0x1000000)

    def next_float_range(self, lo, hi):
        return lo + self.next_float() * (hi - lo)

    def next_int_range(self, lo, hi):
        if lo >= hi:
            return lo
        return lo + (self._next() % (hi - lo))

    def fork(self, child_seed):
        return DeterministicRNG(self.state ^ child_seed)


# ── Hull Profile Generation ─────────────────────────────────────────────

HULL_CLASSES = {
    "fighter":     {"length": (3, 6),    "beam": (1, 2),    "height": (0.5, 1.5), "segments": 8},
    "frigate":     {"length": (6, 12),   "beam": (2, 4),    "height": (1.5, 3),   "segments": 12},
    "cruiser":     {"length": (12, 25),  "beam": (4, 8),    "height": (3, 6),     "segments": 16},
    "battleship":  {"length": (25, 50),  "beam": (8, 14),   "height": (6, 10),    "segments": 20},
    "capital":     {"length": (50, 120), "beam": (14, 30),  "height": (10, 25),   "segments": 24},
}

SLOT_TYPES = ["weapon", "engine", "shield", "sensor", "bay", "utility"]

MODULE_SIZES = ["XS", "S", "M", "L", "XL"]


def generate_hull_profile(rng, hull_class):
    """Generate a parametric hull profile from the given hull class."""
    params = HULL_CLASSES[hull_class]
    length = rng.next_float_range(*params["length"])
    beam = rng.next_float_range(*params["beam"])
    height = rng.next_float_range(*params["height"])
    segments = params["segments"]
    return length, beam, height, segments


def create_hull_mesh(length, beam, height, segments):
    """Create a basic hull mesh from parametric dimensions."""
    mesh = bpy.data.meshes.new("ShipHull")
    bm = bmesh.new()

    # Generate profile cross-sections along the length axis
    num_sections = max(4, segments // 2)
    for i in range(num_sections + 1):
        t = i / num_sections
        # Smooth taper: elliptical cross-section that narrows at bow and stern
        x = -length / 2 + t * length
        taper = math.sin(t * math.pi)
        w = beam / 2 * taper
        h = height / 2 * taper

        if w < 0.01 or h < 0.01:
            # Degenerate section → single vertex at centerline
            bm.verts.new((x, 0, 0))
            continue

        # Elliptical cross-section
        for j in range(segments):
            angle = 2 * math.pi * j / segments
            y = w * math.cos(angle)
            z = h * math.sin(angle)
            bm.verts.new((x, y, z))

    bm.verts.ensure_lookup_table()

    # Connect adjacent cross-sections with faces
    for i in range(num_sections):
        for j in range(segments):
            # Indices into the vertex list
            i0 = i * segments + j
            i1 = i * segments + (j + 1) % segments
            i2 = (i + 1) * segments + j
            i3 = (i + 1) * segments + (j + 1) % segments

            if i0 < len(bm.verts) and i1 < len(bm.verts) and \
               i2 < len(bm.verts) and i3 < len(bm.verts):
                try:
                    bm.faces.new([bm.verts[i0], bm.verts[i1],
                                  bm.verts[i3], bm.verts[i2]])
                except ValueError:
                    pass  # Skip duplicate faces

    bm.to_mesh(mesh)
    bm.free()
    mesh.update()
    return mesh


def generate_slots(rng, hull_class, length, beam, height):
    """Generate module slots based on hull class."""
    slot_counts = {
        "fighter": 3,
        "frigate": 5,
        "cruiser": 8,
        "battleship": 12,
        "capital": 18,
    }
    count = slot_counts.get(hull_class, 5)
    slots = []

    for i in range(count):
        slot_type = SLOT_TYPES[rng.next_int_range(0, len(SLOT_TYPES))]
        size_idx = rng.next_int_range(0, len(MODULE_SIZES))
        x = rng.next_float_range(-length * 0.4, length * 0.4)
        y = rng.next_float_range(-beam * 0.3, beam * 0.3)
        z = rng.next_float_range(-height * 0.3, height * 0.3)

        slots.append({
            "name": f"slot_{slot_type}_{i}",
            "type": slot_type,
            "position": [round(x, 3), round(y, 3), round(z, 3)],
            "size": MODULE_SIZES[size_idx],
        })

    return slots


def export_atlas_meta(filepath, seed, hull_class, tier, slots, bbox_min, bbox_max):
    """Write the .atlas_meta JSON sidecar."""
    meta = {
        "version": 1,
        "seed": seed,
        "hull_class": hull_class,
        "tier": tier,
        "slots": slots,
        "materials": {
            "hull": {
                "metallic": 0.8,
                "roughness": 0.35,
            }
        },
        "bounding_box": {
            "min": [round(v, 3) for v in bbox_min],
            "max": [round(v, 3) for v in bbox_max],
        },
    }
    with open(filepath, "w") as f:
        json.dump(meta, f, indent=2)


# ── Blender Operator ────────────────────────────────────────────────────

class ATLAS_OT_generate_ship(bpy.types.Operator):
    bl_idname = "atlas.generate_ship"
    bl_label = "Generate Ship"
    bl_description = "Generate a procedural spaceship"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        props = context.scene.atlas_ship_props
        seed = props.seed if props.seed != 0 else random.randint(1, 2**32)
        hull_class = props.hull_class
        tier = props.tier

        rng = DeterministicRNG(seed)

        # Generate hull
        length, beam, height, segments = generate_hull_profile(rng, hull_class)
        mesh = create_hull_mesh(length, beam, height, segments)

        obj = bpy.data.objects.new(f"Ship_{hull_class}_{seed}", mesh)
        context.collection.objects.link(obj)
        context.view_layer.objects.active = obj
        obj.select_set(True)

        # Generate and place slots as empties
        slots = generate_slots(rng, hull_class, length, beam, height)
        for slot in slots:
            empty = bpy.data.objects.new(slot["name"], None)
            empty.empty_display_type = "PLAIN_AXES"
            empty.empty_display_size = 0.3
            empty.location = Vector(slot["position"])
            empty.parent = obj
            context.collection.objects.link(empty)

        # Store metadata on object for later export
        obj["atlas_seed"] = seed
        obj["atlas_hull_class"] = hull_class
        obj["atlas_tier"] = tier
        obj["atlas_slots"] = json.dumps(slots)

        self.report({"INFO"}, f"Generated {hull_class} (seed={seed})")
        return {"FINISHED"}


class ATLAS_OT_export_ship(bpy.types.Operator):
    bl_idname = "atlas.export_ship"
    bl_label = "Export for Atlas"
    bl_description = "Export the selected ship for the Atlas Engine"
    bl_options = {"REGISTER"}

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        obj = context.active_object
        if obj is None or "atlas_seed" not in obj:
            self.report({"ERROR"}, "Select a generated ship first")
            return {"CANCELLED"}

        base_path = os.path.splitext(self.filepath)[0]
        obj_path = base_path + ".obj"
        meta_path = base_path + ".atlas_meta"

        # Export OBJ
        try:
            bpy.ops.wm.obj_export(
                filepath=obj_path,
                export_selected_objects=True,
                export_materials=False,
            )
        except Exception as e:
            self.report({"ERROR"}, f"OBJ export failed: {e}")
            return {"CANCELLED"}

        # Export metadata
        seed = obj["atlas_seed"]
        hull_class = obj["atlas_hull_class"]
        tier = obj["atlas_tier"]
        slots = json.loads(obj["atlas_slots"])

        # Compute bounding box
        bbox = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
        bbox_min = [min(v[i] for v in bbox) for i in range(3)]
        bbox_max = [max(v[i] for v in bbox) for i in range(3)]

        export_atlas_meta(meta_path, seed, hull_class, tier, slots, bbox_min, bbox_max)

        self.report({"INFO"}, f"Exported to {obj_path} and {meta_path}")
        return {"FINISHED"}


# ── Properties ──────────────────────────────────────────────────────────

class AtlasShipProperties(bpy.types.PropertyGroup):
    seed: bpy.props.IntProperty(
        name="Seed",
        description="Generation seed (0 = random)",
        default=0,
        min=0,
    )
    hull_class: bpy.props.EnumProperty(
        name="Hull Class",
        items=[
            ("fighter", "Fighter", "Small fast ship"),
            ("frigate", "Frigate", "Medium combat ship"),
            ("cruiser", "Cruiser", "Large multi-role ship"),
            ("battleship", "Battleship", "Heavy warship"),
            ("capital", "Capital", "Massive capital ship"),
        ],
        default="cruiser",
    )
    tier: bpy.props.IntProperty(
        name="Tier",
        description="Visual detail tier (1-5)",
        default=1,
        min=1,
        max=5,
    )


# ── UI Panel ────────────────────────────────────────────────────────────

class ATLAS_PT_ship_generator(bpy.types.Panel):
    bl_label = "Atlas Ship Generator"
    bl_idname = "ATLAS_PT_ship_generator"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Atlas"

    def draw(self, context):
        layout = self.layout
        props = context.scene.atlas_ship_props

        layout.prop(props, "seed")
        layout.prop(props, "hull_class")
        layout.prop(props, "tier")
        layout.separator()
        layout.operator("atlas.generate_ship", icon="MESH_ICOSPHERE")
        layout.separator()
        layout.operator("atlas.export_ship", icon="EXPORT")


# ── Registration ────────────────────────────────────────────────────────

classes = (
    AtlasShipProperties,
    ATLAS_OT_generate_ship,
    ATLAS_OT_export_ship,
    ATLAS_PT_ship_generator,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.Scene.atlas_ship_props = bpy.props.PointerProperty(
        type=AtlasShipProperties
    )


def unregister():
    del bpy.types.Scene.atlas_ship_props
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
