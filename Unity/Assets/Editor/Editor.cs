using Codice.Client.BaseCommands;
using JetBrains.Annotations;
using Newtonsoft.Json;
using PlasticGui.WorkspaceWindow.PendingChanges.Changelists;
using System;
using System.Collections.Generic;
using System.ComponentModel.Composition.Primitives;
using System.Diagnostics;
using System.Drawing.Printing;
using System.Dynamic;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using Unity.VisualScripting;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.AI;
using UnityEngine.SceneManagement;
using UnityEngine.UIElements;
using Transform = UnityEngine.Transform;
using static MenuItems;
using Unity.AI.Navigation;
using Unity.VisualScripting.YamlDotNet.Core.Tokens;
using Unity.Plastic.Newtonsoft.Json.Linq;
using static Codice.CM.WorkspaceServer.DataStore.WkTree.WriteWorkspaceTree;
using System.Drawing.Drawing2D;
using Microsoft.SqlServer.Server;
using log4net.Core;
using static ExportData;
using NUnit.Framework;
using Codice.CM.Common.Merge;
using UnityEditor.VersionControl;
using System.Runtime;
using System.Xml;

public static class ExportData
{
    public static string binPath = "../KittyEngine/Bin/";
    public static string navmeshPath = "../KittyEngine/Bin/Data/Navmesh";
    public static string start_level_name = "AAAA_MainMenu";

    public static string[] foldersToDelete =
    {
        "Models",
        "Materials",
        "UI"
    };
    public static string[] fileTypeBlacklist =
    {
        ".meta",
        ".cs",
        ".cs.meta",
    };
    public static string[] objectBlackList =
    {
        "Camera",
    };
}

public static class Utility
{


    public static void CompileNavMesh(dynamic export)
    {
        NavMeshSurface[] navMeshSurfaces = GameObject.FindObjectsOfType<NavMeshSurface>(true);
        if (navMeshSurfaces.Length == 0)
        {
            //UnityEngine.Debug.LogWarning("No NavMeshSurfaces found");
            return;
        }
        if (navMeshSurfaces.Length > 1)
        {
            UnityEngine.Debug.LogWarning("More than one NavMeshSurface found");
        }
        for (int i = 0; i < navMeshSurfaces.Length; i++)
        {
            NavMeshTriangulation triangulatedNavMesh = NavMesh.CalculateTriangulation();
            Mesh mesh = new Mesh();

            for (int j = 0; j < triangulatedNavMesh.vertices.Count(); j++)
            {
                Vector3 roundedVertex = triangulatedNavMesh.vertices[j];
                roundedVertex.x = (float)Math.Round((Decimal)roundedVertex.x, 2);
                roundedVertex.y = (float)Math.Round((Decimal)roundedVertex.y, 2);
                roundedVertex.z = (float)Math.Round((Decimal)roundedVertex.z, 2);

                triangulatedNavMesh.vertices[j] = roundedVertex;
            }

            mesh.vertices = triangulatedNavMesh.vertices;
            mesh.triangles = triangulatedNavMesh.indices;

            List<Vector3> newVertices = new List<Vector3>();
            List<int> newIndices = new List<int>();

            int counter = 0;

            Dictionary<Vector3, int> vertices = new Dictionary<Vector3, int>();
            for (int triIndex = 0; triIndex < mesh.triangles.Count(); triIndex++)
            {
                int index = mesh.triangles[triIndex];
                Vector3 vertex = mesh.vertices[index];

                if (vertices.TryAdd(vertex, counter))
                {
                    counter++;
                }
            }

            for (int j = 0; j < vertices.Count(); j++)
            {
                UnityEngine.Debug.Log("Fixed Vertex " + vertices.ElementAt(j));
                newVertices.Add(vertices.ElementAt(j).Key);
            }

            for (int k = 0; k < mesh.triangles.Count(); k++)
            {
                if (k % 3 == 0)
                {
                    int corner0 = vertices[mesh.vertices[mesh.triangles[k]]];
                    int corner1 = vertices[mesh.vertices[mesh.triangles[k + 1]]];
                    int corner2 = vertices[mesh.vertices[mesh.triangles[k + 2]]];

                    if (corner0 == corner1 || corner0 == corner2 || corner1 == corner2)
                    {
                        continue;
                    }

                    newIndices.Add(corner0);
                    newIndices.Add(corner1);
                    newIndices.Add(corner2);
                }
            }

            mesh.Clear();

            Mesh unityMesh = new Mesh();

            unityMesh = mesh;


            for (int vertex = 0; vertex < newVertices.Count; vertex++)
            {
                Vector3 temp = newVertices[vertex];
                temp.z = temp.z * -1;
                newVertices[vertex] = temp;
            }

            mesh.vertices = newVertices.ToArray();
            mesh.triangles = newIndices.ToArray();

            if (mesh == null)
            {
                continue;
            }

            ////Build Representation
            //GameObject gameObject = new GameObject();
            //gameObject.AddComponent<MeshFilter>();
            //gameObject.AddComponent<MeshRenderer>();
            //gameObject.GetComponent<MeshFilter>().mesh = unityMesh;
            //gameObject.GetComponent<MeshRenderer>().sharedMaterial = new Material(Shader.Find("Standard"));
            //gameObject.GetComponent<MeshRenderer>().sharedMaterial.color = new Color(1, 0.5f, 0.5f, 0.5f);
            //gameObject.transform.name = "ExportedNavMesh";
            //gameObject.transform.parent = navMeshSurfaces[i].transform;
            //gameObject.tag = "RemoveOnExport";
            //gameObject.SetActive(false);


            string navName = SceneManager.GetActiveScene().name + " Exported NavMesh.obj";
            string filename = ExportData.navmeshPath + "/" + navName;
            MeshToFileNew(mesh, filename);
            UnityEngine.Debug.Log("NavMesh exported as '" + filename + "'");
            AssetDatabase.Refresh();

            export.navmesh = navName;
        }
    }

