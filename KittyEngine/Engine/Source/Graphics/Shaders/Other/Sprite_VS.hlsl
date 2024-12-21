#include "Commons/common.hlsli"
#include "Commons/spriteCommon.hlsli"

float4x4 CreateRotationAroundY(float angleRad)
{
    float4x4 rotate = float4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
	);

    rotate._11 = cos(angleRad);
    rotate._13 = -sin(angleRad);
    rotate._31 = sin(angleRad);
    rotate._33 = cos(angleRad);

    return rotate;
}

float4x4 CreateRotationAroundX(float angleRad)
{
    float4x4 rotate = float4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
	);
    rotate._22 = cos(angleRad);
    rotate._23 = sin(angleRad);
    rotate._32 = -sin(angleRad);
    rotate._33 = cos(angleRad);

    return rotate;
}

float4x4 CreateRotationAroundZ(float angleRad)
{
    float4x4 rotate = float4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
	);
    rotate._11 = cos(angleRad);
    rotate._12 = sin(angleRad);
    rotate._21 = -sin(angleRad);
    rotate._22 = cos(angleRad);

    return rotate;
}

SpritePixelInput main(const SpriteVertexInput input)
{
    SpritePixelInput output;

    float4 pos = input.position;
    float4x4 instanceTransform = input.transform;



    switch (displayMode)
    {
        case 0://normal
        {
            pos = mul(instanceTransform, pos);
            pos = mul(worldToClipSpaceMatrix, pos);

            break;
        }
        case 1: //billboard
        {

            float3 instanceRight = instanceTransform._m00_m10_m20;
            float3 instanceUp = instanceTransform._m01_m11_m21;
            float3 instanceForward = instanceTransform._m02_m12_m22;

            float scaleX = length(instanceRight);
            float scaleY = length(instanceUp);
            float scaleZ = length(instanceForward);

            //set instancetransform's rotation to completeTransform's rotation
            float4x4 instanceRot = instanceTransform;
            instanceRot[0][0] = completeTransform[0][0];
            instanceRot[0][1] = completeTransform[0][1];
            instanceRot[0][2] = completeTransform[0][2];
            instanceRot[1][0] = completeTransform[1][0];
            instanceRot[1][1] = completeTransform[1][1];
            instanceRot[1][2] = completeTransform[1][2];
            instanceRot[2][0] = completeTransform[2][0];
            instanceRot[2][1] = completeTransform[2][1];
            instanceRot[2][2] = completeTransform[2][2];

            //instanceRot[0] *= scaleX;
            //instanceRot[1] *= scaleY;
            //instanceRot[2] *= scaleZ; add these to the scale transform
            float4x4 scaleTransform = float4x4(
                scaleX, 0.0f, 0.0f, 0.0f,
                0.0f, scaleY, 0.0f, 0.0f,
                0.0f, 0.0f, scaleZ, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
            
            pos = mul(scaleTransform, pos);


            //calculate the billboard rotation, based on forward and right vectors
            float rotation = atan2(instanceForward.x, instanceForward.z);

            pos = mul(CreateRotationAroundZ(rotation), pos);

            pos = mul(instanceRot, pos);


            pos = mul(worldToClipSpaceMatrix, pos);


            break;
        }
        case 2: //screen
        {
            pos = mul(instanceTransform, pos);
            pos = mul(completeTransform, pos);

            float xm = (1.0f / float(clientResolution.x));
            float xy = (1.0f / float(clientResolution.y));

            pos.x += (input.transform[0][0] * xm - 1.0f);
            pos.y -= (input.transform[1][1] * xy - 1.0f);
            
            pos.x += (input.transform[0][3] / float(clientResolution.x));
            pos.y -= (input.transform[1][3] / float(clientResolution.y)) * 3.0f;
            
            break;
        }
		case 3: //screenText
        {
            pos = mul(instanceTransform, pos);
            pos = mul(completeTransform, pos);
            break;
        }
        
    }
    output.position = pos;

    output.color = input.color;
    
    output.uv = input.texCoord;


    output.spriteBounds = input.uvRect;
    output.textureRegion = input.uvRegion;
    return output;
}   