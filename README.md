das wird mal ein unity exporter für blender in C

git clone https://git.blender.org/blender.git
cd blender
make update  # holt submodules und prebuilt-libs

# Build-Ordner
mkdir -p ../build_blender && cd ../build_blender
cmake ../blender -DWITH_PYTHON_INSTALL=OFF -DWITH_PYTHON_MODULE=OFF
cmake --build . -j$(nproc)

1) Dateien anlegen
1.1 Neues Modul (C) – Verzeichnis

source/blender/io/io_unity_c_export/

Dateien:

io_unity_c_export.h

io_unity_c_export.c (Writer/Exportlogik)

export_unity_c_op.c (Editor-Operator + File Browser Integration)

CMakeLists.txt
Patch: source/blender/editors/io/CMakeLists.txt

Füge unser neues Lib-Target hinzu (am Ende der target_link_libraries oder analog anderer IO-Module):

# ...
add_subdirectory(../../io/io_unity_c_export ${CMAKE_CURRENT_BINARY_DIR}/io_unity_c_export)
target_link_libraries(bf_editor_io PRIVATE bf_io_unity_c_export)


Je nach Blender-Version sind die Pfade minimal anders (manchmal source/blender/io/… wird schon von Top-Level eingebunden). Wenn CMake meckert, füge add_subdirectory(${CMAKE_SOURCE_DIR}/source/blender/io/io_unity_c_export ...) hinzu.

Patch: source/blender/editors/io/io_ops.c (oder ED_operatortypes_io.c, je nach Version)

Registriere den Operator:

/* am Kopf: */
void EXPORT_OT_unity_c(struct wmOperatorType *ot);

/* in ed_operatortypes_io() o.ä.: */
{
  /* ...bestehende... */
  WM_operatortype_append(EXPORT_OT_unity_c);
}

1.3 Menüeintrag im „File → Export“ hinzufügen

Blenders Menüs sind hauptsächlich in Python definiert. Wir fügen nur einen Eintrag hinzu, der unseren C-Operator aufruft (kein Python-Exporter). Das ist die kleinste notwendige Python-Berührung.

Patch: release/scripts/startup/bl_ui/space_topbar.py

In class TOPBAR_MT_file_export(Menu): draw(...):

# ... bestehende Exporte ...
layout.operator("export_scene.unity_c", text="Unity C Export (.umesh)")


Der idname aus C wird zu Python-style automatisch export_scene.unity_c (Kleinbuchstaben). Falls nicht, kannst du explizit layout.operator("EXPORT_SCENE_unity_c") verwenden.

2) Rebuild
cd ../build_blender
cmake --build . -j$(nproc)
# Binär liegt z.B. unter ./bin/blender


Starte Blender, Menü: File → Export → Unity C Export (.umesh), speichere scene.umesh.
