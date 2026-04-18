#!/usr/bin/env python3
"""
Test suite for create_atlas_project.py

Verifies that the project scaffolder creates constitution-compliant
Atlas projects with the correct structure, manifest, and stub files.
"""

import json
import os
import sys
import pathlib
import tempfile

# Add tools to path for importing
sys.path.insert(0, str(pathlib.Path(__file__).parent.parent / 'tools'))
import create_atlas_project


def test_creates_project_structure():
    """Test that the scaffolder creates all required directories."""
    print("Testing project structure creation...")

    with tempfile.TemporaryDirectory() as tmpdir:
        root = create_atlas_project.create_project("MyGame", pathlib.Path(tmpdir))
        expected_dirs = [
            "src",
            "src/systems",
            "src/components",
            "scripts",
            "assets",
            "replays",
            "migrations",
            "config",
        ]
        for d in expected_dirs:
            assert (root / d).is_dir(), f"Missing directory: {d}"
        print("  ✓ All directories created")


def test_creates_atlas_manifest():
    """Test that the .atlas project file is valid JSON with required fields."""
    print("Testing .atlas manifest creation...")

    with tempfile.TemporaryDirectory() as tmpdir:
        root = create_atlas_project.create_project("TestGame", pathlib.Path(tmpdir))
        manifest_path = root / "TestGame.atlas"
        assert manifest_path.exists(), "Missing .atlas manifest"

        data = json.loads(manifest_path.read_text())
        project = data["project"]
        assert project["name"] == "TestGame", "Name mismatch"
        assert project["version"] == "0.1.0", "Version mismatch"
        assert project["binary_abi"] == "atlas_abi_v1", "ABI mismatch"
        assert project["determinism_profile"] == "fixed32_v1", "Profile mismatch"
        assert project["requires"]["replay_support"] is True, "Replay support missing"
        assert project["requires"]["migration_support"] is True, "Migration support missing"
        print("  ✓ Manifest is valid")


def test_creates_stub_system():
    """Test that the stub system file contains ATLAS_SYSTEM_TRAITS."""
    print("Testing stub system creation...")

    with tempfile.TemporaryDirectory() as tmpdir:
        root = create_atlas_project.create_project("StubTest", pathlib.Path(tmpdir))
        system_path = root / "src" / "systems" / "ExampleSystem.cpp"
        assert system_path.exists(), "Missing ExampleSystem.cpp"

        content = system_path.read_text()
        assert "ATLAS_SYSTEM_TRAITS" in content, "Missing ATLAS_SYSTEM_TRAITS"
        assert "ExampleSystem" in content, "Missing ExampleSystem class"
        assert "deterministic" in content.lower(), "Missing deterministic trait"
        print("  ✓ Stub system is valid")


def test_creates_stub_component():
    """Test that the stub component file contains SchemaVersion."""
    print("Testing stub component creation...")

    with tempfile.TemporaryDirectory() as tmpdir:
        root = create_atlas_project.create_project("CompTest", pathlib.Path(tmpdir))
        component_path = root / "src" / "components" / "ExampleComponent.h"
        assert component_path.exists(), "Missing ExampleComponent.h"

        content = component_path.read_text()
        assert "SchemaVersion" in content, "Missing SchemaVersion"
        assert "#pragma once" in content, "Missing include guard"
        print("  ✓ Stub component is valid")


def test_creates_readme():
    """Test that the README references the project name."""
    print("Testing README creation...")

    with tempfile.TemporaryDirectory() as tmpdir:
        root = create_atlas_project.create_project("ReadmeTest", pathlib.Path(tmpdir))
        readme_path = root / "README.md"
        assert readme_path.exists(), "Missing README.md"

        content = readme_path.read_text()
        assert "ReadmeTest" in content, "Project name not in README"
        assert "deterministic" in content.lower(), "Determinism not mentioned"
        print("  ✓ README is valid")


def test_rejects_existing_directory():
    """Test that the scaffolder refuses to overwrite an existing directory."""
    print("Testing existing directory rejection...")

    with tempfile.TemporaryDirectory() as tmpdir:
        # Create the project once
        create_atlas_project.create_project("Duplicate", pathlib.Path(tmpdir))

        # Second attempt should fail
        try:
            create_atlas_project.create_project("Duplicate", pathlib.Path(tmpdir))
            assert False, "Should have raised SystemExit"
        except SystemExit as e:
            assert e.code == 1, f"Expected exit code 1, got {e.code}"
        print("  ✓ Existing directory rejected")


def test_determinism_rules_yaml():
    """Test that determinism_rules.yaml is valid and contains expected rules."""
    print("Testing determinism_rules.yaml...")

    try:
        import yaml
    except ImportError:
        print("  ⏭ Skipped (PyYAML not installed)")
        return

    rules_path = pathlib.Path(__file__).parent.parent / "tools" / "determinism_rules.yaml"
    assert rules_path.exists(), "Missing determinism_rules.yaml"

    data = yaml.safe_load(rules_path.read_text())
    assert "rules" in data, "Missing 'rules' key"
    assert len(data["rules"]) > 0, "No rules defined"

    # Check required rule IDs exist
    rule_ids = {r["id"] for r in data["rules"]}
    expected_ids = {
        "no_unordered_containers",
        "no_system_time",
        "no_global_rng",
        "no_io",
        "no_threading",
    }
    for rid in expected_ids:
        assert rid in rule_ids, f"Missing rule: {rid}"

    # Check each rule has required fields
    for rule in data["rules"]:
        assert "id" in rule, "Rule missing 'id'"
        assert "forbidden" in rule, f"Rule {rule['id']} missing 'forbidden'"
        assert "message" in rule, f"Rule {rule['id']} missing 'message'"
        assert len(rule["forbidden"]) > 0, f"Rule {rule['id']} has empty 'forbidden' list"

    assert "simulation_dirs" in data, "Missing 'simulation_dirs'"
    assert "exempt_dirs" in data, "Missing 'exempt_dirs'"

    print(f"  ✓ {len(data['rules'])} valid rules loaded")


def main():
    print("Running create_atlas_project tests...\n")

    try:
        test_creates_project_structure()
        test_creates_atlas_manifest()
        test_creates_stub_system()
        test_creates_stub_component()
        test_creates_readme()
        test_rejects_existing_directory()
        test_determinism_rules_yaml()

        print("\n✅ All tests passed!")
        return 0
    except AssertionError as e:
        print(f"\n❌ Test failed: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
