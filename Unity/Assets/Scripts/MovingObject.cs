using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovingObject : MonoBehaviour
{
    private Transform startTransform;
    public Transform endTransform;
    public float travelTime = 1.0f;

    void Start()
    {
        startTransform = transform;
    }

    private void OnDrawGizmos()
    {
        // If endTransform is not assigned, return
        if (endTransform == null)
            return;

        // Set the color of the Gizmo lines
        Gizmos.color = Color.red;

        // Draw a line between startTransform and endTransform
        Gizmos.DrawLine(transform.position, endTransform.position);

        // Set the color of the Gizmo sphere
        Gizmos.color = Color.red;

        // Draw a sphere at the position of endTransform
        Gizmos.DrawSphere(endTransform.position, 0.3f);
    }

}