    public static string MeshToString(Mesh mesh)
    {
        StringBuilder sb = new StringBuilder();

        sb.Append("g ").Append(mesh.name).Append("\n");
        foreach (Vector3 v in mesh.vertices)
        {
            sb.Append(string.Format("v {0} {1} {2}\n", v.x, v.y, v.z));
        }
        sb.Append("\n");
        foreach (Vector3 v in mesh.normals)
        {
            sb.Append(string.Format("vn {0} {1} {2}\n", v.x, v.y, v.z));
        }
        sb.Append("\n");
        foreach (Vector3 v in mesh.uv)
        {
            sb.Append(string.Format("vt {0} {1}\n", v.x, v.y));
        }
        for (int material = 0; material < mesh.subMeshCount; material++)
        {
            sb.Append("\n");

            int[] triangles = mesh.GetTriangles(material);
            for (int i = 0; i < triangles.Length; i += 3)
            {
                sb.Append(string.Format("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n", triangles[i] + 1, triangles[i + 1] + 1, triangles[i + 2] + 1));
            }
        }
        return sb.ToString();
    }

    public static void MeshToFileNew(Mesh mesh, string filename)
    {
        using (StreamWriter sw = new StreamWriter(filename))
        {
            sw.Write(MeshToString(mesh));
        }
    }

