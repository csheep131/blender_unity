// MIT License — use freely.
// Place this file under Assets/Editor/ in your Unity project.

using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

public class BlenderUnityExporter : EditorWindow
{
    // ---- UI State ----
    private string blenderExePath = "";
    private string inputBlendFolder = "Assets";
    private string outputFolder = "Assets/BlenderExports";
    private ExportFormat exportFormat = ExportFormat.FBX; // or GLTF
    private bool applyModifiers = true;
    private bool triangulate = true;
    private bool bakeAnimations = false;
    private bool useZForwardYUp = true; // Unity FBX convention
    private float scale = 1.0f;
    private bool overwrite = true;
    private bool makeUnityPackage = false;
    private string unityPackagePath = "BlenderExports.unitypackage";
    private string assetLabel = "BlenderExport";

    private Vector2 scroll;

    private enum ExportFormat { FBX, GLTF }

    [MenuItem("Tools/Blender → Unity Exporter")]
    public static void ShowWindow()
    {
        var win = GetWindow<BlenderUnityExporter>("Blender Exporter");
        win.minSize = new Vector2(560, 520);
        win.Show();
    }

    private void OnGUI()
    {
        using (var view = new EditorGUILayout.ScrollViewScope(scroll))
        {
            scroll = view.scrollPosition;
            EditorGUILayout.LabelField("Blender → Unity Exporter", EditorStyles.boldLabel);
            EditorGUILayout.HelpBox("Exportiert .blend-Dateien via Blender nach FBX/glTF.", MessageType.Info);
        }
    }
}
