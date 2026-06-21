bl_info = {
    "name": "Unity C Export – Live Sync",
    "author": "Tom",
    "version": (1, 0, 0),
    "blender": (4, 0, 0),
    "location": "3D Viewport > Sidebar (N) > Unity Live",
    "description": "Exportiert beim Speichern automatisch via C-Exporter (.umesh) in den Unity-Assets-Ordner",
    "category": "Import-Export",
}

import bpy

def register():
    print("Unity C Live Sync Add-on aktiviert.")

def unregister():
    print("Unity C Live Sync Add-on deaktiviert.")
