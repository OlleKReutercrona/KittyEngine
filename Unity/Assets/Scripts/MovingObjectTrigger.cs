using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovingObjectTrigger : MonoBehaviour
{
    public List<MovingObject> movingObjects = new List<MovingObject>();

    void Start()
    {
       
    }

    private void OnDrawGizmos()
    {
        foreach (MovingObject movingObject in movingObjects)
        {
            if (movingObject == null) { continue; }

            Gizmos.color = Color.green;
            Gizmos.DrawLine(transform.position, movingObject.transform.position);

            //Gizmos.color = Color.red;
            //Gizmos.DrawSphere(movingObject.transform.position, 0.3f);
        }
    }

}