    public static ExpandoObject ExportComponents(GameObject obj)
    {
        dynamic gameObj = new ExpandoObject();
        if (obj.activeSelf == false)
        {
            return gameObj;
        }

        gameObj.id = obj.transform.GetInstanceID();
        gameObj.name = obj.name;
        gameObj.isStatic = obj.isStatic;

        if (obj.layer != 0)
        {
            gameObj.layer = obj.layer;
        }

        int parentID = int.MaxValue;
        if (obj.transform.parent != null)
        {
            parentID = obj.transform.parent.GetInstanceID();
        }
        gameObj.parentID = parentID;

        if (obj.GetComponent<PostponeInstantiation>())
        {
            gameObj.postponeInstantiation = true;
        }

        //Transform
        {
            var component = obj.GetComponent<Transform>();
            if (component)
            {
                Matrix4x4 matrix = new Matrix4x4();
                Quaternion rotation = component.localRotation;
                Vector3 pos = component.localPosition;
                Vector3 scale = component.localScale;


                matrix = Matrix4x4.TRS(pos, rotation, scale);
                matrix = matrix.transpose;

                gameObj.transform = new
                {
                    m00 = matrix.m00,
                    m01 = matrix.m01,
                    m02 = matrix.m02,
                    m03 = matrix.m03,
                    m10 = matrix.m10,
                    m11 = matrix.m11,
                    m12 = matrix.m12,
                    m13 = matrix.m13,
                    m20 = matrix.m20,
                    m21 = matrix.m21,
                    m22 = matrix.m22,
                    m23 = matrix.m23,
                    m30 = matrix.m30,
                    m31 = matrix.m31,
                    m32 = matrix.m32,
                    m33 = matrix.m33
                };
            }
        }

        dynamic components = new ExpandoObject();
        gameObj.components = components;

        //Model
        {
            var component = obj.GetComponent<MeshFilter>();
            if (component)
            {
                ExtractModelWithMaterial(obj, components, component);
                //UnityEngine.Debug.LogError(obj.name.ToString());
            }
        }

        //Skeleton model
        {
            var component = obj.GetComponent<SkinnedMeshRenderer>();
            if (component)
            {
                ExtractSkeletalModelWithMaterial(obj, components, component);
            }
        }

        //Render Layer
        {
            var component = obj.GetComponent<RenderLayer>();
            if (component)
            {
                components.renderLayer = new { layer = (int)component.layerToRenderOn };
            }
        }

        // VFX
        {
            var component = obj.GetComponent<KittyVFX>();
            if (component)
            {
                if (component.VFXNames.Count > 0)
                {
                    components.vfx = new
                    {
                        sequence_names = component.VFXNames,
                        autoplay = component.autoplay
                    };
                }
            }
        }

        // Camera
        {
            var component = obj.GetComponent<Camera>();
            if (component)
            {
                dynamic rot = new ExpandoObject();

                rot = new
                {
                    x = obj.transform.localEulerAngles.x,
                    y = obj.transform.localEulerAngles.y - 180,
                    z = obj.transform.localEulerAngles.z
                };


                float FOV = Camera.VerticalToHorizontalFieldOfView(component.fieldOfView, component.aspect);

                components.main_camera = new { fov = FOV };
            }
        }

        //Spot Light
        {
            var component = obj.GetComponent<Light>();
            if (component && (int)component.type == 0)
            {
                components.spot_light = new
                {
                    range = component.range,
                    intensity = component.intensity,
                    color = new { r = component.color.r, g = component.color.g, b = component.color.b },
                    innerAngle = component.innerSpotAngle,
                    outerAngle = component.spotAngle
                };
            }
        }

        //Directional Light
        {
            var component = obj.GetComponent<Light>();
            if (component && (int)component.type == 1)
            {
                components.directional_light = new
                {
                    range = component.range,
                    intensity = component.intensity,
                    color = new { r = component.color.r, g = component.color.g, b = component.color.b },
                };
            }
        }

        //Point Light
        {
            var component = obj.GetComponent<Light>();
            if (component && (int)component.type == 2)
            {
                components.point_light = new
                {
                    range = component.range,
                    intensity = component.intensity,
                    color = new { r = component.color.r, g = component.color.g, b = component.color.b },
                };
            }
        }

        {
            if (obj.GetComponent<BallComponent>())
            {
                components.boomerang = new { };
            }
        }

        //Box collider
        {
            var component = obj.GetComponent<BoxCollider>();
            if (component)
            {
                //Matrix4x4 worldMatrix = obj.transform.localToWorldMatrix;

                //Matrix4x4 matrix = new Matrix4x4();

                //Vector3 pos = obj.transform.position;// + component.center;

                //Quaternion rotation = obj.transform.rotation;
                //Vector3 scl = obj.transform.lossyScale;


                //matrix = Matrix4x4.TRS(pos, rotation, scl);
                //matrix = matrix.transpose;

                Matrix4x4 origMatrix = component.gameObject.transform.localToWorldMatrix;
                Matrix4x4 matrix = origMatrix.transpose;
                //matrix = origMatrix.transpose;

               
                Matrix4x4 accumulatedMatrix = new Matrix4x4();
                accumulatedMatrix *= obj.transform.localToWorldMatrix;

                // Physx needs the world transform for static box colliders to be set at import
                dynamic worMat = new ExpandoObject();
                {
                    worMat.m00 = matrix.m00;
                    worMat.m01 = matrix.m01;
                    worMat.m02 = matrix.m02;
                    worMat.m03 = matrix.m03;
                    worMat.m10 = matrix.m10;
                    worMat.m11 = matrix.m11;
                    worMat.m12 = matrix.m12;
                    worMat.m13 = matrix.m13;
                    worMat.m20 = matrix.m20;
                    worMat.m21 = matrix.m21;
                    worMat.m22 = matrix.m22;
                    worMat.m23 = matrix.m23;
                    worMat.m30 = matrix.m30;
                    worMat.m31 = matrix.m31;
                    worMat.m32 = matrix.m32;
                    worMat.m33 = matrix.m33;
                };

                dynamic size = new ExpandoObject();
                size.x = component.size.x;
                size.y = component.size.y;
                size.z = component.size.z;

                dynamic center = new ExpandoObject();
                center.x = component.center.x;
                center.y = component.center.y;
                center.z = component.center.z;

                components.box_collider = new { trigger = component.isTrigger, size = size, offset = center, world_transform = worMat };
            }
        }

        //Sphere collider
        {
            var component = obj.GetComponent<SphereCollider>();
            if (component)
            {
                dynamic center = new ExpandoObject();
                center.x = component.center.x;
                center.y = component.center.y;
                center.z = component.center.z;

                components.sphere_collider = new
                { trigger = component.isTrigger, radius = component.radius, offset = center };
            }
        }

        // Capsule collider
        {
            var component = obj.GetComponent<CapsuleCollider>();
            if (component)
            {
                dynamic center = new ExpandoObject();
                center.x = component.center.x;
                center.y = component.center.y;
                center.z = component.center.z;

                components.capsule_collider = new
                {
                    trigger = component.isTrigger,
                    radius = component.radius,
                    length = component.height,
                    offset = center
                };
            }
        }

        // Mesh collider
        {
            var component = obj.GetComponent<MeshCollider>();
            if (component)
            {
                dynamic size = new ExpandoObject();
                size.path = AssetDatabase.GetAssetPath(component.sharedMesh).TrimStart("Assets/Kitty Assets/".ToCharArray());
                components.mesh_collider = new
                {
                    mesh = size.path
                    //mesh = "Models/" + component.sharedMesh.name
                };
            }
        }

        //Script
        {
            var component = obj.GetComponent<NodeScript>();
            if (component)
            {
                components.script = new
                {
                    scriptName = component.scriptName
                };
            }
        }

        // SFXX
        {
            var component = obj.GetComponent<AudioComponent>();
            if (component)
            {
                components.audio = new
                {
                    fileName = component.soundFileName,
                    shouldLoop = component.shouldLoop,
                    shouldPerformSpatialPlayback = component.shouldPerformSpatialPlayback
                };
            }
        }

        // Player
        {
            var component = obj.GetComponent<Player>();
            if (component)
            {
                components.player = new
                {
                    maxSpeed = component.maxSpeed,
                    movementSpeed = component.moveSpeed,
                    maxRotation = component.maxRotation,
                    rotationSpeed = component.rotationSpeed,
                    spawnPosition = new
                    {
                        x = component.spawnPosition.position.x,
                        y = component.spawnPosition.position.y,
                        z = component.spawnPosition.position.z
                    },
                    characterIndex = (int)component.character
                };
            }
        }

        // Moving Object
        {
            var component = obj.GetComponent<MovingObject>();
            if (component)
            {
                components.movingObject = new
                {
                    travelTime = component.travelTime,
                    endPosition = new
                    {
                        component.endTransform.position.x,
                        component.endTransform.position.y,
                        component.endTransform.position.z
                    }
                };
            }
        }

        // Override Material
        {
            var component = obj.GetComponent<OverrideMaterial>();
            if (component)
            {
                components.overrideMaterial = new
                {
                    materialName = component.material.name,
                    albedo = new
                    {
                        r = component.material.color.r,
                        g = component.material.color.g,
                        b = component.material.color.b
                    },
                    metallic = component.material.GetFloat("_Metallic"),
                    roughness = 1.0f - component.material.GetFloat("_Glossiness"),
                    emission = component.material.IsKeywordEnabled("_EMISSION") ? 1.0f : 0.0f,                    

                    albedoTex = component.material.GetTexture("_MainTex") ? AssetDatabase.GetAssetPath(component.material.GetTexture("_MainTex")).TrimStart("Assets/Kitty Assets/".ToCharArray()) : "",
                };
            }
        }

        // Override Camera
        {
            var component = obj.GetComponent<CameraOverride>();
            if (component)
            {
                components.cameraOverride = new
                {
                    aspectRatio = component.aspectRatio,
                    projectionType = component.projectionType,
                    FOV = component.FOV,
                    ortographicWidth = component.ortographicWidth,
                    nearPlane = component.nearPlane,
                    farPlane = component.farPlane,
                    disableActionCamera = component.disableActionCamera
                };
            }
        }


        // Moving Object Trigger
        {
            var component = obj.GetComponent<MovingObjectTrigger>();
            if (component)
            {
                components.movingObjectTrigger = new
                {
                    movingObjects = component.movingObjects.Select(mo => mo.gameObject.transform.GetInstanceID()).ToList()
                };
            }
        }

        
        // Powerup Pickup
        {
            var component = obj.GetComponent<Powerup>();
            if (component)
            {
                components.powerupPickup = new
                {

                };
            }
        }

        // Powerup Area
        {
            var component = obj.GetComponent<PowerupArea>();
            if (component)
            {
                Vector3 minPos = component.transform.position - component.transform.localScale / 2;
                Vector3 maxPos = component.transform.position + component.transform.localScale / 2;

                components.powerupArea = new
                {
                    min = new
                    {
                        x = minPos.x,
                        y = minPos.y,
                        z = minPos.z
                    },

                    max = new
                    {
                        x = maxPos.x,
                        y = maxPos.y,
                        z = maxPos.z
                    }
                };
            }
        }

        // Portal
        {
            var component = obj.GetComponent<Portal>();
            if (component)
            {
                components.portal = new
                {
                    linkedPortal = component.linkedPortal.transform.GetInstanceID()
                };
            }
        }

        // Spawn Point
        {
            var component = obj.GetComponent<SpawnPoint>();
            if (component)
            {
                components.spawnPoint = new
                {
                    index = component.index

                };
            }
        }

        //Water Plane
        {
            var component = obj.GetComponent<WaterPlane>();
            if (component)
            {
                components.waterPlane = new
                {
                    fogColour = new
                    {
                        r = component.waterFogColour.r,
                        g = component.waterFogColour.g,
                        b = component.waterFogColour.b
                    },
                    fogDensity = component.waterFogDensity,

                    causticColour = new
                    {
                        r = component.causticColour.r,
                        g = component.causticColour.g,
                        b = component.causticColour.b
                    },

                    causticStrength = component.causticStrength,
                    
                    foamColour = new
                    {
                        r = component.waterFoamColour.r,
                        g = component.waterFoamColour.g,
                        b = component.waterFoamColour.b
                    },

                    foamStrength = component.waterFoamStrength
                    
                };
            }
        }

        //Light Settings
        {
            var component = obj.GetComponent<LightSettings>();
            if (component)
            {
                components.lightSettings = new
                {
                    ambientIntensity = component.ambientIntensity
                };
            }
        }


        // Shell Texturing
        {
            var component = obj.GetComponent<ShellTexturing>();
            if (component)
            {
                components.shellTexturing = new
                {
                    mesh = AssetDatabase.GetAssetPath(component.mesh).TrimStart("Assets/Kitty Assets/".ToCharArray()),
                    totalHeight = component.totalHeight,
                    shellCount = component.shellCount,
                    thickness = component.thickness,
                    density = component.density,
                    noiseMin = component.noiseMin,
                    noiseMax = component.noiseMax,
                    aoExp = component.aoExp,
                    bottomColour = new
                    {
                        r = component.bottomColour.r,
                        g = component.bottomColour.g,
                        b = component.bottomColour.b
                    },
                    topColour = new
                    {
                        r = component.topColour.r,
                        g = component.topColour.g,
                        b = component.topColour.b
                    },
                    liveUpdate = component.liveUpdate
                };
            }
        }

        // Battle Area
        {
            var component = obj.GetComponent<BattleArea>();
            if (component)
            {
                components.battleArea = new
                {
                    shrink = component.shrink,
                    shrinkDelay = component.shrinkDelay,
                    shrinkDuration = component.shrinkDuration


                    //// This is in world space
                    //minbounds = new
                    //{
                    //    x = component.Min.x,
                    //    y = component.Min.y,
                    //    z = component.Min.z
                    //},
                    //
                    //maxbounds = new
                    //{
                    //    x = component.Max.x,
                    //    y = component.Max.y,
                    //    z = component.Max.z
                    //},
                    //
                    //center = new
                    //{
                    //    x = component.gameObject.transform.position.x,
                    //    y = component.gameObject.transform.position.y,
                    //    z = component.gameObject.transform.position.z
                    //}
                };
            }
        }

        // Battle Area
        {
            var component = obj.GetComponent<CameraAngleOverride>();
            if (component)
            {
                components.cameraAngleOverride = new
                {
                    isStatic = component.isStatic,
                    position = new
                    {
                        x = obj.transform.position.x,
                        y = obj.transform.position.y,
                        z = obj.transform.position.z
                    },
                    direction = new
                    {
                        x = obj.transform.localEulerAngles.x,
                        y = obj.transform.localEulerAngles.y,
                        z = obj.transform.localEulerAngles.z
                    },
                    fov = Camera.VerticalToHorizontalFieldOfView(Camera.main.fieldOfView, Camera.main.aspect)
                };
            }
        }

        return gameObj;
    }

