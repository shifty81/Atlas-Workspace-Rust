"""
AtlasForge Generator — Blender Addon
Procedural content generation (PCG) tool for spaceships, stations, and asteroids.
Engine-agnostic asset generation pipeline designed for integration into multiple projects.
"""

bl_info = {
    "name": "AtlasForge Generator",
    "author": "AtlasForge",
    "version": (3, 0, 0),
    "blender": (2, 80, 0),
    "location": "View3D > Sidebar > AtlasForge",
    "description": "Engine-based PCG asset pipeline for procedural spaceships, stations, and asteroids",
    "category": "Add Mesh",
}

import bpy
from bpy.props import (
    EnumProperty,
    BoolProperty,
    IntProperty,
    FloatProperty,
    StringProperty,
)

from . import ship_generator
from . import ship_parts
from . import interior_generator
from . import module_system
from . import atlas_exporter
from . import station_generator
from . import asteroid_generator
from . import texture_generator
from . import brick_system
from . import novaforge_importer
from . import render_setup
from . import lod_generator
from . import collision_generator
from . import animation_system
from . import damage_system
from . import power_system
from . import build_validator


class SpaceshipGeneratorProperties(bpy.types.PropertyGroup):
    """Properties for the AtlasForge generator"""
    
    ship_class: EnumProperty(
        name="Ship Class",
        description="Ship class to generate",
        items=[
            ('SHUTTLE', "Shuttle", "Small transport ship"),
            ('FIGHTER', "Fighter", "Single-seat combat ship"),
            ('CORVETTE', "Corvette", "Small multi-crew ship"),
            ('FRIGATE', "Frigate", "Fast combat/utility ship"),
            ('DESTROYER', "Destroyer", "Heavy combat ship"),
            ('CRUISER', "Cruiser", "Large multi-role ship"),
            ('BATTLECRUISER', "Battlecruiser", "Heavy attack cruiser"),
            ('BATTLESHIP', "Battleship", "Heavy capital ship"),
            ('CARRIER', "Carrier", "Fleet carrier ship"),
            ('DREADNOUGHT', "Dreadnought", "Siege capital ship"),
            ('CAPITAL', "Capital", "Largest standard capital ship"),
            ('TITAN', "Titan", "Supercapital flagship"),
            ('INDUSTRIAL', "Industrial", "Cargo hauler"),
            ('MINING_BARGE', "Mining Barge", "Mining vessel"),
            ('EXHUMER', "Exhumer", "Advanced mining vessel"),
            ('EXPLORER', "Explorer", "Long-range exploration ship"),
            ('HAULER', "Hauler", "Dedicated freight transport"),
            ('EXOTIC', "Exotic", "Unique experimental ship"),
        ],
        default='FRIGATE'
    )
    
    generate_interior: BoolProperty(
        name="Generate Interior",
        description="Generate ship interior with rooms and corridors",
        default=True
    )
    
    module_slots: IntProperty(
        name="Module Slots",
        description="Number of additional module slots",
        default=2,
        min=0,
        max=10
    )
    
    seed: IntProperty(
        name="Random Seed",
        description="Seed for procedural generation",
        default=1,
        min=1
    )
    
    hull_complexity: FloatProperty(
        name="Hull Complexity",
        description="Complexity of hull geometry",
        default=1.0,
        min=0.1,
        max=3.0
    )
    
    symmetry: BoolProperty(
        name="Symmetry",
        description="Generate symmetric ship design",
        default=True
    )
    
    style: EnumProperty(
        name="Faction",
        description="NovaForge faction style",
        items=[
            ('SOLARI', "Solari", "Solari faction — golden, elegant, armor-focused"),
            ('VEYREN', "Veyren", "Veyren faction — angular, utilitarian, shield-focused"),
            ('AURELIAN', "Aurelian", "Aurelian faction — sleek, organic, drone-focused"),
            ('KELDARI', "Keldari", "Keldari faction — rugged, industrial, missile-focused"),
        ],
        default='SOLARI'
    )

    novaforge_json_path: StringProperty(
        name="Ship JSON",
        description="Path to a project ship JSON file to import",
        subtype='FILE_PATH',
        default=""
    )

    eveoffline_export_path: StringProperty(
        name="Export Path",
        description="Directory to export OBJ files for the asset pipeline",
        subtype='DIR_PATH',
        default=""
    )

    station_type: EnumProperty(
        name="Station Type",
        description="Type of station to generate",
        items=[
            ('INDUSTRIAL', "Industrial", "Manufacturing and production"),
            ('MILITARY', "Military", "Naval and defense installation"),
            ('COMMERCIAL', "Commercial", "Trade and commerce hub"),
            ('RESEARCH', "Research", "Scientific research facility"),
            ('MINING', "Mining", "Ore refinement and mining support"),
            ('ASTRAHUS', "Astrahus", "Medium Upwell citadel"),
            ('FORTIZAR', "Fortizar", "Large Upwell citadel"),
            ('KEEPSTAR', "Keepstar", "Extra-large Upwell citadel"),
        ],
        default='INDUSTRIAL'
    )

    station_faction: EnumProperty(
        name="Station Faction",
        description="Faction style for the station",
        items=[
            ('SOLARI', "Solari", "Golden cathedral style"),
            ('VEYREN', "Veyren", "Industrial block style"),
            ('AURELIAN', "Aurelian", "Organic dome style"),
            ('KELDARI', "Keldari", "Rusted patchwork style"),
        ],
        default='SOLARI'
    )

    belt_layout: EnumProperty(
        name="Belt Layout",
        description="Asteroid belt shape",
        items=[
            ('SEMICIRCLE', "Semicircle", "Standard semicircular belt"),
            ('SPHERE', "Sphere", "Spherical distribution"),
            ('CLUSTER', "Cluster", "Dense anomaly cluster"),
            ('RING', "Ring", "Sparse outer ring"),
        ],
        default='SEMICIRCLE'
    )

    belt_ore_type: EnumProperty(
        name="Primary Ore",
        description="Primary ore type for the belt",
        items=[
            ('DUSTITE', "Dustite", "Brown-orange, common ore"),
            ('FERRITE', "Ferrite", "Gray metallic ore"),
            ('IGNAITE', "Ignaite", "Red-brown volcanic ore"),
            ('CRYSTITE', "Crystite", "Green crystalline ore"),
            ('SHADITE', "Shadite", "Golden-brown ore"),
            ('CORITE', "Corite", "Blue-cyan icy ore"),
            ('LUMINE', "Lumine", "Dark red dense ore"),
            ('SANGITE', "Sangite", "Bright red metallic ore"),
            ('GLACITE', "Glacite", "Golden valuable ore"),
            ('DENSITE', "Densite", "Light gray banded ore"),
            ('VOIDITE', "Voidite", "Dark nullsec ore"),
            ('SPODUMAIN', "Spodumain", "Silvery reflective ore"),
            ('PYRANITE', "Pyranite", "Purple rare ore"),
            ('STELLITE', "Stellite", "Green luminescent ore"),
            ('COSMITE', "Cosmite", "Orange-gold most valuable ore"),
            ('NEXORITE', "Nexorite", "Cyan crystalline radioactive ore"),
        ],
        default='DUSTITE'
    )

    belt_count: IntProperty(
        name="Asteroid Count",
        description="Number of asteroids in the belt",
        default=30,
        min=5,
        max=200
    )

    generate_textures: BoolProperty(
        name="Generate Textures",
        description="Apply procedural PBR textures to the generated ship",
        default=True
    )

    weathering: FloatProperty(
        name="Weathering",
        description="Amount of surface weathering, dirt, and wear",
        default=0.0,
        min=0.0,
        max=1.0
    )

    naming_prefix: StringProperty(
        name="Naming Prefix",
        description="Project naming prefix applied to all generated elements (e.g. 'EVEOFFLINE')",
        default=""
    )

    turret_hardpoints: IntProperty(
        name="Turret Hardpoints",
        description="Number of turret hardpoints (visual turret fittings with base, ring and barrel)",
        default=0,
        min=0,
        max=10
    )

    hull_taper: FloatProperty(
        name="Hull Taper",
        description="Silhouette taper factor (lower = more tapered, 1.0 = no taper)",
        default=0.85,
        min=0.5,
        max=1.0
    )

    launcher_hardpoints: IntProperty(
        name="Launcher Hardpoints",
        description="Number of missile/torpedo launcher hardpoints",
        default=0,
        min=0,
        max=10
    )

    drone_bays: IntProperty(
        name="Drone Bays",
        description="Number of drone bays",
        default=0,
        min=0,
        max=5
    )

    novaforge_data_dir: StringProperty(
        name="Project Data Dir",
        description="Path to a project data/ships directory",
        subtype='DIR_PATH',
        default=""
    )

    ship_dna_export_path: StringProperty(
        name="Ship DNA Path",
        description="File path to export Ship DNA JSON for reproducible ships",
        subtype='FILE_PATH',
        default=""
    )

    generate_lods: BoolProperty(
        name="Generate LODs",
        description="Generate Level of Detail meshes for game engine export",
        default=False
    )

    generate_collision: BoolProperty(
        name="Generate Collision Mesh",
        description="Generate simplified collision mesh for physics",
        default=False
    )

    collision_type: EnumProperty(
        name="Collision Type",
        description="Type of collision mesh to generate",
        items=[
            ('AUTO', "Auto", "Automatically select based on ship class"),
            ('CONVEX_HULL', "Convex Hull", "Single convex hull wrapping entire ship"),
            ('BOX', "Bounding Box", "Oriented bounding box"),
            ('MULTI_CONVEX', "Multi Convex", "Decomposed into multiple convex parts"),
        ],
        default='AUTO'
    )

    generate_animations: BoolProperty(
        name="Generate Animations",
        description="Set up animation actions for turrets, bay doors, landing gear, and sensors",
        default=False
    )

    batch_output_path: StringProperty(
        name="Batch Output Path",
        description="Directory to export OBJ files when batch generating all ship types",
        subtype='DIR_PATH',
        default=""
    )


