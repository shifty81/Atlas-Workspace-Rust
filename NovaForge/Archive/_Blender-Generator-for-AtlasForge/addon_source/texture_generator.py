"""
Procedural texture generation module for the NovaForge content pipeline.
Creates PBR materials using Blender's shader node system.
Supports hull plating, glow effects, weathering, and NovaForge faction color palettes.
"""

import bpy
import random


# Color palettes for NovaForge factions
COLOR_PALETTES = {
    'SOLARI': {
        'primary': (0.8, 0.65, 0.2, 1.0),
        'secondary': (0.6, 0.45, 0.15, 1.0),
        'accent': (1.0, 0.9, 0.5, 1.0),
    },
    'VEYREN': {
        'primary': (0.4, 0.45, 0.5, 1.0),
        'secondary': (0.3, 0.35, 0.4, 1.0),
        'accent': (0.3, 0.5, 0.9, 1.0),
    },
    'AURELIAN': {
        'primary': (0.2, 0.45, 0.35, 1.0),
        'secondary': (0.15, 0.35, 0.25, 1.0),
        'accent': (0.0, 0.8, 0.7, 1.0),
    },
    'KELDARI': {
        'primary': (0.45, 0.35, 0.25, 1.0),
        'secondary': (0.35, 0.25, 0.15, 1.0),
        'accent': (0.9, 0.5, 0.1, 1.0),
    },
}


def get_palette(style, seed=0):
    """Return the color palette for the given NovaForge faction style."""
    return COLOR_PALETTES.get(style, COLOR_PALETTES['SOLARI'])


def generate_hull_material(style='SOLARI', seed=0, weathering=0.0):
    """
    Generate a procedural hull material using Blender's shader nodes.

    Args:
        style: Design style for color palette selection
        seed: Seed used to vary the NMS palette
        weathering: Amount of surface weathering (0.0 - 1.0)

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Hull_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    # Clear default nodes
    nodes.clear()

    # Create output node
    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (800, 0)

    # Create Principled BSDF
    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (400, 0)
    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    # Set base color from palette
    principled.inputs['Base Color'].default_value = palette['primary']
    principled.inputs['Metallic'].default_value = 0.7
    principled.inputs['Roughness'].default_value = 0.4

    # Add procedural noise for hull panel variation
    tex_coord = nodes.new(type='ShaderNodeTexCoord')
    tex_coord.location = (-600, 0)

    noise = nodes.new(type='ShaderNodeTexNoise')
    noise.location = (-400, 100)
    noise.inputs['Scale'].default_value = 15.0
    noise.inputs['Detail'].default_value = 8.0
    links.new(tex_coord.outputs['Object'], noise.inputs['Vector'])

    # Mix primary and secondary colors using noise
    color_mix = nodes.new(type='ShaderNodeMixRGB')
    color_mix.location = (0, 100)
    color_mix.blend_type = 'MIX'
    color_mix.inputs['Color1'].default_value = palette['primary']
    color_mix.inputs['Color2'].default_value = palette['secondary']
    links.new(noise.outputs['Fac'], color_mix.inputs['Fac'])
    links.new(color_mix.outputs['Color'], principled.inputs['Base Color'])

    # Add Voronoi for hull plating pattern
    voronoi = nodes.new(type='ShaderNodeTexVoronoi')
    voronoi.location = (-400, -150)
    voronoi.inputs['Scale'].default_value = 8.0
    links.new(tex_coord.outputs['Object'], voronoi.inputs['Vector'])

    # Use Voronoi distance to drive roughness variation
    roughness_mix = nodes.new(type='ShaderNodeMath')
    roughness_mix.location = (100, -150)
    roughness_mix.operation = 'MULTIPLY_ADD'
    roughness_mix.inputs[1].default_value = 0.3
    roughness_mix.inputs[2].default_value = 0.3
    links.new(voronoi.outputs['Distance'], roughness_mix.inputs[0])
    links.new(roughness_mix.outputs['Value'], principled.inputs['Roughness'])

    # Apply weathering if requested
    if weathering > 0.0:
        _apply_weathering(mat, weathering)

    return mat


def generate_accent_material(style='SOLARI', seed=0):
    """
    Generate an accent/trim material with emissive glow for details.

    Args:
        style: Design style for color palette selection
        seed: Seed used to vary the NMS palette

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Accent_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (600, 0)

    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (300, 0)
    principled.inputs['Base Color'].default_value = palette['accent']
    principled.inputs['Metallic'].default_value = 0.9
    principled.inputs['Roughness'].default_value = 0.2

    # Add subtle emission for accent glow
    # 'Emission Color' was renamed from 'Emission' in Blender 4.0
    emission_color_key = 'Emission Color' if 'Emission Color' in principled.inputs else 'Emission'
    principled.inputs[emission_color_key].default_value = palette['accent']
    principled.inputs['Emission Strength'].default_value = 0.5

    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    return mat


