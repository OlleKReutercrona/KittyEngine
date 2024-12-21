using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpawnPoint : MonoBehaviour
{
    public int index = -1;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    //draw a circle in the scene view to help visualize the spawn point
    void OnDrawGizmos()
    {
        Gizmos.color = Color.green;
        Gizmos.DrawWireSphere(transform.position, 1.0f);

        Vector3 pos = transform.position;
        Vector3 forward = transform.forward;

        Gizmos.DrawLine(pos, pos + forward*2);
    }
}
