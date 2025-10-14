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

            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Einstellungen", EditorStyles.boldLabel);
            
            blenderExePath = EditorGUILayout.TextField("Blender Pfad", blenderExePath);
            inputBlendFolder = EditorGUILayout.TextField("Input Ordner", inputBlendFolder);
            outputFolder = EditorGUILayout.TextField("Output Ordner", outputFolder);
            
            exportFormat = (ExportFormat)EditorGUILayout.EnumPopup("Export Format", exportFormat);
            applyModifiers = EditorGUILayout.Toggle("Modifiers anwenden", applyModifiers);
            triangulate = EditorGUILayout.Toggle("Triangulieren", triangulate);
            bakeAnimations = EditorGUILayout.Toggle("Animationen backen", bakeAnimations);
            useZForwardYUp = EditorGUILayout.Toggle("Z Forward Y Up", useZForwardYUp);
            scale = EditorGUILayout.FloatField("Skalierung", scale);
            overwrite = EditorGUILayout.Toggle("Überschreiben", overwrite);
            
            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Unity Package", EditorStyles.boldLabel);
            makeUnityPackage = EditorGUILayout.Toggle("Unity Package erstellen", makeUnityPackage);
            if (makeUnityPackage)
            {
                unityPackagePath = EditorGUILayout.TextField("Package Pfad", unityPackagePath);
                assetLabel = EditorGUILayout.TextField("Asset Label", assetLabel);
            }

            EditorGUILayout.Space();
            
            // Zeige ALLE gefundenen .blend-Dateien an (kein Limit!)
            if (GUILayout.Button("Zeige .blend Dateien"))
            {
                ShowAllBlendFiles();
            }
        }
    }

    private void ShowAllBlendFiles()
    {
        if (!Directory.Exists(inputBlendFolder))
        {
            EditorUtility.DisplayDialog("Fehler", $"Ordner nicht gefunden: {inputBlendFolder}", "OK");
            return;
        }

        var blendFiles = Directory.GetFiles(inputBlendFolder, "*.blend", SearchOption.AllDirectories);
        
        StringBuilder sb = new StringBuilder();
        sb.AppendLine($"Gefundene .blend-Dateien: {blendFiles.Length}");
        sb.AppendLine();
        
        // ALLE Dateien anzeigen - KEIN LIMIT!
        foreach (var file in blendFiles)
        {
            sb.AppendLine(file);
        }
        
        EditorUtility.DisplayDialog("Alle .blend-Dateien", sb.ToString(), "OK");
    }
}