    public static void ExportScene(Scene aScene)
    {
        string levelsJsonPath = ExportData.binPath + "Data/Settings/LevelSettings.json";
        string levelPath = ExportData.binPath + "Data/Levels/" + aScene.name + ".json";

        UnityEngine.Debug.ClearDeveloperConsole();
        var total_timer = new System.Diagnostics.Stopwatch();
        total_timer.Start();

        if (!File.Exists(levelsJsonPath))
        {
            System.IO.StreamWriter levelsJsonFile = new System.IO.StreamWriter(levelsJsonPath);
            levelsJsonFile.Write("");
            levelsJsonFile.Close();
        }

        using (System.IO.StreamWriter file = new System.IO.StreamWriter(levelPath))
        {
            GameObject[] allObjects = aScene.GetRootGameObjects();
            List<GameObject> allObjectsList = new List<GameObject>();

            foreach (var obj in allObjects)
            {
                if (obj.transform.tag == "Navmesh") continue;
                if (obj.transform.tag == "EditorOnly") continue;

                List<GameObject> tempList = ChildrenCheck(obj);
                foreach (var gObj in tempList)
                {
                    allObjectsList.Add(gObj);
                }
            }

            file.Write("");

            dynamic export = new ExpandoObject();
            export.scene = aScene.name;
            export.game_objects = new List<dynamic>();


            if (!Directory.Exists(ExportData.navmeshPath))
            {
                Directory.CreateDirectory(ExportData.navmeshPath);
            }

            Utility.CompileNavMesh(export);

            //All objects
            foreach (var obj in allObjectsList)
            {
                if (obj.activeSelf == false)
                {
                    continue;
                }

                export.game_objects.Add(Utility.ExportComponents(obj));
            }

            string json = JsonConvert.SerializeObject(export, Newtonsoft.Json.Formatting.Indented);
            file.WriteLine(json);

            string levelSettings = File.ReadAllText(levelsJsonPath);
            string newLevelName = SceneManager.GetActiveScene().name;

            if (string.IsNullOrEmpty(levelSettings))
            {
                levelSettings = "{ \"start_level_name\": \"AAAA_MainMenu\"}";
            }

            // Update the start_level_name with the new scene's name
            levelSettings = levelSettings.Replace("\"start_level_name\": \"AAAA_MainMenu\"", $"\"start_level_name\": \"{aScene.name}\"");
            File.WriteAllText(levelsJsonPath, levelSettings);
        }

        total_timer.Stop();
        UnityEngine.Debug.Log("Exported scene: '" + aScene.name + "' in " + total_timer.ElapsedMilliseconds + "ms");
    }
}



