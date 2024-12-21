using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    public enum PlayerCharacter
    {
        Player01 = 1,
        Player02 = 2,
        Player03 = 3,
        Player04 = 4
    }

    public Transform spawnPosition;
    public float maxSpeed = 5f;
    public float moveSpeed = 55.0f;
    public float maxRotation = 7.5f;
    public float rotationSpeed = 44.0f;

    public PlayerCharacter character = PlayerCharacter.Player01 ;

    // Create a onDrawGizmos and draw a green sphere at player location
    private void OnDrawGizmos()
    {
        Gizmos.color = Color.green;
        //Gizmos.DrawSphere(transform.position, 0.3f);
        
        // Draw a wired mesh based on the mesh in the objects skinned mesh renderer.
        SkinnedMeshRenderer skinnedMeshRenderer = GetComponentInChildren<SkinnedMeshRenderer>();
        if (skinnedMeshRenderer != null)
        {
            Gizmos.color = Color.green;
            Gizmos.DrawWireMesh(skinnedMeshRenderer.sharedMesh, transform.position, transform.rotation, transform.localScale);
        }
    }
}
