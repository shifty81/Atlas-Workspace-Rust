"""
Syntax and structure validation tests for the Blender Spaceship Generator addon
These tests run without requiring Blender to be installed
"""

import ast
import os
import sys


def test_python_syntax(filepath):
    """Test if a Python file has valid syntax"""
    try:
        with open(filepath, 'r') as f:
            code = f.read()
        ast.parse(code)
        return True, None
    except SyntaxError as e:
        return False, str(e)


def test_addon_structure():
    """Test that all required files exist"""
    print("Testing addon structure...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    required_files = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
        'novaforge_importer.py',
        'render_setup.py',
        'lod_generator.py',
        'collision_generator.py',
        'animation_system.py',
        'damage_system.py',
        'power_system.py',
        'build_validator.py',
    ]
    
    all_exist = True
    for filename in required_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            print(f"✓ {filename} exists")
        else:
            print(f"✗ {filename} is missing")
            all_exist = False
    
    return all_exist


def test_file_syntax():
    """Test Python syntax of all module files"""
    print("\nTesting Python syntax...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    python_files = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
        'novaforge_importer.py',
        'render_setup.py',
        'lod_generator.py',
        'collision_generator.py',
        'animation_system.py',
        'damage_system.py',
        'power_system.py',
        'build_validator.py',
    ]
    
    all_valid = True
    for filename in python_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            valid, error = test_python_syntax(filepath)
            if valid:
                print(f"✓ {filename} has valid syntax")
            else:
                print(f"✗ {filename} has syntax error: {error}")
                all_valid = False
    
    return all_valid