public class MenuItems
{
    [MenuItem("Tools/Export scene #_c")]
    public static void SceneExport()
    {
        Utility.ExportScene(SceneManager.GetActiveScene());
        MarkExportDone();
    }

    [MenuItem("Tools/Export all assets")]
    public static void AssetsExport()
    {
        foreach (string folder in ExportData.foldersToDelete)
        {
            if (Directory.Exists(ExportData.binPath + "Data/Assets/" + folder))
            {
                // Clear folder
                string[] files = Directory.GetFiles(ExportData.binPath + "Data/Assets/" + folder + "/", "*.*", SearchOption.AllDirectories);
                foreach (string file in files)
                {
                    File.SetAttributes(file, FileAttributes.Normal);
                    File.Delete(file);
                }

                // Delete folder
                Directory.Delete(ExportData.binPath + "Data/Assets/" + folder, true);
            }
        }

        string[] assets = Directory.GetFiles("Assets/Kitty Assets", "*.*", SearchOption.AllDirectories);

        foreach (string asset in assets)
        {
            bool blacklisted = false;
            foreach (string ext in ExportData.fileTypeBlacklist)
            {
                if (asset.EndsWith(ext))
                {
                    blacklisted = true;
                    break;
                }
            }

            if (blacklisted)
                continue;

            string dest = asset.Replace("Assets/Kitty Assets", ExportData.binPath + "Data/Assets/");
            Directory.CreateDirectory(Path.GetDirectoryName(dest));
            File.SetAttributes(asset, FileAttributes.Normal);
            
            try
            {
                File.Copy(asset, dest, true);
            }
            catch (Exception e) {}
        }

        UnityEngine.Debug.Log("Asset export complete");
    }

