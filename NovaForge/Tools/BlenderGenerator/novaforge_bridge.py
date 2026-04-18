"""
NovaForge Bridge — Blender Add-on (BG-1 scaffold)

Blender add-on that provides a "NovaForge Sync" N-panel for exporting meshes,
rigs, and animation clips directly into the shared workspace pipeline.

Installation:
    Blender → Edit → Preferences → Add-ons → Install → select this file → enable.

Pipeline integration:
    Exports drop files into  <workspace>/.novaforge/pipeline/assets/  and
    <workspace>/.novaforge/pipeline/animations/, then write a matching
    .change.json event into  <workspace>/.novaforge/pipeline/changes/
    so the NovaForge Editor picks them up automatically via PipelineWatcher.
"""

bl_info = {
    "name":        "NovaForge Bridge",
    "author":      "NovaForge",
    "version":     (0, 1, 0),
    "blender":     (3, 6, 0),
    "location":    "View3D > N-Panel > NovaForge",
    "description": "Export meshes, rigs and animations to the NovaForge workspace pipeline",
    "category":    "Import-Export",
}

import bpy
import json
import os
import time
from bpy.props import StringProperty
from bpy.types import Panel, Operator, AddonPreferences


# ── Preferences ──────────────────────────────────────────────────────────────

class NovaForgePreferences(AddonPreferences):
    bl_idname = __name__

    workspace_root: StringProperty(
        name="Workspace Root",
        description="Absolute path to the NovaForge workspace directory",
        default="",
        subtype="DIR_PATH",
    )

    def draw(self, context):
        layout = self.layout
        layout.label(text="Workspace Root (folder containing .novaforge/):")
        layout.prop(self, "workspace_root", text="")


def _get_prefs(context):
    return context.preferences.addons[__name__].preferences


def _pipeline_dir(context, subdir):
    """Return the absolute path to .novaforge/pipeline/<subdir>."""
    prefs = _get_prefs(context)
    root = prefs.workspace_root.strip()
    if not root:
        return None
    return os.path.join(root, ".novaforge", "pipeline", subdir)


def _changes_dir(context):
    return _pipeline_dir(context, "changes")


def _write_change_event(context, tool, event_type, path, metadata=""):
    """Drop a .change.json file into the pipeline/changes/ directory."""
    changes = _changes_dir(context)
    if not changes:
        return False
    os.makedirs(changes, exist_ok=True)
    ts = int(time.time() * 1000)
    event = {
        "tool":       tool,
        "event_type": event_type,
        "path":       path,
        "timestamp":  ts,
        "metadata":   metadata,
    }
    fname = f"{tool}_{event_type}_{ts}.change.json"
    filepath = os.path.join(changes, fname)
    with open(filepath, "w", encoding="utf-8") as f:
        json.dump(event, f, indent=2)
    return True


# ── BG-1: Export Mesh ────────────────────────────────────────────────────────

class NF_OT_ExportMesh(Operator):
    """Export the active mesh object as .glb into pipeline/assets/"""
    bl_idname  = "novaforge.export_mesh"
    bl_label   = "Export Mesh to NovaForge"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type != "MESH":
            self.report({"ERROR"}, "Select a mesh object first.")
            return {"CANCELLED"}

        assets_dir = _pipeline_dir(context, "assets")
        if not assets_dir:
            self.report({"ERROR"}, "Set the Workspace Root in add-on preferences.")
            return {"CANCELLED"}

        os.makedirs(assets_dir, exist_ok=True)
        filename = bpy.path.clean_name(obj.name) + ".glb"
        out_path = os.path.join(assets_dir, filename)

        bpy.ops.export_scene.gltf(
            filepath=out_path,
            use_selection=True,
            export_format="GLB",
            export_animations=False,
        )

        rel_path = os.path.join("pipeline", "assets", filename)
        _write_change_event(context, "BlenderGenerator", "AssetImported",
                            rel_path, metadata="type=mesh")

        self.report({"INFO"}, f"Exported '{obj.name}' → {out_path}")
        return {"FINISHED"}


