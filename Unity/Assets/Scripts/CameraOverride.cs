using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraOverride : MonoBehaviour
{
    public enum Type
    {
        Perspective,
        Orthographic
    };

    public enum AspectRatio
    {
        _16_9,
        _4_3,
        Free
    }

    public AspectRatio aspectRatio;
    public Type projectionType;
    public float FOV = 60.0f;
    public float ortographicWidth = 5.0f;

    public float nearPlane = 0.3f;
    public float farPlane = 1000.0f;

    public bool disableActionCamera = true;

    private void OnValidate()
    {
        Camera.allCameras[0].ResetAspect();
        Camera.main.aspect = aspectRatio == AspectRatio._16_9 ? 16.0f / 9.0f :
                             aspectRatio == AspectRatio._4_3 ? 4.0f / 3.0f :
                             Camera.main.aspect;

        if (projectionType == Type.Perspective)
        {
            Camera.main.orthographic = false;
            Camera.main.fieldOfView = FOV;
        }
        else
        {
            
            Camera.main.orthographic = true;
            Camera.main.orthographicSize = ortographicWidth / 2.0f / Camera.main.aspect;
        }

        Camera.main.nearClipPlane = nearPlane;
        Camera.main.farClipPlane = farPlane;
    }

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