    [MenuItem("Tools/Export assets and scene #_x")]
    public static void ExportAll()
    {
        UnityEngine.Debug.ClearDeveloperConsole();
        var total_timer = new System.Diagnostics.Stopwatch();
        total_timer.Start();
        SceneExport();
        AssetsExport();
        total_timer.Stop();
        UnityEngine.Debug.Log("Exported assets and scene: '" + SceneManager.GetActiveScene().name + "' in " + total_timer.ElapsedMilliseconds + "ms");

        MarkExportDone();
    }

    [MenuItem("Tools/Run Bat file")]
    public static void RunBat()
    {
        string path = Application.dataPath;
        string subPath = "Unity/Assets";

        path = path.Replace(subPath, "");

        string batDirectory = path + "KittyEngine/";
        string batPath = batDirectory + "generate_project.bat";

        if (File.Exists(batPath))
        {
            Process batProcess = new Process();

            batProcess.StartInfo.FileName = batPath;
            batProcess.StartInfo.WorkingDirectory = batDirectory;
            batProcess.Start();
            return;
        }

        UnityEngine.Debug.LogWarning(".Bat File doesn't excist!");
    }

    [MenuItem("Tools/Export ALL scenes")]
    public static void ExportAllScenes()
    {
        string path = "Assets/Scenes/Pontus_level/MAIN_Levels/";

        Scene currentActiveScene = SceneManager.GetActiveScene();

        var info = new DirectoryInfo(path);
        var fileInfo = info.GetFiles();

        string currentSceneName = new string("");

        try
        {
            var total_timer = new System.Diagnostics.Stopwatch();
            total_timer.Start();
            int counter = 0;

            foreach (var scene in fileInfo)
            {
                if(scene.Extension == ".unity")
                {
                    currentSceneName = scene.Name;
                    string levelPath = path + scene.Name;

                    UnityEngine.Debug.Log("Trying to export " + scene.Name);

                    EditorSceneManager.OpenScene(levelPath, OpenSceneMode.Additive);

                    Scene newScene = EditorSceneManager.GetSceneByPath(levelPath);

                    Utility.ExportScene(newScene);

                    EditorSceneManager.CloseScene(newScene, true);

                    counter++;
                }
            }
            total_timer.Stop();

            UnityEngine.Debug.Log($"Finished exporting ALL {counter} levels - in {total_timer.ElapsedMilliseconds} ms");
        }
        catch (Exception e) 
        {
            ShowPopup popup = new ShowPopup();
            //popup.message = e.Message;
            Label label1 = new Label("Error trying to export scene '" + currentSceneName + "': \n");
            popup.myLabels.Add(label1);

            Label label2 = new Label(e.Message + "\n\n");
            label2.style.color = Color.red;
            popup.myLabels.Add(label2);
            popup.Show();
        }
    }

