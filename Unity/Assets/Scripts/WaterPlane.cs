using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WaterPlane : MonoBehaviour
{
    [SerializeField] public Color waterFogColour = new Color(0.0f, 0.25f, 1.0f, 1.0f);
    [SerializeField] public float waterFogDensity = 0.1f;
    [SerializeField] public Color causticColour = new Color( 0.8f, 0.8f, 1.0f, 1.0f);
    [SerializeField] public float causticStrength = 1.0f;
    [SerializeField] public Color waterFoamColour = new Color(1.0f, 1.0f, 1.0f, 1.0f);
    [SerializeField] public float waterFoamStrength = 1.0f;


    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void OnDrawGizmos()
    {
        float size = 100.0f;
     

        Vector3 min = new Vector3(-size / 2, 0.0f, -size / 2);
        Vector3 max = new Vector3(size / 2, 0.0f, size / 2);

        Vector3 topLeft = transform.TransformPoint(min);
        Vector3 bottomRight = transform.TransformPoint(max);
        Vector3 topRight = transform.TransformPoint(new Vector3(max.x, 0.0f, min.z));
        Vector3 bottomLeft = transform.TransformPoint(new Vector3(min.x, 0.0f, max.z));

        Gizmos.color = Color.red;
        Gizmos.DrawLine(topLeft, topRight);
        Gizmos.DrawLine(topRight, bottomRight);
        Gizmos.DrawLine(bottomRight, bottomLeft);
        Gizmos.DrawLine(bottomLeft, topLeft);
    }

}
