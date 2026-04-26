#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"


Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
Texture2D effectsTex : register(t3);

struct VFXPixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;

    float4 centerWorldPos : CENTERWORLDPOS;
    float4 minWorldPos : MINWORLDPOS;
    float4 maxWorldPos : MAXWORLDPOS;

    float4 worldUPDir : WORLDUP;
};

cbuffer VFXInstance : register(b6) //6 is arbitrary, need to sync up with other graphics
{
    float4 color;
    float2 uvScroll;
    float2 uvScale;
    float4 bloomAttributes;
}

struct ExplosionVFXBlock
{
    float angleMin;
    float angleMax;
    float rangeMin;
    float rangeMax;
};

cbuffer ExplosionBuffer : register(b7)
{
    ExplosionVFXBlock explosionBlock[16];
};

struct MultiPixelOutput
{
    float4 color1 : SV_Target0;
    float4 color2 : SV_Target1;
};

MultiPixelOutput main(VFXPixelInput aInput)
{
    MultiPixelOutput output;

    float2 pos = aInput.texCoord;
    float2 center = float2(0.5f, 0.5f);

    float2 dir = pos - center;
    float len = length(dir);

    float modelScl = length(aInput.maxWorldPos.xz - aInput.minWorldPos.xz);
    

    float outerEdgeDistance = 1.0f / (2.0f);
    float innerEdgeDistance = 1.0f / (4.0f - modelScl / 6.0f);

    float4 rimColour = float4(1.0f, 0.2f, 0.0f, 1.0f);
    float4 fadeColour = float4(1.0f, 0.8f, 0.0f, 1.0f);

    float distanceToCenter = length(aInput.worldPos.xz - aInput.centerWorldPos.xz);
    float progress = (len - outerEdgeDistance) / (innerEdgeDistance - outerEdgeDistance);

	const float4 lerpedColour = lerp(rimColour, fadeColour, progress);

    float angle = atan2(dir.y, dir.x);
    angle -= 3.14159f / 2.0f;
    //angle = 3.14159f - angle;
    if (angle < 0.0f)
    {
        angle += 2.0f * 3.14159f;
    }

    float blocked = false;
    for (int i = 0; i < 16; i++)
    {
        ExplosionVFXBlock block = explosionBlock[i];
        const float padding = 0.01f;
        if (angle >= block.angleMin - padding && angle <= block.angleMax + padding)
	    {
	        float angleT = (angle - block.angleMin) / (block.angleMax - block.angleMin);
	        float range = lerp(block.rangeMin, block.rangeMax, angleT);
	        if (range < distanceToCenter)
	        {
	            blocked = true;
	        }
	    }
    }

    float alpha = saturate(1.0f - progress);
    if (progress < 0.0f || progress > 1.0f)
    {
        alpha = 0.0f;
    }

    alpha = pow(alpha, 5.0f);
    alpha *= 1.0f - blocked;

    output.color1 = lerpedColour * alpha;
    output.color2 = lerpedColour * alpha;

    return output; 
}