    static public List<GameObject> ChildrenCheck(GameObject aGameObject)
    {
        List<GameObject> list = new List<GameObject>();
        int childCount = aGameObject.transform.childCount;
        if (childCount > 0)
        {
            if (aGameObject.transform.CompareTag("EditorOnly"))
            {
                return list;
            }

            list.Add(aGameObject);
            for (int i = 0; i < childCount; i++)
            {
                if (aGameObject.transform.GetChild(i).gameObject.transform.childCount > 0)
                {
                    List<GameObject> secondList = new List<GameObject>();
                    secondList = ChildrenCheck(aGameObject.transform.GetChild(i).gameObject);
                    foreach (var secondObj in secondList)
                    {
                        list.Add(secondObj);
                    }
                }
                else
                {
                    list.Add(aGameObject.transform.GetChild(i).gameObject);
                }
            }
        }
        else
        {
            list.Add(aGameObject);
        }

        return list;
    }

    public static void ExtractModelWithMaterial(GameObject obj, dynamic components, MeshFilter component)
    {
        dynamic size = new ExpandoObject();
        size.path = AssetDatabase.GetAssetPath(component.sharedMesh).TrimStart("Assets/Kitty Assets/".ToCharArray());

        components.model = new
        {
            path = size.path,
        };
    }

    public static void ExtractSkeletalModelWithMaterial(GameObject obj, dynamic components, SkinnedMeshRenderer component)
    {
        dynamic size = new ExpandoObject();
        size.path = AssetDatabase.GetAssetPath(component.sharedMesh).TrimStart("Assets/Kitty Assets/".ToCharArray());
        size.animationPaths = new List<string>();

        // Add animation information if available
        List<string> animationPaths = GetAnimationPaths(obj, AssetDatabase.GetAssetPath(component.sharedMesh));
        if (animationPaths.Count > 0)
        {
            size.animationPaths = animationPaths;
        }

        components.skeletal_model = new
        {
            path = size.path,
            animationPaths = size.animationPaths
        };
    }