class SPACESHIP_OT_generate(bpy.types.Operator):
    """Generate a procedural spaceship"""
    bl_idname = "mesh.generate_spaceship"
    bl_label = "Generate Spaceship"
    bl_options = {'REGISTER', 'UNDO'}
    
    def execute(self, context):
        props = context.scene.spaceship_props
        
        # Generate the spaceship
        hull = ship_generator.generate_spaceship(
            ship_class=props.ship_class,
            seed=props.seed,
            generate_interior=props.generate_interior,
            module_slots=props.module_slots,
            hull_complexity=props.hull_complexity,
            symmetry=props.symmetry,
            style=props.style,
            naming_prefix=props.naming_prefix,
            turret_hardpoints=props.turret_hardpoints,
            hull_taper=props.hull_taper,
            launcher_hardpoints=props.launcher_hardpoints,
            drone_bays=props.drone_bays,
        )

        # Apply procedural textures if requested
        if props.generate_textures:
            texture_generator.apply_textures_to_ship(
                hull,
                style=props.style,
                seed=props.seed,
                weathering=props.weathering,
            )

        # Generate LOD meshes if requested
        if props.generate_lods:
            lod_generator.generate_lods(
                hull,
                ship_class=props.ship_class,
                naming_prefix=props.naming_prefix,
            )

        # Generate collision mesh if requested
        if props.generate_collision:
            col_type = None if props.collision_type == 'AUTO' else props.collision_type
            collision_generator.generate_collision_mesh(
                hull,
                collision_type=col_type,
                ship_class=props.ship_class,
                naming_prefix=props.naming_prefix,
            )

        # Set up animations if requested
        if props.generate_animations:
            config = ship_generator.SHIP_CONFIGS.get(
                props.ship_class, ship_generator.SHIP_CONFIGS['FRIGATE']
            )
            animation_system.setup_ship_animations(
                hull,
                scale=config['scale'],
                naming_prefix=props.naming_prefix,
            )

        self.report({'INFO'}, f"Generated {props.ship_class} class spaceship")
        return {'FINISHED'}