def test_bl_info():
    """Test that __init__.py has valid bl_info"""
    print("\nTesting bl_info metadata...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    init_file = os.path.join(addon_path, '__init__.py')
    
    try:
        with open(init_file, 'r') as f:
            content = f.read()
        
        # Check for bl_info
        if 'bl_info' not in content:
            print("✗ bl_info not found in __init__.py")
            return False
        
        # Parse and check bl_info
        tree = ast.parse(content)
        bl_info_found = False
        
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and target.id == 'bl_info':
                        bl_info_found = True
                        
                        # Check if it's a dictionary
                        if isinstance(node.value, ast.Dict):
                            keys = [k.s if isinstance(k, ast.Str) else (k.value if isinstance(k, ast.Constant) else None) 
                                   for k in node.value.keys]
                            
                            required_keys = ['name', 'author', 'version', 'blender', 'category']
                            for key in required_keys:
                                if key in keys:
                                    print(f"✓ bl_info has '{key}' key")
                                else:
                                    print(f"✗ bl_info missing '{key}' key")
                                    return False
                        break
        
        if not bl_info_found:
            print("✗ bl_info assignment not found")
            return False
        
        return True
        
    except Exception as e:
        print(f"✗ Error checking bl_info: {e}")
        return False


def test_register_functions():
    """Test that register/unregister functions exist"""
    print("\nTesting register/unregister functions...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    files_to_check = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
        'novaforge_importer.py',
        'render_setup.py',
        'lod_generator.py',
        'collision_generator.py',
        'animation_system.py',
        'damage_system.py',
        'power_system.py',
        'build_validator.py',
    ]
    
    all_valid = True
    for filename in files_to_check:
        filepath = os.path.join(addon_path, filename)
        
        try:
            with open(filepath, 'r') as f:
                content = f.read()
            
            has_register = 'def register()' in content
            has_unregister = 'def unregister()' in content
            
            if has_register and has_unregister:
                print(f"✓ {filename} has register/unregister functions")
            else:
                missing = []
                if not has_register:
                    missing.append('register')
                if not has_unregister:
                    missing.append('unregister')
                print(f"✗ {filename} missing: {', '.join(missing)}")
                all_valid = False
                
        except Exception as e:
            print(f"✗ Error checking {filename}: {e}")
            all_valid = False
    
    return all_valid


def test_documentation():
    """Test that documentation files exist"""
    print("\nTesting documentation...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    doc_files = ['README.md', 'USAGE.md']
    
    all_exist = True
    for filename in doc_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            size = os.path.getsize(filepath)
            print(f"✓ {filename} exists ({size} bytes)")
        else:
            print(f"✗ {filename} is missing")
            all_exist = False
    
    return all_exist


def test_turret_hardpoint_configs():
    """Test that all ship configs include turret_hardpoints and respect max 10"""
    print("\nTesting turret hardpoint configurations...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    tree = ast.parse(content)
    ship_configs = None

    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'SHIP_CONFIGS':
                    ship_configs = node.value
                    break

    if ship_configs is None:
        print("✗ SHIP_CONFIGS not found in ship_generator.py")
        return False

    if not isinstance(ship_configs, ast.Dict):
        print("✗ SHIP_CONFIGS is not a dictionary")
        return False

    all_valid = True
    for key_node, value_node in zip(ship_configs.keys, ship_configs.values):
        class_name = key_node.value if isinstance(key_node, ast.Constant) else '?'
        if not isinstance(value_node, ast.Dict):
            continue
        inner_keys = []
        for k in value_node.keys:
            if isinstance(k, ast.Constant):
                inner_keys.append(k.value)
        if 'turret_hardpoints' not in inner_keys:
            print(f"✗ {class_name} missing 'turret_hardpoints' key")
            all_valid = False
        else:
            # Find the value
            idx = inner_keys.index('turret_hardpoints')
            val_node = value_node.values[idx]
            if isinstance(val_node, ast.Constant) and isinstance(val_node.value, int):
                if val_node.value > 10:
                    print(f"✗ {class_name} turret_hardpoints={val_node.value} exceeds max 10")
                    all_valid = False
                else:
                    print(f"✓ {class_name} turret_hardpoints={val_node.value}")
            else:
                print(f"✗ {class_name} turret_hardpoints is not an integer")
                all_valid = False

    return all_valid


def test_naming_prefix_support():
    """Test that key generation modules define _prefixed_name helper"""
    print("\nTesting naming prefix support...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    files_needing_prefix = [
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
    ]

    all_valid = True
    for filename in files_needing_prefix:
        filepath = os.path.join(addon_path, filename)
        with open(filepath, 'r') as f:
            content = f.read()
        if 'def _prefixed_name(' in content:
            print(f"✓ {filename} has _prefixed_name helper")
        else:
            print(f"✗ {filename} missing _prefixed_name helper")
            all_valid = False

    return all_valid


def test_turret_generation_function():
    """Test that ship_parts.py defines generate_turret_hardpoints"""
    print("\nTesting turret generation function...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_turret_hardpoints(': 'generate_turret_hardpoints function',
        '"turret_index"': 'turret_index custom property',
        '"turret_type"': 'turret_type custom property',
        '"tracking_speed"': 'tracking_speed custom property',
        '"rotation_limits"': 'rotation_limits custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_brick_system():
    """Test the brick taxonomy, scale bands, grid system and Ship DNA helpers"""
    print("\nTesting brick system...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    bs_path = os.path.join(addon_path, 'brick_system.py')

    # Import brick_system directly (no bpy dependency)
    import importlib.util
    spec = importlib.util.spec_from_file_location("brick_system", bs_path)
    bs = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bs)

    all_valid = True

    # Check BRICK_TYPES has entries for every category
    for category, brick_names in bs.BRICK_CATEGORIES.items():
        for name in brick_names:
            if name not in bs.BRICK_TYPES:
                print(f"✗ Brick type {name} (category {category}) missing from BRICK_TYPES")
                all_valid = False
            else:
                bt = bs.BRICK_TYPES[name]
                for key in ('category', 'size', 'shape', 'scale_band', 'hardpoints'):
                    if key not in bt:
                        print(f"✗ Brick {name} missing key '{key}'")
                        all_valid = False
    if all_valid:
        print(f"✓ All {sum(len(v) for v in bs.BRICK_CATEGORIES.values())} brick types valid")

    # Scale bands
    for band in ('primary', 'structural', 'detail'):
        val = bs.get_scale_factor(band)
        if not (0 < val <= 1.0):
            print(f"✗ Scale band '{band}' has invalid factor {val}")
            all_valid = False
    if bs.get_scale_factor('primary') > bs.get_scale_factor('structural') > bs.get_scale_factor('detail'):
        print("✓ Scale hierarchy correct (primary > structural > detail)")
    else:
        print("✗ Scale hierarchy incorrect")
        all_valid = False

    # Grid snapping
    snapped = bs.snap_to_grid((1.3, 2.7, 0.1), 1.0)
    if snapped == (1.0, 3.0, 0.0):
        print("✓ snap_to_grid works correctly")
    else:
        print(f"✗ snap_to_grid returned {snapped}, expected (1.0, 3.0, 0.0)")
        all_valid = False

    # Engine archetypes
    for arch in ('MAIN_THRUST', 'MANEUVERING', 'UTILITY_EXHAUST'):
        if bs.get_engine_archetype(arch) is None:
            print(f"✗ Engine archetype {arch} missing")
            all_valid = False
    print("✓ All engine archetypes defined")

    # Ship DNA round-trip
    dna = bs.generate_ship_dna('CRUISER', 42, [{'type': 'REACTOR_CORE', 'pos': [0, 0, 0]}])
    json_str = bs.ship_dna_to_json(dna)
    loaded = bs.ship_dna_from_json(json_str)
    if loaded['seed'] == 42 and loaded['class'] == 'CRUISER' and len(loaded['bricks']) == 1:
        print("✓ Ship DNA round-trip works")
    else:
        print("✗ Ship DNA round-trip failed")
        all_valid = False

    return all_valid


def test_hull_taper_and_cleanup():
    """Test that ship_generator defines taper_hull and apply_cleanup_pass"""
    print("\nTesting hull taper and cleanup pass...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    checks = {
        'def taper_hull(': 'taper_hull function',
        'def apply_cleanup_pass(': 'apply_cleanup_pass function',
        'hull_taper': 'hull_taper parameter',
        '"ship_dna"': 'Ship DNA custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_engine_archetypes():
    """Test that ship_parts uses engine archetypes"""
    print("\nTesting engine archetype integration...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'engine_archetype': 'engine archetype custom property',
        'select_engine_archetype': 'archetype selection',
        '_add_nozzle_flare': 'nozzle flare helper',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_hull_profiles():
    """Test that ship_parts defines hull profiles for all ship classes"""
    print("\nTesting hull shape profiles...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sp_path, 'r') as f:
        sp_content = f.read()
    with open(sg_path, 'r') as f:
        sg_content = f.read()

    all_valid = True

    # Check HULL_PROFILES dict exists
    if 'HULL_PROFILES' not in sp_content:
        print("✗ HULL_PROFILES not found in ship_parts.py")
        return False
    print("✓ HULL_PROFILES dictionary found")

    # Check that every ship class in SHIP_CONFIGS has a profile
    tree = ast.parse(sg_content)
    class_names = []
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'SHIP_CONFIGS':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                class_names.append(k.value)

    for cls in class_names:
        if f"'{cls}'" in sp_content:
            print(f"✓ HULL_PROFILES has entry for {cls}")
        else:
            print(f"✗ HULL_PROFILES missing entry for {cls}")
            all_valid = False

    return all_valid


def test_vertex_noise():
    """Test that ship_parts defines the vertex noise helper"""
    print("\nTesting hull vertex noise...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def _apply_hull_vertex_noise(': 'vertex noise helper function',
        'ship_class': 'ship_class parameter in generate_hull',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_greeble_details():
    """Test that ship_generator defines the greeble detail pass"""
    print("\nTesting greeble detail pass...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_greeble_details(': 'greeble generator function',
        '_GREEBLE_TYPES': 'greeble brick types list',
        'Stage 6b': 'greeble stage in pipeline',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_enhanced_engine_details():
    """Test that ship_parts has exhaust ring and inner cone helpers"""
    print("\nTesting enhanced engine detail helpers...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def _add_exhaust_rings(': 'exhaust rings helper',
        'def _add_inner_cone(': 'inner cone helper',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_module_placement_zones():
    """Test that module_system uses zone-based placement"""
    print("\nTesting module placement zones...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ms_path = os.path.join(addon_path, 'module_system.py')

    with open(ms_path, 'r') as f:
        content = f.read()

    all_valid = True

    if 'total_count' in content:
        print("✓ total_count parameter for spacing found")
    else:
        print("✗ total_count parameter not found")
        all_valid = False

    # Zone-based placement should reference port/starboard or side
    if 'side' in content or 'port' in content or 'starboard' in content:
        print("✓ Zone-based side placement logic found")
    else:
        print("✗ Zone-based placement logic not found")
        all_valid = False

    return all_valid


def test_novaforge_importer():
    """Test that novaforge_importer.py has proper structure and functions"""
    print("\nTesting NovaForge importer...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    nf_path = os.path.join(addon_path, 'novaforge_importer.py')

    if not os.path.exists(nf_path):
        print("✗ novaforge_importer.py not found")
        return False

    with open(nf_path, 'r') as f:
        content = f.read()

    checks = {
        'RACE_TO_STYLE': 'race-to-style mapping',
        'CLASS_MAP': 'class mapping dictionary',
        'NOVAFORGE_SCALES': 'NovaForge scale table',
        'def load_ships_from_file(': 'load_ships_from_file function',
        'def load_ships_from_directory(': 'load_ships_from_directory function',
        'def ship_to_generator_params(': 'ship_to_generator_params function',
        'def get_all_generator_params(': 'get_all_generator_params function',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_launcher_hardpoints():
    """Test that ship_parts.py defines generate_launcher_hardpoints"""
    print("\nTesting launcher hardpoints function...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_launcher_hardpoints(': 'generate_launcher_hardpoints function',
        '"hardpoint_type"': 'hardpoint_type custom property',
        '"launcher_index"': 'launcher_index custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_drone_bays():
    """Test that ship_parts.py defines generate_drone_bays"""
    print("\nTesting drone bays function...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_drone_bays(': 'generate_drone_bays function',
        '"drone_bay_index"': 'drone_bay_index custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_faction_details():
    """Test that ship_parts.py defines add_faction_details and all 4 faction helpers"""
    print("\nTesting faction detail generators...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def add_faction_details(': 'add_faction_details function',
        'def _add_solari_spires(': 'Solari spires helper',
        'def _add_veyren_panels(': 'Veyren panels helper',
        'def _add_aurelian_curves(': 'Aurelian curves helper',
        'def _add_keldari_framework(': 'Keldari framework helper',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_render_setup():
    """Test that render_setup.py has proper structure"""
    print("\nTesting render setup module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    rs_path = os.path.join(addon_path, 'render_setup.py')

    if not os.path.exists(rs_path):
        print("✗ render_setup.py not found")
        return False

    valid, error = test_python_syntax(rs_path)
    if not valid:
        print(f"✗ render_setup.py has syntax error: {error}")
        return False
    print("✓ render_setup.py has valid syntax")

    with open(rs_path, 'r') as f:
        content = f.read()

    checks = {
        'CATALOG_RESOLUTION': 'catalog resolution constant',
        'THUMBNAIL_RESOLUTION': 'thumbnail resolution constant',
        'def setup_catalog_render(': 'setup_catalog_render function',
        'def setup_thumbnail_render(': 'setup_thumbnail_render function',
        'def render_to_file(': 'render_to_file function',
        'def _get_or_create_camera(': 'camera helper',
        'def _setup_three_point_lighting(': 'three-point lighting helper',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_enhanced_textures():
    """Test that texture_generator.py defines new material generators"""
    print("\nTesting enhanced texture functions...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    tg_path = os.path.join(addon_path, 'texture_generator.py')

    with open(tg_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_normal_material(': 'generate_normal_material function',
        'def generate_glow_material(': 'generate_glow_material function',
        'def generate_dirt_material(': 'generate_dirt_material function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_lod_generator():
    """Test that lod_generator.py has proper structure and constants"""
    print("\nTesting LOD generator module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    lg_path = os.path.join(addon_path, 'lod_generator.py')

    if not os.path.exists(lg_path):
        print("✗ lod_generator.py not found")
        return False

    valid, error = test_python_syntax(lg_path)
    if not valid:
        print(f"✗ lod_generator.py has syntax error: {error}")
        return False
    print("✓ lod_generator.py has valid syntax")

    with open(lg_path, 'r') as f:
        content = f.read()

    checks = {
        'LOD_LEVELS': 'LOD levels dictionary',
        'LOD_DISTANCES': 'LOD distances per ship class',
        'def generate_lods(': 'generate_lods function',
        'def get_lod_distances(': 'get_lod_distances function',
        'def apply_decimate(': 'apply_decimate function',
        '"lod_level"': 'lod_level custom property',
        '"lod_ratio"': 'lod_ratio custom property',
        '"lod_distance"': 'lod_distance custom property',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Check that LOD_DISTANCES has entries for all ship classes
    import importlib.util
    spec = importlib.util.spec_from_file_location("lod_generator", lg_path)
    lg = importlib.util.module_from_spec(spec)

    # Patch bpy import for non-Blender environment
    import types
    bpy_mock = types.ModuleType('bpy')
    sys.modules['bpy'] = bpy_mock
    try:
        spec.loader.exec_module(lg)
    finally:
        del sys.modules['bpy']

    sg_path = os.path.join(addon_path, 'ship_generator.py')
    with open(sg_path, 'r') as f:
        sg_content = f.read()

    tree = ast.parse(sg_content)
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'SHIP_CONFIGS':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                cls = k.value
                                if cls in lg.LOD_DISTANCES:
                                    print(f"✓ LOD_DISTANCES has entry for {cls}")
                                else:
                                    print(f"✗ LOD_DISTANCES missing entry for {cls}")
                                    all_valid = False

    return all_valid


def test_collision_generator():
    """Test that collision_generator.py has proper structure and functions"""
    print("\nTesting collision generator module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    cg_path = os.path.join(addon_path, 'collision_generator.py')

    if not os.path.exists(cg_path):
        print("✗ collision_generator.py not found")
        return False

    valid, error = test_python_syntax(cg_path)
    if not valid:
        print(f"✗ collision_generator.py has syntax error: {error}")
        return False
    print("✓ collision_generator.py has valid syntax")

    with open(cg_path, 'r') as f:
        content = f.read()

    checks = {
        'COLLISION_TYPES': 'collision types dictionary',
        'DEFAULT_COLLISION_TYPE': 'default collision type per ship class',
        'def generate_collision_mesh(': 'generate_collision_mesh function',
        'def generate_box_collision(': 'generate_box_collision function',
        'def generate_convex_hull_collision(': 'generate_convex_hull_collision function',
        'def get_collision_type(': 'get_collision_type function',
        '"collision_type"': 'collision_type custom property',
        '"collision_source"': 'collision_source custom property',
        '"is_collision_mesh"': 'is_collision_mesh custom property',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_animation_system():
    """Test that animation_system.py has proper structure and functions"""
    print("\nTesting animation system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    as_path = os.path.join(addon_path, 'animation_system.py')

    if not os.path.exists(as_path):
        print("✗ animation_system.py not found")
        return False

    valid, error = test_python_syntax(as_path)
    if not valid:
        print(f"✗ animation_system.py has syntax error: {error}")
        return False
    print("✓ animation_system.py has valid syntax")

    with open(as_path, 'r') as f:
        content = f.read()

    checks = {
        'FRAME_RATE': 'frame rate constant',
        'ANIMATION_PRESETS': 'animation presets dictionary',
        'def setup_ship_animations(': 'setup_ship_animations function',
        'def create_turret_animation(': 'create_turret_animation function',
        'def create_bay_door_animation(': 'create_bay_door_animation function',
        'def create_landing_gear_animation(': 'create_landing_gear_animation function',
        'def create_radar_spin_animation(': 'create_radar_spin_animation function',
        'TURRET_ROTATE': 'turret rotate preset',
        'BAY_DOOR_OPEN': 'bay door open preset',
        'LANDING_GEAR_EXTEND': 'landing gear preset',
        'RADAR_SPIN': 'radar spin preset',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_damage_system():
    """Test that damage_system.py has proper structure and functions"""
    print("\nTesting damage system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ds_path = os.path.join(addon_path, 'damage_system.py')

    if not os.path.exists(ds_path):
        print("✗ damage_system.py not found")
        return False

    valid, error = test_python_syntax(ds_path)
    if not valid:
        print(f"✗ damage_system.py has syntax error: {error}")
        return False
    print("✓ damage_system.py has valid syntax")

    with open(ds_path, 'r') as f:
        content = f.read()

    checks = {
        'HULL_WEIGHTS': 'hull influence weights dictionary',
        'BRICK_HP': 'brick hit points dictionary',
        'BRICK_MASS': 'brick mass dictionary',
        'SALVAGE_DROP_CHANCE': 'salvage drop chance dictionary',
        'DAMAGE_STATES': 'damage visual states dictionary',
        'DETACH_TIMEOUT': 'detachment timeout constant',
        'class BrickEntity': 'BrickEntity class',
        'class SalvageEntity': 'SalvageEntity class',
        'class ShipDamageState': 'ShipDamageState class',
        'def from_ship_dna(': 'from_ship_dna constructor',
        'def apply_damage(': 'apply_damage method',
        'def rebuild_connections(': 'rebuild_connections method',
        'def get_unsupported_bricks(': 'get_unsupported_bricks method',
        'def tick(': 'tick method',
        'def total_hull_weight(': 'total_hull_weight method',
        'def get_damage_summary(': 'get_damage_summary method',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Functional test — import and exercise the module
    import importlib.util
    spec = importlib.util.spec_from_file_location("damage_system", ds_path)
    ds = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ds)

    bricks = [
        {'type': 'STRUCTURAL_SPINE', 'pos': [0, 0, 0]},
        {'type': 'REACTOR_CORE', 'pos': [0, 2, 0]},
        {'type': 'ENGINE_BLOCK', 'pos': [0, -2, 0]},
    ]
    state = ds.ShipDamageState.from_ship_dna(bricks, grid_size=2.0, seed=1)
    if state.alive_count() == 3:
        print("✓ ShipDamageState.from_ship_dna round-trip works")
    else:
        print(f"✗ Expected 3 bricks, got {state.alive_count()}")
        all_valid = False

    events = state.apply_damage(0, 100)
    if isinstance(events, list):
        print("✓ apply_damage returns event list")
    else:
        print("✗ apply_damage did not return a list")
        all_valid = False

    summary = state.get_damage_summary()
    if isinstance(summary, dict) and 'NOMINAL' in summary:
        print("✓ get_damage_summary returns correct structure")
    else:
        print("✗ get_damage_summary returned unexpected result")
        all_valid = False

    # Edge case: destroy spine to disconnect remaining bricks
    events = state.apply_damage(0, 9999)
    unsupported = state.get_unsupported_bricks()
    if len(unsupported) > 0 or state.alive_count() < 3:
        print("✓ Destroying spine disconnects remaining bricks")
    else:
        print("✗ Spine destruction did not cascade correctly")
        all_valid = False

    return all_valid


def test_power_system():
    """Test that power_system.py has proper structure and functions"""
    print("\nTesting power system module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    ps_path = os.path.join(addon_path, 'power_system.py')

    if not os.path.exists(ps_path):
        print("✗ power_system.py not found")
        return False

    valid, error = test_python_syntax(ps_path)
    if not valid:
        print(f"✗ power_system.py has syntax error: {error}")
        return False
    print("✓ power_system.py has valid syntax")

    with open(ps_path, 'r') as f:
        content = f.read()

    checks = {
        'POWER_GENERATION': 'power generation dictionary',
        'POWER_CONSUMPTION': 'power consumption dictionary',
        'CAPACITOR_SIZE': 'capacitor size per ship class',
        'NON_ESSENTIAL_TYPES': 'non-essential system types',
        'class PowerComponent': 'PowerComponent class',
        'class ShipPowerState': 'ShipPowerState class',
        'def from_ship_dna(': 'from_ship_dna constructor',
        'def from_damage_state(': 'from_damage_state constructor',
        'def tick(': 'tick method',
        'def sync_with_damage_state(': 'sync_with_damage_state method',
        'def get_power_summary(': 'get_power_summary method',
        'def total_generation(': 'total_generation property',
        'def total_consumption(': 'total_consumption property',
        'def capacitor_fraction(': 'capacitor_fraction property',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Functional test
    import importlib.util
    spec = importlib.util.spec_from_file_location("power_system", ps_path)
    ps = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ps)

    dna = {
        'class': 'CRUISER',
        'bricks': [
            {'type': 'STRUCTURAL_SPINE', 'pos': [0, 0, 0]},
            {'type': 'REACTOR_CORE', 'pos': [0, 2, 0]},
            {'type': 'ENGINE_BLOCK', 'pos': [0, -2, 0]},
        ]
    }
    power = ps.ShipPowerState.from_ship_dna(dna)
    if power.total_generation == 100.0:
        print("✓ Reactor generates 100 MW")
    else:
        print(f"✗ Expected 100.0 MW generation, got {power.total_generation}")
        all_valid = False

    if power.power_stable:
        print("✓ Power is stable with reactor")
    else:
        print("✗ Power should be stable with reactor present")
        all_valid = False

    summary = power.get_power_summary()
    if isinstance(summary, dict) and 'net_power' in summary:
        print("✓ get_power_summary returns correct structure")
    else:
        print("✗ get_power_summary returned unexpected result")
        all_valid = False

    # Edge case: no reactor — systems should disable when cap drains
    no_reactor_dna = {
        'class': 'FRIGATE',
        'bricks': [
            {'type': 'STRUCTURAL_SPINE', 'pos': [0, 0, 0]},
            {'type': 'SHIELD_EMITTER', 'pos': [1, 0, 0]},
        ]
    }
    drain_power = ps.ShipPowerState.from_ship_dna(no_reactor_dna)
    if not drain_power.power_stable:
        print("✓ No-reactor ship correctly reports unstable power")
    else:
        print("✗ No-reactor ship should report unstable power")
        all_valid = False

    # Drain and check disable
    for _ in range(100):
        drain_power.tick(dt=1.0)
    if len(drain_power.disabled_ids) > 0:
        print("✓ Non-essential systems disabled after capacitor drain")
    else:
        print("✗ Systems should be disabled after capacitor drain")
        all_valid = False

    # Check all 18 ship classes have capacitor size entries
    bs_path = os.path.join(addon_path, 'brick_system.py')
    spec2 = importlib.util.spec_from_file_location("brick_system", bs_path)
    bs = importlib.util.module_from_spec(spec2)
    spec2.loader.exec_module(bs)

    for ship_class in bs.GRID_SIZES:
        if ship_class in ps.CAPACITOR_SIZE:
            print(f"✓ CAPACITOR_SIZE has entry for {ship_class}")
        else:
            print(f"✗ CAPACITOR_SIZE missing entry for {ship_class}")
            all_valid = False

    return all_valid


def test_build_validator():
    """Test that build_validator.py has proper structure and functions"""
    print("\nTesting build validator module...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    bv_path = os.path.join(addon_path, 'build_validator.py')

    if not os.path.exists(bv_path):
        print("✗ build_validator.py not found")
        return False

    valid, error = test_python_syntax(bv_path)
    if not valid:
        print(f"✗ build_validator.py has syntax error: {error}")
        return False
    print("✓ build_validator.py has valid syntax")

    with open(bv_path, 'r') as f:
        content = f.read()

    checks = {
        'COMPATIBLE_ROLES': 'compatible roles set',
        'class ValidationResult': 'ValidationResult class',
        'class BuildValidator': 'BuildValidator class',
        'def validate_placement(': 'validate_placement method',
        'def place_brick(': 'place_brick method',
        'def remove_brick(': 'remove_brick method',
        'def check_connectivity(': 'check_connectivity method',
        'def check_power_connectivity(': 'check_power_connectivity method',
        'def from_ship_dna(': 'from_ship_dna constructor',
        'def register()': 'register function',
        'def unregister()': 'unregister function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    # Functional test
    import importlib.util
    spec = importlib.util.spec_from_file_location("build_validator", bv_path)
    bv = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bv)

    bs_path = os.path.join(addon_path, 'brick_system.py')
    spec2 = importlib.util.spec_from_file_location("brick_system", bs_path)
    bs = importlib.util.module_from_spec(spec2)
    spec2.loader.exec_module(bs)

    validator = bv.BuildValidator(grid_size=2.0, brick_types=bs.BRICK_TYPES)

    # First brick always valid
    r = validator.validate_placement('STRUCTURAL_SPINE', (0, 0, 0))
    if r.valid:
        print("✓ First spine placement is valid")
    else:
        print(f"✗ First spine placement should be valid: {r.errors}")
        all_valid = False
    validator.place_brick('STRUCTURAL_SPINE', (0, 0, 0))

    # Occupied cell should be invalid
    r = validator.validate_placement('HULL_PLATE', (0, 0, 0))
    if not r.valid:
        print("✓ Occupied cell correctly rejected")
    else:
        print("✗ Occupied cell should be invalid")
        all_valid = False

    # Connectivity check
    if validator.check_connectivity():
        print("✓ check_connectivity works")
    else:
        print("✗ check_connectivity failed")
        all_valid = False

    return all_valid


def test_connecting_geometry():
    """Test that ship_parts.py defines connecting geometry helpers"""
    print("\nTesting connecting geometry functions...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_cockpit_neck(': 'cockpit neck fairing function',
        'def generate_engine_pylons(': 'engine pylon function',
        'def generate_wing_roots(': 'wing root fairing function',
        'def generate_weapon_pylons(': 'weapon pylon function',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_batch_generate_operator():
    """Test that __init__.py defines the batch generate all operator"""
    print("\nTesting batch generate operator...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    init_path = os.path.join(addon_path, '__init__.py')

    with open(init_path, 'r') as f:
        content = f.read()

    checks = {
        'class SPACESHIP_OT_batch_generate': 'batch generate operator class',
        "bl_idname = \"mesh.batch_generate_all\"": 'batch generate bl_idname',
        'batch_output_path': 'batch output path property',
        'SPACESHIP_OT_batch_generate,': 'operator registered in classes tuple',
        'class SPACESHIP_OT_novaforge_pipeline': 'NovaForge pipeline operator class',
        "bl_idname = \"mesh.novaforge_pipeline_export\"": 'NovaForge pipeline bl_idname',
        'SPACESHIP_OT_novaforge_pipeline,': 'pipeline operator registered in classes',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_emission_color_compat():
    """Test that texture_generator.py handles both Emission and Emission Color"""
    print("\nTesting emission color compatibility...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    tg_path = os.path.join(addon_path, 'texture_generator.py')

    with open(tg_path, 'r') as f:
        content = f.read()

    # Should NOT have a bare 'Emission Color' key access without a fallback
    has_compat = "'Emission Color' if 'Emission Color' in principled.inputs else 'Emission'" in content
    no_bare_access = "principled.inputs['Emission Color'].default_value" not in content

    all_valid = True
    if has_compat:
        print("✓ Emission Color / Emission compatibility check found")
    else:
        print("✗ Missing compatibility check for Emission Color / Emission")
        all_valid = False

    if no_bare_access:
        print("✓ No bare 'Emission Color' key access")
    else:
        print("✗ Bare 'Emission Color' key access still present (will fail on Blender 3.x)")
        all_valid = False

    return all_valid


def test_ship_class_consistency():
    """Test that all 18 ship classes are present in every key data structure"""
    print("\nTesting ship class consistency across modules...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    import importlib.util

    # Expected 18 classes from ENGINE_INTEGRATION.md
    expected_classes = {
        'SHUTTLE', 'FIGHTER', 'CORVETTE', 'FRIGATE', 'DESTROYER', 'CRUISER',
        'BATTLECRUISER', 'BATTLESHIP', 'CARRIER', 'DREADNOUGHT', 'CAPITAL',
        'TITAN', 'INDUSTRIAL', 'MINING_BARGE', 'EXHUMER', 'EXPLORER',
        'HAULER', 'EXOTIC',
    }

    all_valid = True

    # 1. SHIP_CONFIGS in ship_generator.py (parse via AST to avoid bpy)
    sg_path = os.path.join(addon_path, 'ship_generator.py')
    with open(sg_path, 'r') as f:
        sg_content = f.read()
    tree = ast.parse(sg_content)
    sg_classes = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'SHIP_CONFIGS':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                sg_classes.add(k.value)
    missing = expected_classes - sg_classes
    if not missing:
        print(f"✓ SHIP_CONFIGS has all {len(expected_classes)} classes")
    else:
        print(f"✗ SHIP_CONFIGS missing: {missing}")
        all_valid = False

    # 2. GRID_SIZES in brick_system.py
    bs_path = os.path.join(addon_path, 'brick_system.py')
    spec = importlib.util.spec_from_file_location("brick_system", bs_path)
    bs = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bs)
    missing = expected_classes - set(bs.GRID_SIZES.keys())
    if not missing:
        print(f"✓ GRID_SIZES has all {len(expected_classes)} classes")
    else:
        print(f"✗ GRID_SIZES missing: {missing}")
        all_valid = False

    # 3. HULL_PROFILES in ship_parts.py (parse via AST)
    sp_path = os.path.join(addon_path, 'ship_parts.py')
    with open(sp_path, 'r') as f:
        sp_content = f.read()
    sp_tree = ast.parse(sp_content)
    hp_classes = set()
    for node in ast.walk(sp_tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'HULL_PROFILES':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                hp_classes.add(k.value)
    missing = expected_classes - hp_classes
    if not missing:
        print(f"✓ HULL_PROFILES has all {len(expected_classes)} classes")
    else:
        print(f"✗ HULL_PROFILES missing: {missing}")
        all_valid = False

    # 4. LOD_DISTANCES in lod_generator.py
    lg_path = os.path.join(addon_path, 'lod_generator.py')
    spec = importlib.util.spec_from_file_location("lod_generator", lg_path)
    lg = importlib.util.module_from_spec(spec)
    import types
    bpy_mock = types.ModuleType('bpy')
    sys.modules['bpy'] = bpy_mock
    try:
        spec.loader.exec_module(lg)
    finally:
        del sys.modules['bpy']
    missing = expected_classes - set(lg.LOD_DISTANCES.keys())
    if not missing:
        print(f"✓ LOD_DISTANCES has all {len(expected_classes)} classes")
    else:
        print(f"✗ LOD_DISTANCES missing: {missing}")
        all_valid = False

    # 5. DEFAULT_COLLISION_TYPE in collision_generator.py
    cg_path = os.path.join(addon_path, 'collision_generator.py')
    with open(cg_path, 'r') as f:
        cg_content = f.read()
    tree = ast.parse(cg_content)
    cg_classes = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'DEFAULT_COLLISION_TYPE':
                    if isinstance(node.value, ast.Dict):
                        for k in node.value.keys:
                            if isinstance(k, ast.Constant):
                                cg_classes.add(k.value)
    missing = expected_classes - cg_classes
    if not missing:
        print(f"✓ DEFAULT_COLLISION_TYPE has all {len(expected_classes)} classes")
    else:
        print(f"✗ DEFAULT_COLLISION_TYPE missing: {missing}")
        all_valid = False

    # 6. CAPACITOR_SIZE in power_system.py
    ps_path = os.path.join(addon_path, 'power_system.py')
    spec = importlib.util.spec_from_file_location("power_system", ps_path)
    ps = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ps)
    missing = expected_classes - set(ps.CAPACITOR_SIZE.keys())
    if not missing:
        print(f"✓ CAPACITOR_SIZE has all {len(expected_classes)} classes")
    else:
        print(f"✗ CAPACITOR_SIZE missing: {missing}")
        all_valid = False

    return all_valid


def run_tests():
    """Run all validation tests"""
    print("=" * 60)
    print("Blender Spaceship Generator - Validation Tests")
    print("=" * 60)
    
    tests = [
        ("Addon Structure", test_addon_structure),
        ("Python Syntax", test_file_syntax),
        ("bl_info Metadata", test_bl_info),
        ("Register Functions", test_register_functions),
        ("Documentation", test_documentation),
        ("Turret Hardpoint Configs", test_turret_hardpoint_configs),
        ("Naming Prefix Support", test_naming_prefix_support),
        ("Turret Generation Function", test_turret_generation_function),
        ("Brick System", test_brick_system),
        ("Hull Taper & Cleanup", test_hull_taper_and_cleanup),
        ("Engine Archetypes", test_engine_archetypes),
        ("Hull Shape Profiles", test_hull_profiles),
        ("Hull Vertex Noise", test_vertex_noise),
        ("Greeble Detail Pass", test_greeble_details),
        ("Enhanced Engine Details", test_enhanced_engine_details),
        ("Module Placement Zones", test_module_placement_zones),
        ("NovaForge Importer", test_novaforge_importer),
        ("Launcher Hardpoints", test_launcher_hardpoints),
        ("Drone Bays", test_drone_bays),
        ("Faction Details", test_faction_details),
        ("Render Setup", test_render_setup),
        ("Enhanced Textures", test_enhanced_textures),
        ("LOD Generator", test_lod_generator),
        ("Collision Generator", test_collision_generator),
        ("Animation System", test_animation_system),
        ("Damage System", test_damage_system),
        ("Power System", test_power_system),
        ("Build Validator", test_build_validator),
        ("Connecting Geometry", test_connecting_geometry),
        ("Batch Generate Operator", test_batch_generate_operator),
        ("Emission Color Compat", test_emission_color_compat),
        ("Ship Class Consistency", test_ship_class_consistency),
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"\n✗ {test_name} raised exception: {e}")
            import traceback
            traceback.print_exc()
            results.append((test_name, False))
    
    print("\n" + "=" * 60)
    print("Test Results Summary:")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {test_name}")
    
    print("=" * 60)
    print(f"Total: {passed}/{total} tests passed")
    
    if passed == total:
        print("✓ All validation tests passed!")
    else:
        print("✗ Some tests failed")
    
    print("=" * 60)
    
    return passed == total


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
