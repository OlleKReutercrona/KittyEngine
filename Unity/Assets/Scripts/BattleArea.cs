using System.Collections;
using System.Collections.Generic;
using System.IO.Compression;
using UnityEditorInternal;
using UnityEngine;
using UnityEngine.UIElements;

public class BattleArea : MonoBehaviour
{
    public bool shrink = false;
    public float shrinkDelay = 15.0f;
    public float shrinkDuration = 15.0f;

    private void Update()
    {

    }

    private void OnDrawGizmos()
    {
        float size = 1.0f;

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

        Gizmos.DrawLine(topLeft, bottomRight);
        Gizmos.DrawLine(bottomLeft, topRight);


        Vector3 testPos = new Vector3(0.0f, 0.0f, 0.0f);
        Vector3 transformed = transform.localToWorldMatrix.inverse.MultiplyPoint3x4(testPos);

        if (
            transformed.x > 0.5f || transformed.x < -0.5f ||
            transformed.z > 0.5f || transformed.z < -0.5f            
            )
        {
               Gizmos.color = Color.red;
        }
        else
        {
            Gizmos.color = Color.green;
        }
        Gizmos.DrawSphere(testPos, 0.2f);
    }
}