class SPACESHIP_OT_import_eveoffline(bpy.types.Operator):
    """Import ships from project JSON data and generate them"""
    bl_idname = "mesh.import_eveoffline_ships"
    bl_label = "Import from Project JSON"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        json_path = bpy.path.abspath(props.novaforge_json_path)

        if not json_path or not json_path.endswith('.json'):
            self.report({'ERROR'}, "Select a valid ship JSON file")
            return {'CANCELLED'}

        import os
        if not os.path.isfile(json_path):
            self.report({'ERROR'}, f"File not found: {json_path}")
            return {'CANCELLED'}

        try:
            ships = atlas_exporter.load_ship_data(json_path)
        except Exception as e:
            self.report({'ERROR'}, f"Failed to load JSON: {e}")
            return {'CANCELLED'}

        count = 0
        for ship_id, ship_data in ships.items():
            config = atlas_exporter.parse_ship_config(ship_data)
            ship_generator.generate_spaceship(
                ship_class=config['ship_class'],
                seed=config['seed'],
                generate_interior=config['generate_interior'],
                module_slots=config['module_slots'],
                hull_complexity=config['hull_complexity'],
                symmetry=config['symmetry'],
                style=config['style'],
            )
            count += 1

        self.report({'INFO'}, f"Generated {count} ships from project data")
        return {'FINISHED'}


