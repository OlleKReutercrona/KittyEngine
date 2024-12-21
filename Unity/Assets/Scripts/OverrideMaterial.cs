using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class OverrideMaterial : MonoBehaviour
{
    // Start is called before the first frame update
    [SerializeField] public Material material;

    void Start()
    {
        if (!material)
        {
            material = new Material(Shader.Find("Standard"));
        }
        Renderer renderer = GetComponent<Renderer>();
        if (!renderer) return;
        renderer.material = material;

    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void OnValidate()
    {
        if (!material)
        {
            material = new Material(Shader.Find("Standard"));
        }
        Renderer renderer = GetComponent<Renderer>();
        if (!renderer) return;

        renderer.material = material;
    }
}