# ── BG-2: Export Rig ─────────────────────────────────────────────────────────
# (milestone BG-2 — stub ready for implementation)

class NF_OT_ExportRig(Operator):
    """Export the active armature + mesh as a skinned .glb into pipeline/animations/"""
    bl_idname  = "novaforge.export_rig"
    bl_label   = "Export Rig to NovaForge"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type not in {"MESH", "ARMATURE"}:
            self.report({"ERROR"}, "Select a mesh or armature object first.")
            return {"CANCELLED"}

        anim_dir = _pipeline_dir(context, "animations")
        if not anim_dir:
            self.report({"ERROR"}, "Set the Workspace Root in add-on preferences.")
            return {"CANCELLED"}

        os.makedirs(anim_dir, exist_ok=True)
        filename = bpy.path.clean_name(obj.name) + "_rig.glb"
        out_path = os.path.join(anim_dir, filename)

        bpy.ops.export_scene.gltf(
            filepath=out_path,
            use_selection=True,
            export_format="GLB",
            export_skins=True,
            export_animations=False,
        )

        rel_path = os.path.join("pipeline", "animations", filename)
        _write_change_event(context, "BlenderGenerator", "AnimationExported",
                            rel_path, metadata="type=rig")

        self.report({"INFO"}, f"Exported rig '{obj.name}' → {out_path}")
        return {"FINISHED"}


# ── BG-2: Export Animation Clip ───────────────────────────────────────────────

class NF_OT_ExportAnimClip(Operator):
    """Export the active NLA tracks as a .glb animation clip into pipeline/animations/"""
    bl_idname  = "novaforge.export_anim_clip"
    bl_label   = "Export Anim Clip to NovaForge"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({"ERROR"}, "No active object.")
            return {"CANCELLED"}

        anim_dir = _pipeline_dir(context, "animations")
        if not anim_dir:
            self.report({"ERROR"}, "Set the Workspace Root in add-on preferences.")
            return {"CANCELLED"}

        os.makedirs(anim_dir, exist_ok=True)
        filename = bpy.path.clean_name(obj.name) + "_anim.glb"
        out_path = os.path.join(anim_dir, filename)

        bpy.ops.export_scene.gltf(
            filepath=out_path,
            use_selection=True,
            export_format="GLB",
            export_animations=True,
            export_nla_strips=True,
        )

        rel_path = os.path.join("pipeline", "animations", filename)
        _write_change_event(context, "BlenderGenerator", "AnimationExported",
                            rel_path, metadata="type=clip")

        self.report({"INFO"}, f"Exported clip for '{obj.name}' → {out_path}")
        return {"FINISHED"}


# ── N-Panel ───────────────────────────────────────────────────────────────────

class NF_PT_SyncPanel(Panel):
    bl_label      = "NovaForge Sync"
    bl_idname     = "NF_PT_sync_panel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category   = "NovaForge"

    def draw(self, context):
        layout = self.layout
        prefs  = _get_prefs(context)

        # Workspace root indicator
        row = layout.row()
        if prefs.workspace_root:
            row.label(text="✔ Workspace set", icon="CHECKMARK")
        else:
            row.label(text="⚠ Set workspace root in preferences", icon="ERROR")

        layout.separator()
        layout.label(text="Export (BG-1 / BG-2):", icon="EXPORT")
        layout.operator("novaforge.export_mesh",      icon="MESH_DATA")
        layout.operator("novaforge.export_rig",       icon="ARMATURE_DATA")
        layout.operator("novaforge.export_anim_clip", icon="ACTION")

        layout.separator()
        layout.label(text="Preferences:", icon="PREFERENCES")
        layout.prop(prefs, "workspace_root", text="Root")


# ── Registration ──────────────────────────────────────────────────────────────

_CLASSES = [
    NovaForgePreferences,
    NF_OT_ExportMesh,
    NF_OT_ExportRig,
    NF_OT_ExportAnimClip,
    NF_PT_SyncPanel,
]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