class SPACESHIP_OT_export_obj(bpy.types.Operator):
    """Export the selected ship as OBJ for the AtlasForge engine"""
    bl_idname = "mesh.export_eveoffline_obj"
    bl_label = "Export OBJ for AtlasForge"
    bl_options = {'REGISTER'}

    def execute(self, context):
        props = context.scene.spaceship_props
        export_dir = bpy.path.abspath(props.eveoffline_export_path)

        if not export_dir:
            self.report({'ERROR'}, "Set an export directory first")
            return {'CANCELLED'}

        import os
        os.makedirs(export_dir, exist_ok=True)

        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "Select a ship object to export")
            return {'CANCELLED'}

        filename = obj.name.replace(' ', '_') + '.obj'
        filepath = os.path.join(export_dir, filename)

        # Select all children too
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        for child in obj.children_recursive:
            child.select_set(True)

        atlas_exporter.export_obj(filepath)

        self.report({'INFO'}, f"Exported to {filepath}")
        return {'FINISHED'}


class SPACESHIP_OT_generate_station(bpy.types.Operator):
    """Generate a procedural space station"""
    bl_idname = "mesh.generate_station"
    bl_label = "Generate Station"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        station_generator.generate_station(
            station_type=props.station_type,
            faction=props.station_faction,
            seed=props.seed,
        )
        self.report({'INFO'}, f"Generated {props.station_type} station ({props.station_faction})")
        return {'FINISHED'}


class SPACESHIP_OT_generate_asteroid_belt(bpy.types.Operator):
    """Generate a procedural asteroid belt"""
    bl_idname = "mesh.generate_asteroid_belt"
    bl_label = "Generate Asteroid Belt"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        asteroid_generator.generate_asteroid_belt(
            layout=props.belt_layout,
            ore_types=[props.belt_ore_type],
            count=props.belt_count,
            seed=props.seed,
        )
        self.report({'INFO'}, f"Generated {props.belt_layout} belt with {props.belt_count} asteroids")
        return {'FINISHED'}


class SPACESHIP_OT_export_ship_dna(bpy.types.Operator):
    """Export Ship DNA JSON for the selected ship"""
    bl_idname = "mesh.export_ship_dna"
    bl_label = "Export Ship DNA"
    bl_options = {'REGISTER'}

    def execute(self, context):
        import os

        props = context.scene.spaceship_props
        export_path = bpy.path.abspath(props.ship_dna_export_path)

        if not export_path or not export_path.endswith('.json'):
            self.report({'ERROR'}, "Set a valid .json export path first")
            return {'CANCELLED'}

        obj = context.active_object
        if obj is None or "ship_dna" not in obj:
            self.report({'ERROR'}, "Select a generated ship (hull) with Ship DNA")
            return {'CANCELLED'}

        os.makedirs(os.path.dirname(export_path) or '.', exist_ok=True)
        with open(export_path, 'w') as f:
            f.write(obj["ship_dna"])

        self.report({'INFO'}, f"Ship DNA exported to {export_path}")
        return {'FINISHED'}


