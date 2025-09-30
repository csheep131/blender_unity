// MIT License
// Unity 2020.3+
// Importiert .umesh (v1) gemäß unserem C-Exporter (Header 'UMSH', version=1).

using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

#if UNITY_2020_3_OR_NEWER
using UnityEditor.AssetImporters;
#else
using UnityEditor.Experimental.AssetImporters;
#endif

[ScriptedImporter(1, "umesh")]
public class UMeshImporter : ScriptedImporter
{
    public override void OnImportAsset(AssetImportContext ctx)
    {
        Debug.Log($"[UMeshImporter] Import {ctx.assetPath}");
        var go = new GameObject(Path.GetFileNameWithoutExtension(ctx.assetPath));
        ctx.AddObjectToAsset("root", go);
        ctx.SetMainObject(go);
    }
}