def generate_engine_material(style='SOLARI', seed=0):
    """
    Generate an engine glow material.

    Args:
        style: Design style for color palette selection
        seed: Seed used to vary the NMS palette

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Engine_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (400, 0)

    emission = nodes.new(type='ShaderNodeEmission')
    emission.location = (200, 0)
    emission.inputs['Color'].default_value = palette['accent']
    emission.inputs['Strength'].default_value = 5.0
    links.new(emission.outputs['Emission'], output.inputs['Surface'])

    return mat


def _apply_weathering(material, amount):
    """
    Add weathering/wear effects to an existing material.

    Uses a Musgrave-like noise texture mixed with darker colour to simulate
    dirt, scratches and wear on the hull surface.

    Args:
        material: The bpy.types.Material to modify
        amount: Intensity of weathering (0.0 - 1.0), clamped internally
    """
    amount = max(0.0, min(1.0, amount))
    nodes = material.node_tree.nodes
    links = material.node_tree.links

    # Find the principled BSDF
    principled = None
    for node in nodes:
        if node.type == 'BSDF_PRINCIPLED':
            principled = node
            break
    if principled is None:
        return

    # Weathering noise texture
    weather_noise = nodes.new(type='ShaderNodeTexNoise')
    weather_noise.location = (-200, -300)
    weather_noise.inputs['Scale'].default_value = 4.0
    weather_noise.inputs['Detail'].default_value = 12.0
    weather_noise.inputs['Roughness'].default_value = 0.8

    # Color ramp to sharpen weathering mask
    ramp = nodes.new(type='ShaderNodeValToRGB')
    ramp.location = (0, -300)
    ramp.color_ramp.elements[0].position = 0.4
    ramp.color_ramp.elements[1].position = 0.6
    links.new(weather_noise.outputs['Fac'], ramp.inputs['Fac'])

    # Darken roughness in weathered areas
    rough_add = nodes.new(type='ShaderNodeMath')
    rough_add.location = (200, -300)
    rough_add.operation = 'MULTIPLY_ADD'
    rough_add.inputs[1].default_value = amount * 0.4
    rough_add.inputs[2].default_value = 0.4
    links.new(ramp.outputs['Color'], rough_add.inputs[0])
    links.new(rough_add.outputs['Value'], principled.inputs['Roughness'])


def apply_textures_to_ship(ship_object, style='SOLARI', seed=0,
                           weathering=0.0):
    """
    Apply procedural textures to all parts of a ship hierarchy.

    Walks the object hierarchy starting from *ship_object* and assigns
    appropriate materials based on part names.  Each hull-category part
    receives a slight per-component roughness/color variation so
    individual components remain visually distinguishable.

    Args:
        ship_object: Root hull object of the ship
        style: Design style for palette selection
        seed: Random seed for NMS palette variation
        weathering: Weathering intensity (0.0 - 1.0)
    """
    hull_mat = generate_hull_material(style=style, seed=seed,
                                      weathering=weathering)
    accent_mat = generate_accent_material(style=style, seed=seed)
    engine_mat = generate_engine_material(style=style, seed=seed)

    rng = random.Random(seed + 7)  # offset to decouple from other seed uses

    def _varied_hull_mat(obj):
        """Create a per-component hull material with slight variation."""
        mat = hull_mat.copy()
        mat.name = f"Hull_{style}_{obj.name}"
        nodes = mat.node_tree.nodes
        for node in nodes:
            if node.type == 'BSDF_PRINCIPLED':
                base_rough = node.inputs['Roughness'].default_value
                node.inputs['Roughness'].default_value = max(
                    0.1, min(1.0, base_rough + rng.uniform(-0.08, 0.08)))
                base_metal = node.inputs['Metallic'].default_value
                node.inputs['Metallic'].default_value = max(
                    0.0, min(1.0, base_metal + rng.uniform(-0.05, 0.05)))
                break
        return mat

    def _assign(obj):
        if obj.type != 'MESH':
            return
        name_lower = obj.name.lower()
        if 'engine' in name_lower:
            if obj.data.materials:
                obj.data.materials[0] = engine_mat
            else:
                obj.data.materials.append(engine_mat)
        elif any(k in name_lower for k in ('weapon', 'hardpoint', 'cockpit',
                                            'wing', 'turret')):
            if obj.data.materials:
                obj.data.materials[0] = accent_mat
            else:
                obj.data.materials.append(accent_mat)
        else:
            varied = _varied_hull_mat(obj)
            if obj.data.materials:
                obj.data.materials[0] = varied
            else:
                obj.data.materials.append(varied)

    _assign(ship_object)
    for child in ship_object.children_recursive:
        _assign(child)


def generate_normal_material(style='SOLARI', seed=0):
    """Generate a material with procedural normal map detail.

    Creates hull panel lines and rivet patterns via bump nodes that
    produce convincing surface detail without modifying geometry.

    Args:
        style: Design style for color palette selection.
        seed: Random seed for palette variation.

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Hull_Normal_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (800, 0)

    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (400, 0)
    principled.inputs['Base Color'].default_value = palette['primary']
    principled.inputs['Metallic'].default_value = 0.7
    principled.inputs['Roughness'].default_value = 0.4
    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    # Texture coordinates
    tex_coord = nodes.new(type='ShaderNodeTexCoord')
    tex_coord.location = (-800, 0)

    # Panel line pattern (Voronoi)
    voronoi = nodes.new(type='ShaderNodeTexVoronoi')
    voronoi.location = (-400, 200)
    voronoi.inputs['Scale'].default_value = 12.0
    links.new(tex_coord.outputs['Object'], voronoi.inputs['Vector'])

    # Rivet pattern (noise, high frequency)
    noise = nodes.new(type='ShaderNodeTexNoise')
    noise.location = (-400, -100)
    noise.inputs['Scale'].default_value = 40.0
    noise.inputs['Detail'].default_value = 4.0
    links.new(tex_coord.outputs['Object'], noise.inputs['Vector'])

    # Combine via math for bump strength
    combine = nodes.new(type='ShaderNodeMath')
    combine.location = (-100, 100)
    combine.operation = 'ADD'
    links.new(voronoi.outputs['Distance'], combine.inputs[0])
    links.new(noise.outputs['Fac'], combine.inputs[1])

    # Bump node for normal perturbation
    bump = nodes.new(type='ShaderNodeBump')
    bump.location = (200, -150)
    bump.inputs['Strength'].default_value = 0.3
    bump.inputs['Distance'].default_value = 0.02
    links.new(combine.outputs['Value'], bump.inputs['Height'])
    links.new(bump.outputs['Normal'], principled.inputs['Normal'])

    return mat