class SPACESHIP_OT_import_novaforge(bpy.types.Operator):
    """Import a single project ship JSON and generate it"""
    bl_idname = "mesh.import_novaforge_ship"
    bl_label = "Generate from Project JSON"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        import os
        props = context.scene.spaceship_props
        json_path = bpy.path.abspath(props.novaforge_json_path)

        if not json_path or not json_path.endswith('.json'):
            self.report({'ERROR'}, "Select a valid ship JSON file")
            return {'CANCELLED'}
        if not os.path.isfile(json_path):
            self.report({'ERROR'}, f"File not found: {json_path}")
            return {'CANCELLED'}

        try:
            ships = novaforge_importer.load_ships_from_file(json_path)
        except Exception as e:
            self.report({'ERROR'}, f"Failed to load JSON: {e}")
            return {'CANCELLED'}

        count = 0
        for ship_id, ship_def in ships.items():
            params = novaforge_importer.ship_to_generator_params(ship_def)
            ship_generator.generate_spaceship(**params)
            count += 1

        self.report({'INFO'}, f"Generated {count} ships from project data")
        return {'FINISHED'}


class SPACESHIP_OT_batch_novaforge(bpy.types.Operator):
    """Batch generate all ships from a project data directory"""
    bl_idname = "mesh.batch_novaforge_ships"
    bl_label = "Batch Generate All Ships"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        import os
        props = context.scene.spaceship_props
        data_dir = bpy.path.abspath(props.novaforge_data_dir)

        if not data_dir or not os.path.isdir(data_dir):
            self.report({'ERROR'}, "Set a valid project data directory")
            return {'CANCELLED'}

        ships = novaforge_importer.load_ships_from_directory(data_dir)
        if not ships:
            self.report({'WARNING'}, "No ship definitions found")
            return {'CANCELLED'}

        count = 0
        for ship_id, ship_def in ships.items():
            params = novaforge_importer.ship_to_generator_params(ship_def)
            ship_generator.generate_spaceship(**params)
            count += 1

        self.report({'INFO'}, f"Batch generated {count} ships from project data")
        return {'FINISHED'}


class SPACESHIP_OT_catalog_render(bpy.types.Operator):
    """Set up and render a catalog image of the selected ship"""
    bl_idname = "mesh.catalog_render"
    bl_label = "Catalog Render"
    bl_options = {'REGISTER'}

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "Select a ship object first")
            return {'CANCELLED'}

        render_setup.setup_catalog_render(obj)
        self.report({'INFO'}, "Catalog render setup complete – press F12 to render")
        return {'FINISHED'}


class SPACESHIP_OT_batch_generate(bpy.types.Operator):
    """Generate all ship types and export each to the batch output folder"""
    bl_idname = "mesh.batch_generate_all"
    bl_label = "Batch Generate All Types"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        import os

        props = context.scene.spaceship_props
        output_dir = bpy.path.abspath(props.batch_output_path)

        if not output_dir:
            self.report({'ERROR'}, "Set a Batch Output Path first")
            return {'CANCELLED'}

        os.makedirs(output_dir, exist_ok=True)

        count = 0
        for ship_class in ship_generator.SHIP_CONFIGS:
            hull = ship_generator.generate_spaceship(
                ship_class=ship_class,
                seed=props.seed,
                generate_interior=props.generate_interior,
                module_slots=props.module_slots,
                hull_complexity=props.hull_complexity,
                symmetry=props.symmetry,
                style=props.style,
                naming_prefix=props.naming_prefix,
                hull_taper=props.hull_taper,
            )

            if props.generate_textures:
                texture_generator.apply_textures_to_ship(
                    hull,
                    style=props.style,
                    seed=props.seed,
                    weathering=props.weathering,
                )

            # Select the hull and all children for export
            bpy.ops.object.select_all(action='DESELECT')
            hull.select_set(True)
            for child in hull.children_recursive:
                child.select_set(True)

            filename = f"{ship_class}_seed{props.seed}.obj"
            filepath = os.path.join(output_dir, filename)
            atlas_exporter.export_obj(filepath)

            count += 1

        self.report({'INFO'},
                     f"Batch generated {count} ship types to {output_dir}")
        return {'FINISHED'}


