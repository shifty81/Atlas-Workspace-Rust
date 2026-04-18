# NovaForge Blender Bridge Add-on
# ----------------------------------
# Blender add-on that exports selected objects to the NovaForge engine
# pipeline export directory for automatic import by the editor.
#
# Install: Blender → Edit → Preferences → Add-ons → Install... → select this file.
# Usage:   File → Export → NovaForge Export, or use the NovaForge panel in 3D Viewport.
#
# The add-on writes exported meshes/scenes to the configured export directory,
# which the editor's BlenderAutoImporter watches and auto-imports.

bl_info = {
    "name": "NovaForge Bridge",
    "author": "NovaForge Team",
    "version": (1, 0, 0),
    "blender": (3, 6, 0),
    "location": "File > Export > NovaForge Export / View3D > Sidebar > NovaForge",
    "description": "Export assets to the NovaForge engine pipeline",
    "category": "Import-Export",
}

import bpy
import os
from bpy.props import StringProperty, EnumProperty, BoolProperty


# ── Preferences ─────────────────────────────────────────────────

class NovaForgePreferences(bpy.types.AddonPreferences):
    bl_idname = __name__

    export_directory: StringProperty(
        name="Export Directory",
        description="Directory where exported assets are placed for NovaForge auto-import",
        subtype='DIR_PATH',
        default="",
    )

    default_format: EnumProperty(
        name="Default Format",
        description="Default export file format",
        items=[
            ('FBX', "FBX", "Autodesk FBX format"),
            ('GLTF', "glTF", "glTF 2.0 format"),
            ('OBJ', "OBJ", "Wavefront OBJ format"),
            ('GLB', "GLB", "glTF Binary format"),
        ],
        default='FBX',
    )

    auto_export_on_save: BoolProperty(
        name="Auto-Export on Save",
        description="Automatically export when saving the .blend file",
        default=False,
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "export_directory")
        layout.prop(self, "default_format")
        layout.prop(self, "auto_export_on_save")


# ── Export Operator ──────────────────────────────────────────────

class NOVAFORGE_OT_export(bpy.types.Operator):
    """Export selected objects to the NovaForge pipeline"""
    bl_idname = "novaforge.export"
    bl_label = "NovaForge Export"
    bl_options = {'REGISTER', 'UNDO'}

    export_format: EnumProperty(
        name="Format",
        items=[
            ('FBX', "FBX", ""),
            ('GLTF', "glTF", ""),
            ('OBJ', "OBJ", ""),
            ('GLB', "GLB", ""),
        ],
        default='FBX',
    )

    export_selected_only: BoolProperty(
        name="Selected Only",
        description="Export only selected objects",
        default=True,
    )

    def execute(self, context):
        prefs = context.preferences.addons[__name__].preferences
        export_dir = prefs.export_directory

        if not export_dir:
            self.report({'ERROR'}, "NovaForge export directory not set. Check add-on preferences.")
            return {'CANCELLED'}

        os.makedirs(export_dir, exist_ok=True)

        # Build filename from blend file or active object name
        blend_name = os.path.splitext(os.path.basename(bpy.data.filepath or "untitled"))[0]
        active_name = context.active_object.name if context.active_object else blend_name

        ext_map = {'FBX': '.fbx', 'GLTF': '.gltf', 'OBJ': '.obj', 'GLB': '.glb'}
        ext = ext_map.get(self.export_format, '.fbx')
        filepath = os.path.join(export_dir, active_name + ext)

        try:
            if self.export_format == 'FBX':
                bpy.ops.export_scene.fbx(
                    filepath=filepath,
                    use_selection=self.export_selected_only,
                    apply_scale_options='FBX_SCALE_ALL',
                )
            elif self.export_format in ('GLTF', 'GLB'):
                bpy.ops.export_scene.gltf(
                    filepath=filepath,
                    use_selection=self.export_selected_only,
                    export_format='GLB' if self.export_format == 'GLB' else 'GLTF_SEPARATE',
                )
            elif self.export_format == 'OBJ':
                bpy.ops.wm.obj_export(
                    filepath=filepath,
                    export_selected_objects=self.export_selected_only,
                )
        except Exception as e:
            self.report({'ERROR'}, f"Export failed: {e}")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Exported to: {filepath}")
        return {'FINISHED'}


# ── Side Panel ───────────────────────────────────────────────────

class NOVAFORGE_PT_panel(bpy.types.Panel):
    """NovaForge export panel in 3D Viewport sidebar"""
    bl_label = "NovaForge"
    bl_idname = "NOVAFORGE_PT_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "NovaForge"

    def draw(self, context):
        layout = self.layout
        prefs = context.preferences.addons[__name__].preferences

        layout.label(text="Export Directory:")
        layout.prop(prefs, "export_directory", text="")

        layout.separator()
        layout.label(text="Export Settings:")
        layout.prop(prefs, "default_format")

        layout.separator()
        row = layout.row(align=True)
        op = row.operator("novaforge.export", text="Export Selected", icon='EXPORT')
        op.export_selected_only = True
        op.export_format = prefs.default_format

        row = layout.row(align=True)
        op = row.operator("novaforge.export", text="Export All", icon='WORLD')
        op.export_selected_only = False
        op.export_format = prefs.default_format

        layout.separator()
        layout.prop(prefs, "auto_export_on_save")


# ── Menu Entry ───────────────────────────────────────────────────

def menu_func_export(self, context):
    self.layout.operator(NOVAFORGE_OT_export.bl_idname, text="NovaForge Export")


# ── Auto-Export Handler ──────────────────────────────────────────

def _on_save_post(dummy):
    """Handler called after a .blend file is saved."""
    prefs = bpy.context.preferences.addons.get(__name__)
    if prefs and prefs.preferences.auto_export_on_save:
        bpy.ops.novaforge.export(
            export_format=prefs.preferences.default_format,
            export_selected_only=False,
        )


# ── Registration ─────────────────────────────────────────────────

_classes = (
    NovaForgePreferences,
    NOVAFORGE_OT_export,
    NOVAFORGE_PT_panel,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)
    if _on_save_post not in bpy.app.handlers.save_post:
        bpy.app.handlers.save_post.append(_on_save_post)


def unregister():
    if _on_save_post in bpy.app.handlers.save_post:
        bpy.app.handlers.save_post.remove(_on_save_post)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