def generate_glow_material(style='SOLARI', seed=0):
    """Generate a glow/emissive material for engine exhausts and running lights.

    Args:
        style: Design style.
        seed: Random seed.

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Glow_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (400, 0)

    emission = nodes.new(type='ShaderNodeEmission')
    emission.location = (200, 0)
    emission.inputs['Color'].default_value = palette['accent']
    emission.inputs['Strength'].default_value = 8.0
    links.new(emission.outputs['Emission'], output.inputs['Surface'])

    return mat


def generate_dirt_material(style='SOLARI', seed=0, intensity=0.5):
    """Generate a dirt/grime overlay material.

    Uses procedural noise to simulate accumulated dirt, oil stains, and
    environmental wear typical of space vessels.

    Args:
        style: Design style.
        seed: Random seed.
        intensity: Dirt intensity (0.0 = clean, 1.0 = heavy grime).

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)
    intensity = max(0.0, min(1.0, intensity))

    mat = bpy.data.materials.new(name=f"Dirt_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (800, 0)

    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (400, 0)
    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    # Dirt color (darker version of primary)
    dirt_color = tuple(c * 0.4 for c in palette['primary'][:3]) + (1.0,)
    clean_color = palette['primary']

    tex_coord = nodes.new(type='ShaderNodeTexCoord')
    tex_coord.location = (-600, 0)

    # Large-scale grime pattern
    noise1 = nodes.new(type='ShaderNodeTexNoise')
    noise1.location = (-300, 100)
    noise1.inputs['Scale'].default_value = 3.0
    noise1.inputs['Detail'].default_value = 10.0
    noise1.inputs['Roughness'].default_value = 0.7
    links.new(tex_coord.outputs['Object'], noise1.inputs['Vector'])

    # Small-scale streaks
    noise2 = nodes.new(type='ShaderNodeTexNoise')
    noise2.location = (-300, -100)
    noise2.inputs['Scale'].default_value = 8.0
    noise2.inputs['Detail'].default_value = 6.0
    links.new(tex_coord.outputs['Object'], noise2.inputs['Vector'])

    # Combine dirt patterns
    mix_noise = nodes.new(type='ShaderNodeMath')
    mix_noise.location = (-50, 0)
    mix_noise.operation = 'MULTIPLY'
    links.new(noise1.outputs['Fac'], mix_noise.inputs[0])
    links.new(noise2.outputs['Fac'], mix_noise.inputs[1])

    # Color ramp to control dirt threshold
    ramp = nodes.new(type='ShaderNodeValToRGB')
    ramp.location = (100, 0)
    ramp.color_ramp.elements[0].position = 1.0 - intensity
    links.new(mix_noise.outputs['Value'], ramp.inputs['Fac'])

    # Mix clean and dirty colors
    color_mix = nodes.new(type='ShaderNodeMixRGB')
    color_mix.location = (250, 100)
    color_mix.inputs['Color1'].default_value = clean_color
    color_mix.inputs['Color2'].default_value = dirt_color
    links.new(ramp.outputs['Color'], color_mix.inputs['Fac'])
    links.new(color_mix.outputs['Color'], principled.inputs['Base Color'])

    # Dirty areas are rougher
    rough_mix = nodes.new(type='ShaderNodeMath')
    rough_mix.location = (250, -100)
    rough_mix.operation = 'MULTIPLY_ADD'
    rough_mix.inputs[1].default_value = 0.4
    rough_mix.inputs[2].default_value = 0.4
    links.new(ramp.outputs['Color'], rough_mix.inputs[0])
    links.new(rough_mix.outputs['Value'], principled.inputs['Roughness'])

    principled.inputs['Metallic'].default_value = 0.5

    return mat


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