class SPACESHIP_OT_novaforge_pipeline(bpy.types.Operator):
    """Read all project data/ships JSON files, generate every ship, and export OBJ models.

    Each ship is generated using its model_data (seed, turrets, engines,
    drones, launchers) and faction style, then exported as
    ``<ship_id>.obj`` into the output directory.  The resulting folder
    can be placed directly into the project's ``data/ships/obj_models/``
    directory for use by the AtlasForge engine.
    """
    bl_idname = "mesh.novaforge_pipeline_export"
    bl_label = "AtlasForge Pipeline Export"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        import os

        props = context.scene.spaceship_props
        data_dir = bpy.path.abspath(props.novaforge_data_dir)
        output_dir = bpy.path.abspath(props.batch_output_path)

        if not data_dir or not os.path.isdir(data_dir):
            self.report({'ERROR'},
                        "Set the Project Data Dir to your data/ships directory")
            return {'CANCELLED'}

        if not output_dir:
            self.report({'ERROR'}, "Set a Batch Output Path first")
            return {'CANCELLED'}

        os.makedirs(output_dir, exist_ok=True)

        ships = novaforge_importer.load_ships_from_directory(data_dir)
        if not ships:
            self.report({'WARNING'}, "No ship definitions found in data dir")
            return {'CANCELLED'}

        count = 0
        for ship_id, ship_def in ships.items():
            params = novaforge_importer.ship_to_generator_params(ship_def)

            hull = ship_generator.generate_spaceship(**params)

            if props.generate_textures:
                texture_generator.apply_textures_to_ship(
                    hull,
                    style=params.get('style', 'SOLARI'),
                    seed=params.get('seed', 1),
                    weathering=props.weathering,
                )

            # Select the hull and all children for export
            bpy.ops.object.select_all(action='DESELECT')
            hull.select_set(True)
            for child in hull.children_recursive:
                child.select_set(True)

            filename = f"{ship_id}.obj"
            filepath = os.path.join(output_dir, filename)
            atlas_exporter.export_obj(filepath)

            count += 1

        self.report(
            {'INFO'},
            f"AtlasForge pipeline: generated {count} ships to {output_dir}")
        return {'FINISHED'}


