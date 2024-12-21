using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ShellTexturing : MonoBehaviour
{
    // Start is called before the first frame update
    [SerializeField] public Mesh mesh;

    [SerializeField] public float totalHeight = 0.5f;
    [SerializeField] public int shellCount = 32;
    [SerializeField] public float thickness = 2.0f;
    [SerializeField] public float density = 512.0f;

    [SerializeField] public float noiseMin = 0.0f;
    [SerializeField] public float noiseMax = 1.0f;
    [SerializeField] public float aoExp = 0.0f;

    [SerializeField] public Color bottomColour;
    [SerializeField] public Color topColour;
    [SerializeField] public bool liveUpdate = true;
    //

    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