    public static List<string> GetAnimationPaths(GameObject obj, string modelPath)
    {
        string modelDirectory = Path.GetDirectoryName(modelPath);

        // Search for all files in the same directory with the "anim_" prefix & Exclude files with ".meta" to the end
        string[] animationPaths = Directory.GetFiles(modelDirectory, "anim_*")
        .Where(path => path.EndsWith(".fbx")).ToArray();

        List<string> paths = new List<string>();
        foreach (var animationPath in animationPaths)
        {
            string relativePath = animationPath.Replace("\\", "/").Replace("Assets/Kitty Assets/", "");
            paths.Add(relativePath);
        }

        return paths;
    }

    public static void MarkExportDone()
    {
        string exportDonePath = ExportData.binPath + "Data/export.done";
        if (!File.Exists(exportDonePath))
        {
            System.IO.StreamWriter exportDoneFile = new System.IO.StreamWriter(exportDonePath);
            exportDoneFile.Write("");
            exportDoneFile.Close();
        }
        else
        {
            File.WriteAllText(exportDonePath, string.Empty);
        }
    }

}

public class ShowPopup : EditorWindow
{
    //static public List<string> myMessages;
    public string message;

    public List<Label> myLabels = new List<Label>();

    static public void Init()
    {
        ShowPopup window = ScriptableObject.CreateInstance<ShowPopup>();
        window.position = new Rect(Screen.width / 2, Screen.height / 2, 400, 100);
        window.ShowPopup();
    }

    void CreateGUI()
    {
        var label = new Label("SOMETHING WENT WRONG, CHECK CONSOLE FOR ERROR MESSAGES\n");
        rootVisualElement.Add(label);

        for (int i = 0; i < myLabels.Count; i++)
        {
            rootVisualElement.Add(myLabels[i]);
        }

        var button = new Button();
        button.text = "I HAVE READ, CLOSE ME PLZ";
        button.clicked += () => this.Close();
        rootVisualElement.Add(button);
    }
}


[InitializeOnLoadAttribute]
public static class HijackPlayButton
{
    static HijackPlayButton()
    {
        EditorApplication.playModeStateChanged += LogPlayModeState;
    }

    private static void LogPlayModeState(PlayModeStateChange state)
    {
        if (state == PlayModeStateChange.ExitingEditMode)
        {
            EditorApplication.ExitPlaymode();
            ExportAll();
            string path = Application.dataPath;
            string subPath = "Unity/Assets";

            path = path.Replace(subPath, "");


            path += "KittyEngine/Bin/";
            string exePath = path + "Project_Release - Hybrid.exe";
            if (!File.Exists(exePath))
            {
                UnityEngine.Debug.LogError("Release build does not exist!" + " Tried to access: " + exePath);
                EditorApplication.Beep();
            }
            else
            {



                Process process = new Process();
                process.StartInfo.FileName = exePath;
                process.StartInfo.WorkingDirectory = path;
                process.Start();
            }
        }
    }
}

[ExecuteInEditMode]
public class PrefabExporter : EditorWindow
{
    [SerializeField] private GameObject prefab;

    public UnityEngine.Object source;

    [MenuItem("Tools/Prefab Exporter")]
    static void Init()
    {
        UnityEditor.EditorWindow window = GetWindow(typeof(PrefabExporter));
        window.Show();
    }

    public void OnGUI()
    {
        string path = binPath + "Data/Prefabs/";

        EditorGUILayout.BeginVertical();
        EditorGUILayout.LabelField("Select Prefab");
        GUILayout.Space(10);

        source = EditorGUILayout.ObjectField(source, typeof(UnityEngine.Object), true);

        GUILayout.FlexibleSpace();

        if(GUILayout.Button("Save", GUILayout.Height(30)))
        {
            GameObject gameObj = (GameObject)source;
            Directory.CreateDirectory(path);

            path += "pf_" + gameObj.name + ".json";

            dynamic export = Utility.ExportComponents(gameObj);

            string fileString = JsonConvert.SerializeObject(export, Newtonsoft.Json.Formatting.Indented);


            System.IO.StreamWriter prefabFile = new System.IO.StreamWriter(path);
            prefabFile.WriteLine("");
            prefabFile.WriteLine(fileString);
            prefabFile.Close();


            UnityEngine.Debug.Log($"Saved prefab to '{Path.GetFullPath(path)}'");
        }

        EditorGUILayout.EndVertical();
    }
}