class SPACESHIP_PT_main_panel(bpy.types.Panel):
    """Main panel for AtlasForge generator"""
    bl_label = "AtlasForge Generator"
    bl_idname = "SPACESHIP_PT_main_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'AtlasForge'
    
    def draw(self, context):
        layout = self.layout
        props = context.scene.spaceship_props
        
        layout.label(text="Ship Configuration:")
        layout.prop(props, "ship_class")
        layout.prop(props, "style")
        layout.prop(props, "seed")
        
        layout.separator()
        layout.label(text="Generation Options:")
        layout.prop(props, "generate_interior")
        layout.prop(props, "module_slots")
        layout.prop(props, "hull_complexity")
        layout.prop(props, "symmetry")

        layout.separator()
        layout.label(text="Naming & Hardpoints:")
        layout.prop(props, "naming_prefix")
        layout.prop(props, "turret_hardpoints")
        layout.prop(props, "launcher_hardpoints")
        layout.prop(props, "drone_bays")

        layout.separator()
        layout.label(text="Hull Shaping:")
        layout.prop(props, "hull_taper")

        layout.separator()
        layout.label(text="Texture Options:")
        layout.prop(props, "generate_textures")
        if props.generate_textures:
            layout.prop(props, "weathering")
        
        layout.separator()
        layout.operator("mesh.generate_spaceship", icon='MESH_CUBE')

        layout.separator()
        layout.label(text="Batch Generation:")
        layout.prop(props, "batch_output_path")
        layout.operator("mesh.batch_generate_all", icon='FILE_REFRESH')

        layout.separator()
        layout.label(text="Project Integration:")
        layout.prop(props, "novaforge_data_dir")
        layout.operator("mesh.import_novaforge_ship", icon='IMPORT')
        layout.operator("mesh.batch_novaforge_ships", icon='FILE_REFRESH')
        layout.operator("mesh.novaforge_pipeline_export", icon='EXPORT')
        layout.operator("mesh.catalog_render", icon='RENDER_STILL')

        layout.separator()
        layout.label(text="AtlasForge Export:")
        layout.prop(props, "novaforge_json_path")
        layout.operator("mesh.import_eveoffline_ships", icon='IMPORT')
        layout.prop(props, "eveoffline_export_path")
        layout.operator("mesh.export_eveoffline_obj", icon='EXPORT')

        layout.separator()
        layout.label(text="Station Generation:")
        layout.prop(props, "station_type")
        layout.prop(props, "station_faction")
        layout.operator("mesh.generate_station", icon='WORLD')

        layout.separator()
        layout.label(text="Asteroid Belt Generation:")
        layout.prop(props, "belt_layout")
        layout.prop(props, "belt_ore_type")
        layout.prop(props, "belt_count")
        layout.operator("mesh.generate_asteroid_belt", icon='OUTLINER_OB_POINTCLOUD')

        layout.separator()
        layout.label(text="Ship DNA Export:")
        layout.prop(props, "ship_dna_export_path")
        layout.operator("mesh.export_ship_dna", icon='FILE_TEXT')

        layout.separator()
        layout.label(text="Game Engine Helpers:")
        layout.prop(props, "generate_lods")
        layout.prop(props, "generate_collision")
        if props.generate_collision:
            layout.prop(props, "collision_type")
        layout.prop(props, "generate_animations")


# Registration
classes = (
    SpaceshipGeneratorProperties,
    SPACESHIP_OT_generate,
    SPACESHIP_OT_import_eveoffline,
    SPACESHIP_OT_export_obj,
    SPACESHIP_OT_generate_station,
    SPACESHIP_OT_generate_asteroid_belt,
    SPACESHIP_OT_export_ship_dna,
    SPACESHIP_OT_import_novaforge,
    SPACESHIP_OT_batch_novaforge,
    SPACESHIP_OT_catalog_render,
    SPACESHIP_OT_batch_generate,
    SPACESHIP_OT_novaforge_pipeline,
    SPACESHIP_PT_main_panel,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    
    bpy.types.Scene.spaceship_props = bpy.props.PointerProperty(
        type=SpaceshipGeneratorProperties
    )
    
    # Register submodules
    ship_generator.register()
    ship_parts.register()
    interior_generator.register()
    module_system.register()
    atlas_exporter.register()
    station_generator.register()
    asteroid_generator.register()
    texture_generator.register()
    brick_system.register()
    novaforge_importer.register()
    render_setup.register()
    lod_generator.register()
    collision_generator.register()
    animation_system.register()
    damage_system.register()
    power_system.register()
    build_validator.register()


def unregister():
    # Unregister submodules
    build_validator.unregister()
    power_system.unregister()
    damage_system.unregister()
    animation_system.unregister()
    collision_generator.unregister()
    lod_generator.unregister()
    render_setup.unregister()
    novaforge_importer.unregister()
    brick_system.unregister()
    texture_generator.unregister()
    asteroid_generator.unregister()
    station_generator.unregister()
    atlas_exporter.unregister()
    module_system.unregister()
    interior_generator.unregister()
    ship_parts.unregister()
    ship_generator.unregister()
    
    del bpy.types.Scene.spaceship_props
    
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
